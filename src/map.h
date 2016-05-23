#ifndef MAP_H
#define MAP_H

#include <stdlib.h>
#include "common.h"
#include "utils.h"

#define ROW_LENGTH (10 * ALLEYS_COUNT - 3)
#define ROWS_COUNT (4 * STREETS_COUNT)
#define ALLEYS_DISTANCE 6

char* map_generate(taxi **taxis, int current_taxi_id);

/* Helper functions */
void map_set_char(char* map, char c, int row, int column);
void map_draw_boundaries(char* map);
void map_clean(char* map);
void map_draw_taxis(char *map, taxi **taxis, int current_taxi_id);
void map_draw_orders();
char map_get_direction_char(direction dir);

#endif