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

#ifndef _BEEOS_RAMDISK_H_
#define _BEEOS_RAMDISK_H_

#include <sys/types.h>

#define BLOCK_SIZE  512

void ramdisk_init(void *addr, size_t size);
ssize_t ramdisk_read_block(void *buf, size_t blocknum);
ssize_t ramdisk_write_block(void *buf, size_t blocknum);

#endif /* _BEEOS_RAMDISK_H_ */
