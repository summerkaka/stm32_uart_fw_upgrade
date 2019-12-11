/**
  ******************************************************************************
  * @file    Project/inc/include.h
  * @author  xiatian
  * @version V0.00
  * @date    2019-12-03
  * @brief   include.h
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INCLUDE_H
#define __INCLUDE_H


/* Includes ------------------------------------------------------------------*/
// std lib
#include "stm32f3xx_hal.h"
#include "main.h"
#include "math.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "stdint.h"

// bsp
#include "gpio.h"
#include "stm32f3xx_it.h"
#include "sys.h"
#include "usart.h"

// app
#include "command.h"



/* Exported macro ------------------------------------------------------------*/
// #define DEBUG                   1
#define APP_ADDRESS             0x08008000
#define FLASH_USER_START_ADDR   APP_ADDRESS                                     /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     0x0803FFFF                                      /* End @ of user Flash area */


/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported variables ------------------------------------------------------- */



/* Exported functions ------------------------------------------------------- */




#endif /*__INCLUDE_H */

