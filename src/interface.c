/*
 * interface.c
 *
 *  Created on: 28 de abr de 2020
 *      Author: kulie
 */


#include "interface.h"

arm_biquad_casd_df1_inst_f32 S_E,S_L,S_H;
param_eq_instance S_EQ;
crossover_instance S_CROSSOVER;

uint32_t Value;
char ValueStr[10];

menu_state_enum menu_state = MENU_HOME;
filter_type_enum filter_type = NONE;
uint8_t enter = 0;

static float32_t eq_coefs[5*NUM_STAGES];
static float32_t eq_state[4*NUM_STAGES];

static float32_t hp_coefs[5*NUM_STAGES];
static float32_t hp_state[4*NUM_STAGES];
static float32_t lp_coefs[5*NUM_STAGES];
static float32_t lp_state[4*NUM_STAGES];

void crossover_init(crossover_instance *S, float32_t *hp_coefs,float32_t *lp_coefs,float32_t *hp_state,float32_t *lp_state);


void interface_init(void){
    param_eq_init(&S_EQ, eq_coefs,eq_state);
	crossover_init(&S_CR, float32_t *coefs,float32_t *eq_state);
}
//***********************************************************************

void interface(float32_t * pSrc, float32_t * pDst, uint16_t blockSize){

	switch(filter_type){

		case EQ:
			effect_echo(&S_EQ, pSrc, pDst, blockSize);
			break;

		case CROSSOVER:
			effect_vibrato(&S_CROSSOVER, pSrc, pDst, blockSize);
			break;

		case NONE:
			effects_pass_through(pSrc, pDst, blockSize);
			break;
	}
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
