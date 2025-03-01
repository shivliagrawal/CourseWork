#include "main.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#include "server.h"
#include "socket.h"

#define DEFAULT_PORT (4747)

/*
 * Print the usage for the program. Takes the name of the program as an
 * argument.
 */
void print_usage(char *name) {
    printf("USAGE: %s [-f|-t|-pNUM_THREADS] [-h] PORT_NO\n", name);
} /* print_usage() */

/*
 * Global variables for signal handling and acceptor instance
 */
volatile sig_atomic_t server_running = 1;
acceptor *acceptor_instance = NULL;

/*
 * Signal handler for SIGUSR1
 */
void sigusr1_handler(int signum) {
    server_running = 0;
    if (acceptor_instance && acceptor_instance->master_socket >= 0) {
        close(acceptor_instance->master_socket);
        acceptor_instance->master_socket = -1;
    }
}

/*
 * Signal handler for SIGCHLD
 */
void sigchld_handler(int signum) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        // Child reaped; no action needed
    }
    errno = saved_errno;
}

/*
 * Runs the webserver.
 */
int main(int argc, char **argv) {
    concurrency_mode mode = E_NO_CONCURRENCY;

    int port_no = 0;
    int num_threads = 0;

    int c = 0;
    while ((c = getopt(argc, argv, "hftp:")) != -1) {
        switch (c) {
        case 'h':
            print_usage(argv[0]);
            return 0;
        case 'f':
        case 't':
        case 'p':
            if (mode != E_NO_CONCURRENCY) {
                fputs("Multiple concurrency modes specified\n", stdout);
                print_usage(argv[0]);
                return -1;
            }
            mode = (concurrency_mode) c;
            if (mode == E_THREAD_POOL) {
                num_threads = atoi(optarg);
            }
            break;
        case '?':
            if (isprint(optopt)) {
                fprintf(stderr, "Unknown option: -%c\n", optopt);
            } else {
                fprintf(stderr, "Unknown option\n");
            }
            // Fall through
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    if (optind > argc) {
        fprintf(stderr, "Extra arguments were specified");
        print_usage(argv[0]);
        return 1;
    } else if (optind == argc) {
        // use default port
        port_no = DEFAULT_PORT;
    } else {
        port_no = atoi(argv[optind]);
    }

    printf("Mode: %d, Port: %d, Threads: %d\n", mode, port_no, num_threads);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Failed to register SIGUSR1 handler");
        return 1;
    }

    struct sigaction sa_chld;
    memset(&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = 0; // Do not set SA_RESTART
    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
      perror("Failed to register SIGCHLD handler");
      return 1;
    }

    acceptor_instance = create_socket_acceptor(port_no);

    if (acceptor_instance == NULL) {
        return 1;
    }

    // Start the server based on the mode
    switch (mode) {
    case E_FORK_PER_REQUEST:
        run_forking_server(acceptor_instance);
        break;
    case E_THREAD_PER_REQUEST:
        run_threaded_server(acceptor_instance);
        break;
    case E_THREAD_POOL:
        run_thread_pool_server(acceptor_instance, num_threads);
        break;
    default:
        run_linear_server(acceptor_instance);
        break;
    }

    if (acceptor_instance) {
        close_socket_acceptor(acceptor_instance);
        acceptor_instance = NULL;
    }

    return 0;
} /* main() */

