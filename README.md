
### A SSD1306 OLED display I2C driver based on STM32F030F4P6 MCU

#### Hardware requirements:
###### STM32F030F4-DEV V1.0

| Device | configuration            |
| ------ | ------------------------ |
| MCU    | STM32F030F4P6            |
| HSE    | 8MHz                     |
| USART1 | TX - PA2, RX - PA3       |
| I2C1   | SCL - PA9, SDA - PA10    |
| LED    | PA4                      |

#### Software
Based on ChibiOS/RT: http://www.chibios.org/dokuwiki/doku.php
