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
#include "defines_sys.h"
#include "crossover.h"
#include "equalizer.h"
#include "interface.h"


I2C_HandleTypeDef hi2c1;
uint32_t AcceleroTicks;
int16_t AcceleroAxis[3];
int16_t TxBuffer[WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE];
int16_t RxBuffer[WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE];

__IO BUFFER_StateTypeDef buffer_offset = BUFFER_OFFSET_NONE;
__IO uint8_t Volume = OUT_MAX_VOLUME;

// SPI -----------------------------
SPI_HandleTypeDef SpiHandle;
uint8_t aTxBuffer[20];

#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
#define BUFFERSIZE              (COUNTOF(aTxBuffer) - 1)

uint8_t aRxBuffer[BUFFERSIZE];

static void spi_init(void);
static void SystemClock_Config(void);
static void Error_Handler(void);
static void Timeout_Error_Handler(void);

// ----------------------------------

volatile uint32_t pushButtonLastTick;
volatile float32_t inputGain;

sys_controls_union controls;

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
	SystemClock_Config();

	DWT_Enable();

	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED6);
	BSP_LED_Init(LED5);

#ifdef OS_USE_SEMIHOSTING
#ifdef CYCLE_COUNTER
	FILE *CycleFile;
	uint32_t cycleCount;
	CycleFile = fopen("cyclecounter_full_interface.txt", "w");
	if (!CycleFile) {
		trace_printf("Error trying to open cycle counter file\n.");
		while(1);
	}

	 DWT_Reset();
	 cycleCount = DWT_GetValue();
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

	ssd1306_Init();

	spi_init();

	float32_t inputF32Buffer[BLOCK_SIZE];
	float32_t outputF32Buffer_H[BLOCK_SIZE];
	float32_t outputF32Buffer_L[BLOCK_SIZE];
	float32_t tempF32Buffer[BLOCK_SIZE];

	coefs_buffers_instance buffers;
	arm_biquad_casd_df1_inst_f32 biquads[NUM_BIQUADS];
	filter_instance filters[NUM_FILTERS];
	float32_t *io[TOTAL_IO_BUFFERS];

	io[INPUT_BUFFER] =		&inputF32Buffer[0];
	io[OUTPUT_BUFFER_H] =	&outputF32Buffer_H[0];
	io[OUTPUT_BUFFER_L] =	&outputF32Buffer_L[0];
	io[OUTPUT_BUFFER_TEMP]= &tempF32Buffer[0];

	controls = interface_init(&buffers, filters, biquads);

	eq_coef_calc(&filters[PARAM_EQ]);
	cross_bind_coef_calc(&filters[CROSS_LP],&filters[CROSS_HP]);

	uint32_t i, k;
	pushButtonLastTick = 0;
	inputGain = 1;


	uint8_t *aTx = &aTxBuffer[0];
	uint8_t *aRx = &aRxBuffer[0];

	sprintf(aTxBuffer, "%s\n%d", "Cross\nON",200);

	while (1) {

//		if(controls.bypass != 0){
//			states_control(filters,&controls,aTxBuffer);
//			controls.disp_enable = 0;
//		}

		if(buffer_offset == BUFFER_OFFSET_HALF)
		{
			for(i=0, k=0; i<(WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE/2); i++) {
				if(i%2) {
					inputF32Buffer[k] = (float32_t)(RxBuffer[i]/32768.0);//convert to float LEFT
					tempF32Buffer[k] = inputF32Buffer[k];

					if(controls.bypass != 0){
						io[OUTPUT_BUFFER_H][k] = io[INPUT_BUFFER][k];
						io[OUTPUT_BUFFER_L][k] = io[INPUT_BUFFER][k];
					}
					k++;
				}
			}

			if(controls.bypass == 0){
				interface(io, filters, biquads, &controls,aTxBuffer);
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

			buffer_offset = BUFFER_OFFSET_NONE;
		}

		if(buffer_offset == BUFFER_OFFSET_FULL)
		{
//			DWT_Reset();

			for(i=(WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE/2), k=0; i<WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE; i++) {
				if(i%2) {
					inputF32Buffer[k] = (float32_t)(RxBuffer[i]/32768.0);//convert to float
					tempF32Buffer[k] = inputF32Buffer[k];

					if(controls.bypass != 0){
						io[OUTPUT_BUFFER_H][k] = io[INPUT_BUFFER][k];
						io[OUTPUT_BUFFER_L][k] = io[INPUT_BUFFER][k];
					}
					k++;
				}
			}

//			cycleCount = DWT_GetValue();

			if(controls.bypass == 0){
				interface(io, filters, biquads, &controls,aTxBuffer);
			}

#ifdef CYCLE_COUNTER
			fprintf(CycleFile, "\nFULL: %lu", (DWT_GetValue()- cycleCount));
#endif

			for(i=(WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE/2), k=0; i<WOLFSON_PI_AUDIO_TXRX_BUFFER_SIZE; i++) {
				if(i%2)	{
					TxBuffer[i] = (int16_t)(outputF32Buffer_L[k]*32768);//back to 1.15
					k++;
				}
				else {
					TxBuffer[i] = (int16_t)(outputF32Buffer_H[k]*32768);//back to 1.15
				}
			}


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

// ************************************************************************
// Hardware config
// ************************************************************************

// SPI ////////////////////////////////////////////////////////////////////
static void spi_init(void)
{
	  SpiHandle.Instance               = SPI1;
	  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
	  SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
	  SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
	  SpiHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;
	  SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
	  SpiHandle.Init.CRCPolynomial     = 7;
	  SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
	  SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	  SpiHandle.Init.NSS               = SPI_NSS_SOFT;
	  SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
	  SpiHandle.Init.Mode 			   = SPI_MODE_MASTER;

	  if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
	   {
	     /* Initialization Error */
	     Error_Handler();
	   }


	switch(HAL_SPI_TransmitReceive(&SpiHandle, (uint8_t*)aTxBuffer, (uint8_t *)aRxBuffer, BUFFERSIZE, 5000))
	{
	case HAL_OK:
	  /* Communication is completed ____________________________________________*/
	  /* Compare the sent and received buffers */
//	  if(Buffercmp((uint8_t*)aTxBuffer, (uint8_t*)aRxBuffer, BUFFERSIZE))
//	  {
//		/* Transfer error in transmission process */
//		Error_Handler();
//	  }

	  /* Turn LED4 on: Transfer in transmission process is correct */
	  BSP_LED_On(LED4);
	  /* Turn LED6 on: Transfer in reception process is correct */
	  BSP_LED_On(LED6);
	  break;

	case HAL_TIMEOUT:
	  /* A Timeout occured______________________________________________________*/
	  /* Call Timeout Handler */
	  Timeout_Error_Handler();
	  break;

	  /* An Error occured_______________________________________________________*/
	case HAL_ERROR:
	  /* Call Timeout Handler */
	  Error_Handler();
	  break;

	default:
	  break;
	}
}

static void Error_Handler(void){
	  /* Turn LED5 (RED) on */
	  BSP_LED_On(LED5);
	  while(1){}
}

static void SystemClock_Config(void){

	  RCC_ClkInitTypeDef RCC_ClkInitStruct;
	  RCC_OscInitTypeDef RCC_OscInitStruct;

	  /* Enable Power Control clock */
	  __PWR_CLK_ENABLE();

	  /* The voltage scaling allows optimizing the power consumption when the device is
	     clocked below the maximum system frequency, to update the voltage scaling value
	     regarding system frequency refer to product datasheet.  */
	  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	  /* Enable HSE Oscillator and activate PLL with HSE as source */
	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	  RCC_OscInitStruct.PLL.PLLM = 8;
	  RCC_OscInitStruct.PLL.PLLN = 336;
	  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	  RCC_OscInitStruct.PLL.PLLQ = 7;
	  HAL_RCC_OscConfig(&RCC_OscInitStruct);

	  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	     clocks dividers */
	  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

static void Timeout_Error_Handler(void){
	  /* Toggle LED5 on */
	  while(1)
	  {
	    BSP_LED_On(LED5);
	    HAL_Delay(500);
	    BSP_LED_Off(LED5);
	    HAL_Delay(500);
	  }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	 /* Turn LED5 on: Transfer error in reception/transmission process */
	 BSP_LED_On(LED5);
}


///////////////////////////////////////////////////////////////////////////

void MX_I2C1_Init(void)
{
	hi2c1.Instance = I2C1;
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

	  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7;
	  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0); // pin 4
	  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);  // pin 5 - 7
	  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

// ************************************************************************


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	if((GPIO_Pin == GPIO_PIN_5)){
		if(HAL_GetTick() > (pushButtonLastTick + BTN_DEBOUNCE)){
			menuValueAdd(&controls);
			pushButtonLastTick = HAL_GetTick();
		}

	}
	else if((GPIO_Pin == GPIO_PIN_7)){
		if(HAL_GetTick() > (pushButtonLastTick + BTN_DEBOUNCE)){
			menuValueEnter(&controls);
			pushButtonLastTick = HAL_GetTick();
		}
	}

	else if((GPIO_Pin == GPIO_PIN_4)){
		if(HAL_GetTick() > (pushButtonLastTick + BTN_DEBOUNCE)){
			menuValueSub(&controls);
			pushButtonLastTick = HAL_GetTick();
		}
		}
}

