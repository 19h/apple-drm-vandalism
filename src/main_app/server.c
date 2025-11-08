#define _GNU_SOURCE
#include "server.h"
#include "handlers.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

// --- Type Definitions ---

// A function pointer type for the connection handler.
typedef void (*connection_handler_fn)(int, app_context_t *);

// Arguments for the connection worker thread.
typedef struct {
    int conn_fd;
    app_context_t *context;
    connection_handler_fn handler;
} connection_worker_args_t;

// Arguments for the main server thread (for M3U8 service).
typedef struct {
    app_context_t *context;
} server_thread_args_t;


// --- Static Helper Functions ---

/**
 * @brief Creates, binds, and listens on a new TCP socket.
 * @return The listening file descriptor on success, -1 on failure.
 */
static int create_listening_socket(const char *host, int port) {
    int listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    if (listen_fd == -1) {
        perror("socket");
        return -1;
    }

    const int optval = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
        perror("setsockopt(SO_REUSEPORT)");
        close(listen_fd);
        return -1;
    }

    struct sockaddr_in serv_addr = {.sin_family = AF_INET};
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(listen_fd);
        return -1;
    }

    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind");
        close(listen_fd);
        return -1;
    }

    if (listen(listen_fd, SOMAXCONN) == -1) {
        perror("listen");
        close(listen_fd);
        return -1;
    }

    fprintf(stderr, "[INFO] Server listening on %s:%d\n", host, port);
    return listen_fd;
}

/**
 * @brief The start routine for a thread that handles a single client connection.
 */
static void *connection_worker(void *arg) {
    connection_worker_args_t *args = (connection_worker_args_t *)arg;

    // Call the appropriate handler for this service.
    args->handler(args->conn_fd, args->context);

    // Cleanup.
    close(args->conn_fd);
    free(args);
    return NULL;
}

/**
 * @brief A generic server loop that accepts connections and spawns worker threads.
 */
static int server_loop(int listen_fd, app_context_t *context, connection_handler_fn handler) {
    while (true) {
        int conn_fd = accept4(listen_fd, NULL, NULL, SOCK_CLOEXEC);
        if (conn_fd == -1) {
            // EPROTO and other connection-aborted errors are recoverable.
            if (errno == EPROTO || errno == ENOPROTOOPT || errno == ECONNABORTED) {
                continue;
            }
            perror("accept4");
            return EXIT_FAILURE;
        }

        connection_worker_args_t *args = malloc(sizeof(connection_worker_args_t));
        if (!args) {
            perror("malloc for thread args failed");
            close(conn_fd);
            continue;
        }
        args->conn_fd = conn_fd;
        args->context = context;
        args->handler = handler;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, connection_worker, args) != 0) {
            perror("pthread_create");
            free(args);
            close(conn_fd);
        } else {
            // Detach the thread so its resources are automatically cleaned up on exit.
            pthread_detach(thread_id);
        }
    }
    return EXIT_SUCCESS; // Should not be reached
}

/**
 * @brief The start routine for the M3U8 server thread.
 */
static void *m3u8_server_thread_start(void *arg) {
    server_thread_args_t *args = (server_thread_args_t *)arg;
    const app_config_t *config = args->context->config;

    int listen_fd = create_listening_socket(config->host, config->m3u8_port);
    if (listen_fd == -1) {
        fprintf(stderr, "[ERROR] Failed to start M3U8 service.\n");
        return NULL;
    }

    server_loop(listen_fd, args->context, handlers_handle_m3u8_connection);

    free(args); // Args are copied, so free the heap allocation.
    close(listen_fd);
    return NULL;
}


// --- Public API Implementation ---

int server_run(app_context_t *context) {
    // Start the M3U8 service in a background thread.
    pthread_t m3u8_thread_id;
    server_thread_args_t *m3u8_args = malloc(sizeof(server_thread_args_t));
    if (!m3u8_args) {
        perror("malloc for m3u8 thread args failed");
        return EXIT_FAILURE;
    }
    m3u8_args->context = context;

    if (pthread_create(&m3u8_thread_id, NULL, m3u8_server_thread_start, m3u8_args) != 0) {
        perror("Failed to create M3U8 server thread");
        free(m3u8_args);
        return EXIT_FAILURE;
    }
    pthread_detach(m3u8_thread_id);

    // Start the primary decryption service on the main thread.
    const app_config_t *config = context->config;
    int decrypt_listen_fd = create_listening_socket(config->host, config->decrypt_port);
    if (decrypt_listen_fd == -1) {
        fprintf(stderr, "[ERROR] Failed to start decryption service.\n");
        // In a real app, we would signal the m3u8 thread to stop here.
        return EXIT_FAILURE;
    }

    // This call will block until an unrecoverable error occurs.
    int status = server_loop(decrypt_listen_fd, context, handlers_handle_decrypt_connection);

    close(decrypt_listen_fd);
    return status;
}
