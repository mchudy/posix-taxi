#ifndef COMMON_H
#define COMMON_H

#define TAXI_STREET_TIME 10
#define STREETS_COUNT 10
#define ALLEYS_COUNT 10
#define MAX_ORDERS 5

#define REVERSE_DIRECTION(dir) ((dir + 2) % 4)

typedef struct position {
    int x;
    int y;
} position;

typedef enum direction {
    LEFT,
    UP,
    RIGHT,
    DOWN
} direction;

typedef struct taxi {
    int id;
    int current_order_id;
    int money;
    int collision;
    /* Set to non-zero value makes taxi stay for 1 move in the same place */
    int stay; 
    direction current_direction;
    direction next_direction;
    position position;
} taxi;

typedef struct order {
    int id;
    position start;
    position end;
    int available;
} order;

int position_equal(position p1, position p2);
direction get_random_direction(position pos, unsigned *seed);

#endif