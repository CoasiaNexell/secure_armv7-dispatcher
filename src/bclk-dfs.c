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
#include <gic.h>
#include <dpc.h>
#include <nx_reg_base.h>

#define SGI_IRQ_8		8

extern volatile unsigned int g_smc_id;
extern volatile unsigned int g_fiq_flag;

/* External function */
extern int  armv7_get_cpuid(void);
extern void set_nonsecure_mode(void);
extern void set_secure_mode(void);

extern void chche_delay_ms(int ms);

extern void s5p4418_cpuidle(int cpu_id, int int_id);
extern void s5p4418_change_pll(volatile int *base, unsigned int pll_data);
extern void smc_set_monitor_fiq(int enable);

void s5p4418_bclk_dfs_handler(void)
{
	int cpu_id = armv7_get_cpuid();

	/* Sequenceal Unlock */
	do {
		/* waitng for event */
		 __asm__ __volatile__ ("wfe");
	} while (g_fiq_flag & (1 << cpu_id));
}

/*******************************************************************************
 * For the BCLK DFS, the remaining CPU changes to the WakeUp state.
 ******************************************************************************/
static void cpu_up_force(void)
{
	g_fiq_flag = 0;
	/* wake up cpux (send event) */
	__asm__ __volatile__ ("sev");
}

/*******************************************************************************
 * For the BCLK DFS, the remaining CPU changes to the IDLE state.
 * That prevents the memory access to the other CPU.
 ******************************************************************************/
static void cpu_down_force(void)
{
	int cpu_id = armv7_get_cpuid();

	g_smc_id = 0x82000009;

	/* Using the SGI(GIC), and calls to the idle of the CORE FIQ. */
	g_fiq_flag = (0xF & ~(1 << cpu_id));
	s5p4418_cpuidle(g_fiq_flag, SGI_IRQ_8);
	cache_delay_ms(0xFFFF);
}

static void send_directcmd(unsigned int cmd, unsigned char chip,
				unsigned int mrx, unsigned short value)
{
	volatile char* drex_reg = (char*)PHY_BASEADDR_DREX_MODULE_CH0_APB;
	mmio_write_32((drex_reg + 0x10), ((cmd << DIRCMD_TYPE_SHIFT)
		| (chip << DIRCMD_CHIP_SHIFT) | (mrx << 16) | value));
}

static void sram_set_auto_refresh(int enb)
{
	char* drex_reg = (char*)PHY_BASEADDR_DREX_MODULE_CH0_APB;
	int value = 0;

	value = mmio_read_32(drex_reg + 0x00);
	value &= ~(0x1 << 5);

	if (enb)
		value |= (0x1 << 5);

	mmio_write_32((drex_reg + 0x00), value);
}

/*******************************************************************************
 * Must be S5P4418
 * PLL Change sequence in S5P4418 (Kernel)
 * must go through the following steps:
 *
 * Step 00. Change to cpu idle state
 * Step 01. DRAM Command - Precharge All
 * Step 02. Waiting for DPC Blank
 * Step 03. Change to PLL(P,M,S)
 * Step 04. Clear & Waiting for DPC Blank
 * Step 05. DRAM Command - Precharge All
 * Step 06. Change to cpu wakeup state
 ******************************************************************************/
void s5p4418_bclk_freqchange(unsigned int pll_data)
{
	int dpc_index = 0;

	/* Step 02. Waiting for DPC Blank */
	if (dpc_enabled(dpc_index)) {
		dpc_set_enable_all(dpc_index, 0);
		while (!dpc_get_pending_all(dpc_index))
			;
	}

	/* Step 01. DRAM Command - Precharge All */
	sram_set_auto_refresh(0);
	send_directcmd(DIRCMD_PALL, DIRCMD_CHIP_0, 0, 0);

	/* Step 03. change to PLL(P,M,S) */
	s5p4418_change_pll((volatile int*)PHY_BASEADDR_CLKPWR_MODULE, pll_data);

	/* Step 05. DRAM Command - Precharge All */
	sram_set_auto_refresh(1);
	send_directcmd(DIRCMD_PALL, DIRCMD_CHIP_0, 0, 0);

	/* Step 04. Clear & Waiting for DPC Blank */
	if (dpc_enabled(dpc_index)) {
		dpc_clear_pending_all(dpc_index);
		dpc_set_enable_all(dpc_index, 1);
	}
}

void s5p4418_bclk_dfs(unsigned int pll_data)
{
	/* Step 00. change to cpu idle state */
	cpu_down_force();

	s5p4418_bclk_freqchange(pll_data);

	/* Step 06. change to cpu wakeup state */
	cpu_up_force();
}

unsigned int tee_wait_bclk_dfs_flag;
void s5p4418_tee_bclkwait(void)
{
	int cpu_id = armv7_get_cpuid();

	tee_wait_bclk_dfs_flag |= (1 << cpu_id);
	while (tee_wait_bclk_dfs_flag & (1 << cpu_id))
		;
}

void s5p4418_tee_bclk(unsigned int pll_data, int wait_flag)
{
	int cpu_id = armv7_get_cpuid();

	while (wait_flag != tee_wait_bclk_dfs_flag)
		;

	s5p4418_bclk_freqchange(pll_data);

	tee_wait_bclk_dfs_flag = 0;
}
