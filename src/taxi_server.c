#include <ctype.h>
#include "map.h"
#include "taxi.h"
#include "order.h"

#define BUFFER_SIZE 10

typedef struct thread_data {
    int socket_fd;
    taxi *taxi;
    taxi **taxis;
    order **orders;

    /* Mutex for synchronizing taxis' moves */
    pthread_mutex_t *taxis_mutex;
    /* Array of mutexes for synchronizing the orders */
    pthread_mutex_t **order_mutexes;
} thread_data;

volatile sig_atomic_t work = 1;

void usage(char *name) {
    fprintf(stderr, "USAGE: %s port \n", name);
}

void sigint_handler(int sig) {
    work = 0;
}

void init_taxis(taxi *taxis[STREETS_COUNT][ALLEYS_COUNT]) {
    int i, j;
    for(i = 0; i < STREETS_COUNT; i++) {
        for (j = 0; j < ALLEYS_COUNT; j++) {
            taxis[i][j] = NULL;
        }
    }
}

void send_map(thread_data *data) {
    char *map;
    map = map_generate(data->taxis, data->taxi, data->taxis_mutex, 
            data->orders, data->order_mutexes);
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

int read_and_change_direction(int socket_fd, thread_data *tdata) {
    char buf[BUFFER_SIZE];
    int n = read(socket_fd, buf, BUFFER_SIZE);
    if (n < 0) {
        FORCE_EXIT("read");
    } else if (n == 0) { // client disconnected
        if(tdata->taxi->current_order_id != -1) {
            order_cancel(tdata->taxi->current_order_id, tdata->orders, tdata->order_mutexes);
        }
        taxi_remove(tdata->taxi, tdata->taxis, tdata->taxis_mutex);
        return 0;
    } else {
        buf[n] = '\0';
        direction dir = extract_direction(buf, n);
        if(dir != -1) {
            taxi_change_direction(tdata->taxi, dir);
        }
    }
    return 1;
}

void* handle_client(void *data) {
    thread_data *tdata = (thread_data*) data;
    int socket_fd = tdata->socket_fd;
    pthread_t tid = pthread_self();
    printf("[TID=%ld] New taxi with id %d\n", tid, tdata->taxi->id);
    fd_set base_rfds, rfds;
    FD_ZERO(&base_rfds);
    FD_SET(socket_fd, &base_rfds);
    struct timespec timeout = {TAXI_STREET_TIME, 0}, elapsed = {TAXI_STREET_TIME + 1, 0}, current_time, new_time;
    int status;
    while(work) {
        rfds = base_rfds;
        if(!timespec_subtract(&timeout, &elapsed, &new_time)) {
            timeout = new_time;
        } else {
            timeout.tv_sec = TAXI_STREET_TIME;
            timeout.tv_nsec = 0;
            if(!taxi_move(tdata->taxi, tdata->taxis, tdata->taxis_mutex, tdata->orders, tdata->order_mutexes)) {
                printf("Game over for taxi %d\n", tdata->taxi->id);
                if(bulk_write(tdata->socket_fd, "GAME OVER\n", 10) < 10) {
                    FORCE_EXIT("write");
                }
                if(tdata->taxi->current_order_id != -1) {
                    order_cancel(tdata->taxi->current_order_id, tdata->orders, tdata->order_mutexes);
                }
                taxi_remove(tdata->taxi, tdata->taxis, tdata->taxis_mutex);
                break;
            }
            send_map(tdata);        
        }
        if(clock_gettime(CLOCK_REALTIME, &current_time) != 0) {
            FORCE_EXIT("clock_gettime");
        }
        status = pselect(socket_fd + 1, &rfds, NULL, NULL, &timeout, NULL);
        if(status > 0) {
            if(!read_and_change_direction(socket_fd, tdata)) {
                break;
            }
        } else if(status == -1) {
            if (EINTR == errno) continue;
            FORCE_EXIT("pselect");
        }
        if(clock_gettime(CLOCK_REALTIME, &new_time) != 0){
            FORCE_EXIT("clock_gettime");
        }
        timespec_subtract(&new_time, &current_time, &elapsed);
    }
    safe_close(socket_fd);
    if(!work) free(tdata->taxi);
    free(tdata);
    return NULL;
}

void* generate_orders(void* data) {
    order_thread_data *tdata = (order_thread_data*) data;
    order **orders = tdata->orders;
    int i;
    unsigned seed = pthread_self();
    while(work) {
        lock_orders(tdata->order_mutexes);
        for(i = 0; i < MAX_ORDERS; i++) {
            if(orders[i] == NULL) {
                orders[i] = get_random_order(orders, i, &seed);
                printf("Created new order (%d,%d) -> (%d, %d)\n", orders[i]->start.x, orders[i]->start.y, orders[i]->end.x, orders[i]->end.y);
            }
        }
        unlock_orders(tdata->order_mutexes);
        msleep(ORDER_GENERATION_INTERVAL, 0);
    }
    free(tdata);
    return NULL;
}

void server_work(int server_socket) {
    int client_socket;
    int current_taxi_id = 0;
    unsigned seed = pthread_self();
    taxi *taxis[STREETS_COUNT][ALLEYS_COUNT];
    order *orders[MAX_ORDERS];
    pthread_mutex_t *order_mutexes[MAX_ORDERS];
    pthread_mutex_t taxis_mutex = PTHREAD_MUTEX_INITIALIZER;
    thread_data *data;
    order_thread_data *order_data = safe_malloc(sizeof(order_thread_data));
    order_data->orders = orders;
    order_data->order_mutexes = &order_mutexes[0];

    init_orders(orders, order_mutexes);
    init_taxis(taxis);
    create_detached_thread(order_data, generate_orders);
    while(work) {
        client_socket = accept_client(server_socket);
        if(client_socket < 0) continue;
        taxi *new_taxi = taxi_create(current_taxi_id, &taxis[0][0], &taxis_mutex, &seed);
        if(new_taxi == NULL) {
            printf("Not enough space on the map, disconnecting client\n");
            safe_close(client_socket);
            continue;
        }
        data = safe_malloc(sizeof(thread_data));
        data->socket_fd = client_socket;
        data->taxi = new_taxi;
        data->taxis = &taxis[0][0];
        data->orders = &orders[0];
        data->order_mutexes = &order_mutexes[0];
        data->taxis_mutex = &taxis_mutex;
        create_detached_thread(data, handle_client);
        current_taxi_id++;
    }
    cleanup_orders(orders, order_mutexes);
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
    server_work(socket_fd);
    safe_close(socket_fd);
    return EXIT_SUCCESS;
}