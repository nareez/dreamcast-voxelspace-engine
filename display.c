#include "display.h"

int currentBuffer;
uint16_t* framebuffer_1;
uint16_t* framebuffer_2;
uint32_t framebufferSize;

/* Initialize double buffer
 * parameter: size of framebuffer */
void dis_initializeDoublebuffer(uint32_t bufferSize){
    framebufferSize = bufferSize * sizeof(uint16_t);
    framebuffer_1 = malloc(framebufferSize);
    framebuffer_2 = malloc(framebufferSize);
    memset(framebuffer_1,'\0', framebufferSize);
    memset(framebuffer_2,'\0', framebufferSize);

    backbuffer = framebuffer_1;
    currentBuffer = 1;
}

//flip double buffer
void dis_flipBuffer(){
    // Store Queue Trasnfer
    if(currentBuffer == 1){
        currentBuffer = 2;
        backbuffer = framebuffer_2;
        //vid_waitvbl();
        sq_cpy(vram_s, framebuffer_1, framebufferSize);

    } else {
        currentBuffer = 1;
        backbuffer = framebuffer_1;
        //vid_waitvbl();
        sq_cpy(vram_s, framebuffer_2, framebufferSize);
    }
    // DMA Trasnfer
    // dcache_flush_range((uint32_t) backbuffer,framebufferSize);
    // while (!pvr_dma_ready());
    // pvr_dma_transfer(backbuffer,(uint32_t) vram_s, framebufferSize,
    //                  PVR_DMA_VRAM32, -1, NULL, (ptr_t) NULL);
}

/* Initialize double buffer
 * parameter: Red, Green, Blue */
void dis_clearBackBuffer(int r, int g, int b){
    memset(backbuffer, PACK_PIXEL(r, g, b), framebufferSize);
}