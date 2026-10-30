import contextlib
import hashlib
import io
import os
from pathlib import Path
import runpy
import subprocess
import sys
import tempfile
import unittest
from unittest import mock


class MakeDefTests(unittest.TestCase):
    GENERATORS = (
        ("curl/makeCurlDef.py", "CURL_EXTERN int curl_review_probe(void);\n"),
        ("freetype2/makeFreetypeDef.py", "FT_EXPORT( int ) FT_Review_Probe(void);\n"),
    )

    def test_archive_and_export_invalidation(self):
        for script, declaration in self.GENERATORS:
            with self.subTest(generator=script), tempfile.TemporaryDirectory() as temporary:
                with mock.patch.object(sys, "path", sys.path[:]):
                    main = runpy.run_path(str(Path(__file__).parent / script))["main"]
                root = Path(temporary)
                headers = root / "headers"
                headers.mkdir()
                header = headers / "api.h"
                header.write_text(declaration, encoding="utf-8")
                archive = root / "input.lib"
                archive_prefix = b"x" * (1024 * 1024)
                archive.write_bytes(archive_prefix + b"first archive")
                output = root / "output.def"
                output.write_bytes(b"EXPORTS\r\n    existing_export\r\n")
                dll = root / "output.dll"
                dll.write_bytes(b"DLL built before archive fingerprinting")
                arguments = [script, str(headers), str(output), str(archive), str(dll)]

                def generate():
                    with mock.patch.object(sys, "argv", arguments), contextlib.redirect_stderr(io.StringIO()):
                        return main()

                self.assertEqual(0, generate())
                self.assertFalse(dll.exists())
                self.assertIn(hashlib.sha256(archive.read_bytes()).hexdigest().encode(), output.read_bytes())
                self.assertIn(b"\r\nEXPORTS\r\n", output.read_bytes())
                dll.write_bytes(b"existing DLL")
                original_def = output.read_bytes()
                original_time = output.stat().st_mtime_ns
                os.utime(archive, (1000000300, 1000000300))
                self.assertEqual(0, generate())
                self.assertTrue(dll.exists())
                self.assertEqual(original_time, output.stat().st_mtime_ns)

                os.utime(dll, (1000000200, 1000000200))
                archive.write_bytes(archive_prefix + b"other archive")
                os.utime(archive, (1000000100, 1000000100))
                self.assertLess(archive.stat().st_mtime_ns, dll.stat().st_mtime_ns)
                with mock.patch("os.remove", side_effect=PermissionError("DLL is loaded")):
                    self.assertEqual(1, generate())
                self.assertEqual(original_def, output.read_bytes())
                self.assertTrue(dll.exists())
                self.assertEqual(0, generate())
                self.assertFalse(dll.exists())
                self.assertNotEqual(original_def, output.read_bytes())
                self.assertIn(hashlib.sha256(archive.read_bytes()).hexdigest().encode(), output.read_bytes())

                dll.write_bytes(b"relinked DLL")
                original_time = output.stat().st_mtime_ns
                self.assertEqual(0, generate())
                self.assertTrue(dll.exists())
                self.assertEqual(original_time, output.stat().st_mtime_ns)

                another_declaration = declaration.replace("Probe", "AnotherProbe").replace("probe", "another_probe")
                header.write_text(declaration + another_declaration, encoding="utf-8")
                self.assertEqual(0, generate())
                self.assertFalse(dll.exists())
                dll.write_bytes(b"relinked DLL")
                original_def = output.read_bytes()
                archive.unlink()
                self.assertEqual(1, generate())
                self.assertEqual(original_def, output.read_bytes())
                self.assertTrue(dll.exists())

    def test_command_line_from_other_directory(self):
        for script, declaration in self.GENERATORS:
            with self.subTest(generator=script), tempfile.TemporaryDirectory(prefix="make def ") as temporary:
                root = Path(temporary)
                headers = root / "headers"
                headers.mkdir()
                (headers / "api.h").write_text(declaration, encoding="utf-8")
                archive = root / "input.lib"
                archive.write_bytes(b"archive")
                output = root / "output.def"
                result = subprocess.run(
                    [sys.executable, "-B", str(Path(__file__).resolve().parent / script),
                     str(headers), str(output), str(archive)],
                    cwd=temporary, capture_output=True, text=True,
                )
                self.assertEqual(0, result.returncode, result.stderr)
                self.assertIn(hashlib.sha256(archive.read_bytes()).hexdigest().encode(), output.read_bytes())


if __name__ == "__main__":
    unittest.main()
