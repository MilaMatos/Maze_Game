#ifndef CONFIG_H
#define CONFIG_H

#define MAZE_WIDTH 15
#define MAZE_HEIGHT 15
#define CUBE_SIZE 1.0f
#define WALL_HEIGHT 1.5f
#define PLAYER_SIZE 0.2f

#define ESCAPE_SECONDS 30.0f

#define PI 3.1415926535
#define PITCH_LIMIT 1.55f

typedef enum {
    STATE_MAIN_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_ESCAPING,
    STATE_WON,
    STATE_LOST
} GameState;

typedef struct {
    float x, y, z;
    float angle;
    float pitch;
    float speed;
} Player;

#endif
