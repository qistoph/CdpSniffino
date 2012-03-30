#include "helpers.h"

size_t snprintnum(char* buffer, size_t buffer_size, unsigned long n, uint8_t base) {
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *str = &buf[sizeof(buf) - 1];
  
  *str = '\0';
  
  if(base < 2) base = 10; // prevent crash
  
  do {
    unsigned long m = n;
    n /= base;
    char c = m - base * n;
    *--str = c < 10 ? c + '0' : c + 'A' - 10;
  } while(n);
  
  unsigned int i = 0;
  while(*str != '\0' && i < (buffer_size - 1)) {
    buffer[i++] = *str++;
  }
  buffer[i] = '\0';
  
  return i;
}
