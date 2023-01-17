#ifndef load_map_h
#define load_map_h

#include <stdlib.h>
#include <stdint.h>
#include "display.h"
#include "stb_image.h"

uint16_t* loadmap_get_texture(uint8_t index);
uint8_t* loadmap_get_heights(uint8_t index);

#endif