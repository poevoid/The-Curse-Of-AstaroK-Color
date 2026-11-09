"""Color-only sprite editor. Python 3/Tkinter; no pip dependencies."""
import tkinter as tk
from tkinter import ttk,messagebox,filedialog,colorchooser
import json
import color_art as art
ZOOM_LEVELS=(1,2,4,8,12,16,24,32,48,64)
NAMES=['BLACK','LT BROWN','RED','ORANGE','GOLD','GREEN','MINT','CYAN','SKY','BLUE','PURPLE','PINK','BROWN','SLATE','SILVER','WHITE','BASE','ALPHA']
class Editor(tk.Tk):
 def __init__(self):
  super().__init__();self.title('Curse of Astarok - Color Sprite Editor');self.geometry('1200x860');self.minsize(940,700)
  self.blocks=art.load_art();self.base=art.load_art(art.ROOT/'art/ColorBase.txt');self.header_palette=art.header_palette();self.palette=art.load_palette(self.header_palette);self.tints=art.header_colors();self.bases=art.load_bases();self.name=None;self.frame=0;self.dirty=False;self.paint=None;self.brush='G';self.history=[];self.redo_stack=[];self.last=None;self.zoom=None;self.stroke_started=False;self.only_alpha=False
  left=ttk.Frame(self,padding=10);left.pack(side='left',fill='y');ttk.Label(left,text='COLOR sprites only').pack(anchor='w')
  self.search=tk.StringVar();self.search.trace_add('write',lambda *_:self.filter());ttk.Entry(left,textvariable=self.search,width=30).pack(fill='x',pady=8)
  self.list=tk.Listbox(left,width=29,exportselection=False);self.list.pack(fill='both',expand=True);self.list.bind('<<ListboxSelect>>',self.select)
  ttk.Label(left,text='Monochrome art and masks\nare protected from this editor.').pack(anchor='w',pady=10)
  right=ttk.Frame(self,padding=10);right.pack(side='right',fill='both',expand=True)
  bar=ttk.Frame(right);bar.pack(fill='x');ttk.Button(bar,text='< Frame',command=lambda:self.step(-1)).pack(side='left');ttk.Button(bar,text='Frame >',command=lambda:self.step(1)).pack(side='left');ttk.Button(bar,text='Save + export SD art',command=self.save).pack(side='right')
  basebar=ttk.Frame(right);basebar.pack(fill='x',pady=(8,0))
  ttk.Label(basebar,text='Sprite BASE (all frames):').pack(side='left')
  self.base_choice=ttk.Combobox(basebar,state='readonly',values=['HEADER (default)']+NAMES[:16],width=19);self.base_choice.pack(side='left',padx=8);self.base_choice.bind('<<ComboboxSelected>>',self.change_base)
  ttk.Button(basebar,text='Reload header RGB',command=self.reload_header).pack(side='right')
  pal=ttk.LabelFrame(right,text='Palette follows SpriteColors.h changes - double-click for an editor RGB override',padding=5);pal.pack(fill='x',pady=8);self.buttons=[]
  for i,n in enumerate(NAMES):
   code=art.SYMBOLS[i];b=tk.Button(pal,text=n,width=8,command=lambda c=code:self.set_brush(c));b.grid(row=i//9,column=i%9,sticky='ew',padx=1,pady=1);b.bind('<Double-Button-1>',lambda e,j=i:self.change_rgb(j));self.buttons.append(b)
  for i in range(9):pal.columnconfigure(i,weight=1)
  self.brush_label=ttk.Label(right,text='');self.brush_label.pack(anchor='w')
  self.info=ttk.Label(right,text='');self.info.pack(anchor='w',pady=6)
  toolbar=ttk.Frame(right);toolbar.pack(fill='x',pady=(0,6));self.tool=tk.StringVar(value='Pencil')
  for name in ('Pencil','Bucket','Alpha only'):ttk.Radiobutton(toolbar,text=name,variable=self.tool,value=name).pack(side='left',padx=(0,8))
  self.alpha_replace_button=ttk.Button(toolbar,text='Replace all ALPHA',command=self.replace_alpha);self.alpha_replace_button.pack(side='left',padx=6)
  ttk.Button(toolbar,text='-',width=3,command=lambda:self.zoom_by(-1)).pack(side='left',padx=(12,2))
  self.zoom_choice=ttk.Combobox(toolbar,state='readonly',values=['Fit']+[str(n)+'x' for n in ZOOM_LEVELS],width=6);self.zoom_choice.pack(side='left');self.zoom_choice.set('Fit');self.zoom_choice.bind('<<ComboboxSelected>>',self.choose_zoom)
  ttk.Button(toolbar,text='+',width=3,command=lambda:self.zoom_by(1)).pack(side='left',padx=2)
  ttk.Button(toolbar,text='Fit',width=4,command=lambda:self.set_zoom(None)).pack(side='left')
  view=ttk.Frame(right);view.pack(fill='both',expand=True);view.rowconfigure(0,weight=1);view.columnconfigure(0,weight=1)
  self.canvas=tk.Canvas(view,bg='#292b31',highlightthickness=0,cursor='crosshair');self.canvas.grid(row=0,column=0,sticky='nsew')
  xscroll=ttk.Scrollbar(view,orient='horizontal',command=self.canvas.xview);xscroll.grid(row=1,column=0,sticky='ew')
  yscroll=ttk.Scrollbar(view,orient='vertical',command=self.canvas.yview);yscroll.grid(row=0,column=1,sticky='ns')
  self.canvas.configure(xscrollcommand=xscroll.set,yscrollcommand=yscroll.set)
  self.canvas.bind('<Configure>',lambda e:self.redraw());self.canvas.bind('<Button-1>',lambda e:self.begin(e,self.brush));self.canvas.bind('<B1-Motion>',self.stroke);self.canvas.bind('<Button-3>',lambda e:self.begin(e,'.',erase=True));self.canvas.bind('<B3-Motion>',self.stroke);self.canvas.bind('<Button-2>',self.pick)
  self.canvas.bind('<MouseWheel>',self.wheel_zoom);self.canvas.bind('<Button-4>',lambda e:self.zoom_by(1,e));self.canvas.bind('<Button-5>',lambda e:self.zoom_by(-1,e))
  for b in (1,3):self.canvas.bind(f'<ButtonRelease-{b}>',lambda e:self.end())
  bottom=ttk.Frame(right);bottom.pack(fill='x',pady=10)
  for text,fn in [('Undo',self.undo),('Redo',self.redo),('Reset frame',self.reset),('Import PNG',self.import_png),('Export PNG',self.export_png)]:ttk.Button(bottom,text=text,command=fn).pack(side='left',padx=3)
  self.status=tk.StringVar(value='Choose a tool and brush. Right: erase | Middle: eyedropper | Wheel: zoom | Scrollbars: pan.');ttk.Label(right,textvariable=self.status,wraplength=820).pack(anchor='w')
  self.bind('<Control-z>',lambda e:self.undo());self.bind('<Control-y>',lambda e:self.redo());self.protocol('WM_DELETE_WINDOW',self.close)
  self.filter();self.list.selection_set(0);self.select();self.refresh_palette()
 def filter(self):
  self.list.delete(0,'end')
  for n in self.blocks:
   if self.search.get().lower() in n.lower():self.list.insert('end',n)
 def select(self,event=None):
  sel=self.list.curselection()
  if sel:self.name=self.list.get(sel[0]);self.frame=0;self.redraw();self.canvas.xview_moveto(0);self.canvas.yview_moveto(0);self.refresh_palette()
 def step(self,n):
  if self.name:self.frame=(self.frame+n)%len(self.blocks[self.name]);self.redraw();self.refresh_palette()
 def resolved(self,c):
  i=art.SYMBOLS.index(c)
  if i==16:i=self.bases.get(self.name,self.tints[self.name][self.frame]) if self.name else 15
  if i==17:return '#353740'
  return '#%02x%02x%02x'%tuple(self.palette[i])
 def refresh_palette(self):
  if self.name:self.base_choice.current(self.bases[self.name]+1 if self.name in self.bases else 0)
  for i,b in enumerate(self.buttons):
   rgb=self.resolved(art.SYMBOLS[i]);r,g,blue=(int(rgb[k:k+2],16) for k in (1,3,5));fg='black' if r*.3+g*.59+blue*.11>140 else 'white'
   b.config(bg=rgb,fg=fg,relief='sunken' if art.SYMBOLS[i]==self.brush else 'raised')
  self.alpha_replace_button.config(state='disabled' if self.brush=='.' else 'normal')
  self.brush_label.config(text='Brush: '+NAMES[art.SYMBOLS.index(self.brush)]+(' - sprite base/player color' if self.brush=='G' else ''))
 def change_base(self,event=None):
  if not self.name:return
  choice=self.base_choice.current()
  if choice<0:return
  old=self.bases.get(self.name)
  if choice==0:self.bases.pop(self.name,None)
  else:self.bases[self.name]=choice-1
  if self.bases.get(self.name)==old:return
  self.dirty=True;self.redraw();self.refresh_palette()
  self.status.set('Sprite BASE changed for all frames. Painted colors and ALPHA stay unchanged. Save + export, copy ASTAROK.ART and restart.')
 def set_brush(self,c):self.brush=c;self.refresh_palette()
 def sync_header(self,force=False):
  header=art.header_palette();tints=art.header_colors()
  palette=[list(h if force or h!=old else custom) for h,old,custom in zip(header,self.header_palette,self.palette)]
  changed=palette!=self.palette
  self.header_palette=header;self.palette=palette;self.tints=tints
  if changed:self.dirty=True
  self.redraw();self.refresh_palette()
  return changed
 def reload_header(self):
  try:
   self.sync_header(force=True);self.status.set('Loaded RGB colors from SpriteColors.h. Save + export, then copy ASTAROK.ART to update the game.')
  except Exception as e:messagebox.showerror('Could not read header colors',str(e),parent=self)
 def change_rgb(self,i):
  if not 1<=i<=15:return
  result=colorchooser.askcolor(self.resolved(art.SYMBOLS[i]),parent=self,title=NAMES[i]+' - color palette RGB')
  if result[0]:self.palette[i]=[round(v) for v in result[0]];self.dirty=True;self.redraw();self.refresh_palette();self.status.set('Color palette changed. Monochrome artwork and tints are unchanged.')
 def fill(self,c,x,y):return ('#252730' if (x+y)%2 else '#3b3e48') if c=='.' else self.resolved(c)
 def redraw(self):
  if not self.name:return
  rows=self.blocks[self.name][self.frame];w=len(rows[0]);h=len(rows)
  cw=max(1,self.canvas.winfo_width());ch=max(1,self.canvas.winfo_height())
  self.cell=self.zoom if self.zoom is not None else max(1,min((cw-20)//w,(ch-20)//h,32))
  self.ox=max(10,(cw-w*self.cell)//2);self.oy=max(10,(ch-h*self.cell)//2)
  self.view_width=max(cw,w*self.cell+20);self.view_height=max(ch,h*self.cell+20)
  self.canvas.configure(scrollregion=(0,0,self.view_width,self.view_height),xscrollincrement=self.cell,yscrollincrement=self.cell)
  self.canvas.delete('all');self.items=[]
  for y,row in enumerate(rows):
   ids=[]
   for x,p in enumerate(row):
    xx=self.ox+x*self.cell;yy=self.oy+y*self.cell;ids.append(self.canvas.create_rectangle(xx,yy,xx+self.cell,yy+self.cell,fill=self.fill(p,x,y),outline='#626571' if self.cell>=5 else '',width=1))
   self.items.append(ids)
  self.info.config(text=f'{self.name} | {w} x {h} | Frame {self.frame+1}/{len(self.blocks[self.name])} | {self.cell}x | color mode only')
 def choose_zoom(self,event=None):
  choice=self.zoom_choice.get();self.set_zoom(None if choice=='Fit' else int(choice[:-1]))
 def set_zoom(self,level,event=None):
  if not self.name:return
  self.end();ax=event.x if event is not None else self.canvas.winfo_width()/2;ay=event.y if event is not None else self.canvas.winfo_height()/2
  sx=(self.canvas.canvasx(ax)-self.ox)/self.cell;sy=(self.canvas.canvasy(ay)-self.oy)/self.cell
  self.zoom=None if level is None else max(1,min(64,int(level)));self.zoom_choice.set('Fit' if self.zoom is None else str(self.zoom)+'x');self.redraw()
  if self.zoom is None:self.canvas.xview_moveto(0);self.canvas.yview_moveto(0)
  else:
   self.canvas.xview_moveto(max(0,self.ox+sx*self.cell-ax)/self.view_width)
   self.canvas.yview_moveto(max(0,self.oy+sy*self.cell-ay)/self.view_height)
 def zoom_by(self,step,event=None):
  levels=[n for n in ZOOM_LEVELS if n>self.cell] if step>0 else [n for n in ZOOM_LEVELS if n<self.cell]
  if levels:self.set_zoom(min(levels) if step>0 else max(levels),event)
  return 'break'
 def wheel_zoom(self,event):
  if event.delta:self.zoom_by(1 if event.delta>0 else -1,event)
  return 'break'
 def point(self,e):return int((self.canvas.canvasx(e.x)-self.ox)//self.cell),int((self.canvas.canvasy(e.y)-self.oy)//self.cell)
 def checkpoint(self):
  self.history.append((self.name,self.frame,self.blocks[self.name][self.frame][:]));self.history=self.history[-100:];self.redo_stack.clear()
 def begin(self,e,c,erase=False):
  if not self.name:return
  self.end()
  if self.tool.get()=='Bucket' and not erase:self.bucket(e,c);return
  self.paint=c;self.only_alpha=self.tool.get()=='Alpha only' and not erase;self.stroke(e)
 def end(self):self.paint=None;self.last=None;self.stroke_started=False
 def stroke(self,e):
  if not self.name or self.paint is None:return
  rows=self.blocks[self.name][self.frame];x,y=self.point(e)
  if not (0<=y<len(rows) and 0<=x<len(rows[0])):self.last=None;return
  x0,y0=self.last or (x,y);n=max(abs(x-x0),abs(y-y0),1);changed=False
  for step in range(n+1):
   px=round(x0+(x-x0)*step/n);py=round(y0+(y-y0)*step/n);old=rows[py][px]
   if old==self.paint or (self.only_alpha and old!='.'):continue
   if not self.stroke_started:self.checkpoint();self.stroke_started=True
   rows[py]=rows[py][:px]+self.paint+rows[py][px+1:];self.canvas.itemconfigure(self.items[py][px],fill=self.fill(self.paint,px,py));changed=True
  self.last=(x,y)
  if changed:self.dirty=True;self.status.set('Unsaved color edits. Save + export, then copy ASTAROK.ART to the SD card root and restart.')
 def bucket(self,e,c):
  rows=self.blocks[self.name][self.frame];x,y=self.point(e);w=len(rows[0]);h=len(rows)
  if not (0<=x<w and 0<=y<h) or rows[y][x]==c:return
  target=rows[y][x];grid=[list(row) for row in rows];todo=[(x,y)];grid[y][x]=c;count=0
  while todo:
   px,py=todo.pop();count+=1
   for nx,ny in ((px-1,py),(px+1,py),(px,py-1),(px,py+1)):
    if 0<=nx<w and 0<=ny<h and grid[ny][nx]==target:grid[ny][nx]=c;todo.append((nx,ny))
  self.checkpoint();self.blocks[self.name][self.frame]=[''.join(row) for row in grid];self.dirty=True;self.redraw();self.status.set(f'Bucket filled {count} connected pixels. Undo restores the fill.')
 def replace_alpha(self):
  if not self.name or self.brush=='.':return
  rows=self.blocks[self.name][self.frame];count=sum(row.count('.') for row in rows)
  if not count:self.status.set('This frame has no ALPHA pixels.');return
  self.end();self.checkpoint();self.blocks[self.name][self.frame]=[row.replace('.',self.brush) for row in rows];self.dirty=True;self.redraw();self.status.set(f'Replaced {count} ALPHA pixels with {NAMES[art.SYMBOLS.index(self.brush)]} in this frame. Undo restores transparency.')
 def pick(self,e):
  if not self.name:return
  x,y=self.point(e);rows=self.blocks[self.name][self.frame]
  if 0<=y<len(rows) and 0<=x<len(rows[0]):self.set_brush(rows[y][x])
 def undo(self):self.swap(self.history,self.redo_stack)
 def redo(self):self.swap(self.redo_stack,self.history)
 def swap(self,src,dst):
  if not src:return
  n,f,old=src.pop();dst.append((n,f,self.blocks[n][f][:]));self.blocks[n][f]=old;self.name=n;self.frame=f;self.dirty=True;self.redraw();self.refresh_palette()
 def reset(self):
  if self.name and messagebox.askyesno('Reset color frame','Restore this color frame to its original base artwork?',parent=self):self.checkpoint();self.blocks[self.name][self.frame]=self.base[self.name][self.frame][:];self.dirty=True;self.redraw()
 def save(self):
  try:
   self.sync_header()
   data,n=art.build_pack(self.blocks,self.palette,self.bases)
   art.write_art(self.blocks);art.save_palette(self.palette,self.header_palette)
   (art.ROOT/'art/ColorBases.json').write_text(json.dumps(self.bases,indent=2)+'\n')
   out=art.ROOT/'sdcard/ASTAROK.ART';tmp=out.with_suffix('.tmp');tmp.write_bytes(data);tmp.replace(out)
   self.dirty=False;self.status.set(f'Saved {n} edited color sheets ({len(data)} bytes). Copy sdcard/ASTAROK.ART to the card root and restart; no recompile.');return True
  except Exception as e:messagebox.showerror('Could not export',str(e),parent=self);return False
 def export_png(self):
  if not self.name:return
  path=filedialog.asksaveasfilename(defaultextension='.png',initialfile=f'{self.name}_{self.frame}.png',filetypes=[('PNG','*.png')])
  if not path:return
  rows=self.blocks[self.name][self.frame];im=tk.PhotoImage(width=len(rows[0]),height=len(rows))
  for y,row in enumerate(rows):
   for x,p in enumerate(row):
    if p!='.':im.put(self.resolved(p),(x,y))
  im.write(path,format='png')
 def import_png(self):
  if not self.name:return
  path=filedialog.askopenfilename(filetypes=[('PNG','*.png')])
  if not path:return
  try:
   im=tk.PhotoImage(file=path);old=self.blocks[self.name][self.frame];w=len(old[0]);h=len(old)
   if (im.width(),im.height())!=(w,h):raise ValueError(f'Frame must be {w} x {h} pixels.')
   rows=[]
   for y in range(h):
    row=''
    for x in range(w):
     if im.transparency_get(x,y):row+='.'
     else:
      rgb=im.get(x,y);i=min(range(16),key=lambda i:sum((rgb[k]-self.palette[i][k])**2 for k in range(3)));row+=art.SYMBOLS[i]
    rows.append(row)
   self.checkpoint();self.blocks[self.name][self.frame]=rows;self.dirty=True;self.redraw();self.status.set('Imported PNG, mapped to the 16-color palette. Monochrome unchanged.')
  except Exception as e:messagebox.showerror('Could not import',str(e),parent=self)
 def close(self):
  if self.dirty:
   choice=messagebox.askyesnocancel('Unsaved color edits','Save and export before closing?',parent=self)
   if choice is None:return
   if choice and not self.save():return
  self.destroy()
if __name__=='__main__':Editor().mainloop()
