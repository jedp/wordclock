/*
 * Spanish word clock.
 */

#include <Wire.h>
#include "RTClib.h"
#include "wordclock.h"

// We keep accurate time with the DS1307 RTC.
RTC_DS1307 RTC;

// Every second, we will blink a light on and off.
bool blinking = false;
int ticks = 0;
int debounceTick = ticks;

// The current matrix of lights on the display.
byte pattern[8];
int i = 0;
int j = 0;

void setup() {
  pinMode(S0_COL, OUTPUT);
  pinMode(S1_COL, OUTPUT);
  pinMode(S2_COL, OUTPUT);
  pinMode(S0_ROW, OUTPUT);
  pinMode(S1_ROW, OUTPUT);
  pinMode(S2_ROW, OUTPUT);

  // Enable pin governs both 4501s.
  pinMode(ENABLE, OUTPUT);

  // Buttons to manually set time.
  pinMode(INCR_HORA, INPUT_PULLUP);
  pinMode(INCR_CINCO_MINUTOS, INPUT_PULLUP);

  Wire.begin();
  RTC.begin();

  if (!RTC.isrunning()) {
    // If necessary, set the clock to the time this sketch was compiled.
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  setTimer1Interrupt2Hz();

  testSequence();

  updateClockPattern();
}

void loop() {
  blinkSeconds();

  displayPattern();

  checkForClockSet();

  // Check the RTC every 21 seconds. (Ticks are at 2Hz frequency.)
  if (ticks > 42) {
    ticks = 0;
    updateClockPattern();
  }
}

void blinkSeconds() {
  if (blinking) {
    setLed(SECUNDO_ROW, SECUNDO_COL);
  }
}

void displayPattern() {
  for (i = 0; i < 8; i++) {
    byte row = pattern[i];
    for (j = 0; j < 8; j++) {
      if ((row >> j) & 0x01) {
        setLed(i, j);
        // 1ms delay for each LED lets it glow brighter.
        delay(1);
      }
    }
  }
}

void updateClockPattern() {
  DateTime now = RTC.now();

  int hour = now.hour();
  int minute = now.minute();

  // It's a 24-hour clock.
  if (hour > 12) hour -= 12;

  int minuto;
  int hora;
  int conj;
  boolean en_punto = false;

  if (minute >= 35) {
    conj = MENOS;
    minute = 65 - minute;
    hour += 1;
    if (hour > 12) hour = 1;
  } else {
    conj = Y;
  }

  hora = hour - 1;

  if (minute >= 30) {
    minuto = MEDIA;
  } else if (minute >= 25) {
    minuto = VEINTICINCO_M;
  } else if (minute >= 20) {
    minuto = VEINTE_M;
  } else if (minute >= 15) {
    minuto = QUINCE_M;
  } else if (minute >= 10) {
    minuto = DIEZ_M;
  } else if (minute >= 5) {
    minuto = CINCO_M;
  } else {
    en_punto = true;
  }

  for (i = 0; i < 8; i++) {
    if (en_punto) {
      pattern[i] = HORAS[hora][i];
    } else {
      pattern[i] = HORAS[hora][i] | CONJ[conj][i] | MINUTOS[minuto][i];
    }
  }
}

/** Respond to button presses to set the clock. */
void checkForClockSet() {
  // Half-second debounce.
  if (debounceTick == ticks) return;

  if (digitalRead(INCR_HORA) == LOW) {
    // Press INCR_HORA button to increment by 3600s (one hour).
    RTC.adjust(RTC.now() + 3600);
    updateClockPattern();
    debounceTick = ticks;
  }

  if (digitalRead(INCR_CINCO_MINUTOS) == LOW) {
    // Press INCR_CINCO_MINUTOS button to increment by 300s (5 min).
    RTC.adjust(RTC.now() + 300);
    updateClockPattern();
    debounceTick = ticks;
  }
}

void testSequence() {
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      setLed(i, j);
      delay(30);
    }
  }
}

/*
 * Set the LED at [row, col] HIGH.
 */
void setLed(int row, int col) {
  digitalWrite(ENABLE, HIGH);

  digitalWrite(S0_ROW,  row       & 0x01);
  digitalWrite(S1_ROW, (row >> 1) & 0x01);
  digitalWrite(S2_ROW, (row >> 2) & 0x01);

  digitalWrite(S0_COL,  col       & 0x01);
  digitalWrite(S1_COL, (col >> 1) & 0x01);
  digitalWrite(S2_COL, (col >> 2) & 0x01);

  digitalWrite(ENABLE, LOW);
}

/*
 * Configure Timer1 to interrupt twice per second.
 */
void setTimer1Interrupt2Hz() {
  cli();

  // Set timer1 to interrupt at 2Hz
  TCCR1A = 0;
  // Setting TCCR1A to all 0s does two things:
  // - Disconnect Pin OC1 from Timer/Counter 1.
  // - Disable PWM.
  TCCR1B = 0;
  // Settings TCCR1B to all 0s does the following:
  // - Disable Input Capture Noise Canceler.
  // - Transfer TCNT1 to ICR (Input Capture Reg) ICP pin falling edge.
  // - Stop Timer/Counter 1.
  TCNT1 = 0;  // Initialize counter.
  // Set Output Compare Register to match on 2Hz intervals.
  OCR1A = 7812; // 16MHz / (2*1024).
  // Clear Timer Counter on match.
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 for 1024 prescaler.
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // Enable compare interrupt.
  TIMSK1 |= (1 << OCIE1A);

  sei();
}

ISR(TIMER1_COMPA_vect) {
  ticks++;
  blinking = !blinking;
}
