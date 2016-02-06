/*
 * Spanish wordclock using the following grid of letters:
 *
 * S E I S C U N A
 * D I E N U E V E
 * O S A T R E S Z
 * C I N C O C H O
 * E . Y M E N O S
 * D V E I N T E Z
 * M E Q U I N C E
 * D I A C I N C O
 */

// Bitmasks for the hour.
const byte HORAS[][8] = {
  { 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // una
  { 0x00, 0x80, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00 }, // dos
  { 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x08, 0x08, 0x38, 0x08, 0x00, 0x00, 0x00, 0x00 },
  { 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00 },
  { 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x80, 0x60, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00 },
  { 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x00, 0xE0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x00, 0x00, 0x84, 0x30, 0x00, 0x00, 0x00, 0x00 },
  { 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00 }  // doce
};

// Bitmasks for the conjunction, "y" or "menos".
const byte CONJ[][8] = {
  { 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00 }, // y
  { 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00 }  // menos
};

// Bitmasks for the closest minute.
const byte MINUTOS[][8] = {
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F }, // cinco
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x93, 0x00, 0x00 }, // diez
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00 }, // quince
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00 }, // veinte
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x08, 0x1F }, // venticinco
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0 }  // media
};

#define UNA 0
#define DOS 1
#define TRES 2
#define CUATRO 3
#define CINCO 4
#define SEIS 5
#define SIETE 6
#define OCHO 7
#define NUEVE 8
#define DIEZ 9
#define ONCE 10
#define DOCE 11

#define Y 0
#define MENOS 1

#define CINCO_M 0
#define DIEZ_M 1
#define QUINCE_M 2
#define VEINTE_M 3
#define VEINTICINCO_M 4
#define MEDIA 5

#define SECUNDO_ROW 4
#define SECUNDO_COL 6

/*
 * The letters are illuminated by an 8x8 grid of LEDs. Each row and column
 * is addressable by a 4051 demultiplexer. The 4051 maps a 3-bit control value
 * to one of its 8 pins, so 6 pins altogether are required on the Arduino to
 * address all 64 LEDs.
 *
 * 4051 pinout:
 *
 *      y4  1   16   Vcc
 *      y6  2   15   y2
 *       z  3   14   y1
 *      y7  4   13   y0
 *      y5  5   12   y3
 *  Enable  6   11   s0
 *     Vee  7   10   s1
 *     Gnd  8    9   s2
 *
 * To address the output pins:
 *
 *   s2   s1   s0    y
 * --------------------
 *    0    0    0    0
 *    0    0    1    1
 *    0    1    0    2
 *    ...
 *    1    1    1    7
 *
 * Z is common input/output. Our columns are +5V, our rows are Gnd.
 *
 * Set Enable LOW to enable output. Set it high to disable.
 */

// 3 bits to select the column (positive voltage).
// Map 4051 pins S0, S1, S2 to arduino digital pins 2, 3, 4.
#define S0_COL 2
#define S1_COL 3
#define S2_COL 4

// 3 bits to select the row (negative voltage).
// Map 4051 pins S0, S1, S2 to arduino digital pins 5, 6, 7.
#define S0_ROW 5
#define S1_ROW 6
#define S2_ROW 7

// Set pin 12 HIGH to disable, LOW to enable.
// Connect to pin E on both 4051s.
#define ENABLE 12

// Default pins used by the Wire library:
// DS1037 SDA <-> A4 (Arduino)
// DS1037 SCL <-> A5

