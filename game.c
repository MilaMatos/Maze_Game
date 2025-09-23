#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "config.h"
#include "render.h"
#include "player.h"
#include "ui.h"

static GameState game_state;
static GameState previous_game_state;
static int collectibles_eaten = 0;
static int total_collectibles = 0;
static float escape_timer = ESCAPE_SECONDS;

static int maze_grid[MAZE_WIDTH][MAZE_HEIGHT] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, {1,0,0,1,0,0,0,0,0,1,1,0,1,2,1}, {1,1,0,1,1,1,0,1,0,0,1,0,1,0,1},
    {1,0,0,0,0,1,0,1,0,1,1,0,0,0,1}, {1,0,1,1,0,1,0,1,0,1,0,0,1,0,1}, {1,0,0,1,0,1,0,0,0,1,0,0,1,0,1},
    {1,1,0,1,0,0,1,1,1,1,0,1,1,0,1}, {1,0,0,0,0,1,1,0,0,0,0,0,0,0,1}, {1,0,1,1,1,1,0,0,1,0,1,1,1,0,1},
    {1,0,0,0,0,0,0,1,1,0,0,0,1,0,1}, {1,0,1,1,1,1,0,1,0,1,1,0,1,2,1}, {1,0,0,0,0,1,0,0,0,0,1,0,0,0,1},
    {1,1,1,0,1,1,1,1,1,0,1,1,1,0,1}, {1,0,0,0,0,0,0,0,1,0,0,0,0,0,1}, {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
static int initial_maze_grid[MAZE_WIDTH][MAZE_HEIGHT];


void reset_game() {
    player_init();
    collectibles_eaten = 0;
    escape_timer = ESCAPE_SECONDS;
    memcpy(maze_grid, initial_maze_grid, sizeof(maze_grid));
    game_set_state(STATE_PLAYING);
    render_update_ambient_light(collectibles_eaten, total_collectibles, game_get_state());
    glutSetCursor(GLUT_CURSOR_NONE);
}

bool game_init() {
    if (!render_init()) return false;

    memcpy(initial_maze_grid, maze_grid, sizeof(maze_grid));
    total_collectibles = 0;
    for (int x = 0; x < MAZE_WIDTH; x++) {
        for (int z = 0; z < MAZE_HEIGHT; z++) {
            if (maze_grid[x][z] == 2) total_collectibles++;
        }
    }

    player_init();
    game_state = STATE_MAIN_MENU;
    return true;
}

void game_update() {
    glutPostRedisplay();

    if (game_state == STATE_PLAYING || game_state == STATE_ESCAPING) {
        player_update(maze_grid);

        int px = (int)(player_get()->x / CUBE_SIZE);
        int pz = (int)(player_get()->z / CUBE_SIZE);

        if (px >= 0 && px < MAZE_WIDTH && pz >= 0 && pz < MAZE_HEIGHT) {
            if (maze_grid[px][pz] == 2) {
                maze_grid[px][pz] = 0;
                collectibles_eaten++;
                render_update_ambient_light(collectibles_eaten, total_collectibles, game_get_state());

                if (collectibles_eaten == total_collectibles) {
                    game_set_state(STATE_ESCAPING);
                    render_update_ambient_light(collectibles_eaten, total_collectibles, game_get_state());
                }
            }
        }

        if (game_state == STATE_ESCAPING) {
            escape_timer -= 16.0f / 1000.0f;
            if (player_get()->y < -8.0f) {
                 game_set_state(STATE_WON);
                 glutSetCursor(GLUT_CURSOR_INHERIT);
            } else if (escape_timer <= 0.0f) {
                escape_timer = 0.0f;
                game_set_state(STATE_LOST);
                glutSetCursor(GLUT_CURSOR_INHERIT);
            }
        }
    }
}


void game_render() {
    render_start_frame();

    if (game_state != STATE_MAIN_MENU) {
        render_scene(maze_grid, game_state);
    }

    if (game_state == STATE_PLAYING || game_state == STATE_ESCAPING) {
        ui_draw_game_hud(collectibles_eaten, total_collectibles, escape_timer, game_state);
    }

    switch (game_state) {
        case STATE_MAIN_MENU:
            ui_draw_main_menu();
            break;
        case STATE_PAUSED:
            ui_draw_pause_menu();
            break;
        case STATE_WON:
            ui_draw_end_screen("win_bg");
            break;
        case STATE_LOST:
            ui_draw_end_screen("loser_bg");
            break;
        default: break;
    }

    render_end_frame();
}

void game_reshape(int w, int h) {
    render_reshape(w, h);
}

void game_handle_mouse_click(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        GameState current_state = game_get_state();

        int choice = ui_check_click(x, y, current_state);

        if (choice == 0) return;

        if (current_state == STATE_MAIN_MENU) {
            if (choice == 1) reset_game();
            if (choice == 2) game_cleanup();
        }
        else if (current_state == STATE_PAUSED) {
            if (choice == 1) {
                 game_set_state(game_get_previous_state());
                 glutSetCursor(GLUT_CURSOR_NONE);
            }
            if (choice == 2) game_set_state(STATE_MAIN_MENU);
            if (choice == 3) game_cleanup();
        }
        else if (current_state == STATE_WON || current_state == STATE_LOST) {
            if (choice == 1) game_set_state(STATE_MAIN_MENU);
            if (choice == 2) game_cleanup();
        }
    }
}


void game_set_state(GameState new_state) {
    if (new_state == STATE_PAUSED) {
        previous_game_state = game_state;
    }
    game_state = new_state;
}

GameState game_get_state() {
    return game_state;
}

GameState game_get_previous_state() {
    return previous_game_state;
}

void game_cleanup() {
    render_cleanup();
    exit(0);
}
