#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_TGA
#include "load_map.h"

typedef struct {
    char texture[30];
    char heights[30];
} surface_info_t;

surface_info_t files[] = {{
    .texture = "/rd/images/texture/C1W.tga",
    .heights  = "/rd/images/height/D1.tga"
}};

uint16_t* separated_RGB_to_RGB565(uint8_t *rgb, int image_width, int image_height){
    uint16_t *result = (uint16_t *) malloc(image_width * image_height * sizeof(uint16_t));
    for (int i = 0; i < image_width * image_height; i++){
        result[i] = PACK_PIXEL(rgb[i * 3], rgb[i * 3 + 1], rgb[i * 3 + 2]);
    }
    return result;
}

uint16_t* loadmap_get_texture(uint8_t index){
    int image_width, image_height, comp;

    uint8_t *separated_RGB = stbi_load(files[index].texture, &image_width, &image_height, &comp, STBI_rgb);
    uint16_t *texture_map = separated_RGB_to_RGB565(separated_RGB, image_width, image_height);

    return texture_map;
}

uint8_t* loadmap_get_heights(uint8_t index){
    int image_width, image_height, comp;

    uint8_t *height_map = stbi_load(files[index].heights, &image_width, &image_height, &comp, STBI_grey);

    return height_map;
}

