#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "render.h"
#include "texture.h"
#include "player.h"

static GLuint wall_texture_id, floor_texture_id, ceiling_texture_id;
static GLuint door_texture_id, sphere_texture_id;
static GLuint menu_bg_texture_id, loser_bg_texture_id;
static GLuint win_bg_texture_id;
static GLUquadric* sphere_quadric = NULL;

static GLfloat base_global_ambient[] = {0.08, 0.08, 0.06, 1.0};
static GLfloat current_global_ambient[] = {0.08, 0.08, 0.06, 1.0};

static void draw_maze(const int maze[MAZE_WIDTH][MAZE_HEIGHT], GameState state);
static void draw_ceiling_and_floor(const int maze[MAZE_WIDTH][MAZE_HEIGHT], GameState state);
static void draw_exit_hole();
static void draw_collectibles(const int maze[MAZE_WIDTH][MAZE_HEIGHT]);
static void draw_exit_sphere(GameState state);
static void draw_textured_cube(GLuint texture_id);
static void setup_lighting();
static void lighting_update_dynamic(const int maze[MAZE_WIDTH][MAZE_HEIGHT], GameState state);

bool render_init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    wall_texture_id = load_texture_bmp("textures/wall4.bmp");
    floor_texture_id = load_texture_bmp("textures/floor.bmp");
    ceiling_texture_id = load_texture_bmp("textures/ceiling.bmp");
    door_texture_id = load_texture_bmp("textures/door4.bmp");
    sphere_texture_id = load_texture_bmp("textures/sphere2.bmp");
    menu_bg_texture_id = load_texture_bmp("textures/menu_bg.bmp");
    loser_bg_texture_id = load_texture_bmp("textures/loser_bg.bmp");
    win_bg_texture_id = load_texture_bmp("textures/win_bg.bmp");

    if (wall_texture_id==0 || floor_texture_id==0 || ceiling_texture_id==0 ||
        door_texture_id==0 || sphere_texture_id==0 || menu_bg_texture_id==0 ||
        loser_bg_texture_id == 0 || win_bg_texture_id == 0) {
        return false;
    }

    sphere_quadric = gluNewQuadric();
    gluQuadricTexture(sphere_quadric, GL_TRUE);

    setup_lighting();
    return true;
}

void render_start_frame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, current_global_ambient);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void render_end_frame() {
    glutSwapBuffers();
}

void render_scene(const int maze[MAZE_WIDTH][MAZE_HEIGHT], GameState state) {
    const Player* p = player_get();

    float look_horizontal_dist = cos(p->pitch);
    float look_x = p->x + cos(p->angle) * look_horizontal_dist;
    float look_y = p->y + sin(p->pitch);
    float look_z = p->z - sin(p->angle) * look_horizontal_dist;

    gluLookAt(p->x, p->y, p->z, look_x, look_y, look_z, 0.0, 1.0, 0.0);

    lighting_update_dynamic(maze, state);

    draw_ceiling_and_floor(maze, state);
    draw_maze(maze, state);

    glBindTexture(GL_TEXTURE_2D, 0);
    draw_collectibles(maze);
    draw_exit_sphere(state);
}

void render_reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / h, 0.1, 100.0);
}

void render_cleanup() {
    if (sphere_quadric) {
        gluDeleteQuadric(sphere_quadric);
    }
}

void render_update_ambient_light(int eaten, int total, GameState state) {
    float max_progressive_red = 0.3f;
    float final_red_boost = 0.35f;

    memcpy(current_global_ambient, base_global_ambient, sizeof(base_global_ambient));

    if (total > 0) {
        float red_increase = (float)eaten / (float)total * max_progressive_red;
        current_global_ambient[0] += red_increase;
    }

    if (state == STATE_ESCAPING || state == STATE_WON || state == STATE_LOST) {
        current_global_ambient[0] += final_red_boost;
    }

    if (current_global_ambient[0] > 1.0f) current_global_ambient[0] = 1.0f;
}

static void setup_lighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, base_global_ambient);

    glDisable(GL_LIGHT0);

    glEnable(GL_LIGHT1);
    GLfloat sphere_diffuse[] = {1.0, 0.1, 0.1, 1.0};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, sphere_diffuse);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.8f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.3f);

    glEnable(GL_LIGHT2);
    GLfloat exit_light_diffuse[] = {1.0, 0.0, 0.0, 1.0};
    glLightfv(GL_LIGHT2, GL_DIFFUSE, exit_light_diffuse);
    glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.7f);
    glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.2f);
}

