// Definition of the direction type, which relies on the enum type compass:
enum compass {NORTH, EAST, SOUTH, WEST};

//  A direction is a value from the enum compass type, ie NORTH, EAST, SOUTH or WEST
typedef enum compass direction;

// prototype of the pacman function to code
direction pacman(char * *, int, int, int, int, direction, bool, int);

