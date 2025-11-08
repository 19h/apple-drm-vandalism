#include "context.h"
#include "auth.h"
#include "apple_music_bridge.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// --- Global Constants Implementation ---
// These are defined here to ensure they have C linkage and are found by the linker.
// They are declared as 'extern' in apple_music_bridge.h.

const char *const ANDROID_ID = "dc28071e981c439e";

const char *const FAIRPLAY_CERTIFICATE =
    "MIIEzjCCA7agAwIBAgIIAXAVjHFZDjgwDQYJKoZIhvcNAQEFBQAwfzELMAkGA1UEBhMCVVMxEz"
    "ARBgNVBAoMCkFwcGxlIEluYy4xJjAkBgNVBAsMHUFwcGxlIENlcnRpZmljYXRpb24gQXV0aG9y"
    "aXR5MTMwMQYDVQQDDCpBcHBsZSBLZXkgU2VydmljZXMgQ2VydGlmaWNhdGlvbiBBdXRob3JpdH"
    "kwHhcNMTIwNzI1MTgwMjU4WhcNMTQwNzI2MTgwMjU4WjAwMQswCQYDVQQGEwJVUzESMBAGA1UE"
    "CgwJQXBwbGUgSW5jMQ0wCwYDVQQDDARGUFMxMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQ"
    "CqZ9IbMt0J0dTKQN4cUlfeQRY9bcnbnP95HFv9A16Yayh4xQzRLAQqVSmisZtBK2/nawZcDmcs"
    "+XapBojRb+jDM4Dzk6/Ygdqo8LoA+BE1zipVyalGLj8Y86hTC9QHX8i05oWNCDIlmabjjWvFBo"
    "EOk+ezOAPg8c0SET38x5u+TwIDAQABo4ICHzCCAhswHQYDVR0OBBYEFPP6sfTWpOQ5Sguf5W3Y"
    "0oibbEc3MAwGA1UdEwEB/wQCMAAwHwYDVR0jBBgwFoAUY+RHVMuFcVlGLIOszEQxZGcDLL4wge"
    "IGA1UdIASB2jCB1zCB1AYJKoZIhvdjZAUBMIHGMIHDBggrBgEFBQcCAjCBtgyBs1JlbGlhbmNl"
    "IG9uIHRoaXMgY2VydGlmaWNhdGUgYnkgYW55IHBhcnR5IGFzc3VtZXMgYWNjZXB0YW5jZSBvZi"
    "B0aGUgdGhlbiBhcHBsaWNhYmxlIHN0YW5kYXJkIHRlcm1zIGFuZCBjb25kaXRpb25zIG9mIHVz"
    "ZSwgY2VydGlmaWNhdGUgcG9saWN5IGFuZCBjZXJ0aWZpY2F0aW9uIHByYWN0aWNlIHN0YXRlbW"
    "VudHMuMDUGA1UdHwQuMCwwKqAooCaGJGh0dHA6Ly9jcmwuYXBwbGUuY29tL2tleXNlcnZpY2Vz"
    "LmNybDAOBgNVHQ8BAf8EBAMCBSAwFAYLKoZIhvdjZAYNAQUBAf8EAgUAMBsGCyqGSIb3Y2QGDQ"
    "EGAQH/BAkBAAAAAQAAAAEwKQYLKoZIhvdjZAYNAQMBAf8EFwF+bjsY57ASVFmeehD2bdu6HLGB"
    "xeC2MEEGCyqGSIb3Y2QGDQEEAQH/BC8BHrKviHJf/Se/ibc7T0/55Bt1GePzaYBVfgF3ZiNuV9"
    "3z8P3qsawAqAXzzh9o5DANBgkqhkiG9w0BAQUFAAOCAQEAVGyCtuLYcYb/aPijBCtaemxuV0Io"
    "kXJn3EgmwYHZynaR6HZmeGRUp9p3f8EXu6XPSekKCCQi+a86hXX9RfnGEjRdvtP+jts5MDSKuU"
    "Ioaqce8cLX2dpUOZXdf3lR0IQM0kXHb5boNGBsmbTLVifqeMsexfZryGw2hE/4WDOJdGQm1gMJ"
    "ZU4jP1b/HSLNIUhHWAaMeWtcJTPRBucR4urAtvvtOWD88mriZNHG+veYw55b+qA36PSqDPMbku"
    "9xTY7fsMa6mxIRmwULQgi8nOk1wNhw3ZO0qUKtaCO3gSqWdloecxpxUQSZCSW7tWPkpXXwDZqe"
    "gUkij9xMFS1pr37RIjCCBVAwggQ4oAMCAQICEEVKuaGraq1Cp4z6TFOeVfUwDQYJKoZIhvcNAQ"
    "ELBQAwUDEsMCoGA1UEAwwjQXBwbGUgRlAgU2VydmljZSBFbmFibGUgUlNBIENBIC0gRzExEzAR"
    "BgNVBAoMCkFwcGxlIEluYy4xCzAJBgNVBAYTAlVTMB4XDTIwMDQwNzIwMjY0NFoXDTIyMDQwNz"
    "IwMjY0NFowWjEhMB8GA1UEAwwYZnBzMjA0OC5pdHVuZXMuYXBwbGUuY29tMRMwEQYDVQQLDApB"
    "cHBsZSBJbmMuMRMwEQYDVQQKDApBcHBsZSBJbmMuMQswCQYDVQQGEwJVUzCCASIwDQYJKoZIhv"
    "cNAQEBBQADggEPADCCAQoCggEBAJNoUHuTRLafofQgIRgGa2TFIf+bsFDMjs+y3Ep1xCzFLE4Q"
    "bnwG6OG0duKUl5IoGUsouzZk9iGsXz5k3ESLOWKz2BFrDTvGrzAcuLpH66jJHGsk/l+ZzsDOJa"
    "oQ22pu0JvzYzW8/yEKvpE6JF/2dsC6V9RDTri3VWFxrl5uh8czzncoEQoRcQsSatHzs4tw/QdH"
    "FtBIigqxqr4R7XiCaHbsQmqbP9h7oxRs/6W/DDA2BgkuFY1ocX/8dTjmH6szKPfGt3KaYCwy3f"
    "uRC+FibTyohtvmlXsYhm7AUzorwWIwN/MbiFQ0OHHtDomIy71wDcTNMnY0jZYtGmIlJETAgYcC"
    "AwEAAaOCAhowggIWMAwGA1UdEwEB/wQCMAAwHwYDVR0jBBgwFoAUrI/yBkpV623/IeMrXzs8fC"
    "7VkZkwRQYIKwYBBQUHAQEEOTA3MDUGCCsGAQUFBzABhilodHRwOi8vb2NzcC5hcHBsZS5jb20v"
    "b2NzcDAzLWZwc3J2cnNhZzEwMzCBwwYDVR0gBIG7MIG4MIG1BgkqhkiG92NkBQEwgacwgaQGCC"
    "sGAQUFBwICMIGXDIGUUmVsaWFuY2Ugb24gdGhpcyBjZXJ0aWZpY2F0ZSBieSBhbnkgcGFydHkg"
    "YXNzdW1lcyBhY2NlcHRhbmNlIG9mIGFueSBhcHBsaWNhYmxlIHRlcm1zIGFuZCBjb25kaXRpb2"
    "5zIG9mIHVzZSBhbmQvb3IgY2VydGlmaWNhdGlvbiBwcmFjdGljZSBzdGF0ZW1lbnRzLjAdBgNV"
    "HQ4EFgQU2RpCSSHFXeoZQQWxbwJuRZ9RrIEwDgYDVR0PAQH/BAQDAgUgMBQGCyqGSIb3Y2QGDQ"
    "EFAQH/BAIFADAjBgsqhkiG92NkBg0BBgEB/wQRAQAAAAMAAAABAAAAAgAAAAMwOQYLKoZIhvdj"
    "ZAYNAQMBAf8EJwG+pUeWbeZBUI0PikyFwSggL5dHaeugSDoQKwcP28csLuh5wplpATAzBgsqhk"
    "iG92NkBg0BBAEB/wQhAfl9TGjP/UY9TyQzYsn8sX9ZvHChok9QrrUhtAyWR1yCMA0GCSqGSIb3"
    "DQEBCwUAA4IBAQBNMzZ6llQ0laLXsrmyVieuoW9+pHeAaDJ7cBiQLjM3ZdIO3Gq5dkbWYYYwJw"
    "ymdxZ74WGZMuVv3ueJKcxG1jAhCRhr0lb6QaPaQQSNW+xnoesb3CLA0RzrcgBp/9WFZNdttJOS"
    "yC93lQmiE0r5RqPpe/IWUzwoZxri8qnsghVFxCBEcMB+U4PJR8WeAkPrji8po2JLYurvgNRhGk"
    "DKcAFPuGEpXdF86hPts+07zazsP0fBjBSVgP3jqb8G31w5W+O+wBW0B9uCf3s0vXU4LuJTAyww"
    "s2ImZ7O/AaY/uXWOyIUMUKPgL1/QJieB7pBoENIJ2CeJS2M3iv00ssmCmTEJ";

