#include "boot_anim.h"
#include "spectrum_sprite.h"

#include "gfx/ae_font.h"


/* Timeline, in frames at AB_FPS. */
enum {
    T_EYES = 18,
    T_BODY = 38,
    T_BODY_DONE = 78,
    T_HANDS = 66,
    T_HANDS_DONE = 92,
    T_CHARGE = 98,
    T_BLAST = 138,
    T_LOGO_DONE = 164,
    T_GRIN = 168,
    T_WINK = 188,
    T_FADE = 200,
};

#define LOGO_H 7
#define LOGO_Y 9
#define STATUS_ROW 21

typedef struct {
    uint8_t width;
    const char *rows[LOGO_H];
} logo_glyph;

/* The A is drawn as a spiked ghost crown; widths sum to 46, plus 5 gaps. */
static const logo_glyph LOGO[6] = {
    {9, {"....#....", "...###...", "...#.#...", "..##.##..", "..#...#..", ".##...##.", "#########"}},
    {6, {"######", "##....", "##....", "#####.", "##....", "##....", "######"}},
    {8, {"########", "...##...", "...##...", "...##...", "...##...", "...##...", "...##..."}},
    {7, {"##...##", "##...##", "##...##", "#######", "##...##", "##...##", "##...##"}},
    {7, {"######.", "##...##", "##...##", "######.", "##.##..", "##..##.", "##...##"}},
    {9, {"....#....", "...###...", "...#.#...", "..##.##..", "..#...#..", ".##...##.", "#########"}},
};

static int isin(uint32_t angle)
{
    int a = (int)(angle & 0xFF);
    int x = a < 128 ? a : a - 128;
    int y = x * (128 - x) / 32;
    if (y > 127) {
        y = 127;
    }
    return a < 128 ? y : -y;
}

static uint32_t mix(uint32_t a, uint32_t b)
{
    uint32_t h = a * 0x9E3779B1u ^ (b * 0x85EBCA77u + 0x165667B1u);
    h ^= h >> 15;
    h *= 0xC2B2AE3Du;
    h ^= h >> 13;
    return h;
}

/* 0 before a, 256 from b onwards, linear in between. */
static int ramp(int f, int a, int b)
{
    if (f <= a) {
        return 0;
    }
    if (f >= b) {
        return 256;
    }
    return (f - a) * 256 / (b - a);
}

