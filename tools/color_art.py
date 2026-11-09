"""Editable color-only artwork and the SD art pack. Standard Python only.
Monochrome artwork and firmware source are never written by this module.
"""
from pathlib import Path
import json,struct,re
ROOT=Path(__file__).resolve().parents[1]
SYMBOLS='0123456789ABCDEFG.'
MAGIC=b'ACA2'
def load_art(path=None):
 blocks={};name=None;frames=[];rows=[]
 for line in Path(path or ROOT/'art/ColorSprites.txt').read_text().splitlines():
  s=line.strip()
  if not s or s.startswith(';'):continue
  if s.startswith('@'):
   if name:
    if rows:frames.append(rows)
    if name in blocks:raise ValueError('Duplicate sprite '+name)
    blocks[name]=frames
   name=s[1:];frames=[];rows=[]
  elif s=='---':
   if rows:frames.append(rows);rows=[]
  else:
   if not name or set(s)-set(SYMBOLS):raise ValueError('Use . for transparency, 0..F for palette colors, G for BASE')
   rows.append(s)
 if name:
  if rows:frames.append(rows)
  blocks[name]=frames
 return blocks
def write_art(blocks,path=None):
 text='; COLOR MODE ONLY. . = transparent, 0..F = palette, G = inherited BASE color.\n; @Name starts a sprite sheet; --- separates animation frames.\n\n'
 for n,frames in blocks.items():text+='@'+n+'\n'+'\n---\n'.join('\n'.join(f) for f in frames)+'\n\n'
 Path(path or ROOT/'art/ColorSprites.txt').write_text(text)
# Migration reference only: the palette shipped before header RGB syncing.
# Actual displayed colors are read from the user's header, never from this list.
LEGACY_HEADER_PALETTE=[
 [0,0,0],[206,146,99],[255,48,49],[255,117,0],
 [255,203,74],[82,170,74],[140,235,173],[74,223,255],
 [132,146,206],[57,73,255],[156,81,239],[255,121,189],
 [148,97,74],[107,113,123],[189,190,189],[255,255,255]]
def validate_palette(palette):
 if not isinstance(palette,list) or len(palette)!=16 or any(not isinstance(c,list) or len(c)!=3 or any(type(v) is not int or not 0<=v<=255 for v in c) for c in palette):raise ValueError('Palette needs 16 RGB colors, 0..255')
 if palette[0]!=[0,0,0]:raise ValueError('Palette slot 0 must stay black')
 return palette
def header_palette():
 """Read ASTAROK_PALETTE RGB calls (or raw RGB565 literals), without eval."""
 text=(ROOT/'CurseOfAstarok/SpriteColors.h').read_text()
 text=re.sub(r'/\*.*?\*/|//[^\r\n]*','',text,flags=re.S)
 match=re.search(r'\bASTAROK_PALETTE\s*\[\s*(?:16)?\s*\]\s*=\s*\{(.*?)\}',text,re.S)
 if not match:raise ValueError('Cannot find ASTAROK_PALETTE[16] in SpriteColors.h')
 entries=[];depth=0;start=0;body=match[1]
 for i,c in enumerate(body):
  if c=='(':depth+=1
  elif c==')':depth-=1
  elif c==',' and depth==0:entries.append(body[start:i].strip());start=i+1
 if body[start:].strip():entries.append(body[start:].strip())
 if depth or len(entries)!=16:raise ValueError('SpriteColors.h needs exactly 16 palette entries')
 def integer(token):
  token=token.strip();m=re.fullmatch(r'(0[xX][0-9a-fA-F]+|[0-9]+)[uUlL]*',token)
  if not m:raise ValueError('Use integer RGB channels or RGB565 literals in ASTAROK_PALETTE')
  literal=m[1];return int(literal,16 if literal.lower().startswith('0x') else 10)
 palette=[]
 for entry in entries:
  rgb=re.fullmatch(r'ASTAROK_RGB\s*\((.*?)\)',entry,re.S)
  if rgb:
   parts=rgb[1].split(',')
   if len(parts)!=3:raise ValueError('ASTAROK_RGB needs three RGB channels')
   palette.append([integer(part) for part in parts])
  else:
   value=integer(entry)
   if not 0<=value<=65535:raise ValueError('RGB565 palette values must fit 16 bits')
   red=value>>11;green=(value>>5)&63;blue=value&31
   palette.append([(red<<3)|(red>>2),(green<<2)|(green>>4),(blue<<3)|(blue>>2)])
 return validate_palette(palette)
def load_palette(header=None):
 """Header edits win; unchanged slots retain saved editor RGB choices."""
 header=header_palette() if header is None else validate_palette(header)
 path=ROOT/'art/ColorPalette.json'
 saved=validate_palette(json.loads(path.read_text())) if path.exists() else header
 path=ROOT/'art/ColorPaletteHeader.json'
 reference=validate_palette(json.loads(path.read_text())) if path.exists() else LEGACY_HEADER_PALETTE
 return [list(h if h!=old else custom) for h,old,custom in zip(header,reference,saved)]
