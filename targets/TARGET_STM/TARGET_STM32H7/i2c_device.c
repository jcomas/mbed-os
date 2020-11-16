/* mbed Microcontroller Library
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************
 *
 * Copyright (c) 2015-2020 STMicroelectronics.
 * Copyright (c) 2020, Arduino SA.
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

 #include "i2c_device.h"
 #include "mbed_assert.h"
 #include "stm32h7xx_ll_rcc.h"
 #include "stm32h7xx_ll_i2c.h"

 #if DEVICE_I2C

#define NSEC_PER_SEC  1000000000
/**
  * @brief  Compute I2C timing according current I2C clock source and required I2C clock.
  * @param  clock_src_freq I2C clock source in Hz.
  * @param  i2c_freq Required I2C clock in Hz.
  * @retval I2C timing or 0 in case of error.
  */
static uint32_t I2C_ComputeTiming(uint32_t clock_src_freq, uint32_t i2c_freq)
{
  uint32_t i2c_hold_time_min, i2c_setup_time_min;
  uint32_t i2c_h_min_time, i2c_l_min_time;
  uint32_t presc = 1U;
  uint32_t timing = 0U;

  switch (i2c_freq) {
    case 100000:
      i2c_h_min_time = 4000U;
      i2c_l_min_time = 4700U;
      i2c_hold_time_min = 500U;
      i2c_setup_time_min = 1250U;
      break;
    case 400000:
      i2c_h_min_time = 600U;
      i2c_l_min_time = 1300U;
      i2c_hold_time_min = 375U;
      i2c_setup_time_min = 500U;
      break;
    case 1000000:
      i2c_h_min_time = 400U;
      i2c_l_min_time = 470U;
      i2c_hold_time_min = 50U;
      i2c_setup_time_min = 125U;
      break;
  }

  /* Calculate period until prescaler matches */
  do {
    uint32_t t_presc = clock_src_freq / presc;
    uint32_t ns_presc = NSEC_PER_SEC / t_presc;
    uint32_t sclh = i2c_h_min_time / ns_presc;
    uint32_t scll = i2c_l_min_time / ns_presc;
    uint32_t sdadel = i2c_hold_time_min / ns_presc;
    uint32_t scldel = i2c_setup_time_min / ns_presc;

    if ((sclh - 1) > 255 ||  (scll - 1) > 255) {
      ++presc;
      continue;
    }

    if (sdadel > 15 || (scldel - 1) > 15) {
      ++presc;
      continue;
    }

    timing = __LL_I2C_CONVERT_TIMINGS(presc - 1,
          scldel - 1, sdadel, sclh - 1, scll - 1);
    break;
  } while (presc < 16);

  return timing;
}

/**
  * @}
  */

/** @defgroup I2C_DEVICE_Exported_Functions I2C_DEVICE Exported Functions
  * @{
  */
/**
 * @brief  Provide the suitable timing depending on requested frequency 
 * @param  hz Required I2C clock in Hz.
 * @retval I2C timing or 0 in case of error.
 */
uint32_t get_i2c_timing(int hz)
{
  uint32_t clock_src_freq;
  uint32_t tim;

  /* we will use D2PCLK1 to calculate I2C timings */
  MBED_ASSERT(RCC_I2C1CLKSOURCE_D2PCLK1 ==__HAL_RCC_GET_I2C1_SOURCE());

  LL_RCC_ClocksTypeDef rcc_clocks;
  LL_RCC_GetSystemClocksFreq(&rcc_clocks);

  tim = I2C_ComputeTiming(rcc_clocks.PCLK1_Frequency, hz);

  return tim;
}
/**
  * @}
  */

#endif // DEVICE_I2C