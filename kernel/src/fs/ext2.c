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

#include "ext2.h"
#include "fs/vfs.h"
#include "kmalloc.h"
#include "dev.h"
#include "util.h"
#include "panic.h"
#include <errno.h>

#define EXT2_MAGIC          0xef53
#define EXT2_NDIR_BLOCKS    12  /* Number of direct blocks */
#define EXT2_BLK_IND        12  /* Indirect blocks index */
#define EXT2_BLK_DBL        13  /* Double indirect blocks index */
#define EXT2_BLK_TPL        14  /* Triple indirect blocks index */

struct ext2_sb
{
    struct sb   base;
    uint32_t    block_size;
    uint32_t    inodes_per_group;
    uint32_t    log_block_size;
    struct ext2_group_desc *gd_table;
};

struct ext2_inode
{
    struct inode base;
    uint32_t blocks[15]; /* pointers to blocks */
};

struct ext2_disk_sb dsb;
uint32_t gd_block;
uint32_t block_size;

int ext2_sb_inode_read(struct inode *inode);


static struct inode *ext2_inode_create(dev_t dev, ino_t ino)
{
    struct ext2_inode *inode;

    inode = (struct ext2_inode *)inode_lookup(dev, ino);
    if (inode == NULL)
    {
        inode = kmalloc(sizeof(struct ext2_inode), 0);
        if (inode != NULL)
            inode_init(&inode->base, dev, ino);
    }
    return (struct inode *)inode;
}

void ext2_inode_delete(struct inode *inode)
{
    kfree(inode, sizeof(struct ext2_inode));
}


static int offset_to_block(off_t offset, struct ext2_inode *inode,
                           struct ext2_sb *sb)
{
    uint32_t triple_block, double_block, indirect_block, block;
    uint8_t ind, dbl, tpl;
    uint32_t *buf;
    uint32_t shift;

    block = (uint32_t)-1;

    shift = 10 + sb->log_block_size;

    /* Is direct? */
    if (offset < EXT2_NDIR_BLOCKS*sb->block_size)
        return inode->blocks[offset >> shift];

    buf = kmalloc(sb->block_size, 0);
    if (buf == NULL)
        return -1;

    indirect_block = inode->blocks[EXT2_BLK_IND];
    double_block = inode->blocks[EXT2_BLK_DBL];
    triple_block = inode->blocks[EXT2_BLK_TPL];

    offset = (offset >> shift) - EXT2_NDIR_BLOCKS;
    ind = offset;
    dbl = offset >> 8;
    tpl = offset >> 16;

    if (tpl) {
        panic("ext2: required triple block %d", triple_block);
    }

    if (dbl) {
        panic("ext2: required double block %d", double_block);
    }

    if (dev_io(0, sb->base.dev, DEV_READ, indirect_block*sb->block_size,
                buf, sb->block_size, NULL) == sb->block_size)
    {
        block = buf[ind];
    }

    kfree(buf, sb->block_size);

    return block;
}

ssize_t ext2_read(struct ext2_inode *inode, void *buf, size_t count,
        off_t offset)
{
    struct ext2_sb *sb = (struct ext2_sb *)inode->base.sb;
    int left;
    int block;
    off_t ext2_off, block_off, file_off;
    ssize_t n;

    if (inode->base.size < offset)
        return 0; /* EOF */
    if (inode->base.size < offset+count)
        count = inode->base.size - offset;

    file_off = offset;
    left = count;
    while (left > 0)
    {
        block = offset_to_block(file_off, inode, sb);
        if (block < 0)
            break;
        block_off = offset % sb->block_size; /* used just by the first block */
        ext2_off = block * sb->block_size + block_off;
        n = MIN(left, sb->block_size - block_off);
        if (dev_io(0, sb->base.dev, DEV_READ, ext2_off, buf, n, NULL) != n)
            break;
        left -= n;
        file_off += n;
        buf = (char *)buf + n;
        offset = 0;
    }
    return count-left;
}

struct inode *ext2_lookup(struct inode *dir, const char *name)
{
    struct ext2_disk_dirent *dirent;
    struct ext2_disk_dirent *dirbuf;
    int count;
    struct inode *inode = NULL;

    if ((dirbuf = kmalloc(dir->size, 0)) == NULL)
        return NULL;

    if (dev_io(0, dir->sb->dev, DEV_READ, ((struct ext2_inode *)dir)->blocks[0]*1024,
            dirbuf, dir->size, NULL) != dir->size)
    {
        kfree(dirbuf, dir->size);
        return NULL;
    }

