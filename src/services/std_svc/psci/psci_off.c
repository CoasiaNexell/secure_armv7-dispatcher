/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: DeokJin, Lee <truevirtue@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <sysheader.h>
#include <plat_pm.h>
#include <gic.h>
#include "psci.h"

/* External Function */
extern void psci_power_down_wfi(void);

/* External Variable */
extern volatile int g_fiq_flag;
extern volatile int g_cpu_kill_num;

void psci_cpu_off_handler(void)
{
	int cpu_id = armv7_get_cpuid();
	int ret;

	/* It is necessary in order to ensure sequential operation.*/
	if (cpu_id != 0) {
		g_fiq_flag |= (1 << cpu_id);
		do {
			cache_delay_ms(0xFFFFF);
		} while(g_fiq_flag & (1 << cpu_id));

		return;
	}

	/* cpu0 operated to the subcpu power off*/
	ret = s5p4418_cpu_off(g_cpu_kill_num);
	if (ret > 0)
		g_fiq_flag = 0;
}

/*************************************************************
 * Must be S5P4418
 * CPU Power Off sequence in S5P4418
 * Reference is made to function psci interface .
 *************************************************************/
int psci_do_cpu_off(unsigned int target_cpu)
{
	unsigned int cpu_id = ((target_cpu >> 0) & 0xFF);

	s5p4418_cpu_off(cpu_id);

	return s5p4418_cpu_check(cpu_id);	// 0: ON, 1:OFF, 2:PENDING
}
