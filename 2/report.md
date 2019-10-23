# EI338 Computer System Engineering: Project Report 2

517030910384 徐尚宁

## Introduction

In this project report, we present an implementation of a simple shell that supports executing last command with `!!`, redirection and multi-stage pipe. A Linux kernel module is also implemented for the second project and can display process information when given a PID.

## Environment

- Arch Linux
- Kernel Version: 5.3.1
- GCC 9.2.0

## Project 1: UNIX Shell

### Parsing

For convenience, a line input up to 79 characters of is accepted. The entire line is read into a buffer (extra characters discarded) and then parsed character by character. Each character is parsed according to the following cases:
- `|`
- `<` or `>`
- A space
- Other characters

`|`, `<`, `>` and spaces all serve to divide the line into arguments and will be replaced with `\0` on encounter to convert the line into multiple null-terminated strings e.g. `echo abcd > abcd.txt` becomes `echo\0abcd\0\0\0abcd.txt`. The pointer to each argument is collected and save in `args` array.

Errors like more than one `<` in the command line will be raised during parsing. On errors, parsing stops and the command line prompts for new input.

### Builtin

The simple shell only supports two built-in commands: `exit` and `!!`.

The history feature is implemented simply by copying and saving the last input line in another character array `old_buffer`. When the user enters `!!`, contents in `old_buffer` will be copied to the input buffer and reparsed.

### Background Jobs

After all arguments are parsed, the code will check whether the last argument is a single `&`. If not, `waitpid` will be used to wait for the command executed in the child process. Otherwise, the shell will just prompt for new input.

### Redirection

When a redirection operator (`<` or `>`) is first encountered, a set of flags will be set to indicate that (using the example of `<`)
- Standard input has been redirected;
- The next argument will be the parsed as a file name, rather than as an argument;

In fact, the two flags that indicate whether stdin and/or stdout is redirected will also be set during the parsing of the pipe, so that we can detect errors that arise when redirection and pipe are used together.

During the parsing of the file name, a pointer to the file name will be stored and will not be added to the argument list.

### Pipe

When a pipe is encountered, parsing will stop at the pipe and our shell will fork a new child. In the parent process, we execute the command line parsed, while in the child process, we restart parsing right after the pipe character. This design allows us to support multi-stage pipe easily, due to the recursive structure of the pipeline, but also leave some problems to solve.

We need to differentiate 3 cases of executing a command:
- If we encounter the first pipe during parsing, our shell will stop parsing immediately, fork a child process A and wait for A. A will fork *again* to B and then execute the command prior to the pipe character. Process B will continue parsing.
- If we encounter a pipe character during parsing, but not the first pipe character, the current process A will stop parsing and fork a child B. A will execute the command *in the current process* prior to the pipe character. B will continue parsing.
- If we reach the end of the line, and we have parsed a pipe before, the final command will be executed in the current process.

Consider the first two cases. Where does B store its parsed command-line arguments? We have two variables for the line input and the argument array: `buffer` and `real_buffer`, `args` and `real_args`. The line input is stored in `real_buffer`, and all pointers to arguments are stored in `real_args`. `buffer` and `args` are only pointers to elements in `real_buffer` and `real_args`.

Just after A forks B, the following code will b executed:

```c
args += n_args + 1;
buffer = p + 1;
```

where `n_args` is the number of A's command-line arguments, and `p` is the pointer to the pipe character in `buffer` that causes A's fork. By moving the pointers `buffer` and `args`, when B start parsing later, it will only see the unparsed line in `real_buffer` and utilize the `real_args` array by placing newly-parsed arguments just after A's arguments.

The appendix gives an example of running our shell at the end of the report.

### Future Improvements

- In our recursive parsing for pipe, the main shell process only wait for the first command in the input. Often, that command will exit quickly so that the main process will be able to print a prompt before other children processes produce output. In a real shell, all processes are forked by the main process and the shell acts as a scheduler for these processes, adjusting their execution according to data flow in the pipes.
- `less` sometimes fails to read keyboard input when running in a pipe. [A Stackoverflow question](https://stackoverflow.com/questions/46930395/pipeline-management-in-linux-shell) suggests that `less` explicitly opens `/dev/tty` to read console input, so we should investigate whether it fail to read the input in our shell.

## Project 2: Linux Kernel Module for Task Information

### Reading from `/proc/pid`

Contrary to intuition, how our LKM (Linux kernel module) reads from `/proc/pid` is implemented in the function `proc_write`, because it controls what our LKM's action if the `/proc/pid` is written. The code for `proc_write` is duplicated below.

```c
static ssize_t proc_write(struct file *file, const char __user *usr_buf, size_t count, loff_t *pos)
{
    char *k_mem;
    long long_pid;

    // allocate kernel memory
    k_mem = kmalloc(count + 1, GFP_KERNEL);

    /* copies user space usr_buf to kernel buffer */
    if (copy_from_user(k_mem, usr_buf, count)) {
        printk(KERN_INFO "Error copying from user\n");
        return -1;
    }
    k_mem[count] = '\0';

    kstrtol(k_mem, 10, &long_pid);
    current_pid = (int) long_pid;

    kfree(k_mem);

    return count;
}
```

Since the memory contents in user space may not be a valid null-terminated string, we should manually add a null character. The string is then converted into an integer and stored in the global variable `current_pid`.

### Writing to `/proc/pid`

The file `/proc/pid` is written when it is read. The following code handles printing the task information

```c
if (tsk = pid_task(find_vpid(current_pid), PIDTYPE_PID))
    rv = snprintf(buffer, BUFFER_SIZE,
             "command = [%s], pid = [%d], state = [%ld]\n",
             tsk->comm, current_pid, tsk->state);
else {
    printk(KERN_INFO "Invalid PID %d written to /proc/pid\n", current_pid);
    return 0;
}
completed = 1;
```

We use a combination of `find_vpid` and `pid_task` to find the `task_struct` of the pid. Information of about the task is formatted in `buffer` and copied to the user memory. Otherwise, an error message is printed to the kernel ring buffer.

## Appendix

Our shell in action:

```
osh>echo abcd
abcd
osh>ls
simple-shell  simple-shell.c
osh>!!                              # history
simple-shell  simple-shell.c
osh>ls > a.txt                      # redirection
osh>cat < a.txt
a.txt
simple-shell
simple-shell.c
osh>wc -l < a.txt > count.txt       # redirect input and output
osh>cat count.txt
3
osh>cat < a.txt | wc -l             # redirection and pipe
3
osh>echo abcd | tr -d a > tr.txt
osh>cat tr.txt
bcd                                 # multi-stage pipe and redirection
osh>cat < tr.txt | tr -d b | tr -d d > new_tr.txt
osh>cat new_tr.txt
c
osh>exit
```
