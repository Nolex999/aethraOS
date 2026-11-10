/* Hardware abstraction: the only way upper layers reach a platform. */
#ifndef AETHRA_HAL_H
#define AETHRA_HAL_H

#include <stdbool.h>
#include <stdint.h>

#define AE_SCREEN_W 320
#define AE_SCREEN_H 240

typedef struct {
    int16_t x, y, w, h;
} ae_rect;

/*
 * Key bits follow the N0120 keypad matrix (bit = row * 6 + column). The masks in
 * the Epsilon kernel leave bits 7, 9-11, 35, 41 and 47 unused, as this table does.
 */
typedef enum {
    AE_KEY_LEFT = 0,
    AE_KEY_UP = 1,
    AE_KEY_DOWN = 2,
    AE_KEY_RIGHT = 3,
    AE_KEY_OK = 4,
    AE_KEY_BACK = 5,
    AE_KEY_HOME = 6,
    AE_KEY_ONOFF = 8,
    AE_KEY_SHIFT = 12,
    AE_KEY_ALPHA = 13,
    AE_KEY_XNT = 14,
    AE_KEY_VAR = 15,
    AE_KEY_TOOLBOX = 16,
    AE_KEY_BACKSPACE = 17,
    AE_KEY_EXE = 52,
    AE_KEY_COUNT = 53,
} ae_key;

#define AE_BATTERY_UNKNOWN (-1)

typedef struct {
    void *ctx;
    void (*display_push)(void *ctx, ae_rect r, const uint16_t *pixels);
    void (*display_fill)(void *ctx, ae_rect r, uint16_t color);
    uint64_t (*keys_scan)(void *ctx);
    uint64_t (*time_ms)(void *ctx);
    void (*sleep_ms)(void *ctx, uint32_t ms);
    int (*battery_percent)(void *ctx); /* AE_BATTERY_UNKNOWN when not measurable */
    bool (*should_quit)(void *ctx);
} ae_hal;

#endif
