#define DEBUG
#include <math.h>
#include "map.h"
#include "taxi.h"

#define BUFFER_SIZE 5000

typedef struct thread_data {
    int socket_fd;
} thread_data;

void usage(char *name) {
    fprintf(stderr, "USAGE: %s port \n", name);
}

void send_map(thread_data *data) {
    int i;
    char buf[BUFFER_SIZE];
    char *map;
    map = map_generate();
    pthread_t tid = pthread_self();
    LOG_DEBUG("[TID=%ld] Sending map", tid);
    ssize_t map_length = strlen(map);
    ssize_t bytes_sent = 0;
    for(i = 0; i < ceil((float)map_length / BUFFER_SIZE); i++) {
        ssize_t bytes_to_send = BUFFER_SIZE;
        if(bytes_sent + BUFFER_SIZE > map_length) {
            bytes_to_send = map_length - bytes_sent;
        }
        strncpy(buf, map + bytes_sent, bytes_to_send);
        if(bulk_write(data->socket_fd, buf, bytes_to_send) < 0) {
            FORCE_EXIT("write");
        }
        bytes_sent += bytes_to_send;
    }
    free(map);
}

void* handle_client(void *data) {
    thread_data *tdata = (thread_data*) data;
    int taxi_id = taxi_create();
    pthread_t tid = pthread_self();
    LOG_DEBUG("[TID=%ld] New client connected", tid);
    if(taxi_id < 0) {
        LOG_DEBUG("[TID=%ld] Not enough space on the map", tid);
    }
    while(1) {
        msleep(TAXI_STREET_TIME, 0);
        send_map(tdata);
    }
    free(tdata);
    safe_close(data->socket_fd);
    return 0;
}

void server_work(int server_socket) {
    int client_socket;
    thread_data *data;
    while(1) {
        pthread_t client_tid;
        pthread_attr_t attr;
        data = safe_malloc(sizeof(thread_data));
        client_socket = accept_client(server_socket);
        if(client_socket < 0) {
            FORCE_EXIT("accept");
        }
        data->socket_fd = client_socket;
        if(pthread_attr_init(&attr) != 0) 
            LOG_ERROR("pthread_attr_init"); //TODO
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if(pthread_create(&client_tid, &attr, handle_client, data) != 0)
            LOG_ERROR("pthread_create"); //TODO
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