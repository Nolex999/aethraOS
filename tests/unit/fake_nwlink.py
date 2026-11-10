"""Stand-in for nwlink. FAKE_NWLINK_OFFSET is the byte it changes in the image."""
import os
import pathlib
import sys

assert sys.argv[1] == "install-nwa"
flash = pathlib.Path(os.environ["FAKE_FLASH"])
image = bytearray(flash.read_bytes())
image[int(os.environ["FAKE_NWLINK_OFFSET"], 16)] ^= 0x55
flash.write_bytes(bytes(image))
# Like the real tool, leave DFU: the calculator disappears for a few polls.
if "FAKE_NWLINK_OFFLINE_POLLS" in os.environ:
    flash.with_suffix(".offline").write_text(os.environ["FAKE_NWLINK_OFFLINE_POLLS"])
