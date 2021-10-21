// chen larry

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_STRING 100
#define MAX_COMMANDS_NUMBER 100
#define MAX_ARGUMENTS 100
#define MAX_PATH 100

typedef struct {
    pid_t pid;
    int args_number;
    char *args[MAX_ARGUMENTS];
} Command;

int main() {
    // declarations
    char line[MAX_STRING] = "";
    Command commands[MAX_COMMANDS_NUMBER];
    int current_command = 0;
    int flag;
    char *token;
    int i, j;

    // shell
    printf("$ ");
    fflush(stdout);
    fgets(line, sizeof(line), stdin);
    line[strlen(line) - 1] = 0;

    while (strcmp(line, "exit")) {
        flag = 0;

        // if no command
        if (strcmp(line, "") == 0) {
            printf("An error occurred\n");
            fflush(stdout);
            // continue
            printf("$ ");
            fflush(stdout);
            fgets(line, sizeof(line), stdin);
            line[strlen(line) - 1] = 0;
            continue;
        }

        /** current command   **/

        // command arguments
        token = strtok(line, " ");
        i = 0;
        while (token != NULL) {
            commands[current_command].args[i] = (char *) malloc(sizeof(char) * sizeof(token));
            // if malloc didnt go well
            if (commands[current_command].args[i] == NULL) {
                printf("An error occurred\n");
                fflush(stdout);
                // continue
                printf("$ ");
                fflush(stdout);
                fgets(line, sizeof(line), stdin);
                line[strlen(line) - 1] = 0;
                current_command++;
                continue;
            }
            strcpy(commands[current_command].args[i], token);
            // continue
            i++;
            token = strtok(NULL, " ");
        }
        // command number of arguments
        commands[current_command].args_number = i;

        /** jobs **/

        if (strcmp(commands[current_command].args[0], "jobs") == 0) {
            // entered function
            flag = 1;

            for (i = 0; i < current_command; i++) {
                // check which process is still alive
                if (commands[i].pid != 0 && kill(commands[i].pid, 0) == 0) {
                    // print arguments
                    for (j = 0; j < commands[i].args_number; j++) {
                        printf("%s ", commands[i].args[j]);
                        fflush(stdout);
                    }
                    printf("\n");
                    fflush(stdout);
                }
            }

        }

        /** history **/

        if (strcmp(commands[current_command].args[0], "history") == 0) {
            // entered function
            flag = 1;

            // update status
            commands[current_command].pid = getpid();

            for (i = 0; i <= current_command; i++) {
                // print arguments
                for (j = 0; j < commands[i].args_number; j++) {
                    printf("%s ", commands[i].args[j]);
                    fflush(stdout);
                }
                // print RUNNING or DONE
                if (commands[i].pid != 0 && kill(commands[i].pid, 0) == 0) {
                    printf("RUNNING");
                    fflush(stdout);
                } else {
                    printf("DONE");
                    fflush(stdout);
                }
                printf("\n");
                fflush(stdout);
            }

            // update status
            commands[current_command].pid = 0;
        }

        /** cd **/

        char temp[MAX_PATH];
        char previous_path[MAX_PATH];
        if (strcmp(commands[current_command].args[0], "cd") == 0) {
            // entered function
            flag = 1;

            if (commands[current_command].args_number == 1) {
                getcwd(temp, sizeof(temp));
                chdir(getenv("HOME"));
                // save previous
                strcpy(previous_path, temp);
            } else if (commands[current_command].args_number == 2) {
                char *partial = strtok(commands[current_command].args[1], "/");
                int fail_flag = 0;
                char next[MAX_PATH];
                getcwd(temp, sizeof(temp));
                while (partial != NULL) {
                    if (strcmp(partial, "~") == 0) {
                        strcpy(next, getenv("HOME"));
                    } else if (strcmp(partial, "-") == 0) {
                        strcpy(next, previous_path);
                    } else if (strcmp(partial, "..") == 0) {
                        strcpy(next, "..");
                    } else {
                        strcpy(next, partial);
                    }
                    if (chdir(next) != 0) {
                        printf("chdir failed\n");
                        fflush(stdout);
                        chdir(temp);
                        fail_flag = 1;
                        break;
                    }
                    // continue
                    partial = strtok(NULL, "/");
                }
                // save previous
                if (fail_flag == 0) {
                    strcpy(previous_path, temp);
                }
            } else {
                printf("Too many arguments\n");
                fflush(stdout);
            }
        }

        /** echo **/
        if (strcmp(commands[current_command].args[0], "echo") == 0) {
            int last = strlen(commands[current_command].args[1]) - 1;
            if (commands[current_command].args[1][0] == '"' && commands[current_command].args[1][last] == '"') {
                commands[current_command].args[1][last] = 0;
                char temp[strlen(commands[current_command].args[1])];
                strcpy(temp, commands[current_command].args[1] + 1);
                strcpy(commands[current_command].args[1], temp);
            }
        }


        /** not built-in commands **/

        if (flag == 0) {
            int ret_code;
            if (strcmp(commands[current_command].args[commands[current_command].args_number - 1], "&") == 0) {
                // delete &
                commands[current_command].args[commands[current_command].args_number - 1] = NULL;
                commands[current_command].args_number--;
                // background
                pid_t pid = fork();
                // if fork failed
                if (pid < 0) {
                    printf("fork failed\n");
                    fflush(stdout);
                }
                // son
                if (pid == 0) {
                    ret_code = execvp(commands[current_command].args[0], commands[current_command].args);
                    if (ret_code == -1) {
                        printf("exec failed\n");
                        fflush(stdout);
                        kill(getpid(), SIGKILL);
                    }
                }
                // father
                if (pid > 0) {
                    commands[current_command].pid = pid;
                    signal(SIGCHLD, SIG_IGN);
                }
            } else {
                // foreground
                pid_t pid = fork();
                // if fork failed
                if (pid < 0) {
                    printf("fork failed\n");
                    fflush(stdout);
                }
                // son
                if (pid == 0) {
                    ret_code = execvp(commands[current_command].args[0], commands[current_command].args);
                    if (ret_code == -1) {
                        printf("exec failed\n");
                        fflush(stdout);
                        kill(getpid(), SIGKILL);
                    }
                }
                // father
                if (pid > 0) {
                    commands[current_command].pid = pid;
                    waitpid(pid, NULL, 0);
                }
            }
        }

        /** next line **/

        printf("$ ");
        fflush(stdout);
        fgets(line, sizeof(line), stdin);
        line[strlen(line) - 1] = 0;
        current_command++;

    }

    /** free **/

    for (i = 0; i < current_command; i++) {
        // free arguments
        for (j = 0; j < commands[i].args_number; j++) {
            free(commands[i].args[j]);
        }
    }

    return 0;
}
