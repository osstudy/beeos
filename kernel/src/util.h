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

#ifndef _BEEOS_UTIL_H_
#define _BEEOS_UTIL_H_

#include <stdint.h>
#include <sys/types.h>

#define ALIGN_UP(val, a) (((val) + ((a) - 1)) & ~((a) - 1))

#define ALIGN_DOWN(val, a) ((val) & ~((a) - 1))

/**
 * Align up to the next power of two
 * @param v     Value to align.
 * @return      Operation result.
 */
static inline unsigned long next_pow2(unsigned long v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

/**
 * First non zero bit position starting from left.
 * @param v     Value under analysis.
 * @return      Zero based bit position.
 *              If the input value is zero then returns 0.
 */
static inline unsigned int fnzb(unsigned long v)
{
    unsigned int n = 0;

    if (v >> 16) { v >>= 16; n += 16;}
    if (v >> 8)  { v >>= 8;  n += 8;}
    if (v >> 4)  { v >>= 4;  n += 4;}
    if (v >> 2)  { v >>= 2;  n += 2;}
    if (v >> 1)  { v >>= 1;  n += 1;}
    return n;
}

static inline int overlaps(uintptr_t b1, size_t sz1, uintptr_t b2, size_t sz2)
{
    uintptr_t e1;
    uintptr_t e2;

    e1 = b1 + sz1;
    e2 = b2 + sz2;
    return ((b1 < e2) & (b2 < e1));
}

static inline int iswithin(uintptr_t b1, size_t sz1, uintptr_t b2, size_t sz2)
{
    uintptr_t e1;
    uintptr_t e2;
    int res;

    if (sz1 != 0)
    {
        e1 = b1 + sz1 - 1;
        if (sz2 != 0)
        {
            e2 = b2 + sz2 - 1;
            /* e1 and e2 are end addresses, the sum is immune to overflow */
            res = ((b1 <= b2) && (e1 >= e2));
        }
        else
        {
            res = ((b1 <= b2) && (b2 <= e1));
        }
    }
    else
    {
        res = ((b1 == b2) && (sz2 == 0));
    }
    return res;
}

#define SWAP(a, b) do { \
    (a) ^= (b); \
    (b) ^= (a); \
    (a) ^= (b); \
    } while(0)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))

/**
 * Get a pointer to the struct start given a pointer to a member.
 * @param member_ptr    Struct member pointer.
 * @param struct_type   Type of the structure the element is embedded in.
 * @param member_name   Name of the member within the struct.
 */
#define struct_ptr(member_ptr, struct_type, member_name) \
    ((struct_type *)((char *)(member_ptr)-offsetof(struct_type,member_name)))

#endif /* _BEEOS_UTIL_H_ */
