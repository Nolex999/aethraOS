#!/usr/bin/env python3
"""AETHRA flasher for the NumWorks N0120 (Epsilon 26.4.0).

Installs external apps and restores the external-apps area. It can only write
inside the two external-apps regions: the kernel, userland, signatures and
bootloader are outside its reach by construction.

Read-only commands: detect, backup, inspect, verify.
Writing commands: install-nwa, restore-apps. Both are dry runs unless --write is
given, and a real write also needs a typed confirmation on an interactive terminal.
"""
import argparse
import hashlib
import os
import pathlib
import re
import shlex
import shutil
import struct
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass
from typing import Callable, List, Optional, Sequence, Tuple

FLASH_BASE = 0x90000000
FLASH_SIZE = 0x800000
SLOT_SIZE = 0x400000
LAYOUT = "@Flash/0x90000000/08*004Kg,01*032Kg,63*064Kg,64*064Kg"
EXPECTED_VERSION = "26.4.0"
SLOT_MAGIC = bytes.fromhex("f00dc0de")
USERLAND_MAGIC = bytes.fromhex("feedc0de")
USERLAND_OFFSET = 0x20000

# The only ranges this tool may ever write. Read from Epsilon 26.4.0 userland headers.
APPS_REGIONS: Tuple[Tuple[int, int], ...] = (
    (0x90180000, 0x903F0000),
    (0x90580000, 0x907F0000),
)
PROTECTED_RANGES: Tuple[Tuple[int, int], ...] = (
    (0x90000000, 0x90180000),
    (0x90400000, 0x90580000),
)

DEFAULT_NWLINK = "npx --yes -- nwlink@0.0.19"


class FlashError(Exception):
    """A refused or failed operation. The message is meant for the user."""


@dataclass
class Slot:
    base: int
    version: str
    image_size: int
    apps_start: int
    apps_end: int


def assert_write_allowed(start: int, end: int) -> None:
    """Raises unless [start, end) is non-empty and inside one external-apps region."""
    if start >= end:
        raise FlashError(f"plage vide ou invalide : {start:#x}-{end:#x}")
    for low, high in APPS_REGIONS:
        if low <= start and end <= high:
            return
    raise FlashError(f"ecriture refusee hors de la zone des apps : {start:#x}-{end:#x}")


def parse_image(data: bytes) -> List[Slot]:
    """Validates a full 8 MiB dump and returns its slots."""
    if len(data) != FLASH_SIZE:
        raise FlashError(f"taille inattendue : {len(data)} octets au lieu de {FLASH_SIZE}")
    slots = []
    for index in range(2):
        off = index * SLOT_SIZE
        if data[off + 8:off + 12] != SLOT_MAGIC or data[off + 0x1C:off + 0x20] != SLOT_MAGIC:
            raise FlashError(f"slot {'AB'[index]} : en-tete absent ou corrompu")
        image_size = struct.unpack_from("<I", data, off + 4)[0]
        version = data[off + 12:off + 20].split(b"\0")[0].decode("ascii", "replace")
        uh = off + USERLAND_OFFSET
        if data[uh:uh + 4] != USERLAND_MAGIC:
            raise FlashError(f"slot {'AB'[index]} : en-tete userland absent")
        apps_start, apps_end = struct.unpack_from("<II", data, uh + 20)
        slots.append(Slot(FLASH_BASE + off, version, image_size, apps_start, apps_end))
    return slots


def check_supported(slots: Sequence[Slot]) -> None:
    for slot, region in zip(slots, APPS_REGIONS):
        if slot.version != EXPECTED_VERSION:
            raise FlashError(f"Epsilon {slot.version} non pris en charge (attendu {EXPECTED_VERSION})")
        if (slot.apps_start, slot.apps_end) != region:
            raise FlashError(f"zone des apps inattendue : {slot.apps_start:#x}-{slot.apps_end:#x}")


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def region_bytes(data: bytes, start: int, end: int) -> bytes:
    return data[start - FLASH_BASE:end - FLASH_BASE]


