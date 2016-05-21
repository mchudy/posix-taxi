#include <stdlib.h>
#include "common.h"
#include "utils.h"

#define ROW_LENGTH (11 * ALLEYS_COUNT + 1)
#define ROWS_COUNT (4 * STREETS_COUNT + 3)

char* map_generate();

/* Helper functions */
void map_set_char(char* map, char c, int row, int column);
void map_draw_boundaries(char* map);
void map_clean(char* map);