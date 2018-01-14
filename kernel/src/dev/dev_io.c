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

#include "dev.h"
#include <errno.h>

ssize_t dev_io(pid_t pid, dev_t dev, int rw, off_t off,
        void *buf, size_t size, int *eof)
{
    ssize_t res = 0;

    if (rw != DEV_READ && rw != DEV_WRITE)
        return -EIO;

    switch (major(dev))
    {
        case major(DEV_TTY):
        case major(DEV_CONSOLE):
            res = dev_io_tty(pid, dev, rw, off, buf, size, eof);
            break;
        case major(DEV_INITRD):
            res = dev_io_ramdisk(pid, dev, rw, off, buf, size, eof);
            break;
        default:
            res = -ENODEV;
            break;
    }
    return res;
}
