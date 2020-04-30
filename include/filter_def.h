/*
 * filter_def.h
 *
 *  Created on: 29 de abr de 2020
 *      Author: kulie
 */

#ifndef FILTER_DEF_H_
#define FILTER_DEF_H_

#include "defines.h"

typedef struct{
	float32_t f0,prev_f0;
	float32_t G, prev_G;
	float32_t Q, prev_Q;
	float32_t *coefs;		//[5*NUM_STAGES];
	float32_t *state;		//[4*NUM_STAGES];
}filter_instance;


#ifdef TEST
typedef struct{
	uint32_t time_count;
	uint32_t up_dw_cout;
	uint32_t time_limit;
	uint32_t freq_max;
	uint32_t freq_min;
	float32_t freq_step;
	uint8_t up_filter;
}vari_eq_instance;
#endif

uint8_t check_variation(filter_instance* S);
uint8_t variator(vari_eq_instance *S,filter_instance* S_EQ);

void set_f0(filter_instance* S, float32_t f0);
void set_Q(filter_instance* S, float32_t Q);
void set_G(filter_instance* S, float32_t G);

#endif /* FILTER_DEF_H_ */
