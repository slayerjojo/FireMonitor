/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SEGGER_RTT.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

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
#define WATCHDOG_Pin GPIO_PIN_0
#define WATCHDOG_GPIO_Port GPIOC
#define SENSOR_ADC_Pin GPIO_PIN_1
#define SENSOR_ADC_GPIO_Port GPIOC
#define FLASH_CS_Pin GPIO_PIN_4
#define FLASH_CS_GPIO_Port GPIOA
#define FLASH_SCK_Pin GPIO_PIN_5
#define FLASH_SCK_GPIO_Port GPIOA
#define FLASH_MISO_Pin GPIO_PIN_6
#define FLASH_MISO_GPIO_Port GPIOA
#define FLASH_MOSI_Pin GPIO_PIN_7
#define FLASH_MOSI_GPIO_Port GPIOA
#define KEY_UP_Pin GPIO_PIN_9
#define KEY_UP_GPIO_Port GPIOE
#define KEY_LEFT_Pin GPIO_PIN_10
#define KEY_LEFT_GPIO_Port GPIOE
#define KEY_DOWN_Pin GPIO_PIN_11
#define KEY_DOWN_GPIO_Port GPIOE
#define KEY_RIGHT_Pin GPIO_PIN_12
#define KEY_RIGHT_GPIO_Port GPIOE
#define EEPROM_WP_Pin GPIO_PIN_15
#define EEPROM_WP_GPIO_Port GPIOE
#define EEPROM_SCL_Pin GPIO_PIN_10
#define EEPROM_SCL_GPIO_Port GPIOB
#define EEPROM_SDA_Pin GPIO_PIN_11
#define EEPROM_SDA_GPIO_Port GPIOB
#define DAC_CS_Pin GPIO_PIN_12
#define DAC_CS_GPIO_Port GPIOB
#define DAC_SCK_Pin GPIO_PIN_13
#define DAC_SCK_GPIO_Port GPIOB
#define DAC_MOSI_Pin GPIO_PIN_15
#define DAC_MOSI_GPIO_Port GPIOB
#define DI2_Pin GPIO_PIN_10
#define DI2_GPIO_Port GPIOD
#define DI1_Pin GPIO_PIN_11
#define DI1_GPIO_Port GPIOD
#define RELAY_SAFE_Pin GPIO_PIN_12
#define RELAY_SAFE_GPIO_Port GPIOD
#define RELAY_FRAME_Pin GPIO_PIN_13
#define RELAY_FRAME_GPIO_Port GPIOD
#define LED1_R_Pin GPIO_PIN_14
#define LED1_R_GPIO_Port GPIOD
#define LED1_G_Pin GPIO_PIN_15
#define LED1_G_GPIO_Port GPIOD
#define LED2_R_Pin GPIO_PIN_6
#define LED2_R_GPIO_Port GPIOC
#define LED2_G_Pin GPIO_PIN_7
#define LED2_G_GPIO_Port GPIOC
#define RELAY_FRAME_FB_Pin GPIO_PIN_8
#define RELAY_FRAME_FB_GPIO_Port GPIOC
#define RELAY_SAFE_FB_Pin GPIO_PIN_9
#define RELAY_SAFE_FB_GPIO_Port GPIOC
#define MODBUS_TX_Pin GPIO_PIN_9
#define MODBUS_TX_GPIO_Port GPIOA
#define MODBUS_RX_Pin GPIO_PIN_10
#define MODBUS_RX_GPIO_Port GPIOA
#define MODBUS_PV_Pin GPIO_PIN_11
#define MODBUS_PV_GPIO_Port GPIOA
#define LED_SEGA_Pin GPIO_PIN_10
#define LED_SEGA_GPIO_Port GPIOC
#define LED_SEGB_Pin GPIO_PIN_11
#define LED_SEGB_GPIO_Port GPIOC
#define LED_SEGC_Pin GPIO_PIN_12
#define LED_SEGC_GPIO_Port GPIOC
#define LED_SEGD_Pin GPIO_PIN_0
#define LED_SEGD_GPIO_Port GPIOD
#define LED_SEGE_Pin GPIO_PIN_1
#define LED_SEGE_GPIO_Port GPIOD
#define LED_SEGF_Pin GPIO_PIN_2
#define LED_SEGF_GPIO_Port GPIOD
#define LED_SEGG_Pin GPIO_PIN_3
#define LED_SEGG_GPIO_Port GPIOD
#define LED_SEGDP_Pin GPIO_PIN_4
#define LED_SEGDP_GPIO_Port GPIOD
#define LED_SEL1_Pin GPIO_PIN_5
#define LED_SEL1_GPIO_Port GPIOD
#define LED_SEL2_Pin GPIO_PIN_6
#define LED_SEL2_GPIO_Port GPIOD
#define LED_SEL3_Pin GPIO_PIN_7
#define LED_SEL3_GPIO_Port GPIOD
#define TEMP_SCL_Pin GPIO_PIN_6
#define TEMP_SCL_GPIO_Port GPIOB
#define TEMP_SDA_Pin GPIO_PIN_7
#define TEMP_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define FW_VERSION "B.1"

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
