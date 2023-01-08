#include <kos.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "gif.h"
#include "display.h"

//TODOs
//Create an FPS counter

//Constants
//TODO move screen info to display.c
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 320
#define MAP_N 1024
#define SCALE_FACTOR 70.0


///////////////////////////////////////////////////////////////////////////////
// Buffers for Heightmap and Colormap
///////////////////////////////////////////////////////////////////////////////
uint8_t* heightmap = NULL;   // Buffer/array to hold height values (1024*1024)
uint8_t* colormap  = NULL;   // Buffer/array to hold color values  (1024*1024)
uint16_t* pixelmap = NULL;

///////////////////////////////////////////////////////////////////////////////
// Camera struct type declaration
///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
// Handle keyboard input (TODO)
///////////////////////////////////////////////////////////////////////////////
int processinput() {
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
        camera.x += cos(camera.angle);
        camera.y += sin(camera.angle);
    }

    if(state->buttons & CONT_DPAD_DOWN){
        camera.x -= cos(camera.angle);
        camera.y -= sin(camera.angle);
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

//TODO jogar isso no GIF.H, usar width e height ao invez de usar 1024*1024
void gifPalletedToDirectColors(uint8_t* colormap, uint8_t* palette, uint16_t* pixelmap){
    for(int i = 0; i < 1024*1024; i++){
        pixelmap[i] = PACK_PIXEL(palette[3 * colormap[i] + 0] * 3
                                ,palette[3 * colormap[i] + 1] * 3
                                ,palette[3 * colormap[i] + 2] * 3);
    }
}

/* romdisk */
extern uint8 romdisk_boot[];
KOS_INIT_ROMDISK(romdisk_boot);

int main(void) {
    int quit = 0;

    //init kos
    pvr_init_defaults();

    //set our video mode
    vid_set_mode(DM_320x240, PM_RGB565);

    //initialize software double buffer
    dis_initializeDoublebuffer(SCREEN_WIDTH*SCREEN_HEIGHT);

    // Declare an array to hold the max. number of possible colors (*3 for RGB)
    uint8_t palette[256 * 3];
    int palsize;

    // Load the colormap, heightmap, and palette from the external GIF files
    // TODO Melhorar para pegar o tamanho do gif dinamicamente
    // TODO segregar o tratamento de cor em outra lib
    colormap = loadgif("/rd/gif/map0.color.gif", NULL, NULL, &palsize, palette);
    heightmap = loadgif("/rd/gif/map0.height.gif", NULL, NULL, NULL, NULL);
    pixelmap = (uint16_t*) malloc(sizeof(uint16_t) * 1024 * 1024);
    gifPalletedToDirectColors(colormap, palette, pixelmap);

    //Main Loop
    while(!quit) 
    {
        processinput();

        float sinangle = sin(camera.angle);
        float cosangle = cos(camera.angle);

        // Left-most point of the FOV
        float plx = cosangle * camera.zfar + sinangle * camera.zfar;
        float ply = sinangle * camera.zfar - cosangle * camera.zfar;

        // Right-most point of the FOV
        float prx = cosangle * camera.zfar - sinangle * camera.zfar;
        float pry = sinangle * camera.zfar + cosangle * camera.zfar;

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

                // Find the offset that we have to go and fetch values from the heightmap
                int mapoffset = ((MAP_N * ((int)(ry) & (MAP_N - 1))) + ((int)(rx) & (MAP_N - 1)));

                // Project height values and find the height on-screen
                int projheight = (int)((camera.height - heightmap[mapoffset]) / z * SCALE_FACTOR + camera.horizon);

                // Only draw pixels if the new projected height is taller than the previous tallest height
                if (projheight < tallestheight) {
                    // Draw pixels from previous max-height until the new projected height
                    for (int y = projheight; y < tallestheight; y++) {
                        DRAW_PIXEL(i, y, pixelmap[mapoffset]);
                    }
                    tallestheight = projheight;
                }
            }
        }
        dis_flipBuffer();
        dis_clearBackBuffer(0, 0x82, 0xFF);
    }

    return 0;
}