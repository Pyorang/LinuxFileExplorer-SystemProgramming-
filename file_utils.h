#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <sys/types.h>
#include <stdbool.h>

void PrintModeToString(mode_t mode, char* modeStr);
void PrintFileTypeDetailed(const char* filename, int showDetailed);
void SendFileInfoToGUI(const char* targetFilename, int pipe_fd);
void ListFiles(const char* option);
char* ListFilesWithDetails();
bool is_valid_directory(const char* path);
void RenameFileOrDirectory(const char* source, const char* target);
void CreateLink(const char* arg1, const char* arg2, const char* arg3);
void RemoveFile(const char* path);
void DisplayFileContents(const char* path);
void SendFileToGUI(const char* path, int pipe_fd);
void CopyFile(const char* source, const char* destination);

#endif
