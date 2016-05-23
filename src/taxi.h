#ifndef TAXI_H
#define TAXI_H

#include <stdlib.h>
#include "common.h"

#define START_MONEY 100
#define COLLISION_COST 50
#define ORDER_FINISHED_MONEY 20

int taxi_create();
void taxi_move(taxi taxi);
void taxi_change_direction(taxi *taxi, direction dir);

#endif