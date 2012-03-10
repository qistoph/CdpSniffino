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

void set_menu(label_type type, const char* value) {
  unsigned int i;
  for(i=0; i<menu_size; ++i) {
    if(menu[i].type == type) {
      menu[i].value = value;
      menu[i].visible = VISIBLE;
    }
  }
}