    count = dir->size;
    dirent = dirbuf;
    while(count > 0)
    {
        /* name are not null terminated */
        if(dirent->name_len == strlen(name)
            && !strncmp(dirent->name, name, dirent->name_len))
        {
            /* TODO: iget first... */
            inode = ext2_inode_create(dir->dev, dirent->inode);
            inode->sb = dir->sb;
            if (ext2_sb_inode_read(inode) != 0)
                ext2_inode_delete(inode);
            break;
        }
        if((dirent->rec_len) == 0)
            break;
        count -= dirent->rec_len;
        dirent = (struct ext2_disk_dirent *)((char *)dirent + dirent->rec_len);
    }

    kfree(dirbuf, dir->size);
    return inode;
}

static int ext2_readdir(struct inode *dir, unsigned int i,
        struct dirent *dent)
{
    struct ext2_disk_dirent *dirent;
    struct ext2_disk_dirent *dirbuf;
    int count, n;
    int ret = -1;

    if ((dirbuf = kmalloc(dir->size, 0)) == NULL)
        return -ENOMEM;

    if (dev_io(0, dir->sb->dev, DEV_READ,
               ((struct ext2_inode *)dir)->blocks[0]*1024,
                dirbuf, dir->size, NULL) != dir->size)
    {
        kfree(dirbuf, dir->size);
        return -1; /* TODO: return a proper errno */
    }

    count = dir->size;
    dirent = dirbuf;
    n = 0;
    while(count > 0)
    {
        if (n++ == i)
        {
            n = MIN(dirent->name_len, NAME_MAX);
            memcpy(&dent->d_name, dirent->name, n);
            dent->d_name[n] = '\0';
            dent->d_ino = dirent->inode;
            ret = 0;
            break;
        }
        if((dirent->rec_len) == 0)
            break;
        count -= dirent->rec_len;
        dirent = (struct ext2_disk_dirent *)((char *)dirent + dirent->rec_len);
    }

    kfree(dirbuf, dir->size);
    return ret;
}

static const struct inode_ops ext2_inode_ops =
{
    .read = (inode_read_t)ext2_read,
    .lookup = ext2_lookup,
    .readdir = ext2_readdir,
};


int ext2_sb_inode_read(struct inode *inode)
{
    int n;
    struct ext2_disk_inode dnode;
    struct ext2_sb *sb = (struct ext2_sb *) inode->sb;
    struct ext2_group_desc *gd;
    int group;
    int table_index;
    int blockno;
    int ind;

    group = ((inode->ino - 1) / sb->inodes_per_group);
    gd = &sb->gd_table[group];

    table_index = (inode->ino - 1 ) % sb->inodes_per_group;
    blockno = ((table_index * 128) / 1024 ) + gd->inode_table;
    ind = table_index % (1024 /128);

    n = dev_io(0, sb->base.dev, DEV_READ, blockno*1024 + ind*sizeof(dnode),
            &dnode, sizeof(dnode), NULL);
    if (n != sizeof(dnode))
        return -1;

    inode->ops = &ext2_inode_ops;
    inode->mode = dnode.mode;
    //inode->nlink = dnode.links_count;
    inode->uid = dnode.uid;
    inode->gid = dnode.gid;
    if (S_ISCHR(inode->mode) || S_ISBLK(inode->mode))
        inode->rdev  = dnode.block[0];
    inode->size = dnode.size;
    //inode->atime
    //inode->dtime
    //inode->mtime
    //inode->blksize = 512; // sicuro???
    //inode->blocks = (dnode.size-1)/inode->blksize+1;
    memcpy(((struct ext2_inode *)inode)->blocks, dnode.block,
           sizeof(dnode.block));

    return 0;
}

struct sb *ext2_sb_create(dev_t dev)
{
    int n;
    struct ext2_sb *sb;
    int num_groups;

    n = dev_io(0, dev, DEV_READ, 1024, &dsb, sizeof(dsb), NULL);
    if (n != sizeof(dsb))
        return NULL;

    if (dsb.magic != EXT2_MAGIC)
        return NULL;

    sb = kmalloc(sizeof(struct ext2_sb), 0);
    if (sb == NULL)
        return NULL;

    sb->inodes_per_group = dsb.inodes_per_group;
    sb->base.dev = dev;
    sb->log_block_size = dsb.log_block_size;
    sb->block_size = 1024 << dsb.log_block_size;
    gd_block = (dsb.log_block_size == 0) ? 3 : 2;
    num_groups = (dsb.blocks_count - 1) / dsb.blocks_per_group + 1;

    n = sizeof(struct ext2_group_desc) * num_groups;
    sb->gd_table = kmalloc(n, 0);
    if (!sb->gd_table)
        return NULL;

    if (dev_io(0, dev, DEV_READ, sb->block_size*(gd_block-1),
               sb->gd_table, n, NULL) != n)
        return NULL;

    /* Now that we can read inodes, we cache the root inode */
    struct inode *root = ext2_inode_create(dev, EXT2_ROOT_INO);
    root->sb = &sb->base;
    ext2_sb_inode_read(root);

    sb_init(&sb->base, dev, root, NULL);

    return &sb->base;
}