class Dfu:
    """dfu-util wrapper. `download` is the single place where anything is written."""

    def __init__(self, command: Sequence[str]):
        self.command = list(command)
        self.alt: Optional[int] = None

    def _run(self, args: Sequence[str]) -> str:
        try:
            done = subprocess.run(self.command + list(args), capture_output=True, text=True)
        except OSError as err:
            raise FlashError(f"impossible de lancer dfu-util : {err}") from err
        if done.returncode != 0:
            raise FlashError(f"dfu-util a echoue : {done.stderr.strip() or done.stdout.strip()}")
        return done.stdout

    def detect(self) -> int:
        """Finds exactly one calculator exposing the expected flash layout."""
        found = []
        for line in self._run(["-l"]).splitlines():
            if LAYOUT in line:
                match = re.search(r"alt=(\d+)", line)
                if match:
                    found.append(int(match.group(1)))
        if not found:
            raise FlashError("aucune N0120 detectee : branche-la et laisse-la sur l'ecran de connexion USB")
        if len(found) > 1:
            raise FlashError("plusieurs appareils detectes : n'en branche qu'un seul")
        self.alt = found[0]
        return self.alt

    def upload(self, start: int, size: int) -> bytes:
        if self.alt is None:
            self.detect()
        with tempfile.TemporaryDirectory() as tmp:
            path = pathlib.Path(tmp) / "read.bin"
            self._run(["-a", str(self.alt), "-s", f"{start:#x}:{size:#x}", "-U", str(path)])
            data = path.read_bytes()
        if len(data) != size:
            raise FlashError(f"lecture incomplete : {len(data)} octets sur {size}")
        return data

    def download(self, start: int, payload: bytes) -> None:
        assert_write_allowed(start, start + len(payload))
        if self.alt is None:
            self.detect()
        with tempfile.TemporaryDirectory() as tmp:
            path = pathlib.Path(tmp) / "write.bin"
            path.write_bytes(payload)
            self._run(["-a", str(self.alt), "-s", f"{start:#x}", "-D", str(path)])


def load_backup(path: pathlib.Path) -> Tuple[bytes, List[Slot]]:
    if not path.is_file():
        raise FlashError(f"sauvegarde introuvable : {path}")
    data = path.read_bytes()
    side = path.with_suffix(path.suffix + ".sha256")
    if not side.is_file():
        raise FlashError(f"empreinte manquante : {side}. Refais une sauvegarde avec la commande backup")
    if side.read_text().split()[0] != sha256(data):
        raise FlashError("la sauvegarde ne correspond pas a son empreinte")
    slots = parse_image(data)
    check_supported(slots)
    return data, slots


def check_device_matches(dfu: Dfu, backup: bytes) -> None:
    """Refuses to write if the connected calculator is not the one in the backup."""
    for low, _ in PROTECTED_RANGES:
        offset = low - FLASH_BASE
        if dfu.upload(low, 0x40) != backup[offset:offset + 0x40]:
            raise FlashError("la calculatrice branchee ne correspond pas a la sauvegarde : refais une sauvegarde")


def confirm_write(summary: str, read_line: Callable[[str], str], interactive: bool) -> None:
    """A write needs a person: no terminal means no write."""
    if not interactive:
        raise FlashError("ecriture refusee : un terminal interactif est obligatoire")
    print(summary)
    print("Tape exactement ECRIRE pour continuer, autre chose pour annuler.")
    if read_line("> ").strip() != "ECRIRE":
        raise FlashError("ecriture annulee")


def verify_protected(dfu: Dfu, backup: bytes) -> None:
    for low, high in PROTECTED_RANGES:
        if sha256(dfu.upload(low, high - low)) != sha256(region_bytes(backup, low, high)):
            raise FlashError(f"ALERTE : la zone protegee {low:#x}-{high:#x} a change. Ne debranche pas et ne mets rien a jour")


