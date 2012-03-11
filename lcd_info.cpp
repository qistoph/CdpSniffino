#include "lcd_info.h"

menu_item menu[] = {
  {
    LABEL_MAC,
    ("Mac"),
    NULL,
    INVISIBLE
  },
  {
    LABEL_DEVICE_ID,
    ("DevID"),
    NULL,
    INVISIBLE
  },
  {
    LABEL_ADDRESS,
    ("Addr"),
    NULL,
    INVISIBLE
  },
  {
    LABEL_PORT_ID,
    ("Port"),
    NULL,
    INVISIBLE
  },
  {
    LABEL_SOFTWARE,
    ("Sw"),
    NULL,
    INVISIBLE
  },
  {
    LABEL_PLATFORM,
    "Pf",
    NULL,
    INVISIBLE
  },
  {
    LABEL_NATIVE_VLAN,
    "Natv.Vlan",
    NULL,
    INVISIBLE
  },
  {
    LABEL_DUPLEX,
    "Duplex",
    NULL,
    INVISIBLE
  }
};

size_t menu_size = sizeof(menu)/sizeof(*menu);

unsigned int menu_current = 0;
unsigned int lcd_more_offset = 0;

void set_menu(label_type type, const char* value) {
  unsigned int i;
  for(i=0; i<menu_size; ++i) {
    if(menu[i].type == type) {
      menu[i].value = value;
      menu[i].visible = VISIBLE;
    }
  }
}

void lcd_info_next() {
  size_t menu_next = menu_current + 1;
  if(menu_next > menu_size) menu_next = 0;
  while(menu[menu_next].visible != VISIBLE &&
    menu_next != menu_current
  ) {
    if(++menu_next > menu_size) {
      menu_next = 0;
    }
  }
  menu_current = menu_next;
  lcd_more_offset = 0;
}

void lcd_info_more() {
  if(++lcd_more_offset > strlen(menu[menu_current].value))
    lcd_more_offset = 0;
}
