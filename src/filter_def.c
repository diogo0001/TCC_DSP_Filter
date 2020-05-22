/*
 * filter_def.c
 *
 *  Created on: 29 de abr de 2020
 *      Author: Diogo Tavares
 */

#include "filter_def.h"


void filter_init(filter_instance *S, float32_t *coefs, float32_t *state){
	S->coefs = &coefs[0];
	S->state = &state[0];
	S->f0 = F0_DEFAULT;
	S->Q = Q_DEFAULT;
	S->G = G_DEFAULT;
}

void set_f0(filter_instance* S, uint16_t f0){
	S->f0 = f0;
}

void set_Q(filter_instance* S, float32_t Q){
	S->Q = Q;
}

void set_G(filter_instance* S, int8_t G){
	S->G = G;
}

uint16_t get_f0(filter_instance* S){
	return S->f0;
}

float32_t get_Q(filter_instance* S){
	return S->Q;
}

int8_t get_G(filter_instance* S){
	return S->G;
}
