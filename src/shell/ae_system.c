#include "shell/ae_shell.h"

#include "boot_anim/boot_anim.h"

enum { TIMER_STATUS = 1, FIRST_STEP_FRAME = 150, FRAMES_PER_STEP = 9, HOME_SLEEP_MS = 16, FRAME_MS = 1000 / AB_FPS };

typedef struct {
    ae_arena arena;
    ae_event_queue events;
    ae_timers timers;
    ae_log log;
    ae_home home;
    const ae_hal *hal;
    uint8_t arena_buf[AE_ARENA_BYTES];
} ae_system;

static ae_system sys;

const ae_log *ae_system_log(void)
{
    return &sys.log;
}

typedef void (*init_fn)(void);

static void step_events(void) { ae_events_init(&sys.events); }
static void step_timers(void) { ae_timers_init(&sys.timers); }
static void step_home(void) { ae_home_init(&sys.home, sys.hal); }
static void step_status_timer(void)
{
    (void)ae_timers_start(&sys.timers, TIMER_STATUS, sys.hal->time_ms(sys.hal->ctx), 1000u, true);
}

/* Every step does real work; the animation only paces when each one runs. */
static const struct {
    const char *name;
    init_fn fn;
} STEPS[] = {
    {"events", step_events},
    {"timers", step_timers},
    {"graphics + shell", step_home},
    {"status timer", step_status_timer},
};
#define STEP_COUNT (sizeof(STEPS) / sizeof(STEPS[0]))

static void run_step(uint32_t index)
{
    STEPS[index].fn();
    ae_log_write(&sys.log, STEPS[index].name);
}

static void push_grid(const ab_grid *cur, const ab_grid *prev, bool force)
{
    uint16_t pixels[AB_CELL_W * AB_CELL_H];
    for (int r = 0; r < AB_ROWS; r++) {
        for (int c = 0; c < AB_COLS; c++) {
            if (!force && !ab_cell_differs(cur->cells[r][c], prev->cells[r][c])) {
                continue;
            }
            ab_cell_pixels(cur->cells[r][c], 0x0000, pixels);
            ae_rect rect = {(int16_t)(AB_MARGIN_X + c * AB_CELL_W), (int16_t)(r * AB_CELL_H), AB_CELL_W, AB_CELL_H};
            sys.hal->display_push(sys.hal->ctx, rect, pixels);
        }
    }
}

/* Returns true if the user skipped the animation. */
static bool boot(ae_run_report *report)
{
    const ae_rect screen = {0, 0, AE_SCREEN_W, AE_SCREEN_H};
    ab_grid *cur = ae_arena_alloc(&sys.arena, sizeof(ab_grid), _Alignof(ab_grid));
    ab_grid *prev = ae_arena_alloc(&sys.arena, sizeof(ab_grid), _Alignof(ab_grid));
    uint64_t start = sys.hal->time_ms(sys.hal->ctx);
    uint32_t done = 0;
    bool skipped = false;

    if (cur != NULL && prev != NULL) {
        sys.hal->display_fill(sys.hal->ctx, screen, 0x0000);
        (void)ab_render(prev, AB_FRAME_COUNT, NULL);
        for (;;) {
            uint32_t frame = (uint32_t)((sys.hal->time_ms(sys.hal->ctx) - start) / FRAME_MS);
            while (done < STEP_COUNT && frame >= FIRST_STEP_FRAME + done * FRAMES_PER_STEP) {
                run_step(done++);
            }
            ab_status st;
            st.stage = done == 0 ? "starting" : STEPS[done - 1].name;
            st.tag = "AETHRA 0.1.0";
            st.percent = (uint8_t)(done * 100u / STEP_COUNT);
            if (ab_render(cur, frame, &st)) {
                break;
            }
            push_grid(cur, prev, false);
            *prev = *cur;
            if (sys.hal->keys_scan(sys.hal->ctx) != 0 || sys.hal->should_quit(sys.hal->ctx)) {
                skipped = true;
                break;
            }
            sys.hal->sleep_ms(sys.hal->ctx, FRAME_MS);
        }
    } else {
        ae_log_write(&sys.log, "boot: animation skipped, no memory");
    }

    while (done < STEP_COUNT) {
        run_step(done++);
    }
    report->init_steps_done = done;
    report->boot_skipped = skipped;
    report->arena_peak = sys.arena.peak;
    ae_arena_reset(&sys.arena);
    return skipped;
}

ae_err ae_system_run(const ae_hal *hal, ae_run_report *report)
{
    uint64_t held = 0;

    if (hal == NULL || report == NULL) {
        return AE_ERR_ARG;
    }
    sys.hal = hal;
    ae_log_init(&sys.log);
    ae_log_write(&sys.log, "AETHRA 0.1.0");
    ae_arena_init(&sys.arena, sys.arena_buf, sizeof sys.arena_buf);

    (void)boot(report);
    /* Ignore keys still held from the boot skip: they are not new presses. */
    held = hal->keys_scan(hal->ctx);

    while (!hal->should_quit(hal->ctx)) {
        uint64_t now = hal->time_ms(hal->ctx);
        uint64_t keys = hal->keys_scan(hal->ctx);
        uint64_t changed = keys ^ held;
        for (uint32_t k = 0; k < AE_KEY_COUNT; k++) {
            if (changed & (1ull << k)) {
                ae_event ev = {(keys >> k) & 1u ? AE_EV_KEY_DOWN : AE_EV_KEY_UP, k};
                (void)ae_events_push(&sys.events, ev);
            }
        }
        held = keys;
        ae_timers_poll(&sys.timers, now, &sys.events);

        ae_event ev;
        while (ae_events_pop(&sys.events, &ev) == AE_OK) {
            ae_home_handle(&sys.home, ev, now);
        }
        ae_home_tick(&sys.home, now);
        ae_home_render(&sys.home, now);
        hal->sleep_ms(hal->ctx, HOME_SLEEP_MS);
    }

    report->events_dropped = sys.events.dropped;
    report->pixels_pushed = sys.home.gfx.pixels_pushed;
    return AE_OK;
}
