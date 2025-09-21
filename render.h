#ifndef RENDER_H
#define RENDER_H

#include <stdbool.h>
#include "config.h"

bool render_init();
void render_start_frame();
void render_scene(const int maze[MAZE_WIDTH][MAZE_HEIGHT], GameState state);
void render_end_frame();
void render_reshape(int w, int h);
void render_cleanup();

#endif
