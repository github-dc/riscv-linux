/*
 * Copyright (C) 2017 SiFive
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 */

#ifndef _LINUX_TIMER_RISCV_H
#define _LINUX_TIMER_RISCV_H

/*
 * All RISC-V systems have a timer attached to every hart.  These timers can be
 * read by the 'rdcycle' pseudo instruction, and can use the SBI to setup
 * events. There is guaranteed to be exactly one timer per hart on all
 * RISC-V systems.
 */
void clocksource_riscv_init(void);

void __init init_clockevent(void);

#endif
