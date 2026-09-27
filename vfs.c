#include "vfs.h"

#include "kheap.h"

fs_node_t *fs_root;

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

static void copy_name(char destination[FS_NAME_MAX], const char *source)
{
    uint32_t index;
    for (index = 0; index < FS_NAME_MAX - 1 && source[index] != '\0'; ++index) {
        destination[index] = source[index];
    }
    destination[index] = '\0';
}

static dirent_t *vfs_readdir(fs_node_t *node, uint32_t index)
{
    fs_node_t *child;

    if (node == 0 || (node->flags & FS_DIRECTORY) == 0) {
        return 0;
    }
    child = node->children;
    while (child != 0 && index != 0) {
        child = child->next;
        --index;
    }
    return child != 0 ? &child->dirent : 0;
}

void vfs_init(void)
{
    fs_node_t *root = (fs_node_t *)kmalloc((uint32_t)sizeof(fs_node_t));
    uint32_t index;

    if (root == 0) {
        fs_root = 0;
        return;
    }
    for (index = 0; index < sizeof(*root); ++index) {
        ((uint8_t *)root)[index] = 0;
    }
    copy_name(root->name, "/");
    root->flags = FS_DIRECTORY;
    root->readdir = vfs_readdir;
    fs_root = root;
}

uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size,
                 uint8_t *buffer)
{
    if (node == 0 || node->read == 0 || buffer == 0) {
        return 0;
    }
    return node->read(node, offset, size, buffer);
}

uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size,
                  const uint8_t *buffer)
{
    if (node == 0 || node->write == 0 || buffer == 0) {
        return 0;
    }
    return node->write(node, offset, size, buffer);
}

void open_fs(fs_node_t *node)
{
    if (node != 0 && node->open != 0) {
        node->open(node);
    }
}

void close_fs(fs_node_t *node)
{
    if (node != 0 && node->close != 0) {
        node->close(node);
    }
}

dirent_t *readdir_fs(fs_node_t *node, uint32_t index)
{
    if (node == 0 || node->readdir == 0) {
        return 0;
    }
    return node->readdir(node, index);
}

fs_node_t *finddir_fs(fs_node_t *node, const char *name)
{
    fs_node_t *child;

    if (node == 0 || name == 0 || (node->flags & FS_DIRECTORY) == 0) {
        return 0;
    }
    for (child = node->children; child != 0; child = child->next) {
        if (string_equal(child->name, name)) {
            return child;
        }
    }
    return 0;
}

fs_node_t *resolve_path_fs(const char *path)
{
    char component[FS_NAME_MAX];
    fs_node_t *node = fs_root;
    uint32_t index = 0;

    if (path == 0 || node == 0) {
        return 0;
    }
    while (path[index] == '/') {
        ++index;
    }
    while (path[index] != '\0') {
        uint32_t length = 0;

        while (path[index] != '\0' && path[index] != '/') {
            if (length >= FS_NAME_MAX - 1U) {
                return 0;
            }
            component[length++] = path[index++];
        }
        component[length] = '\0';
        while (path[index] == '/') {
            ++index;
        }
        if (length == 0) {
            continue;
        }
        node = finddir_fs(node, component);
        if (node == 0) {
            return 0;
        }
    }
    return node;
}

int32_t vfs_add_node(fs_node_t *parent, fs_node_t *node)
{
    fs_node_t **tail;

    if (parent == 0 || node == 0 || (parent->flags & FS_DIRECTORY) == 0 ||
        finddir_fs(parent, node->name) != 0) {
        return -1;
    }

    tail = &parent->children;
    while (*tail != 0) {
        tail = &(*tail)->next;
    }
    *tail = node;
    node->next = 0;
    if (node->dirent.ino == 0) {
        node->dirent.ino = (uint32_t)node;
    }
    if ((node->flags & FS_DIRECTORY) != 0 && node->readdir == 0) {
        node->readdir = vfs_readdir;
    }
    copy_name(node->dirent.name, node->name);
    return 0;
}