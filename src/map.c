#include "map.h"
 
char* map_generate() {
    char *map = (char*) safe_malloc(ROWS_COUNT * ROW_LENGTH);
    map_clean(map);
    map_draw_boundaries(map);
    strcpy(map + ((ROWS_COUNT - 1) * ROW_LENGTH), "100 zl\n");
    return map;
}
 
void map_set_char(char* map, char c, int row, int column) {
    map[row * ROW_LENGTH + column] = c;
}

void map_draw_boundaries(char* map) {
    int i, j;
    for(i = 0; i < ROW_LENGTH - 1; i++) {
        map_set_char(map, '-', 0, i);
        map_set_char(map, '-', ROWS_COUNT - 2, i);
    }
    for(i = 1; i < ROWS_COUNT - 2; i++) {
        map_set_char(map, '|', i, 0);
        map_set_char(map, '|', i, ROW_LENGTH - 2);
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