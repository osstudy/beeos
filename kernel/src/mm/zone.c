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

#include "zone.h"
#include "util.h"
#include <sys/types.h>

void *zone_alloc(struct zone_st *ctx, int order)
{
    struct frame *frame;

    frame = buddy_alloc(&ctx->buddy, order);
    if (!frame)
        return NULL;
    frame->refs++;
    return (ctx->addr + ctx->frame_size*(frame-ctx->buddy.frames));
}

void zone_free(struct zone_st *ctx, void *ptr, int order)
{
    int i;
    struct frame *frame;
    
    i = ((char *) ptr - ctx->addr) / ctx->frame_size;
    frame = &ctx->buddy.frames[i];
    if (frame->refs > 0)
    {
        frame->refs--;
        if (frame->refs == 0)
            buddy_free(&ctx->buddy, frame, order);
    }
}

int zone_init(struct zone_st *ctx, void *addr, size_t size,
        size_t frame_size, int flags)
{
    ctx->addr = addr;
    ctx->size = size;
    ctx->frame_size = frame_size;
    ctx->flags = flags;
    ctx->next = NULL;
    return buddy_init(&ctx->buddy, size/frame_size, frame_size);
}

void zone_dump(struct zone_st *ctx)
{
    buddy_dump(&ctx->buddy, ctx->addr);
}
