#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 80

int main(void)
{
    // command line (of 80) has max of 40 arguments
    char *args[MAX_LINE/2 + 1];
    char buffer[MAX_LINE];
    pid_t pid;

    while (true) {
        printf("osh>");
        fflush(stdout);

        // set to \0 to detect whether there are > MAX_LINE - 1 characters
        buffer[MAX_LINE - 2] = '\0';
        if (!fgets(buffer, MAX_LINE, stdin)) {
            // exit if only EOF is read
            if (feof(stdin))
                break;
            else {
                fprintf(stderr, "Error (%d) in reading input.\n", ferror(stdin));
                continue;
            }
        }
        if (!strcmp(buffer, "exit\n"))
            break;

        char c = buffer[MAX_LINE - 2];
        if (!(c == '\0' || c == '\n'))
            // There are still unread characters in stdin, because
            // buffer[MAX_LINE - 2] is not overridden with either \0 or \n
            while (getchar() != '\n');

        bool prev_char_is_space = true;

        /**
         * Indicates that a "<" or ">" has been parsed, so that subsequent
         * "<" and ">" will be treated as arguments.
         */
        bool stdin_redirection_not_parsed = true,
            stdout_redirection_not_parsed = true;

        /**
         * Indicates that a redirection operator is found and that we are
         * looking for the next argument, which will be the name of the file
         * to which I/O will be redirected to.
         */
        bool search_redirection_filename = false;

        // set to NULL so that we can detect whether every redirection operator
        // has an associated filename
        const char *input_filename = NULL, *output_filename = NULL;

        /**
         * true for parsing the filename for "<", false for ">"
         */
        bool parsing_stdin_redirection_filename;

        int n_args = 0;
        for (char *p = buffer; *p; ++p) {
            if (*p == ' ') {
                if (!prev_char_is_space) {
                    // terminate the previous argument with \0
                    prev_char_is_space = true;
                    *p = '\0';

                    if (stdin_redirection_not_parsed
                            && !strcmp(args[n_args - 1], "<")) {
                        stdin_redirection_not_parsed = false;
                        search_redirection_filename = true;
                        --n_args;   // remove "<" from the argument list
                        parsing_stdin_redirection_filename = true;
                    }
                    if (stdout_redirection_not_parsed
                            && !strcmp(args[n_args - 1], ">")) {
                        stdout_redirection_not_parsed = false;
                        search_redirection_filename = true;
                        --n_args;   // same as above
                        parsing_stdin_redirection_filename = false;
                    }
                }
            }
            else if (*p == '\n')
                *p = '\0';  // ...so it won't show up in command line arguments
            else if (prev_char_is_space) {
                prev_char_is_space = false;

                if (search_redirection_filename) {
                    search_redirection_filename = false;
                    if (parsing_stdin_redirection_filename)
                        input_filename = p;
                    else
                        output_filename = p;
                }
                else
                    args[n_args++] = p;
            }
        }            

        bool foreground = true;
        // remove the last "&" from the argument list.
        if (n_args > 1 && !strcmp(args[n_args - 1], "&")) {
            foreground = false;
            --n_args;
        }
        args[n_args] = NULL;

        if (n_args > 0) {
            if ((pid = fork()) == 0) {
                if (input_filename && !freopen(input_filename, "r", stdin)) {
                    fputs("Error in opening \"", stderr);
                    fputs(input_filename, stderr);
                    perror("\" for redirection");
                    return 0;
                }
                if (output_filename && !freopen(output_filename, "w", stdout)) {
                    fputs("Error in opening \"", stderr);
                    fputs(output_filename, stderr);
                    perror("\" for redirection");
                    return 0;
                }

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