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

// Filter Parameters

#define BLOCK_SIZE 		(WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE)/4
#define F0_DEFAULT 		300.0
#define F0_INC_RATE 	150.0
#define F0_MIN			140.0
#define F0_MAX			3000.0

#define G_DEFAULT 		0.0
#define G_INC_RATE		1.0
#define G_MIN			-15.0
#define G_MAX			15.0

#define Q_DEFAULT_EQ	3.0
#define Q_DEFAULT_CR	0.707
#define Q_DEFAULT		1.0
#define Q_INC_RATE_EQ   1.0
#define Q_INC_RATE_CR   0.1
#define Q_MIN			0.1
#define Q_MAX			9.0

// IO
#define INPUT_BUFFER		0
#define OUTPUT_BUFFER_H		1
#define OUTPUT_BUFFER_L		2
#define OUTPUT_BUFFER_TEMP	3
#define TOTAL_IO_BUFFERS	4

#define BTN_DEBOUNCE		220
#define	AUTO_INC_TIME		1000

#define OUT_MAX_VOLUME		80
#define IN_MAX_GAIN			2.0

// Filter indexes
#define PARAM_EQ 	0
#define CROSS_LP 	1
#define CROSS_HP 	2
#define NUM_FILTERS 3

#define BIQ_EQ 		0
#define BIQ_BW_LP	1
#define BIQ_BW_HP	2
#define BIQ_LR_LP	3
#define BIQ_LR_HP	4
#define NUM_BIQUADS 5

#define NUM_STAGES 		1
#define NUM_STAGES_2 	2
#define VARIATOR_TIME  1

// Debug e tests
#define TEST
//#define TRACE_DEBUG

#undef CYCLE_COUNTER
//#define CYCLE_COUNTER

#endif /* DEFINES_H_ */
