#ifndef PERMISSIONS_UTILS_H
#define PERMISSIONS_UTILS_H

#include <sys/types.h>

mode_t ParseSymbolicMode(const char* symbolicMode, mode_t currentMode);
void ChangePermissions(const char* mode, const char* path);

#endif
