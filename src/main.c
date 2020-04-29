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
#include "crossover.h"
#include "equalizer.h"
#include "interface.h"
#include "defines.h"

//#include "math_helper.h"

//#define TRACE_DEBUG

#undef CYCLE_COUNTER
//#define CYCLE_COUNTER

I2C_HandleTypeDef hi2c1;
uint32_t AcceleroTicks;
int16_t AcceleroAxis[3];
int16_t TxBuffer[WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE];
int16_t RxBuffer[WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE];

__IO BUFFER_StateTypeDef buffer_offset = BUFFER_OFFSET_NONE;
__IO uint8_t Volume = 80;  // set volume to 60% to avoid floor noise


// Arrumar --------------------------------------------------------------------------------
GPIO_PinState encoderLastVal = GPIO_PIN_RESET;
GPIO_PinState encoderNowVal = GPIO_PIN_RESET;
uint32_t encoderValue = 0;
const uint32_t encoderDebounceTime = 5; // in ms
const uint32_t pushButtonDebounceTime = 100;

uint32_t encoderLastTick;
uint32_t pushButtonLastTick;
char encoderValueStr[10];

uint32_t Value;
char ValueStr[10];

void MX_I2C1_Init(void);
void MX_GPIO_Init(void);

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

	MX_I2C1_Init();

	MX_GPIO_Init();

	// ----------------------- Float point 32 processing -------------------------------------------
	float32_t  *inputF32, *outputF32_H, *outputF32_L, *tempOut;
	float32_t inputF32Buffer[BLOCK_SIZE];
	float32_t outputF32Buffer_H[BLOCK_SIZE];
	float32_t outputF32Buffer_L[BLOCK_SIZE];
	float32_t tempF32Buffer[BLOCK_SIZE];
	float32_t tempF32BufferH[BLOCK_SIZE];
	float32_t tempF32BufferL[BLOCK_SIZE];

	inputF32 = &inputF32Buffer[0];
	outputF32_H = &outputF32Buffer_H[0];
	outputF32_L = &outputF32Buffer_L[0];
	tempOut = &tempF32Buffer[0];

	param_eq_instance S_EQ, S_LP, S_HP;
	vari_eq_instance S_V,S_C;

	// Crossover -------------------------------
	float32_t cross_Fc = 500.0;
	float32_t cross_G = 1;
	float32_t cross_Q = 0.5; // cross_Q > 5

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
	S_EQ.f0 = 100.0;
	S_EQ.G = -24;
	S_EQ.Q = 6;
	S_EQ.prev_f0 = S_EQ.f0;
	S_EQ.prev_G = S_EQ.G;
	S_EQ.prev_Q = S_EQ.Q;

	// Variators -------------------------------
	S_V.up_filter = 1;
	S_V.time_count = 0;
	S_V.up_dw_cout = 0;
	S_V.time_limit = 1;
	S_V.freq_max = 10000;
	S_V.freq_min = 1000;
	S_V.freq_step = 20.0;

	S_C.up_filter = 0;
	S_C.time_count = 0;
	S_C.up_dw_cout = 0;
	S_C.time_limit = 5;
	S_C.freq_max = 1000;
	S_C.freq_min = cross_Fc;
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
	uint8_t HALF = BLOCK_SIZE/2;

	uint32_t i, k;

	ssd1306_Init();

	Value = 0;
	encoderNowVal = 0;

	while (1) {

		sprintf(ValueStr, "%lu", Value);
//		trace_printf(ValueStr);
//		trace_printf("\n");
		menuPrintLines("Valor", ValueStr, NULL);

		if(buffer_offset == BUFFER_OFFSET_HALF)
		{


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
					variator(&S_V,&S_EQ); 		// Parametric EQ variation - change S_EQ.f0

//					variator(&S_C,&S_LP,CROSSOVER);		// Crossover fc variation - change S_LP.f0

//					if(S_LP.prev_f0 != S_LP.f0){
//						S_HP.f0 = S_LP.f0;
//						S_HP.prev_f0 = S_HP.f0;
//						S_LP.prev_f0 = S_LP.f0;
//						cross_bind_coef_calc(&S_LP,&S_HP);
//					}

					//---------------------------------------

					arm_biquad_cascade_df1_f32(&S, inputF32, tempOut, BLOCK_SIZE);
					arm_biquad_cascade_df1_f32(&S_L, tempOut, outputF32_L, BLOCK_SIZE);
					arm_biquad_cascade_df1_f32(&S_H, tempOut, outputF32_H, BLOCK_SIZE);

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



#ifdef CYCLE_COUNTER
			fprintf(CycleFile, "\nHALF: %lu", (DWT_GetValue()- cycleCount));
#endif

			buffer_offset = BUFFER_OFFSET_NONE;

		}

		if(buffer_offset == BUFFER_OFFSET_FULL)
		{
			DWT_Reset();
			//cycleCount = DWT_GetValue();


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

					variator(&S_V,&S_EQ); 		// Parametric EQ variation - change S_EQ.f0


//					variator(&S_C,&S_LP,CROSSOVER);		// Crossover fc variation - change S_LP.f0

//					if(S_LP.prev_f0 != S_LP.f0){
//						S_HP.f0 = S_LP.f0;
//						S_HP.prev_f0 = S_HP.f0;
//						S_LP.prev_f0 = S_LP.f0;
//						cross_bind_coef_calc(&S_LP,&S_HP);
//					}

	//				//---------------------------------------

					arm_biquad_cascade_df1_f32(&S, inputF32, tempOut, BLOCK_SIZE);
					arm_biquad_cascade_df1_f32(&S_L, tempOut, outputF32_L, BLOCK_SIZE);
					arm_biquad_cascade_df1_f32(&S_H, tempOut, outputF32_H, BLOCK_SIZE);
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


//***********************************************************************




// ************************************************************************
// Hardware config
// ************************************************************************
void MX_I2C1_Init(void)
{

	hi2c1.Instance = I2C1;
	//  hi2c1.Init.ClockSpeed = 100000; // 9 FPS
	//  hi2c1.Init.ClockSpeed = 100000 * 2; // 19 FPS
	hi2c1.Init.ClockSpeed = 100000 * 4; // 37 FPS
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

	GPIO_InitTypeDef GPIO_InitStruct;

	/**I2C1 GPIO Configuration
	PB8     ------> I2C1_SCL
	PB9     ------> I2C1_SDA
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	__HAL_RCC_I2C1_CLK_ENABLE();

	HAL_I2C_Init(&hi2c1);
}
// ************************************************************************
void MX_GPIO_Init(void){

	 GPIO_InitTypeDef GPIO_InitStruct;

	 __HAL_RCC_GPIOB_CLK_ENABLE();

	  /*Configure GPIO pins : PB3 PB4 PB5 */
	  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
	  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  /*Configure GPIO pin : PB7 */
	  GPIO_InitStruct.Pin = GPIO_PIN_7;
	  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  /* EXTI interrupt init*/

	  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

// ************************************************************************


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	if((GPIO_Pin == GPIO_PIN_7)){
		if(HAL_GetTick() > (encoderLastTick + pushButtonDebounceTime)){ // debounce 20 ms
			if(Value > 0)
				Value --;
			encoderLastTick = HAL_GetTick();
		}
	}
	else if((GPIO_Pin == GPIO_PIN_4)){
		if(HAL_GetTick() > (encoderLastTick + pushButtonDebounceTime)){ // debounce 20 ms
			Value=0;
			encoderLastTick = HAL_GetTick();
		}
	}

	else if((GPIO_Pin == GPIO_PIN_5)){
		if(HAL_GetTick() > (encoderLastTick + pushButtonDebounceTime)){ // debounce 20 ms
			Value ++;
			encoderLastTick = HAL_GetTick();
		}
	}

}

