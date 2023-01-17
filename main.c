#include <kos.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "display.h"
#include "load_map.h"

//TODO list
//Implement delta time
//Fix gif.h unaligned memory access

//Constants
#define MAP_N 1024
#define SCALE_FACTOR 70.0


// Buffers for height_map and texture_map
uint8_t* height_map = NULL;   // Buffer to hold height values in grayscale
uint16_t* texture_map = NULL;   // Buffer to hold pixel color values in RGB565


// Camera struct type declaration
typedef struct {
    float x;         // x position on the map
    float y;         // y position on the map
    float height;    // height of the camera
    float horizon;   // offset of the horizon position (looking up-down)
    float zfar;      // distance of the camera looking forward
    float angle;     // camera angle (radians, clockwise)
} camera_t;

// Camera definition
camera_t camera = {
    .x       = 512.0,
    .y       = 512.0,
    .height  = 70.0,
    .horizon = 60.0,
    .zfar    = 600.0,
    .angle   = 1.5 * 3.141592 // (= 270 deg)
};

// Handle controller input
int process_input() {
    maple_device_t *cont;
    cont_state_t *state;
    
    cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);

    // Check key status
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
    if(state->ltrig){
        return 0;
    }
    if(state->rtrig){
        return 0;
    }
    return 1;
}

//Update Game State
void update_game_state(){
    //TODO remover esses calculos da main
    float sin_angle = sin(camera.angle);
    float cos_angle = cos(camera.angle);

    // Left-most point of the FOV
    float plx = cos_angle * camera.zfar + sin_angle * camera.zfar;
    float ply = sin_angle * camera.zfar - cos_angle * camera.zfar;

    // Right-most point of the FOV
    float prx = cos_angle * camera.zfar - sin_angle * camera.zfar;
    float pry = sin_angle * camera.zfar + cos_angle * camera.zfar;

    // Loop SCREEN_WIDTH rays from left to right
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        float delta_x = (plx + (prx - plx) / SCREEN_WIDTH * i) / camera.zfar;
        float delta_y = (ply + (pry - ply) / SCREEN_WIDTH * i) / camera.zfar;

        // Ray (x,y) coords
        float rx = camera.x;
        float ry = camera.y;

        // Store the tallest projected height per-ray
        float tallest_height = SCREEN_HEIGHT;

        // Loop all depth units until the zfar distance limit
        for (int z = 1; z < camera.zfar; z++) {
            rx += delta_x;
            ry += delta_y;

            // Find the offset that we have to go and fetch values from the height_map
            int map_offset = ((MAP_N * ((int)(ry) & (MAP_N - 1))) + ((int)(rx) & (MAP_N - 1)));

            // Project height values and find the height on-screen
            int proj_height = (int)((camera.height - height_map[map_offset]) / z * SCALE_FACTOR + camera.horizon);

            // Only draw pixels if the new projected height is taller than the previous tallest height
            if (proj_height < tallest_height) {
                // Draw pixels from previous max-height until the new projected height
                for (int y = proj_height; y < tallest_height; y++) {
                    DRAW_PIXEL(i, y, texture_map[map_offset]);
                }
                tallest_height = proj_height;
            }
        }
    }
}

char screen_text[20];
void render(){
    bfont_draw_str(backbuffer, SCREEN_WIDTH, 0, screen_text);
    display_flip_framebuffer();
    display_clear_backbuffer(0, 0x82, 0xFF);
}

/* romdisk */
extern uint8 romdisk_boot[];
KOS_INIT_ROMDISK(romdisk_boot);

int main(void) {
    int quit = 0;

    //init kos
    pvr_init_defaults();

    //initialize display
    display_initialize();

    //initialize level
    texture_map = loadmap_get_texture(0);
    height_map = loadmap_get_heights(0);

    //FPS Counter
    int number_of_frames = 0;
    uint64 start_time = timer_ms_gettime64();
    uint32 current_time = 0;

    //Main Loop
    while(!quit) 
    {
        //FPS Counter
        current_time = timer_ms_gettime64();
        if((current_time - start_time) > 1000){
            double fps = 1000.0 * (double)number_of_frames / (double)(current_time - start_time);
            sprintf(screen_text, "FPS: %.2f", fps);
            number_of_frames = 0;
            start_time = current_time;
        }
        number_of_frames++;

        //Process controller input
        process_input();

        //Update Game State
        update_game_state();

        //render
        render();

    }

    return 0;
}