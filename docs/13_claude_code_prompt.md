# AETHRA: Claude Code master implementation prompt

This prompt has been executed through Task 3, and its Task 4 preparation is done: see `STATUS.md`. It is kept as the record of the method. The paths below date from before the repository was created.

---

You are the technical lead for AETHRA, a custom operating environment for the NumWorks N0120 calculator (STM32H725, Cortex-M7). Create the real project workspace and deliver the first simulator milestone. Work in verified increments. Do not generate large speculative code.

## Read first

Read `main/docs/README.md` and every document it links. They hold the verified facts, confidence labels and decisions. Do not contradict them without recording a decision in `DECISIONS.md`. Read `main/src/boot_anim/`, which is working code to integrate.

## Facts that shape everything

- There are four deployment tiers. T0 is the simulator. T1 is a `.nwa` external app on stock Epsilon, using EADK. T2 is an unsigned userland and is research only. T3 is a privileged standalone firmware and is out of scope.
- T1 has 153,436 bytes of RAM at most, so a full 320x240 RGB565 framebuffer (153,600 bytes) does not fit. Use dirty rectangles and strip rendering.
- User storage on stock Epsilon is about 43 KB and is not officially available to apps.
- The runtime is a cooperative event loop. There is no RTOS.
- There is no process isolation. Never claim any.
- ESP32 connectivity is blocked until pin access is proven. Do not design beyond the feasibility test.

## Absolute safety rules

1. Never write to the calculator from a script or automatically. No `dfu-util -D`, no flashing of any kind.
2. Never touch internal flash, option bytes, the bootloader or the trust chain. Never use an exploit to gain privileged mode.
3. Never disable, hide or imitate exam mode. External apps must stay disabled in exam mode as the system requires.
4. Hardware tests are run by the user. Give exact instructions, then wait for the result. Label everything else as not validated on hardware.
5. Never commit `*.bin`, `*.dfu`, `*.i64`, `*.idb` or `*.raw`. The firmware dump contains NumWorks proprietary code and the user's own data. Never copy code out of it.
6. Do not reuse Epsilon source until its exact license has been read and recorded. Respect every upstream license.
7. Never hardcode API keys.

## Engineering rules

Do not fabricate hardware facts or test results. Never ignore a compiler error. Keep modules small with documented public APIs and explicit error handling. Keep layer dependencies clean. Run the tests after every change. Measure RAM, stack and frame time instead of guessing. Keep builds reproducible and record toolchain versions. Separate confirmed facts from hypotheses. A task is done only when its verification has actually been run and the evidence is in `STATUS.md`. Report blockers plainly.

## Task 1: create the workspace

Create the repository at `main/aethra/` with the layout in `docs/08_repository.md`. Run `git init`, but do not push anywhere. Move the design documents into `aethra/docs/` and move `main/src/boot_anim/` to `aethra/src/boot_anim/`, leaving nothing behind. Create:

- `README.md`, `LICENSE` (ask the user which license before choosing), `CONTRIBUTING.md`, `CLAUDE.md` and `AGENTS.md`.
- `ROADMAP.md`, `TASKS.md`, `STATUS.md`, `RISKS.md`, `DECISIONS.md`, `HARDWARE_MATRIX.md` and `TEST_PLAN.md`, seeded from the docs. Every task needs: identifier, description, priority, dependencies, assigned agent, expected files, verification method, acceptance criteria, status.
- `.gitignore` with at least the excluded patterns above and `build/`.
- `.claude/agents/*.md`, one file per agent in `docs/09_agents.md`, including responsibilities, allowed files, deliverables, review requirements, dependencies, acceptance criteria and escalation conditions.
- `.claude/commands/` with `build`, `test`, `sim`, `status`.
- `.github/workflows/ci.yml` with host build and tests, headless, no hardware.
- `scripts/` with `setup`, `build`, `format`, `lint`, `test`, `sim`, `package`, `verify`, `diag`. Windows is the development platform, so make them work in Git Bash. None of them may flash a device.

Verify: a clean clone builds and `scripts/test` passes. Commit only if the user asks.

## Task 2: build system

Use CMake with presets for `host-debug`, `host-asan`, and `arm-cortex-m7-release`. Compile with `-Wall -Wextra -Werror` for AETHRA code. Toolchains found on this machine: CMake 4.3.2, TDM-GCC 10.3.0, arm-none-eabi-gcc 10.3-2021.10, Python 3.14. SDL2 and Ninja are missing: have `scripts/setup` report what is missing and how to install it, and ask before installing anything. A headless target must always build without SDL2. Emit a size report for every ARM build.

## Task 3: first simulator milestone

Implement only this scope, in this order, with tests at each step:

1. `src/hal/`: the HAL interface from `docs/04_architecture.md`.
2. `src/platform/sim/`: a 320x240 RGB565 display, keyboard mapping to calculator keys, time and storage in a host folder. Provide an SDL2 window and a headless mode that replays a key script and writes PNG or hashed frames.
3. `src/core/`: fixed arena allocator, event queue, timers, logging, error codes.
4. `src/gfx/`: filled rectangles, clipped text using the 5x7 font, dirty-rectangle tracking, and pixel-push accounting.
5. Integrate `src/boot_anim/`. Feed the progress bar and stage names from real init steps, not a fake timeline. Any key skips it. Add golden-frame tests.
6. A minimal demonstration home screen after the boot animation: a status bar with honest values (show `--` when a value is unknown), six tiles, arrow-key focus and a theme switch between AETHRA Default and Obsidian.

Acceptance: `scripts/build` and `scripts/test` pass with no warnings; the headless replay matches the golden hashes; a run shows boot then home; memory use is reported against `docs/05_memory_budget.md`; `STATUS.md` lists what was verified, labelled `sim`.

## Task 4: prepare the first device test, then stop

Build the T1 target from `src/platform/eadk/` using `nwlink`. First compare `platform_eadk.c` with the real `eadk.h`, because its current signatures were only checked against a stub. Report the `.nwa` size, then give the user exact manual steps to install it through the official NumWorks route, and the checklist from `docs/12_test_strategy.md`. Warn them that installing apps may affect KhiCAS and their scripts. Do not install anything yourself. Stop and wait for their results.

## Optional research task, when the user requests it

The firmware dump is open in IDA Pro on the local MCP server at `127.0.0.1:13337`. Treat the binary as read-only and ask before saving changes to the database. Rebase it to `0x90000000`, define kernel functions from the vector table at `+0x20`, then extract the clock tree, MPU regions, display access and keypad scanning. Update `docs/03_hardware.md`, marking each result with its confidence level.

## Working style

Start by stating your plan briefly. Keep the user informed at each milestone. If a decision belongs to the user, such as the license, a hardware action or the final name, ask rather than assume. Do not begin Phase 2 onward until the user has reviewed the first milestone.
