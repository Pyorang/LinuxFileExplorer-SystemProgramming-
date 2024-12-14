#include "help_utils.h"
#include <stdio.h>

void PrintHelp()
{
    printf("===== 명령어 목록 =====\n");
    printf("help                   : 명령어 목록과 기능 설명을 보여줍니다.\n");
    printf("\n");
    printf("cd <path>              : 현재 디렉토리를 <path> 경로로 이동합니다.\n");
    printf("\n");
    printf("mkdir <path>           : <path>에 해당하는 디렉토리를 생성합니다.\n");
    printf("rmdir <path>           : <path>에 해당하는 디렉토리를 삭제합니다.\n");
    printf("\n");
    printf("rename <source> <target>: <source> 이름을 <target> 이름으로 변경합니다.\n");
    printf("\n");
    printf("ls                     : 현재 디렉토리의 내용(파일 및 디렉토리 목록)을 출력합니다.\n");
    printf("ls -l                  : 현재 디렉토리의 내용(파일 이름, 권한, 링크 수, UID, PID, 파일 크기, 파일 생성 시간)을 더 자세히 출력합니다.\n");
    printf("\n");
    printf("ln <original> <new>    : <original> 파일의 하드 링크를 <new>로 생성합니다.\n");
    printf("ln -s <target> <link>  : <target> 파일의 심볼릭 링크를 <link>로 생성합니다.\n");
    printf("\n");
    printf("rm <file>              : <file>을 삭제합니다. (단, 디렉토리는 제외)\n");
    printf("\n");
    printf("chmod <mode> <file>    : <file>의 접근 권한을 <mode>로 변경합니다.\n");
    printf("\n");
    printf("cat <file>             : <file>의 내용을 출력합니다.\n");
    printf("\n");
    printf("cp <source> <dest>     : <source> 파일을 <dest>로 복사합니다.\n");
    printf("\n");
    printf("exec <program>         : 외부 프로그램 <program>을 실행합니다.\n");
    printf("\n");
    printf("ps                     : 현재 실행 중인 프로세스 목록을 출력합니다.\n");
    printf("\n");
    printf("kill <pid>             : <pid>에 SIGTERM 신호를 보내 프로세스를 종료합니다.\n");
    printf("kill -<signal> <pid>   : <pid>에 특정 신호를 보냅니다. 예: -9(SIGKILL).\n");
    printf("========================\n");
}