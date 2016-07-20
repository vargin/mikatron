#ifndef MIKATRON_CONFIG_H
#define MIKATRON_CONFIG_H

#include <avr/io.h>

/**
 * UART Rx/Tx Layout:
 *            D1
 * AVR ----+--|>|-----+----- Tx
 *         |      10K $ R1
 *         +--------(/^\)--- Rx
 *              NPN E   C
 */

// PB0 (pin 5) - I2C SDA.
#define I2C_SDA_PIN DDB0
// PB1 (pin 6) - PWM (speaker) + UART Rx/Tx.
#define SPEAKER_UART_PIN DDB1
// PB2 (pin 7) - I2C SCL.
#define I2C_SCL_PIN DDB2
// PB3 (pin 2) - Alarm Interruption Pin.
#define ALARM_INTERRUPTION_PIN DDB3
// PB4 (pin 3) - Snooze Button Pin.
#define SNOOZE_BUTTON_PIN DDB4

#endif //MIKATRON_CONFIG_H
