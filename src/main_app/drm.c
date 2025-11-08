#include "drm.h"
#include "apple_music_bridge.h"

#include <stdio.h>
#include <string.h>

void *drm_get_kd_context(app_context_t *context, const char *adam_id, const char *uri) {
    // Check if this is a request for the special "preshare" key context.
    bool is_preshare = (strcmp("0", adam_id) == 0);

    if (is_preshare) {
        // --- Thread-Safe Preshare Context Caching ---
        pthread_mutex_lock(&context->preshare_mutex);
        // Double-check if another thread initialized it while we were waiting for the lock.
        if (context->preshare_kd_context != NULL) {
            pthread_mutex_unlock(&context->preshare_mutex);
            return context->preshare_kd_context;
        }
        // If we are the first, we will generate it and then unlock.
    }

    fprintf(stderr, "[INFO] Requesting kd_context for adamId: %s, uri: %s\n", adam_id, uri);

    // --- Context Generation ---

    // 1. Prepare all string arguments required by the C++ library.
    am_string_t adam_id_str = am_string_new(adam_id);
    am_string_t key_uri_str = am_string_new(uri);
    am_string_t key_format_str = am_string_new("com.apple.streamingkeydelivery");
    am_string_t key_format_ver_str = am_string_new_short("1");
    am_string_t server_uri_str = am_string_new("https://play.itunes.apple.com/WebObjects/MZPlay.woa/music/fps");
    am_string_t protocol_type_str = am_string_new("simplified");
    am_string_t fps_cert_str = am_string_new(FAIRPLAY_CERTIFICATE);

    // 2. Get the encrypted persistent key.
    am_shared_ptr_t persistent_key = {.obj = NULL};
    fh_session_ctrl_get_persistent_key(
        &persistent_key, context->foothill_instance, &adam_id_str, &adam_id_str,
        &key_uri_str, &key_format_str, &key_format_ver_str, &server_uri_str,
        &protocol_type_str, &fps_cert_str);

    if (persistent_key.obj == NULL) {
        fprintf(stderr, "[ERROR] Failed to get persistent key for adamId: %s\n", adam_id);
        if (is_preshare) pthread_mutex_unlock(&context->preshare_mutex);
        return NULL;
    }

    // 3. Decrypt the persistent key to get a temporary context object.
    am_shared_ptr_t p_context;
    fh_session_ctrl_decrypt_context(
        &p_context, context->foothill_instance, (am_string_t *)persistent_key.obj);

    if (p_context.obj == NULL) {
        fprintf(stderr, "[ERROR] Failed to decrypt context for adamId: %s\n", adam_id);
        if (is_preshare) pthread_mutex_unlock(&context->preshare_mutex);
        return NULL;
    }

    // 4. Extract the final, usable kd_context handle.
    void *kd_context = *fh_pcontext_get_kd_context(p_context.obj);
    if (kd_context == NULL) {
        fprintf(stderr, "[ERROR] Extracted kd_context is NULL for adamId: %s\n", adam_id);
        if (is_preshare) pthread_mutex_unlock(&context->preshare_mutex);
        return NULL;
    }

    // If this was a preshare key, cache it before releasing the lock.
    if (is_preshare) {
        context->preshare_kd_context = kd_context;
        pthread_mutex_unlock(&context->preshare_mutex);
    }

    return kd_context;
}
