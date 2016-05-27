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
    new_taxi->collision = 0;
    taxis[pos.x * ALLEYS_COUNT + pos.y] = new_taxi;
    if(pthread_mutex_unlock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_lock");
    }
    return new_taxi;
}

void taxi_remove(taxi *t) {
    free(t);
}

void make_random_turn(taxi *t) {
    unsigned seed = time(NULL);
    int n = rand_r(&seed) % 2;
    t->next_direction = (n == 0 ? LEFT : RIGHT);
    LOG_DEBUG("Random dir %d", t->next_direction);
}

void handle_city_edges(taxi *t) {
    if(t->next_direction != -1) return;
    if(t->current_direction == LEFT && t->position.y == 0) {
        if(t->position.x == 0) {
            t->next_direction = LEFT;
        } else if (t->position.x == STREETS_COUNT - 1) {
            t->next_direction = RIGHT;
        } else {
            make_random_turn(t);
        }
    } else if (t->current_direction == RIGHT && t->position.y == ALLEYS_COUNT - 1) {
        if(t->position.x == 0) {
            t->next_direction = RIGHT;
        } else if (t->position.x == STREETS_COUNT - 1) {
            t->next_direction = LEFT;
        } else {
            make_random_turn(t);
        }
    } else if (t->current_direction == UP && t->position.x == 0) {
        if(t->position.y == 0) {
            t->next_direction = RIGHT;
        } else if (t->position.y == ALLEYS_COUNT - 1) {
            t->next_direction = LEFT;
        } else {
            make_random_turn(t);
        }
    } else if (t->current_direction == DOWN && t->position.x == STREETS_COUNT - 1) {
        if(t->position.y == 0) {
            t->next_direction = LEFT;
        } else if (t->position.y == ALLEYS_COUNT - 1) {
            t->next_direction = RIGHT;
        } else {
            make_random_turn(t);
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

/* Returns 0 if game over, 1 otherwise */
int taxi_move(taxi *t, taxi **taxis, pthread_mutex_t *mutex) {
    if(pthread_mutex_lock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_lock");
    }
    if(t->collision) {
        t->collision = 0;
        t->money -= COLLISION_COST;
        t->current_direction = REVERSE_DIRECTION(t->current_direction);
    } else {
        position next_position = t->position;
        handle_city_edges(t);
        update_direction(t);
        switch(t->current_direction) {
            case LEFT:
                next_position.y -= 1;
                break;
            case RIGHT:
                next_position.y += 1;
                break;
            case UP:
                next_position.x -= 1;
                break;
            case DOWN:
                next_position.x += 1;
                break;
        }
        if(taxis[next_position.x * ALLEYS_COUNT + next_position.y] != NULL) {
            LOG_DEBUG("Collision");
            taxis[next_position.x * ALLEYS_COUNT + next_position.y]->collision = 1;
            t->current_direction = REVERSE_DIRECTION(t->current_direction);
            t->money -= COLLISION_COST;
        } else {
            taxis[t->position.x * ALLEYS_COUNT + t->position.y] = NULL;
            t->position = next_position;
            LOG_DEBUG("Taxi %d move to (%d,%d)", t->id, t->position.x, t->position.y);
            taxis[t->position.x * ALLEYS_COUNT + t->position.y] = t;
        }
    }
    t->next_direction = -1;
    if(pthread_mutex_unlock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_lock");
    }
    return 1;
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