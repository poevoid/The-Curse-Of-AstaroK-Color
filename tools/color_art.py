"""Editable color-only artwork and the SD art pack. Standard Python only.
Monochrome artwork and firmware source are never written by this module.
"""
from pathlib import Path
import json,struct
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
def load_palette():return json.loads((ROOT/'art/ColorPalette.json').read_text())
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
 if len(palette)!=16 or any(len(c)!=3 or any(not isinstance(v,int) or not 0<=v<=255 for v in c) for c in palette):raise ValueError('Palette needs 16 RGB colors, 0..255')
 if palette[0]!=[0,0,0]:raise ValueError('Palette slot 0 must stay black')
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
 data,count=build_pack();out=ROOT/'sdcard/ASTAROK.ART';out.parent.mkdir(parents=True,exist_ok=True);out.write_bytes(data)
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
