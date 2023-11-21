#pragma once

typedef struct _ThumbImg {
    unsigned int width_height;
    unsigned int size;
    unsigned char* data;
} ThumbImg;

ThumbImg fetch_gcode_thumb(const char* gcode_filename);