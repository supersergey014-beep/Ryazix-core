#include "keyboard.h"

#include <stdint.h>

#include "idt.h"
#include "io.h"
#include "pic.h"
#include "shell.h"
#include "vga.h"

#define KEYBOARD_DATA_PORT 0x60

static const char keyboard_map[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0A] = '9', [0x0B] = '0', [0x0C] = '-', [0x0D] = '=',
    [0x0E] = '\b', [0x0F] = '\t', [0x10] = 'q', [0x11] = 'w', [0x12] = 'e',
    [0x13] = 'r', [0x14] = 't', [0x15] = 'y', [0x16] = 'u',
    [0x17] = 'i', [0x18] = 'o', [0x19] = 'p', [0x1A] = '[',
    [0x1B] = ']', [0x1C] = '\n', [0x1E] = 'a', [0x1F] = 's',
    [0x20] = 'd', [0x21] = 'f', [0x22] = 'g', [0x23] = 'h',
    [0x24] = 'j', [0x25] = 'k', [0x26] = 'l', [0x27] = ';',
    [0x28] = '\'', [0x29] = '`', [0x2B] = '\\', [0x2C] = 'z',
    [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v', [0x30] = 'b',
    [0x31] = 'n', [0x32] = 'm', [0x33] = ',', [0x34] = '.',
    [0x35] = '/', [0x39] = ' '
};

static const char shifted_keyboard_map[128] = {
    [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$',
    [0x06] = '%', [0x07] = '^', [0x08] = '&', [0x09] = '*',
    [0x0A] = '(', [0x0B] = ')', [0x0C] = '_', [0x0D] = '+',
    [0x0E] = '\b', [0x0F] = '\t', [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E',
    [0x13] = 'R', [0x14] = 'T', [0x15] = 'Y', [0x16] = 'U',
    [0x17] = 'I', [0x18] = 'O', [0x19] = 'P', [0x1A] = '{',
    [0x1B] = '}', [0x1C] = '\n', [0x1E] = 'A', [0x1F] = 'S',
    [0x20] = 'D', [0x21] = 'F', [0x22] = 'G', [0x23] = 'H',
    [0x24] = 'J', [0x25] = 'K', [0x26] = 'L', [0x27] = ':',
    [0x28] = '"', [0x29] = '~', [0x2B] = '|', [0x2C] = 'Z',
    [0x2D] = 'X', [0x2E] = 'C', [0x2F] = 'V', [0x30] = 'B',
    [0x31] = 'N', [0x32] = 'M', [0x33] = '<', [0x34] = '>',
    [0x35] = '?', [0x39] = ' '
};

static uint8_t shift_pressed;
static uint8_t caps_lock_enabled;

static registers_t *keyboard_irq_handler(registers_t *regs)
{
    uint8_t scan_code = inb(KEYBOARD_DATA_PORT);
    uint8_t released = scan_code & 0x80;
    uint8_t code = scan_code & 0x7F;
    char character;
    uint8_t uppercase;

    if (code == 0x2A || code == 0x36) {
        shift_pressed = (uint8_t)!released;
        return regs;
    }

    if (code == 0x3A) {
        if (!released) {
            caps_lock_enabled = (uint8_t)!caps_lock_enabled;
        }
        return regs;
    }

    if (released) {
        return regs;
    }

    character = keyboard_map[code];
    if (character >= 'a' && character <= 'z') {
        uppercase = caps_lock_enabled;
        if (shift_pressed) {
            uppercase = (uint8_t)!uppercase;
        }
        if (uppercase) {
            character = (char)(character - 'a' + 'A');
        }
    } else if (shift_pressed) {
        character = shifted_keyboard_map[code];
    }

    if (character != '\0') {
        shell_handle_input(character);
    }
    return regs;
}

void keyboard_init(void)
{
    idt_register_irq(1);
    irq_register_handler(1, keyboard_irq_handler);
    pic_unmask_irq(1);
}