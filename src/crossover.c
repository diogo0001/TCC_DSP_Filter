#include "crossover.h"


void crossover_init(crossover_instance *S, float32_t *hp_coefs,float32_t *lp_coefs,float32_t *hp_state,float32_t *lp_state);
	S->hp_coefs = hp_coefs;
	S->hp_state = hp_state;
	S->lp_coefs = lp_coefs;
	S->lp_state = lp_state;
	S->f0 = F0_DEFAULT;
	S->prev_f0 = F0_DEFAULT;
	S->Q = Q_DEFAULT;
	S->prev_Q = Q_DEFAULT;
}
//***********************************************************************

void crossover(float32_t * pSrc, float32_t * pDst, uint16_t blockSize){

}

//***********************************************************************

uint8_t cross_check_variation(crossover_instance* S){

	if(S->prev_f0 != S->f0){
		S->prev_f0 = S->f0;
		return 1;
	}
	else if(S->prev_Q != S->Q){
		S->prev_Q = S->Q;
		return 2;
	}

	return 0;
}
//***********************************************************************

uint8_t cross_bind_coef_calc(crossover_instance* S){

	if(S->Q==0) S->Q = 0.001;

	float32_t w0 = 2.0*PI*S->f0 / AUDIO_FREQUENCY_48K;  //2 * Pi * f0 / Fs;
	float32_t cos_w0 = arm_cos_f32(w0);
	float32_t alpha = arm_sin_f32(w0) / (2.0 * S->Q);
	float32_t a0 = 1.0 + alpha;
	a0 = 1/a0;										// normaliza

	S->lp_coefs[0] = ((1.0 - cos_w0) / 2)*a0; 		// b0
	S->lp_coefs[1] = (1.0 - cos_w0)*a0;				//(1 - arm_cos_f32(w0));		// b1
	S->lp_coefs[2] = S->lp_coefs[0];				//(1 - arm_cos_f32(w0)) / 2;	// b2
	S->lp_coefs[3] = -(-2.0 * cos_w0)*a0;			// -a1
	S->lp_coefs[4] = -(1.0 - alpha)*a0;


	S->hp_coefs[0] = -((1 + cos_w0) / 2)*a0; 		// b0
	S->hp_coefs[1] = (1 + cos_w0)*a0;				//(1 + arm_cos_f32(w0));		// b1
	S->hp_coefs[2] = S->hp_coefs[0];				//(1 + arm_cos_f32(w0)) / 2;	// b2
	S->hp_coefs[3] = S->lp_coefs[3];				//-(-2 * arm_cos_f32(w0))*a0;	// -a1
	S->hp_coefs[4] = S->lp_coefs[4];				//-(1 - alpha)*a0;


	#ifdef TRACE_DEBUG
	trace_printf("\nLP **************************");
	trace_printf("\nW0: %f\nalpha: %f\n\n",w0,alpha);
	trace_printf("Coefs CR: \n%f \n%f \n%f \n%f \n%f",S->coefs[0],S->coefs[1],S->coefs[2],S->coefs[3],S->coefs[4]);
	trace_printf("\nHP **************************");
	trace_printf("\nW0: %f\nalpha: %f\n\n",w0,alpha);
	trace_printf("Coefs CR: \n%f \n%f \n%f \n%f \n%f",S->coefs[0],S->coefs[1],S->coefs[2],S->coefs[3],S->coefs[4]);
	trace_printf("\n*****************************");
	#endif

	return 0;
}
//***********************************************************************

void set_cross_f0(crossover_instance* S, float32_t f0){
	S->f0 = f0;
}
//***********************************************************************

void set_cross_Q(crossover_instance* S, float32_t Q){
	S->Q = Q;
}
