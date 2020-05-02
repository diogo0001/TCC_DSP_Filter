/*
 * interface.h

 *
 *  Created on: 28 de abr de 2020
 *      Author: kulie
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "defines.h"
#include "crossover.h"
#include "equalizer.h"
#include "filter_def.h"

typedef enum {
	EQ,
	CROSSOVER
} filter_type_enum;

typedef enum {
	BUTTER,
	LINKWITZ
} crossover_type_enum;

typedef enum {
	MENU_HOME,
	MENU_CROSSOVER,
	MENU_CROSSOVER_FREQUENCY,
	MENU_CROSSOVER_Q,
	MENU_EQ,
	MENU_EQ_Q,
	MENU_EQ_FREQUENCY,
	MENU_EQ_GAIN
} menu_state_enum;

//typedef struct{
//	char eqStr[] 		= "EQ";
//	char crossoverStr[] = "CROSSOVER";
//	char backStr[]		= "Back";
//	char gainStr[] 		= "Gain";
//	char frequencyStr[]	= "Frequency";
//	char qStr[]			= "Q";
//	char butterStr[] 	= "Butter";
//	char linkwitzStr[]	= "Linkwitz";
//	char dbStr[]		= "dB";
//	char hzStr[]		= "Hz";
//}menu_strings_instance;

typedef struct{
	float32_t eq_coefs[5*NUM_STAGES];
	float32_t eq_state[4*NUM_STAGES];
	float32_t hp_coefs[5*NUM_STAGES_2];
	float32_t hp_state[4*NUM_STAGES_2];
	float32_t lp_coefs[5*NUM_STAGES_2];
	float32_t lp_state[4*NUM_STAGES_2];
}coefs_buffers_instance;

void interface_init(coefs_buffers_instance *buffers, filter_instance *filters, arm_biquad_casd_df1_inst_f32 *biquads);
void interface(float32_t **io, arm_biquad_casd_df1_inst_f32 *biquads);
void updateScreen(void);
void menuValueAdd(void);
void menuValueSub(void);
void menuValueEnter(void);
void menuPrintLines(char* firstLine, char* secondLine, char* unity);

#endif /* INTERFACE_H_ */
