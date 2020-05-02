/*
 * defines.h
 *
 *  Created on: 28 de abr de 2020
 *      Author: kulie
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#include <arm_math.h>
#include <wolfson_pi_audio.h>
#include <diag/Trace.h>

// General Use
#define NUM_STAGES 1
#define NUM_STAGES_2 2
#define BLOCK_SIZE (WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE)/4
#define F0_DEFAULT 200.0
#define G_DEFAULT 0.0
#define Q_DEFAULT 0.5

// IO
#define INPUT_BUFFER		0
#define OUTPUT_BUFFER_H		1
#define OUTPUT_BUFFER_L		2
#define OUTPUT_BUFFER_TEMP	3
#define TOTAL_IO_BUFFERS	4

// Filter types
#define PARAM_EQ 	0
#define CROSS_LP 	1
#define CROSS_HP 	2
#define NUM_FILTERS 3
//#define CROSS_BW_LP 1
//#define CROSS_BW_HP 2
//#define CROSS_LR_LP 3
//#define CROSS_LR_HP 4


// Biquad types
#define BIQ_EQ 		0
#define BIQ_BW_LP	1
#define BIQ_BW_HP	2
#define BIQ_LR_LP	3
#define BIQ_LR_HP	4
#define NUM_BIQUADS 5

// Debug e tests
#define TEST
#define TRACE_DEBUG

#undef CYCLE_COUNTER
//#define CYCLE_COUNTER

#endif /* DEFINES_H_ */
