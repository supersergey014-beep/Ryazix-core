#ifndef RYAZIX_VGA_H
#define RYAZIX_VGA_H

#include <stdint.h>

void vga_clear(void);
void vga_putc(char character, uint8_t color);
void vga_puts(const char *str, uint8_t color);

#endif