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

#include "proc.h"
#include <sys/types.h>
#include <errno.h>

int sys_setuid(uid_t uid)
{
    int res = 0;

    if (current_task->euid == 0)
    {
        /* If uid is not root then, after this, it will be
         * impossible for the program to regain root privileges. */
        current_task->uid  = uid;
        current_task->euid = uid;
        current_task->suid = uid;
    }
    else if (current_task->uid == uid
          || current_task->suid == uid)
    {
        current_task->euid = uid;
    }
    else
    {
        res = -EPERM;
    }
    return res;
}
