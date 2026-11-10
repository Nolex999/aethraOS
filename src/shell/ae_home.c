#include "shell/ae_shell.h"

enum { BAR_H = 18, TILE_GAP = 8, COLS = 3, ROWS = 2 };

#define TILE_W ((AE_SCREEN_W - (COLS + 1) * TILE_GAP) / COLS)
#define TILE_H ((AE_SCREEN_H - 2 * BAR_H - (ROWS + 1) * TILE_GAP) / ROWS)

static const struct {
    char icon;
    const char *name;
} TILES[AE_TILE_COUNT] = {
    {'+', "Calcul"}, {'~', "Graphes"}, {'>', "Terminal"}, {'#', "Fichiers"}, {'P', "Python"}, {'*', "Reglages"},
};

static const ae_rect STATUS_BAR = {0, 0, AE_SCREEN_W, BAR_H};
static const ae_rect ACTION_BAR = {0, AE_SCREEN_H - BAR_H, AE_SCREEN_W, BAR_H};

static ae_rect tile_rect(uint32_t i)
{
    ae_rect r;
    r.x = (int16_t)(TILE_GAP + (int)(i % COLS) * (TILE_W + TILE_GAP));
    r.y = (int16_t)(BAR_H + TILE_GAP + (int)(i / COLS) * (TILE_H + TILE_GAP));
    r.w = TILE_W;
    r.h = TILE_H;
    return r;
}

static void invalidate_all(ae_home *h)
{
    ae_rect screen = {0, 0, AE_SCREEN_W, AE_SCREEN_H};
    ae_dirty_add(&h->dirty, screen);
}

void ae_home_init(ae_home *h, const ae_hal *hal)
{
    ae_gfx_init(&h->gfx, hal);
    ae_dirty_clear(&h->dirty);
    h->theme = 0;
    h->focus = 0;
    h->last_seconds = 0xFFFFFFFFu;
    h->toast = NULL;
    h->toast_until_ms = 0;
    invalidate_all(h);
}

static void set_focus(ae_home *h, uint32_t next)
{
    if (next == h->focus || next >= AE_TILE_COUNT) {
        return;
    }
    ae_dirty_add(&h->dirty, tile_rect(h->focus));
    ae_dirty_add(&h->dirty, tile_rect(next));
    h->focus = next;
}

void ae_home_handle(ae_home *h, ae_event ev, uint64_t now_ms)
{
    if (ev.type != AE_EV_KEY_DOWN) {
        return;
    }
    switch (ev.code) {
    case AE_KEY_LEFT:
        if (h->focus % COLS != 0) {
            set_focus(h, h->focus - 1);
        }
        break;
    case AE_KEY_RIGHT:
        if (h->focus % COLS != COLS - 1) {
            set_focus(h, h->focus + 1);
        }
        break;
    case AE_KEY_UP:
        if (h->focus >= COLS) {
            set_focus(h, h->focus - COLS);
        }
        break;
    case AE_KEY_DOWN:
        if (h->focus + COLS < AE_TILE_COUNT) {
            set_focus(h, h->focus + COLS);
        }
        break;
    case AE_KEY_OK:
    case AE_KEY_EXE:
        /* No application exists yet: say so rather than pretend. */
        h->toast = "Pas encore disponible";
        h->toast_until_ms = now_ms + 2000u;
        ae_dirty_add(&h->dirty, ACTION_BAR);
        break;
    case AE_KEY_TOOLBOX:
        h->theme = (h->theme + 1u) % AE_THEME_COUNT;
        invalidate_all(h);
        break;
    default:
        break;
    }
}

void ae_home_tick(ae_home *h, uint64_t now_ms)
{
    uint32_t seconds = (uint32_t)(now_ms / 1000u);
    if (seconds != h->last_seconds) {
        h->last_seconds = seconds;
        ae_dirty_add(&h->dirty, STATUS_BAR);
    }
    if (h->toast != NULL && now_ms >= h->toast_until_ms) {
        h->toast = NULL;
        ae_dirty_add(&h->dirty, ACTION_BAR);
    }
}

