"""Check V22 aliases against a downloaded, pinned official ChatModel source.
Usage: python3 tools/check_model_source.py /path/to/chat_model.py
This checks SDK registration/deprecations, not live account access or prices.
"""
from pathlib import Path
import hashlib,re,sys
root=Path(__file__).resolve().parents[1]
source=Path(sys.argv[1]).read_bytes()
expected='f552fcffcf774520e946121704b5fbbf7ba986e8'
# Pin by content as well as URL: do not silently validate a different SDK revision.
assert hashlib.sha256(source).hexdigest()=='d2a74c51d4ee5cf1f782fc753c3db15d49cb19f8c3d04cadc4b76775b6ba4167', 'Official source hash differs'
text=source.decode()
registered,deprecated=text.split('Deprecated values:',1)
ids=re.findall(r'\{"[^"]+","([^"]+)"',(root/'casio/models.h').read_text())
assert len(ids)==11 and len(set(ids))==len(ids)
for id in ids:
 assert '"'+id+'"' in registered, ('Not registered',id)
 assert '"'+id+'"' not in deprecated, ('Deprecated',id)
 assert not re.search(r'\d{4}-\d{2}-\d{2}',id)
 print('PASS current non-deprecated ChatModel alias:',id)
print('Official openai-python revision:',expected)
print('SHA-256:',hashlib.sha256(source).hexdigest())
print('No live completion, performance or price verification is implied.')
