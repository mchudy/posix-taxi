#define DEBUG
#include <ctype.h>
#include "map.h"
#include "taxi.h"

#define BUFFER_SIZE 10

typedef struct thread_data {
    int socket_fd;
    taxi *taxi;
    taxi **taxis;
    order **orders;
    
    /* Mutex for synchronizing taxis' moves */
    pthread_mutex_t *taxis_mutex;
    pthread_mutex_t **order_mutexes;
} thread_data;

volatile sig_atomic_t work = 1;

void usage(char *name) {
    fprintf(stderr, "USAGE: %s port \n", name);
}

void sigint_handler(int sig) {
	work = 0;
}

void send_map(thread_data *data) {
    char *map;
    map = map_generate(data->taxis, data->taxi, data->taxis_mutex);
    pthread_t tid = pthread_self();
    LOG_DEBUG("[TID=%ld] Sending map", tid);
    if(bulk_write(data->socket_fd, map, strlen(map)) < strlen(map)) {
        FORCE_EXIT("write");
    }
    free(map);
}

direction extract_direction(char *s, int len) {
    while(len-- > 0) {
        char c = toupper(s[len]);
        if(c == 'P') {
            return RIGHT;
        } else if(c == 'L') {
            return LEFT;
        }
    }
    return -1;
}

void* handle_client(void *data) {
    thread_data *tdata = (thread_data*) data;
    int socket_fd = tdata->socket_fd;
    pthread_t tid = pthread_self();
    char buf[BUFFER_SIZE];
    LOG_DEBUG("[TID=%ld] New client connected", tid);
    LOG_DEBUG("[TID=%ld] New taxi with id %d", tid, tdata->taxi->id);
	fd_set base_rfds, rfds;
	FD_ZERO(&base_rfds);
	FD_SET(socket_fd, &base_rfds);
    struct timespec timeout;
    int status;
    while(work) {
        rfds = base_rfds;
        taxi_move(tdata->taxi, tdata->taxis, tdata->taxis_mutex);
        send_map(tdata);
        //TODO: calculate time left
        timeout.tv_sec = TAXI_STREET_TIME;
        timeout.tv_nsec = 0;
        LOG_DEBUG("before select");
        
        status = pselect(socket_fd + 1, &rfds, NULL, NULL, &timeout, NULL);
        if(status > 0) {
            int n = read(socket_fd, buf, BUFFER_SIZE);
            if (n < 0) {
                FORCE_EXIT("read");
            } else if (n == 0) { // client disconnected
                //taxi_remove();
                LOG_DEBUG("Client disconnected");
                break;
            } else {
                buf[n] = '\0';
                LOG_DEBUG("GOT %d bytes: %s", n, buf);
                direction dir = extract_direction(buf, n);
                if(dir != -1) {
                    taxi_change_direction(tdata->taxi, extract_direction(buf, n));
                    LOG_DEBUG("Changing direction to %d", extract_direction(buf, n));
                }
            }
		} else if(status == -1) {
	    	if (EINTR == errno) continue;
			FORCE_EXIT("pselect");
		}
    }
    safe_close(socket_fd);
    free(tdata->taxi);
    free(tdata);
    return NULL;
}

void server_work(int server_socket) {
    int client_socket;
    int current_taxi_id = 0;
    int i, j;
    taxi *taxis[STREETS_COUNT][ALLEYS_COUNT];
    order *orders[MAX_ORDERS];
    for(i = 0; i < STREETS_COUNT; i++) {
        for (j = 0; j < ALLEYS_COUNT; j++) {
            taxis[i][j] = NULL;
        }
    }
	pthread_mutex_t taxis_mutex = PTHREAD_MUTEX_INITIALIZER;
    thread_data *data;
    while(work) {
        client_socket = accept_client(server_socket);
        if(client_socket < 0) continue;
        taxi *new_taxi = taxi_create(current_taxi_id, &taxis[0][0], &taxis_mutex);
        if(new_taxi == NULL) {
            LOG_DEBUG("Not enough space on the map");
            safe_close(client_socket);
            continue;
        }
        data = safe_malloc(sizeof(thread_data));
        data->socket_fd = client_socket;
        data->taxi = new_taxi;
        data->taxis = &taxis[0][0];
        LOG_DEBUG("%d", taxis[0][0] == NULL);
        data->taxis_mutex = &taxis_mutex;
        create_detached_thread(data, handle_client);
        current_taxi_id++;
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
    set_handler(SIG_IGN, SIGPIPE);
    set_handler(sigint_handler, SIGINT);
    socket_fd = bind_inet_socket(port, SOCK_STREAM);
    set_nonblock(socket_fd);
    LOG_DEBUG("Started listening on port %d", port);
    server_work(socket_fd);
    safe_close(socket_fd);
    return EXIT_SUCCESS;
}