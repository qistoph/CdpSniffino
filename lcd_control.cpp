#include "lcd_control.h"

Adafruit_SSD1306 display(OLED_RESET);

void lcd_control_init() {
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // init done
  display.setTextWrap(false);
  
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("CDP Sniffino "VERSION_STR);
  display.print("Initializing");
  display.display();
}

#define LCD_LINE_WIDTH 16

void lcd_control_update() {
  unsigned int delta_t = lcd_delta_t;
  unsigned int ttl = lcd_ttl;
  unsigned int i;
  
  display.clearDisplay();
  display.setCursor(0, 0);
  
  display.write(LCD_CHAR_DELTA);
  if(delta_t < 10) display.print(' ');
  if(delta_t < 100) display.print(' ');
  display.print(delta_t);
  
  display.print("/");
  if(ttl < 10) display.print(' ');
  if(ttl < 100) display.print(' ');
  display.print(ttl);
  
  display.print(" \x12 ");
  display.print(cdp_packets_received);
  if(cdp_packets_received < 10) display.print(' ');
  if(cdp_packets_received < 100) display.print(' ');

  display.print("\n"); // next line
  menu_item* curr = &menu[menu_current];
  char lcd_data[LCD_LINE_WIDTH + 1]; // LCD_LINE_WIDTH + \0

  if(curr != NULL && curr->visible) {
    size_t label_length = strlen(curr->label);
    strncpy(lcd_data, curr->label, LCD_LINE_WIDTH);
    i = label_length;
    lcd_data[i++] = ':';
    lcd_data[i++] = ' ';

    if(lcd_more_offset > 0) lcd_data[i++] = LCD_CHAR_MORE_LEFT;
    size_t value_length = strlen(curr->value);
    if((value_length - lcd_more_offset) > (LCD_LINE_WIDTH - i)) {
      value_length = LCD_LINE_WIDTH - i - 1;
      lcd_data[LCD_LINE_WIDTH - 1] = LCD_CHAR_MORE_RIGHT;
    } else {
      value_length -= lcd_more_offset;
    }
    strncpy(&lcd_data[i], &curr->value[lcd_more_offset], value_length);
  } else {
    for(i=0; i<LCD_LINE_WIDTH; ++i) lcd_data[i] = ' ';
  }

  lcd_data[LCD_LINE_WIDTH] = '\0';
  boolean complete = 0;
  for(i = 0; i<LCD_LINE_WIDTH; ++i) {
    if(lcd_data[i] == '\0') complete = 1;
    if(complete)
      display.write((uint8_t)' ');
    else
      display.write(lcd_data[i]);
  }
  
  display.display();
//  printf("|%s|\n", lcd_data);
}
