/*
 * interface.c
 *
 *  Created on: 28 de abr de 2020
 *      Author: kulie
 */

#include "interface.h"

int8_t nav_count;

//float32_t filter_param;
volatile float32_t f0,G,Q,f_eq;
char output_str_value[10];

vari_eq_instance variator_t;

FILE *CycleFile;
uint32_t cycleCount;


//***********************************************************************

sys_controls_union interface_init(coefs_buffers_instance *buffers, filter_instance *filters, arm_biquad_casd_df1_inst_f32 *biquads){
	filter_init(&filters[PARAM_EQ], buffers->eq_coefs, buffers->eq_state);
	filter_init(&filters[CROSS_LP], buffers->lp_coefs, buffers->lp_state);
	filter_init(&filters[CROSS_HP], buffers->hp_coefs, buffers->hp_state);

	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_EQ], NUM_STAGES,filters[PARAM_EQ].coefs, filters[PARAM_EQ].state);

	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_BW_LP], NUM_STAGES, filters[CROSS_LP].coefs, filters[CROSS_LP].state);
	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_BW_HP], NUM_STAGES, filters[CROSS_HP].coefs, filters[CROSS_HP].state);

	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_LR_LP], NUM_STAGES_2, filters[CROSS_LP].coefs, filters[CROSS_LP].state);
	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_LR_HP], NUM_STAGES_2 ,filters[CROSS_HP].coefs, filters[CROSS_HP].state);

	f0 = F0_DEFAULT;
	f_eq = F0_DEFAULT;
	Q = Q_DEFAULT_EQ;
	G = G_DEFAULT;
	nav_count = MENU_VOLUME_OUTPUT;

	set_Q(&filters[CROSS_LP],(float32_t)Q_LINKWITZ);
	set_Q(&filters[CROSS_HP],(float32_t)Q_LINKWITZ);

	sys_controls_union controls;
	controls.allControls = 0;
	controls.cross = 1;
	controls.eq = 1;
	controls.disp_enable = 1;

#ifdef TEST
	variator_t.freq_max   = F0_MAX;
    variator_t.freq_min   = F0_MIN;
	variator_t.freq_step  = F0_INC_RATE;
	variator_t.time_count = 0;
	variator_t.time_limit = VARIATOR_TIME; 		// time delay
	variator_t.up_dw_cout = 0;
	variator_t.up_filter  = 1;
#endif

#ifdef CYCLE_COUNTER
	CycleFile = fopen("cyclecounter_menuPrintLines.txt", "w");
	if (!CycleFile) {
		trace_printf("Error trying to open cycle counter file\n.");
		while(1);
	}

//	DWT_Reset();
//	cycleCount = DWT_GetValue();
//	fprintf(CycleFile, "\nFULL: %lu", (DWT_GetValue()- cycleCount));
#endif

	return controls;
}


//***********************************************************************

