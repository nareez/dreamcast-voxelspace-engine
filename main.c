#include <kos.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "gif.h"
#include "display.h"

//Constants
#define MAP_N 1024
#define SCALE_FACTOR 70.0

///////////////////////////////////////////////////////////////////////////////
// Buffers for Heightmap and Colormap
///////////////////////////////////////////////////////////////////////////////
uint8_t* heightmap = NULL;   // Buffer/array to hold height values (1024*1024)
uint8_t* colormap  = NULL;   // Buffer/array to hold color values  (1024*1024)
uint16_t* pixelmap = NULL;   // Buffer/array to hold pixel color values (1024*1024)

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

//TODO ver onde colocar essa function
// Convert palleted colors to pixel color map
uint16_t* gifPalletedToDirectColors(uint8_t* colormap, uint8_t* palette, int gifWidth, int height){
    uint16_t* pixelmap = (uint16_t*) malloc(sizeof(uint16_t) * gifWidth * height);
    int brightnessLevel = 3;
    for(int i = 0; i < gifWidth*height; i++){
        pixelmap[i] = PACK_PIXEL(palette[3 * colormap[i] + 0] * brightnessLevel
                                ,palette[3 * colormap[i] + 1] * brightnessLevel
                                ,palette[3 * colormap[i] + 2] * brightnessLevel);
    }
    return pixelmap;
}

//get time passed since power on in miliseconds
//TODO tirar essa função da main
uint32_t getTimeInMilis(){
    uint32_t seconds;
    uint32_t miliseconds;
    timer_ms_gettime(&seconds, &miliseconds);
    return (seconds * 1000) + miliseconds;
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

    // Declare an array to hold the max. number of possible colors (*3 for RGB)
    uint8_t palette[256 * 3];
    int palsize;

    // Load the colormap, heightmap, and palette from the external GIF files
    int gifWidth;
    int gifHeight;
    colormap = loadgif("/rd/gif/map0.color.gif", &gifWidth, &gifHeight, &palsize, palette);
    heightmap = loadgif("/rd/gif/map0.height.gif", NULL, NULL, NULL, NULL);
    // TODO segregar o tratamento de cor em outra lib
    // Convert palleted colors to pixel color map
    pixelmap = gifPalletedToDirectColors(colormap, palette, gifWidth, gifHeight);

    //FPS Counter
    int numberOfFrames = 0;
    uint32 startTime = getTimeInMilis();
    uint32 currentTime = 0;
    char fpsText[20];

    //Main Loop
    while(!quit) 
    {
        //FPS Counter
        currentTime = getTimeInMilis();
        if((currentTime - startTime) > 1000){
            double fps = 1000.0 * (double)numberOfFrames / (double)(currentTime - startTime);
            sprintf(fpsText, "FPS: %.2f", fps);
            numberOfFrames = 0;
            startTime = currentTime;
        }
        numberOfFrames++;

        //Process controller input
        processinput();

        //TODO remover esses calculos da main
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
        
        bfont_draw_str(backbuffer, SCREEN_WIDTH, 0, fpsText);

        dis_flipBuffer();
        dis_clearBackBuffer(0, 0x82, 0xFF);
    }

    return 0;
}