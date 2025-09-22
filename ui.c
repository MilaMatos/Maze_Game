#include <GL/glut.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h> // Adicionado para usar strlen
#include "ui.h"
#include "render.h"

// Estrutura interna para um botão
typedef struct {
    float x, y, w, h;
    const char* text;
} Button;

// Protótipo da função auxiliar
static void draw_fullscreen_texture(const char* texture_name);

// Definições dos botões dos menus
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

// Função auxiliar para desenhar texto
static void draw_text(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
}

// Função auxiliar para desenhar um botão
static void draw_button(Button* btn) {
    // MODIFICAÇÃO: Cor do botão alterada para branco com transparência
    glColor4f(1.0, 1.0, 1.0, 0.15);
    glRectf(btn->x, btn->y, btn->x + btn->w, btn->y + btn->h);

    // MODIFICAÇÃO: Lógica para centralizar o texto no botão
    glColor3f(1.0, 1.0, 1.0);
    int text_width = 0;
    for (const char* c = btn->text; *c != '\0'; c++) {
        text_width += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
    float text_x = btn->x + (btn->w - text_width) / 2.0;
    float text_y = btn->y + (btn->h / 2.0) - 8; // Ajuste vertical para a fonte
    draw_text(text_x, text_y, btn->text);
}

static void begin_ui_render() {
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING); glDisable(GL_TEXTURE_2D); glDisable(GL_DEPTH_TEST);

    // MODIFICAÇÃO: Habilita o blending para permitir transparência
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void end_ui_render() {
    // MODIFICAÇÃO: Desabilita o blending
    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST); glEnable(GL_TEXTURE_2D); glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

void ui_draw_main_menu() {
    begin_ui_render();
    draw_fullscreen_texture("menu_bg");

    int win_w = glutGet(GLUT_WINDOW_WIDTH), win_h = glutGet(GLUT_WINDOW_HEIGHT);

    float btn_x = win_w / 2 - 100;
    main_menu_buttons[0].x = btn_x; main_menu_buttons[0].y = win_h / 2;
    draw_button(&main_menu_buttons[0]);
    main_menu_buttons[1].x = btn_x; main_menu_buttons[1].y = win_h / 2 - 70;
    draw_button(&main_menu_buttons[1]);
    end_ui_render();
}

void ui_draw_pause_menu() {
    begin_ui_render();
    // Não desenha a textura de fundo para manter o jogo visível
    draw_fullscreen_texture("menu_bg");

    int win_w = glutGet(GLUT_WINDOW_WIDTH), win_h = glutGet(GLUT_WINDOW_HEIGHT);
    // Adiciona um overlay escuro semi-transparente
    glColor4f(0.0, 0.0, 0.0, 0.7);
    glRectf(0, 0, win_w, win_h);

    glColor3f(1.0, 1.0, 0.8);
    draw_text(win_w / 2 - 50, win_h - 100, "PAUSADO");

    float btn_x = win_w / 2 - 100;
    // MODIFICAÇÃO: Posições dos botões de pause alteradas
    pause_menu_buttons[0].x = btn_x; pause_menu_buttons[0].y = win_h / 2;
    draw_button(&pause_menu_buttons[0]);
    pause_menu_buttons[1].x = btn_x; pause_menu_buttons[1].y = win_h / 2 - 70;
    draw_button(&pause_menu_buttons[1]);
    pause_menu_buttons[2].x = btn_x; pause_menu_buttons[2].y = win_h / 2 - 140;
    draw_button(&pause_menu_buttons[2]);
    end_ui_render();
}

void ui_draw_win_screen() {
    begin_ui_render();
    draw_fullscreen_texture("win_bg");
    int win_w = glutGet(GLUT_WINDOW_WIDTH), win_h = glutGet(GLUT_WINDOW_HEIGHT);

    float btn_x = win_w / 2 - 110;
    end_screen_buttons[0].x = btn_x;
    end_screen_buttons[0].y = win_h / 2 - 150;
    draw_button(&end_screen_buttons[0]);

    end_screen_buttons[1].x = btn_x;
    end_screen_buttons[1].y = win_h / 2 - 220;
    draw_button(&end_screen_buttons[1]);

    end_ui_render();
}

void ui_draw_lost_screen() {
    begin_ui_render();
    draw_fullscreen_texture("loser_bg");
    int win_w = glutGet(GLUT_WINDOW_WIDTH), win_h = glutGet(GLUT_WINDOW_HEIGHT);


    float btn_x = win_w / 2 - 110;
    end_screen_buttons[0].x = btn_x; end_screen_buttons[0].y = win_h / 2 - 150;
    draw_button(&end_screen_buttons[0]);
    end_screen_buttons[1].x = btn_x; end_screen_buttons[1].y = win_h / 2 - 220;
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

// Função auxiliar para desenhar a imagem de fundo
static void draw_fullscreen_texture(const char* texture_name) {
    GLuint tex_id = render_get_texture_id(texture_name);
    if (tex_id == 0) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glColor3f(1.0, 1.0, 1.0); // A textura não precisa de transparência aqui

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

int ui_check_main_menu_click(int x, int y) {
    for (int i = 0; i < num_main_menu_buttons; i++) {
        if (is_point_in_rect(x, y, &main_menu_buttons[i])) return i + 1;
    }
    return 0;
}

int ui_check_pause_menu_click(int x, int y) {
    for (int i = 0; i < num_pause_menu_buttons; i++) {
        if (is_point_in_rect(x, y, &pause_menu_buttons[i])) return i + 1;
    }
    return 0;
}

int ui_check_end_screen_click(int x, int y) {
    for (int i = 0; i < num_end_screen_buttons; i++) {
        if (is_point_in_rect(x, y, &end_screen_buttons[i])) return i + 1;
    }
    return 0;
}
