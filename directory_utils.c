#include "directory_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ROOT_DIR "/tmp/test"
#define MAX_CMD_SIZE 128

void EnsureRootDir()
{
    struct stat st;
    if (stat(ROOT_DIR, &st) == -1)
    {
        if (mkdir(ROOT_DIR, 0755) != 0)
        {
            perror("Failed to create root directory");
            exit(1);
        }
    }
}

void GetVirtualPath(char* virtualPath, const char* realPath)
{
    if (strncmp(realPath, ROOT_DIR, strlen(ROOT_DIR)) == 0)
    {
        strcpy(virtualPath, realPath + strlen(ROOT_DIR));
        if (strlen(virtualPath) == 0)
            strcpy(virtualPath, "/");
    }
    else
    {
        strcpy(virtualPath, "/");
    }
}

void GetRealPath(char* realPath, const char* virtualPath)
{
    snprintf(realPath, MAX_CMD_SIZE, "%s%s", ROOT_DIR, virtualPath);
}

void PrintCurrentDirectory()
{
    char realPath[MAX_CMD_SIZE];
    char virtualPath[MAX_CMD_SIZE];

    if (getcwd(realPath, MAX_CMD_SIZE) == NULL)
    {
        perror("getcwd failed");
        return;
    }

    GetVirtualPath(virtualPath, realPath);
    printf("%s", virtualPath);
}

char* GetCurrentDirectory()
{
    char realPath[MAX_CMD_SIZE];

    if (getcwd(realPath, MAX_CMD_SIZE) == NULL)
    {
        perror("getcwd failed");
        return NULL;
    }

    char* result = (char*)malloc(strlen(realPath) + 1);
    if (result == NULL)
    {
        perror("malloc failed");
        return NULL;
    }

    strcpy(result, realPath);

    return result;
}



void ChangeDirectory(const char* path)
{
    if (path == NULL)
    {
        printf("Error: cd requires a path argument.\n");
        return;
    }

    char realPath[MAX_CMD_SIZE];
    char virtualPath[MAX_CMD_SIZE];
    char tempPath[MAX_CMD_SIZE * 2];

    if (getcwd(realPath, MAX_CMD_SIZE) == NULL)
    {
        perror("getcwd failed");
        return;
    }

    GetVirtualPath(virtualPath, realPath);

    if (strcmp(path, "/") == 0)
    {
        GetRealPath(realPath, "/");
    }

    else if (strcmp(path, "..") == 0 || strcmp(path, "/..") == 0)
    {
        if (strcmp(virtualPath, "/") == 0)
        {
            printf("Error: Cannot move above the root directory (%s)\n", ROOT_DIR);
            return;
        }

        if (strlen(realPath) + strlen("/..") >= sizeof(tempPath))
        {
            printf("Error: Path is too long.\n");
            return;
        }

        snprintf(tempPath, sizeof(tempPath), "%s/..", realPath);
        realpath(tempPath, realPath);
    }

    else if (path[0] != '/')
    {
        if (strlen(virtualPath) + strlen(path) + 2 >= sizeof(tempPath))
        {
            printf("Error: Path is too long.\n");
            return;
        }

        snprintf(tempPath, sizeof(tempPath), "%s/%s", virtualPath, path);
        GetRealPath(realPath, tempPath);
    }

    else
    {
        if (strlen(ROOT_DIR) + strlen(path) + 1 >= sizeof(realPath))
        {
            printf("Error: Path is too long.\n");
            return;
        }

        GetRealPath(realPath, path);
    }

    if (strncmp(realPath, ROOT_DIR, strlen(ROOT_DIR)) != 0)
    {
        printf("Error: Cannot access directories outside of %s\n", ROOT_DIR);
        return;
    }

    if (chdir(realPath) != 0)
        perror("cd failed");

    else
    {
        getcwd(realPath, sizeof(realPath));
        GetVirtualPath(virtualPath, realPath);
        printf("Changed directory to %s\n", virtualPath);
    }
}

void CreateDirectory(const char* path)
{
    if (path == NULL)
    {
        printf("Error: mkdir requires a path argument.\n");
        return;
    }

    if (mkdir(path, 0755) != 0)
    {
        perror("mkdir failed");
        return;
    }

    printf("Directory '%s' created successfully.\n", path);
}

void RemoveDirectory(const char* path)
{
    if (path == NULL)
    {
        printf("Error: rmdir requires a path argument.\n");
        return;
    }

    if (rmdir(path) != 0)
    {
        perror("rmdir failed");
        return;
    }

    printf("Directory '%s' removed successfully.\n", path);
}