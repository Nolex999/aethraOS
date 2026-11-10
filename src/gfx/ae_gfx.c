#include "gfx/ae_gfx.h"

#include <stddef.h>

#include "gfx/ae_font.h"

#define RGB565(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))

static const ae_theme THEMES[AE_THEME_COUNT] = {
    {"AETHRA", RGB565(18, 16, 28), RGB565(36, 32, 54), RGB565(238, 234, 246), RGB565(150, 142, 172),
     RGB565(170, 112, 255), RGB565(255, 92, 92), RGB565(106, 220, 150)},
    {"Obsidian", RGB565(0, 0, 0), RGB565(24, 24, 24), RGB565(228, 228, 228), RGB565(130, 130, 130),
     RGB565(255, 255, 255), RGB565(240, 110, 110), RGB565(170, 220, 170)},
};

const ae_theme *ae_theme_get(uint32_t index)
{
    return &THEMES[index % AE_THEME_COUNT];
}

ae_rect ae_rect_intersect(ae_rect a, ae_rect b)
{
    int x0 = a.x > b.x ? a.x : b.x;
    int y0 = a.y > b.y ? a.y : b.y;
    int x1 = a.x + a.w < b.x + b.w ? a.x + a.w : b.x + b.w;
    int y1 = a.y + a.h < b.y + b.h ? a.y + a.h : b.y + b.h;
    ae_rect r = {(int16_t)x0, (int16_t)y0, (int16_t)(x1 - x0), (int16_t)(y1 - y0)};
    if (r.w <= 0 || r.h <= 0) {
        r.x = 0;
        r.y = 0;
        r.w = 0;
        r.h = 0;
    }
    return r;
}

void ae_gfx_init(ae_gfx *g, const ae_hal *hal)
{
    ae_rect screen = {0, 0, AE_SCREEN_W, AE_SCREEN_H};
    g->hal = hal;
    g->clip = screen;
    g->pixels_pushed = 0;
    g->calls = 0;
}

void ae_gfx_fill(ae_gfx *g, ae_rect r, uint16_t color)
{
    ae_rect c = ae_rect_intersect(r, g->clip);
    if (c.w == 0) {
        return;
    }
    g->hal->display_fill(g->hal->ctx, c, color);
    g->pixels_pushed += (uint32_t)c.w * (uint32_t)c.h;
    g->calls++;
}

void ae_gfx_frame(ae_gfx *g, ae_rect r, uint16_t color)
{
    ae_rect top = {r.x, r.y, r.w, 2};
    ae_rect bottom = {r.x, (int16_t)(r.y + r.h - 2), r.w, 2};
    ae_rect left = {r.x, r.y, 2, r.h};
    ae_rect right = {(int16_t)(r.x + r.w - 2), r.y, 2, r.h};
    ae_gfx_fill(g, top, color);
    ae_gfx_fill(g, bottom, color);
    ae_gfx_fill(g, left, color);
    ae_gfx_fill(g, right, color);
}

/* Pushes a glyph, cropped to the clip rectangle. Requires scale <= 3. */
void ae_gfx_glyph_scaled(ae_gfx *g, int x, int y, char ch, int scale, uint16_t fg, uint16_t bg)
{
    uint16_t pixels[(AE_CELL_W * 3) * (AE_CELL_H * 3)];
    if (scale < 1 || scale > 3) {
        return;
    }
    int glyph = (ch >= 32 && ch <= 126) ? ch - 32 : '?' - 32;
    int w = AE_CELL_W * scale;
    int h = AE_CELL_H * scale;
    ae_rect full = {(int16_t)x, (int16_t)y, (int16_t)w, (int16_t)h};
    ae_rect c = ae_rect_intersect(full, g->clip);
    if (c.w == 0) {
        return;
    }
    int n = 0;
    for (int py = c.y - y; py < c.y - y + c.h; py++) {
        for (int px = c.x - x; px < c.x - x + c.w; px++) {
            int cx = px / scale;
            int cy = py / scale;
            bool on = cx < 5 && cy >= 1 && cy < 8 && ((ae_font5x7[glyph][cx] >> (cy - 1)) & 1);
            pixels[n++] = on ? fg : bg;
        }
    }
    g->hal->display_push(g->hal->ctx, c, pixels);
    g->pixels_pushed += (uint32_t)n;
    g->calls++;
}

int ae_gfx_text(ae_gfx *g, int x, int y, const char *s, uint16_t fg, uint16_t bg)
{
    int start = x;
    if (s == NULL) {
        return 0;
    }
    for (; *s != '\0' && x < AE_SCREEN_W; s++) {
        ae_gfx_glyph_scaled(g, x, y, *s, 1, fg, bg);
        x += AE_CELL_W;
    }
    return x - start;
}

void ae_dirty_clear(ae_dirty *d)
{
    d->count = 0;
}

static ae_rect bounds(ae_rect a, ae_rect b)
{
    int x0 = a.x < b.x ? a.x : b.x;
    int y0 = a.y < b.y ? a.y : b.y;
    int x1 = a.x + a.w > b.x + b.w ? a.x + a.w : b.x + b.w;
    int y1 = a.y + a.h > b.y + b.h ? a.y + a.h : b.y + b.h;
    ae_rect r = {(int16_t)x0, (int16_t)y0, (int16_t)(x1 - x0), (int16_t)(y1 - y0)};
    return r;
}

void ae_dirty_add(ae_dirty *d, ae_rect r)
{
    if (r.w <= 0 || r.h <= 0) {
        return;
    }
    for (uint8_t i = 0; i < d->count; i++) {
        ae_rect overlap = ae_rect_intersect(d->rects[i], r);
        if (overlap.w > 0) {
            d->rects[i] = bounds(d->rects[i], r);
            return;
        }
    }
    if (d->count == AE_DIRTY_MAX) {
        ae_rect all = r;
        for (uint8_t i = 0; i < d->count; i++) {
            all = bounds(all, d->rects[i]);
        }
        d->rects[0] = all;
        d->count = 1;
        return;
    }
    d->rects[d->count++] = r;
}
