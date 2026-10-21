#ifndef FS_H
#define FS_H
int  fs_list(const char *path);
int  fs_copy(const char *src, const char *dst);
int  fs_remove(const char *path);
#endif
