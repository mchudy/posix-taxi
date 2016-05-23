#include <math.h>
#include "utils.h"
#include "taxi.h"

//TODO synchronization
taxi* taxi_create(int id, taxi **taxis) {
    position available[100];
    int available_count = taxi_get_available_positions(taxis, available);
    LOG_DEBUG("%d places available", available_count);
    if(available_count == 0) {
        return NULL;
    }
    unsigned seed = time(NULL);
    position pos = available[rand_r(&seed) % available_count];
    taxi *new_taxi = safe_malloc(sizeof(taxi));
    new_taxi->id = id;
    new_taxi->money = START_MONEY;
    taxis[pos.x * ALLEYS_COUNT + pos.y] = new_taxi;
    return new_taxi;
}

void taxi_remove(taxi *t) {
    free(t);
}

void taxi_move(taxi t) {
    
}

void taxi_change_direction(taxi *taxi, direction dir) {
    //TODO synchronization
    //taxi->next_direction = dir;
}

int taxi_get_available_positions(taxi **taxis,
                                position *available) {
    int i, j;
    int available_count = 0;
    for(i = 0; i < STREETS_COUNT; i++) {
        for(j = 0; j < ALLEYS_COUNT; j++) {
            if(taxi_is_position_available(taxis, i, j)) {
                position pos = {i, j};
                available[available_count] = pos;
                available_count++;
            }
        }
    }
    return available_count;
}

taxi* taxi_get(taxi **taxis, int x, int y) {
    return taxis[x * ALLEYS_COUNT + y];
}

int taxi_is_position_available(taxi **taxis, int x, int y) {
    int i, j;
    for (i = -2; i <= 2; i++)
    {
        for(j = -2; j <= 2; j++) {
            if((x + i < 0) || (x + i > STREETS_COUNT - 1) || (y + j < 0) || (y + j > ALLEYS_COUNT - 1)) {
                continue;
            }
            if(taxi_get(taxis, x + i, j + y) != NULL) {
                return 0;
            }
        }
    }
    return 1;
}

