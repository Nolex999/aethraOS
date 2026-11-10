"""Stand-in for dfu-util, backed by a file. Used only by test_flash.py.

Environment: FAKE_FLASH (image path), FAKE_LOG (appended commands),
FAKE_LIST ("one", "none", "two", "wrong"), FAKE_CORRUPT_WRITE ("1" flips a byte).
"""
import os
import pathlib
import sys

LAYOUT = "@Flash/0x90000000/08*004Kg,01*032Kg,63*064Kg,64*064Kg"
BASE = 0x90000000
args = sys.argv[1:]
flash = pathlib.Path(os.environ["FAKE_FLASH"])
with open(os.environ["FAKE_LOG"], "a") as log:
    log.write(" ".join(args) + "\n")

offline = flash.with_suffix(".offline")
if "-l" in args and offline.exists():
    remaining = int(offline.read_text())
    if remaining > 0:
        offline.write_text(str(remaining - 1))
        sys.exit(0)

if "-l" in args:
    mode = os.environ.get("FAKE_LIST", "one")
    line = f'Found DFU: [0483:a291] ver=0100, devnum=9, cfg=1, intf=0, path="1-2", alt=0, name="{{}}", serial="X"'
    if mode == "one":
        print(line.format(LAYOUT))
    elif mode == "two":
        print(line.format(LAYOUT))
        print(line.format(LAYOUT))
    elif mode == "wrong":
        print(line.format("@Flash/0x08000000/16*128Kg"))
    sys.exit(0)

spec = args[args.index("-s") + 1].split(":")
start = int(spec[0], 16) - BASE
image = bytearray(flash.read_bytes())

if "-U" in args:
    size = int(spec[1], 16)
    pathlib.Path(args[args.index("-U") + 1]).write_bytes(bytes(image[start:start + size]))
elif "-D" in args:
    payload = bytearray(pathlib.Path(args[args.index("-D") + 1]).read_bytes())
    if os.environ.get("FAKE_CORRUPT_WRITE") == "1":
        payload[0] ^= 0xFF
    image[start:start + len(payload)] = payload
    flash.write_bytes(bytes(image))
else:
    sys.exit(3)
