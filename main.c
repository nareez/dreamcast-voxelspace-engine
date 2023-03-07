#include <kos.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "display.h"
#include "load_map.h"

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
    .zfar    = 300,
    .angle   = 5.0 //1.5 * 3.141592 // (= 270 deg)
};

// Handle controller input
int process_input() {
    maple_device_t *cont;
    cont_state_t *state;
    
    cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);

    // Check key status
    state = (cont_state_t *)maple_dev_status(cont);
    int buttons = state->buttons;

    if(!state) {
        printf("Error reading controller\n");
        return -1;
    }
    if(buttons & CONT_START){
        return 0;
    }

    camera.height += (buttons & CONT_A) ? -1 : ((buttons & CONT_Y) ? 1 : 0);
    camera.horizon += (buttons & CONT_B) ? 1.5 : ((buttons & CONT_X) ? -1.5 : 0);
    camera.angle += (buttons & CONT_DPAD_LEFT) ? -0.02 : ((buttons & CONT_DPAD_RIGHT) ? 0.02 : 0);

    if(buttons & CONT_DPAD_UP){
        camera.x += fcos(camera.angle);
        camera.y += fsin(camera.angle);
    } else if(buttons & CONT_DPAD_DOWN){
        camera.x -= fcos(camera.angle);
        camera.y -= fsin(camera.angle);
    }
    // if(state->ltrig){
    //     return 0;
    // }
    // if(state->rtrig){
    //     return 0;
    // }
    return 1;
}

//Update Game State
void update_game_state(){

    const float sin_angle = fsin(camera.angle);
    const float cos_angle = fcos(camera.angle);

    // Left-most point of the FOV
    const float plx = cos_angle * camera.zfar + sin_angle * camera.zfar;
    const float ply = sin_angle * camera.zfar - cos_angle * camera.zfar;

    // Right-most point of the FOV
    const float prx = cos_angle * camera.zfar - sin_angle * camera.zfar;
    const float pry = sin_angle * camera.zfar + cos_angle * camera.zfar;
    
    //
    const float deltax_step = (prx - plx) / SCREEN_WIDTH;
    const float deltay_step = (pry - ply) / SCREEN_WIDTH;

    // Loop SCREEN_WIDTH rays from left to right
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        const float delta_x = (plx + deltax_step * i) / camera.zfar;
        const float delta_y = (ply + deltay_step * i) / camera.zfar;

        // Ray (x,y) coords
        float rx = camera.x + delta_x;
        float ry = camera.y + delta_y;

        // Store the tallest projected height per-ray
        float tallest_height = SCREEN_HEIGHT;

        // Loop all depth units until the zfar distance limit
        for (int z = 1; z < camera.zfar; z+=2) {
            // Find the offset that we have to go and fetch values from the height_map
            int map_offset = ((MAP_N * ((int)(ry) & (MAP_N - 1))) + ((int)(rx) & (MAP_N - 1)));

            // Project height values and find the height on-screen
            int proj_height = (int)(((float)camera.height - height_map[map_offset]) / z * SCALE_FACTOR + camera.horizon);
 
            // Only draw pixels if the new projected height is taller than the previous tallest height
            if (proj_height < tallest_height) {
                // Draw pixels from previous max-height until the new projected height
                for (int y = tallest_height - 1; y >= proj_height; y--) {
                    DRAW_PIXEL(i, y, texture_map[map_offset]);
                }
                tallest_height = proj_height;
            }

            rx += delta_x;
            ry += delta_y;

            // Find the offset that we have to go and fetch values from the height_map
            map_offset = ((MAP_N * ((int)(ry) & (MAP_N - 1))) + ((int)(rx) & (MAP_N - 1)));

            // Project height values and find the height on-screen
            proj_height = (int)(((float)camera.height - height_map[map_offset]) / (z+1) * SCALE_FACTOR + camera.horizon);
 
            // Only draw pixels if the new projected height is taller than the previous tallest height
            if (proj_height < tallest_height) {
                // Draw pixels from previous max-height until the new projected height
                for (int y = tallest_height - 1; y >= proj_height; y--) {
                    DRAW_PIXEL(i, y, texture_map[map_offset]);
                }
                tallest_height = proj_height;
            }

            rx += delta_x;
            ry += delta_y;
        }
    }
}

char screen_text[20];
void render(){
    bfont_draw_str(backbuffer, SCREEN_WIDTH, 0, screen_text);
    display_flip_framebuffer();
    display_clear_framebuffer(0, 0x82, 0xFF);
}

/* romdisk */
extern uint8 romdisk_boot[];
KOS_INIT_ROMDISK(romdisk_boot);

int main(void) {
    int quit = 0;

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