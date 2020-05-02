/*
 * filter_def.c
 *
 *  Created on: 29 de abr de 2020
 *      Author: kulie
 */

#include "filter_def.h"


void filter_init(filter_instance *S, float32_t *coefs, float32_t *state){
	S->coefs = &coefs[0];
	S->state = &state[0];
	S->f0 = F0_DEFAULT;
	S->Q = Q_DEFAULT;
	S->G = G_DEFAULT;
}

void set_f0(filter_instance* S, float32_t f0){
	S->f0 = f0;
}

void set_Q(filter_instance* S, float32_t Q){
	S->Q = Q;
}

void set_G(filter_instance* S, float32_t G){
	S->G = G;
}
