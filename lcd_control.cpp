#include "lcd_control.h"

LiquidCrystal lcd(8, 7, 6, 5, 4, 3, 2);

void lcd_control_init() {
  byte charScrollDots[8] = {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b10101,
    0b00000
  };
  
  byte charDelta[8] = {
    0b00000,
    0b00000,
    0b00100,
    0b01010,
    0b10001,
    0b11111,
    0b00000,
    0b00000
  };

  lcd.begin(LCD_COLS, LCD_ROWS);
  
  lcd.createChar(LCD_CHAR_SCROLLDOTS, charScrollDots);
  lcd.createChar(LCD_CHAR_DELTA, charDelta);
  
  lcd.print("CDP Sniffino "VERSION_STR);
  lcd.setCursor(0, 1);
  lcd.print("Initializing");
  lcd.write(LCD_CHAR_SCROLLDOTS);
  
  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, HIGH); // backlight on
}


void lcd_control_update() {
  unsigned int delta_t = lcd_delta_t;
  unsigned int ttl = lcd_ttl;
  unsigned int i;
  
  lcd.setCursor(0, 0);
  
  lcd.write(LCD_CHAR_DELTA);
  //lcd.print("t ");
  if(delta_t < 10) lcd.print(' ');
  if(delta_t < 100) lcd.print(' ');
  lcd.print(delta_t);
  
  lcd.print("/");
  if(ttl < 10) lcd.print(' ');
  if(ttl < 100) lcd.print(' ');
  lcd.print(ttl);
  
  lcd.print(" @ ");
  lcd.print(cdp_packets_received);
  if(cdp_packets_received < 10) lcd.print(' ');
  if(cdp_packets_received < 100) lcd.print(' ');

  lcd.setCursor(0, 1);
  menu_item* curr = &menu[menu_current];
  char lcd_data[21]; // 20 + \0

  if(curr != NULL && curr->visible) {
    size_t label_length = strlen(curr->label);
    strncpy(lcd_data, curr->label, 20);
    i = label_length;
    lcd_data[i++] = ':';
    lcd_data[i++] = ' ';

    if(lcd_more_offset > 0) lcd_data[i++] = LCD_CHAR_SCROLLDOTS;
    size_t value_length = strlen(curr->value);
    if((value_length - lcd_more_offset) > (20 - i)) {
      value_length = 20 - i - 1;
      lcd_data[19] = LCD_CHAR_SCROLLDOTS;
    } else {
      value_length -= lcd_more_offset;
    }
    strncpy(&lcd_data[i], &curr->value[lcd_more_offset], value_length);
  } else {
    for(i=0; i<20; ++i) lcd_data[i] = ' ';
  }

  lcd_data[20] = '\0';
  boolean complete = 0;
  for(i = 0; i<20; ++i) {
    if(lcd_data[i] == '\0') complete = 1;
    if(complete)
      lcd.write((uint8_t)' ');
    else
      lcd.write(lcd_data[i]);
  }
//  printf("|%s|\n", lcd_data);
}
