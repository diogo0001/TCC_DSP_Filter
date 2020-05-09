/*
 * interface.c
 *
 *  Created on: 28 de abr de 2020
 *      Author: kulie
 */

#include "interface.h"

menu_state_enum menu_state = MENU_CROSSOVER_FREQUENCY;
filter_type_enum filter_type = EQ;
uint8_t nav_count;

//float32_t filter_param;
float32_t f0, G,Q;
char ValueStr[10];

vari_eq_instance variator_t;

//***********************************************************************

sys_flags_union interface_init(coefs_buffers_instance *buffers, filter_instance *filters, arm_biquad_casd_df1_inst_f32 *biquads){
	filter_init(&filters[PARAM_EQ], buffers->eq_coefs, buffers->eq_state);
	filter_init(&filters[CROSS_LP], buffers->lp_coefs, buffers->lp_state);
	filter_init(&filters[CROSS_HP], buffers->hp_coefs, buffers->hp_state);

	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_EQ], NUM_STAGES,filters[PARAM_EQ].coefs, filters[PARAM_EQ].state);

	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_BW_LP], NUM_STAGES, filters[CROSS_LP].coefs, filters[CROSS_LP].state);
	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_BW_HP], NUM_STAGES, filters[CROSS_HP].coefs, filters[CROSS_HP].state);

	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_LR_LP], NUM_STAGES_2, filters[CROSS_LP].coefs, filters[CROSS_LP].state);
	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_LR_HP], NUM_STAGES_2 ,filters[CROSS_HP].coefs, filters[CROSS_HP].state);

	f0 = F0_DEFAULT;
	Q = Q_DEFAULT_CR;
	nav_count = 0;

	set_f0(&filters[PARAM_EQ],7.0);
	set_Q(&filters[PARAM_EQ],7.0);
	set_G(&filters[PARAM_EQ],12.0);

	set_Q(&filters[CROSS_LP],Q);
	set_Q(&filters[CROSS_HP],Q);

	sys_flags_union flags;
	flags.allFlags = 0;
	flags.copy_coefs = 1;
	flags.butter = 0;

	sprintf(ValueStr, "%f.1", get_f0(&filters[CROSS_LP]));
	menuPrintLines("CROSSOVER", ValueStr, NULL);

#ifdef TEST
	variator_t.freq_max   = F0_MAX;
    variator_t.freq_min   = F0_MIN;
	variator_t.freq_step  = F0_INC_RATE;
	variator_t.time_count = 0;
	variator_t.time_limit = VARIATOR_TIME; 		// vel
	variator_t.up_dw_cout = 0;
	variator_t.up_filter  = 1;
#endif

	return flags;
}


//***********************************************************************

void interface(float32_t **io,  filter_instance *filters, arm_biquad_casd_df1_inst_f32 *biquads, sys_flags_union *flags){

	uint8_t check_f0, check_Q, check_G;

	arm_biquad_cascade_df1_f32(&biquads[BIQ_EQ], io[INPUT_BUFFER], io[OUTPUT_BUFFER_TEMP], BLOCK_SIZE);

	if(flags->butter == 0){		//(cross_type == BUTTER){
		Q  = 0.5;
		arm_biquad_cascade_df1_f32(&biquads[BIQ_BW_LP], io[OUTPUT_BUFFER_TEMP], io[OUTPUT_BUFFER_L], BLOCK_SIZE);
		arm_biquad_cascade_df1_f32(&biquads[BIQ_BW_HP], io[OUTPUT_BUFFER_TEMP], io[OUTPUT_BUFFER_H], BLOCK_SIZE);
	}
	else{
		Q = 0.707;
		arm_biquad_cascade_df1_f32(&biquads[BIQ_LR_LP], io[OUTPUT_BUFFER_TEMP], io[OUTPUT_BUFFER_L], BLOCK_SIZE);
		arm_biquad_cascade_df1_f32(&biquads[BIQ_LR_HP], io[OUTPUT_BUFFER_TEMP], io[OUTPUT_BUFFER_H], BLOCK_SIZE);
	}

	check_f0 = cross_check_f0_variation(&filters[CROSS_LP],&filters[CROSS_HP],f0);
	check_Q = cross_check_Q_variation(&filters[CROSS_LP],&filters[CROSS_HP],Q);

	if(check_f0 != 0){
		sprintf(ValueStr, "%f", get_f0(&filters[CROSS_LP]));
		menuPrintLines("f0", ValueStr, NULL);
		check_f0 = 0;
	}
	if(check_Q != 0){
		sprintf(ValueStr, "%f", get_Q(&filters[CROSS_LP]));
		menuPrintLines("Q", ValueStr, NULL);
		check_Q = 0;
	}

	//variator(&variator_t, &filters[PARAM_EQ], get_f0(&filters[PARAM_EQ]));

	states_control();
}

