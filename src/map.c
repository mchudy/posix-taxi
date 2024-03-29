#include "map.h"
#include "order.h"

char* map_generate(taxi **taxis, taxi *current_taxi, pthread_mutex_t *mutex, 
                   order **orders, pthread_mutex_t **order_mutexes) {
    char *map = (char*) safe_malloc(ROWS_COUNT * ROW_LENGTH);
    map_clean(map);
    map_draw_boundaries(map);
    if(pthread_mutex_lock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_lock");
    }
    map_draw_orders(map, current_taxi, orders, order_mutexes);
    map_draw_taxis(map, taxis, current_taxi->id);
    sprintf(map + (ROWS_COUNT - 1) * ROW_LENGTH, "$%d\n", current_taxi->money);
    if(pthread_mutex_unlock(mutex) != 0) {
        FORCE_EXIT("pthread_mutex_unlock");
    }
    return map;
}

void map_set_char(char* map, char c, int row, int column) {
    map[row * ROW_LENGTH + column] = c;
}

void map_draw_boundaries(char* map) {
    int i, j, k;
    // horizontal boundaries
    for(i = 0; i < ROW_LENGTH - 1; i++) {
        map_set_char(map, '-', 0, i);
        map_set_char(map, '-', ROWS_COUNT - 2, i);
    }
    // vertical boundaries
    for(i = 1; i < ROWS_COUNT - 2; i++) {
        map_set_char(map, '|', i, 0);
        map_set_char(map, '|', i, ROW_LENGTH - 2);
    }
    // places between streets
    for(i = 0; i < STREETS_COUNT - 1; i++) {
        for(j = 0; j < ALLEYS_COUNT - 1; j++) {
            map_set_char(map, '|', 3 + i * 4, j * 10 + 5);
            map_set_char(map, '|', 3 + i * 4, j * 10 + 10);
            for(k = 0; k < ALLEYS_DISTANCE; k++) {
                map_set_char(map, '-', 2 + i * 4, j * 10 + 5 + k);
                map_set_char(map, '-', 4 + i * 4, j * 10 + 5 + k);
            }
        }
    }
}

void map_clean(char* map) {
    int i, j;
    for(i = 0; i < ROWS_COUNT; i++) {
        for(j = 0; j < ROW_LENGTH; j++) {
            if(j == ROW_LENGTH - 1) {
                map_set_char(map, '\n', i, j);
            } else {
                map_set_char(map, ' ', i, j);
            }       
        }
    }
}

void map_draw_taxis(char *map, taxi **taxis, int current_taxi_id) {
    int i, j;
    for(i = 0; i < STREETS_COUNT; i++) {
        for (j = 0; j < ALLEYS_COUNT; j++) {
            taxi *t = taxis[i * ALLEYS_COUNT + j];
            if(t != NULL) {
                char taxi_char = t->id == current_taxi_id ? '#' : 'T';
                map_set_char(map, taxi_char, 1 + i * 4, j * 10 + 2);
                map_set_char(map, map_get_direction_char(t->current_direction), 
                        1 + i * 4, j * 10 + 3);
            }
        }
    }
}

char map_get_direction_char(direction dir) {
    switch(dir) {
        case LEFT:
            return '<';
            break;
        case RIGHT:
            return '>';
            break;
        case UP:
            return '^';
            break;
        case DOWN:
        default:
            return 'V';
    }
}

void map_draw_orders(char *map, taxi *t, order **orders, pthread_mutex_t **order_mutexes) {
    // order number string (including \0)
    char str[2];
    if(t->current_order_id == -1) {
        // showing all available start points
        int i;
        lock_orders(order_mutexes);
        for(i = 0; i < MAX_ORDERS; i++) {
            if(orders[i] == NULL || !orders[i]->available) continue;
            if(snprintf(str, 2, "%d", orders[i]->id) < 0) {
                FORCE_EXIT("snprintf");
            }
            map_set_char(map, 'A', 1 + orders[i]->start.x * 4, orders[i]->start.y * 10 + 2);
            map_set_char(map, str[0], 1 + orders[i]->start.x * 4, orders[i]->start.y * 10 + 3);
        }
        unlock_orders(order_mutexes);
    } else {
        // showing only end point for the current order
        if(snprintf(str, 2, "%d", t->current_order_id) < 0) {
            FORCE_EXIT("snprintf");
        }
        map_set_char(map, 'B', 1 + orders[t->current_order_id]->end.x * 4, 
                     orders[t->current_order_id]->end.y * 10 + 2);
        map_set_char(map, str[0], 1 + orders[t->current_order_id]->end.x * 4, 
                     orders[t->current_order_id]->end.y * 10 + 3);
    }
}
