/*
 * interface.c
 *
 *  Created on: 28 de abr de 2020
 *      Author: kulie
 */

#include "interface.h"

arm_biquad_casd_df1_inst_f32 S_BIQ_E;
arm_biquad_casd_df1_inst_f32 S_BIQ_BW_L;
arm_biquad_casd_df1_inst_f32 S_BIQ_BW_H;
arm_biquad_casd_df1_inst_f32 S_BIQ_LR_L;
arm_biquad_casd_df1_inst_f32 S_BIQ_LR_H;

param_eq_instance S_EQ;
crossover_instance S_CROSSOVER_BW;
crossover_instance S_CROSSOVER_LR;

uint32_t Value;
char ValueStr[10];

menu_state_enum menu_state = MENU_HOME;
filter_type_enum filter_type = NONE;
uint8_t enter = 0;


//***********************************************************************

void interface_init(buffers_instance *S){
    param_eq_init(&S_EQ, S->eq_coefs, S->eq_state);
	crossover_init(&S_CROSSOVER_BW, S->butt_hp_coefs, S->butt_lp_coefs, S->butt_hp_state, S->butt_lp_state);
	crossover_init(&S_CROSSOVER_LR, S->link_hp_coefs, S->link_lp_coefs, S->link_hp_state, S->link_lp_state);

	arm_biquad_cascade_df1_init_f32(&S_BIQ_E, NUM_STAGES,S_EQ.coefs,S_EQ.state);

	arm_biquad_cascade_df1_init_f32(&S_BIQ_BW_L, NUM_STAGES,S_CROSSOVER_BW.lp_coefs,S_CROSSOVER_BW.lp_state);
	arm_biquad_cascade_df1_init_f32(&S_BIQ_BW_H, NUM_STAGES,S_CROSSOVER_BW.hp_coefs,S_CROSSOVER_BW.hp_state);

	arm_biquad_cascade_df1_init_f32(&S_BIQ_LR_L, NUM_STAGES,S_CROSSOVER_LR.lp_coefs,S_CROSSOVER_LR.lp_state);
	arm_biquad_cascade_df1_init_f32(&S_BIQ_LR_H, NUM_STAGES,S_CROSSOVER_LR.hp_coefs,S_CROSSOVER_LR.hp_state);
}
//***********************************************************************

void interface(float32_t * pSrc, float32_t * pDst, uint16_t blockSize){

//	switch(filter_type){
//
//		case EQ:
//			effect_echo(&S_EQ, pSrc, pDst, blockSize);
//			break;
//
//		case CROSSOVER:
//			effect_vibrato(&S_CROSSOVER, pSrc, pDst, blockSize);
//			break;
//
//		case NONE:
//			effects_pass_through(pSrc, pDst, blockSize);
//			break;
//	}
}

//***********************************************************************

void updateScreen(void){

}

//***********************************************************************

void menuValueAdd(void){
	Value++;
}

//***********************************************************************

void menuValueSub(void){
	if(Value != 0)
		Value--;
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


