#include "common.h"
#include <stdlib.h>

int position_equal(position p1, position p2) {
    return p1.x == p2.x && p1.y == p2.y;
}

direction get_random_direction(position pos, unsigned *seed) {
    direction dir = rand_r(seed) % 4;
    return dir;
}