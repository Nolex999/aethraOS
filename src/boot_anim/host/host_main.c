/*
 * Host runner for the boot animation.
 *   boot_anim_host              plays it in a truecolor terminal
 *   boot_anim_host --raw FILE   writes every frame as raw RGB565 little-endian
 */
#include <stdio.h>
#include <string.h>

#include "boot_anim/boot_anim.h"

#ifdef _WIN32
#include <windows.h>
static void enable_ansi(void)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(out, &mode)) {
        SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}
static void sleep_ms(unsigned ms) { Sleep(ms); }
#else
#include <unistd.h>
static void enable_ansi(void) {}
static void sleep_ms(unsigned ms) { usleep(ms * 1000u); }
#endif

/* Demo stages only: on a device the caller reports its real init steps. */
static ab_status demo_status(uint32_t frame)
{
    static const char *const stages[] = {"sim: memory arenas", "sim: graphics", "sim: input", "sim: storage", "sim: shell"};
    ab_status st;
    uint32_t pct = frame <= 150 ? 0 : (frame - 150) * 100 / 46;
    if (pct > 100) {
        pct = 100;
    }
    st.percent = (uint8_t)pct;
    st.stage = stages[pct >= 100 ? 4 : pct / 25];
    st.tag = "AETHRA 0.0.1  //  simulator build";
    return st;
}

static int write_raw(const char *path)
{
    static ab_grid grid;
    static uint16_t fb[AB_SCREEN_W * AB_SCREEN_H];
    FILE *out = fopen(path, "wb");
    if (!out) {
        perror(path);
        return 1;
    }
    for (uint32_t f = 0; f < AB_FRAME_COUNT; f++) {
        ab_status st = demo_status(f);
        ab_render(&grid, f, &st);
        ab_grid_blit(&grid, 0x0000, fb);
        if (fwrite(fb, sizeof(fb), 1, out) != 1) {
            perror("fwrite");
            fclose(out);
            return 1;
        }
    }
    fclose(out);
    return 0;
}

static void play_terminal(void)
{
    static ab_grid grid;
    enable_ansi();
    fputs("\x1b[2J\x1b[?25l", stdout);
    for (uint32_t f = 0; f < AB_FRAME_COUNT; f++) {
        ab_status st = demo_status(f);
        ab_render(&grid, f, &st);
        fputs("\x1b[H", stdout);
        for (int y = 0; y < AB_ROWS; y++) {
            uint16_t last = 0;
            int have = 0;
            for (int x = 0; x < AB_COLS; x++) {
                ab_cell c = grid.cells[y][x];
                if (c.ch != ' ' && (!have || c.fg != last)) {
                    int r = ((c.fg >> 11) & 0x1F) * 255 / 31;
                    int g = ((c.fg >> 5) & 0x3F) * 255 / 63;
                    int b = (c.fg & 0x1F) * 255 / 31;
                    printf("\x1b[38;2;%d;%d;%dm", r, g, b);
                    last = c.fg;
                    have = 1;
                }
                putchar(c.ch);
            }
            fputs("\x1b[K\n", stdout);
        }
        fflush(stdout);
        sleep_ms(1000 / AB_FPS);
    }
    fputs("\x1b[0m\x1b[?25h\n", stdout);
}

int main(int argc, char **argv)
{
    if (argc == 3 && strcmp(argv[1], "--raw") == 0) {
        return write_raw(argv[2]);
    }
    if (argc != 1) {
        fprintf(stderr, "usage: %s [--raw FILE]\n", argv[0]);
        return 2;
    }
    play_terminal();
    return 0;
}
