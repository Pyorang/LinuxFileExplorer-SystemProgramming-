#include "file_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <stdbool.h>

#define MAX_CMD_SIZE (128)
#define MAX_ENTRY_COUNT 256
#define MAX_ENTRY_LENGTH 256
#define OUTPUT_BUFFER_SIZE 8192
#define CHUNK_SIZE 1024
#define ROOT_DIR "/tmp/test"

void PrintModeToString(mode_t mode, char* modeStr)
{
    if (S_ISREG(mode)) modeStr[0] = '-';
    else if (S_ISDIR(mode)) modeStr[0] = 'd';
    else if (S_ISLNK(mode)) modeStr[0] = 'l';
    else if (S_ISCHR(mode)) modeStr[0] = 'c';
    else if (S_ISBLK(mode)) modeStr[0] = 'b';
    else if (S_ISFIFO(mode)) modeStr[0] = 'p';
    else if (S_ISSOCK(mode)) modeStr[0] = 's';
    else modeStr[0] = '?';

    modeStr[1] = (mode & S_IRUSR) ? 'r' : '-';
    modeStr[2] = (mode & S_IWUSR) ? 'w' : '-';
    modeStr[3] = (mode & S_IXUSR) ? 'x' : '-';

    modeStr[4] = (mode & S_IRGRP) ? 'r' : '-';
    modeStr[5] = (mode & S_IWGRP) ? 'w' : '-';
    modeStr[6] = (mode & S_IXGRP) ? 'x' : '-';

    modeStr[7] = (mode & S_IROTH) ? 'r' : '-';
    modeStr[8] = (mode & S_IWOTH) ? 'w' : '-';
    modeStr[9] = (mode & S_IXOTH) ? 'x' : '-';

    modeStr[10] = '\0';
}

void PrintFileTypeDetailed(const char* filename, int showDetailed)
{
    struct stat st;

    if (lstat(filename, &st) == 0)
    {
        char modeStr[11];
        PrintModeToString(st.st_mode, modeStr);

        if (showDetailed)
        {
            struct passwd* pwd = getpwuid(st.st_uid);
            struct group* grp = getgrgid(st.st_gid);

            char linkTarget[256] = { 0 };
            if (S_ISLNK(st.st_mode))
            {
                ssize_t len = readlink(filename, linkTarget, sizeof(linkTarget) - 1);
                if (len != -1)
                    linkTarget[len] = '\0';
            }

            printf("%-20s %10s %3ld %-8s %-8s %8ldytes %s",
                filename, modeStr, (long)st.st_nlink,
                pwd ? pwd->pw_name : "UNKNOWN",
                grp ? grp->gr_name : "UNKNOWN",
                (long)st.st_size,
                ctime(&st.st_mtime));

            if (S_ISLNK(st.st_mode))
                printf(" -> %s", linkTarget);

            printf("\n");
        }
        else
        {
            printf("%-20s %10s %8ldbytes %s",
                filename, modeStr, (long)st.st_size, ctime(&st.st_mtime));
        }
    }
    else
        perror("stat failed");
}

void SendFileInfoToGUI(const char* targetFilename, int pipe_fd)
{
    DIR* dir = opendir(".");
    if (dir == NULL)
    {
        perror("opendir failed");
        return;
    }

    struct dirent* entry;
    struct stat st;
    char buffer[1024];
    int found = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, targetFilename) == 0)
        {
            if (lstat(entry->d_name, &st) == 0)
            {
                char modeStr[11];
                PrintModeToString(st.st_mode, modeStr);

                char timeStr[64];
                struct tm* t = localtime(&st.st_mtime);
                strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", t);

                snprintf(buffer, sizeof(buffer), "[FileInfo] %s %s %ldbytes %s\n",
                         targetFilename, modeStr, (long)st.st_size, timeStr);

                if (write(pipe_fd, buffer, strlen(buffer)) == -1)
                {
                    perror("Failed to write to pipe");
                }
                found = 1;
            }
            else
            {
                perror("lstat failed");
            }
            break;
        }
    }

    closedir(dir);

    if (!found)
    {
        snprintf(buffer, sizeof(buffer), "[FileInfo] File '%s' not found.\n", targetFilename);
        if (write(pipe_fd, buffer, strlen(buffer)) == -1)
        {
            perror("Failed to write not found message to pipe");
        }
    }
}

