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

        // exit if only EOF is read
        if (!fgets(buffer, MAX_LINE, stdin)) {
            if (feof(stdin))
                break;
            else {
                printf("Error (%d) in reading input.\n", ferror(stdin));
                continue;
            }
        }
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

        if (!complete_line)
            while (getchar() != '\n');

        bool foreground = true;
        // remove the last "&" from the argument list.
        if (n_args > 1 && !strcmp(args[n_args - 1], "&")) {
            foreground = false;
            --n_args;
        }
        args[n_args] = NULL;

        if (n_args > 0) {
            if ((pid = fork()) == 0) {
                if (execvp(args[0], args) == -1) {
                    fputs("Error in running ", stderr);
                    perror(args[0]);
                    return 0;
                }
            }
            else if (pid > 0) {
                if (foreground)
                    waitpid(pid, NULL, 0);
            }
            else
                perror("Error in creating new process");
        }
    }

    return 0;
}
