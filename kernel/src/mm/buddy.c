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

#include "buddy.h"
#include "kprintf.h"
#include "kmalloc.h"
#include "util.h"
#include "panic.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/*
 * We use a bit for each couple of buddies.
 * This function toggles the bit corresponding to the couple
 * and returns the modified bit value.
 */
static int toggle_bit(struct buddy_sys *ctx, int block_idx,
        unsigned int order)
{
    unsigned int i;
    unsigned long *word, bit;

    i = block_idx >> (order + 1);
    word = &ctx->free_area[order].map[i / (8 * sizeof(unsigned long))];
    bit = 1 << i % (8 * sizeof(unsigned long));
    *word ^= bit;           /* Toggle the bit value */
    return *word & bit;     /* Return the current value */
}

/*
 * Deallocate a frame
 */
void buddy_free(struct buddy_sys *ctx, struct frame *frame, unsigned int order)
{
    unsigned int block_idx, buddy_idx, order_curr;

    order_curr = order;
    block_idx = frame - ctx->frames;
    while (order_curr != ctx->order_max)
    {
        /* Check if there is any buddy in the list of the same order */
        buddy_idx = block_idx ^ (1 << order_curr);
        /*
         * Here we could have passed block_idx. Same bit would be toggled.
         * Non zero value is returned if the buddy is still allocated.
         */
        if (toggle_bit(ctx, buddy_idx, order_curr))
            break;

        /* Remove the buddy from its free list */
        list_delete(&ctx->frames[buddy_idx].link);
        /* Coalesce into one bigger block */
        order_curr++;

        /* Always keep track of the left-side index */
        if (buddy_idx < block_idx)
            block_idx = buddy_idx;
    }

    /* Insert the block at the end of the proper list */
    list_insert_before(&ctx->free_area[order_curr].list,
            &ctx->frames[block_idx].link);
}

/*
 * Allocate a frame
 */
struct frame *buddy_alloc(struct buddy_sys *ctx, unsigned int order)
{
    struct frame *frame = NULL;
    int left_idx, right_idx;
    unsigned int i;

    for (i = order; i <= ctx->order_max; i++)
    {
        if (!list_empty(&ctx->free_area[i].list))
        {
            frame = list_container(ctx->free_area[i].list.next,
                    struct frame, link);
            list_delete(&frame->link);
            left_idx = frame - ctx->frames;
            break;
        }
    }
    if (i > ctx->order_max)
        return NULL;

    if (i != ctx->order_max) /* Order max does't have any buddy */
        toggle_bit(ctx, left_idx, i);

    /* Eventually split */
    while (i > order)
    {
        i--;
        right_idx = left_idx + (1 << i);
        list_insert_before(&ctx->free_area[i].list, &ctx->frames[right_idx].link);
        toggle_bit(ctx, right_idx, i);
    }
    return frame;
}

/*
 * Initialize a buddy allocator
 */
int buddy_init(struct buddy_sys *ctx, unsigned int frames_num,
        unsigned int frame_size)
{
    unsigned int i;
    unsigned int count;

    /*
     * Initialize the free frames table
     */

    ctx->order_bit = fnzb(frame_size);
    ctx->order_max = fnzb(frames_num);
    ctx->free_area = kmalloc(sizeof(struct free_list) * (ctx->order_max+1), 0);
    if (!ctx->free_area)
        panic("Buddy init");

    /*
     * Initialize free frames table row for each order.
     */

    for (i = 0; i < ctx->order_max; i++)
    {
        /* Num of buddies of order i. Divide number of blocks by 2^(i+1)  */
        count = (frames_num >> (i+1));
        /* Compute the required number of unsigned longs to hold the bitmap */
        count = (count - 1) / (8 * sizeof(unsigned long)) + 1;
        ctx->free_area[i].map = kmalloc(sizeof(unsigned long) * count, 0);
        if (!ctx->free_area[i].map)
            panic("Buddy init");
        memset(ctx->free_area[i].map, 0, sizeof(unsigned long) * count);
        list_init(&ctx->free_area[i].list);
    }
    /* Initialize the last (order_max) entry with a null buddy */
    list_init(&ctx->free_area[i].list);
    ctx->free_area[i].map = NULL;

    /*
     * Create the frames list
     */

    ctx->frames = kmalloc(frames_num * sizeof(struct frame), 0);
    if (!ctx->frames)
        panic("Buddy init");
    for (i = 0; i < frames_num; i++)
    {
        list_init(&ctx->frames[i].link);
        ctx->frames[i].refs = 1;
    }
    return 0;
}

/*
 * Dump buddy status
 */
void buddy_dump(struct buddy_sys *ctx, char *base)
{
    unsigned int i;
    size_t freemem = 0;
    struct list_link *frame_link;

    kprintf("-----------------------------------------\n");
    kprintf("   Buddy Dump\n");
    kprintf("-----------------------------------------\n");
    for (i = 0; i <= ctx->order_max; i++)
    {
        kprintf("order: %d", i);

        if (list_empty(&ctx->free_area[i].list))
            kprintf("   [ empty ]\n");
        else
        {
            kprintf("\n");
            for (frame_link = ctx->free_area[i].list.next;
                 frame_link != &ctx->free_area[i].list;
                 frame_link = frame_link->next)
            {
                struct frame *frame = list_container(frame_link, struct frame, link);
                unsigned int frame_idx = frame - ctx->frames;
                char *frame_ptr = base + (frame_idx << ctx->order_bit);
                kprintf("    [0x%p : 0x%p)\n", frame_ptr, frame_ptr + (1 << (ctx->order_bit+i)));
                freemem += (1 << (ctx->order_bit + i));
            }
        }
    }
    kprintf("free: %u\n", freemem);
}
