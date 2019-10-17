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

        /**
         * The previous character is space, "<", or ">". Indicates that the
         * next character is the start of an argument if it is not a space.
         */
        bool prev_char_is_space = true;

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
        
        /**
         * Indicates an error in parsing the command line. The shell will stop
         * parsing and prompt for new input.
         */
        bool error_in_parsing = false;

        int n_args = 0;
        for (char *p = buffer; *p; ++p) {
            if (*p == '<') {
                // If the stdin has been redirected, then either we just parsed
                // a "<" and is looking for a filename, or we have found and
                // set the variable input_filename.
                if (search_redirection_filename || input_filename) {
                    fputs("Error: attempt to redirect stdin a second time "
                        "with \"<\".\n", stderr);
                    error_in_parsing = true;
                    break;
                }
                else {
                    *p = '\0';
                    prev_char_is_space = true;
                    search_redirection_filename = true;
                    parsing_stdin_redirection_filename = true;
                }
            }
            else if (*p == '>') {
                // same as the case for "<"
                if (search_redirection_filename || output_filename) {
                    fputs("Error: attempt to redirect stdout a second time "
                        "with \">\".\n", stderr);
                    error_in_parsing = true;
                }
                else {
                    *p = '\0';
                    prev_char_is_space = true;
                    search_redirection_filename = true;
                    parsing_stdin_redirection_filename = false;
                }
            }
            else if (*p == ' ') {
                if (!prev_char_is_space) {
                    // terminate the previous argument with \0
                    prev_char_is_space = true;
                    *p = '\0';
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
        if (error_in_parsing)
            continue;

        bool foreground = true;
        // remove the last "&" from the argument list.
        if (n_args > 1 && !strcmp(args[n_args - 1], "&")) {
            foreground = false;
            --n_args;
        }
        args[n_args] = NULL;

        if (n_args > 0) {
            if ((pid = fork()) == 0) {
                if (search_redirection_filename) {
                    fputs("Error: the last redirection operator is not "
                        "supplied with a file name.\n", stderr);
                    return 0;
                }
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
