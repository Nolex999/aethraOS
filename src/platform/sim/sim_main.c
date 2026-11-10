/*
 * AETHRA simulator.
 *   aethra_sim                       opens a window (Windows)
 *   aethra_sim --headless ...        virtual time, no window
 *     --end MS                       stop time (default 12000)
 *     --press MS:KEY                 left up down right ok back toolbox var
 *     --capture MS[:FILE.ppm]        prints the frame hash, optionally saves it
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform/sim/sim_platform.h"
#include "shell/ae_shell.h"

static sim_ctx sim;

static int key_by_name(const char *name)
{
    static const struct {
        const char *name;
        int key;
    } table[] = {{"left", AE_KEY_LEFT}, {"up", AE_KEY_UP},   {"down", AE_KEY_DOWN},       {"right", AE_KEY_RIGHT},
                 {"ok", AE_KEY_OK},     {"back", AE_KEY_BACK}, {"toolbox", AE_KEY_TOOLBOX}, {"var", AE_KEY_VAR}};
    for (size_t i = 0; i < sizeof table / sizeof table[0]; i++) {
        if (strcmp(table[i].name, name) == 0) {
            return table[i].key;
        }
    }
    return -1;
}

static int run_headless(int argc, char **argv)
{
    sim_init(&sim, 12000u);
    for (int i = 2; i < argc; i++) {
        if (i + 1 >= argc) {
            fprintf(stderr, "missing value for %s\n", argv[i]);
            return 2;
        }
        char *arg = argv[++i];
        char *rest = strchr(arg, ':');
        if (strcmp(argv[i - 1], "--end") == 0) {
            sim.end_ms = (uint32_t)strtoul(arg, NULL, 10);
        } else if (strcmp(argv[i - 1], "--press") == 0 && rest != NULL) {
            int key = key_by_name(rest + 1);
            if (key < 0 || !sim_add_press(&sim, (uint32_t)strtoul(arg, NULL, 10), (uint32_t)key)) {
                fprintf(stderr, "bad --press %s\n", arg);
                return 2;
            }
        } else if (strcmp(argv[i - 1], "--capture") == 0) {
            if (!sim_add_capture(&sim, (uint32_t)strtoul(arg, NULL, 10), rest != NULL ? rest + 1 : NULL)) {
                fprintf(stderr, "too many captures\n");
                return 2;
            }
        } else {
            fprintf(stderr, "unknown option %s\n", argv[i - 1]);
            return 2;
        }
    }

    ae_hal hal = sim_headless_hal(&sim);
    ae_run_report report;
    memset(&report, 0, sizeof report);
    if (ae_system_run(&hal, &report) != AE_OK) {
        return 1;
    }
    for (uint32_t i = 0; i < sim.capture_count; i++) {
        printf("capture %u ms: %016llx%s\n", (unsigned)sim.captures[i].at_ms, (unsigned long long)sim.captures[i].hash,
               sim.captures[i].taken ? "" : " (not reached)");
    }
    printf("arena peak %u bytes, steps %u, boot skipped %d, dropped events %u\n", (unsigned)report.arena_peak,
           (unsigned)report.init_steps_done, report.boot_skipped, (unsigned)report.events_dropped);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc >= 2 && strcmp(argv[1], "--headless") == 0) {
        return run_headless(argc, argv);
    }
#ifdef _WIN32
    ae_hal hal;
    ae_run_report report;
    sim_init(&sim, 0);
    if (!sim_window_open(&sim, &hal)) {
        fprintf(stderr, "cannot open window\n");
        return 1;
    }
    puts("Keys: arrows, Enter=OK, Esc=Back, Tab=Toolbox (theme), V=Var");
    return ae_system_run(&hal, &report) == AE_OK ? 0 : 1;
#else
    fprintf(stderr, "this build has no window backend: use --headless\n");
    return 2;
#endif
}
