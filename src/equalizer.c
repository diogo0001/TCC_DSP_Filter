/*
 * equalizer.c
 *
 *  Created on: 28 de abr de 2020
 *      Author: Diogo Tavares
 */

#include "equalizer.h"

uint8_t eq_coef_calc(filter_instance* S){

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

	#ifdef TRACE_DEBUG
	trace_printf("\nEQ **************************");
	trace_printf("\nb0: %f \nb1: %f \nb2: %f \na1: %f \na2: %f",S->coefs[0],S->coefs[1],S->coefs[2],S->coefs[3],S->coefs[4]);
	trace_printf("\n*****************************");
	#endif


    return 0;
}

// ***********************************************************************

uint8_t eq_check_f0_variation(filter_instance* S, float32_t f0){

	if(f0 != S->f0){
		S->f0 = f0;
		eq_coef_calc(S);
		return 3;
	}
	return 0;
}

// ***********************************************************************

uint8_t eq_check_Q_variation(filter_instance* S, float32_t Q){

	if(Q!= S->Q){
		S->Q = Q;
		eq_coef_calc(S);
		return 4;
	}
	return 0;
}

// ***********************************************************************

uint8_t eq_check_G_variation(filter_instance* S, float32_t G){

	if(G != S->G){
		S->G = G;
		eq_coef_calc(S);
		return 5;
	}
	return 0;
}

// ***********************************************************************

float32_t f0_variator(vari_eq_instance *S_VAR, float32_t f0){

	S_VAR->time_count++;

	if(S_VAR->time_count > S_VAR->time_limit){
		S_VAR->time_count = 0;

		if(S_VAR->up_filter == 1){

			f0 += S_VAR->freq_step;  	// variator

			if(f0 > (S_VAR->freq_max - S_VAR->freq_step))
				S_VAR->up_filter = 0;
		}
		else{
			f0 -= S_VAR->freq_step;		// variator

			if(f0 < (S_VAR->freq_min + S_VAR->freq_step))
				S_VAR->up_filter = 1;
		}
	}

	return f0;
}

