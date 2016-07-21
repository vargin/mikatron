#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "config.h"
#include "uart.h"
#include "speaker.h"
#include "clock.h"

volatile bool interrupt = false;

ISR(PCINT0_vect) {
  interrupt = true;
}

void debug(const char *str, bool newLine = true) {
  while (*str) {
    TxByte(*str++);
  }

  if (newLine) {
    TxByte('\n');
  }
}

void enablePowerDownMode() {
  Speaker::play(MELODY_DOUBLE_BEEP);

  debug("[app] going to power down...");

  PORTB &= ~_BV(SPEAKER_UART_PIN);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
  sei();

  PORTB |= _BV(SPEAKER_UART_PIN);

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

int main(void) {
  // Setup outputs. Set port to HIGH to signify UART default condition.
  DDRB |= _BV(SPEAKER_UART_PIN);
  PORTB |= _BV(SPEAKER_UART_PIN);

  // Setup inputs.
  DDRB &= ~(_BV(ALARM_INTERRUPTION_PIN) | _BV(SNOOZE_BUTTON_PIN));

  // Enable Alarm interruption that comes from RTC module.
  PCMSK |= _BV(PCINT3);
  GIMSK |= _BV(PCIE);

  setupAlarms();

  debug("[app] all setup.");

  Speaker::play(MELODY_MODE);

  interrupt = false;

  while (1) {
    if (interrupt) {
      debug("[clock] alarm triggered!");

      for (uint8_t i = 0; i < 5; i++) {
        Speaker::play(MELODY_ALARM);
        _delay_ms(200);
      }

      debug("[clock] scheduling a new alarm.");
      Clock::setAlarm(Alarm1Type::MATCH_HOURS, Clock::getNextAlarm());

      printAlarm();

      interrupt = false;
    }

    enablePowerDownMode();
  }
}