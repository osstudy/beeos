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

/*
 * This family of functions is used to do low-level port input and output.
 * The out* functions do port output, the in* functions do port input.
 * The b-suffix functions are byte-width, the w-suffix functions are
 * word-width, the l-suffix functions are long-width.
 */

#ifndef _BEEOS_ARCH_X86_IO_H_
#define _BEEOS_ARCH_X86_IO_H_

#include <stdint.h>

/*
 * Write a 8-bit byte to an output port.
 *
 * @param port  Port address (uint16_t)
 * @param val   Value (uint8_t)
 */
#define outb(port, val) \
    asm volatile("out %w1, %b0" : : "a"(val), "Nd"(port))

/*
 * Read a 8-bit byte from an input port.
 *
 * @param port  Port address (uint16_t)
 * @param val   Value (uint8_t)
 */
#define inb(port, val) \
    asm volatile("in %b0, %w1" : "=a"(val) : "Nd"(port))

/*
 * Write a 16-bit word to an output port.
 *
 * @param port  Port address (uint16_t)
 * @param val   Value (uint8_t)
 */
#define outw(port, val) \
    asm volatile("out %w1, %w0" : : "a"(val), "Nd"(port))

/*
 * Read a 16-bit word from an input port.
 *
 * @param port  Port address (uint16_t)
 * @param val   Value (uint16_t)
 */
#define inw(port, val) \
    asm volatile("in %w0, %w1" : "=a"(val) : "Nd"(port))

/**
* Write a 32-bit long word to an output port.
*
* @param port  Port address (uint16_t)
* @param val   Value (uint32_t)
*/
#define outl(port, val) \
    asm volatile("out %w1, %d0" : : "a"(val), "Nd"(port))

/**
 * Read a 32-bit long word from an input port.
 *
 * @param port  Port address
 * @return      Read value
 */
#define inl(port, val) \
    asm volatile("in %d0, %w1" : "=a"(val) : "Nd"(port))


#endif /* _BEEOS_ARCH_X86_IO_H_ */
