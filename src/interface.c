/*
 * interface.c
 *
 *  Created on: 28 de abr de 2020
 *      Author: kulie
 */

#include "interface.h"

menu_state_enum menu_state = MENU_HOME;
crossover_type_enum cross_type = BUTTER;
filter_type_enum filter_type = EQ;
uint8_t enter = 0;


//***********************************************************************

void interface_init(coefs_buffers_instance *buffers, filter_instance *filters, arm_biquad_casd_df1_inst_f32 *biquads){
	filter_init(&filters[PARAM_EQ], buffers->eq_coefs, buffers->eq_state);
	filter_init(&filters[CROSS_LP], buffers->lp_coefs, buffers->lp_state);
	filter_init(&filters[CROSS_HP], buffers->hp_coefs, buffers->hp_state);

	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_EQ], NUM_STAGES,filters[PARAM_EQ].coefs, filters[PARAM_EQ].state);

	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_BW_LP], NUM_STAGES, filters[CROSS_LP].coefs, filters[CROSS_LP].state);
	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_BW_HP], NUM_STAGES, filters[CROSS_HP].coefs, filters[CROSS_HP].state);

//	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_LR_LP], NUM_STAGES_2, filters[CROSS_LP].coefs, filters[CROSS_LP].state);
//	arm_biquad_cascade_df1_init_f32(&biquads[BIQ_LR_HP], NUM_STAGES_2 ,filters[CROSS_HP].coefs, filters[CROSS_HP].state);
}
//***********************************************************************

void interface(float32_t **io, arm_biquad_casd_df1_inst_f32 *biquads){

	arm_biquad_cascade_df1_f32(&biquads[BIQ_EQ], io[INPUT_BUFFER], io[OUTPUT_BUFFER_TEMP], BLOCK_SIZE);

	if(cross_type == BUTTER){
		arm_biquad_cascade_df1_f32(&biquads[BIQ_BW_LP], io[OUTPUT_BUFFER_TEMP], io[OUTPUT_BUFFER_L], BLOCK_SIZE);
		arm_biquad_cascade_df1_f32(&biquads[BIQ_BW_HP], io[OUTPUT_BUFFER_TEMP], io[OUTPUT_BUFFER_H], BLOCK_SIZE);
	}else{
		arm_biquad_cascade_df1_f32(&biquads[BIQ_LR_LP], io[OUTPUT_BUFFER_TEMP], io[OUTPUT_BUFFER_L], BLOCK_SIZE);
		arm_biquad_cascade_df1_f32(&biquads[BIQ_LR_HP], io[OUTPUT_BUFFER_TEMP], io[OUTPUT_BUFFER_H], BLOCK_SIZE);
	}

}

//***********************************************************************

void updateScreen(void){

}

//***********************************************************************

void menuValueAdd(void){
//	Value++;
}

//***********************************************************************

void menuValueSub(void){
//	if(Value != 0)
//		Value--;
}

//***********************************************************************

void menuValueEnter(void){

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

/* interface(float32_t * pSrc, float32_t * pDst, uint16_t blockSize){
	menu_strings_instance strings; // declarar as strings aqui
	.
	.
	.

}
*/
