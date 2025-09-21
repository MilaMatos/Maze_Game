#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include "render.h"
#include "texture.h"
#include "player.h"

// Variáveis de textura
static GLuint wall_texture_id, floor_texture_id, ceiling_texture_id;
static GLuint door_texture_id, sphere_texture_id;
static GLUquadric* sphere_quadric = NULL;

// Protótipos de funções locais
static void draw_maze(const int maze[MAZE_WIDTH][MAZE_HEIGHT], GameState state);
static void draw_ceiling_and_floor();
static void draw_collectibles(const int maze[MAZE_WIDTH][MAZE_HEIGHT]);
static void draw_textured_cube(GLuint texture_id);
static void setup_lighting();
static void lighting_update_dynamic(const int maze[MAZE_WIDTH][MAZE_HEIGHT]);

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

    if (wall_texture_id == 0 || floor_texture_id == 0 || ceiling_texture_id == 0 ||
        door_texture_id == 0 || sphere_texture_id == 0) {
        return false;
    }

    sphere_quadric = gluNewQuadric();
    gluQuadricTexture(sphere_quadric, GL_TRUE);

    setup_lighting();
    return true;
}

void render_start_frame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
    float look_y = 0.5f + sin(p->pitch);
    float look_z = p->z - sin(p->angle) * look_horizontal_dist;

    gluLookAt(p->x, 0.5, p->z, look_x, look_y, look_z, 0.0, 1.0, 0.0);

    lighting_update_dynamic(maze);

    draw_ceiling_and_floor();
    draw_maze(maze, state);

    glBindTexture(GL_TEXTURE_2D, 0);
    draw_collectibles(maze);
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

static void setup_lighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    GLfloat global_ambient[] = {0.08, 0.08, 0.06, 1.0};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    glDisable(GL_LIGHT0);

    glEnable(GL_LIGHT1);
    GLfloat sphere_diffuse[] = {1.0, 0.1, 0.1, 1.0};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, sphere_diffuse);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.3f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.1f);

    glEnable(GL_LIGHT2);
    GLfloat exit_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat exit_pos[] = {13.5f * CUBE_SIZE, WALL_HEIGHT, 13.5f * CUBE_SIZE, 1.0f};
    glLightfv(GL_LIGHT2, GL_DIFFUSE, exit_diffuse);
    glLightfv(GL_LIGHT2, GL_POSITION, exit_pos);
    glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.3f);
    glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.1f);
}

static void lighting_update_dynamic(const int maze[MAZE_WIDTH][MAZE_HEIGHT]) {
    const Player* p = player_get();

    float min_dist_sq = -1.0f;
    float closest_sphere_pos[4] = {0,0,0,1};
    bool found_sphere = false;

    for (int x = 0; x < MAZE_WIDTH; x++) {
        for (int z = 0; z < MAZE_HEIGHT; z++) {
            if (maze[x][z] == 2) {
                float sx = x * CUBE_SIZE + 0.5f;
                float sz = z * CUBE_SIZE + 0.5f;
                float dist_sq = (p->x - sx)*(p->x - sx) + (p->z - sz)*(p->z - sz);
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

    if (found_sphere) {
        glEnable(GL_LIGHT1);
        glLightfv(GL_LIGHT1, GL_POSITION, closest_sphere_pos);
    } else {
        glDisable(GL_LIGHT1);
    }
}

static void draw_maze(const int maze[MAZE_WIDTH][MAZE_HEIGHT], GameState state) {
    glBindTexture(GL_TEXTURE_2D, wall_texture_id);
    GLfloat wall_specular[] = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat wall_shininess[] = { 10.0 };
    glMaterialfv(GL_FRONT, GL_SPECULAR, wall_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, wall_shininess);

    for (int x = 0; x < MAZE_WIDTH; x++) {
        for (int z = 0; z < MAZE_HEIGHT; z++) {
            if (maze[x][z] == 1 || (maze[x][z] == 9 && state == STATE_PLAYING)) {
                glPushMatrix();
                glTranslatef(x * CUBE_SIZE + 0.5f, WALL_HEIGHT / 2.0f, z * CUBE_SIZE + 0.5f);
                glScalef(CUBE_SIZE, WALL_HEIGHT, CUBE_SIZE);
                glColor3f(1.0, 1.0, 1.0);
                draw_textured_cube(wall_texture_id);
                glPopMatrix();
            } else if (maze[x][z] == 9 && state != STATE_PLAYING) {
                glPushMatrix();
                glTranslatef(x * CUBE_SIZE + 0.5f, 0.00001f, z * CUBE_SIZE + 0.5f);

                GLfloat finish_emission[] = {0.5, 0.1, 0.1, 1.0};
                glMaterialfv(GL_FRONT, GL_EMISSION, finish_emission);

                glBindTexture(GL_TEXTURE_2D, door_texture_id);
                glColor3f(1.0, 1.0, 1.0);

                glBegin(GL_QUADS);
                    glNormal3f(0, 1, 0);
                    glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, 0, 0.5);
                    glTexCoord2f(1.0, 0.0); glVertex3f( 0.5, 0, 0.5);
                    glTexCoord2f(1.0, 1.0); glVertex3f( 0.5, 0, -0.5);
                    glTexCoord2f(0.0, 1.0); glVertex3f(-0.5, 0, -0.5);
                glEnd();

                GLfloat no_emission[] = {0.0, 0.0, 0.0, 1.0};
                glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);

                glPopMatrix();
            }
        }
    }
}

static void draw_ceiling_and_floor() {
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
        glNormal3f(0, 1, 0);
        glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0);
        glTexCoord2f(0.0, MAZE_HEIGHT); glVertex3f(0, 0, MAZE_HEIGHT * CUBE_SIZE);
        glTexCoord2f(MAZE_WIDTH, MAZE_HEIGHT); glVertex3f(MAZE_WIDTH * CUBE_SIZE, 0, MAZE_HEIGHT * CUBE_SIZE);
        glTexCoord2f(MAZE_WIDTH, 0.0); glVertex3f(MAZE_WIDTH * CUBE_SIZE, 0, 0);
    glEnd();
}

