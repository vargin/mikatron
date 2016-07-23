#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "speaker.h"
#include "clock.h"

#define DEBUG_ENABLED false

#if(DEBUG_ENABLED)
#include "uart.h"
#endif

volatile bool interrupt = false;

ISR(PCINT0_vect) {
  interrupt = true;
}

void debug(const char *str, bool newLine = true) {
  #if(DEBUG_ENABLED)
  while (*str) {
    TxByte(*str++);
  }

  if (newLine) {
    TxByte('\n');
  }
  #endif
}

void enablePowerDownMode() {
  Speaker::play(MELODY_DOUBLE_BEEP);

  debug("[app] going to power down...");

  PORTB &= ~_BV(PB1);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
  sei();

  PORTB |= _BV(PB1);

  debug("[app] waken up!");
}

void printAlarm() {
  debug("[clock] next alarm is ", false);
  debug(Clock::getAlarm().toString());
}

void printTime() {
  debug("[clock] current time is ", false);
  debug(Clock::getTime().toString());
}

void setupAlarms() {
  cli();

  Clock::init();
  debug("[clock] initialization started.");

  // First print current time (for debug purposes).
  printTime();

  // Delete all previously saved alarms, to make sure we have the latest ones.
  Clock::clearAlarms();
  debug("[clock] all cleared");

  ClockTime time = Clock::getTime();
  Clock::addAlarm(ClockTime(time.hour(), time.minute() + 1, 0));
  Clock::addAlarm(ClockTime(time.hour(), time.minute() + 2, 0));
  Clock::addAlarm(ClockTime(time.hour(), time.minute() + 3, 0));
  debug("[clock] all alarms recorded");

  printAlarm();

  sei();
}

/**
 * GPIO Layout:
 *  - PB0 (pin 5) - I2C SDA;
 *  - PB1 (pin 6) - PWM (speaker) + UART Rx/Tx;
 *  - PB2 (pin 7) - I2C SCL;
 *  - PB3 (pin 2) - Alarm Interruption Pin;
 *  - PB4 (pin 3) - Snooze Button Pin.
 *
 * UART Rx/Tx Layout:
 *            D1
 * AVR ----+--|>|-----+----- Tx
 *         |      10K $ R1
 *         +--------(/^\)--- Rx
 *              NPN E   C
 */
int main(void) {
  // Setup outputs. Set port to HIGH to signify UART default condition.
  DDRB |= _BV(DDB1);
  PORTB |= _BV(PB1);

  // Setup inputs.
  DDRB &= ~(_BV(DDB3) | _BV(DDB4));

  // Enable Alarm interruption that comes from RTC module.
  PCMSK |= _BV(PCINT3);
  GIMSK |= _BV(PCIE);

  setupAlarms();

  debug("[app] all setup.");

  Speaker::play(MELODY_BEEP);

  interrupt = false;

  while (1) {
    if (interrupt) {
      debug("[clock] alarm triggered!");

      uint8_t snoozePressed = 0;
      uint8_t snoozePressTime = 0;
      while(true) {
        Speaker::play(MELODY_ALARM);

        if (!snoozePressed) {
          snoozePressed = PINB & _BV(PINB4);
        } else if (snoozePressTime > 3) {
          break;
        } else {
          snoozePressTime += 1;
        }

        _delay_ms(100);
      }

      debug("[clock] scheduling a new alarm.");
      Clock::setAlarm(Alarm1Type::MATCH_HOURS, Clock::getNextAlarm());

      printAlarm();

      interrupt = false;
    }

    enablePowerDownMode();
  }
}