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

// char *ampercent;

struct trex
{
    int filled;
    pid_t pid;
    int is_back;
    char **command;
};
struct trex *records;

struct symbols
{
    int index;
    char *symb;
};

/*for storing current running process*/
struct Node
{
    char **command;
    pid_t pid;
    struct Node *next;
};
struct Node *head;

char *string_add(char *string1, char *string2)
{
    int str1_len = strlen(string1);
    char *iter1 = string1 + str1_len;
    char *iter2 = string2;
    while (*iter2 != '\0')
    {
        *iter1 = *iter2;
        iter1 += 1;
        iter2 += 1;
    }
    *iter1 = '\0';
    return string1;
}

void pid_func(int counter, char **command)
{
    if (*(bg_flag) == 1)
    {
        printf("[!][!][!] `pid`, `pid all` and `pid current`, these commands cannot be run in background.\n[!][!][!] Running the command in foreground.\n");
    }
    if (counter < 2)
    {
        printf("[-][-][-] Pid of this shell : %d\n", (int)getpid());
    }
    else if (!strcmp("all\0", command[1]))
    {
        printf("[-][-][-] Below are all the processes spawned in this shell.\n");

        for (int i = 0; i < *(commands_executed); i++)
        {
            printf("%d. ProcessId : %-15d", i + 1, (int)records[i].pid);
            printf("Command :");

            for (int j = 0; j < records[i].filled; j++)
            {
                printf(" %s", (records + i)->command[j]);
            }
            printf("\n");
        }
    }
    else if (!strcmp("current\0", command[1]))
    {
        printf("[-][-][-] Below are all the currently running processes in this shell\n");
        if (head == NULL)
        {
            printf("[!][!][!] No process is currently running.\n");
        }
        else
        {
            struct Node *starting = head;
            int l = 1;

            while (starting != NULL)
            {
                printf("%d. ProcessId : %-15d", l, (int)starting->pid);
                printf("Command :");
                char **cmd1 = starting->command;
                while (*cmd1 != NULL)
                {
                    printf(" %s", *(cmd1));
                    cmd1 = cmd1 + 1;
                }
                printf("\n");

                starting = starting->next;
                l += 1;
            }
        }
    }
    /*Saving command to history*/
    (records + *(commands_executed))->filled = counter;
    (records + *(commands_executed))->command = command;
    (records + *(commands_executed))->pid = getpid();
    (records + *(commands_executed))->is_back = *(bg_flag);
    *(commands_executed) += 1;

    *(flag_double) = 0;
    *(flag_single) = 0;
    *(and_counter) = -1;
}

void cd_func(int counter, char *current_dir, char *login_name, char **command)
{
    if (counter > 2)
    {
        printf("[!][!][!] Too many arguments for cd\n");
    }
    else
    {
        if (*(bg_flag) == 1)
        {
            printf("[!][!][!] Cannot run `cd` command in background, running it in foreground.\n");
        }
        if (counter == 1)
        {
            char *home1 = "/home/";
            char *current_dir1 = current_dir;
            while (*home1 != '\0')
            {
                *current_dir1 = *home1;
                current_dir1 += 1;
                home1 += 1;
            }
            *current_dir1 = '\0';

            current_dir = string_add(current_dir, login_name);
            if (chdir("/") < 0)
            {
                printf("[!][!][!] No such file or directory found.\n");
            };
            if (chdir(current_dir) < 0)
            {
                printf("[!][!][!] No such file or directory found.\n");
            };

            // strcpy(current_dir, string_add("home", login_name));
        }
        else
        {
            if (chdir(command[1]) < 0)
            {
                printf("[!][!][!] No such file or directory found.\n");
            }
            getcwd(current_dir, 150);
            // printf("this is change : %s\n", current_dir);
        }
    }
    /*Saving command to history*/
    (records + *(commands_executed))->filled = counter;
    (records + *(commands_executed))->command = command;
    (records + *(commands_executed))->pid = getpid();
    (records + *(commands_executed))->is_back = *(bg_flag);
    *(commands_executed) += 1;

    *(flag_double) = 0;
    *(flag_single) = 0;
    *(and_counter) = -1;
}

