#include <kos.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "display.h"
#include "load_map.h"

//TODO list
//Implement delta time
//Change all int to _t
//use SH4 fast math

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
    uint16_t height;    // height of the camera
    float horizon;   // offset of the horizon position (looking up-down)
    uint16_t zfar;      // distance of the camera looking forward
    float angle;     // camera angle (radians, clockwise)
} camera_t;

// Camera definition
camera_t camera = {
    .x       = 512.0,
    .y       = 512.0,
    .height  = 70,
    .horizon = 60.0,
    .zfar    = 600,
    .angle   = 5.0 //1.5 * 3.141592 // (= 270 deg)
};

//TODO bug do sin(0);
float sintab[314];
void init_sintab(){
    for (int i = 0; i <= 628; i+=2){
        sintab[i/2] = sin(i/100.0);
    }
}

float sine(float angle){
    return sintab[(int)(angle*100.0/2.0)];
}

float costab[314];
void init_costab(){
    for (int i = 0; i <= 628; i+=2){
        costab[i/2] = cos(i/100.0);
    }
}

float cosine(float angle){
    return costab[(int)(angle*100.0/2.0)];
}

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
        if(camera.height > 10){
            camera.height--;
        }
    }
    if(state->buttons & CONT_B){
        camera.horizon += 1.5;
    }
    if((state->buttons & CONT_X)) {
        camera.horizon -= 1.5;
    }
    if((state->buttons & CONT_Y)) {
        if(camera.height < 110){
            camera.height++;
        }
    }
    if(state->buttons & CONT_DPAD_UP){
        camera.x += cosine(camera.angle);
        camera.y += sine(camera.angle);
    }
    if(state->buttons & CONT_DPAD_DOWN){
        camera.x -= cosine(camera.angle);
        camera.y -= sine(camera.angle);
    }
    if(state->buttons & CONT_DPAD_LEFT){
        camera.angle -= 0.02;
        camera.angle = camera.angle >= 0.0 ? fmod(camera.angle, 6.28) : 6.28 - abs(fmod(camera.angle, 6.28));
    }
    if(state->buttons & CONT_DPAD_RIGHT){
        camera.angle = fmod((camera.angle + 0.02), (6.28));
    }
    if(state->ltrig){
        return 0;
    }
    if(state->rtrig){
        return 0;
    }
    return 1;
}

float perspecive_divide_table[512][600];
void init_perspecive_divide_table(){
    for(int i = -255; i < 255; i++){
        for(int j = 0; j < 600; j++){
            perspecive_divide_table[i + 255][j] = i / (float)j  * SCALE_FACTOR;
        }
    }
}

float perspecive_divide(int16_t height, uint16_t zfar){
    return perspecive_divide_table[height + 255][zfar];
}

//Update Game State
void update_game_state(){
    //TODO remover esses calculos da main
    float sin_angle = sine(camera.angle);
    float cos_angle = cosine(camera.angle);

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
            // int proj_height = (int)(((float)camera.height - height_map[map_offset]) / z * SCALE_FACTOR + camera.horizon);
            int proj_height = (int) (perspecive_divide_table[camera.height - height_map[map_offset] + 255][z] + camera.horizon);

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

    init_sintab();
    init_costab();
    init_perspecive_divide_table();

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
            sprintf(screen_text, "fps: %.2f", fps);
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