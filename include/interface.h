/*
 * interface.h

 *
 *  Created on: 28 de abr de 2020
 *      Author: kulie
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <defines_sys.h>
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "crossover.h"
#include "equalizer.h"
#include "filter_def.h"

//char crossover_str[] 		= "Crossover";
//char on_str[] 				= "ON";
//char off_str[] 				= "OFF";
//char crossover_freq_str[]	= "Crossover: fc";
//char crossover_type_str[]	= "Crossover: Type";
//char eq_str[] 				= "EQ";
//char eq_freq_str[]			= "EQ: f0";
//char eq_Q_str[]				= "EQ: Q";
//char eq_G_str[]				= "EQ: G";
//char inp_gain_str[]			= "Input Gain:";
//char outp_vol_str[]			= "Volume Out";
//char link_str[]			= "Linkwitz";
//char butt_str[]			= "Butterworth";

typedef enum {
	EQ,
	CROSSOVER
} filter_type_enum;

typedef enum {
	BUTTER,
	LINKWITZ
} crossover_type_enum;

typedef enum {
	MENU_CROSSOVER_ONOFF,
	MENU_CROSSOVER_FREQUENCY,
	MENU_CROSSOVER_TYPE,
	MENU_EQ_ONOFF,
	MENU_EQ_FREQUENCY,
	MENU_EQ_Q,
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
		uint16_t bypass   	 	: 1;
		uint16_t disp_enable   	: 1;
		uint16_t copy_coefs  	: 1;
		uint16_t filter_order 	: 1;
		uint16_t enter		 	: 1;
		uint16_t butter		 	: 1;
		uint16_t eq				: 1;
		uint16_t cross			: 1;
		uint16_t check_var	 	: 3;
		uint16_t gain_in		: 5;
		};
	uint16_t allControls;
}sys_controls_union;

sys_controls_union interface_init(coefs_buffers_instance *buffers, filter_instance *filters, arm_biquad_casd_df1_inst_f32 *biquads);
void interface(float32_t **io, filter_instance *filters, arm_biquad_casd_df1_inst_f32 *biquads, sys_controls_union *controls, uint8_t *aTxBuffer);
void states_control(filter_instance *filters, sys_controls_union *controls, uint8_t *aTxBuffer);
void menuValueAdd(sys_controls_union *controls);
void menuValueSub(sys_controls_union *controls);
void menuValueEnter(sys_controls_union *controls);
uint8_t check_variations(filter_instance *filters);

void menuPrintLines(char* firstLine, char* secondLine, char* unity);

void menuUpdateValue(char* value);

#endif /* INTERFACE_H_ */
