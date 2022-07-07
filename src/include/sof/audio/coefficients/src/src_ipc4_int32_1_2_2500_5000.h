/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright(c) 2022 Intel Corporation. All rights reserved.
 *
 */

/** \cond GENERATED_BY_TOOLS_TUNE_SRC */
#include <sof/audio/src/src.h>
#include <stdint.h>

const int32_t src_int32_1_2_2500_5000_fir[44] = {
	92264,
	-964298,
	-1868435,
	753449,
	6033863,
	5159487,
	-7478051,
	-18530652,
	-5041824,
	28778130,
	37883728,
	-11541415,
	-74555025,
	-53886474,
	68545714,
	154209033,
	39426567,
	-220084170,
	-298911528,
	102280042,
	880520439,
	1516662804,
	1516662804,
	880520439,
	102280042,
	-298911528,
	-220084170,
	39426567,
	154209033,
	68545714,
	-53886474,
	-74555025,
	-11541415,
	37883728,
	28778130,
	-5041824,
	-18530652,
	-7478051,
	5159487,
	6033863,
	753449,
	-1868435,
	-964298,
	92264

};

struct src_stage src_int32_1_2_2500_5000 = {
	1, 0, 1, 44, 44, 2, 1, 0, 1,
	src_int32_1_2_2500_5000_fir};
/** \endcond */