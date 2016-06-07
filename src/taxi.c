#include <math.h>
#include "utils.h"
#include "taxi.h"

taxi* taxi_create(int id, taxi **taxis, pthread_mutex_t *mutex, unsigned *seed) {
    position available[STREETS_COUNT * ALLEYS_COUNT];
    if(pthread_mutex_lock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_lock");
    }
    int available_count = taxi_get_available_positions(taxis, available);
    if(available_count == 0) {
        return NULL;
    }
    position pos = available[rand_r(seed) % available_count];
    taxi *new_taxi = safe_malloc(sizeof(taxi));
    taxi_init(new_taxi, id, pos, seed);
    taxis[pos.x * ALLEYS_COUNT + pos.y] = new_taxi;
    if(pthread_mutex_unlock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_unlock");
    }
    return new_taxi;
}

/* Returns 0 if game over, 1 otherwise */
int taxi_move(taxi *t, taxi **taxis, pthread_mutex_t *mutex, order **orders,
              pthread_mutex_t **order_mutexes) {
    if(pthread_mutex_lock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_lock");
    }
    if(t->collision) {
        t->collision = 0;
        t->money -= COLLISION_COST;
        t->next_direction = -1;
    } else if (t->stay) {
        t->stay = 0;
    } else {
        taxi_handle_city_edges(t);
        taxi_update_direction(t);
        position next_position = taxi_get_next_position(t->position, t->current_direction);
        if(taxis[next_position.x * ALLEYS_COUNT + next_position.y] != NULL) {
            taxi *collision_taxi = taxis[next_position.x * ALLEYS_COUNT + next_position.y];
            collision_taxi->collision = 1;
            collision_taxi->current_direction  = REVERSE_DIRECTION(collision_taxi->current_direction);
            t->current_direction = REVERSE_DIRECTION(t->current_direction);
            t->money -= COLLISION_COST;
            printf("Collision: taxi %d with taxi %d\n", t-> id, collision_taxi->id);
        } else {
            taxis[t->position.x * ALLEYS_COUNT + t->position.y] = NULL;
            t->position = next_position;
            taxis[t->position.x * ALLEYS_COUNT + t->position.y] = t;
            LOG_DEBUG("Taxi %d move to (%d,%d)", t->id, t->position.x, t->position.y);
            if(t->current_order_id == -1) {
                // not handling any order, check if there is one available on the new position
                taxi_try_take_order(t, orders, order_mutexes);
            } else {
                // handling order, check if the endpoint has been achieved
                taxi_try_finish_order(t, orders, order_mutexes);   
            }
        }
        t->next_direction = -1;
    }
    if(pthread_mutex_unlock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_unlock");
    }
    return t->money > 0;
}

void taxi_change_direction(taxi *t, direction dir) {
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

void taxi_remove(taxi *t, taxi **taxis, pthread_mutex_t *mutex) {
    if(pthread_mutex_lock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_lock");
    }
    taxis[t->position.x * ALLEYS_COUNT + t->position.y] = NULL;
    if(pthread_mutex_unlock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_unlock");
    }
    free(t);
}

void taxi_init(taxi *new_taxi, int id, position pos, unsigned *seed) {
    new_taxi->id = id;
    new_taxi->money = START_MONEY;
    new_taxi->current_order_id = -1;
    new_taxi->current_direction = get_random_direction(pos, seed);
    new_taxi->next_direction = -1;
    new_taxi->position = pos;
    new_taxi->collision = 0;
    new_taxi->stay = 0;
}

void taxi_make_random_turn(taxi *t) {
    unsigned seed = time(NULL);
    int n = rand_r(&seed) % 2;
    t->next_direction = (n == 0 ? LEFT : RIGHT);
}

void taxi_update_direction(taxi *t) {
    if(t->next_direction == LEFT) {
        t->current_direction = (t->current_direction - 1) % 4;
    } else if (t->next_direction == RIGHT) {
        t->current_direction = (t->current_direction + 1) % 4;
    }
}

void taxi_handle_city_edges(taxi *t) {
    if(t->next_direction != -1) return;
    if(t->current_direction == LEFT && t->position.y == 0) {
        if(t->position.x == 0) {
            t->next_direction = LEFT;
        } else if (t->position.x == STREETS_COUNT - 1) {
            t->next_direction = RIGHT;
        } else {
            taxi_make_random_turn(t);
        }
    } else if (t->current_direction == RIGHT && t->position.y == ALLEYS_COUNT - 1) {
        if(t->position.x == 0) {
            t->next_direction = RIGHT;
        } else if (t->position.x == STREETS_COUNT - 1) {
            t->next_direction = LEFT;
        } else {
            taxi_make_random_turn(t);
        }
    } else if (t->current_direction == UP && t->position.x == 0) {
        if(t->position.y == 0) {
            t->next_direction = RIGHT;
        } else if (t->position.y == ALLEYS_COUNT - 1) {
            t->next_direction = LEFT;
        } else {
            taxi_make_random_turn(t);
        }
    } else if (t->current_direction == DOWN && t->position.x == STREETS_COUNT - 1) {
        if(t->position.y == 0) {
            t->next_direction = LEFT;
        } else if (t->position.y == ALLEYS_COUNT - 1) {
            t->next_direction = RIGHT;
        } else {
            taxi_make_random_turn(t);
        }
    }
}

position taxi_get_next_position(position pos, direction dir) {
    position next_position = pos;
    switch(dir) {
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
    return next_position;
}

void taxi_try_take_order(taxi *t, order **orders, pthread_mutex_t **order_mutexes) {
    int i;
    for(i = 0; i < MAX_ORDERS; i++) {
        if(pthread_mutex_lock(order_mutexes[i]) != 0) {
            FORCE_EXIT("pthread_mutex_lock");
        }
        if(orders[i] != NULL && orders[i]->available && position_equal(orders[i]->start, t->position)){
            t->current_order_id = i;
            t->stay = 1;
            orders[i]->available = 0;
            printf("Taxi %d taking order %d\n", t->id, i);
            if(pthread_mutex_unlock(order_mutexes[i]) != 0) {
                FORCE_EXIT("pthread_mutex_unlock");
            }
            break;
        }
        if(pthread_mutex_unlock(order_mutexes[i]) != 0) {
            FORCE_EXIT("pthread_mutex_unlock");
        }
    }
}

void taxi_try_finish_order(taxi *t, order **orders, pthread_mutex_t **order_mutexes) {
    order *current_order = orders[t->current_order_id];
    if(position_equal(t->position, current_order->end)) {
        printf("Taxi %d finished order %d\n", t->id, t->current_order_id);
        t->money += ORDER_FINISHED_MONEY;
        if(pthread_mutex_lock(order_mutexes[t->current_order_id]) != 0) {
            FORCE_EXIT("pthread_mutex_lock");
        }                
        free(current_order);
        orders[t->current_order_id] = NULL;
        if(pthread_mutex_unlock(order_mutexes[t->current_order_id]) != 0) {
            FORCE_EXIT("pthread_mutex_unlock");
        }
        t->stay = 1;
        t->current_order_id = -1;
    }
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