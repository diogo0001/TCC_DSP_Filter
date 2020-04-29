/*
 * equalizer.c
 *
 *  Created on: 28 de abr de 2020
 *      Author: kulie
 */

#include "equalizer.h"

void param_eq_init(param_eq_instance *S, float32_t *coefs,float32_t *eq_state){
	S->coefs = coefs;
	S->eq_state = eq_state;
	S->f0 = F0_DEFAULT;
	S->prev_f0 = F0_DEFAULT;
	S->G = G_DEFAULT;
	S->prev_G = G_DEFAULT;
	S->Q = Q_DEFAULT;
	S->prev_Q = Q_DEFAULT;
}
//***********************************************************************

void param_eq(float32_t * pSrc, float32_t * pDst, uint16_t blockSize){

}
//***********************************************************************

uint8_t eq_coef_calc(param_eq_instance* S){

	float32_t a,b,B,K,w0,w;

	B = S->f0/S->Q;
	K = pow(10.0,(S->G/20));
	w0 = 2*PI*S->f0/AUDIO_FREQUENCY_48K;
	w = PI*B/AUDIO_FREQUENCY_48K;
	a = arm_sin_f32(w)/arm_cos_f32(w); // tg
	a = (1 - a)/(1 + a);
	b = -arm_cos_f32(w0);

    S->coefs[0] = (1+a+K-K*a)*0.5;		// b0
    S->coefs[1] = (b+b*a);				// b1
    S->coefs[2] = (1+a-K+K*a)*0.5;		// b2
    S->coefs[3] = -S->coefs[1]; 		// -a1
    S->coefs[4] = -a;					// -a2

    return 0;
}
//***********************************************************************

uint8_t eq_check_variation(param_eq_instance* S){

	if(S->prev_f0 != S->f0){
		S->prev_f0 = S->f0;
		return 1;
	}
	else if(S->prev_G != S->G){
		S->prev_G = S->G;
		return 2;
	}
	else if(S->prev_Q != S->Q){
		S->prev_Q = S->Q;
		return 3;
	}
	return 0;
}
//***********************************************************************

uint8_t variator(vari_eq_instance *S, param_eq_instance* S_EQ){

	S->time_count++;

	if(S->time_count > S->time_limit){
		S->time_count = 0;

		if(S->up_filter == 1){

			S_EQ->f0 += S->freq_step;

			if(S_EQ->f0 > (S->freq_max - S->freq_step))
				S->up_filter = 0;

		}
		else{
			S_EQ->f0 -= S->freq_step;

			if(S_EQ->f0 < (S->freq_min + S->freq_step))
				S->up_filter = 1;
		}
	}
	if(eq_check_variation(S_EQ))
		eq_coef_calc(S_EQ);


	return 0;
}

//***********************************************************************
void set_eq_f0(param_eq_instance* S, float32_t f0){
	S->f0 = f0;
}
//***********************************************************************

void set_eq_Q(param_eq_instance* S, float32_t Q){
	S->Q = Q;
}
//***********************************************************************

void set_eq_G(param_eq_instance* S, float32_t G){
	S->G = G;
}