void sigint_handler(int signo)
{
    printf("\nCaught SIGINT -- Interupting current running command.\n");
}

void sigint_handler_main(int signo)
{
    printf("\nCaught SIGINT -- Exiting forcefully.\n");
    struct Node *kill_head = head;
    while (head != NULL)
    {
        kill(kill_head->pid, SIGKILL);
        head = kill_head->next;
        free(kill_head);
        kill_head = head;
    }
    _exit(1);
}

/*For calculating length of interger*/
int int_len(int num)
{
    int len = 0;
    while (num)
    {
        num = num / 10;
        len++;
    }

    return len;
}

void deleteNode(int pid)
{
    // Store head node
    struct Node *temp = head, *prev;

    // If head node itself holds the key to be deleted
    if (temp != NULL && temp->pid == pid)
    {
        head = temp->next; // Changed head
        free(temp);        // free old head
        printf("Child with Pid :%d exited.\n", pid);

        return;
    }

    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && temp->pid != pid)
    {
        prev = temp;
        temp = temp->next;
    }

    // If key was not present in linked list
    if (temp == NULL)
        return;

    // Unlink the node from linked list
    prev->next = temp->next;

    free(temp); // Free memory
    printf("Child with Pid :%d exited.\n", pid);
}

void handler(int sig, siginfo_t *sip, void *notused)
{

    int status;
    status = 0;
    // pid_t child_pid = (pid_t)info->si_pid;
    // waitpid(child_pid, NULL, 0);
    pid_t pid = waitpid(sip->si_pid, &status, WNOHANG);

    if (pid <= 0)
    {
        /**/
    }
    else
    {
        deleteNode(pid);
    }
    fflush(stdout);
}

void exec_cmd_redirect(int argCounter, char **command, struct symbols separators[30], int separator_index)
{
    pid_t pid_arr[30];
    int i;
    int not_there = 0;
    int n = separator_index;

    for (i = 0; i < n; i++)
    {
        // signal(SIGCHLD, SIG_IGN);
        if ((pid_arr[i] = fork()) < 0)
        {
            printf("Fork failed.\n");
            _exit(1);
        }
        else if (pid_arr[i] == 0)
        {
            if (strcmp(separators[i].symb, "<\0") == 0)
            {
                FILE *fp;

                fp = freopen(*(command + separators[i].index + 1), "r", stdin);
                printf("switched to taking input from file.and chilk_pid is : %d \n", getpid());

                int addition;
                if (i == 0)
                {
                    addition = 0;
                }
                else
                {
                    addition = separators[i - 1].index + 1;
                }

                if (i + 1 < n && strcmp(separators[i + 1].symb, ">\0") == 0)
                {

                    FILE *fp1;
                    pid_t g_child_pid;

                    if ((g_child_pid = fork()) < 0)
                    {
                        printf("Fork failed.\n");
                        _exit(1);
                    }
                    else if (g_child_pid == 0)
                    {
                        fflush(stdout);
                        fp1 = freopen(*(command + separators[i + 1].index + 1), "w", stdout);
                        execvp(command[0 + addition], command);
                        // fclose(fp);
                        // fclose(fp1);
                        printf("NOT A VALID CMD\n");

                        _exit(1);
                    }
                    else
                    {
                        waitpid(g_child_pid, NULL, 0);
                        fflush(stdout);
                        i += 1;
                        // pid_arr[i] = -1;
                        not_there += 1;
                        fclose(fp1);
                    }
                }
                else
                {
                    execvp(command[0 + addition], command);
                    // fclose(fp);

                    // printf("NOT A VALID CMD\n");

                    _exit(1);
                }

                fclose(fp);
            }
            else if (strcmp(separators[i].symb, ">\0") == 0)
            {
                FILE *fp;
                printf("this is i :%d\n", i);
                fp = freopen(*(command + separators[i].index + 1), "w", stdout);

                printf("switched to shooting output to file. and chilt_pid is : %d \n", getpid());

                int addition;
                if (i == 0)
                {
                    addition = 0;
                }
                else
                {
                    addition = separators[i - 1].index + 1;
                }

                execvp(command[0 + addition], command);
                fclose(fp);
                // printf("NOT A VALID CMD\n");

                _exit(1);
            }
            else if (strcmp(separators[i].symb, "|\0") == 0)
            {
                FILE *fp;
                // FILE *fp1;

                fp = freopen(*(command + separators[i].index + 1), "w", stdout);
                printf("switched to taking input from file.and chils_pid is : %d \n", getpid());

                int addition;
                if (i == 0)
                {
                    addition = 0;
                }
                else
                {
                    addition = separators[i - 1].index + 1;
                }

                execvp(command[0 + addition], command);
                fclose(fp);

                // printf("NOT A VALID CMD\n");

                _exit(1);
            }

            //exec vp

            _exit(1);
        }
        else
        {
        }
    }

    i = 0;
    pid_t pid;

    while (i < n - not_there)
    {

        pid = waitpid(pid_arr[i], NULL, 0);
        printf("this has waited for child with pid : %d \n", pid);
        *(command + separators[i].index) = separators[i].symb;
        i++;
    }
    /*Saving command to history*/
    (records + *(commands_executed))->filled = argCounter;
    (records + *(commands_executed))->command = command;
    (records + *(commands_executed))->pid = getpid(); // this is wrong here..later need to design for each process
    (records + *(commands_executed))->is_back = *(bg_flag);
    *(commands_executed) += 1;

    *(flag_double) = 0;
    *(flag_single) = 0;
}

