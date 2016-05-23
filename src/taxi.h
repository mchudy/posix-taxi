#ifndef TAXI_H
#define TAXI_H

#include <stdlib.h>
#include "common.h"

#define START_MONEY 100
#define COLLISION_COST 50
#define ORDER_FINISHED_MONEY 20
#define MIN_START_DISTANCE 3

taxi* taxi_create(int id, taxi **taxis);
void taxi_move(taxi taxi);
void taxi_change_direction(taxi *taxi, direction dir);
int taxi_get_available_positions(taxi **taxis,
                                position *available);
taxi* taxi_get(taxi **taxis, int x, int y);
int taxi_is_position_available(taxi **taxis, int x, int y);

#endif