// --- Static Helper Function Prototypes ---

static bool initialize_environment(const app_config_t *config);
static bool create_request_context(app_context_t *context);
static bool initialize_playback_lease(app_context_t *context);

// --- Public API Implementation ---

app_context_t *context_create(const app_config_t *config) {
    fprintf(stderr, "[INFO] Initializing application context...\n");

    app_context_t *context = calloc(1, sizeof(app_context_t));
    if (!context) {
        perror("Failed to allocate memory for app_context");
        return NULL;
    }
    context->config = config;

    if (pthread_mutex_init(&context->preshare_mutex, NULL) != 0) {
        perror("Failed to initialize mutex");
        goto cleanup_error;
    }

    if (!initialize_environment(config)) {
        fprintf(stderr, "[ERROR] Failed to initialize environment.\n");
        goto cleanup_error;
    }

    if (!create_request_context(context)) {
        fprintf(stderr, "[ERROR] Failed to create request context.\n");
        goto cleanup_error;
    }

    if (config->login_given) {
        if (!auth_perform_login(context)) {
            fprintf(stderr, "[ERROR] Authentication failed.\n");
            goto cleanup_error;
        }
        fprintf(stderr, "[INFO] Login successful.\n");
    }

    if (!initialize_playback_lease(context)) {
        fprintf(stderr, "[ERROR] Failed to initialize playback lease manager.\n");
        goto cleanup_error;
    }

    context->foothill_instance = fh_session_ctrl_instance();
    if (!context->foothill_instance) {
        fprintf(stderr, "[ERROR] Failed to get FootHill instance.\n");
        goto cleanup_error;
    }

    fprintf(stderr, "[INFO] Application context initialized successfully.\n");
    return context;

cleanup_error:
    context_destroy(context);
    return NULL;
}

