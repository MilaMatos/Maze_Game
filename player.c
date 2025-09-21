#include <GL/glut.h>
#include <math.h>
#include "player.h"
#include "game.h"
#include <ctype.h>

static Player player;
static bool key_states[256] = {false};

void player_init() {
    player.x = 7.5f;
    player.z = 7.5f;
    player.angle = PI / 2.0f;
    player.pitch = 0.0f;
    player.speed = 0.03f;
}

bool check_collision(float nx, float nz, const int maze[MAZE_WIDTH][MAZE_HEIGHT]) {
    float p_min_x = nx - PLAYER_SIZE / 2.0f;
    float p_max_x = nx + PLAYER_SIZE / 2.0f;
    float p_min_z = nz - PLAYER_SIZE / 2.0f;
    float p_max_z = nz + PLAYER_SIZE / 2.0f;

    int grid_x = (int)(nx / CUBE_SIZE);
    int grid_z = (int)(nz / CUBE_SIZE);

    for (int x = grid_x - 1; x <= grid_x + 1; x++) {
        for (int z = grid_z - 1; z <= grid_z + 1; z++) {
            if (x < 0 || x >= MAZE_WIDTH || z < 0 || z >= MAZE_HEIGHT) continue;

            if (maze[x][z] == 1 || (maze[x][z] == 9 && game_get_state() == STATE_PLAYING)) {
                float w_min_x = x * CUBE_SIZE;
                float w_max_x = x * CUBE_SIZE + CUBE_SIZE;
                float w_min_z = z * CUBE_SIZE;
                float w_max_z = z * CUBE_SIZE + CUBE_SIZE;

                if (p_max_x > w_min_x && p_min_x < w_max_x && p_max_z > w_min_z && p_min_z < w_max_z) {
                    return true;
                }
            }
        }
    }
    return false;
}

void player_update(const int maze[MAZE_WIDTH][MAZE_HEIGHT]) {
    float move_x = 0.0f, move_z = 0.0f;
    if (key_states['w']) { move_x += cos(player.angle) * player.speed; move_z += -sin(player.angle) * player.speed; }
    if (key_states['s']) { move_x -= cos(player.angle) * player.speed; move_z -= -sin(player.angle) * player.speed; }
    if (key_states['a']) { move_x += -sin(player.angle) * player.speed; move_z += -cos(player.angle) * player.speed; }
    if (key_states['d']) { move_x += sin(player.angle) * player.speed; move_z += cos(player.angle) * player.speed; }

    if (move_x != 0.0f || move_z != 0.0f) {
        if (!check_collision(player.x + move_x, player.z + move_z, maze)) {
            player.x += move_x;
            player.z += move_z;
        }
    }
}

void player_handle_keyboard(unsigned char key, bool is_pressed) {
    if (key >= 'a' && key <= 'z') key_states[key] = is_pressed;
    if (key >= 'A' && key <= 'Z') key_states[tolower(key)] = is_pressed;

    if (is_pressed) {
        if (key == 27) { // Tecla ESC
             GameState current_state = game_get_state();
             if (current_state == STATE_PLAYING || current_state == STATE_ESCAPING) {
                 game_set_state(STATE_PAUSED);
                 glutSetCursor(GLUT_CURSOR_INHERIT); // Mostra o cursor
             }
        }
    }
}

void player_handle_mouse_motion(int x, int y) {
    GameState current_state = game_get_state();
    if (current_state == STATE_PLAYING || current_state == STATE_ESCAPING) {
        int w_center = glutGet(GLUT_WINDOW_WIDTH) / 2;
        int h_center = glutGet(GLUT_WINDOW_HEIGHT) / 2;
        if (x == w_center && y == h_center) return;

        float dx = (x - w_center) * 0.003f;
        float dy = (y - h_center) * 0.003f;

        player.angle -= dx;
        player.pitch -= dy;

        if (player.pitch > PITCH_LIMIT) player.pitch = PITCH_LIMIT;
        if (player.pitch < -PITCH_LIMIT) player.pitch = -PITCH_LIMIT;

        glutWarpPointer(w_center, h_center);
    }
}

const Player* player_get() {
    return &player;
}

bool player_get_key_state(unsigned char key) {
    return key_states[key];
}

void player_set_key_state(unsigned char key, bool state) {
    key_states[key] = state;
}
