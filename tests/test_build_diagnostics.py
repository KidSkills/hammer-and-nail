import importlib.util
import unittest
from pathlib import Path


def load_build_module():
    build_path = Path(__file__).resolve().parents[1] / "build.py"
    spec = importlib.util.spec_from_file_location("repo_build", build_path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


class BuildDiagnosticReportTest(unittest.TestCase):
    def test_failed_module_is_explicit_in_diagnostic_metadata(self):
        build = load_build_module()

        report = build.build_diagnostic_report(
            [
                build.ModuleResult("backend", True, 0.42, "ok", "target/debug/backend"),
                build.ModuleResult("market", False, 1.25, "compile error"),
            ],
            "deadbeef",
            logd_relpaths=["diagnostic/build-deadbeef.logd"],
            password="pw",
        )

        self.assertEqual(report["overall_status"], "failed")
        self.assertEqual(report["exit_code"], 1)
        self.assertEqual(report["passed"], 1)
        self.assertEqual(report["failed"], 1)
        self.assertEqual(report["failed_modules"], ["market"])
        self.assertEqual(report["modules"][0]["status"], "passed")
        self.assertIs(report["modules"][0]["success"], True)
        self.assertEqual(report["modules"][1]["status"], "failed")
        self.assertIs(report["modules"][1]["success"], False)

    def test_logd_error_marks_report_failed_even_when_modules_pass(self):
        build = load_build_module()

        report = build.build_diagnostic_report(
            [build.ModuleResult("backend", True, 0.42, "ok")],
            "deadbeef",
            logd_error="encryptly pack failed",
        )

        self.assertEqual(report["overall_status"], "failed")
        self.assertEqual(report["exit_code"], 1)
        self.assertEqual(report["failed_modules"], [])
        self.assertEqual(report["diagnostic_logd_error"], "encryptly pack failed")


if __name__ == "__main__":
    unittest.main()
