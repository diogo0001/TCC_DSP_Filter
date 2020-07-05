/*
 * interface.h

 *
 *  Created on: 28 de abr de 2020
 *      Author: Diogo Tavares
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <defines_sys.h>
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "crossover.h"
#include "equalizer.h"
#include "filter_def.h"

//typedef enum {
//	EQ,
//	CROSSOVER
//} filter_type_enum;
//
//typedef enum {
//	BUTTER,
//	LINKWITZ
//} crossover_type_enum;

typedef enum {
	MENU_VOLUME_OUTPUT,
	MENU_EQ_ONOFF,
	MENU_EQ_FREQUENCY,
	MENU_EQ_Q,
	MENU_EQ_GAIN,
	MENU_CROSSOVER_ONOFF,
	MENU_CROSSOVER_FREQUENCY,
	MENU_CROSSOVER_TYPE,
	MENU_CROSSOVER_POLARITY
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
		//uint16_t bypass   	 	: 1;
		uint32_t disp_enable   	: 1;
		//uint16_t i2c_flag	  	: 1;
		uint32_t filter_order 	: 1;
		uint32_t enter		 	: 1;
		//uint16_t butter		 	: 1;
		uint32_t eq				: 1;
		uint32_t cross			: 1;
		//uint16_t check_var	 	: 3;
		uint32_t add_i2c_index	: 4;
		uint32_t vol			: 7;
		uint32_t vol_en			: 1;
		uint32_t polarity		: 1;
		uint32_t polarit_en		: 1;
		uint32_t unused			: 13;
		};
	uint32_t allControls;
}sys_controls_union;

sys_controls_union interface_init(coefs_buffers_instance *buffers, filter_instance *filters, arm_biquad_casd_df1_inst_f32 *biquads);
void interface(float32_t **io, filter_instance *filters, arm_biquad_casd_df1_inst_f32 *biquads, sys_controls_union *controls);
void states_control(filter_instance *filters, sys_controls_union *controls);
void menuValueAdd(sys_controls_union *controls);
void menuValueSub(sys_controls_union *controls);
void menuValueEnter(sys_controls_union *controls);
uint8_t check_variations(filter_instance *filters);

void menuPrintLines(char* firstLine, char* secondLine, sys_controls_union *controls);

void menuUpdateValue(char* value);

#endif /* INTERFACE_H_ */
