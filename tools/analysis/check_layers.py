#!/usr/bin/env python3
"""Fails if a source file includes a layer it is not allowed to depend on."""
import pathlib
import re
import sys

ALLOWED = {
    "core": {"core"},
    "hal": {"hal"},
    "gfx": {"gfx", "hal"},
    "boot_anim": {"boot_anim", "gfx"},
    "shell": {"shell", "core", "hal", "gfx", "boot_anim"},
    "platform": {"platform", "shell", "core", "hal", "gfx", "boot_anim"},
}
INCLUDE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.M)


def violations(root: pathlib.Path):
    for path in sorted(root.rglob("*.[ch]")):
        layer = path.relative_to(root).parts[0]
        text = path.read_text(encoding="utf-8")
        if "eadk.h" in text and "#include <eadk.h>" in text and "eadk" not in path.parts:
            yield f"{path}: only platform/eadk may include eadk.h"
        for inc in INCLUDE.findall(text):
            if "/" not in inc:
                continue  # local to the same directory
            target = inc.split("/")[0]
            if target == "..":
                continue
            if target not in ALLOWED.get(layer, set()):
                yield f"{path}: layer '{layer}' must not include '{inc}'"


def main() -> int:
    found = list(violations(pathlib.Path(sys.argv[1])))
    for v in found:
        print(v)
    print(f"{len(found)} layering violation(s)")
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main())
