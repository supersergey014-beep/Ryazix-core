#ifndef RYAZIX_INITRD_H
#define RYAZIX_INITRD_H

#include <stdint.h>

#include "vfs.h"

int32_t initialise_initrd(uint32_t location);
int32_t initialise_initrd_range(uint32_t location, uint32_t size);

#endif