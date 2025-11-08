#pragma once

#include "context.h"

/**
 * @brief Handles a new client connection for the decryption service.
 *
 * This function manages the entire lifecycle of a single decryption request.
 * It reads the track metadata (Adam ID, key URI) from the client, obtains a
 * decryption context using the DRM module, and then enters a loop to read
 * encrypted audio samples, decrypt them, and write them back to the client.
 * The connection is closed upon completion or error.
 *
 * @param conn_fd The file descriptor for the accepted client socket.
 * @param context The application context, required for accessing the DRM module.
 */
void handlers_handle_decrypt_connection(int conn_fd, app_context_t *context);

/**
 * @brief Handles a new client connection for the M3U8 service.
 *
 * This function reads a track's Adam ID from the client, requests the
 * corresponding M3U8 playlist URL from the Apple Music service, and writes
 * the URL back to the client. The connection is closed after the response
 * is sent.
 *
 * @param conn_fd The file descriptor for the accepted client socket.
 * @param context The application context, required for making asset requests.
 */
void handlers_handle_m3u8_connection(int conn_fd, app_context_t *context);

/**
 * @brief Retrieves the M3U8 playlist URL for a given Adam ID.
 *
 * This function encapsulates the interaction with the SVPlaybackLeaseManager
 * to request a playback asset.
 *
 * @param context The application context.
 * @param adam_id The Adam ID of the track.
 * @return A dynamically allocated string containing the M3U8 URL on success,
 *         or NULL on failure. The caller is responsible for freeing the
 *         returned string.
 */
char *handlers_get_m3u8_url(app_context_t *context, unsigned long adam_id);
