/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};
/* Definitions for t4G_uart_rev_h */
osThreadId_t t4G_uart_rev_hHandle;
const osThreadAttr_t t4G_uart_rev_h_attributes = {
  .name = "t4G_uart_rev_h",
  .priority = (osPriority_t) osPriorityNormal2,
  .stack_size = 512 * 4
};
/* Definitions for uart_debug_send */
osThreadId_t uart_debug_sendHandle;
const osThreadAttr_t uart_debug_send_attributes = {
  .name = "uart_debug_send",
  .priority = (osPriority_t) osPriorityBelowNormal,
  .stack_size = 128 * 4
};
/* Definitions for uart_debug_rev */
osThreadId_t uart_debug_revHandle;
const osThreadAttr_t uart_debug_rev_attributes = {
  .name = "uart_debug_rev",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
osThreadId_t t4G_data_sendHandle;
const osThreadAttr_t t4G_data_send_attributes = {
  .name = "t4G_data_send",
  .priority = (osPriority_t) osPriorityNormal1,
  .stack_size = 128 * 4
};
 
extern void task_t4G_data_send(void *argument);

osThreadId_t t4G_client_0Handle;
const osThreadAttr_t t4G_dclient_0_attributes = {
  .name = "t4G_client_0",
  .priority = (osPriority_t) osPriorityNormal1,
  .stack_size = 512 * 4
};
 
extern void task_client_0(void *argument);

osThreadId_t rf_callbackHandle;
const osThreadAttr_t rf_callback_attributes = {
  .name = "rf_callback",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 512 * 4
};
 
extern void task_rf_callback(void *argument);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
extern void task_4g_uart_rev_handle(void *argument);
extern void task_uart_debug_send(void *argument);
extern void task_uart_debug_rev_handle(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of t4G_uart_rev_h */
  t4G_uart_rev_hHandle = osThreadNew(task_4g_uart_rev_handle, NULL, &t4G_uart_rev_h_attributes);

  /* creation of uart_debug_send */
  uart_debug_sendHandle = osThreadNew(task_uart_debug_send, NULL, &uart_debug_send_attributes);

  /* creation of uart_debug_rev */
  uart_debug_revHandle = osThreadNew(task_uart_debug_rev_handle, NULL, &uart_debug_rev_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  t4G_data_sendHandle = osThreadNew(task_t4G_data_send, NULL, &t4G_data_send_attributes);	
	
	t4G_client_0Handle = osThreadNew(task_client_0, NULL, &t4G_dclient_0_attributes);	

  rf_callbackHandle = osThreadNew(task_rf_callback, NULL, &rf_callback_attributes);	
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
	uint32_t delay = 0;
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
		HAL_GPIO_WritePin(WDI_GPIO_Port,WDI_Pin,GPIO_PIN_RESET);
		delay++;
		if(delay>=100)
		{
			delay = 0;
			HAL_GPIO_TogglePin(S2_R_GPIO_Port,S2_R_Pin);
		}
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
