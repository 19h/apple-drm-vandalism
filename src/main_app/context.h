#pragma once

#include "apple_music_bridge.h"
#include "config.h"
#include <pthread.h>

// The size of the opaque buffer to hold the SVPlaybackLeaseManager C++ object.
// This value is taken from the original implementation.
#define LEASE_MANAGER_SIZE 16

/**
 * @brief A structure to hold the entire runtime state of the application.
 *
 * This object encapsulates all live handles, session objects, and shared state,
 * eliminating the need for global variables. It is created once at startup
 * and passed to functions that require access to the application's state.
 */
typedef struct app_context {
    const app_config_t *config; // Read-only application configuration

    // --- Apple Music Library Handles ---
    am_shared_ptr_t request_context;
    am_shared_ptr_t presentation_interface;
    uint8_t lease_manager[LEASE_MANAGER_SIZE]; // Opaque storage for C++ object

    // --- DRM State ---
    void *foothill_instance;      // Handle to the SVFootHillSessionCtrl singleton
    void *preshare_kd_context;    // Cached key decryption context for preshared keys
    pthread_mutex_t preshare_mutex; // Mutex to protect preshare_kd_context access
} app_context_t;

/**
 * @brief Initializes the application context and the Apple Music libraries.
 *
 * This function performs all one-time setup, including:
 * - Setting up the device GUID and other identifiers.
 * - Creating and configuring the main RequestContext.
 * - Performing user login if credentials are provided.
 * - Initializing the playback lease manager.
 * - Getting a handle to the DRM session controller.
 *
 * @param config A pointer to the application's configuration settings.
 * @return A pointer to a newly allocated and initialized app_context_t object,
 *         or NULL on failure.
 */
app_context_t *context_create(const app_config_t *config);

/**
 * @brief Frees all resources associated with the application context.
 *
 * @param context A pointer to the app_context_t object to be destroyed.
 *                If NULL, the function does nothing.
 */
void context_destroy(app_context_t *context);
