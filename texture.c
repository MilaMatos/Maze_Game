#include "texture.h"
#include <stdio.h>
#include <stdlib.h>

GLuint load_texture_bmp(const char* filename) {
    GLuint texture_id;
    unsigned char header[54];
    unsigned int data_pos, width, height, image_size;
    unsigned char* data;

    FILE* file = fopen(filename, "rb");
    if (!file) { printf("Image could not be opened: %s\n", filename); return 0; }
    if (fread(header, 1, 54, file) != 54) { fclose(file); printf("Not a correct BMP file: %s\n", filename); return 0; }
    if (header[0] != 'B' || header[1] != 'M') { fclose(file); printf("Not a correct BMP file: %s\n", filename); return 0; }

    data_pos = *(int*)&(header[0x0A]);
    width = *(int*)&(header[0x12]);
    height = *(int*)&(header[0x16]);
    image_size = *(int*)&(header[0x22]);

    if (image_size == 0) image_size = width * height * 3;
    if (data_pos == 0) data_pos = 54;

    data = (unsigned char*)malloc(image_size);
    fseek(file, data_pos, SEEK_SET);
    fread(data, 1, image_size, file);
    fclose(file);

    for (unsigned int i = 0; i < image_size; i += 3) {
        unsigned char temp = data[i];
        data[i] = data[i+2];
        data[i+2] = temp;
    }

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return texture_id;
}
