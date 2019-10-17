#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 80

int main(void)
{
    // command line (of 80) has max of 40 arguments
    char *real_args[MAX_LINE/2 + 1];
    char real_buffer[MAX_LINE];

    /**
     * Save the last line for the history feature.
     */
    char old_buffer[MAX_LINE] = "";

    /**
     * The subshell created by a pipe has access to portions of real_args and
     * real_buffer exposed by the following two pointers, repsectively.
     *
     * Only in subshells will these pointers change.
     */
    char **args = real_args, *buffer = real_buffer;

    /**
     * Indicates whether this process of the shell is forked due to a pipe.
     */
    bool in_pipe = false;

    /**
     * Indicates whether stdin/stdout has been directed.
     */
    bool stdin_redirected, stdout_redirected;

    pid_t pid;

    while (true) {
        stdout_redirected = false;

        // if in a pipe, real_buffer != buffer
        if (!in_pipe) {
            printf("osh>");
            fflush(stdout);

            // If this is a child process forked by pipe, then its stdin has
            // been redirected, so we can't reset the flag.
            stdin_redirected = false;

            // set to \0 to detect whether there are > MAX_LINE - 1 characters
            buffer[MAX_LINE - 2] = '\0';
            if (!fgets(buffer, MAX_LINE, stdin)) {
                // exit if only EOF is read
                if (feof(stdin))
                    break;
                else {
                    fprintf(stderr, "Error (%d) in reading input.\n",
                            ferror(stdin));
                    continue;
                }
            }
            if (!strcmp(buffer, "exit\n"))
                break;
            if (!strcmp(buffer, "!!\n")) {
                if (old_buffer[0] == '\0') {
                    fputs("simple-shell: No command in history.\n", stderr);
                    continue;
                }
                else
                    strncpy(buffer, old_buffer, MAX_LINE);
            }
            else
                strncpy(old_buffer, buffer, MAX_LINE);

            char c = buffer[MAX_LINE - 2];
            if (!(c == '\0' || c == '\n'))
                // There are still unread characters in stdin, because
                // buffer[MAX_LINE - 2] is not overridden with either \0 or \n
                while (getchar() != '\n');
        }

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
        char *p;
        for (p = buffer; *p; ++p) {
            if (*p == '|') {
                if (stdout_redirected) {
                    fputs("simple-shell: can't use pipe because stdout has "
                        "been redirected with \">\".\n", stderr);
                    if (in_pipe)
                        return 1;
                    else {
                        error_in_parsing = true;
                        break;
                    }
                }
                stdout_redirected = true;
                // terminate the current argument, because there may not be a
                // space between the argument and the pipe character
                *p = '\0';
                break;
            }
            else if (*p == '<') {
                if (stdin_redirected) {
                    fputs("simple-shell: attempt to redirect stdin a second time "
                        "with \"<\".\n", stderr);
                    error_in_parsing = true;
                    break;
                }
                else {
                    *p = '\0';
                    prev_char_is_space = true;
                    stdin_redirected = true;
                    search_redirection_filename = true;
                    parsing_stdin_redirection_filename = true;
                }
            }
            else if (*p == '>') {
                if (stdout_redirected) {
                    fputs("simple-shell: attempt to redirect stdout a second time "
                        "with \">\".\n", stderr);
                    error_in_parsing = true;
                }
                else {
                    *p = '\0';
                    prev_char_is_space = true;
                    stdout_redirected = true;
                    search_redirection_filename = true;
                    parsing_stdin_redirection_filename = false;
                }
            }
            else if (*p == ' ') {
                prev_char_is_space = true;
                *p = '\0';
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

        if (search_redirection_filename) {
            fputs("simple-shell: the last redirection operator is not "
                "supplied with a file name.\n", stderr);
            continue;
        }

        bool foreground = true;
        // remove the last "&" from the argument list.
        if (n_args > 1 && !strcmp(args[n_args - 1], "&")) {
            foreground = false;
            --n_args;
        }
        args[n_args] = NULL;

        if (n_args > 0) {
            if (in_pipe) {
                // no further pipe down the road, run the command in the
                // current process
                if (output_filename || !stdout_redirected) {
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
            }

            if ((pid = fork()) == 0) {
                if (!output_filename && stdout_redirected) {
                    // need to pipe the output
                    int pipefd[2];
                    if (pipe(pipefd) == -1) {
                        perror("simple-shell: fail to create the pipe");
                        return 0;
                    }

                    pid_t subshell_pid;
                    if ((subshell_pid = fork()) == 0) {
                        close(pipefd[1]);
                        dup2(pipefd[0], STDIN_FILENO);

                        args += n_args + 1;
                        buffer = p + 1;

                        in_pipe = true;
                        stdin_redirected = true;
                        continue;
                    }
                    else if (subshell_pid > 0) {
                        close(pipefd[0]);
                        dup2(pipefd[1], STDOUT_FILENO);
                    }
                    else {
                        perror("simple-shell: fail to create a subshell for the pipe");
                        return 0;
                    }
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
