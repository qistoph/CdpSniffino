#ifndef _LCD_CONTROL_H_
#define _LCD_CONTROL_H_

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "pins.h"
#include "lcd_info.h"

#define LCD_COLS 20
#define LCD_ROWS 2

/////////////////////////

#define LCD_CHAR_SCROLLDOTS 1
#define LCD_CHAR_DELTA 2

// Configure your LCD pins here
// For instructions see http://arduino.cc/en/Tutorial/LiquidCrystal
// and http://arduino.cc/en/Reference/LiquidCrystal
extern LiquidCrystal lcd;

void lcd_control_init();
void lcd_control_update();

#endif
