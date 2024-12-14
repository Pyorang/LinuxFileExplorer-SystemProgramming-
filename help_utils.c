#include "help_utils.h"
#include <stdio.h>

void PrintHelp()
{
    printf("===== ��ɾ� ��� =====\n");
    printf("help                   : ��ɾ� ��ϰ� ��� ������ �����ݴϴ�.\n");
    printf("\n");
    printf("cd <path>              : ���� ���丮�� <path> ��η� �̵��մϴ�.\n");
    printf("\n");
    printf("mkdir <path>           : <path>�� �ش��ϴ� ���丮�� �����մϴ�.\n");
    printf("rmdir <path>           : <path>�� �ش��ϴ� ���丮�� �����մϴ�.\n");
    printf("\n");
    printf("rename <source> <target>: <source> �̸��� <target> �̸����� �����մϴ�.\n");
    printf("\n");
    printf("ls                     : ���� ���丮�� ����(���� �� ���丮 ���)�� ����մϴ�.\n");
    printf("ls -l                  : ���� ���丮�� ����(���� �̸�, ����, ��ũ ��, UID, PID, ���� ũ��, ���� ���� �ð�)�� �� �ڼ��� ����մϴ�.\n");
    printf("\n");
    printf("ln <original> <new>    : <original> ������ �ϵ� ��ũ�� <new>�� �����մϴ�.\n");
    printf("ln -s <target> <link>  : <target> ������ �ɺ��� ��ũ�� <link>�� �����մϴ�.\n");
    printf("\n");
    printf("rm <file>              : <file>�� �����մϴ�. (��, ���丮�� ����)\n");
    printf("\n");
    printf("chmod <mode> <file>    : <file>�� ���� ������ <mode>�� �����մϴ�.\n");
    printf("\n");
    printf("cat <file>             : <file>�� ������ ����մϴ�.\n");
    printf("\n");
    printf("cp <source> <dest>     : <source> ������ <dest>�� �����մϴ�.\n");
    printf("\n");
    printf("exec <program>         : �ܺ� ���α׷� <program>�� �����մϴ�.\n");
    printf("\n");
    printf("ps                     : ���� ���� ���� ���μ��� ����� ����մϴ�.\n");
    printf("\n");
    printf("kill <pid>             : <pid>�� SIGTERM ��ȣ�� ���� ���μ����� �����մϴ�.\n");
    printf("kill -<signal> <pid>   : <pid>�� Ư�� ��ȣ�� �����ϴ�. ��: -9(SIGKILL).\n");
    printf("========================\n");
}