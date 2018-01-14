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

#include "fs/vfs.h"
#include "elf.h"
#include "kmalloc.h"
#include "proc.h"
#include "arch/x86/paging.h"

#include <sys/types.h>
#include <stddef.h>
#include <errno.h>
#include <limits.h>

static char *push(char *sp, const char *str)
{
    int n = strlen(str) + 1;
    sp -= n;
    memcpy(sp, str, n);
    return sp;
}

static char *push_all(uintptr_t *base, char *sp, const char *str[],
        ptrdiff_t delta, uintptr_t *nout)
{
    int n;
    for (n = 0; str[n]; n++);
    if (nout)
        *nout = n;
    base[n] = 0;
    while (n-- > 0)
    {
        sp = push(sp, str[n]);
        base[n] = (uintptr_t)sp + delta;
    }
    return sp;
}

static void stack_init(uintptr_t *base, const char *argv[], const char *envp[])
{
    char *sp = (char *)base + ARG_MAX;
    ptrdiff_t delta = (char *)KVBASE - sp;

    sp = push_all(&base[3], sp, argv, delta, &base[0]);
    base[1] = (uintptr_t)&base[3] + delta;

    if (envp != NULL)
    {
        /* (+1) push above the argv null terminator */
        sp = push_all(&base[4+base[0]], sp, envp, delta, NULL);
        base[2] = (uintptr_t)&base[4+base[0]] + delta;
    }
    else
        base[2] = 0;
}


int sys_execve(const char *path, const char *argv[], const char *envp[])
{
    int ret = 0;
    struct elf_hdr eh;
    struct elf_prog_hdr ph;
    struct inode *inode;
    unsigned int i, off;
    uint32_t pgdir, vaddr;
    void *ustack;

    if (current_task->arch.ifr == NULL || argv == NULL)
        return -EINVAL;

    inode = fs_namei(path);
    if (!inode)
        return -ENOENT;

    if (fs_read(inode, &eh, sizeof(eh), 0) != sizeof(eh)
        || eh.magic != ELF_MAGIC)
        return -ENOEXEC;

    /* Immediatelly copy argv and envp arrays in a temporary user stack
     * allocated via kmalloc (shared betweek virtual spaces). */
    ustack = kmalloc(ARG_MAX, 0);
    if (ustack == NULL)
        return -ENOMEM;
    stack_init(ustack, argv, envp);

    pgdir = page_dir_dup(0);
    page_dir_switch(pgdir);

    /* The function has been called via a syscall */
    /* TODO: Create user stack only if we where in user space
     * Otherwise esp is not in the frame */
    /* Minimal user stack */
    if ((ret = (int)page_map((char *)KVBASE-PAGE_SIZE, -1)) < 0) {
        kfree(ustack, ARG_MAX);
        goto bad;
    }
    memcpy((char *)KVBASE-ARG_MAX, ustack, ARG_MAX);

    /* Release user stack copy */
    kfree(ustack, ARG_MAX);

    /* Start with an unknown program break */
    current_task->brk = 0;

    for (i = 0, off = eh.phoff; i < eh.phnum; i++, off += sizeof(ph)) {

        if (fs_read(inode, &ph, sizeof(ph), off) != sizeof(ph)) {
            ret = -ENOEXEC;
            goto bad;
        }

        if (ph.type != ELF_PROG_TYPE_LOAD)
            continue;

        if (ph.memsz < ph.filesz ||
            KVBASE <= ph.vaddr + ph.memsz) {
            ret = -ENOEXEC;
            goto bad;
        }

        /* Look for program brk (temporary... not very elegant) */
        if ((ph.flags & ELF_PROG_FLAG_READ) &&
            (ph.flags & ELF_PROG_FLAG_WRITE) &&
            current_task->brk < ph.vaddr+ph.memsz) {
            current_task->brk = ph.vaddr+ph.memsz;
        }

        for (vaddr = ALIGN_DOWN(ph.vaddr, PAGE_SIZE);
             vaddr < ph.vaddr + ph.memsz;
             vaddr += PAGE_SIZE) {
            if ((ret = (int)page_map((char *)vaddr, -1)) < 0)
                goto bad;
        }

        if (ph.filesz != 0) {
            if ((ret = fs_read(inode, (void *)ph.vaddr, ph.filesz, ph.offset))
                    != ph.filesz)
                goto bad;
        }
        if (ph.memsz - ph.filesz > 0)
            memset((void *)(ph.vaddr + ph.filesz), 0, ph.memsz - ph.filesz);
    }

    /*** FIXME ARCH specific code ***/

    /* Release the old dir just before jump */
    page_dir_del(current_task->arch.pgdir);
    current_task->arch.pgdir = pgdir;

    /* We assume that ARG_MAX is lass than PAGE_SIZE */
    current_task->arch.ifr->usr_esp = KVBASE-ARG_MAX;
    current_task->arch.ifr->eip = eh.entry;

    /*
     * POSIX1. All signals are set to their default action unless the process
     * that calls exec is ignoring the signal.
     */
    for (i = 0; i < SIGNALS_NUM; i++)
    {
        if (current_task->signals[i].sa_handler != SIG_IGN)
        {
            memset(&current_task->signals[i], 0, sizeof(struct sigaction));
            current_task->signals[i].sa_handler = SIG_DFL;
        }
    }

    return ret;

bad:
    /* Switch back to the old dir */
    page_dir_switch(current_task->arch.pgdir);
    /* Release the new dir, this also release all the mapped pages. */
    page_dir_del(pgdir);
    return ret;
}
