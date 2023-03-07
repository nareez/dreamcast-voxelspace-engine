#include "display.h"

uint32_t framebuffer_size;
uint16_t* backbuffer;

/* Initialize backbuffer
 * parameter: size of backbuffer */
void display_initialize_framebuffer(){
    framebuffer_size = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t);

    // allocate memory with 32-byte alignment
    backbuffer = (uint16_t*) memalign(32, framebuffer_size);

    memset(backbuffer,'\0', framebuffer_size);
}

void display_initialize(){
    //PVR INIT
    pvr_init_defaults();
    pvr_dma_init(); 
    
    //set our video mode
    vid_set_mode(DM_320x240, PM_RGB565);

    //initialize software double buffer
    display_initialize_framebuffer();
}

//flip double buffer
void display_flip_framebuffer(){
    // DMA Trasnfer
    vid_waitvbl();
    dcache_flush_range((uint32_t) backbuffer,framebuffer_size);
    while (!pvr_dma_ready());
    pvr_dma_transfer(backbuffer, (uint32_t) vram_s, framebuffer_size, PVR_DMA_VRAM32, -1, NULL, (ptr_t) NULL);
}

/* Initialize double buffer
 * parameter: Red, Green, Blue */
void display_clear_framebuffer(int r, int g, int b){
    memset(backbuffer, PACK_PIXEL(r, g, b), framebuffer_size);
}