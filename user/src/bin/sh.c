/*
 * Copyright (c) 2015-2017, Davide Galassi. All rights reserved.
 *
 * This file is part of the BeeOS software.
 *
 * BeeOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with BeeOS; if not, see <http://www.gnu/licenses/>.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <limits.h>

#define CMD_MAX 64

static char cmd[CMD_MAX];
static char cwd[PATH_MAX];
static pid_t fgpid = -1;
static int fgterm;

const char *user = "guest";
const char *host = "beeos";

static void print_prompt(void)
{
    char *tty;
    
    tty = getenv("TTY");
    if (tty == NULL)
        tty = "tty?";

    if (getcwd(cwd, PATH_MAX) < 0)
        perror("getcwd");
    printf("%s@%s:%s$ ", tty, host, cwd);
}

static void sigint(int signo)
{
    /* Nothing to do */
}

static void sigchld(int signo)
{
    int status;
    pid_t pid;

    if (signo != SIGCHLD)
        return;

    while (1) {
        pid = waitpid(-1, &status, WNOHANG);
        if (pid > 0) {
            if (pid == fgpid)
                fgterm = 1;
        } else {
            break;
        }
    }
}


static int execute(int argc, char *argv[])
{
    sigset_t zeromask, newmask, oldmask;
    pid_t pid, pgrp;
    int status;
    char *cmd;
    int bg = 0;

    cmd = argv[0];

    /* check built-in commands first */
    if (strcmp(cmd, "exit") == 0) {
        if (getppid() != 1)
            exit(0);
    } else if (strcmp(cmd, "cd") == 0) {
        if ((status = chdir(argv[1])) < 0)
            printf("sh: cd: %s\n", strerror(errno));
    } else {
        /* Block SIGCHLD */
        (void)sigemptyset(&zeromask);
        (void)sigemptyset(&newmask);
        (void)sigaddset(&newmask, SIGCHLD);
        (void)sigprocmask(SIG_BLOCK, &newmask, &oldmask);

        if (argc > 1 && *argv[argc-1] == '&') {
            /* Background job */
            argc--;
            bg = 1;
        }

        fgterm = 0;

        fgpid = pid = fork();
        if (pid >= 0) {
            /* Create process group */
            if (setpgid(pid, pid) >= 0) {
                /* Set process group of controlling terminal */
                if (pid == 0) {
                    if (!bg) {
                        tcsetpgrp(STDOUT_FILENO, getpid());
                    }
                    status = execvpe(cmd, argv, environ);
                    if (status < 0) {
                        printf("sh: %s: %s\n", cmd, strerror(errno));
                        status = 1;
                    }
                    exit(status);
                } else if (!bg) {
                    pgrp = tcgetpgrp(STDOUT_FILENO);
                    tcsetpgrp(STDOUT_FILENO, pid);
                    while (!fgterm)
                        sigsuspend(&zeromask);
                    tcsetpgrp(STDOUT_FILENO, pgrp);
                }
            } else {
                perror("setpgid error");
                printf("command runs in parent group\n");
            }
        } else {
            perror("fork error");
        }
        sigprocmask(SIG_SETMASK, &oldmask, NULL);
    }
    return status;
}

static int interactive(void)
{
    int fd;
    char *argv[20];
    int argc;

    /*
     * POSIX1. On execve() syscall family, all signals are set to their
     * default action unless the process that calls exec is ignoring the
     * signal. Thus here instead of ignoring the SIGINT signal we just
     * set it to a dummy handler.
     */
    if (signal(SIGINT, sigint) < 0)
        perror("signal: SIGINT");

    if (signal(SIGCHLD, sigchld) < 0)
        perror("signal: SIGCHLD");

    fd = open("console", O_RDWR, 0); /* stdin (fd=1) */
    if (fd < 0)
        return -1;
    dup(0); /* stdout (fd=2) */
    dup(0); /* stderr (fd=3) */

    while (1) {
        print_prompt();
        if (fgets(cmd, CMD_MAX, stdin)) {
            argc = 0;
            argv[argc++] = strtok(cmd, " ");
            while ((argv[argc++] = strtok(NULL, " ")) != NULL);
            argc--;
            execute(argc, argv);
        }
    }
    return 0;
}


int main(int argc, char *argv[])
{
    int status;
    sigset_t mask;

    setpgid(0, 0);
    //tcsetpgrp(STDOUT_FILENO, getpid());
    
    /* Be sure that SIGCHLD is unblocked */
    (void)sigemptyset(&mask);
    (void)sigaddset(&mask, SIGCHLD);
    (void)sigprocmask(SIG_UNBLOCK, &mask, NULL);

    if (signal(SIGCHLD, sigchld) < 0)
        perror("signal");

    if (argc > 2 && strcmp(argv[1], "-c") == 0)
        status = execute(argc-2, &argv[2]);
    else
        status = interactive();
    
    return status;
}

