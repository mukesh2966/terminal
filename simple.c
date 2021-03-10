#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

int main()
{
    char *private_line;
    // char **command;
    while (1)
    {

        private_line = (char *)malloc(sizeof(char) * 1000);

        if (fgets(private_line, 1000, stdin) == NULL)
        {
            break;
        }

        printf("line : %s\n", private_line);
    }
}