static void draw_textured_cube(GLuint texture_id) {
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glBegin(GL_QUADS);
        glNormal3f(0.0, 0.0, 1.0); glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, -0.5, 0.5);
        glTexCoord2f(1.0, 0.0); glVertex3f( 0.5, -0.5, 0.5); glTexCoord2f(1.0, 1.0); glVertex3f( 0.5,  0.5, 0.5);
        glTexCoord2f(0.0, 1.0); glVertex3f(-0.5,  0.5, 0.5);
        glNormal3f(0.0, 0.0, -1.0); glTexCoord2f(1.0, 0.0); glVertex3f(-0.5, -0.5, -0.5);
        glTexCoord2f(1.0, 1.0); glVertex3f(-0.5,  0.5, -0.5); glTexCoord2f(0.0, 1.0); glVertex3f( 0.5,  0.5, -0.5);
        glTexCoord2f(0.0, 0.0); glVertex3f( 0.5, -0.5, -0.5);
        glNormal3f(0.0, 1.0, 0.0); glTexCoord2f(0.0, 1.0); glVertex3f(-0.5, 0.5, -0.5);
        glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, 0.5,  0.5); glTexCoord2f(1.0, 0.0); glVertex3f( 0.5, 0.5,  0.5);
        glTexCoord2f(1.0, 1.0); glVertex3f( 0.5, 0.5, -0.5);
        glNormal3f(0.0, -1.0, 0.0); glTexCoord2f(1.0, 1.0); glVertex3f(-0.5, -0.5, -0.5);
        glTexCoord2f(0.0, 1.0); glVertex3f( 0.5, -0.5, -0.5); glTexCoord2f(0.0, 0.0); glVertex3f( 0.5, -0.5,  0.5);
        glTexCoord2f(1.0, 0.0); glVertex3f(-0.5, -0.5,  0.5);
        glNormal3f(1.0, 0.0, 0.0); glTexCoord2f(1.0, 0.0); glVertex3f(0.5, -0.5, -0.5);
        glTexCoord2f(1.0, 1.0); glVertex3f(0.5,  0.5, -0.5); glTexCoord2f(0.0, 1.0); glVertex3f(0.5,  0.5,  0.5);
        glTexCoord2f(0.0, 0.0); glVertex3f(0.5, -0.5,  0.5);
        glNormal3f(-1.0, 0.0, 0.0); glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, -0.5, -0.5);
        glTexCoord2f(1.0, 0.0); glVertex3f(-0.5, -0.5,  0.5); glTexCoord2f(1.0, 1.0); glVertex3f(-0.5,  0.5,  0.5);
        glTexCoord2f(0.0, 1.0); glVertex3f(-0.5,  0.5, -0.5);
    glEnd();
}

static void draw_collectibles(const int maze[MAZE_WIDTH][MAZE_HEIGHT]) {
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat mat_shininess[] = {128.0};
    GLfloat mat_emission[] = {1.0, 0.1, 0.1, 1.0};
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);

    glBindTexture(GL_TEXTURE_2D, sphere_texture_id);
    glColor3f(1.0, 1.0, 1.0);

    for(int x = 0; x < MAZE_WIDTH; x++) {
        for(int z = 0; z < MAZE_HEIGHT; z++) {
            if(maze[x][z] == 2) {
                glPushMatrix();
                float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
                float y_offset = 0.5f + sin(t * 2.0f + x + z) * 0.1f;
                glTranslatef(x * CUBE_SIZE + 0.5f, y_offset, z * CUBE_SIZE + 0.5f);
                gluSphere(sphere_quadric, 0.12, 16, 16);
                glPopMatrix();
            }
        }
    }
    GLfloat no_emission[] = {0.0, 0.0, 0.0, 1.0};
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
}
// static void draw_player_model() { ... }