void context_destroy(app_context_t *context) {
    if (!context) {
        return;
    }
    pthread_mutex_destroy(&context->preshare_mutex);
    free(context);
    fprintf(stderr, "[INFO] Application context destroyed.\n");
}


// --- Static Helper Function Implementations ---

static bool initialize_environment(const app_config_t *config) {
    fprintf(stderr, "[INFO] Initializing environment...\n");
    setenv("ANDROID_DNS_MODE", "local", 1);

    if (config->proxy && strlen(config->proxy) > 0) {
        fprintf(stderr, "[INFO] Using proxy: %s\n", config->proxy);
        setenv("http_proxy", config->proxy, 1);
        setenv("https_proxy", config->proxy, 1);
    }

    static const char *resolvers[] = {"1.1.1.1", "1.0.0.1"};
    resolv_set_nameservers_for_net(0, resolvers, 2, ".");

    am_string_t android_id_str = am_string_new(ANDROID_ID);
    fh_config(&android_id_str);

    am_shared_ptr_t guid_instance;
    ssc_device_guid_instance(&guid_instance);

    static uint8_t guid_ret[88];
    static unsigned int sdk_version = 29;
    static uint8_t is_debug = 1;
    am_string_t empty_str = am_string_new_short("");
    ssc_device_guid_configure(
        &guid_ret, guid_instance.obj, &android_id_str, &empty_str, &sdk_version, &is_debug);

    return true;
}

