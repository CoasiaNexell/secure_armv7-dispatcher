/*
 * Copyright (C) 2016  Nexell Co., Ltd. All Rights Reserved.
 * Nexell Co. Proprietary & Confidential
 *
 * Nexell informs that this code and information is provided "as is" base
 * and without warranty of any kind, either expressed or implied, including
 * but not limited to the implied warranties of merchantability and/or
 * fitness for a particular puporse.
 *
 * Module	:
 * File		:
 * Description	:
 * Author	: DeokJin, Lee <truevirtue@nexell.co.kr>
 * History	: 2017.03.14 new release
 */
#include <sysheader.h>
#include <gic.h>

void gic_sgi_caller(int cpu_id, int int_id)
{
#if 0
	char* cpu_base  = (char*)gicc_get_baseaddr();
#endif
	char* dist_base = (char*)gicd_get_baseaddr();

	int target_list = (0x0 << 24);		// Target List Filter [25:24]
	int cpu_target  = (cpu_id << 16);	// CPU Interface Number
	int sgi_id 	= (int_id << 0);	// Gnerate Interrupt ID
#if 0
	int eoir	= 0;

	/* Set the FIQ EN */
	gicc_set_eoir(cpu_base, (1 << 3));
	/*
	 * GIC set the path that is connected to the
	 * internal IP interrupts, generated. (0: Secure, 1: Non-Secure)
	 */
	gicd_set_group(dist_base, 0xFFFF00FF);
#endif
	/* GIC - Interrupt Set Enable */
	gicd_set_enable(dist_base, (1 << sgi_id));

	/* GIC - Software Generate Interrupt */
	gicd_set_sgir(dist_base, (target_list | cpu_target | sgi_id));
}


void s5p4418_cpuidle(int cpu_id, int int_id)
{
	gic_sgi_caller(cpu_id, int_id);
}
