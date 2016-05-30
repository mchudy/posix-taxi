#include "order.h"

order* get_random_order(order **orders, int id, unsigned *seed) {
    order *new_order = safe_malloc(sizeof(order));
    new_order->available = 1;
    new_order->id = id;
    position start;
    position end;
    int i, repeated_start = 1;
    while(repeated_start) {
        repeated_start = 0;
        start.x = rand_r(seed) % STREETS_COUNT;
        start.y = rand_r(seed) % ALLEYS_COUNT;
        for(i = 0; i < MAX_ORDERS; i++) {
            if(orders[i] != NULL && orders[i]->available && position_equal(orders[i]->start, start)) {
                repeated_start = 1;
                continue;
            }
        }
    }
    end.x = rand_r(seed) % STREETS_COUNT;
    end.y = rand_r(seed) % ALLEYS_COUNT;
    while(position_equal(start, end)) {
        end.x = rand_r(seed) % STREETS_COUNT;
        end.y = rand_r(seed) % ALLEYS_COUNT;
    }
    new_order->start = start;
    new_order->end = end;
    return new_order;
}

void init_orders(order *orders[MAX_ORDERS], pthread_mutex_t *order_mutexes[MAX_ORDERS]) {
    int i;
    for(i = 0; i < MAX_ORDERS; i++) {
        orders[i] = NULL;
        order_mutexes[i] = safe_malloc(sizeof(pthread_mutex_t));
        if(pthread_mutex_init(order_mutexes[i], NULL) != 0) {
            FORCE_EXIT("pthread_mutex_init");   
        }
    }
}


void cleanup_orders(order *orders[MAX_ORDERS], pthread_mutex_t *order_mutexes[MAX_ORDERS]) {
    int i;
    for(i = 0; i < MAX_ORDERS; i++) {
        if(orders[i] != NULL) {
            free(orders[i]);
        }
        if(pthread_mutex_destroy(order_mutexes[i]) != 0) {
            FORCE_EXIT("pthread_mutex_init");   
        }
        free(order_mutexes[i]);
   }
}

void order_cancel(int id, order **orders, pthread_mutex_t **order_mutexes) {
    if(pthread_mutex_lock(order_mutexes[id]) != 0) {
        FORCE_EXIT("pthread_mutex_lock");
    }
    LOG_DEBUG("Cancelling order %d", id);
    free(orders[id]);
    orders[id] = NULL;
    if(pthread_mutex_unlock(order_mutexes[id]) != 0) {
        FORCE_EXIT("pthread_mutex_unlock");
    }
}

void lock_orders(pthread_mutex_t **order_mutexes) {
    int i;
    for(i = 0; i < MAX_ORDERS; i++) {
        if(pthread_mutex_lock(order_mutexes[i]) != 0) {
            FORCE_EXIT("pthread_mutex_lock");
        }
    }
}

void unlock_orders(pthread_mutex_t **order_mutexes) {
    int i;
    for(i = 0; i < MAX_ORDERS; i++) {
        if(pthread_mutex_unlock(order_mutexes[i]) != 0) {
            FORCE_EXIT("pthread_mutex_unlock");
        }
    }
}