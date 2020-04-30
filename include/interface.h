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


typedef enum {
	NONE,
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

typedef struct{
	char eqStr[] 		= "EQ";
	char crossoverStr[] = "CROSSOVER";
	char backStr[]		= "Back";
	char gainStr[] 		= "Gain";
	char frequencyStr[]	= "Frequency";
	char qStr[]			= "Q";
	char butterStr[] 	= "Butter";
	char linkwitzStr[]	= "Linkwitz";
	char dbStr[]		= "dB";
	char hzStr[]		= "Hz";
}menu_strings_instance;


typedef struct{
	float32_t *inputF32;
	float32_t *outputF32_H;
	float32_t *outputF32_L;
	float32_t *temp;
} in_out_instance;

typedef struct{
	float32_t eq_coefs[5];
	float32_t eq_state[4];

	float32_t butt_hp_coefs[5*NUM_STAGES];
	float32_t butt_hp_state[4*NUM_STAGES];
	float32_t butt_lp_coefs[5*NUM_STAGES];
	float32_t butt_lp_state[4*NUM_STAGES];

	float32_t link_hp_coefs[5*NUM_STAGES_2];
	float32_t link_hp_state[4*NUM_STAGES_2];
	float32_t link_lp_coefs[5*NUM_STAGES_2];
	float32_t link_lp_state[4*NUM_STAGES_2];
}buffers_instance;


void interface_init(buffers_instance *S);
void interface(float32_t * pSrc, float32_t * pDst, uint16_t blockSize);
void updateScreen(void);
void menuValueAdd(void);
void menuValueSub(void);
void menuValueEnter(void);
void menuPrintLines(char* firstLine, char* secondLine, char* unity);


#endif /* INTERFACE_H_ */
