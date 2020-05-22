/*
 * filter_def.h
 *
 *  Created on: 29 de abr de 2020
 *      Author: Diogo Tavares
 */

#ifndef FILTER_DEF_H_
#define FILTER_DEF_H_

#include <defines_sys.h>

typedef struct{
	float32_t *coefs;
	float32_t *state;
	float32_t Q;
	uint16_t f0;
	int8_t G;
}filter_instance;


typedef struct{
	uint32_t time_count;
	uint32_t up_dw_cout;
	uint32_t time_limit;
	uint32_t freq_max;
	uint32_t freq_min;
	float32_t freq_step;
	uint8_t up_filter;
}vari_eq_instance;

uint16_t f0_variator(vari_eq_instance *S_VAR, uint16_t f0);

void filter_init(filter_instance *S, float32_t *coefs, float32_t *state);
void set_f0(filter_instance* S, uint16_t f0);
void set_Q(filter_instance* S, float32_t Q);
void set_G(filter_instance* S, int8_t G);

uint16_t get_f0(filter_instance* S);
float32_t get_Q(filter_instance* S);
int8_t get_G(filter_instance* S);

#endif /* FILTER_DEF_H_ */
