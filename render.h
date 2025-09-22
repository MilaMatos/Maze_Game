#ifndef RENDER_H
#define RENDER_H

#include <stdbool.h>
#include <GL/glut.h>
#include "config.h"

bool render_init();
void render_start_frame();
void render_scene(const int maze[MAZE_WIDTH][MAZE_HEIGHT], GameState state);
void render_end_frame();
void render_reshape(int w, int h);
void render_cleanup();
GLuint render_get_texture_id(const char* name);
void render_update_ambient_light(int eaten, int total, GameState state);

#endif
