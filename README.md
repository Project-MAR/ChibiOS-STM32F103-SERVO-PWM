# ChibiOS-STM32F103-SERVO-PWM

[Demo video](https://youtu.be/GgNfk8-xC3k)
 - Create 5 example threads
 - Using 1 external pin interrupt for decode encoder switch (in Thread4)
 - Using Hardware PWM frequency 50 Hz to drive servo motor 
 -1 mailbox to communicate between Thread3 and Thread4 
 
## Thread4
 - Always blocking, Waiting for interrupt signal from falling edge of GPIOB25.
 - When interrupt come, it check GPIOB14 to determine the direction of a rotary encoder.
 - Then it send the direction to Thread3 vai a mailbox.
 
## Thread3
 - Always blocking, Waiting for mailbox from Thread4.
 - When receive the direction of a rotary encoder vai a mailbox, it drive a servo motor.

## Other Threads
 - Just for testing and famillar with ChibiOS/RT.
 
## Hardware: STM32F103RTC6
<p align="center">
  <img src="https://github.com/Project-MAR/ChibiOS-STM32F103-SERVO-PWM/blob/main/img/STM32F103RCT6.jpg" width="500" height="600">
</p>

## Pinout
<p align="center">
  <img src="https://github.com/Project-MAR/ChibiOS-STM32F103-SERVO-PWM/blob/main/img/pinout.JPG">
</p>

## Boot Bode
- BOOT0: No.2 on Switch or Mark 0 on PCB
- BOOT1: No.1 on Switch or Mark 1 on PCB, BOOT1 is connected to PB2 
- To used serial programming. BOOT0 is UP and BOOT1 is DOWN
- Serial config: 115200 8E1
<p align="center">
  <img src="https://github.com/Project-MAR/ChibiOS-STM32F103-SERVO-PWM/blob/main/img/serial_boot.jpg">
</p>

## Hardware: STM32F103RTC6
