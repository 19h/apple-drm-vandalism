#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h> // For strdup()

// Global variable to hold the child process ID.
// Must be volatile sig_atomic_t for safe access from a signal handler.
static volatile sig_atomic_t g_child_pid = -1;

/**
 * @brief Signal handler to terminate the child process.
 *
 * @param signum The signal number received.
 */
static void signal_handler(int signum) {
    (void)signum; // Unused parameter
    if (g_child_pid > 0) {
        kill(g_child_pid, SIGKILL);
    }
    // The process will terminate after the handler returns.
}

/**
 * @brief Creates a directory path, including all parent directories.
 *
 * @param path The full directory path to create.
 * @param mode The permissions for the new directories.
 * @return 0 on success, -1 on failure.
 */
static int mkdir_p(const char *path, mode_t mode) {
    char *p = NULL;
    char *temp_path = strdup(path);
    if (temp_path == NULL) {
        perror("strdup");
        return -1;
    }

    // Iterate through the path, creating directories as needed.
    for (p = temp_path + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(temp_path, mode) == -1 && errno != EEXIST) {
                perror("mkdir");
                free(temp_path);
                return -1;
            }
            *p = '/';
        }
    }

    if (mkdir(temp_path, mode) == -1 && errno != EEXIST) {
        perror("mkdir");
        free(temp_path);
        return -1;
    }

    free(temp_path);
    return 0;
}

int main(int argc, char *argv[], char *envp[]) {
    (void)argc;

    // 1. Set up signal handling to catch termination signals.
    struct sigaction sa = {0};
    sa.sa_handler = signal_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    // 2. Change directory to the intended chroot root.
    if (chdir("./rootfs") != 0) {
        perror("chdir to ./rootfs failed");
        return EXIT_FAILURE;
    }

    // 3. Confine the process to the new root directory.
    if (chroot("./") != 0) {
        perror("chroot failed");
        return EXIT_FAILURE;
    }

    // 4. Create essential device nodes inside the chroot.
    if (mknod("/dev/urandom", S_IFCHR | 0666, makedev(1, 9)) == -1 && errno != EEXIST) {
        perror("mknod for /dev/urandom failed");
        return EXIT_FAILURE;
    }

    // 5. Ensure the main application binary is executable.
    if (chmod("/system/bin/main_app", 0755) == -1) {
        perror("chmod for /system/bin/main_app failed");
        return EXIT_FAILURE;
    }

    // 6. Fork the process to create a child that will run the main app.
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return EXIT_FAILURE;
    }

    if (pid > 0) {
        // --- Parent Process ---
        g_child_pid = pid;
        // Wait for the child process to terminate.
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE;
    } else {
        // --- Child Process ---
        // Create necessary data directories inside the chroot.
        const char *data_dir = "/data/data/com.apple.android.music/files/mpl_db";
        if (mkdir_p(data_dir, 0777) == -1) {
            fprintf(stderr, "Failed to create directory: %s\n", data_dir);
            return EXIT_FAILURE;
        }

        // Replace the child process with the main application.
        // The new process inherits the chroot environment.
        execve("/system/bin/main_app", argv, envp);

        // execve only returns on error.
        perror("execve for /system/bin/main_app failed");
        return EXIT_FAILURE;
    }
}
