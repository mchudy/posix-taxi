#include <math.h>
#include "taxi.h"
#include "utils.h"

int taxi_create() {
    taxi new_taxi;
    
    return 0;
}

void taxi_move(taxi taxi) {
    
}

void taxi_change_direction(taxi *taxi, direction dir) {
    //TODO synchronization
    //taxi->next_direction = dir;
}

int taxi_get_available_positions(taxi taxis[STREETS_COUNT][ALLEYS_COUNT], 
                                int available[STREETS_COUNT][ALLEYS_COUNT]) {
    int i, j;
    for(i = 0; i < STREETS_COUNT; i++) {
        for(j = 0; j < ALLEYS_COUNT; j++) {
            available[i][j] = 0;
            if(taxis[i][j].id != - 1) continue;
            
        }
    }
    return 0;
}

/**
 *  Returns Chebychev distance between two points
 */
int taxi_get_distance(position p1, position p2) {
    return MAX(abs(p2.x - p1.x), abs(p2.y - p1.y));
}