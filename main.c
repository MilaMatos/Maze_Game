#include <GL/glut.h>
#include "game.h"
#include "player.h"

void display_callback() {
    game_render();
}

void reshape_callback(int w, int h) {
    game_reshape(w, h);
}

void keyboard_callback(unsigned char key, int x, int y) {
    player_handle_keyboard(key, true);
}

void keyboard_up_callback(unsigned char key, int x, int y) {
    player_handle_keyboard(key, false);
}

void mouse_motion_callback(int x, int y) {
    player_handle_mouse_motion(x, y);
}

void mouse_click_callback(int button, int state, int x, int y) {
    game_handle_mouse_click(button, state, x, y);
}

void update_callback(int value) {
    game_update();
    glutTimerFunc(16, update_callback, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("MAZE OF LOST SOULS");

    if (!game_init()) {
        return -1;
    }

    glutDisplayFunc(display_callback);
    glutReshapeFunc(reshape_callback);
    glutKeyboardFunc(keyboard_callback);
    glutKeyboardUpFunc(keyboard_up_callback);
    glutPassiveMotionFunc(mouse_motion_callback);
    glutMouseFunc(mouse_click_callback);
    glutTimerFunc(16, update_callback, 0);

    glutMainLoop();

    game_cleanup();
    return 0;
}
