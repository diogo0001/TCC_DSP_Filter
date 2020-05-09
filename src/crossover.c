#include "crossover.h"


//***********************************************************************

uint8_t cross_check_f0_variation(filter_instance* SL, filter_instance *SH, float32_t f0){

	if(f0 != SL->f0){
		SL->f0 = f0;
		SH->f0 = f0;
		cross_bind_coef_calc(SL, SH);
		return 1;
	}
	return 0;
}
//***********************************************************************

uint8_t cross_check_Q_variation(filter_instance* SL, filter_instance *SH, float32_t Q){
	if(Q != SL->Q){
		SL->Q = Q;
		SH->Q = Q;
		cross_bind_coef_calc(SL, SH);
		return 1;
	}
	return 0;
}

//***********************************************************************

uint8_t cross_bind_coef_calc(filter_instance *SL, filter_instance *SH){

	if(SL->Q==0) SL->Q = 0.001;

	float32_t w0 = 2.0*PI*SL->f0 / AUDIO_FREQUENCY_48K;  //2 * Pi * f0 / Fs;
	float32_t cos_w0 = arm_cos_f32(w0);
	float32_t alpha = arm_sin_f32(w0) / (2.0 * SL->Q);
	float32_t a0 = 1.0 + alpha;
	a0 = 1/a0;									// normaliza

	SL->coefs[0] = ((1.0 - cos_w0) / 2)*a0; 	// b0
	SL->coefs[1] = (1.0 - cos_w0)*a0;			//(1 - arm_cos_f32(w0));		// b1
	SL->coefs[2] = SL->coefs[0];				//(1 - arm_cos_f32(w0)) / 2;	// b2
	SL->coefs[3] = -(-2.0 * cos_w0)*a0;			// -a1
	SL->coefs[4] = -(1.0 - alpha)*a0;

	SH->coefs[0] = -((1 + cos_w0) / 2)*a0; 		// b0
	SH->coefs[1] = (1 + cos_w0)*a0;				//(1 + arm_cos_f32(w0));		// b1
	SH->coefs[2] = SH->coefs[0];				//(1 + arm_cos_f32(w0)) / 2;	// b2
	SH->coefs[3] = SL->coefs[3];				//-(-2 * arm_cos_f32(w0))*a0;	// -a1
	SH->coefs[4] = SL->coefs[4];				//-(1 - alpha)*a0;


	SL->coefs[5] = SL->coefs[0];
	SL->coefs[6] = SL->coefs[1];
	SL->coefs[7] = SL->coefs[2];
	SL->coefs[8] = SL->coefs[3];
	SL->coefs[9] = SL->coefs[4];

	SH->coefs[5] = SH->coefs[0];
	SH->coefs[6] = SH->coefs[1];
	SH->coefs[7] = SH->coefs[2];
	SH->coefs[8] = SH->coefs[3];
	SH->coefs[9] = SH->coefs[4];


	#ifdef TRACE_DEBUG
	trace_printf("\nLP **************************");
	trace_printf("\nb0: %f \nb1: %f \nb2: %f \na1: %f \na2: %f",SL->coefs[0],SL->coefs[1],SL->coefs[2],SL->coefs[3],SL->coefs[4]);
	trace_printf("\nHP **************************");
	trace_printf("\nb0: %f \nb1: %f \nb2: %f \na1: %f \na2: %f",SH->coefs[0],SH->coefs[1],SH->coefs[2],SH->coefs[3],SH->coefs[4]);
	trace_printf("\n*****************************");
	#endif

	return 0;
}
//***********************************************************************

uint8_t cross_copycat_coefs(float32_t *src){

	uint8_t i;
	uint8_t half = (NUM_STAGES_2*5)/2;

	for(i=0;i<half;i++){
		src[i+half] = src[i];
	}
	return 0;
}