void ListFiles(const char* option)
{
    DIR* dir = opendir(".");
    if (dir == NULL)
    {
        perror("opendir failed");
        return;
    }

    struct dirent* entry;
    char files[256][256];
    char dirs[256][256];
    int fileCount = 0, dirCount = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (entry->d_type == DT_DIR)
            strncpy(dirs[dirCount++], entry->d_name, sizeof(dirs[0]));
        else
            strncpy(files[fileCount++], entry->d_name, sizeof(files[0]));
    }

    closedir(dir);

    qsort(dirs, dirCount, sizeof(dirs[0]), (int (*)(const void*, const void*))strcmp);
    qsort(files, fileCount, sizeof(files[0]), (int (*)(const void*, const void*))strcmp);

    int showDetailed = option && strcmp(option, "-l") == 0;

    if (!showDetailed)
        printf("\n[Directories]\n");
    for (int i = 0; i < dirCount; i++)
        PrintFileTypeDetailed(dirs[i], showDetailed);

    if (!showDetailed)
        printf("\n[Files]\n");
    for (int i = 0; i < fileCount; i++)
        PrintFileTypeDetailed(files[i], showDetailed);
}

char* ListFilesWithDetails()
{
    DIR* dir = opendir(".");
    if (dir == NULL)
    {
        perror("opendir failed");
        return NULL;
    }

    struct dirent* entry;
    char files[MAX_ENTRY_COUNT][MAX_ENTRY_LENGTH];
    char dirs[MAX_ENTRY_COUNT][MAX_ENTRY_LENGTH];
    int fileCount = 0, dirCount = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (entry->d_type == DT_DIR)
            strncpy(dirs[dirCount++], entry->d_name, sizeof(dirs[0]) - 1);
        else
            strncpy(files[fileCount++], entry->d_name, sizeof(files[0]) - 1);
    }
    closedir(dir);

    qsort(dirs, dirCount, sizeof(dirs[0]), (int (*)(const void*, const void*))strcmp);
    qsort(files, fileCount, sizeof(files[0]), (int (*)(const void*, const void*))strcmp);

    char* output = (char*)malloc(OUTPUT_BUFFER_SIZE);
    if (!output)
    {
        perror("malloc failed");
        return NULL;
    }
    output[0] = '\0';

    strcat(output, "[Files]:");
    for (int i = 0; i < fileCount; i++)
    {
        strcat(output, " ");
        strcat(output, files[i]);
    }

    strcat(output, " [Directories]:");
    for (int i = 0; i < dirCount; i++)
    {
        strcat(output, " ");
        strcat(output, dirs[i]);
    }

    return output;
}

bool is_valid_directory(const char* path) 
{
    struct stat st;

    if (strncmp(path, "/tmp/test", strlen("/tmp/test")) != 0) {
        printf("The path '%s' does not start with '/tmp/test'.\n", path);
        return false;
    }

    if (stat(path, &st) != 0) {
        perror("stat failed");
        return false;
    }

    if (!S_ISDIR(st.st_mode)) {
        printf("The path '%s' is not a directory.\n", path);
        return false;
    }

    return true;
}

void RenameFileOrDirectory(const char* source, const char* target)
{
    if (source == NULL || target == NULL)
    {
        printf("Error: rename requires source and target arguments.\n");
        return;
    }

    if (rename(source, target) != 0)
    {
        perror("rename failed");
        return;
    }

    printf("Successfully renamed '%s' to '%s'.\n", source, target);
}

