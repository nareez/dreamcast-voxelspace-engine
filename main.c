#include <kos.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "gif.h"

//Constants
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define MAP_N 1024
#define SCALE_FACTOR 70.0

//Macros
#define PACK_PIXEL(r, g, b) ( ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3) )
#define DRAW_PIXEL(x, y, color) \
	if((x >= 0) && (x < SCREEN_HEIGHT) && (y >= 0) && (y < SCREEN_WIDTH)) \
		vram_s[(y * SCREEN_WIDTH) + x] = color;


///////////////////////////////////////////////////////////////////////////////
// Buffers for Heightmap and Colormap
///////////////////////////////////////////////////////////////////////////////
uint8_t* heightmap = NULL;   // Buffer/array to hold height values (1024*1024)
uint8_t* colormap  = NULL;   // Buffer/array to hold color values  (1024*1024)

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
        camera.angle -= 0.01;
    }

    if(state->buttons & CONT_DPAD_RIGHT){
        camera.angle += 0.01;
    }

    if(state->ltrig)
        return 0;

    if(state->rtrig)
        return 0;

    return 1;
}

int clearScreen(){
	int pixelNumber;
	for(pixelNumber = 0; pixelNumber < SCREEN_HEIGHT*SCREEN_WIDTH; pixelNumber++){
        vram_s[pixelNumber] = PACK_PIXEL(36, 36, 56);
	}
	
	return 1;
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

    // Declare an array to hold the max. number of possible colors (*3 for RGB)
    uint8_t palette[256 * 3];
    int palsize;

    // Load the colormap, heightmap, and palette from the external GIF files
    colormap = loadgif("/rd/gif/map0.color.gif", NULL, NULL, &palsize, palette);
    heightmap = loadgif("/rd/gif/map0.height.gif", NULL, NULL, NULL, NULL);

    //Main Loop
    while(!quit) 
    {
        vid_waitvbl();
        processinput();

        float sinangle = sin(camera.angle);
        float cosangle = cos(camera.angle);

        // Left-most point of the FOV
        float plx = cosangle * camera.zfar + sinangle * camera.zfar;
        float ply = sinangle * camera.zfar - cosangle * camera.zfar;

        // Right-most point of the FOV
        float prx = cosangle * camera.zfar - sinangle * camera.zfar;
        float pry = sinangle * camera.zfar + cosangle * camera.zfar;

        clearScreen(36,36,56);
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
                        uint8_t color_index = colormap[mapoffset];
                        DRAW_PIXEL(i, y, PACK_PIXEL(palette[3 * color_index + 0]
                                                   ,palette[3 * color_index + 1]
                                                   ,palette[3 * color_index + 2]));
                    }
                    tallestheight = projheight;
                }
            }
        }
    }

    return 0;
}