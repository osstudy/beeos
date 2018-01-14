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

#include "sys.h"
#include "fs/vfs.h"
#include "kprintf.h"
#include <errno.h>

int sys_mknod(const char *pathname, mode_t mode, dev_t dev)
{
    int res = 0;
    struct inode *inode;
    static ino_t next_ino = 0;

    inode = fs_namei(pathname);
    if (inode == NULL)
    {
        /*
         * This should be a superblock virtual fs function.
         * For now we assume that mknod is only for devices.
         */
        inode = inode_create(dev, next_ino++);

        /*
         * TODO: DEVFS is required
         * At this point this new inode must be linked to the directory.
         */
    }
    else
    {
        iput(inode); /* Release */
        res = -EEXIST; /* file exists */
    }
    return res;
}
