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
#include <psci.h>

void psci_system_off(void)
{
	DBGOUT(" %s \r\n", __func__);
	return;
}

/*************************************************************
 * System Reset the Reference Fucntion
 *************************************************************/
void psci_system_reset(unsigned int reason)
{
	if (reason != 0) {
		mmio_write_32(&pReg_Alive->ALIVESCRATCHRST1, 0xFFFFFFFF);
		mmio_write_32(&pReg_Alive->ALIVESCRATCHSET1, reason);
	}

	/* s5p4418 Reset  */
	s5p4418_reset_cpu();
}
