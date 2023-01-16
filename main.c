#include <kos.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "display.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_TGA
#include "stb_image.h"

//TODO list
//Implement delta time
//Fix gif.h unaligned memory access

//Constants
#define MAP_N 1024
#define SCALE_FACTOR 70.0


// Buffers for Heightmap and pixelMap
uint8_t* heightMap = NULL;   // Buffer to hold height values in grayscale
uint16_t* pixelMap = NULL;   // Buffer to hold pixel color values in RGB565


// Camera struct type declaration
typedef struct {
    float x;         // x position on the map
    float y;         // y position on the map
    float height;    // height of the camera
    float horizon;   // offset of the horizon position (looking up-down)
    float zfar;      // distance of the camera looking forward
    float angle;     // camera angle (radians, clockwise)
} camera_t;

camera_t camera = {
    .x       = 512.0,
    .y       = 512.0,
    .height  = 70.0,
    .horizon = 60.0,
    .zfar    = 600.0,
    .angle   = 1.5 * 3.141592 // (= 270 deg)
};

// Handle controller input
int processInput() {
    maple_device_t *cont;
    cont_state_t *state;
    
    cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);

    /* Check key status */
    state = (cont_state_t *)maple_dev_status(cont);

    if(!state) {
        printf("Error reading controller\n");
        return -1;
    }
    if(state->buttons & CONT_START){
        return 0;
    }
    if(state->buttons & CONT_A){
        camera.height--;
    }
    if(state->buttons & CONT_B){
        camera.horizon += 1.5;
    }
    if((state->buttons & CONT_X)) {
        camera.horizon -= 1.5;
    }
    if((state->buttons & CONT_Y)) {
        camera.height++;
    }
    if(state->buttons & CONT_DPAD_UP){
        camera.x += fcos(camera.angle);
        camera.y += fsin(camera.angle);
    }
    if(state->buttons & CONT_DPAD_DOWN){
        camera.x -= fcos(camera.angle);
        camera.y -= fsin(camera.angle);
    }
    if(state->buttons & CONT_DPAD_LEFT){
        camera.angle -= 0.02;
    }
    if(state->buttons & CONT_DPAD_RIGHT){
        camera.angle += 0.02;
    }
    if(state->ltrig)
        return 0;

    if(state->rtrig)
        return 0;

    return 1;
}

//Update Game State
void updateGameState(){
    //TODO remover esses calculos da main
    float sh4FSCARadianSine = fsin(camera.angle);
    float sh4FSCARadianCosine = fcos(camera.angle);

    // Left-most point of the FOV
    float plx = sh4FSCARadianCosine * camera.zfar + sh4FSCARadianSine * camera.zfar;
    float ply = sh4FSCARadianSine * camera.zfar - sh4FSCARadianCosine * camera.zfar;

    // Right-most point of the FOV
    float prx = sh4FSCARadianCosine * camera.zfar - sh4FSCARadianSine * camera.zfar;
    float pry = sh4FSCARadianSine * camera.zfar + sh4FSCARadianCosine * camera.zfar;

    // Loop 320 rays from left to right
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        float deltax = (plx + (prx - plx) / SCREEN_WIDTH * i) / camera.zfar;
        float deltay = (ply + (pry - ply) / SCREEN_WIDTH * i) / camera.zfar;

        // Ray (x,y) coords
        float rx = camera.x;
        float ry = camera.y;

        // Store the tallest projected height per-ray
        float tallestheight = SCREEN_HEIGHT;

        // Loop all depth units until the zfar distance limit
        for (int z = 1; z < camera.zfar; z++) {
            rx += deltax;
            ry += deltay;

            // Find the offset that we have to go and fetch values from the heightMap
            int mapoffset = ((MAP_N * ((int)(ry) & (MAP_N - 1))) + ((int)(rx) & (MAP_N - 1)));

            // Project height values and find the height on-screen
            int projheight = (int)((camera.height - heightMap[mapoffset]) / z * SCALE_FACTOR + camera.horizon);

            // Only draw pixels if the new projected height is taller than the previous tallest height
            if (projheight < tallestheight) {
                // Draw pixels from previous max-height until the new projected height
                for (int y = projheight; y < tallestheight; y++) {
                    DRAW_PIXEL(i, y, pixelMap[mapoffset]);
                }
                tallestheight = projheight;
            }
        }
    }
}

uint16_t* RGB_channels_to_pixelMapRGB565(uint8_t *rgb, int image_width, int image_height){
    uint16_t *result = (uint16_t *) malloc(image_width * image_height * sizeof(uint16_t));
    for (int i = 0; i < image_width * image_height; i++){
        result[i] = PACK_PIXEL(rgb[i * 3], rgb[i * 3 + 1], rgb[i * 3 + 2]);
    }
    return result;
}

char fpsText[20];
void render(){
    bfont_draw_str(backbuffer, SCREEN_WIDTH, 0, fpsText);
    dis_flipBuffer();
    dis_clearBackBuffer(0, 0x82, 0xFF);
}

/* romdisk */
extern uint8 romdisk_boot[];
KOS_INIT_ROMDISK(romdisk_boot);

int main(void) {
    int quit = 0;

    //init kos
    pvr_init_defaults();

    //initialize display
    dis_initializeDisplay();

    int image_width, image_height, comp;

    uint8_t *RGBMap = stbi_load("/rd/images/texture/C1W.tga", &image_width, &image_height, &comp, STBI_rgb);
    pixelMap = RGB_channels_to_pixelMapRGB565(RGBMap, image_width, image_height); //TODO melhorar isso aqui

    heightMap = stbi_load("/rd/images/height/D1.tga", &image_width, &image_height, &comp, STBI_grey);

    //FPS Counter
    int numberOfFrames = 0;
    uint64 startTime = timer_ms_gettime64();
    uint32 currentTime = 0;

    //Main Loop
    while(!quit) 
    {
        //FPS Counter
        currentTime = timer_ms_gettime64();
        if((currentTime - startTime) > 1000){
            double fps = 1000.0 * (double)numberOfFrames / (double)(currentTime - startTime);
            sprintf(fpsText, "FPS: %.2f", fps);
            numberOfFrames = 0;
            startTime = currentTime;
        }
        numberOfFrames++;

        //Process controller input
        processInput();

        //Update Game State
        updateGameState();

        //render
        render();

    }

    return 0;
}
