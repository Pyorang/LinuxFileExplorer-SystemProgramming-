#ifndef DIRECTORY_UTILS_H
#define DIRECTORY_UTILS_H

#include <sys/types.h>
#include <sys/stat.h>

void EnsureRootDir();
void GetVirtualPath(char* virtualPath, const char* realPath);
void GetRealPath(char* realPath, const char* virtualPath);
void PrintCurrentDirectory();
char* GetCurrentDirectory();
void ChangeDirectory(const char* path);
void CreateDirectory(const char* path);
void RemoveDirectory(const char* path);

#endif
