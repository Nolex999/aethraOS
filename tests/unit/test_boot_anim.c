#include <stdio.h>
#include <string.h>

#include "boot_anim/boot_anim.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

static ab_grid a, b;

static void test_deterministic(void)
{
    ab_status st = {"stage", "tag", 50};
    for (uint32_t f = 0; f < AB_FRAME_COUNT; f += 7) {
        ab_render(&a, f, &st);
        ab_render(&b, f, &st);
        int same = 1;
        for (int y = 0; y < AB_ROWS; y++) {
            for (int x = 0; x < AB_COLS; x++) {
                if (ab_cell_differs(a.cells[y][x], b.cells[y][x])) {
                    same = 0;
                }
            }
        }
        CHECK(same);
    }
}

static void test_only_printable_ascii(void)
{
    ab_status st = {"bad\x01\x7f\tstage", "tag\n", 100};
    for (uint32_t f = 0; f < AB_FRAME_COUNT; f++) {
        ab_render(&a, f, &st);
        for (int y = 0; y < AB_ROWS; y++) {
            for (int x = 0; x < AB_COLS; x++) {
                CHECK(a.cells[y][x].ch >= 32 && a.cells[y][x].ch <= 126);
            }
        }
    }
}

static void test_end_of_animation(void)
{
    ab_status st = {0, 0, 0};
    CHECK(!ab_render(&a, AB_FRAME_COUNT - 1, &st));
    CHECK(ab_render(&a, AB_FRAME_COUNT, &st));
    for (int y = 0; y < AB_ROWS; y++) {
        for (int x = 0; x < AB_COLS; x++) {
            CHECK(a.cells[y][x].ch == ' ');
        }
    }
}

static void test_hostile_status_stays_in_grid(void)
{
    char longtext[400];
    memset(longtext, 'x', sizeof longtext - 1);
    longtext[sizeof longtext - 1] = '\0';
    ab_status st = {longtext, longtext, 255};
    ab_render(&a, 195, &st);
    CHECK(a.cells[22][AB_COLS - 1].ch == ' ');
    /* Percent above 100 is clamped: the bar must be exactly full, not overrun. */
    CHECK(a.cells[21][3].ch == '=');
    CHECK(a.cells[21][AB_COLS - 9].ch == ']');
}

static void test_null_status(void)
{
    ab_status st = {0, 0, 0};
    CHECK(!ab_render(&a, 195, &st));
    CHECK(!ab_render(&a, 195, NULL));
}

static void test_progress_comes_from_caller(void)
{
    ab_status zero = {0, 0, 0};
    ab_status full = {0, 0, 100};
    ab_render(&a, 195, &zero);
    ab_render(&b, 195, &full);
    CHECK(a.cells[21][3].ch == '.');
    CHECK(b.cells[21][3].ch == '=');
}

static void test_blit_stays_in_framebuffer(void)
{
    static uint16_t fb[AB_SCREEN_W * AB_SCREEN_H + 2];
    ab_status st = {"s", "t", 10};
    fb[AB_SCREEN_W * AB_SCREEN_H] = 0xBEEF;
    fb[AB_SCREEN_W * AB_SCREEN_H + 1] = 0xBEEF;
    ab_render(&a, 100, &st);
    ab_grid_blit(&a, 0, fb);
    CHECK(fb[AB_SCREEN_W * AB_SCREEN_H] == 0xBEEF);
    CHECK(fb[AB_SCREEN_W * AB_SCREEN_H + 1] == 0xBEEF);
}

static void test_dirty_cells_ignore_blank_colour(void)
{
    ab_cell x = {0x1234, ' '};
    ab_cell y = {0x4321, ' '};
    ab_cell z = {0x4321, '#'};
    CHECK(!ab_cell_differs(x, y));
    CHECK(ab_cell_differs(y, z));
}

static void report_dirty_cost(void)
{
    ab_status st = {"s", "t", 40};
    long total = 0;
    long peak = 0;
    ab_render(&a, 0, &st);
    for (uint32_t f = 1; f < AB_FRAME_COUNT; f++) {
        long dirty = 0;
        ab_render(&b, f, &st);
        for (int y = 0; y < AB_ROWS; y++) {
            for (int x = 0; x < AB_COLS; x++) {
                if (ab_cell_differs(a.cells[y][x], b.cells[y][x])) {
                    dirty++;
                }
            }
        }
        a = b;
        total += dirty;
        if (dirty > peak) {
            peak = dirty;
        }
    }
    printf("dirty cells per frame: mean %ld, peak %ld, of %d\n", total / (long)(AB_FRAME_COUNT - 1), peak, AB_COLS * AB_ROWS);
}

int main(void)
{
    test_deterministic();
    test_only_printable_ascii();
    test_end_of_animation();
    test_hostile_status_stays_in_grid();
    test_null_status();
    test_progress_comes_from_caller();
    test_blit_stays_in_framebuffer();
    test_dirty_cells_ignore_blank_colour();
    report_dirty_cost();
    printf(failures ? "%d FAILED\n" : "all tests passed\n", failures);
    return failures ? 1 : 0;
}
