#define DEBUG
#include "utils.h"
#include "common.h"

void usage(char *name) {
    fprintf(stderr, "USAGE: %s port \n", name);
}

void server_work(int socket_fd) {
    while(1) {
    }
}

int parse_arguments(char **argv, int argc, uint16_t *port) {
    if(argc != 2) {
        return 0;
    }
    *port = atoi(argv[1]);
    if(!is_valid_port(*port)) {
        return 0;
	}
    return 1;
}

int main(int argc, char **argv) {
    int socket_fd;
    uint16_t port;
    if(!parse_arguments(argv, argc, &port)) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

	socket_fd = bind_inet_socket(port, SOCK_STREAM);
    LOG_DEBUG("Started listening on port %d", port);
    server_work(socket_fd);
    safe_close(socket_fd);
    return EXIT_SUCCESS;
}