/*Function that use execvp to execute commands*/
void exec_cmd(int argCounter, char **command)
{

    // printf("process within parent : %d", getpid());
    fflush(stdout); //-------------may be problematic

    // printf("pid parent1 : %d", getpid());

    if (*flag_single || *flag_double)
    {
        printf("[~][~] Message [~][~]\n[~][~] Always use <\"> for enclosing a string and <'> inside the string.\n[~][~] <\"> are not allowed inside a string and <\'> are not allowed for enclosing a string.\n");
    }

    pid_t pid;
    if ((pid = fork()) < (pid_t)0)
    {
        printf("Fork failed.\n");
        _exit(1);
    }
    else if (pid == (pid_t)0)
    {

        if (*bg_flag == 0)
        {
            printf("Executing :");
            for (int i = 0; i < argCounter; i++)
            {
                printf(" %s", command[i]);
            }
            printf("\n");
        }
        else
        {
            // printf("this is the pid : %d", getpid());
            setpgid(0, 0);
        }

        execvp(command[0], command);
        printf("[!][!][!] Please provide a valid command.\n");
        printf("\"");
        for (int i = 0; i < argCounter - 1; i++)
        {
            printf("%s ", command[i]);
        }
        printf("%s\" ", command[argCounter - 1]);
        printf("is not a valid command.\n");

        _exit(1);
    }
    else
    {
        signal(SIGINT, sigint_handler);

        if (*bg_flag == 0)
        {
            waitpid(pid, NULL, 0);

            printf("Done with :");
            for (int i = 0; i < argCounter; i++)
            {
                printf(" %s", command[i]);
            }
            printf("\n");
        }
        else
        {

            if (head == NULL)
            {
                head = (struct Node *)malloc(sizeof(struct Node));
                head->command = command;
                head->pid = pid;
                head->next = NULL;
            }
            else
            {
                struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));
                new_node->command = command;
                new_node->pid = pid;
                new_node->next = head;
                head = new_node;
            };
        }

        /*Saving command to history*/
        (records + *(commands_executed))->filled = argCounter;
        (records + *(commands_executed))->command = command;
        (records + *(commands_executed))->pid = pid;
        (records + *(commands_executed))->is_back = *(bg_flag);
        *(commands_executed) += 1;

        *(flag_double) = 0;
        *(flag_single) = 0;
        *(and_counter) = -1;
        // *(bg_flag) = 0;
    }
}