static void lighting_update_dynamic(const int maze[MAZE_WIDTH][MAZE_HEIGHT], GameState state) {
    const Player* p = player_get();

    float min_dist_sq = -1.0f;
    float closest_sphere_pos[4] = {0,0,0,1};
    bool found_sphere = false;
    for (int x = 0; x < MAZE_WIDTH; x++) {
        for (int z = 0; z < MAZE_HEIGHT; z++) {
            if (maze[x][z] == 2) {
                float sx = x * CUBE_SIZE + 0.5f;
                float sz = z * CUBE_SIZE + 0.5f;
                float dist_sq = (p->x - sx) * (p->x - sx) + (p->z - sz) * (p->z - sz);
                if (!found_sphere || dist_sq < min_dist_sq) {
                    min_dist_sq = dist_sq;
                    closest_sphere_pos[0] = sx;
                    closest_sphere_pos[1] = 0.5f;
                    closest_sphere_pos[2] = sz;
                    found_sphere = true;
                }
            }
        }
    }
    if(found_sphere){
        glEnable(GL_LIGHT1);
        glLightfv(GL_LIGHT1, GL_POSITION, closest_sphere_pos);
    } else {
        glDisable(GL_LIGHT1);
    }

    if (state == STATE_ESCAPING || state == STATE_WON || state == STATE_LOST) {
        glEnable(GL_LIGHT2);
        float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        float y_offset = 0.5f + sin(t * 2.0f + 13 + 13) * 0.1f;
        GLfloat exit_light_pos[] = {13.5f * CUBE_SIZE, y_offset, 13.5f * CUBE_SIZE, 1.0f};
        glLightfv(GL_LIGHT2, GL_POSITION, exit_light_pos);
    } else {
        glDisable(GL_LIGHT2);
    }
}

static void draw_maze(const int maze[MAZE_WIDTH][MAZE_HEIGHT], GameState state) {
    glBindTexture(GL_TEXTURE_2D, wall_texture_id);
    GLfloat wall_specular[] = {0.1, 0.1, 0.1, 1.0};
    GLfloat wall_shininess[] = {10.0};
    glMaterialfv(GL_FRONT, GL_SPECULAR, wall_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, wall_shininess);

    for(int x = 0; x < MAZE_WIDTH; x++){
        for(int z = 0; z < MAZE_HEIGHT; z++){
            if(maze[x][z] == 1 || (x == 13 && z == 13 && state == STATE_PLAYING)){
                glPushMatrix();
                glTranslatef(x * CUBE_SIZE + 0.5f, WALL_HEIGHT / 2.0f, z * CUBE_SIZE + 0.5f);
                glScalef(CUBE_SIZE, WALL_HEIGHT, CUBE_SIZE);
                glColor3f(1.0, 1.0, 1.0);
                draw_textured_cube(wall_texture_id);
                glPopMatrix();
            }
        }
    }
}

static void draw_ceiling_and_floor(const int maze[MAZE_WIDTH][MAZE_HEIGHT], GameState state) {
    GLfloat mat_specular[] = {0.1, 0.1, 0.1, 1.0};
    GLfloat mat_shininess[] = {10.0};
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glBindTexture(GL_TEXTURE_2D, ceiling_texture_id);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_QUADS);
        glNormal3f(0, -1, 0);
        glTexCoord2f(0.0, 0.0); glVertex3f(0, WALL_HEIGHT, 0);
        glTexCoord2f(MAZE_WIDTH, 0.0); glVertex3f(MAZE_WIDTH * CUBE_SIZE, WALL_HEIGHT, 0);
        glTexCoord2f(MAZE_WIDTH, MAZE_HEIGHT); glVertex3f(MAZE_WIDTH * CUBE_SIZE, WALL_HEIGHT, MAZE_HEIGHT * CUBE_SIZE);
        glTexCoord2f(0.0, MAZE_HEIGHT); glVertex3f(0, WALL_HEIGHT, MAZE_HEIGHT * CUBE_SIZE);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, floor_texture_id);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_QUADS);
    for (int x = 0; x < MAZE_WIDTH; x++) {
        for (int z = 0; z < MAZE_HEIGHT; z++) {
            if (x == 13 && z == 13) continue;
            glNormal3f(0, 1, 0);
            glTexCoord2f(0.0, 0.0); glVertex3f(x * CUBE_SIZE, 0, z * CUBE_SIZE);
            glTexCoord2f(0.0, 1.0); glVertex3f(x * CUBE_SIZE, 0, (z + 1) * CUBE_SIZE);
            glTexCoord2f(1.0, 1.0); glVertex3f((x + 1) * CUBE_SIZE, 0, (z + 1) * CUBE_SIZE);
            glTexCoord2f(1.0, 0.0); glVertex3f((x + 1) * CUBE_SIZE, 0, z * CUBE_SIZE);
        }
    }
    glEnd();

    if (state != STATE_PLAYING) {
        draw_exit_hole();
    }
}

