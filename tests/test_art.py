"""Round-trip every editable frame and validate dimensions and source encoding."""
import sys,json,random
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import art_tools as a
m=json.loads((a.ROOT/'art/manifest.json').read_text());blocks=a.load_art(a.ROOT/'art/MonochromeSprites.txt')
for name,info in m.items():
 f=blocks[name];encoded=a.encode(f,info['kind']);assert a.decode(encoded,info['kind'],info['width'],info['height'])==f,name
# Non-byte-aligned dimensions and long runs exercise compressor bit lengths.
r=random.Random(42)
for w,h in [(1,1),(5,7),(13,13),(128,64),(19,9)]:
 for mode in ['empty','full','noise']:
  f=[[''.join('#' if (mode=='full' or (mode=='noise' and r.randrange(2))) else '.' for x in range(w)) for y in range(h)]]
  b=a.encode(f,'compressed');assert a.decode(b,'compressed')==f,(w,h,mode)
print(f'PASS: {len(m)} sheets, {sum(len(f) for f in blocks.values())} frames; edge-case codec round trips')
