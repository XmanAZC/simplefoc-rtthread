#include "Arduino.h"

#include <stdlib.h>

RTTConsole Serial;

size_t RTTConsole::write(uint8_t ch) {
  rt_kprintf("%c", ch);
  return 1;
}

size_t RTTConsole::write(const uint8_t* buffer, size_t size) {
  if (!buffer || size == 0) {
    return 0;
  }

  for (size_t i = 0; i < size; ++i) {
    rt_kprintf("%c", buffer[i]);
  }
  return size;
}

uint16_t makeWord(uint16_t w) {
  return w;
}

uint16_t makeWord(byte h, byte l) {
  return (uint16_t)(((uint16_t)h << 8) | (uint16_t)l);
}

long random(long howbig) {
  if (howbig <= 0) {
    return 0;
  }
  return rand() % howbig;
}

long random(long howsmall, long howbig) {
  if (howsmall >= howbig) {
    return howsmall;
  }
  return howsmall + random(howbig - howsmall);
}