static uint16_t rgb565(int r, int g, int b)
{
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

static uint16_t hsv(uint8_t h, uint8_t s, uint8_t v)
{
    int region = h / 43;
    int rem = (h - region * 43) * 6;
    int p = v * (255 - s) / 256;
    int q = v * (255 - s * rem / 256) / 256;
    int t = v * (255 - s * (255 - rem) / 256) / 256;
    switch (region) {
    case 0: return rgb565(v, t, p);
    case 1: return rgb565(q, v, p);
    case 2: return rgb565(p, v, t);
    case 3: return rgb565(p, q, v);
    case 4: return rgb565(t, p, v);
    default: return rgb565(v, p, q);
    }
}

static uint16_t dim(uint16_t c, int amount)
{
    int r = ((c >> 11) & 0x1F) * amount / 256;
    int g = ((c >> 5) & 0x3F) * amount / 256;
    int b = (c & 0x1F) * amount / 256;
    return (uint16_t)((r << 11) | (g << 5) | b);
}

static void put(ab_grid *g, int x, int y, char ch, uint16_t fg)
{
    if (x >= 0 && x < AB_COLS && y >= 0 && y < AB_ROWS) {
        g->cells[y][x].ch = ch;
        g->cells[y][x].fg = fg;
    }
}

static char sprite_head(int x, int y)
{
    if (x < 0 || x >= SP_W || y < 0 || y >= SP_H) {
        return '.';
    }
    return SP_HEAD[y][x < SP_HALF ? x : SP_W - 1 - x];
}

static void draw_fog(ab_grid *g, int f)
{
    for (int y = 0; y < AB_ROWS; y++) {
        for (int x = 0; x < AB_COLS; x++) {
            int n = isin((uint32_t)(x * 5 + f)) + isin((uint32_t)(y * 11 - f * 2)) +
                    isin((uint32_t)(x * 3 + y * 7 + f / 2));
            uint32_t h = mix((uint32_t)x, (uint32_t)y);
            if (n > 110 && (h & 7) < 3) {
                put(g, x, y, (h & 8) ? '.' : ',', hsv(200, 220, (uint8_t)(34 + (n - 110) / 4)));
            }
        }
    }
}

static void draw_prompt(ab_grid *g, int f)
{
    static const char text[] = "> aethra: cold start";
    int fade = 256 - ramp(f, 30, 44);
    if (fade <= 0) {
        return;
    }
    int n = f > 20 ? 20 : f;
    uint16_t c = dim(hsv(200, 90, 200), fade);
    for (int i = 0; i < n; i++) {
        put(g, 1 + i, 1, text[i], c);
    }
    if ((f / 6) & 1) {
        put(g, 1 + n, 1, '_', c);
    }
}

/* Body, rim, eyes, mouth for the head and hands. `level` is 0..256 brightness. */
static void shade(ab_grid *g, int x, int y, char attr, int f, int level, int charge)
{
    uint32_t h = mix((uint32_t)x, (uint32_t)y);
    int wave = isin((uint32_t)(x * 9 + y * 13 + f * 3)) / 3;
    switch (attr) {
    case 'b': {
        static const char ink[] = "####%#8#";
        int v = 214 - y * 4 + wave / 2 + (int)(h & 15);
        if (v < 88) {
            v = 88;
        }
        if (v > 255) {
            v = 255;
        }
        put(g, x, y, ink[(h + (uint32_t)f / 11) % 8], dim(hsv(198, 205, (uint8_t)v), level));
        break;
    }
    case 'd':
        put(g, x, y, (h & 1) ? '+' : '=', dim(hsv(205, 225, 118), level));
        break;
    case 'e':
        put(g, x, y, '@', dim(hsv(40, 34, 255), level));
        break;
    case 'p':
        put(g, x, y, 'O', dim(hsv(0, 255, (uint8_t)(120 + charge * 135 / 256)), level));
        break;
    case 'm':
        put(g, x, y, (h & 1) ? ':' : '.', dim(hsv(232, 220, 86), level));
        break;
    case 't':
        put(g, x, y, 'V', dim(rgb565(245, 245, 235), level));
        break;
    case 'l':
        put(g, x, y, (((uint32_t)y + (uint32_t)f / 4) & 1) ? '~' : '=', dim(hsv(238, 120, 250), level));
        break;
    default:
        break;
    }
}

static void draw_gas(ab_grid *g, int ox, int oy, int f, int reveal)
{
    for (int y = 0; y < AB_ROWS; y++) {
        for (int x = 0; x < AB_COLS; x++) {
            if (g->cells[y][x].ch != ' ' && g->cells[y][x].ch != '.' && g->cells[y][x].ch != ',') {
                continue;
            }
            uint32_t h = mix((uint32_t)x + 57u, (uint32_t)y * 3u);
            int rise = 1 + (int)((h + (uint32_t)f / 2) % 3u);
            int drift = isin((uint32_t)(y * 17 + f * 2)) / 70;
            if (sprite_head(x - ox + drift, y - oy + rise) == '.') {
                continue;
            }
            if ((int)(h % 256u) >= reveal / 2) {
                continue;
            }
            static const char puff[] = ".':~";
            int v = 128 - rise * 28 + isin((uint32_t)(x * 7 + f * 4)) / 4;
            put(g, x, y, puff[(h + (uint32_t)f / 3) % 4], hsv(200, 210, (uint8_t)(v < 30 ? 30 : v)));
        }
    }
}

static void draw_ghost(ab_grid *g, int ox, int oy, int f, int reveal, int charge)
{
    int eyes = ramp(f, T_EYES, T_EYES + 18);
    int blink = (f > T_BODY_DONE && (f % 46) < 3) ? 1 : 0;
    for (int sy = 0; sy < SP_H; sy++) {
        for (int sx = 0; sx < SP_W; sx++) {
            char a = sprite_head(sx, sy);
            if (a == '.') {
                continue;
            }
            int x = ox + sx;
            int y = oy + sy;
            if (a == 'e' || a == 'p') {
                if (eyes == 0) {
                    continue;
                }
                if (blink) {
                    put(g, x, y, '-', hsv(40, 34, 230));
                } else {
                    shade(g, x, y, a, f, eyes, charge);
                }
                continue;
            }
            int gate = (int)(mix((uint32_t)sx * 7u, (uint32_t)sy) & 0xFF);
            if (gate < reveal - 34) {
                if (a == 'l') {
                    int wag = isin((uint32_t)f * 10u) / 70;
                    shade(g, x + wag, y, a, f, 256, charge);
                } else {
                    shade(g, x, y, a, f, 256, charge);
                }
            } else if (gate < reveal) {
                put(g, x, y, '.', hsv(200, 160, 170));
            }
        }
    }
}

static void draw_hand(ab_grid *g, int ox, int oy, bool mirror, int f, int level, int charge)
{
    for (int sy = 0; sy < SP_HAND_H; sy++) {
        for (int sx = 0; sx < SP_HAND_W; sx++) {
            char a = SP_HAND[sy][mirror ? SP_HAND_W - 1 - sx : sx];
            if (a != '.') {
                shade(g, ox + sx, oy + sy, a, f, level, charge);
            }
        }
    }
}

static void draw_ball(ab_grid *g, int cx, int cy, int f)
{
    int growth = ramp(f, T_CHARGE, T_BLAST - 6);
    int radius = 10 + growth * 38 / 256;
    int r2 = radius * radius;
    static const char core[] = "@%#*+=:";
    for (int y = cy - 8; y <= cy + 8; y++) {
        for (int x = cx - 13; x <= cx + 13; x++) {
            int dx = (x - cx) * AB_CELL_W;
            int dy = (y - cy) * AB_CELL_H;
            int d2 = dx * dx + dy * dy;
            if (d2 > r2) {
                continue;
            }
            int depth = (r2 - d2) * 256 / r2;
            int swirl = isin((uint32_t)(dx * dy / 64 + f * 9)) + isin((uint32_t)(dx / 3 - f * 7));
            int idx = (depth / 40 + (swirl > 40 ? 1 : 0) + (f / 2)) % 7;
            uint16_t c = depth < 70 ? hsv(226, 215, 255) : hsv(190, 240, (uint8_t)(190 - depth / 3 + swirl / 6));
            put(g, x, y, core[idx], c);
        }
    }
    /* Energy feeding the ball from the edges of the screen. */
    for (int i = 0; i < 26; i++) {
        uint32_t h = mix((uint32_t)i, 77u);
        int phase = (f * 2 + (int)(h & 255)) & 255;
        int dist = 256 - phase;
        int ang = (int)(h >> 8);
        int x = cx + isin((uint32_t)ang + 64u) * dist / 1000;
        int y = cy + isin((uint32_t)ang) * dist / 1700;
        put(g, x, y, (i & 1) ? '*' : '+', hsv(225, 190, (uint8_t)(255 - dist / 3)));
    }
}

static void draw_blast(ab_grid *g, int cx, int cy, int f)
{
    int t = f - T_BLAST;
    int front = t * 56;
    int width = 46;
    for (int y = 0; y < AB_ROWS; y++) {
        for (int x = 0; x < AB_COLS; x++) {
            int dx = (x - cx) * AB_CELL_W;
            int dy = (y - cy) * AB_CELL_H;
            int ax = dx < 0 ? -dx : dx;
            int ay = dy < 0 ? -dy : dy;
            int d = ax > ay ? ax + ay * 3 / 8 : ay + ax * 3 / 8; /* close to a circle */
            if (d > front || d < front - width) {
                continue;
            }
            int edge = (front - d) * 256 / width;
            static const char shock[] = "@#*+:.";
            put(g, x, y, shock[(edge * 6 / 256) % 6],
                edge < 90 ? rgb565(255, 255, 255) : hsv(208, (uint8_t)(edge * 3 / 4), 255));
        }
    }
}

static void draw_logo(ab_grid *g, int f)
{
    static const char noise[] = "@#%&$*+=?/";
    int left = (AB_COLS - 51) / 2;
    int settle = ramp(f, T_LOGO_DONE, T_LOGO_DONE + 12);
    int x0 = left;
    for (int i = 0; i < 6; i++) {
        for (int r = 0; r < LOGO_H; r++) {
            for (int c = 0; c < LOGO[i].width; c++) {
                if (LOGO[i].rows[r][c] != '#') {
                    continue;
                }
                int x = x0 + c;
                int y = LOGO_Y + r;
                int dx = x - AB_COLS / 2;
                int dist = (dx < 0 ? -dx : dx) + (y > 12 ? y - 12 : 12 - y);
                uint32_t h = mix((uint32_t)x, (uint32_t)y * 5u);
                int local = (f - T_BLAST) * 2 - dist - (int)(h & 7);
                if (local < 0) {
                    continue;
                }
                if (local < 12) {
                    put(g, x, y, noise[(h + (uint32_t)f) % 10], hsv((uint8_t)(190 + local * 4), 180, 255));
                    continue;
                }
                int band = isin((uint32_t)(x * 4 - f * 5)) / 8;
                uint8_t hue = (uint8_t)(190 + r * 6);
                int v = 214 + band + settle / 12;
                put(g, x, y, '#', hsv(hue, (uint8_t)(180 - settle / 6), (uint8_t)(v > 255 ? 255 : v)));
            }
            /* Ectoplasm dripping from the bottom edge of each letter. */
            if (r == LOGO_H - 1 && settle > 0) {
                for (int c = 0; c < LOGO[i].width; c++) {
                    if (LOGO[i].rows[r][c] != '#') {
                        continue;
                    }
                    uint32_t h = mix((uint32_t)i * 13u + 5u, (uint32_t)c);
                    if ((h & 3) != 0) {
                        continue;
                    }
                    int len = 1 + (int)((h >> 3) % 3u);
                    int fall = (f / 4 + (int)(h >> 6)) % 7;
                    for (int d = 0; d < len && d <= fall; d++) {
                        put(g, x0 + c, LOGO_Y + LOGO_H + d, d == len - 1 ? '\'' : ':',
                            dim(hsv(200, 190, 190), settle * (len - d) / len));
                    }
                }
            }
        }
        x0 += LOGO[i].width + 1;
    }
}

static void draw_grin(ab_grid *g, int f)
{
    int level = ramp(f, T_GRIN, T_GRIN + 12);
    int ox = (AB_COLS - SP_GRIN_W) / 2;
    int oy = 2;
    bool wink = f >= T_WINK && f < T_WINK + 10;
    if (level == 0) {
        return;
    }
    for (int r = 0; r < SP_GRIN_H; r++) {
        for (int c = 0; c < SP_GRIN_W; c++) {
            char a = SP_GRIN[r][c];
            if (a == '.') {
                continue;
            }
            if (wink && c < SP_GRIN_W / 2 && (a == 'e' || a == 'p')) {
                if (r == 2) {
                    put(g, ox + c, oy + r, '-', dim(hsv(40, 34, 255), level));
                }
                continue;
            }
            shade(g, ox + c, oy + r, a, f, level, 256);
        }
    }
}

static int put_text(ab_grid *g, int x, int y, const char *s, uint16_t color, int limit)
{
    int n = 0;
    if (!s) {
        return 0;
    }
    while (s[n] != '\0' && x + n < AB_COLS && n < limit) {
        char ch = s[n];
        put(g, x + n, y, (ch >= 32 && ch <= 126) ? ch : '?', color);
        n++;
    }
    return n;
}

static void draw_status(ab_grid *g, int f, const ab_status *st)
{
    int level = ramp(f, T_BLAST + 14, T_LOGO_DONE);
    if (level == 0 || !st) {
        return;
    }
    int pct = st->percent > 100 ? 100 : st->percent;
    int width = AB_COLS - 12;
    int filled = width * pct / 100;
    put(g, 2, STATUS_ROW, '[', dim(hsv(200, 70, 200), level));
    put(g, 3 + width, STATUS_ROW, ']', dim(hsv(200, 70, 200), level));
    for (int i = 0; i < width; i++) {
        if (i < filled) {
            uint8_t hue = (uint8_t)(180 + i * 50 / width);
            int pulse = isin((uint32_t)(i * 8 - f * 9)) / 5;
            put(g, 3 + i, STATUS_ROW, '=', dim(hsv(hue, 200, (uint8_t)(220 + pulse)), level));
        } else {
            put(g, 3 + i, STATUS_ROW, '.', dim(hsv(200, 80, 70), level));
        }
    }
    int tens = pct >= 100 ? 1 : 0;
    char num[5];
    num[0] = tens ? '1' : ' ';
    num[1] = (pct >= 10) ? (char)('0' + (pct / 10) % 10) : ' ';
    num[2] = (char)('0' + pct % 10);
    num[3] = '%';
    num[4] = '\0';
    put_text(g, AB_COLS - 7, STATUS_ROW, num, dim(hsv(200, 40, 240), level), 4);
    put_text(g, 2, STATUS_ROW + 1, st->stage, dim(hsv(200, 50, 220), level), AB_COLS - 4);
    put_text(g, 2, STATUS_ROW + 2, st->tag, dim(hsv(200, 110, 130), level), AB_COLS - 4);
}

bool ab_render(ab_grid *grid, uint32_t frame, const ab_status *status)
{
    for (int y = 0; y < AB_ROWS; y++) {
        for (int x = 0; x < AB_COLS; x++) {
            grid->cells[y][x].ch = ' ';
            grid->cells[y][x].fg = 0;
        }
    }
    if (frame >= AB_FRAME_COUNT) {
        return true;
    }

    int f = (int)frame;
    int ball_x = AB_COLS / 2;
    int ball_y = 17;

    draw_fog(grid, f);
    if (f < 46) {
        draw_prompt(grid, f);
    }

    if (f < T_BLAST + 8) {
        int ox = (AB_COLS - SP_W) / 2 + isin((uint32_t)f * 3u) / 90;
        int oy = 1 + isin((uint32_t)f * 4u) / 110;
        int reveal = ramp(f, T_BODY, T_BODY_DONE) * 290 / 256;
        int charge = ramp(f, T_CHARGE, T_BLAST);
        if (f >= T_BLAST) {
            reveal = 290 - ramp(f, T_BLAST, T_BLAST + 8) * 290 / 256;
        }
        draw_ghost(grid, ox, oy, f, reveal, charge);
        draw_gas(grid, ox, oy, f, reveal);

        int in = ramp(f, T_HANDS, T_HANDS_DONE);
        if (in > 0 && f < T_BLAST) {
            int close = ramp(f, T_CHARGE, T_BLAST - 8);
            int lx = -10 + in * 11 / 256 + close * 6 / 256;
            int rx = AB_COLS - SP_HAND_W - lx;
            int ly = 10 + isin((uint32_t)f * 7u) / 64 + close * 4 / 256;
            int ry = 10 + isin((uint32_t)f * 7u + 90u) / 64 + close * 4 / 256;
            draw_hand(grid, lx, ly, false, f, in, charge);
            draw_hand(grid, rx, ry, true, f, in, charge);
        }
        if (f >= T_CHARGE && f < T_BLAST) {
            draw_ball(grid, ball_x, ball_y, f);
        }
    }

    if (f >= T_BLAST) {
        draw_logo(grid, f);
        draw_grin(grid, f);
    }
    if (f >= T_BLAST && f < T_BLAST + 16) {
        draw_blast(grid, ball_x, ball_y, f);
    }
    draw_status(grid, f, status);

    if (f >= T_FADE) {
        int level = 256 - ramp(f, T_FADE, (int)AB_FRAME_COUNT - 1);
        for (int y = 0; y < AB_ROWS; y++) {
            for (int x = 0; x < AB_COLS; x++) {
                grid->cells[y][x].fg = dim(grid->cells[y][x].fg, level);
            }
        }
    }
    return false;
}

void ab_cell_pixels(ab_cell cell, uint16_t bg, uint16_t *pixels)
{
    int glyph = (cell.ch >= 32 && cell.ch <= 126) ? cell.ch - 32 : 0;
    for (int y = 0; y < AB_CELL_H; y++) {
        for (int x = 0; x < AB_CELL_W; x++) {
            bool on = false;
            if (x < 5 && y >= 1 && y < 8) {
                on = (ae_font5x7[glyph][x] >> (y - 1)) & 1;
            }
            pixels[y * AB_CELL_W + x] = on ? cell.fg : bg;
        }
    }
}

void ab_grid_blit(const ab_grid *grid, uint16_t bg, uint16_t *framebuffer)
{
    uint16_t pix[AB_CELL_W * AB_CELL_H];
    for (int i = 0; i < AB_SCREEN_W * AB_SCREEN_H; i++) {
        framebuffer[i] = bg;
    }
    for (int r = 0; r < AB_ROWS; r++) {
        for (int c = 0; c < AB_COLS; c++) {
            ab_cell_pixels(grid->cells[r][c], bg, pix);
            for (int y = 0; y < AB_CELL_H; y++) {
                uint16_t *dst = framebuffer + (r * AB_CELL_H + y) * AB_SCREEN_W + AB_MARGIN_X + c * AB_CELL_W;
                for (int x = 0; x < AB_CELL_W; x++) {
                    dst[x] = pix[y * AB_CELL_W + x];
                }
            }
        }
    }
}

bool ab_cell_differs(ab_cell a, ab_cell b)
{
    if (a.ch != b.ch) {
        return true;
    }
    return a.ch != ' ' && a.fg != b.fg;
}
