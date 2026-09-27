#ifndef RYAZIX_VFS_H
#define RYAZIX_VFS_H

#include <stdint.h>

#define FS_FILE 0x01U
#define FS_DIRECTORY 0x02U
#define FS_NAME_MAX 64U

typedef struct dirent_struct {
    char name[FS_NAME_MAX];
    uint32_t ino;
} dirent_t;

struct fs_node_struct;
typedef struct fs_node_struct fs_node_t;

typedef uint32_t (*fs_read_t)(fs_node_t *node, uint32_t offset,
                              uint32_t size, uint8_t *buffer);
typedef uint32_t (*fs_write_t)(fs_node_t *node, uint32_t offset,
                               uint32_t size, const uint8_t *buffer);
typedef void (*fs_open_t)(fs_node_t *node);
typedef void (*fs_close_t)(fs_node_t *node);
typedef dirent_t *(*fs_readdir_t)(fs_node_t *node, uint32_t index);

struct fs_node_struct {
    char name[FS_NAME_MAX];
    uint32_t mask;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t length;
    fs_read_t read;
    fs_write_t write;
    fs_open_t open;
    fs_close_t close;
    fs_readdir_t readdir;
    fs_node_t *children;
    fs_node_t *next;
    const uint8_t *data;
    dirent_t dirent;
};

extern fs_node_t *fs_root;

void vfs_init(void);
uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size,
                 uint8_t *buffer);
uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size,
                  const uint8_t *buffer);
void open_fs(fs_node_t *node);
void close_fs(fs_node_t *node);
dirent_t *readdir_fs(fs_node_t *node, uint32_t index);
fs_node_t *finddir_fs(fs_node_t *node, const char *name);
fs_node_t *resolve_path_fs(const char *path);
int32_t vfs_add_node(fs_node_t *parent, fs_node_t *node);
int32_t vfs_create_file(const char *name, const uint8_t *data, uint32_t length);

#endif