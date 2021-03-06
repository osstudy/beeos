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
#include "proc.h"
#include <errno.h>

int sys_fstat(int fdn, struct stat *buf)
{
    struct inode *inode;

    if (current_task->fd[fdn].file == NULL)
        return -EBADF;  /* Bad file descriptor */

    inode = current_task->fd[fdn].file->inode;
    if (inode == NULL)
        return -ENOENT;

    buf->st_dev = inode->dev;
    buf->st_ino = inode->ino;
    buf->st_mode = inode->mode;
    buf->st_nlink = 0;     // TODO
    buf->st_uid = inode->uid;
    buf->st_gid = inode->gid;
    if (S_ISBLK(inode->mode) || S_ISCHR(inode->mode))
        buf->st_rdev = 0;   // TODO
    else
        buf->st_rdev = 0;
    buf->st_size = inode->size;
    buf->st_atime = 0;  // TODO
    buf->st_mtime = 0;
    buf->st_ctime = 0;
    buf->st_blksize = 0;
    buf->st_blocks = 0;

    return 0;
}
