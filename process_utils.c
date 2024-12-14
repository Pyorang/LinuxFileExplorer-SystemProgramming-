#include "process_utils.h"
#include <stdio.h>
#include <stdlib.h>     
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <dirent.h>   
#include <string.h>

void ExecProgram(const char* program)
{
    if (program == NULL)
    {
        printf("Error: exec requires a program name.\n");
        return;
    }

    pid_t pid = fork();
    if (pid == -1)
        perror("fork failed");

    else if (pid == 0)
    {
        execlp(program, program, NULL);
        perror("execlp failed");
        exit(1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status))
            printf("Program exited with status %d\n", WEXITSTATUS(status));

    }
}

void ListProcesses()
{
    DIR* proc_dir = opendir("/proc");
    if (proc_dir == NULL)
    {
        perror("opendir /proc failed");
        return;
    }

    struct dirent* entry;
    printf("PID\tCMD\n");

    while ((entry = readdir(proc_dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0)
        {
            if (strlen(entry->d_name) > 255)
            {
                fprintf(stderr, "Warning: Process name too long: %s\n", entry->d_name);
                continue;
            }

            char cmd_path[512];
            snprintf(cmd_path, sizeof(cmd_path), "/proc/%s/comm", entry->d_name);

            FILE* cmd_file = fopen(cmd_path, "r");
            if (cmd_file)
            {
                char cmd[256];
                if (fgets(cmd, sizeof(cmd), cmd_file) != NULL)
                {
                    printf("%s\t%s", entry->d_name, cmd);
                }
                fclose(cmd_file);
            }
        }
    }

    closedir(proc_dir);
}

void KillProcess(const char* arg1, const char* arg2)
{
    if(arg1 == NULL)
    {
        printf("Error: kill requires at least a PID argument.\n");
        return;
    }

    int signal = SIGTERM;
    pid_t pid;

    if (arg1[0] == '-')
    {
        signal = atoi(arg1 + 1);
        if (arg2 == NULL)
        {
            printf("Error: kill requires a PID argument after the signal.\n");
            return;
        }
        pid = atoi(arg2);
    }
    else
        pid = atoi(arg1);


    if (kill(pid, signal) == -1)
        perror("kill failed");
    else
        printf("Signal %d sent to process %d\n", signal, pid);
}