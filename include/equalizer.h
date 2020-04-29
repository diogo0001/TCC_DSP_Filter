/*
 * equalizer.h
 *
 *  Created on: 28 de abr de 2020
 *      Author: kulie
 */

#ifndef EQUALIZER_H_
#define EQUALIZER_H_

#include "defines.h"

typedef struct{
	float32_t f0,prev_f0;
	float32_t G, prev_G;
	float32_t Q, prev_Q;
	float32_t *coefs;			//[5*NUM_STAGES];
	float32_t *eq_state;		//[4*NUM_STAGES];
}param_eq_instance;


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


void param_eq_init(param_eq_instance *S, float32_t *coefs,float32_t *eq_state);
void param_eq(float32_t * pSrc, float32_t * pDst, uint16_t blockSize);

uint8_t eq_coef_calc(param_eq_instance* S);
uint8_t eq_check_variation(param_eq_instance* S);
uint8_t variator(vari_eq_instance *S,param_eq_instance* S_EQ);

void set_eq_f0(param_eq_instance* S, float32_t f0);
void set_eq_Q(param_eq_instance* S, float32_t Q);
void set_eq_G(param_eq_instance* S, float32_t G);

#endif /* EQUALIZER_H_ */
