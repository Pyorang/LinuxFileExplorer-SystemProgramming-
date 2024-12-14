#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <sys/types.h>

void ExecProgram(const char* program);
void ListProcesses();
void KillProcess(const char* arg1, const char* arg2);

#endif
