#include "initrd.h"

#include "kheap.h"

#define TAR_BLOCK_SIZE 512U
#define INITRD_MAX_SIZE (16U * 1024U * 1024U)

typedef struct tar_header_struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
} __attribute__((packed)) tar_header_t;

static uint32_t next_inode = 1;

static uint32_t bounded_length(const char *string, uint32_t maximum)
{
    uint32_t length = 0;
    while (length < maximum && string[length] != '\0') {
        ++length;
    }
    return length;
}

static void copy_range(char *destination, const char *source, uint32_t length)
{
    uint32_t index;
    for (index = 0; index < length; ++index) {
        destination[index] = source[index];
    }
    destination[length] = '\0';
}

static uint32_t parse_octal(const char *field, uint32_t length)
{
    uint32_t value = 0;
    uint32_t index = 0;

    while (index < length && (field[index] == ' ' || field[index] == '\0')) {
        ++index;
    }
    while (index < length && field[index] >= '0' && field[index] <= '7') {
        if (value > (UINT32_MAX - 7U) / 8U) {
            return UINT32_MAX;
        }
        value = value * 8U + (uint32_t)(field[index] - '0');
        ++index;
    }
    return value;
}

static uint32_t tar_read(fs_node_t *node, uint32_t offset, uint32_t size,
                        uint8_t *buffer)
{
    uint32_t remaining;
    uint32_t index;

    if (offset >= node->length) {
        return 0;
    }
    remaining = node->length - offset;
    if (size > remaining) {
        size = remaining;
    }
    for (index = 0; index < size; ++index) {
        buffer[index] = node->data[offset + index];
    }
    return size;
}

static fs_node_t *create_node(const char *name, uint32_t flags)
{
    fs_node_t *node = (fs_node_t *)kmalloc((uint32_t)sizeof(fs_node_t));
    uint32_t index;

    if (node == 0) {
        return 0;
    }
    for (index = 0; index < sizeof(*node); ++index) {
        ((uint8_t *)node)[index] = 0;
    }
    copy_range(node->name, name, bounded_length(name, FS_NAME_MAX - 1U));
    node->flags = flags;
    node->dirent.ino = next_inode++;
    return node;
}

static fs_node_t *ensure_directory(fs_node_t *parent, const char *name)
{
    fs_node_t *node = finddir_fs(parent, name);

    if (node != 0) {
        return (node->flags & FS_DIRECTORY) != 0 ? node : 0;
    }
    node = create_node(name, FS_DIRECTORY);
    if (node == 0 || vfs_add_node(parent, node) != 0) {
        if (node != 0) {
            kfree(node);
        }
        return 0;
    }
    return node;
}

static int32_t add_tar_node(const tar_header_t *header, uint8_t *data,
                            uint32_t length)
{
    char path[256];
    char component[FS_NAME_MAX];
    uint32_t prefix_length = bounded_length(header->prefix, sizeof(header->prefix));
    uint32_t name_length = bounded_length(header->name, sizeof(header->name));
    uint32_t path_length = 0;
    uint32_t index = 0;
    fs_node_t *parent = fs_root;
    fs_node_t *node;
    uint32_t component_length;
    uint32_t node_flags;

    if (parent == 0 || name_length == 0 || prefix_length + name_length + 2U > sizeof(path)) {
        return -1;
    }
    if (prefix_length != 0) {
        copy_range(path, header->prefix, prefix_length);
        path_length = prefix_length;
        path[path_length++] = '/';
    }
    copy_range(path + path_length, header->name, name_length);
    path_length += name_length;

    while (index < path_length) {
        component_length = 0;
        while (index < path_length && path[index] != '/') {
            if (component_length >= FS_NAME_MAX - 1U) {
                return -1;
            }
            component[component_length++] = path[index++];
        }
        component[component_length] = '\0';
        while (index < path_length && path[index] == '/') {
            ++index;
        }
        if (component_length == 0) {
            continue;
        }

        if (index < path_length) {
            parent = ensure_directory(parent, component);
            if (parent == 0) {
                return -1;
            }
            continue;
        }

        node = finddir_fs(parent, component);
        if (node != 0) {
            return 0;
        }
        node_flags = (header->typeflag == '5') ? FS_DIRECTORY : FS_FILE;
        node = create_node(component, node_flags);
        if (node == 0) {
            return -1;
        }
        node->length = node_flags == FS_FILE ? length : 0;
        node->data = node_flags == FS_FILE ? data : 0;
        node->read = node_flags == FS_FILE ? tar_read : 0;
        return vfs_add_node(parent, node);
    }
    return 0;
}

int32_t initialise_initrd_range(uint32_t location, uint32_t size)
{
    uint32_t offset = 0;

    if (fs_root == 0 || location == 0 || size < TAR_BLOCK_SIZE) {
        return -1;
    }
    while (offset <= size - TAR_BLOCK_SIZE) {
        const tar_header_t *header = (const tar_header_t *)(location + offset);
        uint32_t file_size;
        uint32_t padded_size;
        uint8_t all_zero = 1;
        uint32_t index;

        for (index = 0; index < TAR_BLOCK_SIZE; ++index) {
            if (((const uint8_t *)header)[index] != 0) {
                all_zero = 0;
                break;
            }
        }
        if (all_zero) {
            return 0;
        }

        file_size = parse_octal(header->size, sizeof(header->size));
        if (file_size == UINT32_MAX || file_size > size - offset - TAR_BLOCK_SIZE) {
            return -1;
        }
        padded_size = (file_size + TAR_BLOCK_SIZE - 1U) & ~(TAR_BLOCK_SIZE - 1U);
        if (padded_size > size - offset - TAR_BLOCK_SIZE) {
            return -1;
        }

        if (header->typeflag == '\0' || header->typeflag == '0' ||
            header->typeflag == '5') {
            if (add_tar_node(header, (uint8_t *)(location + offset + TAR_BLOCK_SIZE),
                             file_size) != 0) {
                return -1;
            }
        }
        offset += TAR_BLOCK_SIZE + padded_size;
    }
    return 0;
}

int32_t initialise_initrd(uint32_t location)
{
    return initialise_initrd_range(location, INITRD_MAX_SIZE);
}