"""Tests the flasher against a fake dfu-util. No test ever talks to real hardware."""
import ast
import hashlib
import os
import pathlib
import struct
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "flash"))
import aethra_flash as af  # noqa: E402

HERE = pathlib.Path(__file__).resolve().parent
DUMP = ROOT.parents[1] / "dfu-util-0.11-binaries" / "win64" / "epsilon_flash_8mb.bin"


def make_image(version=b"26.4.0"):
    image = bytearray(b"\xff" * af.FLASH_SIZE)
    for index, (start, end) in enumerate(af.APPS_REGIONS):
        off = index * af.SLOT_SIZE
        image[off:off + 32] = (
            struct.pack("<II", 2, 0x17BFD8) + af.SLOT_MAGIC + version.ljust(8, b"\0") + b"cc9262e\0" + af.SLOT_MAGIC
        )
        uh = off + af.USERLAND_OFFSET
        image[uh:uh + 4] = af.USERLAND_MAGIC
        image[uh + 20:uh + 28] = struct.pack("<II", start, end)
        image[start - af.FLASH_BASE:start - af.FLASH_BASE + 4] = b"APPS"
    return bytes(image)


class Base(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.dir = pathlib.Path(self.tmp.name)
        self.flash = self.dir / "flash.bin"
        self.flash.write_bytes(make_image())
        self.log = self.dir / "log.txt"
        self.env = {"FAKE_FLASH": str(self.flash), "FAKE_LOG": str(self.log)}
        os.environ.update(self.env)
        os.environ["FAKE_LIST"] = "one"
        os.environ["FAKE_CORRUPT_WRITE"] = "0"
        self.dfu = af.Dfu([sys.executable, "-I", str(HERE / "fake_dfu.py")])
        self.backup = self.dir / "backup.bin"

    def tearDown(self):
        self.tmp.cleanup()

    def make_backup(self):
        af.cmd_backup(self.dfu, self.backup)

    def writes(self):
        return [line for line in self.log.read_text().splitlines() if " -D " in line]

    def nwlink(self, offset):
        os.environ["FAKE_NWLINK_OFFSET"] = hex(offset)
        return [sys.executable, "-I", str(HERE / "fake_nwlink.py")]


class Allowlist(unittest.TestCase):
    def test_inside_regions(self):
        af.assert_write_allowed(0x90180000, 0x903F0000)
        af.assert_write_allowed(0x90580000, 0x90590000)

    def test_refused(self):
        for start, end in [
            (0x08000000, 0x08010000),
            (0x90000000, 0x90010000),
            (0x9017FFFF, 0x90180001),
            (0x903EFFFF, 0x903F0001),
            (0x90400000, 0x90410000),
            (0x90180000, 0x90580000),
            (0x907F0000, 0x90800000),
            (0x90190000, 0x90190000),
        ]:
            with self.assertRaises(af.FlashError, msg=hex(start)):
                af.assert_write_allowed(start, end)

    def test_regions_never_touch_protected_ranges(self):
        for low, high in af.APPS_REGIONS:
            for plow, phigh in af.PROTECTED_RANGES:
                self.assertTrue(high <= plow or phigh <= low)


class WritePathAudit(unittest.TestCase):
    def test_only_download_passes_the_write_flag(self):
        source = (ROOT / "tools" / "flash" / "aethra_flash.py").read_text(encoding="utf-8")
        tree = ast.parse(source)
        users = set()
        for func in ast.walk(tree):
            if isinstance(func, ast.FunctionDef):
                for node in ast.walk(func):
                    if isinstance(node, ast.Constant) and node.value == "-D":
                        users.add(func.name)
        self.assertEqual(users, {"download"})


class Detect(Base):
    def test_one(self):
        self.assertEqual(self.dfu.detect(), 0)

    def test_none(self):
        os.environ["FAKE_LIST"] = "none"
        with self.assertRaises(af.FlashError):
            self.dfu.detect()

    def test_two(self):
        os.environ["FAKE_LIST"] = "two"
        with self.assertRaises(af.FlashError):
            self.dfu.detect()

    def test_wrong_layout(self):
        os.environ["FAKE_LIST"] = "wrong"
        with self.assertRaises(af.FlashError):
            self.dfu.detect()


class Backup(Base):
    def test_writes_image_and_hash(self):
        self.make_backup()
        data, slots = af.load_backup(self.backup)
        self.assertEqual(hashlib.sha256(data).hexdigest(), hashlib.sha256(self.flash.read_bytes()).hexdigest())
        self.assertEqual(len(slots), 2)
        self.assertEqual(self.writes(), [])

    def test_refuses_overwrite(self):
        self.make_backup()
        with self.assertRaises(af.FlashError):
            self.make_backup()

    def test_detects_tampering(self):
        self.make_backup()
        data = bytearray(self.backup.read_bytes())
        data[100] ^= 1
        self.backup.write_bytes(bytes(data))
        with self.assertRaises(af.FlashError):
            af.load_backup(self.backup)

    def test_rejects_other_version(self):
        self.flash.write_bytes(make_image(b"27.0.0"))
        self.make_backup()
        with self.assertRaises(af.FlashError):
            af.load_backup(self.backup)

    @unittest.skipUnless(DUMP.is_file(), "real dump not available")
    def test_real_dump_parses(self):
        slots = af.parse_image(DUMP.read_bytes())
        af.check_supported(slots)
        self.assertEqual([(s.apps_start, s.apps_end) for s in slots], list(af.APPS_REGIONS))


class Restore(Base):
    def run_restore(self, write=True, answer="ECRIRE", interactive=True):
        af.cmd_restore_apps(self.dfu, self.backup, write, lambda _: answer, interactive)

    def damage_apps(self):
        image = bytearray(self.flash.read_bytes())
        image[0x190000] = 0x12
        self.flash.write_bytes(bytes(image))

    def test_dry_run_writes_nothing(self):
        self.make_backup()
        self.run_restore(write=False)
        self.assertEqual(self.writes(), [])

    def test_no_terminal_refused(self):
        self.make_backup()
        with self.assertRaises(af.FlashError):
            self.run_restore(interactive=False)
        self.assertEqual(self.writes(), [])

    def test_wrong_answer_refused(self):
        self.make_backup()
        with self.assertRaises(af.FlashError):
            self.run_restore(answer="oui")
        self.assertEqual(self.writes(), [])

    def test_restores_only_apps(self):
        self.make_backup()
        self.damage_apps()
        self.run_restore()
        self.assertEqual(self.flash.read_bytes(), self.backup.read_bytes())
        self.assertEqual(len(self.writes()), 2)

    def test_readback_mismatch_detected(self):
        self.make_backup()
        os.environ["FAKE_CORRUPT_WRITE"] = "1"
        with self.assertRaises(af.FlashError):
            self.run_restore()

    def test_other_calculator_refused(self):
        self.make_backup()
        image = bytearray(self.flash.read_bytes())
        image[0x10] ^= 1
        self.flash.write_bytes(bytes(image))
        with self.assertRaises(af.FlashError):
            self.run_restore()
        self.assertEqual(self.writes(), [])


class Install(Base):
    def setUp(self):
        super().setUp()
        os.environ.pop("FAKE_NWLINK_OFFLINE_POLLS", None)

    def install(self, nwlink, write=True, answer="ECRIRE", interactive=True, attempts=5):
        app = self.dir / "app.nwa"
        app.write_bytes(b"NWA")
        af.cmd_install_nwa(self.dfu, app, self.backup, write, nwlink, lambda _: answer, interactive, attempts, 0)

    def test_waits_for_device_after_leaving_dfu(self):
        self.make_backup()
        os.environ["FAKE_NWLINK_OFFLINE_POLLS"] = "2"
        self.install(self.nwlink(0x190000))
        self.assertEqual(self.flash.with_suffix(".offline").read_text(), "0")

    def test_damage_detected_after_device_returns(self):
        self.make_backup()
        os.environ["FAKE_NWLINK_OFFLINE_POLLS"] = "2"
        with self.assertRaises(af.FlashError) as ctx:
            self.install(self.nwlink(0x30000))
        self.assertIn("ALERTE", str(ctx.exception))

    def test_device_never_back_is_not_an_alert(self):
        self.make_backup()
        os.environ["FAKE_NWLINK_OFFLINE_POLLS"] = "99"
        self.install(self.nwlink(0x190000), attempts=3)

    def test_nwlink_launcher_is_resolved(self):
        self.make_backup()
        self.install([pathlib.Path(sys.executable).name, "-I", str(HERE / "fake_nwlink.py")])
        os.environ.pop("FAKE_NWLINK_OFFSET", None)

    def test_dry_run_does_not_call_nwlink(self):
        self.make_backup()
        before = self.flash.read_bytes()
        self.install(self.nwlink(0x190000), write=False)
        self.assertEqual(self.flash.read_bytes(), before)

    def test_install_in_apps_region_passes(self):
        self.make_backup()
        self.install(self.nwlink(0x190000))
        self.assertNotEqual(self.flash.read_bytes(), self.backup.read_bytes())

    def test_damage_to_protected_area_is_reported(self):
        self.make_backup()
        with self.assertRaises(af.FlashError) as ctx:
            self.install(self.nwlink(0x30000))
        self.assertIn("ALERTE", str(ctx.exception))

    def test_no_terminal_refused(self):
        self.make_backup()
        before = self.flash.read_bytes()
        with self.assertRaises(af.FlashError):
            self.install(self.nwlink(0x190000), interactive=False)
        self.assertEqual(self.flash.read_bytes(), before)


if __name__ == "__main__":
    unittest.main(verbosity=1)
