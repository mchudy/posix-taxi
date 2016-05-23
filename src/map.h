#ifndef MAP_H
#define MAP_H

#include <stdlib.h>
#include "common.h"
#include "utils.h"

#define ROW_LENGTH (10 * ALLEYS_COUNT - 3)
#define ROWS_COUNT (4 * STREETS_COUNT)
#define ALLEYS_DISTANCE 6

char* map_generate(taxi **taxis);

/* Helper functions */
void map_set_char(char* map, char c, int row, int column);
void map_draw_boundaries(char* map);
void map_clean(char* map);
void map_draw_taxis(char *map, taxi **taxis);
void map_draw_order();

#endif