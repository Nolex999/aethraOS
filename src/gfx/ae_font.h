#ifndef AETHRA_FONT_H
#define AETHRA_FONT_H

#include <stdint.h>

/* 5x7 glyphs for ASCII 32..126, one byte per column, bit 0 at the top. */
extern const uint8_t ae_font5x7[95][5];

#endif
