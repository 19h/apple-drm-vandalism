#include "auth.h"
#include "apple_music_bridge.h"
#include "context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

// The file path for 2FA code when using the file-based method.
#define TWO_FA_CODE_FILE_PATH "/data/2fa.txt"
#define TWO_FA_CODE_LENGTH 6

/**
 * @brief A module-scoped global context pointer.
 *
 * This is a workaround for the C++ library's callback mechanism, which does
 * not provide a `userdata` parameter. This pointer is set by
 * `auth_perform_login` before running the flow and is cleared immediately
 * after, minimizing its scope of use. The callbacks use this pointer to
 * access the application's configuration and state.
 */
static app_context_t *g_auth_context = NULL;

// --- Static Helper Functions for 2FA ---

static bool file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

static bool read_2fa_code_from_file(char *out_buffer, size_t buffer_len) {
    fprintf(stderr, "[PROMPT] Enter your 2FA code into the file: %s\n", TWO_FA_CODE_FILE_PATH);
    fprintf(stderr, "[PROMPT] Waiting for input...\n");

    for (int i = 0; i < 20; ++i) { // Wait for up to 60 seconds
        if (file_exists(TWO_FA_CODE_FILE_PATH)) {
            FILE *fp = fopen(TWO_FA_CODE_FILE_PATH, "r");
            if (!fp) {
                perror("Failed to open 2FA code file");
                return false;
            }
            if (fgets(out_buffer, buffer_len, fp) == NULL) {
                fprintf(stderr, "[ERROR] Failed to read from 2FA code file.\n");
                fclose(fp);
                remove(TWO_FA_CODE_FILE_PATH);
                return false;
            }
            fclose(fp);
            remove(TWO_FA_CODE_FILE_PATH);
            // Clean up potential newline characters
            out_buffer[strcspn(out_buffer, "\r\n")] = 0;
            fprintf(stderr, "[INFO] 2FA code file detected. Logging in...\n");
            return true;
        }
        sleep(3);
    }

    fprintf(stderr, "[ERROR] Timed out waiting for 2FA code file.\n");
    return false;
}

static bool read_2fa_code_from_stdin(char *out_buffer, size_t buffer_len) {
    printf("[PROMPT] Please enter your 6-digit 2FA code: ");
    fflush(stdout);
    if (fgets(out_buffer, buffer_len, stdin) == NULL) {
        fprintf(stderr, "[ERROR] Failed to read 2FA code from stdin.\n");
        return false;
    }
    // Clean up potential newline characters
    out_buffer[strcspn(out_buffer, "\r\n")] = 0;
    return true;
}


// --- Public API Implementation ---

bool auth_perform_login(app_context_t *context) {
    fprintf(stderr, "[INFO] Starting authentication flow...\n");

    // Set the global context for the callbacks to use.
    g_auth_context = context;

    am_shared_ptr_t flow;
    std_make_shared_auth_flow(&flow, &context->request_context);

    // This is a blocking call that will trigger the callbacks.
    ssc_auth_flow_run(flow.obj);

    am_shared_ptr_t *resp = ssc_auth_flow_response(flow.obj);
    bool success = false;
    if (resp && resp->obj) {
        const int resp_type = ssc_auth_response_type(resp->obj);
        fprintf(stderr, "[INFO] Authentication response type: %d\n", resp_type);
        if (resp_type == 6) { // 6 indicates success
            success = true;
        }
    } else {
        fprintf(stderr, "[ERROR] Authentication flow returned no response.\n");
    }

    // Clear the global context now that the flow is complete.
    g_auth_context = NULL;
    return success;
}

