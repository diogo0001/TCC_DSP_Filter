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
	MENU_EQ_GAIN,
	MENU_GAIN_INPUT,
	MENU_VOLUME_OUTPUT
} menu_state_enum;

typedef struct{
	float32_t eq_coefs[5*NUM_STAGES];
	float32_t eq_state[4*NUM_STAGES];
	float32_t hp_coefs[5*NUM_STAGES_2];
	float32_t hp_state[4*NUM_STAGES_2];
	float32_t lp_coefs[5*NUM_STAGES_2];
	float32_t lp_state[4*NUM_STAGES_2];
}coefs_buffers_instance;

typedef union {
	struct {
		uint8_t bypass   	 : 1;
		uint8_t btn_enable   : 1;
		uint8_t copy_coefs   : 1;
		uint8_t filter_order : 1;
		uint8_t check_var	 : 2;
		uint8_t enter		 : 1;
		uint8_t butter		 : 1;
		};
	uint8_t allFlags;
}sys_flags_union;

sys_flags_union interface_init(coefs_buffers_instance *buffers, filter_instance *filters, arm_biquad_casd_df1_inst_f32 *biquads);
void interface(float32_t **io, filter_instance *filters, arm_biquad_casd_df1_inst_f32 *biquads, sys_flags_union *flags);
void states_control(filter_instance *filters, sys_flags_union *flags);
void menuValueAdd(sys_flags_union *flags);
void menuValueSub(sys_flags_union *flags);
void menuValueEnter(sys_flags_union *flags);
void menuPrintLines(char* firstLine, char* secondLine, char* unity);

#endif /* INTERFACE_H_ */
