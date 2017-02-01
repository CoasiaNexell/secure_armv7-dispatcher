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
#include "sysheader.h"

extern void     flushICache(void);
extern void     enableICache(CBOOL enable);

#if defined(CHIPID_NXP4330)
#define AUTO_DETECT_EMA     (0)
#else
#define AUTO_DETECT_EMA     (1)
#endif
#define EMA_VALUE           (1)     // Manual setting - 1(001): 1.1V, 3(011): 1.0V

#if (AUTO_DETECT_EMA == 1)
struct asv_tb_info {
	int ids;
	int ro;
};

static struct asv_tb_info asv_tables[] = {
	[0] = { .ids = 10, .ro = 110, },
	[1] = { .ids = 15, .ro = 130, },
	[2] = { .ids = 20, .ro = 140, },
	[3] = { .ids = 50, .ro = 170, },
	[4] = { .ids = 50, .ro = 170, },
};

#define ASV_ARRAY_SIZE  (int)(sizeof(asv_tables)/sizeof(asv_tables[0]))

static inline U32 MtoL(U32 data, int bits)
{
	U32 result = 0, mask = 1;
	int i = 0;

	for (i = 0; i < bits ; i++) {
		if (data & (1 << i))
			result |= mask << (bits-i-1);
	}

	return result;
}

CBOOL isEMA3(U32 ecid_1, U32 ecid_2)
{
	int field = 0;

	if (ecid_2 & 0x1) {
		int gs = MtoL((ecid_2 >> 1) & 0x07, 3);
		int ag = MtoL((ecid_2 >> 4) & 0x0F, 4);

		field = (ag - gs);
	} else {
		struct asv_tb_info *tb = &asv_tables[0];
		int ids;
		int ro;
		int ids_L, ro_L;
		int i = 0;

		ids = MtoL((ecid_1>>16) & 0xFF, 8);
		ro  = MtoL((ecid_1>>24) & 0xFF, 8);

		/* find ids Level */
		for (i = 0; i < ASV_ARRAY_SIZE; i++) {
			if (tb[i].ids >= ids)
				break;
		}
		ids_L = i < ASV_ARRAY_SIZE ? i : (ASV_ARRAY_SIZE - 1);

		/* find ro Level */
		for (i = 0; i < ASV_ARRAY_SIZE; i++) {
			if (tb[i].ro >= ro)
				break;
		}
		ro_L = i < ASV_ARRAY_SIZE ? i : (ASV_ARRAY_SIZE - 1);

		/* find Lowest ASV Level */
		field = ids_L > ro_L ? ro_L : ids_L;
	}

	return (field > 2 ? CTRUE : CFALSE);
}
#endif // #if (AUTO_DETECT_EMA == 1)

void setEMA(void)
{
#if (AUTO_DETECT_EMA == 1)
	U32 ecid_1, ecid_2;
#endif
	U32 ema, temp;

#if (AUTO_DETECT_EMA == 1)
	ecid_1 = ReadIO32(&pReg_ECID->ECID[1]);
	ecid_2 = ReadIO32(&pReg_ECID->ECID[2]);

	if (isEMA3(ecid_1, ecid_2))
		ema = 3;
	else
		ema = 1;
#else
	ema = EMA_VALUE; //; cortex-A9 L1 Cache EMA value (1: 1.1V, 3: 1.0V)
#endif

	enableICache(CFALSE);
	flushICache();

	// L2 Cache
	temp = ReadIO32(&pReg_Tieoff->TIEOFFREG[0]) & ~(7 << 22);
	temp |= (ema << 22);
	WriteIO32(&pReg_Tieoff->TIEOFFREG[0], temp);

	// L1 Cache
	temp = ReadIO32(&pReg_Tieoff->TIEOFFREG[1]) & ~(7 << 2);
	temp |= (ema << 2);
	WriteIO32(&pReg_Tieoff->TIEOFFREG[1], temp);

	enableICache(CTRUE);

	NOTICE("EMA VALUE : %s\r\n", (ema == 3 ? "011" : "001"));

	return;
}