static void put2(char *p, uint32_t v)
{
    p[0] = (char)('0' + (v / 10u) % 10u);
    p[1] = (char)('0' + v % 10u);
}

static void draw_status(ae_home *h, const ae_theme *t)
{
    char uptime[6] = "00:00";
    char battery[5] = "--% ";
    int level = h->gfx.hal->battery_percent(h->gfx.hal->ctx);
    put2(uptime, (h->last_seconds / 60u) % 100u);
    put2(uptime + 3, h->last_seconds % 60u);
    if (level >= 0) {
        uint32_t v = level > 100 ? 100u : (uint32_t)level;
        battery[0] = v >= 100u ? '1' : ' ';
        put2(battery + 1, v % 100u);
        battery[3] = '%';
    }
    ae_gfx_fill(&h->gfx, STATUS_BAR, t->surface_alt);
    ae_gfx_text(&h->gfx, 6, 4, "AETHRA", t->accent, t->surface_alt);
    ae_gfx_text(&h->gfx, 54, 4, "Maths", t->ink_muted, t->surface_alt);
    ae_gfx_text(&h->gfx, 240, 4, uptime, t->ink, t->surface_alt);
    ae_gfx_text(&h->gfx, 284, 4, battery, t->ink, t->surface_alt);
}

static void draw_tile(ae_home *h, const ae_theme *t, uint32_t i)
{
    ae_rect r = tile_rect(i);
    ae_gfx_fill(&h->gfx, r, t->surface_alt);
    if (i == h->focus) {
        ae_gfx_frame(&h->gfx, r, t->accent);
    }
    ae_gfx_glyph_scaled(&h->gfx, r.x + (r.w - AE_CELL_W * 3) / 2 + 2, r.y + 14, TILES[i].icon, 3,
                        i == h->focus ? t->accent : t->ink_muted, t->surface_alt);
    int name_w = 0;
    while (TILES[i].name[name_w] != '\0') {
        name_w++;
    }
    ae_gfx_text(&h->gfx, r.x + (r.w - name_w * AE_CELL_W) / 2, r.y + r.h - 18, TILES[i].name, t->ink, t->surface_alt);
}

static void draw_actions(ae_home *h, const ae_theme *t)
{
    ae_gfx_fill(&h->gfx, ACTION_BAR, t->surface_alt);
    if (h->toast != NULL) {
        ae_gfx_text(&h->gfx, 6, ACTION_BAR.y + 4, h->toast, t->danger, t->surface_alt);
    } else {
        ae_gfx_text(&h->gfx, 6, ACTION_BAR.y + 4, "OK ouvrir", t->ink_muted, t->surface_alt);
        ae_gfx_text(&h->gfx, 90, ACTION_BAR.y + 4, "toolbox theme", t->ink_muted, t->surface_alt);
        ae_gfx_text(&h->gfx, 320 - 6 - 8 * AE_CELL_W, ACTION_BAR.y + 4, ae_theme_get(h->theme)->name, t->accent, t->surface_alt);
    }
}

void ae_home_render(ae_home *h, uint64_t now_ms)
{
    const ae_theme *t = ae_theme_get(h->theme);
    (void)now_ms;
    for (uint8_t d = 0; d < h->dirty.count; d++) {
        ae_rect area = h->dirty.rects[d];
        h->gfx.clip = area;
        ae_gfx_fill(&h->gfx, area, t->surface);
        draw_status(h, t);
        for (uint32_t i = 0; i < AE_TILE_COUNT; i++) {
            if (ae_rect_intersect(tile_rect(i), area).w > 0) {
                draw_tile(h, t, i);
            }
        }
        draw_actions(h, t);
    }
    ae_dirty_clear(&h->dirty);
    ae_rect screen = {0, 0, AE_SCREEN_W, AE_SCREEN_H};
    h->gfx.clip = screen;
}
