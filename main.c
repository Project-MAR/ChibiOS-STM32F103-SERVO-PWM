/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "rt_test_root.h"
#include "oslib_test_root.h"

static PWMConfig servo_pwm_config = {
  1000000,                                    /* 1MHz PWM clock frequency.    */
  20000,                                      /* PWM period 20 ms(50Hz).      */
  NULL,                                       /* Period callback.             */
  {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},            /* CH1 mode and callback.       */
   {PWM_OUTPUT_DISABLED, NULL},               /* CH2 mode and callback.       */
   {PWM_OUTPUT_DISABLED, NULL},               /* CH3 mode and callback.       */
   {PWM_OUTPUT_DISABLED, NULL}                /* CH4 mode and callback.       */
  },
  0,                                         /* Control Register 2.            */
  0                                          /* DMA/Interrupt Enable Register. */
};

#define ENCODEINC       (msg_t)1
#define ENCODEDES       (msg_t)2

static mailbox_t mb;
static msg_t msgServoCmd;

/*
 * Green LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palClearPad(GPIOC, LED_BUILTIN);
    chThdSleepMilliseconds(500);
    palSetPad(GPIOC, LED_BUILTIN);
    chThdSleepMilliseconds(500);
  }
}

static THD_WORKING_AREA(waThread5, 128);
static THD_FUNCTION(Thread5, arg) {

  (void)arg;
  chRegSetThreadName("sos_signal");
  while (true) {
    if (!palReadPad(GPIOB, SWBOOT1)) {
      palClearPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(200);
      palSetPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(200);
      palClearPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(200);
      palSetPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(200);
      palClearPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(200);
      palSetPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(500);

      palClearPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(500);
      palSetPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(500);
      palClearPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(500);
      palSetPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(500);
      palClearPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(500);
      palSetPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(500);

      palClearPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(200);
      palSetPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(200);
      palClearPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(200);
      palSetPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(200);
      palClearPad(GPIOC, GPIOC_PIN6);
      chThdSleepMilliseconds(200);
      palSetPad(GPIOC, GPIOC_PIN6);

      chThdSleepMilliseconds(1000);
    }else{
      chThdSleepMilliseconds(100);
    }
  }
}

/*
 * PB2 on switch BOOT1
 */
static THD_WORKING_AREA(waThread2, 128);
static THD_FUNCTION(Thread2, arg) {

  (void)arg;
  chRegSetThreadName("swBOOT1");
  while (true) {
    if (!palReadPad(GPIOB, SWBOOT1)) {
      //palClearPad(GPIOC, GPIOC_PIN6);
    }else{
      //palSetPad(GPIOC, GPIOC_PIN6);
    }
    chThdSleepMilliseconds(10);
  }
}

/*
 * PA8 on PWM MODE
 */
static THD_WORKING_AREA(waThread3, 128);
static THD_FUNCTION(Thread3, arg) {
  (void)arg;
  msg_t msg;
  int pwmVal = 0;
  chRegSetThreadName("pwm");
  pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 300));

  while (true) {
    /* Waiting for a message.*/
    chMBFetchTimeout(&mb, &msg, TIME_INFINITE);
    if(msg == ENCODEINC)
      pwmVal += 50;
    else
      pwmVal -= 50;

    if(pwmVal > 1150)
      pwmVal = 1150;
    else if(pwmVal < 350)
      pwmVal = 350;

    pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, pwmVal));
    chThdSleepMilliseconds(50);
  }
}

/*
 * Rotary Encoder
 * PB14 -> GA
 * PB15 -> GB
 */
static THD_WORKING_AREA(waThread4, 128);
static THD_FUNCTION(Thread4, arg) {
  (void)arg;
  msg_t msg;

  chRegSetThreadName("rotary_encoder");

  palEnablePadEvent(GPIOB, 15, PAL_EVENT_MODE_FALLING_EDGE);
  while (true) {
    palWaitPadTimeout(GPIOB, 15, TIME_INFINITE);
    if(palReadPad(GPIOB, 14) == PAL_HIGH) {
        // Value is decreasing
        palTogglePad(GPIOC, 8);
        msg = chMBPostTimeout(&mb, ENCODEDES, TIME_IMMEDIATE);
        if (msg != MSG_OK) {
          // TODO:
        }
    }else {
      // Value is increasing
      palTogglePad(GPIOC, 9);
      msg = chMBPostTimeout(&mb, ENCODEINC, TIME_IMMEDIATE);
      if (msg != MSG_OK) {
        // TODO:
      }
    }
    chThdSleepMilliseconds(10);
  }
}

/*
 * Application entry point.
 */
int main(void) {
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();
  chMBObjectInit(&mb, &msgServoCmd, 1);

  palSetPadMode(GPIOA, 8, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  pwmStart(&PWMD1, &servo_pwm_config);

  /*
   * Activates the serial driver 2 using the driver default configuration.
   */
  //sdStart(&SD2, NULL);

  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);
  chThdCreateStatic(waThread3, sizeof(waThread3), HIGHPRIO,   Thread3, NULL);
  chThdCreateStatic(waThread4, sizeof(waThread4), HIGHPRIO,   Thread4, NULL);
  chThdCreateStatic(waThread5, sizeof(waThread5), LOWPRIO,    Thread5, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (true) {
    palSetPad(GPIOC, 7);
    chThdSleepMilliseconds(5000);
    palClearPad(GPIOC, 7);
    chThdSleepMilliseconds(5000);
  }
}
