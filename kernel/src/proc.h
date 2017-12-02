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

#ifndef _BEEOS_PROC_H_
#define _BEEOS_PROC_H_

#include "proc/task.h"

/* Default process timeslice (milliseconds) */
#define SCHED_TIMESLICE     100

extern struct task *current_task;

void scheduler(void);

void scheduler_init(void);

void wakeup(void *ctx);

/**
 * Process pending (non masked) signals.
 */
int do_signal(void);

/**
 * First user-mode process trampoline.
 */
void init(void);

/**
 * Idle procedure.
 * This is executed by the first kernel process (pid=0) after that the
 * init process (pid=1) has been started.
 */
void idle(void);

#endif /* _BEEOS_PROC_H_ */
