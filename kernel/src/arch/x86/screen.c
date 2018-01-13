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

#include "io.h"
#include "vmem.h"
#include "driver/screen.h"

#define VIDEO_BUF	((uint16_t *) (0xB8000 + KVBASE))

#define BLACK			0
#define BLUE 			1
#define GREEN			2
#define CYAN			3
#define RED 			4
#define MAGENTA			5
#define BROWN 			6
#define LIGHT_GREY 		7
#define DARK_GREY 		8
#define LIGHT_BLUE 		9
#define LIGHT_GREEN 	10
#define LIGHT_CYAN 		11
#define LIGHT_RED 		12
#define LIGHT_MAGENTA 	13
#define LIGHT_BROWN		14
#define WHITE			15

#define MAKE_COLOR(bg, fg) 		((bg << 4) | fg)
#define MAKE_ENTRY(bg, fg, c)	((MAKE_COLOR(bg, fg) << 8) | c)

#define VGA_WIDTH		80
#define VGA_HEIGHT		25

/*
 * Copy the backbuffer and update the hardware cursor. 
 *
 * The framebuffer has two I/O ports, one for accepting the data, and one 
 * for describing the data being received. Port 0x03D4 is the port that 
 * describes the data and port 0x03D5 is for the data itself.
 */
void screen_update(struct screen *scr)
{
    int i;
	uint16_t pos = scr->pos_y * SCREEN_WIDTH + scr->pos_x;
    uint16_t *buf = (uint16_t *)VIDEO_BUF;

    /* Copy the backbuffer */
    for (i = 0; i < sizeof(scr->buf); i++)
        buf[i] = MAKE_ENTRY(BLACK, LIGHT_GREY, scr->buf[i]);

    /* Update the hardware cursor */
	outb(0x03D4, 14);  /* the highest 8 bits of the position */
	outb(0x03D5, pos >> 8);
	outb(0x03D4, 15);  /* the lowest 8 bits of the position */
	outb(0x03D5, pos);

    scr->dirty = 0;
}

