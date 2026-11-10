/* Immediate-mode drawing without a framebuffer: 153,600 bytes do not fit in T1 RAM. */
#ifndef AETHRA_GFX_H
#define AETHRA_GFX_H

#include "hal/ae_hal.h"

#define AE_CELL_W 6
#define AE_CELL_H 10
#define AE_DIRTY_MAX 8u

typedef struct {
    const ae_hal *hal;
    ae_rect clip;
    uint32_t pixels_pushed;
    uint32_t calls;
} ae_gfx;

void ae_gfx_init(ae_gfx *g, const ae_hal *hal);
void ae_gfx_fill(ae_gfx *g, ae_rect r, uint16_t color);
/* 2 px inner border. */
void ae_gfx_frame(ae_gfx *g, ae_rect r, uint16_t color);
/* Returns the width drawn, in pixels. Control characters render as '?'. */
int ae_gfx_text(ae_gfx *g, int x, int y, const char *s, uint16_t fg, uint16_t bg);
/* Draws one glyph at scale 1..3. */
void ae_gfx_glyph_scaled(ae_gfx *g, int x, int y, char ch, int scale, uint16_t fg, uint16_t bg);
ae_rect ae_rect_intersect(ae_rect a, ae_rect b);

/* Dirty rectangles. Overflow collapses the list into one bounding box. */
typedef struct {
    ae_rect rects[AE_DIRTY_MAX];
    uint8_t count;
} ae_dirty;

void ae_dirty_clear(ae_dirty *d);
void ae_dirty_add(ae_dirty *d, ae_rect r);

/* Colour tokens: a theme is data. */
typedef struct {
    const char *name;
    uint16_t surface, surface_alt, ink, ink_muted, accent, danger, ok;
} ae_theme;

#define AE_THEME_COUNT 2u
const ae_theme *ae_theme_get(uint32_t index);

#endif
