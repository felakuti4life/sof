/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright(c) 2022 Intel Corporation. All rights reserved.
 *
 */

/** \cond GENERATED_BY_TOOLS_TUNE_SRC */
#include <sof/audio/src/src.h>
#include <stdint.h>

const int32_t src_int32_1_2_3401_5000_fir[64] = {
	-215059,
	343353,
	967908,
	-138853,
	-2266856,
	-1294917,
	3442531,
	4606166,
	-2964392,
	-9547171,
	-1201646,
	14242400,
	10560413,
	-14999111,
	-24611242,
	7112011,
	39537187,
	13255746,
	-47816108,
	-46570877,
	39244993,
	87674840,
	-3164913,
	-124553313,
	-69404491,
	137804306,
	186966304,
	-96277075,
	-374161567,
	-84159399,
	830404442,
	1674668039,
	1674668039,
	830404442,
	-84159399,
	-374161567,
	-96277075,
	186966304,
	137804306,
	-69404491,
	-124553313,
	-3164913,
	87674840,
	39244993,
	-46570877,
	-47816108,
	13255746,
	39537187,
	7112011,
	-24611242,
	-14999111,
	10560413,
	14242400,
	-1201646,
	-9547171,
	-2964392,
	4606166,
	3442531,
	-1294917,
	-2266856,
	-138853,
	967908,
	343353,
	-215059

};

struct src_stage src_int32_1_2_3401_5000 = {
	1, 0, 1, 64, 64, 2, 1, 0, 1,
	src_int32_1_2_3401_5000_fir};
/** \endcond */