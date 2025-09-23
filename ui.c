#include <GL/glut.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "ui.h"
#include "render.h"

typedef struct {
    float x, y, w, h;
    const char* text;
} Button;

static void draw_fullscreen_texture(const char* texture_name);
static int check_button_array_click(int x, int y, Button buttons[], int num_buttons);

static Button main_menu_buttons[] = {
    {0, 0, 200, 50, "Iniciar Jogo"},
    {0, 0, 200, 50, "Sair"}
};
static int num_main_menu_buttons = 2;

static Button pause_menu_buttons[] = {
    {0, 0, 200, 50, "Continuar"},
    {0, 0, 200, 50, "Voltar ao Menu"},
    {0, 0, 200, 50, "Sair do Jogo"}
};
static int num_pause_menu_buttons = 3;

static Button end_screen_buttons[] = {
    {0, 0, 220, 50, "Voltar ao Menu"},
    {0, 0, 220, 50, "Sair do Jogo"}
};
static int num_end_screen_buttons = 2;

static void draw_text(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
}

static void draw_button(Button* btn) {
    glColor4f(1.0, 1.0, 1.0, 0.15);
    glRectf(btn->x, btn->y, btn->x + btn->w, btn->y + btn->h);

    glColor3f(1.0, 1.0, 1.0);
    int text_width = 0;
    for (const char* c = btn->text; *c != '\0'; c++) {
        text_width += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
    float text_x = btn->x + (btn->w - text_width) / 2.0;
    float text_y = btn->y + (btn->h / 2.0) - 8;
    draw_text(text_x, text_y, btn->text);
}

static void begin_ui_render() {
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING); glDisable(GL_TEXTURE_2D); glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void end_ui_render() {
    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST); glEnable(GL_TEXTURE_2D); glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

void ui_draw_main_menu() {
    begin_ui_render();
    draw_fullscreen_texture("menu_bg");

    int win_w = glutGet(GLUT_WINDOW_WIDTH);
    int win_h = glutGet(GLUT_WINDOW_HEIGHT);

    float btn_x = win_w / 2 - 100;
    main_menu_buttons[0].x = btn_x; main_menu_buttons[0].y = win_h / 2;
    draw_button(&main_menu_buttons[0]);
    main_menu_buttons[1].x = btn_x; main_menu_buttons[1].y = win_h / 2 - 70;
    draw_button(&main_menu_buttons[1]);
    end_ui_render();
}

void ui_draw_pause_menu() {
    begin_ui_render();
    draw_fullscreen_texture("menu_bg");

    int win_w = glutGet(GLUT_WINDOW_WIDTH);
    int win_h = glutGet(GLUT_WINDOW_HEIGHT);
    glColor4f(0.0, 0.0, 0.0, 0.7);
    glRectf(0, 0, win_w, win_h);

    glColor3f(1.0, 1.0, 0.8);
    draw_text(win_w / 2 - 50, win_h - 100, "PAUSADO");

    float btn_x = win_w / 2 - 100;
    pause_menu_buttons[0].x = btn_x; pause_menu_buttons[0].y = win_h / 2;
    draw_button(&pause_menu_buttons[0]);
    pause_menu_buttons[1].x = btn_x; pause_menu_buttons[1].y = win_h / 2 - 70;
    draw_button(&pause_menu_buttons[1]);
    pause_menu_buttons[2].x = btn_x; pause_menu_buttons[2].y = win_h / 2 - 140;
    draw_button(&pause_menu_buttons[2]);
    end_ui_render();
}

void ui_draw_end_screen(const char* bg_texture_name) {
    begin_ui_render();
    draw_fullscreen_texture(bg_texture_name);

    int win_w = glutGet(GLUT_WINDOW_WIDTH);
    int win_h = glutGet(GLUT_WINDOW_HEIGHT);

    float btn_x = win_w / 2 - 110;
    end_screen_buttons[0].x = btn_x;
    end_screen_buttons[0].y = win_h / 2 - 150;
    draw_button(&end_screen_buttons[0]);

    end_screen_buttons[1].x = btn_x;
    end_screen_buttons[1].y = win_h / 2 - 220;
    draw_button(&end_screen_buttons[1]);

    end_ui_render();
}

void ui_draw_game_hud(int eaten, int total, float timer, GameState state) {
    begin_ui_render();
    char text[100];
    if (state == STATE_ESCAPING) {
        sprintf(text, "SAIDA ABERTA! TEMPO: %.1f", timer);
    } else {
        sprintf(text, "Esferas: %d / %d", eaten, total);
    }
    glColor3f(1.0, 1.0, 1.0);
    draw_text(20, glutGet(GLUT_WINDOW_HEIGHT) - 30, text);
    end_ui_render();
}

static void draw_fullscreen_texture(const char* texture_name) {
    GLuint tex_id = render_get_texture_id(texture_name);
    if (tex_id == 0) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glColor3f(1.0, 1.0, 1.0);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex2f(0, 0);
        glTexCoord2f(1.0, 0.0); glVertex2f(glutGet(GLUT_WINDOW_WIDTH), 0);
        glTexCoord2f(1.0, 1.0); glVertex2f(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
        glTexCoord2f(0.0, 1.0); glVertex2f(0, glutGet(GLUT_WINDOW_HEIGHT));
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

static bool is_point_in_rect(int px, int py, Button* btn) {
    int gl_y = glutGet(GLUT_WINDOW_HEIGHT) - py;
    return (px >= btn->x && px <= btn->x + btn->w && gl_y >= btn->y && gl_y <= btn->y + btn->h);
}

static int check_button_array_click(int x, int y, Button buttons[], int num_buttons) {
    for (int i = 0; i < num_buttons; i++) {
        if (is_point_in_rect(x, y, &buttons[i])) return i + 1;
    }
    return 0;
}

int ui_check_click(int x, int y, GameState state) {
    switch (state) {
        case STATE_MAIN_MENU:
            return check_button_array_click(x, y, main_menu_buttons, num_main_menu_buttons);
        case STATE_PAUSED:
            return check_button_array_click(x, y, pause_menu_buttons, num_pause_menu_buttons);
        case STATE_WON:
        case STATE_LOST:
            return check_button_array_click(x, y, end_screen_buttons, num_end_screen_buttons);
        default:
            return 0;
    }
}
