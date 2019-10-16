/**
 * Simple shell interface program.
 *
 * Operating System Concepts - Tenth Edition
 * Copyright John Wiley & Sons - 2018
 * Modified by Shining Xu
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 80

int main(void)
{
    char *args[MAX_LINE/2 + 1];     // command line (of 80) has max of 40 arguments
    char buffer[MAX_LINE];
    bool should_run = true;
    pid_t pid;

    while (should_run) {
        printf("osh>");
        fflush(stdout);

        fgets(buffer, MAX_LINE, stdin);
        if (!strcmp(buffer, "exit\n"))
            break;

        bool prev_char_is_space = true;

        /**
         * Indicates that the line has < 80 characters, including the newline
         * character.
         */
        bool complete_line = false;

        int n_args = 0;
        for (char *p = buffer; *p; ++p) {
            if (*p == ' ') {
                if (!prev_char_is_space) {
                    // terminate the previous argument with \0
                    prev_char_is_space = true;
                    *p = '\0';
                }
            }
            else if (*p == '\n') {
                complete_line = true;
                *p = '\0';  // remove the newline character
            }
            else if (prev_char_is_space) {
                prev_char_is_space = false;
                args[n_args++] = p;
            }
        }
        args[n_args] = NULL;

        if (!complete_line)
            while (getchar() != '\n');

        if (n_args > 0) {
            if ((pid = fork()) == 0) {
                execvp(args[0], args);
            }
            else if (pid > 0)
                waitpid(pid, NULL, 0);
        }
    }

    return 0;
}
