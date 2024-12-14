#include "directory_utils.h"
#include "file_utils.h"
#include "help_utils.h"
#include "permissions_utils.h"
#include "process_utils.h"
#include "signal_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_CMD_SIZE (128)
#define ROOT_DIR "/tmp/test"

int pipe_to_gui[2];
int pipe_from_gui[2];

void ExecPythonGUI()
{
    if (pipe(pipe_to_gui) == -1 || pipe(pipe_from_gui) == -1)
    {
        perror("pipe failed");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        dup2(pipe_to_gui[0], STDIN_FILENO);
        dup2(pipe_from_gui[1], STDOUT_FILENO);
        close(pipe_to_gui[1]);
        close(pipe_from_gui[0]);

        execlp("python3", "python3", "gui.py", NULL);
        perror("execlp failed");
        exit(1);
    }
    else
    {
        close(pipe_to_gui[0]);
        close(pipe_from_gui[1]);
        printf("Python GUI process started with PID: %d\n", pid);
    }
}

int main()
{
    char* tok_str;

    char* arg1 = NULL;
    char* arg2 = NULL;
    char* arg3 = NULL;

    ExecPythonGUI();
    EnsureRootDir();

    if (chdir(ROOT_DIR) != 0)
    {
        perror("Failed to set root directory");
        return 1;
    }

    SetUpCtrlC();

    while (1)
    {
        //새로고침
        char* currentDir = GetCurrentDirectory();
        if (currentDir != NULL) 
        {
            write(pipe_to_gui[1], currentDir, strlen(currentDir));
            write(pipe_to_gui[1], "\n", 1);
            free(currentDir);
        }

        char* listFiles = ListFilesWithDetails();
        if (listFiles != NULL) 
        {
            write(pipe_to_gui[1], listFiles, strlen(listFiles));
            write(pipe_to_gui[1], "\n", 1); //구분자 추가
            free(listFiles);
        }
        
        //GUi에서 읽기기
        char response[MAX_CMD_SIZE];
        int n = read(pipe_from_gui[0], response, sizeof(response) - 1);
        if (n > 0)
        {
            response[n] = '\0';
            printf("GUI Response: %s\n", response);
        }

        tok_str = strtok(response, " \n");
        arg1 = strtok(NULL, " \n");
        arg2 = strtok(NULL, " \n");
        arg3 = strtok(NULL, " \n");

        if (tok_str == NULL)
            continue;

        if (strcmp(tok_str, "quit") == 0)
            break;

        else if (strcmp(tok_str, "cd") == 0)
            ChangeDirectory(arg1);

        else if (strcmp(tok_str, "mkdir") == 0)
            CreateDirectory(arg1);

        else if (strcmp(tok_str, "rmdir") == 0)
            RemoveDirectory(arg1);

        else if (strcmp(tok_str, "rename") == 0)
            RenameFileOrDirectory(arg1, arg2);

        else if (strcmp(tok_str, "rm") == 0)
            RemoveFile(arg1);

        else if (strcmp(tok_str, "chmod") == 0)
            ChangePermissions(arg1, arg2);

        else if (strcmp(tok_str, "cat") == 0)
            SendFileToGUI(arg1, pipe_to_gui[1]);

        else if(strcmp(tok_str, "watchFile") == 0)
            SendFileInfoToGUI(arg1, pipe_to_gui[1]);

        else if (strcmp(tok_str, "moveFile") == 0)
        {
            if(is_valid_directory(arg2))
            {
                size_t combined_length = strlen(arg1) + strlen(arg2) + 2;
                char* combined = (char*)malloc(combined_length);
                snprintf(combined, combined_length, "%s/%s", arg2, arg1);
                RenameFileOrDirectory(arg1, combined);
            }
        }

        else if (strcmp(tok_str, "cp") == 0)
            CopyFile(arg1, arg2);
        
        else
            printf("Unknown command: %s\n", tok_str);

    }
    return 0;
}