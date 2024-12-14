#include "signal_utils.h"
#include "directory_utils.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

void HandleCtrlC()
{
    printf("\n!!! (Ctrl + C) �Է��� ���Խ��ϴ�. ���� ó���ϰڽ��ϴ�. !!!\n");
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