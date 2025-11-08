#include "config.h"
#include "context.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief The main entry point for the application.
 *
 * This function orchestrates the application's lifecycle:
 * 1. Initializes configuration from command-line arguments.
 * 2. Initializes the application context and third-party libraries.
 * 3. Runs the main server loop.
 * 4. Cleans up all resources upon termination.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line argument strings.
 * @return EXIT_SUCCESS on normal termination, EXIT_FAILURE on error.
 */
int main(int argc, char *argv[]) {
    app_config_t *config = NULL;
    app_context_t *context = NULL;
    int exit_status = EXIT_FAILURE;

    // 1. Create configuration from command-line arguments.
    config = config_create(argc, argv);
    if (config == NULL) {
        fprintf(stderr, "Failed to initialize application configuration.\n");
        // No resources to clean up yet, so we can exit directly.
        return EXIT_FAILURE;
    }

    // 2. Create the application context and initialize libraries.
    context = context_create(config);
    if (context == NULL) {
        fprintf(stderr, "Failed to initialize application context.\n");
        goto cleanup;
    }

    // 3. Run the server. This is the main blocking call.
    exit_status = server_run(context);

cleanup:
    // 4. Clean up all resources in reverse order of creation.
    fprintf(stderr, "Shutting down...\n");
    context_destroy(context);
    config_destroy(config);

    return exit_status;
}
