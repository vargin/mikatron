#include <avr/io.h>
#include <stdlib.h>
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

void debug(const char *str) {
  while (*str) {
    TxByte(*str++);
  }
}

char numberString[10];
void debugNumber(uint32_t number) {
  ltoa(number, numberString, 10);
  debug(numberString);
}

void enablePowerDownMode() {
  debug("[app] going to power down...");

  Speaker::play(MELODY_DOUBLE_BEEP);

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

void setupAlarms() {
  cli();

  Clock::init();
  debug("[clock] initialization started.");

  // First print current time (for debug purposes).
  ClockTime time = Clock::getTime();

  debug("[clock] current time is ");
  debugNumber(time.hour());
  debug(":");
  debugNumber(time.minute());
  debug(":");
  debugNumber(time.second());

  // Delete all previously saved alarms, to make sure we have the latest ones.
  Clock::clearAlarms();
  debug("[clock] all cleared");

  Clock::addAlarm(ClockTime(time.hour(), time.minute() + 1, 0));
  debug("[clock] all alarms recorded");

  ClockTime alarm = Clock::getAlarm();

  debug("[clock] current alarm is ");
  debugNumber(alarm.hour());
  debug(":");
  debugNumber(alarm.minute());
  debug(":");
  debugNumber(alarm.second());

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

  while (1) {
    if (interrupt) {
      debug("[clock] alarm triggered!");
      interrupt = false;

      Speaker::play(MELODY_ALARM);
      Speaker::play(MELODY_ALARM);
      Speaker::play(MELODY_ALARM);
    }
  }
}