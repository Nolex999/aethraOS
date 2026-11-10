/*
 * Replays a key script in virtual time and compares frames with golden hashes.
 *   test_replay GOLDEN_FILE            compare
 *   test_replay GOLDEN_FILE --update   rewrite the golden file after human review
 */
#include <stdio.h>
#include <string.h>

#include "platform/sim/sim_platform.h"
#include "shell/ae_shell.h"

static sim_ctx sim;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

static const uint32_t CAPTURE_MS[] = {900, 2500, 4200, 5000, 6500, 7400, 8000, 8600, 9600, 10300};

static void script(void)
{
    sim_init(&sim, 11000u);
    for (size_t i = 0; i < sizeof CAPTURE_MS / sizeof CAPTURE_MS[0]; i++) {
        (void)sim_add_capture(&sim, CAPTURE_MS[i], NULL);
    }
    (void)sim_add_press(&sim, 8200, AE_KEY_RIGHT);
    (void)sim_add_press(&sim, 9000, AE_KEY_OK);
    (void)sim_add_press(&sim, 10000, AE_KEY_TOOLBOX);
}

static void test_skip_runs_every_init_step(void)
{
    ae_run_report r;
    sim_init(&sim, 2000u);
    (void)sim_add_press(&sim, 500, AE_KEY_OK);
    ae_hal hal = sim_headless_hal(&sim);
    memset(&r, 0, sizeof r);
    CHECK(ae_system_run(&hal, &r) == AE_OK);
    CHECK(r.boot_skipped);
    CHECK(r.init_steps_done == 4);
}

int main(int argc, char **argv)
{
    ae_run_report report;
    if (argc < 2) {
        fprintf(stderr, "usage: test_replay GOLDEN_FILE [--update]\n");
        return 2;
    }
    bool update = argc > 2 && strcmp(argv[2], "--update") == 0;

    script();
    ae_hal hal = sim_headless_hal(&sim);
    memset(&report, 0, sizeof report);
    CHECK(ae_system_run(&hal, &report) == AE_OK);
    CHECK(!report.boot_skipped);
    CHECK(report.init_steps_done == 4);
    CHECK(report.events_dropped == 0);
    CHECK(report.arena_peak > 0 && report.arena_peak <= AE_ARENA_BYTES);
    printf("arena peak: %u bytes\n", (unsigned)report.arena_peak);

    if (update) {
        FILE *f = fopen(argv[1], "w");
        CHECK(f != NULL);
        for (uint32_t i = 0; f != NULL && i < sim.capture_count; i++) {
            fprintf(f, "%u %016llx\n", (unsigned)sim.captures[i].at_ms, (unsigned long long)sim.captures[i].hash);
        }
        if (f != NULL) {
            fclose(f);
        }
    } else {
        FILE *f = fopen(argv[1], "r");
        CHECK(f != NULL);
        for (uint32_t i = 0; f != NULL && i < sim.capture_count; i++) {
            unsigned at = 0;
            unsigned long long want = 0;
            CHECK(fscanf(f, "%u %llx", &at, &want) == 2);
            CHECK(at == sim.captures[i].at_ms);
            CHECK(sim.captures[i].taken);
            if (want != sim.captures[i].hash) {
                printf("FAIL frame at %u ms: got %016llx want %016llx\n", at, (unsigned long long)sim.captures[i].hash, want);
                failures++;
            }
        }
        if (f != NULL) {
            fclose(f);
        }
    }

    test_skip_runs_every_init_step();
    printf(failures ? "%d FAILED\n" : "all tests passed\n", failures);
    return failures ? 1 : 0;
}
