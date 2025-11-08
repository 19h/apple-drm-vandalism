#define _GNU_SOURCE
#include "handlers.h"
#include "apple_music_bridge.h"
#include "drm.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>

// --- Static I/O Helper Functions ---

/**
 * @brief Reads an exact number of bytes from a file descriptor.
 *
 * This function repeatedly calls read() until the requested number of bytes
 * has been read or an error occurs.
 *
 * @param fd The file descriptor to read from.
 * @param buf The buffer to store the read data.
 * @param count The number of bytes to read.
 * @return true on success, false on failure (e.g., EOF or error).
 */
static bool read_full(int fd, void *buf, size_t count) {
    size_t bytes_read = 0;
    while (bytes_read < count) {
        ssize_t result = read(fd, (uint8_t *)buf + bytes_read, count - bytes_read);
        if (result <= 0) {
            if (result < 0) perror("read");
            return false; // Error or EOF
        }
        bytes_read += result;
    }
    return true;
}

/**
 * @brief Writes an exact number of bytes to a file descriptor.
 *
 * This function repeatedly calls write() until all bytes have been written
 * or an error occurs.
 *
 * @param fd The file descriptor to write to.
 * @param buf The buffer containing the data to write.
 * @param count The number of bytes to write.
 * @return true on success, false on failure.
 */
static bool write_full(int fd, const void *buf, size_t count) {
    size_t bytes_written = 0;
    while (bytes_written < count) {
        ssize_t result = write(fd, (const uint8_t *)buf + bytes_written, count - bytes_written);
        if (result < 0) {
            perror("write");
            return false;
        }
        bytes_written += result;
    }
    return true;
}


// --- Public API Implementation ---

void handlers_handle_decrypt_connection(int conn_fd, app_context_t *context) {
    while (true) {
        uint8_t adam_size;
        if (!read_full(conn_fd, &adam_size, sizeof(adam_size)) || adam_size == 0) break;

        char adam_id[adam_size + 1];
        if (!read_full(conn_fd, adam_id, adam_size)) break;
        adam_id[adam_size] = '\0';

        uint8_t uri_size;
        if (!read_full(conn_fd, &uri_size, sizeof(uri_size))) break;

        char uri[uri_size + 1];
        if (!read_full(conn_fd, uri, uri_size)) break;
        uri[uri_size] = '\0';

        void *kd_context = drm_get_kd_context(context, adam_id, uri);
        if (kd_context == NULL) {
            fprintf(stderr, "[ERROR] Could not get kd_context for adamId %s. Closing connection.\n", adam_id);
            break;
        }

        // Loop to decrypt audio samples for the current track.
        while (true) {
            uint32_t sample_size_net;
            if (!read_full(conn_fd, &sample_size_net, sizeof(sample_size_net))) break;

            uint32_t sample_size = ntohl(sample_size_net);
            if (sample_size == 0) break; // End of track signal

            uint8_t *sample_buf = malloc(sample_size);
            if (sample_buf == NULL) {
                perror("malloc for sample buffer failed");
                break;
            }

            if (!read_full(conn_fd, sample_buf, sample_size)) {
                free(sample_buf);
                break;
            }

            fh_decrypt_sample(kd_context, 5, sample_buf, sample_buf, sample_size);

            if (!write_full(conn_fd, sample_buf, sample_size)) {
                free(sample_buf);
                break;
            }
            free(sample_buf);
        }
    }
}

char *handlers_get_m3u8_url(app_context_t *context, unsigned long adam_id) {
    am_string_t hls_str = am_string_new_short("HLS");
    am_vector_t hls_param_vec = am_vector_new(&hls_str);
    static uint8_t z0 = 0;
    am_shared_ptr_t asset_resp;

    plm_request_asset(&asset_resp, context->lease_manager, &adam_id, &hls_param_vec, &z0);

    if (asset_resp.obj == NULL) {
        fprintf(stderr, "[WARN] Asset request for adamId %lu returned null response.\n", adam_id);
        return NULL;
    }

    if (!ssc_playback_asset_response_has_valid_asset(asset_resp.obj)) {
        fprintf(stderr, "[WARN] Asset request for adamId %lu returned no valid asset.\n", adam_id);
        return NULL;
    }

    am_shared_ptr_t *playback_asset = ssc_playback_asset_response_get_asset(asset_resp.obj);
    if (playback_asset == NULL || playback_asset->obj == NULL) {
        fprintf(stderr, "[WARN] Valid asset response for adamId %lu contains null asset pointer.\n", adam_id);
        return NULL;
    }

    am_string_t m3u8_url_str;
    ssc_playback_asset_get_url(&m3u8_url_str, playback_asset->obj);

    const char *m3u8_url = am_string_data(&m3u8_url_str);
    if (m3u8_url == NULL || strlen(m3u8_url) == 0) {
        fprintf(stderr, "[WARN] Playback asset for adamId %lu has an empty URL.\n", adam_id);
        return NULL;
    }

    return strdup(m3u8_url);
}

void handlers_handle_m3u8_connection(int conn_fd, app_context_t *context) {
    while (true) {
        uint8_t adam_size;
        if (!read_full(conn_fd, &adam_size, sizeof(adam_size)) || adam_size == 0) break;

        char adam_id_str[adam_size + 1];
        if (!read_full(conn_fd, adam_id_str, adam_size)) break;
        adam_id_str[adam_size] = '\0';

        char *end_ptr;
        unsigned long adam_id = strtoul(adam_id_str, &end_ptr, 10);
        if (*end_ptr != '\0') {
            fprintf(stderr, "[ERROR] Invalid Adam ID received: %s\n", adam_id_str);
            break;
        }

        char *m3u8_url = handlers_get_m3u8_url(context, adam_id);
        if (m3u8_url) {
            fprintf(stderr, "[INFO] M3U8 for adamId %lu: %s\n", adam_id, m3u8_url);
            char *response = NULL;
            if (asprintf(&response, "%s\n", m3u8_url) == -1) response = NULL;
            if (response) {
                write_full(conn_fd, response, strlen(response));
                free(response);
            }
            free(m3u8_url);
        } else {
            fprintf(stderr, "[WARN] Failed to get M3U8 for adamId %lu.\n", adam_id);
            write_full(conn_fd, "\n", 1);
        }
    }
}
