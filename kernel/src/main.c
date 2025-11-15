#include <stdint.h>
#include <stddef.h>
#include <limine.h>

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

void sleep(size_t cycles){
    for(size_t i = 0; i < cycles; i++){
        for(size_t j = 0; j < 10000; j++) asm("nop");
    }
}

void kernel_main(void) {
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    uint8_t* fb_ptr = framebuffer->address;
    size_t counter = 0;
    uint32_t colors[] = {
    0xFF1A73FF,
    0xFFFF3D3D,
    0xFF34FF96,
    0xFFFFC400,
    0xFF9A4DFF,
};
    for(;;){
        for(size_t y = 0; y < framebuffer->height; y++){
            for(size_t x = 0; x < framebuffer->width; x++){
                *(uint32_t*)(fb_ptr + y*framebuffer->pitch + x * sizeof(uint32_t)) = colors[counter];
            }
        }
        sleep(10000);
        counter = (counter + 1) % (sizeof(colors)/sizeof(colors[0]));
    }
}