static bool create_request_context(app_context_t *context) {
    fprintf(stderr, "[INFO] Creating request context...\n");

    am_string_t db_path = am_string_new("/data/data/com.apple.android.music/files/mpl_db");
    std_make_shared_request_context(&context->request_context, &db_path);

    uint8_t cfg_ptr_buf[480];
    *(void **)(cfg_ptr_buf) = &VTABLE_SSC_REQUEST_CONTEXT_CONFIG;
    am_shared_ptr_t req_ctx_cfg = {.obj = cfg_ptr_buf + 32, .ctrl_blk = cfg_ptr_buf};
    ssc_request_context_config_ctor(req_ctx_cfg.obj);

    am_string_t str_buf;
    str_buf = am_string_new("/data/data/com.apple.android.music/files/mpl_db");
    ssc_request_context_config_set_base_dir(req_ctx_cfg.obj, &str_buf);
    str_buf = am_string_new_short("Music");
    ssc_request_context_config_set_client_id(req_ctx_cfg.obj, &str_buf);
    str_buf = am_string_new_short("4.9");
    ssc_request_context_config_set_version_id(req_ctx_cfg.obj, &str_buf);
    str_buf = am_string_new_short("Android");
    ssc_request_context_config_set_platform_id(req_ctx_cfg.obj, &str_buf);
    str_buf = am_string_new_short("10");
    ssc_request_context_config_set_product_version(req_ctx_cfg.obj, &str_buf);
    str_buf = am_string_new_short("Samsung S9");
    ssc_request_context_config_set_device_model(req_ctx_cfg.obj, &str_buf);
    str_buf = am_string_new_short("7663313");
    ssc_request_context_config_set_build_version(req_ctx_cfg.obj, &str_buf);
    str_buf = am_string_new_short("en-US");
    ssc_request_context_config_set_locale_id(req_ctx_cfg.obj, &str_buf);
    ssc_request_context_config_set_lang_id(req_ctx_cfg.obj, &str_buf);

    ssc_request_context_manager_configure(&context->request_context);
    static uint8_t init_buf[88];
    ssc_request_context_init(&init_buf, context->request_context.obj, &req_ctx_cfg);

    str_buf = am_string_new("/data/data/com.apple.android.music/files");
    ssc_request_context_set_fairplay_dir(context->request_context.obj, &str_buf);

    std_make_shared_android_presentation_interface(&context->presentation_interface);
    api_set_dialog_handler(context->presentation_interface.obj, &auth_dialog_handler);
    api_set_credentials_handler(context->presentation_interface.obj, &auth_credential_handler);
    ssc_request_context_set_presentation_interface(
        context->request_context.obj, &context->presentation_interface);

    return true;
}

static bool initialize_playback_lease(app_context_t *context) {
    fprintf(stderr, "[INFO] Initializing playback lease manager...\n");

    extern void *endLeaseCallback;
    extern void *pbErrCallback;

    plm_ctor(context->lease_manager, &endLeaseCallback, &pbErrCallback);

    uint8_t auto_refresh = 1;
    plm_refresh_lease_auto(context->lease_manager, &auto_refresh);
    plm_request_lease(context->lease_manager, &auto_refresh);

    return true;
}

// --- Stub for Android-specific resolver function ---
// This is required to satisfy the linker on non-Android systems.
void _resolv_set_nameservers_for_net(unsigned netid, const char **servers, int numservers, const char *domains) {
    (void)netid;
    (void)servers;
    (void)numservers;
    (void)domains;
    // This is a no-op stub.
}
