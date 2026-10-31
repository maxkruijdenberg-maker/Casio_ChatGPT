"""Publish payload-only immutable Git tags and verify actual GitHub ZIPs.

No GitHub API or working-tree checkout needed. Refuse overwriting a tag.
prepare: create local download tags. publish: prepare, push, verify.
verify: download, compare every byte, then write reports and download page.
"""
from pathlib import Path
import argparse, concurrent.futures, csv, datetime, hashlib, io, json, os
import re, subprocess, tempfile, time, urllib.error, urllib.request, zipfile
ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / 'downloads/manifest.json'

def sha(data):
    return hashlib.sha256(data).hexdigest()

def git(*args, data=None, env=None):
    return subprocess.check_output(['git', '-C', str(ROOT), *args], input=data, env=env)

def safe_name(name):
    if not name or '\\' in name or '\x00' in name or name.startswith('/'):
        raise ValueError(f'Unsafe archive member: {name!r}')
    if any(part in ('', '.', '..', '.git') for part in name.split('/')):
        raise ValueError(f'Unsafe archive member: {name!r}')
    return name

def unpack(data, prefix=''):
    result = {}
    with zipfile.ZipFile(io.BytesIO(data)) as z:
        bad = z.testzip()
        if bad:
            raise ValueError(f'Broken ZIP member: {bad}')
        for info in z.infolist():
            if info.is_dir():
                continue
            if not info.filename.startswith(prefix):
                raise ValueError(f'Member outside package prefix: {info.filename}')
            name = safe_name(info.filename[len(prefix):])
            if name in result or (info.external_attr >> 16) & 0o170000 == 0o120000:
                raise ValueError(f'Duplicate or symlink ZIP member: {name}')
            result[name] = z.read(info)
    if not result:
        raise ValueError('Empty download')
    return result

def validate(payload):
    # Detect damaged nested Gerber ZIPs before publishing them.
    for name, data in payload.items():
        if name.lower().endswith('.zip'):
            unpack(data)
    if 'dist/sha256.json' in payload:
        for name, expected in json.loads(payload['dist/sha256.json']).items():
            if sha(payload['dist/' + safe_name(name)]) != expected:
                raise ValueError(f'Firmware checksum mismatch: {name}')
    if 'CHECKSUMS.sha256' in payload:
        for line in payload['CHECKSUMS.sha256'].decode().splitlines():
            expected, name = line.split('  ', 1)
            if not re.fullmatch(r'[0-9a-f]{64}', expected) or sha(payload[safe_name(name)]) != expected:
                raise ValueError(f'Package checksum mismatch: {name}')
    if 'BOM-JLCPCB.csv' in payload and 'CPL-JLCPCB.csv' in payload:
        bom = csv.DictReader(io.StringIO(payload['BOM-JLCPCB.csv'].decode('utf-8-sig')))
        refs = [ref for row in bom for ref in row['Designator'].split(',')]
        cpl = csv.DictReader(io.StringIO(payload['CPL-JLCPCB.csv'].decode('utf-8-sig')))
        placed = [row['Designator'] for row in cpl]
        if len(refs) != len(set(refs)) or len(placed) != len(set(placed)) or set(refs) != set(placed):
            raise ValueError('BOM/CPL designators do not match')

def payload_for(entry):
    if 'archive' in entry:
        result = unpack((ROOT / entry['archive']).read_bytes(), entry.get('strip_prefix', ''))
    else:
        result = {safe_name(Path(name).name): (ROOT / name).read_bytes() for name in entry['files']}
        if len(result) != len(entry['files']):
            raise ValueError('Duplicate destination filenames')
    validate(result)
    return result

