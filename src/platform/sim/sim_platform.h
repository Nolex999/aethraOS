/* Host simulator: a virtual-time headless backend, and a Win32 window on Windows. */
#ifndef AETHRA_SIM_PLATFORM_H
#define AETHRA_SIM_PLATFORM_H

#include "hal/ae_hal.h"

#define SIM_MAX_PRESSES 32u
#define SIM_MAX_CAPTURES 16u

typedef struct {
    uint32_t at_ms;
    uint32_t key;
} sim_press;

typedef struct {
    uint32_t at_ms;
    uint64_t hash;
    const char *ppm_path; /* optional */
    bool taken;
} sim_capture;

typedef struct {
    uint16_t fb[AE_SCREEN_W * AE_SCREEN_H];
    uint64_t now_ms;
    uint32_t end_ms;
    sim_press presses[SIM_MAX_PRESSES];
    uint32_t press_count;
    sim_capture captures[SIM_MAX_CAPTURES];
    uint32_t capture_count;
    int battery;
} sim_ctx;

void sim_init(sim_ctx *s, uint32_t end_ms);
bool sim_add_press(sim_ctx *s, uint32_t at_ms, uint32_t key);
bool sim_add_capture(sim_ctx *s, uint32_t at_ms, const char *ppm_path);
ae_hal sim_headless_hal(sim_ctx *s);
uint64_t sim_hash(const sim_ctx *s);
bool sim_write_ppm(const sim_ctx *s, const char *path);

#ifdef _WIN32
/* Opens a window and returns a real-time HAL. */
bool sim_window_open(sim_ctx *s, ae_hal *hal);
#endif

#endif
