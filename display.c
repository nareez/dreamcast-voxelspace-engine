#include "display.h"

int current_buffer;
uint16_t* framebuffer_1;
uint16_t* framebuffer_2;
uint32_t framebuffer_size;
uint16_t* backbuffer;

/* Initialize double buffer
 * parameter: size of framebuffer */
void display_initialize_doublebuffer(){
    framebuffer_size = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t);
    framebuffer_1 = (uint16_t*) malloc(framebuffer_size);
    framebuffer_2 = (uint16_t*) malloc(framebuffer_size);
    memset(framebuffer_1,'\0', framebuffer_size);
    memset(framebuffer_2,'\0', framebuffer_size);

    backbuffer = framebuffer_1;
    current_buffer = 1;
}

void display_initialize(){
    //set our video mode
    vid_set_mode(DM_320x240, PM_RGB565);

    //initialize software double buffer
    display_initialize_doublebuffer();
}


//flip double buffer
void display_flip_framebuffer(){
    // Store Queue Trasnfer
    if(current_buffer == 1){
        current_buffer = 2;
        backbuffer = framebuffer_2;
        //vid_waitvbl();
        sq_cpy(vram_s, framebuffer_1, framebuffer_size);

    } else {
        current_buffer = 1;
        backbuffer = framebuffer_1;
        //vid_waitvbl();
        sq_cpy(vram_s, framebuffer_2, framebuffer_size);
    }
    // DMA Trasnfer
    // dcache_flush_range((uint32_t) backbuffer,framebuffer_size);
    // while (!pvr_dma_ready());
    // pvr_dma_transfer(backbuffer,(uint32_t) vram_s, framebuffer_size,
    //                  PVR_DMA_VRAM32, -1, NULL, (ptr_t) NULL);
}

/* Initialize double buffer
 * parameter: Red, Green, Blue */
void display_clear_backbuffer(int r, int g, int b){
    memset(backbuffer, PACK_PIXEL(r, g, b), framebuffer_size);
}