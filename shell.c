#include "shell.h"

#include <stdint.h>

#include "vfs.h"
#include "vga.h"

#define SHELL_BUFFER_SIZE 128U
#define SHELL_READ_SIZE 128U

static char command_buffer[SHELL_BUFFER_SIZE];
static uint32_t command_length;
static uint8_t shell_active;

static uint32_t string_length(const char *string)
{
    uint32_t length = 0;
    while (string[length] != '\0') {
        ++length;
    }
    return length;
}

static int string_equal(const char *left, const char *right)
{
    uint32_t index = 0;
    while (left[index] != '\0' && right[index] != '\0') {
        if (left[index] != right[index]) {
            return 0;
        }
        ++index;
    }
    return left[index] == right[index];
}

static void print_prompt(void)
{
    vga_puts("ryazix> ", 0x0B);
}

static void shell_list_files(void)
{
    uint32_t index;
    dirent_t *entry;

    if (fs_root == 0) {
        vga_puts("VFS unavailable\n", 0x0C);
        return;
    }
    for (index = 0; (entry = readdir_fs(fs_root, index)) != 0; ++index) {
        vga_puts(entry->name, 0x0F);
        vga_puts("\n", 0x0F);
    }
}

static void shell_cat_file(const char *name)
{
    fs_node_t *node = resolve_path_fs(name);
    uint8_t buffer[SHELL_READ_SIZE];
    uint32_t offset = 0;
    uint32_t bytes;

    if (node == 0 || (node->flags & FS_FILE) == 0) {
        vga_puts("cat: file not found\n", 0x0C);
        return;
    }
    open_fs(node);
    while (offset < node->length) {
        bytes = read_fs(node, offset, sizeof(buffer), buffer);
        if (bytes == 0) {
            break;
        }
        for (uint32_t index = 0; index < bytes; ++index) {
            vga_putc((char)buffer[index], 0x0F);
        }
        offset += bytes;
    }
    if (node->length == 0 || offset == node->length) {
        vga_puts("\n", 0x0F);
    }
    close_fs(node);
}

static void shell_execute(char *command)
{
    uint32_t command_size = string_length(command);

    if (command_size == 0) {
        return;
    }
    if (string_equal(command, "help")) {
        vga_puts("help clear version ls cat echo\n", 0x0F);
    } else if (string_equal(command, "clear")) {
        vga_clear();
    } else if (string_equal(command, "version")) {
        vga_puts("Ryazix Kernel v1.0.0 (x86 Monolithic)\n", 0x0F);
    } else if (string_equal(command, "ls")) {
        shell_list_files();
    } else if (command_size >= 4 && command[0] == 'c' && command[1] == 'a' &&
               command[2] == 't' && command[3] == ' ') {
        shell_cat_file(command + 4);
    } else if (command_size >= 5 && command[0] == 'e' && command[1] == 'c' &&
               command[2] == 'h' && command[3] == 'o' && command[4] == ' ') {
        vga_puts(command + 5, 0x0F);
        vga_puts("\n", 0x0F);
    } else {
        vga_puts("Unknown command. Type help.\n", 0x0C);
    }
}

void shell_handle_input(char character)
{
    if (!shell_active) {
        return;
    }
    if (character == '\b') {
        if (command_length != 0) {
            --command_length;
            command_buffer[command_length] = '\0';
            vga_putc('\b', 0x0F);
        }
        return;
    }
    if (character == '\n') {
        vga_putc('\n', 0x0F);
        command_buffer[command_length] = '\0';
        shell_execute(command_buffer);
        command_length = 0;
        command_buffer[0] = '\0';
        print_prompt();
        return;
    }
    if (character < 0x20 || character > 0x7E ||
        command_length >= SHELL_BUFFER_SIZE - 1U) {
        return;
    }
    command_buffer[command_length++] = character;
    command_buffer[command_length] = '\0';
    vga_putc(character, 0x0F);
}

void shell_run(void)
{
    shell_active = 1;
    command_length = 0;
    command_buffer[0] = '\0';
    vga_puts("\nRyazix Kernel Shell\n", 0x0B);
    print_prompt();

    for (;;) {
        __asm__ volatile("sti; hlt");
    }
}