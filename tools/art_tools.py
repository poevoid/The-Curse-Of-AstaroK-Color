"""Legacy monochrome reference reader/codec; rebuild exports COLOR assets only. Standard Python only."""
from pathlib import Path
import json,re
ROOT=Path(__file__).resolve().parents[1]

def decode(data,kind,w=None,h=None):
    if kind=='compressed':
        w=(data[0]&127)+1;h=(data[1]&63)+1;pos=16;value=data[0]>>7;reverse=bool(data[1]&64);zig=bool(data[1]&128)
        raw=[0]*(w*((h+7)//8));out=0
        def bit():
            nonlocal pos
            v=(data[pos//8]>>(pos%8))&1;pos+=1;return v
        while out<len(raw)*8:
            size=1
            while bit():size+=1
            n=1+bit() if size==1 else (1<<(size-1))+1+sum(bit()<<k for k in range(size-1))
            for _ in range(min(n,len(raw)*8-out)):
                if value:raw[out//8]|=1<<(7-out%8 if reverse else out%8)
                out+=1
                if zig and out%8==0:reverse=not reverse
            value^=1
        data=raw
    elif kind=='native':w,h=data[:2];data=data[2:]
    size=w*((h+7)//8)
    assert len(data)%size==0,(len(data),w,h)
    return [ [''.join('#' if data[f*size+(y//8)*w+x]&(1<<(y%8)) else '.' for x in range(w)) for y in range(h)] for f in range(len(data)//size)]

def encode(frames,kind):
    w=len(frames[0][0]);h=len(frames[0]);raw=[]
    for rows in frames:
        assert len(rows)==h and all(len(r)==w and set(r)<=set('.#') for r in rows),'Invalid sprite rows'
        for page in range((h+7)//8):
            for x in range(w):raw.append(sum((1<<b) for b in range(8) if page*8+b<h and rows[page*8+b][x]=='#'))
    if kind=='native':return [w,h]+raw
    if kind=='mask':return raw
    assert len(frames)==1
    best=None
    for rev in (0,1):
      for zig in (0,1):
        sequence=[];direction=rev
        for v in raw:
            sequence.extend((v>>(7-b if direction else b))&1 for b in range(8))
            if zig:direction^=1
        first=sequence[0];bits=[];i=0
        while i<len(sequence):
            j=i+1
            while j<len(sequence) and sequence[j]==sequence[i]:j+=1
            n=j-i
            if n<=2:bits.extend((0,n-1))
            else:
                size=(n-1).bit_length();bits.extend([1]*(size-1)+[0]);res=n-(1<<(size-1))-1;bits.extend((res>>k)&1 for k in range(size-1))
            i=j
        out=[(w-1)|(first<<7),(h-1)|(rev<<6)|(zig<<7)]+[sum(v<<k for k,v in enumerate(bits[j:j+8])) for j in range(0,len(bits),8)]
        if best is None or len(out)<len(best):best=out
    return best

def load_art(path):
    blocks={};name=None;frames=[];rows=[]
    for line in Path(path).read_text().splitlines():
        s=line.strip()
        if not s or s.startswith(';'):continue
        if s.startswith('@'):
            if name:
                if rows:frames.append(rows)
                blocks[name]=frames
            name=s[1:];frames=[];rows=[]
        elif s=='---':
            if rows:frames.append(rows);rows=[]
        else:
            if name is None or set(s)-set('.#'):raise ValueError('Expected @Name, --- or pixel rows')
            rows.append(s)
    if name:
        if rows:frames.append(rows)
        blocks[name]=frames
    return blocks

def write_art(blocks,path=None):
    text='; Editable sprite pixels: # = foreground/mask, . = empty.\n; @Name starts a sprite sheet; --- separates animation frames.\n; Keep dimensions and frame counts unchanged. Edit masks alongside silhouettes.\n\n'
    for name,frames in blocks.items():text+='@'+name+'\n'+'\n---\n'.join('\n'.join(r) for r in frames)+'\n\n'
    if path is None:raise ValueError('Monochrome reference is protected; use color_art.write_art for edits')
    Path(path).write_text(text)

def rebuild():
    from color_art import rebuild as rebuild_color
    return rebuild_color()
if __name__=='__main__':
    count,size=rebuild();print(f'Exported {count} color sheets ({size} bytes). Copy sdcard/ASTAROK.ART to the card root; no recompile needed. Monochrome is unchanged.')
