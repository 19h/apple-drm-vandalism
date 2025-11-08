#pragma once

#include "context.h"

/**
 * @brief Retrieves a key decryption context for a given track.
 *
 * This function handles the interaction with the FairPlay DRM system to obtain
 * an opaque handle (the kd_context) that can be used to decrypt audio samples.
 *
 * It includes a thread-safe caching mechanism for the "preshare" context
 * (where adam_id is "0") to improve performance.
 *
 * @param context The application context, containing the DRM session instance
 *                and the preshare context cache.
 * @param adam_id The Adam ID (unique identifier) of the track.
 * @param uri The key URI as specified in the M3U8 playlist.
 * @return A void pointer to the opaque kd_context on success, or NULL on
 *         failure. The caller should not attempt to inspect or free this
 *         pointer.
 */
void *drm_get_kd_context(app_context_t *context, const char *adam_id, const char *uri);