static void draw_exit_hole() {
    float x = 13.0f, z = 13.0f;
    float hole_depth = -20.0f;

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.0, 0.0, 0.0);

    glBegin(GL_QUADS);
        glVertex3f(x * CUBE_SIZE, hole_depth, z * CUBE_SIZE);
        glVertex3f((x + 1) * CUBE_SIZE, hole_depth, z * CUBE_SIZE);
        glVertex3f((x + 1) * CUBE_SIZE, hole_depth, (z + 1) * CUBE_SIZE);
        glVertex3f(x * CUBE_SIZE, hole_depth, (z + 1) * CUBE_SIZE);
    glEnd();

    glBegin(GL_QUADS);
        glVertex3f(x * CUBE_SIZE, 0, z * CUBE_SIZE);
        glVertex3f((x + 1) * CUBE_SIZE, 0, z * CUBE_SIZE);
        glVertex3f((x + 1) * CUBE_SIZE, hole_depth, z * CUBE_SIZE);
        glVertex3f(x * CUBE_SIZE, hole_depth, z * CUBE_SIZE);

        glVertex3f(x * CUBE_SIZE, 0, (z + 1) * CUBE_SIZE);
        glVertex3f(x * CUBE_SIZE, hole_depth, (z + 1) * CUBE_SIZE);
        glVertex3f((x + 1) * CUBE_SIZE, hole_depth, (z + 1) * CUBE_SIZE);
        glVertex3f((x + 1) * CUBE_SIZE, 0, (z + 1) * CUBE_SIZE);

        glVertex3f(x * CUBE_SIZE, 0, z * CUBE_SIZE);
        glVertex3f(x * CUBE_SIZE, hole_depth, z * CUBE_SIZE);
        glVertex3f(x * CUBE_SIZE, hole_depth, (z + 1) * CUBE_SIZE);
        glVertex3f(x * CUBE_SIZE, 0, (z + 1) * CUBE_SIZE);

        glVertex3f((x + 1) * CUBE_SIZE, 0, z * CUBE_SIZE);
        glVertex3f((x + 1) * CUBE_SIZE, 0, (z + 1) * CUBE_SIZE);
        glVertex3f((x + 1) * CUBE_SIZE, hole_depth, (z + 1) * CUBE_SIZE);
        glVertex3f((x + 1) * CUBE_SIZE, hole_depth, z * CUBE_SIZE);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

