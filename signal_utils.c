#include "signal_utils.h"
#include "directory_utils.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

void HandleCtrlC()
{
    printf("\n!!! (Ctrl + C) 입력이 들어왔습니다. 무시 처리하겠습니다. !!!\n");
    fflush(stdout);
    PrintCurrentDirectory();
    fflush(stdout);
    printf(" $ ");
    fflush(stdout);
}

void SetUpCtrlC()
{
    signal(SIGINT, HandleCtrlC);
}