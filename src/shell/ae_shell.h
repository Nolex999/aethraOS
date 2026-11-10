/* Boot sequence, home screen and main loop. */
#ifndef AETHRA_SHELL_H
#define AETHRA_SHELL_H

#include "core/ae_core.h"
#include "gfx/ae_gfx.h"
#include "hal/ae_hal.h"

#define AE_ARENA_BYTES (64u * 1024u)
#define AE_TILE_COUNT 6u

typedef struct {
    ae_gfx gfx;
    ae_dirty dirty;
    uint32_t theme;
    uint32_t focus;
    uint32_t last_seconds;
    const char *toast;
    uint64_t toast_until_ms;
} ae_home;

void ae_home_init(ae_home *h, const ae_hal *hal);
void ae_home_handle(ae_home *h, ae_event ev, uint64_t now_ms);
void ae_home_tick(ae_home *h, uint64_t now_ms);
void ae_home_render(ae_home *h, uint64_t now_ms);

typedef struct {
    size_t arena_peak;
    uint32_t events_dropped;
    uint32_t pixels_pushed;
    uint32_t init_steps_done;
    bool boot_skipped;
} ae_run_report;

/* Plays the boot animation, runs the init steps, then the home screen until should_quit. */
ae_err ae_system_run(const ae_hal *hal, ae_run_report *report);

/* Retained system log, for diagnostics and tests. */
const ae_log *ae_system_log(void);

#endif
