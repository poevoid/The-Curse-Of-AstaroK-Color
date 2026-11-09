"""Validate independent color art, ALPHA, palette and immutable monochrome data."""
from pathlib import Path
import sys,hashlib,random,struct
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import color_art as a
mono=[a.ROOT/'art/MonochromeSprites.txt',*sorted((a.ROOT/'CurseOfAstarok/src/artdata').glob('*.h'))]
before={p:hashlib.sha256(p.read_bytes()).digest() for p in mono}
blocks=a.load_art();data,n=a.build_pack();assert data[:4]==b'ACA2'
assert struct.unpack_from('<I',data,8)[0]==len(data)
r=random.Random(221)
for w,h in [(1,1),(13,17),(128,64)]:
 for symbol in list(a.SYMBOLS)+['random']:
  rows=[''.join(r.choice(a.SYMBOLS) if symbol=='random' else symbol for _ in range(w)) for _ in range(h)]
  assert a.decode_frame(a.encode_frame(rows),w,h)==rows
assert a.SYMBOLS.index('.')==17 and a.SYMBOLS.index('0')==0
assert all(hashlib.sha256(p.read_bytes()).digest()==before[p] for p in mono)
print(f'PASS: {len(blocks)} color sheets, {n} edited sheets, {len(data)}-byte pack; all RGB slots, BASE and ALPHA round trips; monochrome untouched')
