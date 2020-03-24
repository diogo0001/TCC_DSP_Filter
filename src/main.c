#include <stm32f4xx.h>
#include <arm_math.h>
#include <stm32f4_discovery.h>
#include <stm32f4_discovery_accelerometer.h>
#include <wolfson_pi_audio.h>
#include <diag/Trace.h>
#include <dwt.h>
#include <string.h>
#include "ssd1306.h"
#include "ssd1306_tests.h"

#include "fdacoefs_Q5_HP.h"

//#include "math_helper.h"

#define NUM_STAGES 1
#define BLOCK_SIZE (WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE)/4

#undef CYCLE_COUNTER
//#define CYCLE_COUNTER

I2C_HandleTypeDef hi2c1;
uint32_t AcceleroTicks;
int16_t AcceleroAxis[3];
int16_t TxBuffer[WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE];
int16_t RxBuffer[WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE];

__IO BUFFER_StateTypeDef buffer_offset = BUFFER_OFFSET_NONE;
__IO uint8_t Volume = 60;  // set volume to 60% to avoid floor noise

float32_t hertz2rad(float32_t fo);

typedef enum{
	FLOAT32,
	Q15
}filter_type;

filter_type FILTER_TYPE = FLOAT32;

typedef struct{
	float32_t f0,prev_f0;
	float32_t G, prev_G;
	float32_t Q, prev_Q;
	float32_t coefs[5*NUM_STAGES];
	float32_t eq_state[4*NUM_STAGES];
}param_eq_instance;


typedef struct{
	int time_count;
	int up_dw_cout;
	int time_limit;
	int freq_limit;
	float32_t freq_step;
	char up_filter;
}vari_eq_instance;

int variator(vari_eq_instance *S,param_eq_instance* S_EQ);

int eq_coef_calc(param_eq_instance* S);

int lp_coef_calc(param_eq_instance* S);

int hp_coef_calc(param_eq_instance* S);