def cmd_detect(dfu: Dfu) -> None:
    print(f"N0120 detectee, interface alt={dfu.detect()}")
    slots = parse_image_header(dfu)
    for slot in slots:
        print(f"Slot {slot.base:#x} : Epsilon {slot.version}")


def parse_image_header(dfu: Dfu) -> List[Slot]:
    slots = []
    for index in range(2):
        base = FLASH_BASE + index * SLOT_SIZE
        header = dfu.upload(base, 0x20)
        if header[8:12] != SLOT_MAGIC:
            raise FlashError(f"en-tete de slot invalide a {base:#x}")
        version = header[12:20].split(b"\0")[0].decode("ascii", "replace")
        slots.append(Slot(base, version, struct.unpack_from("<I", header, 4)[0], 0, 0))
    return slots


def cmd_backup(dfu: Dfu, out: pathlib.Path) -> None:
    if out.exists():
        raise FlashError(f"{out} existe deja : choisis un autre nom")
    data = dfu.upload(FLASH_BASE, FLASH_SIZE)
    slots = parse_image(data)
    out.write_bytes(data)
    out.with_suffix(out.suffix + ".sha256").write_text(f"{sha256(data)}  {out.name}\n")
    print(f"Sauvegarde ecrite : {out} ({len(data)} octets), Epsilon {slots[0].version}")


def cmd_inspect(path: pathlib.Path) -> None:
    data = path.read_bytes()
    slots = parse_image(data)
    for slot in slots:
        print(f"Slot {slot.base:#x} : Epsilon {slot.version}, image {slot.image_size:#x} octets")
        print(f"  zone des apps {slot.apps_start:#x}-{slot.apps_end:#x}")
        region = region_bytes(data, slot.apps_start, slot.apps_end)
        used = sum(1 for i in range(0, len(region), 0x10000) if region[i:i + 0x10000].count(0xFF) != 0x10000)
        print(f"  {used} blocs de 64 Kio occupes sur {len(region) // 0x10000}")
    try:
        check_supported(slots)
        print("Version prise en charge.")
    except FlashError as err:
        print(f"NON PRIS EN CHARGE : {err}")


def cmd_verify(dfu: Dfu, backup_path: pathlib.Path) -> None:
    backup, _ = load_backup(backup_path)
    verify_protected(dfu, backup)
    print("Zones protegees identiques a la sauvegarde.")


def cmd_restore_apps(dfu: Dfu, backup_path: pathlib.Path, write: bool, read_line: Callable[[str], str], interactive: bool) -> None:
    backup, _ = load_backup(backup_path)
    dfu.detect()
    for low, high in APPS_REGIONS:
        print(f"Restauration prevue : {low:#x}-{high:#x} ({(high - low) // 1024} Kio)")
    if not write:
        print("Simulation : rien n'a ete ecrit. Ajoute --write pour ecrire.")
        return
    check_device_matches(dfu, backup)
    confirm_write("Le contenu actuel de la zone des apps sera remplace.", read_line, interactive)
    for low, high in APPS_REGIONS:
        payload = region_bytes(backup, low, high)
        dfu.download(low, payload)
        if sha256(dfu.upload(low, high - low)) != sha256(payload):
            raise FlashError(f"relecture differente apres ecriture de {low:#x}")
    verify_protected(dfu, backup)
    print("Zone des apps restauree et verifiee.")


def wait_for_device(dfu: Dfu, attempts: int, delay_s: float) -> bool:
    """nwlink leaves DFU after installing, so the calculator must come back first."""
    for attempt in range(attempts):
        try:
            dfu.detect()
            return True
        except FlashError:
            if attempt + 1 < attempts:
                time.sleep(delay_s)
    return False


