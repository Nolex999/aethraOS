/*
 * AETHRA boot animation: an ASCII ghost materialises, charges a shadow ball,
 * and the blast writes the logo.
 *
 * The renderer is stateless and deterministic: a frame index and a status
 * produce one character grid. It uses no floating point, no heap and no libc,
 * so the same file builds for the host and for Cortex-M7.
 */
#ifndef AETHRA_BOOT_ANIM_H
#define AETHRA_BOOT_ANIM_H

#include <stdbool.h>
#include <stdint.h>

#define AB_SCREEN_W 320
#define AB_SCREEN_H 240
#define AB_CELL_W 6
#define AB_CELL_H 10
#define AB_COLS 53
#define AB_ROWS 24
#define AB_MARGIN_X ((AB_SCREEN_W - AB_COLS * AB_CELL_W) / 2)

#define AB_FPS 30
#define AB_FRAME_COUNT 210u

typedef struct {
    uint16_t fg; /* RGB565 */
    char ch;     /* printable ASCII, ' ' when empty */
} ab_cell;

typedef struct {
    ab_cell cells[AB_ROWS][AB_COLS];
} ab_grid;

/*
 * Real boot information supplied by the caller. The progress bar only shows
 * what the caller reports: the animation never invents progress.
 * Strings may be NULL. Control characters are replaced and long strings are
 * clipped to the grid.
 */
typedef struct {
    const char *stage;
    const char *tag;
    uint8_t percent; /* clamped to 100 */
} ab_status;

/* Renders one frame. Returns true once frame >= AB_FRAME_COUNT (grid is blank). */
bool ab_render(ab_grid *grid, uint32_t frame, const ab_status *status);

/* Rasterises one cell into AB_CELL_W * AB_CELL_H pixels, row by row. */
void ab_cell_pixels(ab_cell cell, uint16_t bg, uint16_t *pixels);

/* Rasterises a grid into a full AB_SCREEN_W * AB_SCREEN_H RGB565 framebuffer. */
void ab_grid_blit(const ab_grid *grid, uint16_t bg, uint16_t *framebuffer);

/* True when a cell must be redrawn. Colour changes on blank cells do not count. */
bool ab_cell_differs(ab_cell a, ab_cell b);

#endif
