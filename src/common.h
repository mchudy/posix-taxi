#ifndef COMMON_H
#define COMMON_H

#define TAXI_STREET_TIME 2
#define STREETS_COUNT 10
#define ALLEYS_COUNT 10
#define MAX_TAXIS_PER_CROSSING 5

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

typedef struct city {
    taxi taxis[STREETS_COUNT][ALLEYS_COUNT][MAX_TAXIS_PER_CROSSING];
} city;

#endif