void CreateLink(const char* arg1, const char* arg2, const char* arg3)
{
    if (arg1 == NULL || arg2 == NULL)
    {
        printf("Error: link requires original file and new file arguments.\n");
        return;
    }

    if (strcmp(arg1, "-s") == 0)
    {
        if (symlink(arg2, arg3) != 0)
        {
            perror("symlink failed");
            return;
        }
        printf("Symbolic link created: '%s' -> '%s'\n", arg3, arg2);
    }

    else
    {
        if (link(arg1, arg2) != 0)
        {
            perror("link failed");
            return;
        }
        printf("Hard link created: '%s' -> '%s'\n", arg2, arg1);
    }
}

void RemoveFile(const char* path)
{
    if (path == NULL)
    {
        printf("Error: remove requires file arguments.\n");
        return;
    }

    if (unlink(path) != 0)
    {
        perror("remove failed");
        return;
    }

    printf("File '%s' removed successfully.\n", path);
}

void DisplayFileContents(const char* path)
{
    if (path == NULL)
    {
        printf("Error: cat requires filename arguments.\n");
        return;
    }

    int rfd = open(path, O_RDONLY);
    if (rfd == -1)
    {
        perror("Open File");
        return;
    }

    char* buffer = (char*)malloc(MAX_CMD_SIZE);
    if (buffer == NULL)
    {
        perror("Memory allocation failed");
        close(rfd);
        return;
    }

    ssize_t bytesRead;
    while ((bytesRead = read(rfd, buffer, MAX_CMD_SIZE)) > 0)
    {
        if (fwrite(buffer, 1, bytesRead, stdout) != (size_t)bytesRead)
        {
            perror("Write to stdout failed");
            break;
        }
    }

    if (bytesRead == -1)
    {
        perror("Read from File");
    }

    free(buffer);
    close(rfd);
}

void SendFileToGUI(const char* path, int pipe_fd)
{
    if (path == NULL)
    {
        printf("Error: SendFileToGUI requires a file path.\n");
        return;
    }

    int file_fd = open(path, O_RDONLY);
    if (file_fd == -1)
    {
        perror("Failed to open file");
        return;
    }

    const char* start_marker = "[START_OF_FILE]\n";
    if (write(pipe_fd, start_marker, strlen(start_marker)) != (ssize_t)strlen(start_marker))
    {
        perror("Failed to write start marker to pipe");
        close(file_fd);
        return;
    }

    char buffer[CHUNK_SIZE];
    ssize_t bytesRead;

    while ((bytesRead = read(file_fd, buffer, CHUNK_SIZE)) > 0)
    {
        if (write(pipe_fd, buffer, bytesRead) != bytesRead)
        {
            perror("Failed to write to pipe");
            break;
        }
    }

    if (bytesRead == -1)
    {
        perror("Error reading file");
    }

    const char* end_marker = "[END_OF_FILE]\n";
    if (write(pipe_fd, end_marker, strlen(end_marker)) != (ssize_t)strlen(end_marker))
    {
        perror("Failed to write end marker to pipe");
    }

    close(file_fd);
}


void CopyFile(const char* source, const char* destination)
{
    if (source == NULL || destination == NULL)
    {
        printf("Error: cp requires original file and new file arguments.\n");
        return;
    }

    struct stat st;
    if (stat(source, &st) != 0)
    {
        perror("stat failed");
        return;
    }

    int rfd = open(source, O_RDONLY);
    if (rfd == -1)
    {
        perror("Open Source File");
        return;
    }

    int wfd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
    if (wfd == -1)
    {
        perror("Open Destination File");
        close(rfd);
        return;
    }

    char* buffer = (char*)malloc(MAX_CMD_SIZE);
    if (buffer == NULL)
    {
        perror("Memory allocation failed");
        close(rfd);
        close(wfd);
        return;
    }

    ssize_t bytesRead;
    while ((bytesRead = read(rfd, buffer, MAX_CMD_SIZE)) > 0)
    {
        if (write(wfd, buffer, bytesRead) != bytesRead)
        {
            perror("Write to Destination File");
            break;
        }
    }

    if (bytesRead == -1)
        perror("Read from Source File");

    free(buffer);
    close(rfd);
    close(wfd);

    if (bytesRead != -1)
        printf("File '%s' copied successfully to '%s'.\n", source, destination);
}