void exec_decider(int argCounter, char **command)
{
    struct symbols separators[30];
    int separator_index = 0;
    for (int i = 0; i < argCounter; i++)
    {
        if (!strcmp(command[i], "<") || !strcmp(command[i], ">") || !strcmp(command[i], "|"))
        {
            separators[separator_index].index = i;
            separators[separator_index].symb = command[i];
            separator_index += 1;
            command[i] = NULL;
        }
    }

    if (separator_index > 0)
    {
        //use exec_cmd_redirect
        exec_cmd_redirect(argCounter, command, separators, separator_index);
    }
    else
    {
        //use exec_cmd_normal
        exec_cmd(argCounter, command);
    }
}

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
    // if (string[strlen(string) - 1] == '&')
    // {
    //     // if (counter - 1 == *and_counter)
    //     // {
    //     //     // continue;
    //     // }

    //     // else
    //     // {

    if (!strcmp(string, "&"))
    {
        *(bg_flag) = 1;
        counter = counter - 1;
        /*when only & is given*/
        if (counter == 0)
        {
            printf("Unexpected token `&`\n");
        }

        command[counter] = NULL;
    }
    // else
    // {
    //     string[strlen(string) - 1] = '\0';
    //     command[counter - 1] = string;
    //     // printf("k%sk\n", command[counter - 1]);
    // }
    // }
    // }

    // printf("k%sk\n", string);

    return counter;
}

void shift_char(char **iter)
{
    char *shift = *iter;
    int kj = 0;
    while (*shift != '\0')
    {
        *(*iter + kj - 1) = *(shift);
        kj += 1;
        shift += 1;
    }
    *(*iter + kj - 1) = *(shift);
}

/*To make arguments array that will be used by execvp*/
int tokenizer(char *private_line, char **command)
{

    int counter = 0;
    char *iter;
    char *start;
    char *temp_char;

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
            if (*(iter - 1) == '\\')
            {
                //nothing ~ just shifting
                shift_char(&iter);
            }
        }
        else if (*iter == '\'' && surr_comma == '1')
        {
            surr_comma = '\'';
            *(flag_single) = 1;
            // start += 1;
            if (*(iter - 1) == '\\')
            {
                //nothing ~ just shifting
                shift_char(&iter);
            }
        }
        else if (*iter == '"' && surr_comma == '"')
        {
            if (*(iter - 1) == '\\')
            {
                //nothing ~ just shifting
                shift_char(&iter);
            }
            else
            {
                surr_comma = '1';
                start += 1;
                *iter = '\0';
                command[counter] = start;
                counter += 1;
                // printf("counter1 : %d, command : %s", counter, command[counter - 1]);
                start = iter + 1;
            }
        }
        else if (*iter == '\'' && surr_comma == '\'')
        {
            if (*(iter - 1) == '\\')
            {
                //nothing ~ just shifting
                shift_char(&iter);
            }
            else
            {
                surr_comma = '1';
                start += 1;
                *iter = '\0';
                command[counter] = start;
                counter += 1;
                // printf("counter2 : %d, command : %s", counter, command[counter - 1]);
                start = iter + 1;
            }
        }
        else if (*iter == '"' && surr_comma == '\'')
        {
            if (*(iter - 1) == '\\')
            {
                //nothing ~ just shifting
                shift_char(&iter);
            }
        }
        else if (*iter == '\'' && surr_comma == '"')
        {
            if (*(iter - 1) == '\\')
            {
                //nothing ~ just shifting
                shift_char(&iter);
            }
        }

        else if ((*iter == '>' || *iter == '&' || *iter == '|' || *iter == '<') && surr_comma == '1')
        {
            temp_char = (char *)malloc(sizeof(char) * 2);
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
                // printf("counter3 : %d, command : %s", counter, command[counter - 1]);
                // **(command + counter) = temp_char;
                command[counter] = temp_char;
                counter += 1;
                // printf("counter4 : %d, command : %s", counter, command[counter - 1]);
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
                // printf("counter5 : %d, command : %s", counter, command[counter - 1]);
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
        // printf("counter6 : %d, command : %s", counter, command[counter - 1]);
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

    for (int i = 0; i < counter; i++)
    {
        printf("These are tokens : %s\n", command[i]);
    }

    return counter;
}

void read_cmd(char *file_name)
{
    FILE *stream;

    /* opening file for reading */
    stream = fopen(file_name, "r");
    if (stream == NULL)
    {
        printf("[!][!][!] Error : %s does not exist.\n", file_name);
        // exit(-1);
        return;
    }

    char *private_line;
    char **command;
    while (1)
    {

        private_line = (char *)malloc(sizeof(char) * MAX_LEN);

        if (fgets(private_line, MAX_LEN, stream) == NULL)
        {
            break;
        }

        int no_of_spaces = spaces(private_line);

        // /*no of arguments <= no_of_spaces+1 always*/
        command = (char **)malloc(sizeof(char *) * (no_of_spaces + 2));

        int counter = tokenizer(private_line, command);

        exec_decider(counter, command);
    }
    fclose(stream);
}

