#ifndef GAME_H
#define GAME_H
#include "config.h"
#include <stdbool.h>

bool game_init();
void game_update();
void game_render();
void game_reshape(int w, int h);
void game_handle_mouse_click(int button, int state, int x, int y);
void game_cleanup();
GameState game_get_state();
void game_set_state(GameState new_state);

#endif
