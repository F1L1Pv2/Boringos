#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#include "font.h"

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

void sleep(size_t cycles){
    for(size_t i = 0; i < cycles; i++){
        for(size_t j = 0; j < 10000; j++) asm("nop");
    }
}

void draw_rect_on_framebuffer(
    void* framebuffer_data, 
    size_t framebuffer_width, size_t framebuffer_pitch, size_t framebuffer_height, 
    int x, int y, 
    size_t width, size_t height, 
    uint32_t color)
{
    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }

    if ((size_t)x + width > framebuffer_width)
        width = framebuffer_width - x;
    if ((size_t)y + height > framebuffer_height)
        height = framebuffer_height - y;

    if (width <= 0 || height <= 0)
        return;

    uint8_t* base = (uint8_t*)framebuffer_data;
    for (size_t fy = y; fy < (size_t)y + height; fy++) {
        uint32_t* row = (uint32_t*)(base + fy * framebuffer_pitch);
        for (size_t fx = x; fx < (size_t)x + width; fx++) {
            row[fx] = color;
        }
    }
}

void draw_character_on_framebuffer(
    void* framebuffer_data, 
    size_t framebuffer_width, size_t framebuffer_pitch, size_t framebuffer_height, 
    int x, int y,
    char ch
){
    uint8_t* base = framebuffer_data;

    for(size_t fy = 0; fy < ATLAS_CHAR_HEIGHT; fy++){
        for(size_t fx = 0; fx < ATLAS_CHAR_WIDTH; fx++){
            int px = x + fx;
            int py = y + fy;

            if(px < 0 || py < 0) continue;
            if(px >= (int)framebuffer_width || py >= (int)framebuffer_height) continue;

            uint32_t* pixel = (uint32_t*)(base + py*framebuffer_pitch + px*sizeof(uint32_t));

            uint8_t intensity = font_bin[fy * (256 * ATLAS_CHAR_WIDTH) + ch * ATLAS_CHAR_WIDTH + fx];

            *pixel = 0xFF000000 | ((uint32_t)intensity << 16) | ((uint32_t)intensity << 8) | (uint32_t)intensity;
        }
    }
}

void putc_and_scroll(char ch,
    void* framebuffer_data, 
    size_t framebuffer_width, size_t framebuffer_pitch, size_t framebuffer_height, 
    size_t* cur_x, size_t* cur_y){
    if(*cur_x + ATLAS_CHAR_WIDTH > framebuffer_width || ch == '\n'){
        *cur_x = 0;
        *cur_y += ATLAS_CHAR_HEIGHT;
    }
    if(ch == '\n') return;
    draw_character_on_framebuffer(framebuffer_data, framebuffer_width, framebuffer_pitch, framebuffer_height, *cur_x, *cur_y, ch);
    *cur_x += ATLAS_CHAR_WIDTH;
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

    size_t cur_x = 0;
    size_t cur_y = 0;
    #define putc(ch) putc_and_scroll((ch),framebuffer->address,framebuffer->width,framebuffer->pitch,framebuffer->height,&cur_x,&cur_y)

    const char* hello_string = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut eu ullamcorper sapien, in gravida neque. Nulla placerat efficitur nisi. Aliquam aliquet dolor vel libero condimentum posuere et vitae augue. Mauris interdum lectus urna, non scelerisque diam eleifend et. Ut porttitor diam tristique, iaculis urna sit amet, viverra dolor. Donec posuere purus in feugiat sollicitudin. Proin quis purus ac justo vehicula scelerisque sed in urna. Cras pulvinar vel lacus ac porta. Fusce eleifend sapien sed dui vestibulum, at sodales lacus iaculis. Praesent commodo urna ac vestibulum iaculis. Etiam id ex id diam molestie suscipit. Suspendisse quis dapibus ante, sed imperdiet ligula. Nam rhoncus lectus eu leo faucibus, sed iaculis arcu lacinia. Nulla condimentum pretium porttitor. Sed vitae justo a lectus tempus laoreet. Phasellus vel nunc justo. Cras dictum est sit amet porta euismod. Sed suscipit urna ac justo viverra finibus. Phasellus efficitur nibh mi, vitae placerat velit aliquet elementum. Vivamus non porttitor nulla. Vestibulum id consequat ligula, a commodo dolor. Etiam ornare ante quis velit finibus pharetra eget eu ligula. Fusce tellus mauris, facilisis sit amet posuere fringilla, pharetra quis ligula. Curabitur eget hendrerit mi, eu tincidunt massa. Sed feugiat ipsum non sapien semper, nec dictum nisi facilisis. Mauris ac ipsum sed ante hendrerit sollicitudin sed vel metus. Suspendisse eu dolor et lacus aliquam dictum. Maecenas tincidunt augue arcu. Fusce consectetur nibh at dui congue, non posuere risus pulvinar. Integer lacinia est sit amet ligula iaculis, ac hendrerit est feugiat. Vestibulum placerat libero vitae lacinia feugiat. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Duis non nunc eu mi fringilla semper. Sed pretium in mi sed efficitur. Proin semper mi quis lectus malesuada, quis feugiat lectus elementum. Nullam consequat bibendum eros, quis gravida lectus consequat in. Phasellus placerat hendrerit consectetur. Mauris condimentum quam eu quam egestas fermentum. Etiam ultricies purus vitae velit aliquam, id hendrerit leo auctor. Cras vehicula porttitor urna ac vulputate. Sed varius urna nec quam sodales rutrum. Proin egestas tellus non orci vehicula varius. Vestibulum sit amet nisl et arcu pulvinar pulvinar. Donec rhoncus aliquet magna a sagittis. Suspendisse sodales arcu ac dui hendrerit eleifend eu quis risus. Duis tincidunt dolor et congue pulvinar. ";
    char* ptr = hello_string;
    while(*ptr){
        putc(*ptr);
        ptr++;
        if(cur_y >= framebuffer->height) break;
    }
    
    #undef putc

    for(;;){
        // for(size_t y = 0; y < framebuffer->height; y++){
        //     for(size_t x = 0; x < framebuffer->width; x++){
        //         *(uint32_t*)(fb_ptr + y*framebuffer->pitch + x * sizeof(uint32_t)) = colors[counter];
        //     }
        // }
        // sleep(10000);
        // counter = (counter + 1) % (sizeof(colors)/sizeof(colors[0]));
    }
}
