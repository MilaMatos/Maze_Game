#ifndef UI_H
#define UI_H

#include "config.h"

void ui_draw_main_menu();
void ui_draw_pause_menu();
void ui_draw_end_screen(const char* bg_texture_name);
void ui_draw_game_hud(int eaten, int total, float timer, GameState state);

int ui_check_click(int x, int y, GameState state);

#endif
