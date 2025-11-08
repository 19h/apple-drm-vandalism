#pragma once

#include "context.h"

/**
 * @brief Starts and runs the application's network services.
 *
 * This function is the main entry point for the server's execution loop.
 * It sets up and runs two services in parallel:
 *   1. The M3U8 service, which runs in a background thread.
 *   2. The decryption service, which runs on the calling (main) thread.
 *
 * This function will block until the primary decryption service terminates
 * due to an unrecoverable error.
 *
 * @param context The fully initialized application context, containing the
 *                configuration (ports, host) and state required by the
 *                request handlers.
 * @return An exit code, typically EXIT_SUCCESS or EXIT_FAILURE.
 */
int server_run(app_context_t *context);
