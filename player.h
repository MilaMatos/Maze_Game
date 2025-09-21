#ifndef PLAYER_H
#define PLAYER_H

#include "config.h"
#include <stdbool.h>

void player_init();
void player_update(const int maze[MAZE_WIDTH][MAZE_HEIGHT]);
void player_handle_keyboard(unsigned char key, bool is_pressed);
void player_handle_mouse_motion(int x, int y);

const Player* player_get();
bool player_get_key_state(unsigned char key);
void player_set_key_state(unsigned char key, bool state);

#endif