static void draw_textured_cube(GLuint texture_id) {
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glBegin(GL_QUADS);
        glNormal3f(0.0, 0.0, 1.0);
        glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, -0.5, 0.5);
        glTexCoord2f(1.0, 0.0); glVertex3f( 0.5, -0.5, 0.5);
        glTexCoord2f(1.0, 1.0); glVertex3f( 0.5,  0.5, 0.5);
        glTexCoord2f(0.0, 1.0); glVertex3f(-0.5,  0.5, 0.5);

        glNormal3f(0.0, 0.0, -1.0);
        glTexCoord2f(1.0, 0.0); glVertex3f(-0.5, -0.5, -0.5);
        glTexCoord2f(1.0, 1.0); glVertex3f(-0.5,  0.5, -0.5);
        glTexCoord2f(0.0, 1.0); glVertex3f( 0.5,  0.5, -0.5);
        glTexCoord2f(0.0, 0.0); glVertex3f( 0.5, -0.5, -0.5);

        glNormal3f(0.0, 1.0, 0.0);
        glTexCoord2f(0.0, 1.0); glVertex3f(-0.5, 0.5, -0.5);
        glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, 0.5,  0.5);
        glTexCoord2f(1.0, 0.0); glVertex3f( 0.5, 0.5,  0.5);
        glTexCoord2f(1.0, 1.0); glVertex3f( 0.5, 0.5, -0.5);

        glNormal3f(0.0, -1.0, 0.0);
        glTexCoord2f(1.0, 1.0); glVertex3f(-0.5, -0.5, -0.5);
        glTexCoord2f(0.0, 1.0); glVertex3f( 0.5, -0.5, -0.5);
        glTexCoord2f(0.0, 0.0); glVertex3f( 0.5, -0.5,  0.5);
        glTexCoord2f(1.0, 0.0); glVertex3f(-0.5, -0.5,  0.5);

        glNormal3f(1.0, 0.0, 0.0);
        glTexCoord2f(1.0, 0.0); glVertex3f(0.5, -0.5, -0.5);
        glTexCoord2f(1.0, 1.0); glVertex3f(0.5,  0.5, -0.5);
        glTexCoord2f(0.0, 1.0); glVertex3f(0.5,  0.5,  0.5);
        glTexCoord2f(0.0, 0.0); glVertex3f(0.5, -0.5,  0.5);

        glNormal3f(-1.0, 0.0, 0.0);
        glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, -0.5, -0.5);
        glTexCoord2f(1.0, 0.0); glVertex3f(-0.5, -0.5,  0.5);
        glTexCoord2f(1.0, 1.0); glVertex3f(-0.5,  0.5,  0.5);
        glTexCoord2f(0.0, 1.0); glVertex3f(-0.5,  0.5, -0.5);
    glEnd();
}

static void draw_collectibles(const int maze[MAZE_WIDTH][MAZE_HEIGHT]) {
    GLfloat s[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat n[] = {128.0};
    GLfloat e[] = {1.0, 0.1, 0.1, 1.0};
    glMaterialfv(GL_FRONT, GL_SPECULAR, s);
    glMaterialfv(GL_FRONT, GL_SHININESS, n);
    glMaterialfv(GL_FRONT, GL_EMISSION, e);
    glBindTexture(GL_TEXTURE_2D, sphere_texture_id);
    glColor3f(1.0, 1.0, 1.0);

    for(int x = 0; x < MAZE_WIDTH; x++) {
        for(int z = 0; z < MAZE_HEIGHT; z++) {
            if(maze[x][z] == 2) {
                glPushMatrix();
                float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
                float y = 0.5f + sin(t * 2.0f + x + z) * 0.1f;
                glTranslatef(x * CUBE_SIZE + 0.5f, y, z * CUBE_SIZE + 0.5f);
                gluSphere(sphere_quadric, 0.12, 16, 16);
                glPopMatrix();
            }
        }
    }
    GLfloat ne[] = {0.0, 0.0, 0.0, 1.0};
    glMaterialfv(GL_FRONT, GL_EMISSION, ne);
}

static void draw_exit_sphere(GameState state) {
    if (state != STATE_ESCAPING && state != STATE_WON && state != STATE_LOST) {
        return;
    }

    GLfloat mat_specular[] = {0.1, 0.1, 0.1, 1.0};
    GLfloat mat_shininess[] = {10.0};
    GLfloat mat_emission[] = {1.0, 0.5, 0.5, 1.0};
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, door_texture_id);
    glColor3f(0.9, 0.5, 0.5);

    glPushMatrix();
    float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float y_offset = 0.5f + sin(t * 2.0f + 13 + 13) * 0.1f;
    glTranslatef(13.5f * CUBE_SIZE, y_offset, 13.5f * CUBE_SIZE);

    gluSphere(sphere_quadric, 0.15, 16, 16);

    glPopMatrix();

    GLfloat no_emission[] = {0.0, 0.0, 0.0, 1.0};
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
}


GLuint render_get_texture_id(const char* name) {
    if (strcmp(name, "menu_bg") == 0) {
        return menu_bg_texture_id;
    }
    else if (strcmp(name, "loser_bg") == 0) {
        return loser_bg_texture_id;
    }
    else if (strcmp(name, "win_bg") == 0) {
        return win_bg_texture_id;
    }
    return 0;
}
