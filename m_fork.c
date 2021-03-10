#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

void exec_cmd_redirect()
{
    pid_t pid_arr[30];
    int i;
    int n = 10 + 1;

    for (i = 0; i < n; i++)
    {
        if ((pid_arr[i] = fork()) < 0)
        {
            printf("Fork failed.\n");
            _exit(1);
        }
        else if (pid_arr[i] == 0)
        {

            //exec vp
            printf("i am a child with pid : %d with parentID : %d\n", getpid(), getppid());
            _exit(1);
        }
    }

    pid_t pid;
    i = 0;
    while (i < n)
    {
        pid = waitpid(pid_arr[i], NULL, 0);
        printf("this has waited for child with pid : %d \n", pid);
        i++;
    }
}
int main()
{
    exec_cmd_redirect();
}