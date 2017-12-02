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
#include "kmalloc.h"
#include "timer.h"
#include <unistd.h>
#include <errno.h>


extern unsigned long timer_ticks;

static void sleep_timer_handler(void *data)
{
    struct task *task = (struct task *)data;
    task->state = TASK_RUNNING;
}

int sys_nanosleep(const struct timespec *req, struct timespec *rem)
{
    long ms;
    struct timer_event *tm;
    unsigned long when;
    unsigned long now;
    
    if (req->tv_sec < 0 || req->tv_nsec < 0 || req->tv_nsec > 999999999)
        return -EINVAL;

    tm = kmalloc(sizeof(struct timer_event), 0);
    if (!tm)
        return -ENOMEM;

    current_task->state = TASK_SLEEPING;
    
    ms = req->tv_sec * 1000 + req->tv_nsec / 1000000;
    when = timer_ticks + msecs_to_ticks(ms);
    timer_event_init(tm, sleep_timer_handler, current_task, when);

    /* Do this after the timer initialization but before queue insertion */
    list_insert_before(&current_task->timers, &tm->plink);
    
    timer_event_add(tm);

    scheduler();

    list_delete(&tm->link); /* in case of an early wakeup we are still linked */
    list_delete(&tm->plink);
    kfree(tm, sizeof(struct timer_event));

    now = timer_ticks;
    if (now < when)
    {
        ms = ticks_to_msecs(when - now);
        rem->tv_sec = ms / 1000;
        rem->tv_nsec = (ms % 1000) * 1000000;
        return -EINTR; /* Early wakeup */
    }

    rem->tv_sec = 0;
    rem->tv_nsec = 0;
    return 0;
}
