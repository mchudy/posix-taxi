#include "map.h"
 
 //TODO synchronize
char* map_generate(taxi **taxis) {
    char *map = (char*) safe_malloc(ROWS_COUNT * ROW_LENGTH);
    map_clean(map);
    map_draw_boundaries(map);
    map_draw_taxis(map, taxis);
    strcpy(map + ((ROWS_COUNT - 1) * ROW_LENGTH), "100 zl\n");
    return map;
}
 
void map_set_char(char* map, char c, int row, int column) {
    map[row * ROW_LENGTH + column] = c;
}

void map_draw_boundaries(char* map) {
    int i, j, k;
    for(i = 0; i < ROW_LENGTH - 1; i++) {
        map_set_char(map, '-', 0, i);
        map_set_char(map, '-', ROWS_COUNT - 2, i);
    }
    for(i = 1; i < ROWS_COUNT - 2; i++) {
        map_set_char(map, '|', i, 0);
        map_set_char(map, '|', i, ROW_LENGTH - 2);
    }
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

void map_draw_taxis(char *map, taxi **taxis) {
    int i, j;
    for(i = 0; i < STREETS_COUNT; i++) {
        for (j = 0; j < ALLEYS_COUNT; j++) {
            if(taxis[i * ALLEYS_COUNT + j] != NULL) {
                map_set_char(map, 'T', 1 + i * 4, j * 10 + 2);
            }
        }
    }
}

void map_draw_order() {
    
}