def create_tag(entry, payload):
    # A temporary Git index avoids touching the user's staging area or checkout.
    with tempfile.TemporaryDirectory(prefix='casio-download-index-') as td:
        env = dict(os.environ, GIT_INDEX_FILE=str(Path(td) / 'index'))
        git('read-tree', '--empty', env=env)
        records = []
        for name, data in sorted(payload.items()):
            blob = git('hash-object', '-w', '--stdin', data=data).decode().strip()
            records.append(f'100644 {blob}\t{name}\0'.encode())
        git('update-index', '-z', '--index-info', data=b''.join(records), env=env)
        tree = git('write-tree', env=env).decode().strip()
    tag = entry['tag']
    existing = subprocess.run(['git', '-C', str(ROOT), 'rev-parse', '--verify', f'refs/tags/{tag}^{{tree}}'],
                              stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    if existing.returncode == 0:
        if existing.stdout.decode().strip() != tree:
            raise ValueError(f'{tag} already has different content. Choose a NEW revision; never overwrite a download tag.')
        return
    commit = git('commit-tree', tree, data=f"Download snapshot: {entry['title']}\n".encode()).decode().strip()
    git('tag', '-a', tag, commit, '-m', f"Immutable payload-only download: {entry['title']}")

def sync_existing_tags(entries):
    refs = [f"refs/tags/{entry['tag']}" for entry in entries]
    advertised = git('ls-remote', '--tags', 'origin', *refs).decode().splitlines()
    present = {line.split()[1] for line in advertised if len(line.split()) == 2}
    wanted = [ref for ref in refs if ref in present]
    if wanted:
        # Fetch only these package tags; never force-replace a conflicting local tag.
        git('fetch', '--no-tags', 'origin', *(f'{ref}:{ref}' for ref in wanted))


def download_url(repository, entry):
    return f"https://github.com/{repository}/archive/refs/tags/{entry['tag']}.zip"

def check_remote(repository, entry, payload):
    tag = entry['tag']
    url = download_url(repository, entry)
    for attempt in range(3):
        try:
            request = urllib.request.Request(url, headers={'User-Agent': 'Casio-download-verifier/1.0'})
            # No Authorization header: these are public download links.
            with urllib.request.urlopen(request, timeout=25) as response:
                status = response.status
                body = response.read(32 * 1024 * 1024 + 1)
                final_url = response.url
            if status != 200 or len(body) > 32 * 1024 * 1024:
                raise ValueError('Download status or size invalid')
            # GitHub wraps tag archives in one repository/tag root directory.
            with zipfile.ZipFile(io.BytesIO(body)) as z:
                roots = {info.filename.split('/')[0] for info in z.infolist()}
            if len(roots) != 1:
                raise ValueError('Unexpected GitHub archive layout')
            actual = unpack(body, next(iter(roots)) + '/')
            if actual != payload:
                missing, extra = sorted(payload.keys() - actual.keys()), sorted(actual.keys() - payload.keys())
                changed = sorted(name for name in payload.keys() & actual.keys() if payload[name] != actual[name])
                raise ValueError(f'GitHub payload differs: missing={missing}, extra={extra}, changed={changed}')
            validate(actual)
            return {'id': entry['id'], 'title': entry['title'], 'tag': tag,
                    'url': url, 'resolved_url': final_url,
                    'http_status': status, 'bytes': len(body), 'zip_sha256': sha(body),
                    'payload_files': {name: sha(data) for name, data in sorted(actual.items())}}
        except (urllib.error.HTTPError, urllib.error.URLError, TimeoutError, OSError):
            if attempt == 2:
                raise
            time.sleep((2, 5)[attempt])

def verify_public_repository(repository):
    request = urllib.request.Request(f'https://github.com/{repository}', headers={'User-Agent': 'Casio-download-verifier/1.0'})
    with urllib.request.urlopen(request, timeout=25) as response:
        page = response.read(2 * 1024 * 1024).decode('utf-8', errors='replace')
    if not re.search(r'<meta\s+name="octolytics-dimension-repository_public"\s+content="true"', page):
        raise ValueError('Repository is not verified public. Do not claim login-free downloads or change visibility automatically.')


def verify(config, payloads):
    verify_public_repository(config['repository'])
    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
        jobs = [pool.submit(check_remote, config['repository'], entry, payload)
                for entry, payload in zip(config['entries'], payloads)]
        results = [job.result() for job in jobs]  # Fail before emitting any links/report if any download fails.
    report = {'verified_utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
              'manifest_sha256': sha(MANIFEST.read_bytes()), 'repository_public': True, 'downloads': results}
    (ROOT / 'downloads/verified.json').write_text(json.dumps(report, indent=2) + '\n')
    rows = ['# Downloads', '', 'Deze bestanden zijn na publicatie vanaf GitHub teruggelezen. Alle bestanden in de ZIP zijn op inhoud gecontroleerd.', '',
            '**V40.1:** pak volledig uit en open `START-HIER.html`. Dit pakket is voor de ATOM-testbench en de oudere Rev E-module.', '',
            '**Nieuwe XIAO-print met drie draden:** gebruik de Rev G-PCB en de aparte Rev G XIAO-firmware. De Casio-app blijft V40.1.', '',
            '| Pakket | Download |', '|---|---|']
    for result in results[:3]:
        rows.append(f"| {result['title']} | [ZIP downloaden]({result['url']}) |")
    rows += ['', '## Archief', '', '**Deze ontwerpen zijn niet geschikt voor de huidige drie-draads Rev G-opstelling.**', '']
    for result in results[3:]:
        rows.append(f"- [{result['title']}]({result['url']})")
    rows += ['', 'De PCB-downloads bevatten rechtstreeks Gerber-ZIP, BOM CSV en CPL CSV.', '',
             f"Controle uitgevoerd: {report['verified_utc']}. Controleverslag: [verified.json](downloads/verified.json).", '',
             'De oorspronkelijke raw-links blijven als oude verwijzingen bestaan; gebruik de ZIP-links hierboven.', '']
    (ROOT / 'DOWNLOADS.md').write_text('\n'.join(rows))
    for result in results:
        print(f"VERIFIED {result['id']}: {result['url']}")

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=['prepare', 'verify', 'publish'])
    args = parser.parse_args()
    config = json.loads(MANIFEST.read_text())
    payloads = [payload_for(entry) for entry in config['entries']]
    if args.action == 'publish':
        sync_existing_tags(config['entries'])
    if args.action in ('prepare', 'publish'):
        for entry, payload in zip(config['entries'], payloads):
            create_tag(entry, payload)
        print(f"Prepared {len(payloads)} immutable download snapshots; checkout/index unchanged.")
    if args.action == 'publish':
        git('push', 'origin', *(f"refs/tags/{entry['tag']}" for entry in config['entries']))
    if args.action in ('verify', 'publish'):
        verify(config, payloads)

if __name__ == '__main__':
    main()
