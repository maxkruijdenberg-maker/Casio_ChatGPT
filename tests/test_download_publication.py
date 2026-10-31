"""Download publication guardrails; no network or firmware hardware required."""
import contextlib
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest import mock
import urllib.error
import zipfile

spec = importlib.util.spec_from_file_location('publish_downloads', Path(__file__).resolve().parents[1] / 'tools/publish_downloads.py')
pub = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pub)

def archive(files):
    out = io.BytesIO()
    with zipfile.ZipFile(out, 'w') as z:
        for name, data in files.items():
            z.writestr(name, data)
    return out.getvalue()

class PublicationTests(unittest.TestCase):
    entry = {'id': 'sample', 'title': 'Sample', 'tag': 'download-sample-r1'}

    def response(self, body):
        response = mock.MagicMock()
        response.__enter__.return_value = response
        response.status = 200
        response.url = 'https://codeload.github.com/owner/repo/zip/refs/tags/download-sample-r1'
        response.read.return_value = body
        return response

    def test_valid_zip_is_compared_file_by_file_without_auth_header(self):
        body = archive({'repo-tag/START.txt': b'correct'})
        with mock.patch.object(pub, 'git', return_value=b'1234\n'), mock.patch.object(pub.urllib.request, 'urlopen', return_value=self.response(body)) as get:
            result = pub.check_remote('owner/repo', self.entry, {'START.txt': b'correct'})
        self.assertEqual(result['http_status'], 200)
        self.assertEqual(result['payload_files']['START.txt'], hashlib.sha256(b'correct').hexdigest())
        self.assertFalse(get.call_args.args[0].has_header('Authorization'))

    def test_404_prevents_report_page_and_link_output(self):
        with tempfile.TemporaryDirectory() as td, mock.patch.object(pub, 'ROOT', Path(td)), mock.patch.object(pub, 'git', return_value=b'1234\n'), mock.patch.object(pub.time, 'sleep'), mock.patch.object(pub.urllib.request, 'urlopen', side_effect=urllib.error.HTTPError('https://example.invalid', 404, 'Not Found', {}, None)):
            output = io.StringIO()
            with contextlib.redirect_stdout(output), self.assertRaises(urllib.error.HTTPError):
                pub.verify({'repository': 'owner/repo', 'entries': [self.entry]}, [{'START.txt': b'correct'}])
            self.assertFalse((Path(td) / 'DOWNLOADS.md').exists())
            self.assertFalse((Path(td) / 'downloads/verified.json').exists())
            self.assertEqual(output.getvalue(), '')

    def test_200_with_wrong_file_bytes_is_rejected(self):
        with mock.patch.object(pub, 'git', return_value=b'1234\n'), mock.patch.object(pub.urllib.request, 'urlopen', return_value=self.response(archive({'root/START.txt': b'wrong'}))):
            with self.assertRaisesRegex(ValueError, 'changed='):
                pub.check_remote('owner/repo', self.entry, {'START.txt': b'correct'})

    def test_missing_file_is_rejected(self):
        with mock.patch.object(pub, 'git', return_value=b'1234\n'), mock.patch.object(pub.urllib.request, 'urlopen', return_value=self.response(archive({'root/OTHER.txt': b'correct'}))):
            with self.assertRaisesRegex(ValueError, 'missing='):
                pub.check_remote('owner/repo', self.entry, {'START.txt': b'correct'})

    def test_html_error_page_is_not_accepted_as_a_zip(self):
        with mock.patch.object(pub, 'git', return_value=b'1234\n'), mock.patch.object(pub.urllib.request, 'urlopen', return_value=self.response(b'<html>not a ZIP</html>')):
            with self.assertRaises(zipfile.BadZipFile):
                pub.check_remote('owner/repo', self.entry, {'START.txt': b'correct'})

    def test_corrupt_firmware_checksum_is_rejected(self):
        with self.assertRaisesRegex(ValueError, 'checksum'):
            pub.validate({'dist/sha256.json': json.dumps({'firmware.bin': '0' * 64}).encode(), 'dist/firmware.bin': b'wrong'})

    def test_unsafe_archive_path_is_rejected(self):
        with self.assertRaises(ValueError):
            pub.unpack(archive({'../outside.txt': b'bad'}))

    def test_bom_cpl_mismatch_is_rejected(self):
        with self.assertRaisesRegex(ValueError, 'BOM/CPL'):
            pub.validate({'BOM-JLCPCB.csv': b'Designator\nR1\n', 'CPL-JLCPCB.csv': b'Designator\nR2\n'})

    def test_private_repository_is_rejected_without_changing_visibility(self):
        with mock.patch.object(pub.urllib.request, 'urlopen', return_value=self.response(b'<meta name="octolytics-dimension-repository_public" content="false">')):
            with self.assertRaisesRegex(ValueError, 'not verified public'):
                pub.verify_public_repository('owner/repo')

    def test_public_repository_metadata_is_required(self):
        with mock.patch.object(pub.urllib.request, 'urlopen', return_value=self.response(b'<html>login page</html>')):
            with self.assertRaisesRegex(ValueError, 'not verified public'):
                pub.verify_public_repository('owner/repo')

    def test_fresh_clone_reuses_canonical_remote_download_tag(self):
        with tempfile.TemporaryDirectory() as td:
            source, clone = Path(td) / 'source', Path(td) / 'clone'
            source.mkdir()
            with mock.patch.object(pub, 'ROOT', source):
                pub.git('init', '-q')
                pub.git('config', 'user.email', 'download-test@example.invalid')
                pub.git('config', 'user.name', 'Download test')
                pub.git('commit', '--allow-empty', '-qm', 'Main')
                pub.create_tag(self.entry, {'START.txt': b'correct'})
                canonical = pub.git('rev-parse', self.entry['tag'])
            subprocess.check_call(['git', 'clone', '-q', '--no-tags', str(source), str(clone)])
            with mock.patch.object(pub, 'ROOT', clone):
                pub.sync_existing_tags([self.entry])
                pub.create_tag(self.entry, {'START.txt': b'correct'})
                self.assertEqual(pub.git('rev-parse', self.entry['tag']), canonical)

    def test_tag_is_immutable_and_checkout_and_index_stay_intact(self):
        with tempfile.TemporaryDirectory() as td, mock.patch.object(pub, 'ROOT', Path(td)):
            pub.git('init', '-q')
            pub.git('config', 'user.email', 'download-test@example.invalid')
            pub.git('config', 'user.name', 'Download test')
            (Path(td) / 'existing.txt').write_text('existing work')
            pub.git('add', 'existing.txt')
            pub.git('commit', '-qm', 'Existing work')
            (Path(td) / 'staged.txt').write_text('user staging')
            pub.git('add', 'staged.txt')
            head, index = pub.git('rev-parse', 'HEAD'), pub.git('write-tree')
            pub.create_tag(self.entry, {'START.txt': b'correct'})
            tag = pub.git('rev-parse', self.entry['tag'])
            pub.create_tag(self.entry, {'START.txt': b'correct'})
            self.assertEqual(pub.git('rev-parse', self.entry['tag']), tag)
            with self.assertRaisesRegex(ValueError, 'NEW revision'):
                pub.create_tag(self.entry, {'START.txt': b'different'})
            self.assertEqual(pub.git('rev-parse', 'HEAD'), head)
            self.assertEqual(pub.git('write-tree'), index)
            self.assertEqual((Path(td) / 'existing.txt').read_text(), 'existing work')

if __name__ == '__main__':
    unittest.main()
