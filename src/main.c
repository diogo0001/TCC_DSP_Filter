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

//#define TRACE_DEBUG

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
__IO uint8_t Volume = 80;  // set volume to 60% to avoid floor noise

//*****************************************************************************************

typedef enum{
	EQ,
	CROSSOVER
}vari_mode;

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
	uint32_t time_count;
	uint32_t up_dw_cout;
	uint32_t time_limit;
	uint32_t freq_max;
	uint32_t freq_min;
	float32_t freq_step;
	uint8_t up_filter;
}vari_eq_instance;

//*****************************************************************************************

int variator(vari_eq_instance *S,param_eq_instance* S_EQ, vari_mode mode);

int eq_coef_calc(param_eq_instance* S);

int cross_bind_coef_calc(param_eq_instance* SL, param_eq_instance* SH);

int check_variation(param_eq_instance* S);


//*****************************************************************************************
int main(int argc, char* argv[])
{
	UNUSED(argc);
	UNUSED(argv);

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
	float32_t  *inputF32, *outputF32_H, *outputF32_L, *tempOut;
	float32_t inputF32Buffer[BLOCK_SIZE];
	float32_t outputF32Buffer_H[BLOCK_SIZE];
	float32_t outputF32Buffer_L[BLOCK_SIZE];
	float32_t tempF32Buffer[BLOCK_SIZE];

	inputF32 = &inputF32Buffer[0];
	outputF32_H = &outputF32Buffer_H[0];
	outputF32_L = &outputF32Buffer_L[0];
	tempOut = &tempF32Buffer[0];

	param_eq_instance S_EQ, S_LP, S_HP;
	vari_eq_instance S_V,S_C;

	// Crossover -------------------------------
	float32_t cross_Fc = 500.0;
	float32_t cross_G = 1;
	float32_t cross_Q = 0.7; // cross_Q > 5

	S_LP.f0 = cross_Fc;
	S_LP.G = cross_G;
	S_LP.Q = cross_Q;
	S_LP.prev_f0 = S_LP.f0;
	S_LP.prev_G = S_LP.G;
	S_LP.prev_Q = S_LP.Q;

	S_HP.f0 = cross_Fc;
	S_HP.G = cross_G;
	S_HP.Q = cross_Q;
	S_HP.prev_f0 = S_HP.f0;
	S_HP.prev_G = S_HP.G;
	S_HP.prev_Q = S_HP.Q;

	// Param EQ --------------------------------
	S_EQ.f0 = 1000.0;
	S_EQ.G = 24;
	S_EQ.Q = 15;
	S_EQ.prev_f0 = S_EQ.f0;
	S_EQ.prev_G = S_EQ.G;
	S_EQ.prev_Q = S_EQ.Q;

	// Variators -------------------------------
	S_V.up_filter = 1;
	S_V.time_count = 0;
	S_V.up_dw_cout = 0;
	S_V.time_limit = 1;
	S_V.freq_max = 1200;
	S_V.freq_min = S_EQ.f0;
	S_V.freq_step = 20.0;

	S_C.up_filter = 0;
	S_C.time_count = 0;
	S_C.up_dw_cout = 0;
	S_C.time_limit = 5;
	S_C.freq_max = 1000;
	S_V.freq_min = cross_Fc;
	S_C.freq_step = 20.0;

	eq_coef_calc(&S_EQ);
	cross_bind_coef_calc(&S_LP,&S_HP);

	trace_printf("\nCoefs LP: \n%f \n%f \n%f \n%f \n%f\n",S_LP.coefs[0],S_LP.coefs[1],S_LP.coefs[2],S_LP.coefs[3],S_LP.coefs[4]);
	trace_printf("\nCoefs HP: \n%f \n%f \n%f \n%f \n%f\n",S_HP.coefs[0],S_HP.coefs[1],S_HP.coefs[2],S_HP.coefs[3],S_HP.coefs[4]);

	trace_printf("\nCoefs EQ: \n%f \n%f \n%f \n%f \n%f\n",S_EQ.coefs[0],S_EQ.coefs[1],S_EQ.coefs[2],S_EQ.coefs[3],S_EQ.coefs[4]);

	arm_biquad_casd_df1_inst_f32 S,S_L,S_H;

	arm_biquad_cascade_df1_init_f32(&S, NUM_STAGES,S_EQ.coefs,S_EQ.eq_state);
	arm_biquad_cascade_df1_init_f32(&S_L, NUM_STAGES,S_LP.coefs,S_LP.eq_state);
	arm_biquad_cascade_df1_init_f32(&S_H, NUM_STAGES,S_HP.coefs,S_HP.eq_state);


	//trace_printf("End of initialization.\n");

	uint8_t pass_through = 0;

	uint32_t i, k;

	while (1) {
		if(buffer_offset == BUFFER_OFFSET_HALF)
		{
			if(FILTER_TYPE==FLOAT32){

				for(i=0, k=0; i<(WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE/2); i++) {
					if(i%2) {
						inputF32Buffer[k] = (float32_t)(RxBuffer[i]/32768.0);//convert to float LEFT
						tempF32Buffer[k] = inputF32Buffer[k];

						if(pass_through != 0){
							outputF32_H[k] = inputF32[k];
							outputF32_L[k] = inputF32[k];
						}
						k++;
					}

				}

				if(pass_through == 0){


					// Frequency variation automation --------
//					variator(&S_V,&S_EQ,EQ); 		// Parametric EQ variation - change S_EQ.f0

//					if(S_EQ.prev_f0 != S_EQ.f0){
//						S_EQ.prev_f0 = S_EQ.f0;
//						eq_coef_calc(&S_EQ);
//					}

//					variator(&S_C,&S_LP,CROSSOVER);		// Crossover fc variation - change S_LP.f0

//					if(S_LP.prev_f0 != S_LP.f0){
//						S_HP.f0 = S_LP.f0;
//						S_HP.prev_f0 = S_HP.f0;
//						S_LP.prev_f0 = S_LP.f0;
//						cross_bind_coef_calc(&S_LP,&S_HP);
//					}

					//---------------------------------------


//					arm_biquad_cascade_df1_f32(&S, inputF32, outputF32_H, BLOCK_SIZE);

//					arm_biquad_cascade_df1_f32(&S, inputF32, outputF32_L, BLOCK_SIZE);

//					arm_biquad_cascade_df1_f32(&S, inputF32, tempOut, BLOCK_SIZE);
//					arm_biquad_cascade_df1_f32(&S_H, outputF32_L, outputF32_H, BLOCK_SIZE);
//					arm_biquad_cascade_df1_f32(&S_L, tempOut, outputF32_L, BLOCK_SIZE);

					arm_biquad_cascade_df1_f32(&S_H, inputF32, outputF32_H, BLOCK_SIZE);
					arm_biquad_cascade_df1_f32(&S_L, inputF32, outputF32_L, BLOCK_SIZE);
				}



				for(i=0, k=0; i<(WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE/2); i++) {
					if(i%2)	{
						TxBuffer[i] = (int16_t)(outputF32Buffer_L[k]*32768);//back to 1.15
						k++;
					}
					else {
						TxBuffer[i] = (int16_t)(outputF32Buffer_H[k]*32768);//back to 1.15

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
						tempF32Buffer[k] = inputF32Buffer[k];

						if(pass_through != 0){
							outputF32_H[k] = inputF32[k];
							outputF32_L[k] = inputF32[k];
						}
						k++;
					}
				}

				if(pass_through == 0){
					// Frequency variation automation --------

//					variator(&S_V,&S_EQ,EQ); 		// Parametric EQ variation - change S_EQ.f0


//					variator(&S_C,&S_LP,CROSSOVER);		// Crossover fc variation - change S_LP.f0

//					if(S_LP.prev_f0 != S_LP.f0){
//						S_HP.f0 = S_LP.f0;
//						S_HP.prev_f0 = S_HP.f0;
//						S_LP.prev_f0 = S_LP.f0;
//						cross_bind_coef_calc(&S_LP,&S_HP);
//					}

	//				//---------------------------------------

//					arm_biquad_cascade_df1_f32(&S, inputF32, outputF32_H, BLOCK_SIZE);

//					arm_biquad_cascade_df1_f32(&S, inputF32, outputF32_L, BLOCK_SIZE);

//					arm_biquad_cascade_df1_f32(&S, inputF32, tempOut, BLOCK_SIZE);
//					arm_biquad_cascade_df1_f32(&S_H, outputF32_L, outputF32_H, BLOCK_SIZE);
//					arm_biquad_cascade_df1_f32(&S_L, tempOut, outputF32_L, BLOCK_SIZE);

					arm_biquad_cascade_df1_f32(&S_H, inputF32, outputF32_H, BLOCK_SIZE);
					arm_biquad_cascade_df1_f32(&S_L, inputF32, outputF32_L, BLOCK_SIZE);
				}

				//crossover(inputF32, outputF32, BLOCK_SIZE);


				for(i=(WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE/2), k=0; i<WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE; i++) {
					if(i%2)	{
						TxBuffer[i] = (int16_t)(outputF32Buffer_L[k]*32768);//back to 1.15
						k++;
					}
					else {
						TxBuffer[i] = (int16_t)(outputF32Buffer_H[k]*32768);//back to 1.15
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

//***********************************************************************

int eq_coef_calc(param_eq_instance* S){

	float32_t a,b,B,K,w0;

	B = S->f0/S->Q;
	K = pow(10.0,(S->G/20));
	w0 = PI*B/AUDIO_FREQUENCY_48K;
	a = arm_sin_f32(w0)/arm_cos_f32(w0); // tg
	b = -arm_cos_f32(2*PI*(S->f0/AUDIO_FREQUENCY_48K));
	a = (1 - a)/(1 + a);

//	if(S->G>1){
//
//	}else if(S->G<=1 && S->G>0){
//
//	}

    S->coefs[0] = (1+a+K-K*a)*0.5;		// b0
    S->coefs[1] = (b+b*a);				// b1
    S->coefs[2] = (1+a-K+K*a)*0.5;		// b2
    S->coefs[3] = -S->coefs[1]; 		// -a1
    S->coefs[4] = -a;					// -a2

    return 0;
}
//***********************************************************************

int cross_bind_coef_calc(param_eq_instance* SL, param_eq_instance* SH){

	if(SL->Q==0) SL->Q = 0.001;

	float32_t w0 = 2.0*PI*SL->f0 / AUDIO_FREQUENCY_48K;  //2 * Pi * f0 / Fs;
	float32_t cos_w0 = arm_cos_f32(w0);
	float32_t alpha = arm_sin_f32(w0) / (2.0 * SL->Q);
	float32_t a0 = 1.0 + alpha;
	a0 = 1/a0;										// normaliza

	SL->coefs[0] = ((1.0 - cos_w0) / 2)*a0; 									// b0
	SL->coefs[1] = 2*SL->coefs[0];					//(1 - arm_cos_f32(w0));		// b1
	SL->coefs[2] = SL->coefs[0];					//(1 - arm_cos_f32(w0)) / 2;	// b2
	SL->coefs[3] = -(-2.0 * cos_w0)*a0;										// -a1
	SL->coefs[4] = -(1.0 - alpha)*a0;


	SH->coefs[0] = ((1 + arm_cos_f32(w0)) / 2)*a0; 									// b0
	SH->coefs[1] = -2.0*SH->coefs[0];					//(1 + arm_cos_f32(w0));		// b1
	SH->coefs[2] = SH->coefs[0];					//(1 + arm_cos_f32(w0)) / 2;	// b2
	SH->coefs[3] = SL->coefs[3];					//-(-2 * arm_cos_f32(w0))*a0;	// -a1
	SH->coefs[4] = SL->coefs[4];					//-(1 - alpha)*a0;


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

int variator(vari_eq_instance *S, param_eq_instance* S_EQ, vari_mode mode){

	S->time_count++;

	if(S->time_count > S->time_limit){
		S->time_count = 0;

		if(S->up_filter == 1){

			S_EQ->f0 += S->freq_step;

			if(S_EQ->f0 > (S->freq_max - S->freq_step))
				S->up_filter = 0;

//			S->up_dw_cout++;

		}
		else{
			S_EQ->f0 -= S->freq_step;

			if(S_EQ->f0 < (S->freq_min + S->freq_step))
				S->up_filter = 1;

//			S->up_dw_cout--;
		}
	}
	if(mode==EQ && check_variation(S_EQ))
		eq_coef_calc(S_EQ);


	return 0;
}

//***********************************************************************
int check_variation(param_eq_instance* S){

	if(S->prev_f0 != S->f0){
		S->prev_f0 = S->f0;
		return 1;
	}
	return 0;
}
