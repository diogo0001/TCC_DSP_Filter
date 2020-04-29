/*
 * crossover.h
 *
 *  Created on: 10 de mar de 2020
 *      Author: kulie
 */

#ifndef CROSSOVER_H_
#define CROSSOVER_H_

#include <arm_math.h>
#include "defines.h"


typedef struct{
	float32_t f0,prev_f0;
	float32_t Q, prev_Q;
	float32_t *hp_coefs;		//[5*NUM_STAGES];
	float32_t *hp_state;		//[4*NUM_STAGES];
	float32_t *lp_coefs;		//[5*NUM_STAGES];
	float32_t *lp_state;		//[4*NUM_STAGES];
}crossover_instance;


void crossover_init(crossover_instance *S, float32_t *hp_coefs,float32_t *lp_coefs,float32_t *hp_state,float32_t *lp_state);
void crossover(float32_t * pSrc, float32_t * pDst, uint16_t blockSize);

uint8_t cross_bind_coef_calc(crossover_instance *S);
uint8_t cross_check_variation(crossover_instance* S);

void set_cross_f0(crossover_instance* S, float32_t f0);
void set_cross_Q(crossover_instance* S, float32_t Q);

#endif /* CROSSOVER_H_ */