void auth_dialog_handler(long j, am_shared_ptr_t *proto_dialog_ptr, am_shared_ptr_t *resp_handler) {
    (void)resp_handler;
    if (!g_auth_context) return;

    const char *title = am_string_data(ssc_protocol_dialog_title(proto_dialog_ptr->obj));
    const char *message = am_string_data(ssc_protocol_dialog_message(proto_dialog_ptr->obj));
    fprintf(stderr, "[CALLBACK] Dialog: {title: '%s', message: '%s'}\n", title, message);

    // Manually construct the ProtocolDialogResponse object on the stack.
    uint8_t resp_ptr_buf[72];

    *(void **)(resp_ptr_buf) = &VTABLE_SSC_PROTOCOL_DIALOG_RESPONSE;
    am_shared_ptr_t diag_resp = {.obj = resp_ptr_buf + 24, .ctrl_blk = resp_ptr_buf};
    ssc_protocol_dialog_response_ctor(diag_resp.obj);

    // If this is the initial "Sign In" dialog, find and select the "Use Existing Apple ID" button.
    if (strcmp("Sign In", title) == 0) {
        am_vector_t *buttons = ssc_protocol_dialog_buttons(proto_dialog_ptr->obj);
        for (am_shared_ptr_t *b = buttons->begin; b != buttons->end; ++b) {
            const char *btn_title = am_string_data(ssc_protocol_button_title(b->obj));
            if (strcmp("Use Existing Apple ID", btn_title) == 0) {
                ssc_protocol_dialog_response_set_selected_button(diag_resp.obj, b);
                break;
            }
        }
    }

    api_handle_protocol_dialog_response(
        g_auth_context->presentation_interface.obj, &j, &diag_resp);
}

void auth_credential_handler(am_shared_ptr_t *cred_req_handler, am_shared_ptr_t *cred_resp_handler) {
    (void)cred_resp_handler;

    if (!g_auth_context) return;

    const app_config_t *config = g_auth_context->config;
    const bool needs_2fa = ssc_credentials_request_requires_hsa2(cred_req_handler->obj);
    const char *title = am_string_data(ssc_credentials_request_title(cred_req_handler->obj));
    const char *message = am_string_data(ssc_credentials_request_message(cred_req_handler->obj));
    fprintf(stderr, "[CALLBACK] Credential Request: {title: '%s', message: '%s', 2FA: %s}\n", title, message, needs_2fa ? "yes" : "no");

    if (needs_2fa) {
        char code_buf[TWO_FA_CODE_LENGTH + 2] = {0}; // +2 for newline and null
        bool code_read_ok = false;
        if (config->code_from_file) {
            code_read_ok = read_2fa_code_from_file(code_buf, sizeof(code_buf));
        } else {
            code_read_ok = read_2fa_code_from_stdin(code_buf, sizeof(code_buf));
        }

        if (!code_read_ok || strlen(code_buf) != TWO_FA_CODE_LENGTH) {
            fprintf(stderr, "[ERROR] Invalid 2FA code received.\n");
            // Abort by not responding? Or respond with empty? For now, just return.
            return;
        }
        // Append the 2FA code to the password. The buffer in config was allocated to be large enough.
        strcat(config->password, code_buf);
    }

    // Manually construct the CredentialsResponse object on the stack.
    uint8_t resp_ptr_buf[80];

    *(void **)(resp_ptr_buf) = &VTABLE_SSC_CREDENTIALS_RESPONSE;
    am_shared_ptr_t cred_resp = {.obj = resp_ptr_buf + 24, .ctrl_blk = resp_ptr_buf};
    ssc_credentials_response_ctor(cred_resp.obj);

    // Populate the response with credentials.
    am_string_t username_str = am_string_new(config->username);
    ssc_credentials_response_set_username(cred_resp.obj, &username_str);

    am_string_t password_str = am_string_new(config->password);
    ssc_credentials_response_set_password(cred_resp.obj, &password_str);

    ssc_credentials_response_set_response_type(cred_resp.obj, 2); // 2 = OK/Submit

    api_handle_credentials_response(
        g_auth_context->presentation_interface.obj, &cred_resp);
}