def cmd_install_nwa(
    dfu: Dfu,
    nwa: pathlib.Path,
    backup_path: pathlib.Path,
    write: bool,
    nwlink: Sequence[str],
    read_line: Callable[[str], str],
    interactive: bool,
    poll_attempts: int = 45,
    poll_delay_s: float = 2.0,
) -> None:
    if not nwa.is_file():
        raise FlashError(f"app introuvable : {nwa}")
    backup, _ = load_backup(backup_path)
    dfu.detect()
    print(f"Installation prevue : {nwa} ({nwa.stat().st_size} octets) avec {' '.join(nwlink)}")
    if not write:
        print("Simulation : rien n'a ete ecrit. Ajoute --write pour ecrire.")
        return
    check_device_matches(dfu, backup)
    confirm_write(
        "nwlink va ecrire l'app au debut de la zone des apps : KhiCAS et tes autres apps seront ecrasees.\n"
        "Elles restent recuperables avec restore-apps et cette sauvegarde.",
        read_line,
        interactive,
    )
    command = [shutil.which(nwlink[0]) or nwlink[0], *nwlink[1:], "install-nwa", str(nwa)]
    try:
        done = subprocess.run(command, capture_output=True, text=True)
    except OSError as err:
        raise FlashError(f"impossible de lancer nwlink : {err}") from err
    if done.returncode != 0:
        raise FlashError(f"nwlink a echoue : {done.stderr.strip() or done.stdout.strip()}")
    print("App ecrite. Lance-la pour le test, puis rebranche l'ecran USB pour la verification automatique.")
    if not wait_for_device(dfu, poll_attempts, poll_delay_s):
        print("Verification non faite : calculatrice absente. Remets-la sur l'ecran USB et lance la commande verify.")
        return
    verify_protected(dfu, backup)
    print("App installee. Kernel, userland et signatures sont intacts.")


def default_dfu_util() -> str:
    env = os.environ.get("AETHRA_DFU_UTIL")
    if env:
        return env
    found = shutil.which("dfu-util")
    if found:
        return found
    repo_guess = pathlib.Path(__file__).resolve().parents[3].parent / "dfu-util-0.11-binaries" / "win64" / "dfu-util.exe"
    return str(repo_guess)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Flasheur AETHRA pour NumWorks N0120")
    parser.add_argument("--dfu-util", default=None, help="commande dfu-util")
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("detect", help="detecte la calculatrice (lecture seule)")
    p = sub.add_parser("backup", help="sauvegarde les 8 Mio de flash (lecture seule)")
    p.add_argument("--out", type=pathlib.Path, required=True)
    p = sub.add_parser("inspect", help="analyse une sauvegarde")
    p.add_argument("file", type=pathlib.Path)
    p = sub.add_parser("verify", help="compare les zones protegees a une sauvegarde")
    p.add_argument("--backup", type=pathlib.Path, required=True)
    p = sub.add_parser("restore-apps", help="restaure la zone des apps depuis une sauvegarde")
    p.add_argument("--backup", type=pathlib.Path, required=True)
    p.add_argument("--write", action="store_true")
    p = sub.add_parser("install-nwa", help="installe une app .nwa")
    p.add_argument("nwa", type=pathlib.Path)
    p.add_argument("--backup", type=pathlib.Path, required=True)
    p.add_argument("--write", action="store_true")
    p.add_argument("--nwlink", default=DEFAULT_NWLINK)
    return parser


def split_command(text: str) -> List[str]:
    return shlex.split(text, posix=os.name != "nt") if " " in text and not pathlib.Path(text).exists() else [text]


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_parser().parse_args(argv)
    interactive = sys.stdin.isatty()
    try:
        if args.command == "inspect":
            cmd_inspect(args.file)
            return 0
        dfu = Dfu(split_command(args.dfu_util or default_dfu_util()))
        if args.command == "detect":
            cmd_detect(dfu)
        elif args.command == "backup":
            cmd_backup(dfu, args.out)
        elif args.command == "verify":
            cmd_verify(dfu, args.backup)
        elif args.command == "restore-apps":
            cmd_restore_apps(dfu, args.backup, args.write, input, interactive)
        elif args.command == "install-nwa":
            cmd_install_nwa(dfu, args.nwa, args.backup, args.write, split_command(args.nwlink), input, interactive)
    except FlashError as err:
        print(f"ERREUR : {err}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
