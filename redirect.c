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
    FILE *fp;

    printf("This text is redirected to stdout\n");

    char *user_input = (char *)malloc(sizeof(char) * 100);

    fp = freopen("file.txt", "r", stdin);
    // fgets(user_input, 100, stdin);
    scanf("%s", user_input);

    // char str1[20];

    printf("k%sk\n", user_input);

    // printf("This text is redirected to file.txt\n");
    fclose(fp);

    return (0);
}