def save_palette(palette,header=None):
 header=header_palette() if header is None else validate_palette(header);validate_palette(palette)
 for name,data in [('ColorPalette.json',palette),('ColorPaletteHeader.json',header)]:
  path=ROOT/'art'/name;temp=path.with_suffix('.tmp');temp.write_text(json.dumps(data,indent=2)+'\n');temp.replace(path)

def load_bases():
 path=ROOT/'art/ColorBases.json'
 return json.loads(path.read_text()) if path.exists() else {}
def rgb565(rgb):return ((rgb[0]&248)<<8)|((rgb[1]&252)<<3)|(rgb[2]>>3)
def encode_frame(rows):
 values=[SYMBOLS.index(c) for r in rows for c in r];out=bytearray();i=0
 while i<len(values):
  n=1
  while i+n<len(values) and values[i+n]==values[i] and n<8:n+=1
  out.append(((n-1)<<5)|values[i]);i+=n
 return out
def decode_frame(data,w,h):
 out=[]
 for b in data:
  c=b&31;n=(b>>5)+1
  if c>=len(SYMBOLS) or len(out)+n>w*h:raise ValueError('Malformed color run')
  out.extend(SYMBOLS[c]*n)
  if len(out)==w*h:return [''.join(out[y*w:(y+1)*w]) for y in range(h)]
 raise ValueError('Truncated color frame')
def put24(data,pos,value):
 if not 0<=value<0x1000000:raise ValueError('Art pack exceeds 16 MB')
 data[pos:pos+3]=value.to_bytes(3,'little')
def get24(data,pos):return int.from_bytes(data[pos:pos+3],'little')
def build_pack(blocks=None,palette=None,bases=None):
 blocks=blocks if blocks is not None else load_art();palette=palette if palette is not None else load_palette()
 meta=json.loads((ROOT/'art/color_manifest.json').read_text());sprites=meta['sprites'];base=load_art(ROOT/'art/ColorBase.txt')
 if set(blocks)!=set(sprites):raise ValueError('Keep the original sprite names')
 bases=load_bases() if bases is None else bases
 if not isinstance(bases,dict) or set(bases)-set(sprites) or any(type(c) is not int or not 0<=c<16 for c in bases.values()):raise ValueError('Sprite bases must name an existing sprite and an opaque palette slot, 0..15')
 validate_palette(palette)
 data=bytearray(512);count=0
 for name,info in sprites.items():
  frames=blocks[name];w=info['width'];h=info['height'];nf=info['frames']
  if len(frames)!=nf or any(len(f)!=h or any(len(r)!=w or set(r)-set(SYMBOLS) for r in f) for f in frames):raise ValueError(f'{name}: keep {nf} frame(s), {w}x{h}')
  override=bases.get(name)
  if frames==base[name] and override is None:continue
  count+=1;start=len(data);put24(data,48+info['id']*3,start)
  data.extend(bytes([w,h,nf,0 if override is None else override+1])+bytes(3*nf))
  for i,rows in enumerate(frames):
   if rows==base[name][i] and override is None:continue
   put24(data,start+4+i*3,len(data));encoded=encode_frame(rows)
   assert decode_frame(encoded,w,h)==rows
   data.extend(encoded)
 struct.pack_into('<4sIIHH',data,0,MAGIC,meta['schema'],len(data),len(sprites),0)
 for i,c in enumerate(palette):struct.pack_into('<H',data,16+i*2,rgb565(c))
 return bytes(data),count
def rebuild():
 header=header_palette();palette=load_palette(header);data,count=build_pack(palette=palette);out=ROOT/'sdcard/ASTAROK.ART';out.parent.mkdir(parents=True,exist_ok=True);out.write_bytes(data);save_palette(palette,header)
 return count,len(data)
def header_colors():
 """Resolve the existing header's named family/default frame colors for preview."""
 import re
 text=(ROOT/'CurseOfAstarok/SpriteColors.h').read_text();defs=dict(re.findall(r'^#define\s+(\w+)\s+(\w+)\b',text,re.M))
 def value(s):
  for _ in range(10):
   if s.isdigit():return int(s)
   s=defs.get(s,'1')
  return 1
 colors={}
 meta=json.loads((ROOT/'art/color_manifest.json').read_text())['sprites']
 for name,info in meta.items():
  macro='ART_'+re.sub(r'([a-z])([A-Z])',r'\1_\2',name).upper();colors[name]=[value(macro)]*info['frames']
 colors['Rune_Symbols']=[value('RUNE_SYMBOL_'+str(i)) for i in range(8)]
 colors['ItemShop_Potions']=[value('POTION_'+s) for s in ['FIRE','HEALTH','SPEED']]
 return colors
if __name__=='__main__':
 n,size=rebuild();print(f'Exported {n} edited color sprite sheets, {size} bytes. Copy sdcard/ASTAROK.ART to the card root and restart the game. Monochrome was not changed.')
