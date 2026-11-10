/* Stub used only to compile-check platform/eadk. Replace by checking against the real eadk.h. */
#ifndef AETHRA_EADK_STUB_H
#define AETHRA_EADK_STUB_H

#include <stdbool.h>
#include <stdint.h>

typedef uint16_t eadk_color_t;
typedef struct {
    uint16_t x, y, width, height;
} eadk_rect_t;
typedef uint64_t eadk_keyboard_state_t;

void eadk_display_push_rect_uniform(eadk_rect_t rect, eadk_color_t color);
void eadk_display_push_rect(eadk_rect_t rect, const eadk_color_t *color);
void eadk_timing_msleep(uint32_t ms);
uint64_t eadk_timing_millis(void);
eadk_keyboard_state_t eadk_keyboard_scan(void);

#endif