int show_hist(int *hist_no, char **command)
{
    int val = 0;
    char *h = command[0];
    if (*h == 'H' && *(h + 1) == 'I' && *(h + 2) == 'S' && *(h + 3) == 'T')
    {
        val = 1;
    }

    h = h + 4;
    *hist_no = atoi(h);
    return val;
}

int exec_hist(int *hist_no, char **command)
{
    int val = 0;
    char *h = command[0];
    if (*h == '!' && *(h + 1) == 'H' && *(h + 2) == 'I' && *(h + 3) == 'S' && *(h + 4) == 'T')
    {
        val = 1;
    }
    h = h + 5;
    *hist_no = atoi(h);
    return val;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("[!][!][!] Please provide atleast one file with the executable.\nUsage: ./shell <file1> <file2> ...\n");

        return 0;
    }

    // printf("process within main : %d", getpid());

    printf("[~][~] Message [~][~]\n[~][~] Always use <\"> for enclosing a string and <'> inside the string.\n[~][~] <\"> are not allowed inside a string and <\'> are not allowed for enclosing a string.\n");

    records = (struct trex *)malloc(sizeof(struct trex) * MAX_LEN);
    head = NULL;

    // signal(SIGCHLD, handler);
    struct sigaction action;
    action.sa_sigaction = handler; /* Note use of sigaction, not handler */
    sigfillset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO; /* Note flag - otherwise NULL in function */

    sigaction(SIGCHLD, &action, NULL);

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

    for (int i = 1; i < argc; i++)
    {
        printf("EXECUTING commands of <file%d> :\n", i);
        read_cmd(argv[i]);
        printf("-------------------Done with <file%d>-----------------------\n", i);
    }

    printf("-----------------------------------------------------------\n[!][!][!] Done with all the files.\n[!][!][!] Moving to interactive mode.\n");

    printf("\n[i][i][i] To view additionally available user commands type `help`\n\n");

    char *user_input;

    char **command;

    char *show_term = (char *)malloc(sizeof(char) * 30);

    // char show_term[30];
    getlogin_r(show_term, 30);
    // printf("show_term : %s\n", show_term);

    char *login_name = (char *)malloc(sizeof(char) * 30);
    getlogin_r(login_name, 30);

    char *hname = (char *)malloc(sizeof(char) * 30);
    // char hname[30];
    gethostname(hname, 30);
    // printf("hname : %s", hname);

    show_term = string_add(show_term, "@");
    show_term = string_add(show_term, hname);
    free(hname);
    show_term = string_add(show_term, ":");

    // printf("show_term : k%sk\n", show_term);

    char *current_dir = (char *)malloc(sizeof(char) * 150);
    // char current_dir[150];
    getcwd(current_dir, 150);

    char *tip = (char *)malloc(sizeof(char) * 200);

    int *hist_no = (int *)malloc(sizeof(int));

    while (1)
    {
        signal(SIGINT, sigint_handler_main);
        char *start1 = show_term;
        char *tip1 = tip;
        while (*start1 != '\0')
        {
            *tip1 = *start1;
            tip1 += 1;
            start1 += 1;
        }
        *tip1 = '\0';

        // printf("this is tip : %s\n", tip);
        // tip = show_term;

        tip = string_add(tip, current_dir);
        tip = string_add(tip, "~>");

        if (strcmp(user_input, "\0") != 0)
        {
            printf("%s : ", tip);
            fflush(stdout);
        }

        user_input = (char *)malloc(sizeof(char) * MAX_LEN);

        fgets(user_input, MAX_LEN, stdin);

        int no_of_spaces = spaces(user_input);

        command = (char **)malloc(sizeof(char *) * (no_of_spaces + 2));

        int counter = tokenizer(user_input, command);

        if (command[0] == NULL)
        {
            // printf("\n");
            continue;
        }

        if (strcmp(command[0], "help") == 0)
        {
            printf("\nBelow is the list of additionally available user commands :\n");
            printf("%-35s", "1) HISTn ");
            printf(": Will print the last `n` commands.\n");
            printf("%-35s", "2) !HISTn ");
            printf(": Execute the `n`th command from history.\n");

            printf("%-35s", "3) pid ");
            printf(": Will print the process-id of the shell.\n");

            printf("%-35s", "4) pid all ");
            printf(": Will print the process-id of all the commands executed till now.\n");

            printf("%-35s", "5) cd ");
            printf(": To change the directory.\n");

            printf("%-35s", "6) pid current");
            printf(": Will print the process-id of all currently active processes.\n");

            printf("%-35s", "7) HISTORY BRIEF ");
            printf(": For short description of command history.\n");
            printf("%-35s", "8) HISTORY FULL ");
            printf(": For full description of command history.\n");
            printf("%-35s", "9) EXEC <COMMAND_NAME> ");
            printf("%-35s", ": For executing a terminal command.\n");
            printf("%-35s", "10) EXEC <COMMAND_INDEX_NUMBER> ");
            printf(": For executing a command by its index in cmd_history.\n\n");
            continue;
        }

        *hist_no = -1;
        if (show_hist(hist_no, command) && strcmp(command[0], "HISTORY") != 0)
        {
            if (*(bg_flag) == 1)
            {
                printf("[!][!][!] Cannot run `HISTn` in background, running it in foreground.\n");
            }
            if (*hist_no <= 0)
            {
                printf("[!][!][!] To view commands from history, please provide a valid no.\n");
                continue;
            }
            // printf("this is the received show_hist_no : %dk\n", *hist_no);
            // int start_cmd = *(commands_executed) - *(hist_no);
            if (*hist_no < *commands_executed)
            {
                printf("[-][-][-] Last %d commands were :\n", *hist_no);
                printf("    :   \n");
                printf("    :   \n");

                for (int i = *commands_executed - *hist_no; i < *commands_executed; i++)
                {
                    printf(" %d.", i + 1);
                    for (int j = 0; j < records[i].filled; j++)
                    {
                        printf(" %s", (records + i)->command[j]);
                    }
                    if ((records + i)->is_back == 1)
                    {
                        printf(" &");
                    }
                    printf("\n");
                }
            }
            else if (*hist_no == *commands_executed)
            {
                printf("[-][-][-] Last %d commands are :\n", *hist_no);

                for (int i = *commands_executed - *hist_no; i < *commands_executed; i++)
                {
                    printf(" %d.", i + 1);
                    for (int j = 0; j < records[i].filled; j++)
                    {
                        printf(" %s", (records + i)->command[j]);
                    }
                    if ((records + i)->is_back == 1)
                    {
                        printf(" &");
                    }
                    printf("\n");
                }
            }
            else
            {
                printf("[-][-][-] Total commands executed till now are :%d ,i.e. less than %d, so printing the last %d commands :\n", *(commands_executed), *hist_no, *(commands_executed));
                for (int i = 0; i < *commands_executed; i++)
                {
                    printf(" %d.", i + 1);
                    for (int j = 0; j < records[i].filled; j++)
                    {
                        printf(" %s", (records + i)->command[j]);
                    }
                    if ((records + i)->is_back == 1)
                    {
                        printf(" &");
                    }
                    printf("\n");
                }
            }
        }
        else if (exec_hist(hist_no, command))
        {
            // printf("this is the received exec_hist_no : %dk\n", *hist_no);
            if (*hist_no <= 0 || *hist_no > *commands_executed)
            {
                printf("[!][!][!] To execute a command from history,please provide a valid COMMAND_NUMBER. The command with %d as command_no does not exist.\n", *hist_no);
                continue;
            }
            else
            {
                if (records[*hist_no - 1].is_back == 1)
                {
                    *(bg_flag) = 1;
                }
                if (!strcmp(records[*hist_no - 1].command[0], "cd\0"))
                {
                    cd_func(records[*hist_no - 1].filled, current_dir, login_name, records[*hist_no - 1].command);
                }
                else if (!strcmp(records[*hist_no - 1].command[0], "pid\0"))
                {
                    pid_func(records[*hist_no - 1].filled, records[*hist_no - 1].command);
                }
                else
                {
                    exec_decider(records[*hist_no - 1].filled, records[*hist_no - 1].command);
                }
            }
        }
        else if (!strcmp("STOP\0", command[0]))
        {
            if (*(bg_flag) == 1)
            {
                printf("[!][!][!] Cannot run `STOP` command in background, running it in foreground.\n");
            }
            struct Node *kill_head = head;
            while (head != NULL)
            {
                kill(kill_head->pid, SIGKILL);
                head = kill_head->next;
                free(kill_head);
                kill_head = head;
            }
            break;
        }
        else if (!(strcmp("HISTORY\0", command[0])))
        {
            if (*(bg_flag) == 1)
            {
                printf("[!][!][!] Cannot run command in background, running it in foreground.\n");
            }
            if (!(strcmp("BRIEF\0", command[1])))
            {

                if (*(commands_executed) == 0)
                {
                    printf("[!][!][!] No HISTORY to show till now.\n");
                    continue;
                }

                printf("Below are the commands executed in the current session.\n");
                for (int i = 0; i < *(commands_executed); i++)
                {
                    printf(" %d. %s\n", i + 1, ((records + i)->command[0]));
                }
            }
            else if (!strcmp("FULL\0", command[1]))
            {
                if (*(commands_executed) == 0)
                {
                    printf("[!][!][!] No HISTORY to show till now.\n");
                    continue;
                }
                printf("Below are the full-commands executed in the current session.\n");

                for (int i = 0; i < *(commands_executed); i++)
                {
                    printf(" %d.", i + 1);
                    for (int j = 0; j < records[i].filled; j++)
                    {
                        printf(" %s", (records + i)->command[j]);
                    }
                    if ((records + i)->is_back == 1)
                    {
                        printf(" &");
                    }
                    printf("\n");
                }
            }
            else
            {
                printf("[!][!][!] Please choose a correct command.\nThe correct commands with HISTORY are :\n 1) HISTORY BRIEF -- For short description of command history.\n 2) HISTORY FULL  -- For full description of command history.\n");
                // printf("command executed -- %d", *(commands_executed));
            }
        }
        else if (!(strcmp("EXEC\0", command[0])))
        {
            int command_no = atoi(command[1]);

            if (command_no)
            {

                if (strlen(command[1]) == int_len(command_no))
                {
                    // command_no = 1;
                }
                else
                {
                    command_no = 0;
                }
            }

            if (command_no) //  if integer
            {
                if (command_no <= *(commands_executed) && command_no > 0)
                {
                    if (records[command_no - 1].is_back == 1)
                    {
                        *(bg_flag) = 1;
                    }
                    if (!strcmp(records[command_no - 1].command[0], "cd\0"))
                    {
                        cd_func(records[command_no - 1].filled, current_dir, login_name, records[command_no - 1].command);
                    }
                    else if (!strcmp(records[command_no - 1].command[0], "pid\0"))
                    {
                        pid_func(records[command_no - 1].filled, records[command_no - 1].command);
                    }
                    else
                    {
                        exec_decider(records[command_no - 1].filled, records[command_no - 1].command);
                    }
                }
                else
                {
                    printf("[!][!][!] Please type a correct COMMAND_INDEX_NUMBER. The command with %d as index_no does not exist.\n", command_no);
                }
            }
            else // if string
            {
                // counter -1 because we do not need EXEC in execvp
                if (!strcmp(command[1], "cd\0"))
                {
                    cd_func(counter - 1, current_dir, login_name, command + 1);
                }
                else if (!strcmp(command[1], "pid\0"))
                {
                    pid_func(counter - 1, command + 1);
                }
                else
                {
                    exec_decider(counter - 1, command + 1);
                }
            }
        }
        else if (!strcmp("cd\0", command[0]))
        {
            cd_func(counter, current_dir, login_name, command);
        }
        else if (!strcmp("pid\0", command[0]))
        {
            pid_func(counter, command);
        }
        else
        {
            exec_decider(counter, command);
        }
    }

    printf("\"Exiting normally, bye.\"\n");

    // printf("%d", *(commands_executed));

    return 0;
}