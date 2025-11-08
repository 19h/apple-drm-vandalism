#pragma once

#include <stdbool.h>

/**
 * @brief A structure to hold all application configuration settings.
 *
 * This struct is populated at startup from command-line arguments and is
 * treated as read-only for the lifetime of the application.
 */
typedef struct app_config {
    // Network settings
    const char *host;
    int decrypt_port;
    int m3u8_port;
    const char *proxy;

    // Authentication settings
    char *username;
    char *password; // Mutable to allow appending 2FA code
    bool login_given;
    bool code_from_file;
} app_config_t;

/**
 * @brief Parses command-line arguments and creates a configuration object.
 *
 * This function encapsulates the call to the gengetopt parser, populates a new
 * app_config_t struct with the results, and performs any necessary
 * post-processing (e.g., splitting the login string).
 *
 * @param argc The argument count from main().
 * @param argv The argument vector from main().
 * @return A pointer to a newly allocated app_config_t object on success,
 *         or NULL on failure (e.g., parsing error).
 */
app_config_t *config_create(int argc, char *argv[]);

/**
 * @brief Frees all resources associated with a configuration object.
 *
 * @param config A pointer to the app_config_t object to be destroyed.
 *               If NULL, the function does nothing.
 */
void config_destroy(app_config_t *config);
