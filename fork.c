#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#define MAX_LEN 1000

int *commands_executed;
int *flag_double;
int *flag_single;
int *and_counter;
int *bg_flag;
int *pid_main;

/*To count the no. of spaces or tabs*/
int spaces(char *str)
{
    int no_of_spaces = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == ' ' || str[i] == '\t')
        {
            no_of_spaces += 1;
        }
    }

    return no_of_spaces;
}

/*for catching & in the cmd*/
int set_bg_flag(char **command, int counter)
{

    *(bg_flag) = 0;
    char *string = command[counter - 1];
    // printf("k%sk\n", string);
    if (strcmp(string, "&") == 0)
    {

        *(bg_flag) = 1;

        if (counter == 1)
        {
            printf("Unexpected token `&`\n");
        }
    }

    return counter;
}

/*To make arguments array that will be used by execvp*/
int tokenizer(char *private_line, char **command)
{

    int counter = 0;
    char *iter;
    char *start;
    char *temp_char = (char *)malloc(sizeof(char) * 2);

    iter = private_line;
    start = private_line;

    // int flag_double_private = 0;

    char surr_comma = '1';

    while (*iter != '\n' && *iter != '\0')
    {
        if (*iter == '"' && surr_comma == '1')
        {
            // flag_double_private = !flag_double_private;
            surr_comma = '"';
            *(flag_double) = 1;
            // start += 1;
        }
        else if (*iter == '\'' && surr_comma == '1')
        {
            surr_comma = '\'';
            *(flag_single) = 1;
            // start += 1;
        }
        else if (*iter == '"' && surr_comma == '"')
        {
            if (*(iter - 1) == '\\')
            {
                //nothing
            }
            else
            {
                surr_comma = '1';
                start += 1;
                *iter = '\0';
                command[counter] = start;
                counter += 1;
                printf("counter1 : %d, command : %s", counter, command[counter - 1]);
                start = iter + 1;
            }
        }
        else if (*iter == '\'' && surr_comma == '\'')
        {
            if (*(iter - 1) == '\\')
            {
                //nothing
            }
            else
            {
                surr_comma = '1';
                start += 1;
                *iter = '\0';
                command[counter] = start;
                counter += 1;
                printf("counter2 : %d, command : %s", counter, command[counter - 1]);
                start = iter + 1;
            }
        }

        else if ((*iter == '>' || *iter == '&' || *iter == '|' || *iter == '<') && surr_comma == '1')
        {
            temp_char[0] = *iter;
            temp_char[1] = '\0';
            *iter = '\0';

            if (*iter == *start)
            {
                command[counter] = temp_char;
                counter += 1;
                start = iter + 1;
            }
            else
            {
                command[counter] = start;
                start = iter + 1;
                counter += 1;
                printf("counter3 : %d, command : %s", counter, command[counter - 1]);
                // **(command + counter) = temp_char;
                command[counter] = temp_char;
                counter += 1;
                printf("counter4 : %d, command : %s", counter, command[counter - 1]);
            }
        }

        else if ((*iter == ' ' || *iter == '\t') && surr_comma == '1')
        {
            if ((int)(iter - start) == 0)
            {
                /*simply move start also by 1*/
                start += 1;
            }
            else
            {
                *iter = '\0';
                // start = private_line;
                command[counter] = start;
                counter += 1;
                printf("counter5 : %d, command : %s", counter, command[counter - 1]);
                start = iter + 1;
            }
        }
        else
        {
            // printf("this is not space: %c\n", *iter);
        }
        iter += 1;
    }
    if ((int)(iter - start) != 0)
    {
        // printf("herer...");
        *iter = '\0';
        // start = private_line;
        command[counter] = start;
        counter += 1;
        printf("counter6 : %d, command : %s", counter, command[counter - 1]);
    }
    command[counter] = NULL;

    if (counter != 0)
    {

        counter = set_bg_flag(command, counter);
    }
    else
    {

        *(bg_flag) = 0;
    }

    return counter;
}

int main()
{
    commands_executed = (int *)malloc(sizeof(int));
    *(commands_executed) = 0;

    flag_double = (int *)malloc(sizeof(int));
    flag_single = (int *)malloc(sizeof(int));
    and_counter = (int *)malloc(sizeof(int));
    bg_flag = (int *)malloc(sizeof(int));
    pid_main = (int *)malloc(sizeof(int));
    // ampercent = (char *)malloc(sizeof(char) * 2);

    *(flag_double) = 0;
    *(flag_single) = 0;
    *(and_counter) = -1;
    *(bg_flag) = 0;
    *(pid_main) = getpid();

    char *user_input;

    char **command;

    user_input = (char *)malloc(sizeof(char) * MAX_LEN);

    int t = 6;
    while (t--)
    {
        printf("Type here : ");

        fgets(user_input, MAX_LEN, stdin);

        if (!strcmp(user_input, "\0"))
        {
            break;
        }

        int no_of_spaces = spaces(user_input);

        command = (char **)malloc(sizeof(char *) * (no_of_spaces + 2));

        int counter = tokenizer(user_input, command);

        for (int i = 0; i < counter; i++)
        {
            printf("K%sK ", command[i]);
        }
        printf("counter is : %d", counter);
        printf("\n");
    }
    char k2 = '\"';
    printf("K%cK", k2);

    char *st = (char *)malloc(sizeof(char) * 100);

    scanf("%s", st);

    char *t1 = st;

    while (*t1 != '\0')
    {
        printf("this is t1 : %c\n", *t1);
        t1 += 1;
    }
}