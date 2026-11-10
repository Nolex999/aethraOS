#include <stdio.h>
#include <string.h>

#include "core/ae_core.h"
#include "gfx/ae_gfx.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

static void test_arena(void)
{
    static uint8_t buf[64];
    ae_arena a;
    ae_arena_init(&a, buf, sizeof buf);
    uint8_t *p = ae_arena_alloc(&a, 3, 1);
    uint32_t *q = ae_arena_alloc(&a, 4, 4);
    CHECK(p == buf);
    CHECK(((uintptr_t)q & 3u) == 0);
    CHECK(ae_arena_alloc(&a, 1000, 1) == NULL);
    CHECK(ae_arena_alloc(&a, 1, 3) == NULL);
    CHECK(ae_arena_alloc(&a, (size_t)-1, 1) == NULL);
    size_t peak = a.peak;
    ae_arena_reset(&a);
    CHECK(a.used == 0 && a.peak == peak);
}

static void test_events(void)
{
    ae_event_queue q;
    ae_event e = {AE_EV_KEY_DOWN, 0};
    ae_events_init(&q);
    CHECK(ae_events_pop(&q, &e) == AE_ERR_EMPTY);
    for (uint32_t i = 0; i < AE_EVENT_QUEUE_LEN; i++) {
        e.code = i;
        CHECK(ae_events_push(&q, e) == AE_OK);
    }
    CHECK(ae_events_push(&q, e) == AE_ERR_FULL);
    CHECK(q.dropped == 1);
    for (uint32_t i = 0; i < AE_EVENT_QUEUE_LEN; i++) {
        CHECK(ae_events_pop(&q, &e) == AE_OK);
        CHECK(e.code == i);
    }
}

static void test_timers(void)
{
    ae_timers t;
    ae_event_queue q;
    ae_event e;
    ae_timers_init(&t);
    ae_events_init(&q);
    CHECK(ae_timers_start(&t, 7, 100, 50, true) == AE_OK);
    CHECK(ae_timers_start(&t, 8, 100, 20, false) == AE_OK);
    ae_timers_poll(&t, 110, &q);
    CHECK(q.count == 0);
    ae_timers_poll(&t, 120, &q);
    CHECK(q.count == 1);
    CHECK(ae_events_pop(&q, &e) == AE_OK && e.code == 8);
    ae_timers_poll(&t, 151, &q);
    CHECK(ae_events_pop(&q, &e) == AE_OK && e.code == 7);
    ae_timers_poll(&t, 152, &q);
    CHECK(q.count == 0);
    ae_timers_poll(&t, 202, &q);
    CHECK(q.count == 1);
    ae_timers_stop(&t, 7);
    ae_events_init(&q);
    ae_timers_poll(&t, 1000, &q);
    CHECK(q.count == 0);
    for (uint32_t i = 0; i < AE_TIMER_SLOTS; i++) {
        CHECK(ae_timers_start(&t, 100 + i, 0, 1, false) == AE_OK);
    }
    CHECK(ae_timers_start(&t, 999, 0, 1, false) == AE_ERR_FULL);
}

static void test_log(void)
{
    ae_log l;
    char out[AE_LOG_BYTES + 16];
    ae_log_init(&l);
    ae_log_write(&l, "one");
    ae_log_write(&l, "two");
    CHECK(ae_log_read(&l, out, sizeof out) == 8);
    CHECK(strcmp(out, "one\ntwo\n") == 0);
    for (int i = 0; i < 2000; i++) {
        ae_log_write(&l, "filler line");
    }
    ae_log_write(&l, "last");
    uint32_t n = ae_log_read(&l, out, sizeof out);
    CHECK(n > 0 && n < AE_LOG_BYTES);
    CHECK(strcmp(out + n - 5, "last\n") == 0);
    CHECK(strncmp(out, "filler line\n", 12) == 0);
    CHECK(ae_log_read(&l, out, 4) == 3);
}

static uint32_t fill_pixels;
static void stub_fill(void *c, ae_rect r, uint16_t color)
{
    (void)c;
    (void)color;
    fill_pixels += (uint32_t)r.w * (uint32_t)r.h;
}

static void test_gfx_clip(void)
{
    ae_hal hal;
    ae_gfx g;
    memset(&hal, 0, sizeof hal);
    hal.display_fill = stub_fill;
    ae_gfx_init(&g, &hal);
    ae_rect off = {-10, -10, 20, 20};
    ae_gfx_fill(&g, off, 1);
    CHECK(fill_pixels == 100);
    ae_rect gone = {400, 0, 10, 10};
    ae_gfx_fill(&g, gone, 1);
    CHECK(fill_pixels == 100 && g.calls == 1);
}

static void test_dirty(void)
{
    ae_dirty d;
    ae_dirty_clear(&d);
    ae_rect a = {0, 0, 10, 10};
    ae_rect b = {5, 5, 10, 10};
    ae_rect empty = {0, 0, 0, 5};
    ae_dirty_add(&d, empty);
    CHECK(d.count == 0);
    ae_dirty_add(&d, a);
    ae_dirty_add(&d, b);
    CHECK(d.count == 1 && d.rects[0].w == 15 && d.rects[0].h == 15);
    ae_dirty_clear(&d);
    for (int i = 0; i < 9; i++) {
        ae_rect r = {(int16_t)(i * 30), 0, 10, 10};
        ae_dirty_add(&d, r);
    }
    CHECK(d.count == 1 && d.rects[0].x == 0 && d.rects[0].w == 250);
}

static void test_themes(void)
{
    CHECK(ae_theme_get(0)->name != NULL);
    CHECK(ae_theme_get(AE_THEME_COUNT) == ae_theme_get(0));
}

int main(void)
{
    test_arena();
    test_events();
    test_timers();
    test_log();
    test_gfx_clip();
    test_dirty();
    test_themes();
    printf(failures ? "%d FAILED\n" : "all tests passed\n", failures);
    return failures ? 1 : 0;
}
