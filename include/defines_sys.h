/*
 * defines.h
 *
 *  Created on: 28 de abr de 2020
 *      Author: Diogo Tavares
 */

#ifndef DEFINES_SYS_H_
#define DEFINES_SYS_H_

#include <arm_math.h>
#include <wolfson_pi_audio.h>
#include <diag/Trace.h>
#include <dwt.h>

// Filter Parameters
#define TOTAL_MENU_STATES 	8

#define BLOCK_SIZE 		(WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE)/4
#define F0_DEFAULT 		200
#define F0_INC_RATE 	20
#define F0_MIN			60
#define F0_MAX			3000
#define F0_MAX_EQ		8000

#define G_DEFAULT 		0
#define G_INC_RATE		1
#define G_MIN			-18
#define G_MAX			18

#define Q_DEFAULT_EQ	3.0
#define Q_LINKWITZ		0.707
#define Q_BUTTERW		0.5
#define Q_DEFAULT		1.0
//#define Q_INC_RATE_EQ   1.0
//#define Q_INC_RATE_CR   0.1
#define Q_MIN			1.0
#define Q_MAX			15.0

// IO
#define INPUT_BUFFER		0
#define OUTPUT_BUFFER_H		1
#define OUTPUT_BUFFER_L		2
#define OUTPUT_BUFFER_TEMP	3
#define TOTAL_IO_BUFFERS	4

#define BTN_DEBOUNCE		220
#define	AUTO_INC_TIME		1000

#define OUT_MAX_VOLUME		100
#define OUT_MIN_VOLUME		0
#define OUT_DEFAUL_VOLUME	80

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
#define VARIATOR_TIME 	1

// Debug e tests
//#define TEST

//#define TRACE_DEBUG

//#define CYCLE_COUNTER

#define ENABLE_POLARITY

#endif /* DEFINES_SYS_H_ */
