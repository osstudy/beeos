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

#ifndef BEEOS_ARCH_X86_PIC_H_
#define BEEOS_ARCH_X86_PIC_H_

/*
 * Mask an IRQ.
 * This prevents specific IRQs to be fired by the PIC.
 *
 * @param n     IRQ number
 */
void pic_mask(int n);

/*
 * Unmask an IRQ.
 * This allows specific IRQs to be fired by the PIC.
 *
 * @param n     IRQ number
 */
void pic_unmask(int n);

/*
 * PIC initialization.
 * After this all the IRQ numbers are masked.
 */
void pic_init(void);

#endif /* BEEOS_ARCH_X86_PIC_H_ */
