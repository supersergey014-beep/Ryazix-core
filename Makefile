CC = gcc
AS = as
LD = ld
QEMU ?= qemu-system-i386

TARGET := ryazix.bin
BUILD_DIR := build
SRC_DIR := src

CFLAGS := -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib \
	-fno-builtin -Wall -Wextra -MMD -MP
ASFLAGS := --32
LDFLAGS := -m elf_i386 -T $(SRC_DIR)/linker.ld -nostdlib

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
C_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst %.s,$(BUILD_DIR)/%.o,$(ASM_SOURCES))
OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)
DEPFILES := $(C_OBJECTS:.o=.d)

.PHONY: all clean qemu

all: $(TARGET)

$(TARGET): $(OBJECTS) $(SRC_DIR)/linker.ld
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

qemu: $(TARGET)
	$(QEMU) -kernel $(TARGET) $(if $(INITRD),-initrd $(INITRD),)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPFILES)