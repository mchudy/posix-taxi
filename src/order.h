#ifndef ORDER_H
#define ORDER_H

#include <stdlib.h>
#include <pthread.h>
#include "common.h"
#include "taxi.h"
#include "utils.h"

#define ORDER_GENERATION_INTERVAL 10

typedef struct order_thread_data {
    order **orders;
    pthread_mutex_t **order_mutexes;
} order_thread_data;

order* get_random_order(order **orders, int id, unsigned *seed);
void init_orders(order *orders[MAX_ORDERS], pthread_mutex_t *order_mutexes[MAX_ORDERS]);
void cleanup_orders(order *orders[MAX_ORDERS], pthread_mutex_t *order_mutexes[MAX_ORDERS]);
void order_cancel(int id, order **orders, pthread_mutex_t **order_mutexes);

void lock_orders(pthread_mutex_t **order_mutexes);
void unlock_orders(pthread_mutex_t **order_mutexes);

#endif