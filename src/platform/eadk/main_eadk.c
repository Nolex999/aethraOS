/*
 * T1 entry point: AETHRA as an external app on stock Epsilon.
 *
 * Status: built with the real eadk.h from nwlink 0.0.19, never run on a calculator.
 */
#include <eadk.h>

#include "shell/ae_shell.h"

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "AETHRA";
const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;

static void push(void *ctx, ae_rect r, const uint16_t *pixels)
{
    eadk_rect_t rect = {(uint16_t)r.x, (uint16_t)r.y, (uint16_t)r.w, (uint16_t)r.h};
    (void)ctx;
    eadk_display_push_rect(rect, pixels);
}

static void fill(void *ctx, ae_rect r, uint16_t color)
{
    eadk_rect_t rect = {(uint16_t)r.x, (uint16_t)r.y, (uint16_t)r.w, (uint16_t)r.h};
    (void)ctx;
    eadk_display_push_rect_uniform(rect, color);
}

static uint64_t keys(void *ctx)
{
    (void)ctx;
    return eadk_keyboard_scan();
}

static uint64_t now_ms(void *ctx)
{
    (void)ctx;
    return eadk_timing_millis();
}

static void sleep_for(void *ctx, uint32_t ms)
{
    (void)ctx;
    eadk_timing_msleep(ms);
}

static int battery(void *ctx)
{
    (void)ctx;
    return AE_BATTERY_UNKNOWN; /* The unit of eadk_battery_level is unknown: show "--" rather than guess. */
}

/* Back returns to Epsilon, since Home and On/Off belong to the system in T1. */
static bool quit(void *ctx)
{
    (void)ctx;
    return (eadk_keyboard_scan() & (1ull << AE_KEY_BACK)) != 0;
}

int main(void)
{
    ae_hal hal = {NULL, push, fill, keys, now_ms, sleep_for, battery, quit};
    ae_run_report report;
    return ae_system_run(&hal, &report) == AE_OK ? 0 : 1;
}
