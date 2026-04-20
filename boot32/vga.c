#include <boot32/vga.h> 

static inline uint16_t vga_entry(uint8_t uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len] != '\0')
		len++;
	return len;
}

int u32_to_str(uint32_t u, uint8_t base, int pad, char* buf, size_t buf_length) {
    /* Digits and letters have been exhausted */
    if (base > 36) return 1;
    if (pad == 0 || buf_length == 0) return 2;
    if (pad > 0 && (size_t)pad > buf_length) return 3;

    size_t i = 1;
    char sbuf[buf_length];

    while (true) {
        uint8_t d = u%base;
        u /= base;
        
        /* Use ascii code for base 10 numbers by default */
        char c = d+48;
        if (d > 9) {
            /* Start using letters as digits, if base is higher than 10 */
            c = (d-10)+97;
        }
        
        if (buf_length < i+1) return 4;

        sbuf[i-1] = c;
        
        if (u == 0) break;
        i++;
    }


    /* i = length of number/string */
    if (pad > 0 && (size_t)pad > i) {
        for (size_t j=0; j<pad-i; j++) buf[j] = '0';
        for (size_t j=0; j<i; j++) {
            buf[j+pad-i] = sbuf[(i-1)-j];
        }

        buf[pad] = '\0';
    } else {
        for (size_t j=0; j<i; j++) {
            buf[j] = sbuf[(i-1)-j];
        }

        buf[i] = '\0';
    }
    return 0; 
}



#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000 

size_t vga_row;
size_t vga_column;
uint8_t vga_color;
bool vga_scrolling = false;
uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;

void vga_setcolor(enum VGA_COLOR fg, enum VGA_COLOR bg) {
	vga_color = fg | bg << 4;
}

void vga_setfgcolor(enum VGA_COLOR fg) {
	vga_color = (vga_color & 0xF0) | fg;
}

void vga_setbgcolor(enum VGA_COLOR bg) {
	vga_color = (vga_color & 0x0F) | (bg << 4);
}

void vga_initialize() {
	vga_row = 0;
	vga_column = 0;
	vga_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);	
}

void vga_clear() {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			vga_buffer[index] = vga_entry(' ', vga_color);
		}
	}
}

void vga_scrolldown() {
    /* Copy every row to the row above it */
    for (int y=1; y<VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
			int indexFrom = y * VGA_WIDTH + x;
            int indexTo = (y-1) * VGA_WIDTH + x;

			vga_buffer[indexTo] = vga_buffer[indexFrom];
		}
    }

    /* Clear newly row */
    for (int x=0; x<VGA_WIDTH; x++) {
        int index = (VGA_HEIGHT-1) * VGA_WIDTH + x;
        vga_buffer[index] = vga_entry(' ', vga_color);
    }
}

void vga_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	vga_buffer[index] = vga_entry(c, color);
}

void vga_putchar(char c) {
    if (vga_scrolling) {
        vga_scrolling = false;
        vga_scrolldown();
    }
    if (c != '\n') vga_putentryat(c, vga_color, vga_column, vga_row);


    vga_column++;
	if (vga_column == VGA_WIDTH || c == '\n') {
		vga_column = 0;
        if (vga_row < VGA_HEIGHT-1) vga_row++;
        else vga_scrolling = true;
	}
}

void vga_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		vga_putchar(data[i]);
}

void vga_print(const char* data) {
	vga_write(data, strlen(data));
}

void vga_print_u32(uint32_t u, uint8_t base, int padding) {
    char buf[65];
    
    int e = u32_to_str(u, base, padding, buf, 65);
    if (e == 0) {
        vga_print(buf);
    } else {
        vga_print("!#ERR");
        u32_to_str(e, 10, -1, buf, 65);
        vga_print(buf);
    }
}

void vga_print_color(const char* data, enum VGA_COLOR fg) {
    enum VGA_COLOR old = vga_color;
    vga_setcolor(fg, vga_color >> 4 & 0xF);
	vga_print(data);
    vga_color = old;
}

void vga_print_u32_color(uint32_t u, uint8_t base, int padding, enum VGA_COLOR fg) {
    enum VGA_COLOR old = vga_color;
    vga_setcolor(fg, vga_color >> 4 & 0xF);
	vga_print_u32(u, base, padding);
    vga_color = old;
}


