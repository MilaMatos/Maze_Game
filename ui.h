#ifndef UI_H
#define UI_H

#include "config.h"

void ui_draw_main_menu();
void ui_draw_pause_menu();
void ui_draw_win_screen();
void ui_draw_lost_screen();
void ui_draw_game_hud(int eaten, int total, float timer, GameState state);

int ui_check_main_menu_click(int x, int y);
int ui_check_pause_menu_click(int x, int y);

int ui_check_end_screen_click(int x, int y);

#endif
