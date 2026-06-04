#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "daemon.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

int create_daemon() {

    switch(fork()) {
        case -1:
            return EAGAIN; // Fork failed
        case 0:
            break; // Child process continues
        default:
            exit(0); // Parent process exits
    }

    // Child process continues
    if (setsid() < 0) {
        return EAGAIN; // Failed to create a new session
    }

    // Change the working directory to the root directory
    if (chdir("/") < 0) {
        return EAGAIN; // Failed to change directory
    }

    switch(fork()) {
        case -1:
            return EAGAIN; // Fork failed
        case 0:
            break; // Second child process continues
        default:
            exit(0); // First child process exits
    }

    umask(0); // Reset file mode creation mask

    signal(SIGHUP, SIG_IGN); // Ignore SIGHUP signal

    // Close standard file descriptors
    
    int fd = open("/dev/null", O_RDWR);
    if (fd < 0) return EAGAIN;

    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    if (fd > 2) close(fd);


    return 0; // Daemon created successfully
}