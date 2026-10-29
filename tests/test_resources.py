import importlib.util
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]

class NativeResourcesTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        spec = importlib.util.spec_from_file_location('resources', ROOT / 'tools/generate_resources.py')
        cls.module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(cls.module)
        cls.output = cls.module.generate(ROOT)

    def test_every_dialog_is_preserved(self):
        original = (ROOT / 'resources/dialogs/dialogs.rc').read_text()
        ids = lambda text: set(re.findall(r'^(\d+) DIALOG(?:EX)?\b', text, re.M))
        self.assertEqual(ids(original), ids(self.output))
        self.assertEqual(81, len(ids(self.output)))

    def test_no_borland_controls_and_readable_font(self):
        self.assertNotRegex(self.output, r'(?i)"bor(?:btn|shade|radio|check|static|dlg)[^"]*"')
        self.assertIn('"Segoe UI"', self.output)
        self.assertNotIn('100000 lire', self.output)
        self.assertIn('50,00 EUR', self.output)

    def test_radio_and_checkbox_semantics(self):
        self.assertIn('BS_AUTORADIOBUTTON', self.output)
        self.assertIn('BS_AUTOCHECKBOX', self.output)
        # Handlers toggle model state; standard controls toggle their own view.

    def test_assets_present(self):
        self.assertIn('1201 BITMAP', self.output)
        self.assertIn('ZARROSIM ICON', self.output)
        self.assertIn('1 ICON', self.output)
        self.assertIn('STRINGTABLE', self.output)

    def dialog_controls(self, number):
        block = re.search(rf'^{number} DIALOGEX\b.*?(?=^\d+ DIALOGEX\b|^1 VERSIONINFO|\Z)', self.output, re.M | re.S)[0]
        return {int(m[2]): m for line in block.splitlines() if (m := self.module.CONTROL.match(line)) and int(m[2]) >= 0}

    def test_settings_have_room_for_labels_and_actions(self):
        controls = self.dialog_controls(14)
        self.assertGreaterEqual(int(controls[108][7]), 250)
        self.assertGreaterEqual(int(controls[1][7]), 60)
        self.assertGreaterEqual(int(controls[1][8]), 20)
        self.assert_buttons_do_not_overlap(controls)

    def assert_buttons_do_not_overlap(self, controls):
        buttons = [(key, tuple(map(int, m.groups()[-4:]))) for key, m in controls.items() if m[3].strip('"') == 'BUTTON']
        for i, (a, (ax, ay, aw, ah)) in enumerate(buttons):
            for b, (bx, by, bw, bh) in buttons[i+1:]:
                self.assertFalse(ax < bx+bw and bx < ax+aw and ay < by+bh and by < ay+ah, f'Buttons {a} and {b} overlap')

    def test_school_actions_have_separate_click_targets(self):
        self.assert_buttons_do_not_overlap(self.dialog_controls(10))

if __name__ == '__main__':
    unittest.main()