int main(int argc, char* argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	uint32_t i, k;

	// Initialise the HAL Library; it must be the first
	// instruction to be executed in the main program.
	HAL_Init();

	DWT_Enable();

#ifdef OS_USE_SEMIHOSTING
#ifdef CYCLE_COUNTER
	FILE *CycleFile;
	uint32_t cycleCount;
	CycleFile = fopen("cyclecounter.txt", "w");
	if (!CycleFile) {
		trace_printf("Error trying to open cycle counter file\n.");
		while(1);
	}

	// DWT_Reset();

	// cycleCount = DWT_GetValue();

#endif
#endif

	WOLFSON_PI_AUDIO_Init((INPUT_DEVICE_LINE_IN << 8) | OUTPUT_DEVICE_BOTH, 80, AUDIO_FREQUENCY_48K);

	WOLFSON_PI_AUDIO_SetInputMode(INPUT_DEVICE_LINE_IN);

	WOLFSON_PI_AUDIO_SetMute(AUDIO_MUTE_ON);

	WOLFSON_PI_AUDIO_Play(TxBuffer, RxBuffer, WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE);

	WOLFSON_PI_AUDIO_SetVolume(Volume);

	BSP_ACCELERO_Init();


	// ----------------------- Float point 32 processing -------------------------------------------
	float32_t  *inputF32, *outputF32_H, *outputF32_L;
	float32_t inputF32Buffer[BLOCK_SIZE];
	float32_t outputF32Buffer_H[BLOCK_SIZE];
	float32_t outputF32Buffer_L[BLOCK_SIZE];

	inputF32 = &inputF32Buffer[0];
	outputF32_H = &outputF32Buffer_H[0];
	outputF32_L = &outputF32Buffer_L[0];

	param_eq_instance S_EQ;
	vari_eq_instance S_V;

	S_EQ.f0 = 100.0;
	S_EQ.G = 24.0;
	S_EQ.Q = 8.3;
	S_EQ.prev_f0 = S_EQ.f0;
	S_EQ.prev_G = S_EQ.G;
	S_EQ.prev_Q = S_EQ.Q ;

	S_V.up_filter = 1;
	S_V.time_count = 0;
	S_V.up_dw_cout = 0;
	S_V.time_limit = 1;
	S_V.freq_limit = 3000;
	S_V.freq_step = 20.0;

	eq_coef_calc(&S_EQ);

	trace_printf("Coefs: \n%f \n%f \n%f \n%f \n%f\n",S_EQ.coefs[0],S_EQ.coefs[1],S_EQ.coefs[2],S_EQ.coefs[3],S_EQ.coefs[4]);

	arm_biquad_casd_df1_inst_f32 S;
	arm_biquad_cascade_df1_init_f32(&S, NUM_STAGES,S_EQ.coefs,S_EQ.eq_state);


	//trace_printf("End of initialization.\n");

	char pass_through = 0;
	char up_filter = 1;
	int time_count = 0;
	int aux_cout = 0;
	const int time_limit = 1;
	const int freq_limit = 3000;
	float32_t freq_step = 20.0;

	while (1) {
		if(buffer_offset == BUFFER_OFFSET_HALF)
		{
			if(FILTER_TYPE==FLOAT32){

				for(i=0, k=0; i<(WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE/2); i++) {
					if(i%2) {
						inputF32Buffer[k] = (float32_t)(RxBuffer[i]/32768.0);//convert to float LEFT

						if(pass_through != 0){
							outputF32_H[k] = inputF32[k];
							outputF32_L[k] = inputF32[k];
						}
						k++;
					}

				}

				// Frequency variation automation --------
//				time_count++;
//				if(time_count>time_limit){
//					time_count = 0;
//
//					if(up_filter == 1){
//						aux_cout++;
//						if(S_EQ.f0 > freq_limit)
//							up_filter = 0;
//
//						S_EQ.f0 += freq_step;
//
//					}
//					else{
//						aux_cout--;
//						if(S_EQ.f0<100)
//							up_filter = 1;
//
//						S_EQ.f0 -= freq_step;
//					}
//				}
//
				variator(&S_V,&S_EQ);

				if(S_EQ.prev_f0 != S_EQ.f0){
					S_EQ.prev_f0 = S_EQ.f0;
					eq_coef_calc(&S_EQ);
				}

				//---------------------------------------


				if(pass_through == 0){
					arm_biquad_cascade_df1_f32(&S, inputF32, outputF32_H, BLOCK_SIZE);
					arm_biquad_cascade_df1_f32(&S, inputF32, outputF32_L, BLOCK_SIZE);
				}



				for(i=0, k=0; i<(WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE/2); i++) {
					if(i%2)	{
						TxBuffer[i] = (int16_t)(outputF32Buffer_H[k]*32768);//back to 1.15
						k++;
					}
					else {
						TxBuffer[i] = (int16_t)(outputF32Buffer_L[k]*32768);//back to 1.15

					}
				}
			}


#ifdef CYCLE_COUNTER
			fprintf(CycleFile, "\nHALF: %lu", (DWT_GetValue()- cycleCount));
#endif

			buffer_offset = BUFFER_OFFSET_NONE;

		}

		if(buffer_offset == BUFFER_OFFSET_FULL)
		{
			DWT_Reset();
			//cycleCount = DWT_GetValue();

			if(FILTER_TYPE==FLOAT32){
				for(i=(WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE/2), k=0; i<WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE; i++) {
					if(i%2) {
						inputF32Buffer[k] = (float32_t)(RxBuffer[i]/32768.0);//convert to float

						if(pass_through != 0){
							outputF32_H[k] = inputF32[k];
							outputF32_L[k] = inputF32[k];
						}
						k++;
					}
				}

				// Frequency variation automation --------
//				time_count++;
//				if(time_count>time_limit){
//					time_count = 0;
//
//					if(up_filter == 1){
//						aux_cout++;
//						if(S_EQ.f0 > freq_limit)
//							up_filter = 0;
//
//						S_EQ.f0 += freq_step;
//
//					}
//					else{
//						aux_cout--;
//						if(S_EQ.f0<100)
//							up_filter = 1;
//
//						S_EQ.f0 -= freq_step;
//					}
//				}
				variator(&S_V,&S_EQ);

				if(S_EQ.prev_f0 != S_EQ.f0){
					S_EQ.prev_f0 = S_EQ.f0;
					eq_coef_calc(&S_EQ);
				}
				//---------------------------------------


				if(pass_through == 0){
					arm_biquad_cascade_df1_f32(&S, inputF32, outputF32_H, BLOCK_SIZE);
					arm_biquad_cascade_df1_f32(&S, inputF32, outputF32_L, BLOCK_SIZE);
				}

				//crossover(inputF32, outputF32, BLOCK_SIZE);



				for(i=(WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE/2), k=0; i<WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE; i++) {
					if(i%2)	{
						TxBuffer[i] = (int16_t)(outputF32Buffer_H[k]*32768);//back to 1.15
						k++;
					}
					else {
						TxBuffer[i] = (int16_t)(outputF32Buffer_L[k]*32768);//back to 1.15
					}
				}
			}



#ifdef CYCLE_COUNTER
			fprintf(CycleFile, "\nFULL: %lu", (DWT_GetValue()- cycleCount));
#endif

			buffer_offset = BUFFER_OFFSET_NONE;
		}
		//TEST_Main();
	}
	return 0;
}


