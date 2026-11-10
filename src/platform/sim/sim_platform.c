#include "platform/sim/sim_platform.h"

#include <stdio.h>
#include <string.h>

enum { PRESS_MS = 50 };

void sim_init(sim_ctx *s, uint32_t end_ms)
{
    memset(s, 0, sizeof *s);
    s->end_ms = end_ms;
    s->battery = AE_BATTERY_UNKNOWN;
}

bool sim_add_press(sim_ctx *s, uint32_t at_ms, uint32_t key)
{
    if (s->press_count == SIM_MAX_PRESSES || key >= AE_KEY_COUNT) {
        return false;
    }
    s->presses[s->press_count].at_ms = at_ms;
    s->presses[s->press_count].key = key;
    s->press_count++;
    return true;
}

bool sim_add_capture(sim_ctx *s, uint32_t at_ms, const char *ppm_path)
{
    if (s->capture_count == SIM_MAX_CAPTURES) {
        return false;
    }
    s->captures[s->capture_count].at_ms = at_ms;
    s->captures[s->capture_count].ppm_path = ppm_path;
    s->captures[s->capture_count].taken = false;
    s->capture_count++;
    return true;
}

uint64_t sim_hash(const sim_ctx *s)
{
    uint64_t h = 0xCBF29CE484222325ull;
    for (int i = 0; i < AE_SCREEN_W * AE_SCREEN_H; i++) {
        h = (h ^ (s->fb[i] & 0xFFu)) * 0x100000001B3ull;
        h = (h ^ (s->fb[i] >> 8)) * 0x100000001B3ull;
    }
    return h;
}

bool sim_write_ppm(const sim_ctx *s, const char *path)
{
    FILE *f = fopen(path, "wb");
    if (f == NULL) {
        return false;
    }
    fprintf(f, "P6\n%d %d\n255\n", AE_SCREEN_W, AE_SCREEN_H);
    for (int i = 0; i < AE_SCREEN_W * AE_SCREEN_H; i++) {
        uint16_t c = s->fb[i];
        unsigned char rgb[3] = {(unsigned char)((c >> 11) * 255 / 31), (unsigned char)(((c >> 5) & 0x3F) * 255 / 63),
                                (unsigned char)((c & 0x1F) * 255 / 31)};
        fwrite(rgb, 1, 3, f);
    }
    return fclose(f) == 0;
}

static void h_push(void *ctx, ae_rect r, const uint16_t *pixels)
{
    sim_ctx *s = ctx;
    if (r.x < 0 || r.y < 0 || r.w <= 0 || r.h <= 0 || r.x + r.w > AE_SCREEN_W || r.y + r.h > AE_SCREEN_H) {
        return;
    }
    for (int y = 0; y < r.h; y++) {
        memcpy(&s->fb[(r.y + y) * AE_SCREEN_W + r.x], pixels + y * r.w, (size_t)r.w * sizeof(uint16_t));
    }
}

static void h_fill(void *ctx, ae_rect r, uint16_t color)
{
    sim_ctx *s = ctx;
    if (r.x < 0 || r.y < 0 || r.w <= 0 || r.h <= 0 || r.x + r.w > AE_SCREEN_W || r.y + r.h > AE_SCREEN_H) {
        return;
    }
    for (int y = r.y; y < r.y + r.h; y++) {
        for (int x = r.x; x < r.x + r.w; x++) {
            s->fb[y * AE_SCREEN_W + x] = color;
        }
    }
}

static uint64_t h_keys(void *ctx)
{
    sim_ctx *s = ctx;
    uint64_t keys = 0;
    for (uint32_t i = 0; i < s->press_count; i++) {
        if (s->now_ms >= s->presses[i].at_ms && s->now_ms < (uint64_t)s->presses[i].at_ms + PRESS_MS) {
            keys |= 1ull << s->presses[i].key;
        }
    }
    return keys;
}

static uint64_t h_time(void *ctx)
{
    return ((sim_ctx *)ctx)->now_ms;
}

/* Captures fire on the last state drawn before their time passes. */
static void h_sleep(void *ctx, uint32_t ms)
{
    sim_ctx *s = ctx;
    uint64_t next = s->now_ms + ms;
    for (uint32_t i = 0; i < s->capture_count; i++) {
        sim_capture *c = &s->captures[i];
        if (!c->taken && next >= c->at_ms) {
            c->hash = sim_hash(s);
            c->taken = true;
            if (c->ppm_path != NULL) {
                (void)sim_write_ppm(s, c->ppm_path);
            }
        }
    }
    s->now_ms = next;
}

static int h_battery(void *ctx)
{
    return ((sim_ctx *)ctx)->battery;
}

static bool h_quit(void *ctx)
{
    sim_ctx *s = ctx;
    return s->now_ms >= s->end_ms;
}

ae_hal sim_headless_hal(sim_ctx *s)
{
    ae_hal hal = {s, h_push, h_fill, h_keys, h_time, h_sleep, h_battery, h_quit};
    return hal;
}
