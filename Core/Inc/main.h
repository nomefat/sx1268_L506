/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define DEBUG_UART huart6
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RF1_SPI4_SCK_Pin GPIO_PIN_2
#define RF1_SPI4_SCK_GPIO_Port GPIOE
#define RF1_RESET_Pin GPIO_PIN_3
#define RF1_RESET_GPIO_Port GPIOE
#define RF1_NSS_Pin GPIO_PIN_4
#define RF1_NSS_GPIO_Port GPIOE
#define RF1_SPI4_MISO_Pin GPIO_PIN_5
#define RF1_SPI4_MISO_GPIO_Port GPIOE
#define RF1_SPI4_MOSI_Pin GPIO_PIN_6
#define RF1_SPI4_MOSI_GPIO_Port GPIOE
#define RF1_BUSY_Pin GPIO_PIN_8
#define RF1_BUSY_GPIO_Port GPIOI
#define RF1_DIO1_EXTI9_Pin GPIO_PIN_9
#define RF1_DIO1_EXTI9_GPIO_Port GPIOI
#define RF1_DIO1_EXTI9_EXTI_IRQn EXTI9_5_IRQn
#define RF1_DIO2_Pin GPIO_PIN_10
#define RF1_DIO2_GPIO_Port GPIOI
#define RF1_DIO3_Pin GPIO_PIN_11
#define RF1_DIO3_GPIO_Port GPIOI
#define RF1_TX_EN_Pin GPIO_PIN_0
#define RF1_TX_EN_GPIO_Port GPIOF
#define RF1_RX_EN_Pin GPIO_PIN_1
#define RF1_RX_EN_GPIO_Port GPIOF
#define RF2_RESET_Pin GPIO_PIN_2
#define RF2_RESET_GPIO_Port GPIOF
#define RF2_DIO1_EXTI3_Pin GPIO_PIN_3
#define RF2_DIO1_EXTI3_GPIO_Port GPIOF
#define RF2_DIO1_EXTI3_EXTI_IRQn EXTI3_IRQn
#define RF2_DIO2_Pin GPIO_PIN_4
#define RF2_DIO2_GPIO_Port GPIOF
#define RF2_DIO3_Pin GPIO_PIN_5
#define RF2_DIO3_GPIO_Port GPIOF
#define RF2_NSS_Pin GPIO_PIN_6
#define RF2_NSS_GPIO_Port GPIOF
#define RF2_SPI5_SCK_Pin GPIO_PIN_7
#define RF2_SPI5_SCK_GPIO_Port GPIOF
#define RF2_SPI5_MISO_Pin GPIO_PIN_8
#define RF2_SPI5_MISO_GPIO_Port GPIOF
#define RF2_SPI5_MOSI_Pin GPIO_PIN_9
#define RF2_SPI5_MOSI_GPIO_Port GPIOF
#define RF2_BUSY_Pin GPIO_PIN_10
#define RF2_BUSY_GPIO_Port GPIOF
#define RF2_TX_EN_Pin GPIO_PIN_2
#define RF2_TX_EN_GPIO_Port GPIOC
#define RF2_RX_EN_Pin GPIO_PIN_3
#define RF2_RX_EN_GPIO_Port GPIOC
#define EN_1_8V_Pin GPIO_PIN_0
#define EN_1_8V_GPIO_Port GPIOA
#define RF2_PWR_EN_Pin GPIO_PIN_2
#define RF2_PWR_EN_GPIO_Port GPIOH
#define RF1_PWR_EN_Pin GPIO_PIN_3
#define RF1_PWR_EN_GPIO_Port GPIOH
#define USB_KEY_IIC2_SCL_Pin GPIO_PIN_4
#define USB_KEY_IIC2_SCL_GPIO_Port GPIOH
#define USB_KEY_IIC2_SDA_Pin GPIO_PIN_5
#define USB_KEY_IIC2_SDA_GPIO_Port GPIOH
#define L506_PWR_EN_Pin GPIO_PIN_0
#define L506_PWR_EN_GPIO_Port GPIOB
#define EN_48V_Pin GPIO_PIN_1
#define EN_48V_GPIO_Port GPIOB
#define WDI_Pin GPIO_PIN_9
#define WDI_GPIO_Port GPIOE
#define LD0_Pin GPIO_PIN_10
#define LD0_GPIO_Port GPIOE
#define DATA7_Pin GPIO_PIN_11
#define DATA7_GPIO_Port GPIOE
#define DATA6_Pin GPIO_PIN_12
#define DATA6_GPIO_Port GPIOE
#define DATA5_Pin GPIO_PIN_13
#define DATA5_GPIO_Port GPIOE
#define DATA4_Pin GPIO_PIN_14
#define DATA4_GPIO_Port GPIOE
#define DATA3_Pin GPIO_PIN_15
#define DATA3_GPIO_Port GPIOE
#define DATA2_Pin GPIO_PIN_6
#define DATA2_GPIO_Port GPIOH
#define DATA1_Pin GPIO_PIN_7
#define DATA1_GPIO_Port GPIOH
#define S7_R_Pin GPIO_PIN_8
#define S7_R_GPIO_Port GPIOH
#define S7_G_Pin GPIO_PIN_9
#define S7_G_GPIO_Port GPIOH
#define S6_R_Pin GPIO_PIN_10
#define S6_R_GPIO_Port GPIOH
#define S6_G_Pin GPIO_PIN_11
#define S6_G_GPIO_Port GPIOH
#define S5_R_Pin GPIO_PIN_12
#define S5_R_GPIO_Port GPIOB
#define S5_G_Pin GPIO_PIN_13
#define S5_G_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_14
#define LED1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_15
#define LED2_GPIO_Port GPIOB
#define LED3_Pin GPIO_PIN_8
#define LED3_GPIO_Port GPIOD
#define LED4_Pin GPIO_PIN_9
#define LED4_GPIO_Port GPIOD
#define LED5_Pin GPIO_PIN_10
#define LED5_GPIO_Port GPIOD
#define LED6_Pin GPIO_PIN_11
#define LED6_GPIO_Port GPIOD
#define LED7_Pin GPIO_PIN_12
#define LED7_GPIO_Port GPIOD
#define LED8_Pin GPIO_PIN_13
#define LED8_GPIO_Port GPIOD
#define LED9_Pin GPIO_PIN_14
#define LED9_GPIO_Port GPIOD
#define LED10_Pin GPIO_PIN_15
#define LED10_GPIO_Port GPIOD
#define USB_KEY_VCC_EN7_Pin GPIO_PIN_2
#define USB_KEY_VCC_EN7_GPIO_Port GPIOG
#define USB_KEY_VCC_EN6_Pin GPIO_PIN_3
#define USB_KEY_VCC_EN6_GPIO_Port GPIOG
#define USB_KEY_VCC_EN5_Pin GPIO_PIN_4
#define USB_KEY_VCC_EN5_GPIO_Port GPIOG
#define USB_KEY_VCC_EN4_Pin GPIO_PIN_5
#define USB_KEY_VCC_EN4_GPIO_Port GPIOG
#define USB_KEY_VCC_EN3_Pin GPIO_PIN_6
#define USB_KEY_VCC_EN3_GPIO_Port GPIOG
#define USB_KEY_VCC_EN2_Pin GPIO_PIN_7
#define USB_KEY_VCC_EN2_GPIO_Port GPIOG
#define USB_KEY_VCC_EN1_Pin GPIO_PIN_8
#define USB_KEY_VCC_EN1_GPIO_Port GPIOG
#define S4_R_Pin GPIO_PIN_13
#define S4_R_GPIO_Port GPIOH
#define S4_G_Pin GPIO_PIN_14
#define S4_G_GPIO_Port GPIOH
#define S3_R_Pin GPIO_PIN_15
#define S3_R_GPIO_Port GPIOH
#define S3_G_Pin GPIO_PIN_0
#define S3_G_GPIO_Port GPIOI
#define S2_R_Pin GPIO_PIN_1
#define S2_R_GPIO_Port GPIOI
#define S2_G_Pin GPIO_PIN_2
#define S2_G_GPIO_Port GPIOI
#define SENSOR_CAL_KEY_Pin GPIO_PIN_3
#define SENSOR_CAL_KEY_GPIO_Port GPIOI
#define S1_R_Pin GPIO_PIN_3
#define S1_R_GPIO_Port GPIOD
#define S1_G_Pin GPIO_PIN_4
#define S1_G_GPIO_Port GPIOD
#define L506_UART2_TX_Pin GPIO_PIN_5
#define L506_UART2_TX_GPIO_Port GPIOD
#define L506_UART2_RX_Pin GPIO_PIN_6
#define L506_UART2_RX_GPIO_Port GPIOD
#define SYS_R_Pin GPIO_PIN_9
#define SYS_R_GPIO_Port GPIOG
#define SYS_G_Pin GPIO_PIN_10
#define SYS_G_GPIO_Port GPIOG
#define L506_LOGIC_OE_Pin GPIO_PIN_15
#define L506_LOGIC_OE_GPIO_Port GPIOG
#define L506_PWR_KEY_Pin GPIO_PIN_3
#define L506_PWR_KEY_GPIO_Port GPIOB
#define L506_RESET_Pin GPIO_PIN_4
#define L506_RESET_GPIO_Port GPIOB
#define L506_NETLIGHT_Pin GPIO_PIN_5
#define L506_NETLIGHT_GPIO_Port GPIOB
#define L506_WAKEUP_Pin GPIO_PIN_6
#define L506_WAKEUP_GPIO_Port GPIOB
#define L506_STATUS_Pin GPIO_PIN_7
#define L506_STATUS_GPIO_Port GPIOB
#define UART8_485_CON_Pin GPIO_PIN_9
#define UART8_485_CON_GPIO_Port GPIOB
#define RF1_LED_CRC_ERROR_Pin GPIO_PIN_4
#define RF1_LED_CRC_ERROR_GPIO_Port GPIOI
#define RF1_LED_1_Pin GPIO_PIN_5
#define RF1_LED_1_GPIO_Port GPIOI
#define RF2_LED_CRC_ERROR_Pin GPIO_PIN_6
#define RF2_LED_CRC_ERROR_GPIO_Port GPIOI
#define RF2_LED_2_Pin GPIO_PIN_7
#define RF2_LED_2_GPIO_Port GPIOI
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
