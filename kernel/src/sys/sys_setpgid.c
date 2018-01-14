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

#include <sys/types.h>
#include <errno.h>
#include "proc.h"

/*
 * Sets the process group ID to pgid in the process whose process ID equals
 * pid. If the two arguments are equal, the process specified by pid becomes
 * a process group leader. If pid is 0, the process ID of the caller is used.
 * Also, if pgid is 0, the process ID specified by pid is used as the process
 * group ID.
 * A process can set the process group ID of only itself or any of its
 * children.
 *
 * Furthermore, it can’t change the process group ID of one of its
 * children after that child has called one of the exec functions.
 * NOTE: for simplicity, we don't respect this last requirement here!!!
 */

int sys_setpgid(pid_t pid, pid_t pgid)
{
    struct task *task = NULL;
    int res = 0;

    if (pgid < 0)
        return -EINVAL;

    if (pid == 0)
        pid = current_task->pid;
    if (pgid == 0)
        pgid = pid;

    if (pid != current_task->pid)
    {
        struct task *child, *sib;

        child = list_container(current_task->children.next, struct task,
                               children);
        if (child->pptr == current_task)
        {
            /* We are not a hierarchy leaf */
            sib = child;
            do {
                if (sib->pid == pid)
                {
                    task = sib;
                    break;
                }
                sib = list_container(sib->sibling.next, struct task, sibling);
            } while (sib != child);
        }
    }
    else
    {
        task = current_task;
    }

    if (task != NULL)
        task->pgid = pgid;
    else
        res = -ESRCH;
    return res;
}
