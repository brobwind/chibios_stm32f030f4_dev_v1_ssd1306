/*
 * Copyright (C) 2016 https://www.brobwind.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "ssd1306.h"


static mutex_t logMtx;
int broprintf(const char *fmt, ...) {
  va_list ap;
  int formatted_bytes;

  chMtxLock(&logMtx);
  va_start(ap, fmt);
  formatted_bytes = chvprintf((BaseSequentialStream *)&SD1, fmt, ap);
  va_end(ap);
  chMtxUnlock(&logMtx);

  return formatted_bytes;
}

/*===========================================================================*/
/* OLED display                                                              */
/*===========================================================================*/

static const I2CConfig i2ccfg = { // I2CCLK=48MHz, SCL=~100kHz
#if 0 // 100kHz @ 48MHz
  STM32_TIMINGR_PRESC(0x0B)  |
  STM32_TIMINGR_SCLDEL(0x04) | STM32_TIMINGR_SDADEL(0x02) |
  STM32_TIMINGR_SCLH(0x0F)   | STM32_TIMINGR_SCLL(0x13),
#else // 400kHz @ 48Mhz
  STM32_TIMINGR_PRESC(0x05)  |
  STM32_TIMINGR_SCLDEL(0x03) | STM32_TIMINGR_SDADEL(0x03) |
  STM32_TIMINGR_SCLH(0x03)   | STM32_TIMINGR_SCLL(0x09),
#endif
  0,
  0
};

static const SSD1306Config ssd1306cfg = {
  &I2CD1,
  &i2ccfg,
  SSD1306_SAD_0X78,
};

static SSD1306Driver SSD1306D1;

static void __attribute__((unused)) delayUs(uint32_t val) {
  (void)val;
}

static void __attribute__((unused)) delayMs(uint32_t val) {
  chThdSleepMilliseconds(val);
}

static THD_WORKING_AREA(waOledDisplay, 512);
static __attribute__((noreturn)) THD_FUNCTION(OledDisplay, arg) {
  (void)arg;

  chRegSetThreadName("OledDisplay");

  ssd1306ObjectInit(&SSD1306D1);
  ssd1306Start(&SSD1306D1, &ssd1306cfg);

  ssd1306FillScreen(&SSD1306D1, 0x00);

  while (TRUE) {
	ssd1306GotoXy(&SSD1306D1, 0, 32);
	ssd1306Puts(&SSD1306D1, "Hello, world!", &ssd1306_font_7x10, SSD1306_COLOR_WHITE);

	ssd1306UpdateScreen(&SSD1306D1);

    chThdSleepMilliseconds(30);
  }

  ssd1306Stop(&SSD1306D1);
}

/*===========================================================================*/
/* LED blinker                                                               */
/*===========================================================================*/

static THD_WORKING_AREA(waLedBlinker, 24);
static THD_FUNCTION(LedBlinker, arg) {
  (void)arg;
  chRegSetThreadName("LedBlinker");

  while (TRUE) {
    /* LED on */
    palClearPad(GPIOA, GPIOA_LED_GREEN);
    chThdSleepMilliseconds(1);

    /* LED off */
    palSetPad(GPIOA, GPIOA_LED_GREEN);
    chThdSleepMilliseconds(999);
  }
}

/*
 * Application entry point.
 */
int __attribute__((noreturn)) main(void)
{
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * I2CD1 I/O pins setup.(It bypasses board.h configurations)
   */
  palSetPadMode(GPIOA, GPIOA_PIN9, PAL_MODE_ALTERNATE(4) | PAL_STM32_OSPEED_HIGHEST);   /* SCL */
  palSetPadMode(GPIOA, GPIOA_PIN10, PAL_MODE_ALTERNATE(4) | PAL_STM32_OSPEED_HIGHEST);  /* SDA */

  chMtxObjectInit(&logMtx);

  /*
   * Activates the serial driver 1 using the driver default configuration.
   */
  sdStart(&SD1, NULL);

 /*
   * Creates the LED blinker thread.
   */
  chThdCreateStatic(waLedBlinker, sizeof(waLedBlinker), NORMALPRIO, LedBlinker, NULL);

  /*
   * Creates the OLED display thread.
   */
  chThdCreateStatic(waOledDisplay, sizeof(waOledDisplay), NORMALPRIO, OledDisplay, NULL);

  while(TRUE){
    chThdSleepMilliseconds(500);
  }
}
