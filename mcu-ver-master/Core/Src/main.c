/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
#define DAT_SIZE 1000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t dumb_prng(){
    static uint8_t prev1 = 220;
    static uint8_t prev2 = 6;
    uint8_t ret = prev1 + prev2;
    prev1 = prev2;
    prev2 = ret;
    return ret;
}

void spi_recv(uint8_t* recv, size_t dat_size, SPI_TypeDef* spi_dev){
    // Global interrupt disable to ensure smooth operation
    __disable_irq();

    // Receive Data
    LL_SPI_Disable(spi_dev);
    LL_SPI_Enable(spi_dev);
    LL_SPI_SetMode(spi_dev, LL_SPI_MODE_MASTER);

    __asm__ __volatile__ (
    // r0: A byte of zeroes
    // r5: Temp storage for received value
    // r6: Temp storage for SPI status register value
    // r7: Array pointer
    // Setup
    "mov r7, %[ptr]\n\t"          // Set up array pointer
    "mov r0, #0\n\t"              // Set up zero register
    "ldr r5, [%[SPI_DR]]\n\t"     // Read value from RX buffer to clear anything that might be there already
    // Main loop
    "loop1_start:\n\t"            //
    "cmp r7, %[ptr_end]\n\t"      // Have we reached the end of the array?
    "bcs loop1_end\n\t"           // If so, exit the loop
    "wait_loop1:\n\t"             // Beginning of polling loop for RX/TX readiness bits
    "ldr r6, [%[SPI_SR]]\n\t"     // Load SPI status register
    "tst r6, %[COND]\n\t"         // Is the TX buffer ready?
    "beq wait_loop1\n\t"          // If not, keep polling
    "strb r0, [%[SPI_DR]]\n\t"    // Write a bunch of zeroes to the TX buffer
    "wait_loop2:\n\t"             // Beginning of polling loop for RX/TX readiness bits
    "ldr r6, [%[SPI_SR]]\n\t"     // Load SPI status register
    "tst r6, %[COND2]\n\t"        // Is the RX buffer ready?
    "beq wait_loop2\n\t"          // If not, keep polling
    "ldr r5, [%[SPI_DR]]\n\t"     // Read value from RX buffer
    "strb r5, [r7], #1\n\t"       // Store received value and increment pointer
    "b loop1_start\n\t"           // Loop the loop
    "loop1_end:\n\t"              //
    :
    : [SPI_SR] "r" (&(spi_dev->SR)), [SPI_DR] "r" (&(spi_dev->DR)), [COND] "I" (SPI_SR_TXE), [COND2] "I" (SPI_SR_RXNE), [ptr] "r" (recv), [ptr_end] "r" (recv + dat_size)
    : "r5", "r6", "r7", "r0", "memory"
    );

    __enable_irq();

    // Wait for SPI peripheral to finish anything it's doing
    while(LL_SPI_IsActiveFlag_BSY(spi_dev));
    LL_SPI_Disable(spi_dev);

}

uint8_t spi_test(const uint8_t* dat, size_t dat_size, SPI_TypeDef* spi_dev){
    uint8_t recv[dat_size];
    for(size_t i = 0; i < dat_size; i++){
        recv[i] = 0;
    }
    spi_recv(recv, dat_size, spi_dev);
    // Make sure the data is what it's supposed to be
    for(unsigned int i = 0; i < dat_size; i++){
        if(recv[i] != dat[i]){
            return 1;
        }
    }
    return 0;
}

uint8_t uart_test(const uint8_t* dat, size_t dat_size, UART_HandleTypeDef* uart_dev){
    uint8_t recv[dat_size];
    for(size_t i = 0; i < dat_size; i++){
        recv[i] = 0;
    }
    HAL_UART_Abort(uart_dev);
    HAL_UART_Receive(uart_dev, recv, dat_size, HAL_MAX_DELAY);
    // Make sure the data is what it's supposed to be
    for(unsigned int i = 0; i < dat_size; i++){
        if(recv[i] != dat[i]){
            return 1;
        }
    }
    return 0;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

    // Generate test data
    uint8_t dat[DAT_SIZE];
    for(unsigned int i = 0; i < DAT_SIZE; i++){
      dat[i] = dumb_prng();
    }

    // Initially set LED low
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

    // Variable for counting the number of failed tests
    uint8_t errcnt = 0;


    LL_SPI_Disable(SPI1);
    LL_SPI_Disable(SPI2);
    LL_SPI_Disable(SPI3);

    // Wait for button press
    while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin));

    // Test SPI
    errcnt += spi_test(dat, DAT_SIZE, SPI1);
    HAL_Delay(10);
    errcnt += spi_test(dat, DAT_SIZE, SPI2);
    HAL_Delay(10);
    errcnt += spi_test(dat, DAT_SIZE, SPI3);
    HAL_Delay(1);
    errcnt+= uart_test(dat, DAT_SIZE, &huart2);

    // Light up the LED if all tests passed
    if(errcnt == 0){
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 20;
  RCC_OscInitStruct.PLL.PLLN = 128;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
