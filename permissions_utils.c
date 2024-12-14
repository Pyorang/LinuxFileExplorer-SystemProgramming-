#include "permissions_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

mode_t ParseSymbolicMode(const char* symbolicMode, mode_t currentMode)
{
    if (strlen(symbolicMode) < 3)
    {
        printf("Error: Invalid symbolic mode format.\n");
        return (mode_t)-1;
    }

    char who = symbolicMode[0];
    char op = symbolicMode[1];
    const char* perms = symbolicMode + 2;

    if (strchr("ugoas", who) == NULL || strchr("+-=", op) == NULL)
    {
        printf("Error: Invalid symbolic mode format.\n");
        return (mode_t)-1;
    }

    mode_t mask = 0;

    for (const char* p = perms; *p != '\0'; ++p)
    {
        if (*p == 'r') mask |= S_IRUSR | S_IRGRP | S_IROTH;
        else if (*p == 'w') mask |= S_IWUSR | S_IWGRP | S_IWOTH;
        else if (*p == 'x') mask |= S_IXUSR | S_IXGRP | S_IXOTH;
        else
        {
            printf("Error: Invalid permission character '%c'.\n", *p);
            return (mode_t)-1;
        }
    }

    mode_t target = 0;
    if (who == 'u') target = mask & (S_IRWXU);
    else if (who == 'g') target = mask & (S_IRWXG);
    else if (who == 'o') target = mask & (S_IRWXO);
    else if (who == 'a') target = mask;

    if (op == '+')
        return currentMode | target;
    else if (op == '-')
        return currentMode & ~target;
    else if (op == '=')
        return (currentMode & ~mask) | target;

    return currentMode;
}

void ChangePermissions(const char* mode, const char* path)
{
    if (mode == NULL || path == NULL)
    {
        printf("Error: chmod requires file mode and file name arguments.\n");
        return;
    }

    struct stat st;
    if (stat(path, &st) != 0)
    {
        perror("stat failed");
        return;
    }

    mode_t newMode;

    if (isdigit(mode[0]))
    {
        char* end;
        newMode = strtol(mode, &end, 8);
        if (*end != '\0' || newMode > 0777 || newMode < 0)
        {
            printf("Error: Invalid octal mode '%s'.\n", mode);
            return;
        }
    }
    else
    {
        newMode = ParseSymbolicMode(mode, st.st_mode);
        if (newMode == (mode_t)-1)
        {
            return;
        }
    }

    if (chmod(path, newMode) != 0)
    {
        perror("chmod failed");
        return;
    }

    printf("Permissions of '%s' updated successfully.\n", path);
}