//***********************************************************************


void states_control(filter_instance *filters, sys_flags_union *flags){

	switch(menu_state){
		case MENU_HOME:
			break;

		case MENU_CROSSOVER:
			break;

		case MENU_CROSSOVER_FREQUENCY:
			break;

		case MENU_CROSSOVER_Q:
			break;

		case MENU_EQ:
			break;

		case MENU_EQ_Q:
			break;

		case MENU_EQ_FREQUENCY:

			break;

		case MENU_EQ_GAIN:
			break;

		default: break;
	}
}

//***********************************************************************

void menuValueAdd(sys_flags_union *flags){

	switch(menu_state){

		case MENU_HOME:
			break;

		case MENU_CROSSOVER:
			break;

		case MENU_CROSSOVER_FREQUENCY:
			if(f0 < F0_MAX)
				f0 += F0_INC_RATE;
			break;

		case MENU_CROSSOVER_Q:
			if(Q < Q_MAX)
				Q += Q_INC_RATE_CR;
			break;

		case MENU_EQ:
			break;

		case MENU_EQ_FREQUENCY:
			if(f0 < F0_MAX)
				f0 += F0_INC_RATE;
			break;

		case MENU_EQ_Q:
			if(Q < Q_MAX)
				Q += Q_INC_RATE_EQ;
			break;

		case MENU_EQ_GAIN:
			if(G < G_MAX)
				G += G_INC_RATE;
			break;

		default: break;
	}
}

//***********************************************************************

void menuValueSub(sys_flags_union *flags){

	switch(menu_state){
		case MENU_HOME:
			break;

		case MENU_CROSSOVER:
			break;

		case MENU_CROSSOVER_FREQUENCY:
			if(f0 < F0_MAX)
				f0 -= F0_INC_RATE;
			break;

		case MENU_CROSSOVER_Q:
			if(Q < Q_MAX)
				Q -= Q_INC_RATE_CR;
			break;

		case MENU_EQ:
			break;

		case MENU_EQ_FREQUENCY:
			if(f0 < F0_MAX)
				f0 -= F0_INC_RATE;
			break;

		case MENU_EQ_Q:
			if(Q < Q_MAX)
				Q -= Q_INC_RATE_EQ;
			break;

		case MENU_EQ_GAIN:
			if(G < G_MAX)
				G -= G_INC_RATE;
			break;

		default: break;
	}
}

//***********************************************************************

void menuValueEnter(sys_flags_union *flags){
	flags->enter = 1;
}


//***********************************************************************

void menuPrintLines(char* firstLine, char* secondLine, char* unity){
	ssd1306_SetCursor(0,0);
	ssd1306_Fill(Black);
	ssd1306_WriteString(firstLine, Font_11x18, White);
	ssd1306_WriteChar(':', Font_11x18, White);
	ssd1306_SetCursor(0,30);
	ssd1306_WriteString(secondLine, Font_16x26, White);

	if(unity != NULL){
		ssd1306_SetCursor(70,30);
		ssd1306_WriteString(unity, Font_16x26, White);
	}
	ssd1306_UpdateScreen();
}

