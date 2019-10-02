# EI338 Computer System Engineering: Project Report 1

517030910384 徐尚宁

## Introduction

In this project report, we present the exercises that we have completed and two Linux kernel modules that create entries in the `/proc` file system.

## Environment

- Arch Linux
- Kernel Version: 5.3.1

## Printing Messages in the Kernel Buffer

Following instructions outlined in the textbook, we add the following lines

```c
printk(KERN_INFO "%llu\n", GOLDEN_RATIO_PRIME);
printk(KERN_INFO "%lu %d\n", jiffies, HZ);
```

in the function `simple_init` and

```c
printk(KERN_INFO "%lu\n", gcd(3300, 24));
printk(KERN_INFO "%lu\n", jiffies);
```

in `simple_exit` to print the golden ration prime, `jiffies` and `HZ` in the kernel buffer.

## The `/proc` File System

Based on code provided in the text, we design two modules that create two entries, `/proc/jiffies` and `/proc/seconds`, respectively. `/proc/jiffies` contains the value of the variable `jiffies` i.e. the number of timer interrupts since system boot. `/proc/seconds` is an estimate of the uptime of the system based on `jiffies` and the period of the timer interrupt.

The `proc_jiffies` module provides `/proc/jiffies`. We mainly modify the `proc_read` function to copy the value of `jiffies` to the user's buffer, as shown below.

```c
ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos)
{
        int rv = 0;
        char buffer[BUFFER_SIZE];
        static int completed = 0;

        if (completed) {
                completed = 0;
                return 0;
        }

        completed = 1;

        rv = sprintf(buffer, "%lu\n", jiffies);

        // copies the contents of buffer to userspace usr_buf
        copy_to_user(usr_buf, buffer, rv);

        return rv;
}
```

To compile our modules, we should add our files to the list of requirements in `Makefile`.

```makefile
obj-m += simple.o hello.o proc_jiffies.o proc_seconds.o
```

The code for the `proc_seconds` module is similar, except for the following line

```c
rv = sprintf(buffer, "%lu\n", jiffies / HZ);
```

Since in the kernel the complexity of floating-point arithmetic should be avoided, we use integer division here.
