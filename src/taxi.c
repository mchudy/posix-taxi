#include <math.h>
#include "utils.h"
#include "taxi.h"

//TODO synchronization
taxi* taxi_create(int id, taxi **taxis, pthread_mutex_t *mutex) {
    position available[100];
    if(pthread_mutex_lock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_lock");
    }
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
    new_taxi->current_direction = get_random_direction(pos, &seed);
    new_taxi->next_direction = -1;
    new_taxi->position = pos;
    taxis[pos.x * ALLEYS_COUNT + pos.y] = new_taxi;
    if(pthread_mutex_unlock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_lock");
    }
    return new_taxi;
}

void taxi_remove(taxi *t) {
    free(t);
}

void handle_city_edges(taxi *t) {
    if(t->next_direction == -1){
        //TODO: add randomness
        if(t->current_direction == LEFT && t->position.y == 0) {
            t->next_direction = LEFT;
        } else if (t->current_direction == RIGHT && t->position.y == ALLEYS_COUNT - 1) {
            t->next_direction = LEFT;
        } else if (t->current_direction == UP && t->position.x == 0) {
            t->next_direction = LEFT;
        } else if (t->current_direction == DOWN && t->position.x == STREETS_COUNT - 1) {
            t->next_direction = LEFT;
        }
    }
}

void update_direction(taxi *t) {
    if(t->next_direction == LEFT) {
        t->current_direction = (t->current_direction - 1) % 4;
    } else if (t->next_direction == RIGHT) {
        t->current_direction = (t->current_direction + 1) % 4;
    }
}

void taxi_move(taxi *t, taxi **taxis, pthread_mutex_t *mutex) {
    //TODO: seperate mutex for operations only on one taxi?
    if(pthread_mutex_lock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_lock");
    }
    taxis[t->position.x * ALLEYS_COUNT + t->position.y] = NULL;
    handle_city_edges(t);
    update_direction(t);
    switch(t->current_direction) {
        case LEFT:
            t->position.y -= 1;
            break;
        case RIGHT:
            t->position.y += 1;
            break;
        case UP:
            t->position.x -= 1;
            break;
        case DOWN:
            LOG_DEBUG("GOING DONW");
            t->position.x += 1;
            break;
    }
    taxis[t->position.x * ALLEYS_COUNT + t->position.y] = t;
    t->next_direction = -1;
    if(pthread_mutex_unlock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_lock");
    }
}

void taxi_change_direction(taxi *t, direction dir) {
    //TODO synchronization
    if(dir == LEFT) {
        if((t->current_direction == UP && t->position.y == 0) || 
           (t->current_direction == RIGHT && t->position.x == 0) || 
           (t->current_direction == DOWN && t->position.y == ALLEYS_COUNT - 1) ||
           (t->current_direction == LEFT && t->position.x == STREETS_COUNT - 1)
        ) return;
    } else if (dir == RIGHT) {
         if((t->current_direction == DOWN && t->position.y == 0) || 
           (t->current_direction == LEFT && t->position.x == 0) || 
           (t->current_direction == UP && t->position.y == ALLEYS_COUNT - 1) ||
           (t->current_direction == RIGHT && t->position.x == STREETS_COUNT - 1)
        ) return;
    }
    t->next_direction = dir;
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

direction get_random_direction(position pos, unsigned *seed) {
    direction dir = rand_r(seed) % 4;
    return dir;
}