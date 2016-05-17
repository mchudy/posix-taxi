typedef struct position {
    int row;
    int column;
} position;

typedef enum direction {
    LEFT,
    RIGHT,
    UP,
    DOWN
} direction;

typedef struct taxi {
    int id;
    int current_order_id;
    int money;
    direction current_direction;
    direction next_direction;
    struct position position;
} taxi;

typedef struct order {
    int id;
    position start;
    position end;
    int available;
} order;