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

#define NUM_STAGES 1
#define NUM_STAGES_2 2
#define BLOCK_SIZE (WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE)/4
#define F0_DEFAULT 100.0
#define G_DEFAULT 0.0
#define Q_DEFAULT 3.0

#define TEST
//#define TRACE_DEBUG

#undef CYCLE_COUNTER
//#define CYCLE_COUNTER

#endif /* DEFINES_H_ */
