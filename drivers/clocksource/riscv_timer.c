/*
 * Copyright (C) 2012 Regents of the University of California
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

#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/delay.h>
#include <clocksource/riscv_timer.h>
#include <asm/timex.h>
#include <asm/sbi.h>

#define MINDELTA 100
#define MAXDELTA 0x7fffffff

/*
 * See <clocksource/riscv_timer.h> for the rationale behind pre-allocating
 * per-cpu timers on RISC-V systems.
 */
static DEFINE_PER_CPU(struct clock_event_device, riscv_clock_event) = {
	.name           = "riscv_timer_clockevent",
	.features       = CLOCK_EVT_FEAT_ONESHOT,
	.rating         = 300,
	.set_state_oneshot  = NULL,
	.set_state_shutdown = NULL,
};

static struct clocksource cs = {
	.name = "riscv_clocksource",
	.rating = 300,
	.mask = CLOCKSOURCE_MASK(BITS_PER_LONG),
	.flags = CLOCK_SOURCE_IS_CONTINUOUS,
};

static int set_next_event(unsigned long delta, struct clock_event_device *ce)
{
	/*
	 * init_clockevent() allocates a timer for each CPU. Since we're writing
	 * the timer comparison register here we can't allow the timers to cross
	 * harts.
	 */
	BUG_ON(ce != this_cpu_ptr(&riscv_clock_event));
	sbi_set_timer(get_cycles64() + delta);
	return 0;
}

void riscv_timer_interrupt(void)
{
	struct clock_event_device *evdev = this_cpu_ptr(&riscv_clock_event);

	evdev->event_handler(evdev);
}

static unsigned long long rdtime(struct clocksource *cs)
{
	/*
	 * It's guaranteed that all the timers across all the harts are
	 * synchronized within one tick of each other, so while this could
	 * technically go backwards when hopping between CPUs, practically it
	 * won't happen.
	 */
	return get_cycles64();
}

void clocksource_riscv_init(void)
{
	cs.read = rdtime;
	clocksource_register_hz(&cs, riscv_timebase);
}

void __init init_clockevent(void)
{
	struct clock_event_device *ce = this_cpu_ptr(&riscv_clock_event);

	ce->cpumask = cpumask_of(smp_processor_id());
	ce->set_next_event = set_next_event;

	clockevents_config_and_register(ce, riscv_timebase, MINDELTA, MAXDELTA);

	/* Enable timer interrupts. */
	csr_set(sie, SIE_STIE);
}

