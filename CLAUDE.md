# AETHRA: working rules

AETHRA is an operating environment for the NumWorks N0120. Read `docs/README.md` for verified facts and `docs/04_architecture.md` for the design.

## Safety rules

1. Never write to a calculator, automatically or otherwise. No `dfu-util -D`, no `--write` on the flasher, no `nwlink install-nwa`. The user runs these.
2. Never touch internal flash, option bytes, the bootloader or the trust chain. Never use an exploit to gain privilege.
3. Never disable, hide or imitate exam mode.
4. Never commit firmware dumps or IDA databases, and never copy code from them.
5. Do not reuse Epsilon source until its license has been read and recorded.
6. Never hardcode credentials.

## Engineering rules

- `scripts/test` must pass after every change. Report real results only.
- Respect the layers in `docs/04_architecture.md`: `tools/analysis/check_layers.py` enforces them.
- `-Wall -Wextra -Werror`, no heap, no floating point in the boot path, explicit error codes.
- Golden frames change only after a human has looked at the new image: `test_replay GOLDEN --update`.
- A result from the simulator is `sim`. Only the user can produce a `device` result.
- A task is done when its verification has run and the evidence is in `STATUS.md`.

## Commands

`scripts/build [preset]`, `scripts/test`, `scripts/sim`, `scripts/lint`, `scripts/package`, `scripts/diag`.

Hardware constraints that shape the code: at most 153,436 bytes of RAM in T1, no full framebuffer, no GPIO or UART access from an app, about 43 KB of user storage.