/*------------------------------------------------------
Callbacks implementation:
--------------------------------------------------------*/

/**
  * @brief  Manages the DMA full Transfer complete event.
  */
void WOLFSON_PI_AUDIO_TransferComplete_CallBack(void)
{
	buffer_offset = BUFFER_OFFSET_FULL;
}

/**
  * @brief  Manages the DMA Half Transfer complete event.
  */
void WOLFSON_PI_AUDIO_HalfTransfer_CallBack(void)
{
	  buffer_offset = BUFFER_OFFSET_HALF;
}

/**
  * @brief  Manages the DMA FIFO error interrupt.
  * @param  None
  * @retval None
  */
void WOLFSON_PI_AUDIO_OUT_Error_CallBack(void)
{
  /* Stop the program with an infinite loop */
  while (1);
}

float32_t hertz2rad(float32_t fo){

//	float32_t f = fo / AUDIO_FREQUENCY_48K;

	return  2 * PI *fo / AUDIO_FREQUENCY_48K;
}

int eq_coef_calc(param_eq_instance* S){

	float32_t a,b,B,K,w0;

	B = S->f0/S->Q;
	K = pow(10.0,(S->G/20));
	w0 = PI*B/AUDIO_FREQUENCY_48K;
	a = arm_sin_f32(w0)/arm_cos_f32(w0); // tg
	b = -arm_cos_f32(2*PI*(S->f0/AUDIO_FREQUENCY_48K));
	a = (1 - a)/(1 + a);

    S->coefs[0] = (1+a+K-K*a)*0.5;		// b0
    S->coefs[1] = (b+b*a);				// b1
    S->coefs[2] = (1+a-K+K*a)*0.5;		// b2
    S->coefs[3] = -S->coefs[1]; 			// a1
    S->coefs[4] = -a;					// a2


//    S->coefs[0] = 1;		// b0
//	S->coefs[1] = 1;				// b1
//	S->coefs[2] = 1;		// b2
//	S->coefs[3] = -1; 			// a1
//	S->coefs[4] = -1;					// a2

    return 0;

}

int lp_coef_calc(param_eq_instance* S){

	float32_t w0 =   2 * PI *S->f0 / AUDIO_FREQUENCY_48K;  //2 * Pi * f0 / Fs;
	float32_t alpha = arm_sin_f32(w0) / (2 * S->Q);


	S->coefs[0] = (1 - arm_cos_f32(w0)) / 2; 	// b0
	S->coefs[1] = (1 - arm_cos_f32(w0));		// b1
	S->coefs[2] = (1 - arm_cos_f32(w0)) / 2;	// b2
	S->coefs[3] = -(-2 * arm_cos_f32(w0));		// a1
	S->coefs[4] = -(1 - alpha);					// a2

	return 0;
}

int hp_coef_calc(param_eq_instance* S){

	float32_t w0 =  hertz2rad(S->f0);  //2 * Pi * f0 / Fs;
	float32_t alpha = arm_sin_f32(w0) / (2 * S->Q);

	S->coefs[0] = (1 + arm_cos_f32(w0)) / 2; 	// b0
	S->coefs[1] = -(1 + arm_cos_f32(w0));		// b1
	S->coefs[2] = (1 + arm_cos_f32(w0)) / 2;	// b2
	S->coefs[3] = -(-2 * arm_cos_f32(w0));		// a1
	S->coefs[4] = -(1 - alpha);					// a2

	return 0;
}

int variator(vari_eq_instance *S, param_eq_instance* S_EQ){

	S->time_count++;
	if(S->time_count>S->time_limit){
		S->time_count = 0;

		if(S->up_filter == 1){
			S->up_dw_cout++;
			if(S_EQ->f0 > S->freq_limit)
				S->up_filter = 0;

			S_EQ->f0 += S->freq_step;

		}
		else{
			S->up_dw_cout--;
			if(S_EQ->f0<100)
				S->up_filter = 1;

			S_EQ->f0 -= S->freq_step;
		}
	}
	return 0;
}
