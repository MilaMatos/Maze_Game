#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "config.h"
#include "render.h"
#include "player.h"
#include "ui.h"

// --- Variáveis de Estado do Jogo ---
GameState game_state;
int collectibles_eaten = 0;
int total_collectibles = 0;
float escape_timer = ESCAPE_SECONDS;

// --- Layout do Labirinto ---
int maze_grid[MAZE_WIDTH][MAZE_HEIGHT] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, {1,0,0,1,0,0,0,0,0,1,1,0,1,0,1}, {1,1,0,1,1,1,0,1,0,0,1,0,1,0,1},
    {1,0,0,0,0,1,0,1,0,1,1,0,0,0,1}, {1,0,1,1,0,1,0,1,0,1,0,0,1,0,1}, {1,0,0,1,0,1,0,0,0,1,0,0,1,0,1},
    {1,1,0,1,0,0,1,1,1,1,0,1,1,0,1}, {1,0,0,0,0,1,1,0,0,0,0,0,0,0,1}, {1,0,1,1,1,1,0,0,1,0,1,1,1,0,1},
    {1,0,0,0,0,0,0,1,1,0,0,0,1,0,1}, {1,0,1,1,1,1,0,1,0,1,1,0,1,2,1}, {1,0,0,0,0,1,0,0,0,0,1,0,0,0,1},
    {1,1,1,0,1,1,1,1,1,0,1,1,1,0,1}, {1,0,0,0,0,0,0,0,1,0,0,0,0,9,1}, {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
// Cópia para resetar o jogo
int initial_maze_grid[MAZE_WIDTH][MAZE_HEIGHT];


void reset_game() {
    player_init();
    collectibles_eaten = 0;
    escape_timer = ESCAPE_SECONDS;
    memcpy(maze_grid, initial_maze_grid, sizeof(maze_grid));
    game_state = STATE_PLAYING;
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
    glutPostRedisplay(); // Marca a tela para redesenhar

    // A lógica de atualização só acontece se estivermos jogando
    if (game_state != STATE_PLAYING && game_state != STATE_ESCAPING) {
        return;
    }

    player_update(maze_grid);

    int px = (int)(player_get()->x / CUBE_SIZE);
    int pz = (int)(player_get()->z / CUBE_SIZE);

    if (px >= 0 && px < MAZE_WIDTH && pz >= 0 && pz < MAZE_HEIGHT) {
        if (maze_grid[px][pz] == 2) {
            maze_grid[px][pz] = 0;
            collectibles_eaten++;
            if (collectibles_eaten == total_collectibles) {
                game_state = STATE_ESCAPING;
            }
        }
    }

    if (game_state == STATE_ESCAPING) {
        escape_timer -= 16.0f / 1000.0f;
        // Área de vitória: precisa estar na coordenada X da saída e ter passado dela em Z
        if (px == 13 && player_get()->z > 13.5 * CUBE_SIZE) {
             game_state = STATE_WON;
             glutSetCursor(GLUT_CURSOR_INHERIT);
        } else if (escape_timer <= 0.0f) {
            escape_timer = 0.0f;
            game_state = STATE_LOST;
            glutSetCursor(GLUT_CURSOR_INHERIT);
        }
    }
}


void game_render() {
    render_start_frame();

    // 1. Desenha a cena 3D se não estivermos no menu principal
    if (game_state != STATE_MAIN_MENU) {
        render_scene(maze_grid, game_state);
    }

    // 2. Desenha o HUD (timer/esferas) se estivermos jogando
    if (game_state == STATE_PLAYING || game_state == STATE_ESCAPING) {
        ui_draw_game_hud(collectibles_eaten, total_collectibles, escape_timer, game_state);
    }

    // 3. Desenha os menus/telas de UI por cima de tudo
    switch (game_state) {
        case STATE_MAIN_MENU:
            ui_draw_main_menu();
            break;
        case STATE_PAUSED:
            ui_draw_pause_menu();
            break;
        case STATE_WON:
            ui_draw_win_screen();
            break;
        case STATE_LOST:
            ui_draw_lost_screen();
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
        int choice = 0;
        switch (game_get_state()) {
            case STATE_MAIN_MENU:
                choice = ui_check_main_menu_click(x, y);
                if (choice == 1) reset_game();
                if (choice == 2) game_cleanup();
                break;
            case STATE_PAUSED:
                choice = ui_check_pause_menu_click(x, y);
                if (choice == 1) { // Continuar
                     game_set_state(STATE_PLAYING);
                     glutSetCursor(GLUT_CURSOR_NONE);
                }
                if (choice == 2) game_set_state(STATE_MAIN_MENU);
                if (choice == 3) game_cleanup(); // Sair
                break;
            // --- NOVO: Lógica para os cliques nas telas de vitória e derrota ---
            case STATE_WON:
            case STATE_LOST:
                choice = ui_check_end_screen_click(x, y);
                if (choice == 1) game_set_state(STATE_MAIN_MENU); // Voltar ao Menu
                if (choice == 2) game_cleanup(); // Sair
                break;
            default: break;
        }
    }
}

void game_set_state(GameState new_state) {
    game_state = new_state;
}

GameState game_get_state() {
    return game_state;
}


void game_cleanup() {
    render_cleanup();
    exit(0);
}