void interface(float32_t **io,  filter_instance *filters, arm_biquad_casd_df1_inst_f32 *biquads, sys_controls_union *controls){

	uint8_t temp_out_filter_index = OUTPUT_BUFFER_TEMP;

	if(controls->disp_enable)
		states_control(filters,controls);

	// Check parameters variations
	check_variations(filters);

	if(controls->eq == 0 && controls->cross == 0){
		uint16_t i;

		for(i=0;i<BLOCK_SIZE;i++){

			io[OUTPUT_BUFFER_L][i] = io[INPUT_BUFFER][i];
			io[OUTPUT_BUFFER_H][i] = io[INPUT_BUFFER][i];
		}
		return;
	}
	else if(controls->eq==1 && controls->cross==1){
		temp_out_filter_index = OUTPUT_BUFFER_TEMP;
	}
	else if(controls->eq==0 && controls->cross==1){
		temp_out_filter_index = INPUT_BUFFER;
	}
	else if(controls->eq==1 && controls->cross==0){
		arm_biquad_cascade_df1_f32(&biquads[BIQ_EQ], io[INPUT_BUFFER], io[OUTPUT_BUFFER_TEMP], BLOCK_SIZE);

		uint16_t i;

		for(i=0;i<BLOCK_SIZE;i++){

			io[OUTPUT_BUFFER_L][i] = io[OUTPUT_BUFFER_TEMP][i];
			io[OUTPUT_BUFFER_H][i] = io[OUTPUT_BUFFER_TEMP][i];
		}
		return;
	}

	// Filters process
	if(controls->eq){
		arm_biquad_cascade_df1_f32(&biquads[BIQ_EQ], io[INPUT_BUFFER], io[temp_out_filter_index], BLOCK_SIZE);

	}
	if(controls->cross){
		if(controls->butter){
			cross_check_Q_variation(&filters[CROSS_LP], &filters[CROSS_HP], (float32_t)Q_BUTTERW);
			arm_biquad_cascade_df1_f32(&biquads[BIQ_BW_LP], io[temp_out_filter_index], io[OUTPUT_BUFFER_L], BLOCK_SIZE);
			arm_biquad_cascade_df1_f32(&biquads[BIQ_BW_HP], io[temp_out_filter_index], io[OUTPUT_BUFFER_H], BLOCK_SIZE);
		}
		else{
			cross_check_Q_variation(&filters[CROSS_LP], &filters[CROSS_HP], (float32_t)Q_LINKWITZ);
			arm_biquad_cascade_df1_f32(&biquads[BIQ_LR_LP], io[temp_out_filter_index], io[OUTPUT_BUFFER_L], BLOCK_SIZE);
			arm_biquad_cascade_df1_f32(&biquads[BIQ_LR_HP], io[temp_out_filter_index], io[OUTPUT_BUFFER_H], BLOCK_SIZE);
		}
	}


#ifdef TEST
	f0 = f0_variator(&variator_t, get_f0(&filters[CROSS_LP]));
#endif

}

//***********************************************************************

uint8_t check_variations(filter_instance *filters){

	uint8_t index = 0;

	index = cross_check_f0_variation(&filters[CROSS_LP], &filters[CROSS_HP], f0);
//	index = cross_check_Q_variation(&filters[CROSS_LP], &filters[CROSS_HP], Q);

	index = eq_check_f0_variation((&filters[PARAM_EQ]), f_eq);
	index = eq_check_Q_variation((&filters[PARAM_EQ]), Q);
	index = eq_check_G_variation((&filters[PARAM_EQ]), G);

	return index;
}
//***********************************************************************

void states_control(filter_instance *filters, sys_controls_union *controls){

	switch(nav_count){

		case MENU_CROSSOVER_ONOFF:
			if(controls->cross)
				menuPrintLines("Cross:", "ON",controls);
			else
				menuPrintLines("Cross:", "OFF", controls);
			break;

		case MENU_CROSSOVER_FREQUENCY:
//			f0 = get_f0(&filters[CROSS_LP]);
			sprintf(output_str_value, "%d", (uint16_t)f0);
			menuPrintLines("Cross fc:", output_str_value, controls);
			break;

		case MENU_CROSSOVER_TYPE:
			if(controls->butter)
				menuPrintLines("Cross Type:", "ButterW", controls);
			else
				menuPrintLines("Cross Type:", "LinkRly", controls);
			break;

		case MENU_EQ_ONOFF:
			if(controls->eq)
				menuPrintLines("EQ:", "ON", controls);
			else
				menuPrintLines("EQ:", "OFF", controls);
			break;

		case MENU_EQ_Q:
//			Q = get_Q(&filters[PARAM_EQ]);
			sprintf(output_str_value, "%d", (uint8_t)Q);
			menuPrintLines("EQ Q:", output_str_value, controls);
			break;

		case MENU_EQ_FREQUENCY:
//			f_eq = get_f0(&filters[PARAM_EQ]);
			sprintf(output_str_value, "%d", (uint16_t)f_eq);
			menuPrintLines("EQ f0:", output_str_value, controls);
			break;

		case MENU_EQ_GAIN:
//			G = get_G(&filters[PARAM_EQ]);
			sprintf(output_str_value, "%d", (int8_t)G);
			menuPrintLines("EQ G:", output_str_value, controls);
			break;

		case MENU_VOLUME_OUTPUT:
			sprintf(output_str_value, "%d", OUT_MAX_VOLUME);
			menuPrintLines("Volume:", output_str_value, controls);
			break;

		default: break;
	}
}

