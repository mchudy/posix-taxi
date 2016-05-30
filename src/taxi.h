#ifndef TAXI_H
#define TAXI_H

#include <stdlib.h>
#include "common.h"

#define START_MONEY 100
#define COLLISION_COST 50
#define ORDER_FINISHED_MONEY 20
#define MIN_START_DISTANCE 3

taxi* taxi_create(int id, taxi **taxis, pthread_mutex_t *mutex, unsigned *seed);

/* Returns 0 if game over, 1 otherwise */
int taxi_move(taxi *t, taxi **taxis, pthread_mutex_t *mutex, order **orders,
              pthread_mutex_t **order_mutexes);
              
void taxi_change_direction(taxi *t, direction dir);

void taxi_remove(taxi *t, taxi **taxis, pthread_mutex_t *mutex);

/* Helper functions */
void taxi_init(taxi *new_taxi, int id, position pos, unsigned *seed);
void taxi_make_random_turn(taxi *t);
void taxi_update_direction(taxi *t);
void taxi_handle_city_edges(taxi *t);
position taxi_get_next_position(position pos, direction dir);
void taxi_try_take_order(taxi *t, order **orders, pthread_mutex_t **order_mutexes);
void taxi_try_finish_order(taxi *t, order **orders, pthread_mutex_t **order_mutexes);
int taxi_get_available_positions(taxi **taxis, position *available);
taxi* taxi_get(taxi **taxis, int x, int y);
int taxi_is_position_available(taxi **taxis, int x, int y);

#endif