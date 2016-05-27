#ifndef TAXI_H
#define TAXI_H

#include <stdlib.h>
#include "common.h"

#define START_MONEY 50
#define COLLISION_COST 50
#define ORDER_FINISHED_MONEY 20
#define MIN_START_DISTANCE 3

taxi* taxi_create(int id, taxi **taxis, pthread_mutex_t *mutex);
int taxi_move(taxi *t, taxi **taxis, pthread_mutex_t *mutex);
void taxi_change_direction(taxi *taxi, direction dir);
void taxi_remove(taxi *t, taxi **taxis, pthread_mutex_t *mutex);

int taxi_get_available_positions(taxi **taxis,
                                position *available);
taxi* taxi_get(taxi **taxis, int x, int y);
int taxi_is_position_available(taxi **taxis, int x, int y);

direction get_random_direction(position pos, unsigned *seed);

#endif