//***********************************************************************

void menuValueAdd(sys_controls_union *controls){

	controls->disp_enable = 1;

	// Circular increment
	if(controls->enter == 0){
		nav_count ++;

		if(nav_count == TOTAL_MENU_STATES)
			nav_count = 0;
	}
	else{
		switch(nav_count){

			case MENU_CROSSOVER_ONOFF:
				controls->cross = !controls->cross;
				break;

			case MENU_CROSSOVER_FREQUENCY:
				if(f0 < F0_MAX - F0_INC_RATE)
					f0 += F0_INC_RATE;
				break;

			case MENU_CROSSOVER_TYPE:
				controls->butter = !controls->butter;
				break;

			case MENU_EQ_ONOFF:
				controls->eq = !controls->eq;
				break;

			case MENU_EQ_FREQUENCY:
				if(f_eq< F0_MAX_EQ - F0_INC_RATE)
					f_eq += F0_INC_RATE;
				break;

			case MENU_EQ_Q:
				if(Q < Q_MAX)
					Q++;
				break;

			case MENU_EQ_GAIN:
				if(G < G_MAX)
					G++;
				break;

			default: break;
		}
	}
}

//***********************************************************************

void menuValueSub(sys_controls_union *controls){

	controls->disp_enable = 1;

	// Circular decrement
	if(controls->enter == 0){
		if(nav_count > 0)
			nav_count --;
		else
			nav_count = TOTAL_MENU_STATES - 1;
	}
	else{
		switch(nav_count){

			case MENU_CROSSOVER_ONOFF:
				controls->cross = !controls->cross;
				break;

			case MENU_CROSSOVER_FREQUENCY:
				if(f0 > F0_MIN + F0_INC_RATE)
					f0 -= F0_INC_RATE;
				break;

			case MENU_CROSSOVER_TYPE:
				controls->butter = !controls->butter;
				break;

			case MENU_EQ_ONOFF:
				controls->eq = !controls->eq;
				break;

			case MENU_EQ_FREQUENCY:
				if(f_eq > F0_MIN + F0_INC_RATE)
					f_eq -= F0_INC_RATE;
				break;

			case MENU_EQ_Q:
				if(Q > Q_MIN)
					Q--;
				break;

			case MENU_EQ_GAIN:
				if(G > G_MIN)
					G--;
				break;

			default: break;
		}
	}
}

//***********************************************************************

void menuValueEnter(sys_controls_union *controls){
	controls->enter = !controls->enter;
//	controls->disp_enable = 1;
}

//***********************************************************************

void menuPrintLines(char* firstLine, char* secondLine, sys_controls_union *controls){

	if(controls->disp_enable){

		ssd1306_SetCursor(0,0);
		ssd1306_Fill(Black);									// 12329 cycles
		ssd1306_WriteString(firstLine, Font_11x18, White);		// 20508
		ssd1306_SetCursor(4,30);
		ssd1306_WriteString(secondLine, Font_11x18, White);		// 63368  Font_16x26

		ssd1306_UpdateScreen_light((uint8_t)controls->add_i2c_index);		// colocar em controls			// 4231302

		controls->add_i2c_index++;

		if(controls->add_i2c_index==8){
			controls->add_i2c_index = 0;
			controls->disp_enable = 0;
		}
	}
}

