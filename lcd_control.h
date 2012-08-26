#ifndef _LCD_CONTROL_H_
#define _LCD_CONTROL_H_

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pins.h"
#include "lcd_info.h"

/////////////////////////

#define LCD_CHAR_MORE_LEFT 0xae
#define LCD_CHAR_MORE_RIGHT 0xaf
#define LCD_CHAR_DELTA 0x7f

extern Adafruit_SSD1306 display;

void lcd_control_init();
void lcd_control_update();

#endif
