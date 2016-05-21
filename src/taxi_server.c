#define DEBUG
#include "utils.h"
#include "common.h"
#include "map.h"

#define BUFFER_SIZE 100

void usage(char *name) {
    fprintf(stderr, "USAGE: %s port \n", name);
}

void* handle_client(void *data) {
    int socket_fd = (int)data;
    pthread_t tid = pthread_self();
    LOG_DEBUG("[TID=%ld] New client connected", tid);
    int status;
    char buf[BUFFER_SIZE] = "map\n";
    while(1) {
        msleep(TAXI_STREET_TIME, 0);
        LOG_DEBUG("[TID=%ld] Sending map", tid);
        if(bulk_write(socket_fd, buf, BUFFER_SIZE) < 0){
			FORCE_EXIT("write");
		}
    }
    return 0;
}

void server_work(int server_socket) {
    int client_socket;
    while(1) {
        client_socket = accept_client(server_socket);
        if(client_socket < 0) {
            FORCE_EXIT("accept");
        }
        pthread_t client_tid;
        pthread_attr_t attr;
        if(pthread_attr_init(&attr)!=0)
            LOG_ERROR("pthread_attr_init");
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if(pthread_create(&client_tid, &attr, handle_client, (void*)client_socket) != 0)
            LOG_ERROR("pthread_create");
        pthread_attr_destroy(&attr);
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
