CC = gcc
AS = as
LD = ld
QEMU ?= qemu-system-i386

TARGET := ryazix.bin
BUILD_DIR := build

CFLAGS := -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib \
	-fno-builtin -Wall -Wextra -MMD -MP
ASFLAGS := --32
LDFLAGS := -m elf_i386 -T linker.ld -nostdlib

C_SOURCES := \
	gdt.c \
	idt.c \
	initrd.c \
	io.c \
	kernel.c \
	keyboard.c \
	kheap.c \
	pic.c \
	pmm.c \
	scheduler.c \
	shell.c \
	syscall.c \
	task.c \
	timer.c \
	vfs.c \
	vmm.c

ASM_SOURCES := boot.s gdt_flush.s interrupts.s process.s
OBJECTS := $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o) $(ASM_SOURCES:.s=.o))
DEPFILES := $(BUILD_DIR)/$(C_SOURCES:.c=.d)

.PHONY: all clean qemu

all: $(TARGET)

$(TARGET): $(OBJECTS) linker.ld
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

qemu: $(TARGET)
	$(QEMU) -kernel $(TARGET) $(if $(INITRD),-initrd $(INITRD),)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPFILES)