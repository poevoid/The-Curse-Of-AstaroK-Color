"""Run under a display (or Xvfb): Tk editing, alpha PNG and safe saves."""
from pathlib import Path
import sys,shutil,tempfile,hashlib
from types import SimpleNamespace
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import color_art as art
import sprite_editor as gui
original=art.ROOT
with tempfile.TemporaryDirectory() as work:
 root=Path(work);shutil.copytree(original/'art',root/'art');shutil.copytree(original/'CurseOfAstarok',root/'CurseOfAstarok');(root/'sdcard').mkdir();art.ROOT=root
 frozen=[root/'CurseOfAstarok/SpriteColors.h',root/'art/MonochromeSprites.txt',*sorted((root/'CurseOfAstarok/src/artdata').glob('*.h'))]
 hashes={p:hashlib.sha256(p.read_bytes()).digest() for p in frozen}
 app=gui.Editor();app.update();app.search.set('Player_Idle_00');app.list.selection_set(0);app.select();app.update()
 assert app.name=='Player_Idle_00';assert len(app.blocks)==147;assert not any('mask' in n.lower() for n in app.blocks)
 header=root/'CurseOfAstarok/SpriteColors.h';header_text=header.read_text()
 # Editing the header while open refreshes changed slots without clobbering
 # an unsaved GUI override on another slot. Explicit Reload imports every slot.
 app.palette[2]=[77,88,99]
 header.write_text(header_text.replace('ASTAROK_RGB( 82, 170,  74)','ASTAROK_RGB(55, 109, 169)'))
 app.sync_header();assert app.palette[5]==[55,109,169] and app.palette[2]==[77,88,99]
 assert app.buttons[5].cget('background')=='#376da9'
 app.reload_header();assert app.palette[2]==[255,48,49]
 hashes[header]=hashlib.sha256(header.read_bytes()).digest()

 untouched=app.blocks[app.name][0][:];tints=app.tints[app.name][:]
 app.base_choice.current(1);app.change_base();assert app.bases[app.name]==0 and app.resolved('G')=='#000000'
 app.base_choice.current(0);app.change_base();assert app.name not in app.bases
 app.base_choice.current(2);app.change_base();assert app.bases[app.name]==1
 assert app.resolved('G')=='#ce9263' and app.blocks[app.name][0]==untouched and app.tints[app.name]==tints

 def event(x,y):return SimpleNamespace(x=int(app.ox+x*app.cell+app.cell//2-app.canvas.canvasx(0)),y=int(app.oy+y*app.cell+app.cell//2-app.canvas.canvasy(0)))
 # Exercise connected fill, isolated alpha, opaque protection and one-action undo.
 original_frames=[f[:] for f in app.blocks[app.name]]
 ring=['00000','0...0','0.0.0','0...0','00000']
 app.blocks[app.name]=[ring[:],ring[:]];app.frame=0;app.redraw();app.set_brush('2');app.tool.set('Bucket')
 app.begin(event(1,1),'2');app.end();filled=app.blocks[app.name][0][:]
 assert sum(row.count('2') for row in filled)==8 and filled[2][2]=='0' and filled[0]=='00000'
 assert app.blocks[app.name][1]==ring
 app.undo();assert app.blocks[app.name][0]==ring
 app.redo();assert app.blocks[app.name][0]==filled
 app.undo();app.blocks[app.name][0]=['.0.','0.0','.0.'];app.redraw()
 app.begin(event(0,0),'2');app.end();assert app.blocks[app.name][0]==['20.','0.0','.0.']
 app.undo();assert app.blocks[app.name][0]==['.0.','0.0','.0.']
 app.blocks[app.name][0]=ring[:];app.redraw();app.tool.set('Alpha only')
 app.begin(event(0,0),'2');app.stroke(event(4,4));app.end()
 assert app.blocks[app.name][0]==['00000','02..0','0.0.0','0..20','00000']
 app.undo();assert app.blocks[app.name][0]==ring
 app.set_brush('0');app.replace_alpha();assert app.blocks[app.name][0]==['00000']*5
 assert app.blocks[app.name][1]==ring
 app.undo();app.set_brush('G');app.replace_alpha();assert sum(row.count('G') for row in app.blocks[app.name][0])==8
 app.undo();app.set_brush('.');history=len(app.history);app.replace_alpha();assert len(app.history)==history and app.blocks[app.name][0]==ring
 # Actual Tk mouse bindings invoke the selected bucket tool.
 app.tool.set('Bucket');app.set_brush('2');e=event(1,1)
 app.canvas.event_generate('<ButtonPress-1>',x=e.x,y=e.y);app.canvas.event_generate('<ButtonRelease-1>',x=e.x,y=e.y);app.update()
 assert app.blocks[app.name][0]==filled;app.undo()
 # Large sprites exercise scrolling, nonrecursive filling and coordinate mapping.
 app.blocks[app.name][0]=['G'*128]*64;app.redraw();app.set_zoom(64);app.update()
 assert app.cell==64 and app.zoom_choice.get()=='64x'
 app.canvas.xview_moveto(.6);app.canvas.yview_moveto(.5);app.update()
 center=SimpleNamespace(x=app.canvas.winfo_width()//2,y=app.canvas.winfo_height()//2)
 px,py=app.point(center);assert px>20 and py>10
 app.tool.set('Pencil');app.begin(center,'2');app.end();assert app.blocks[app.name][0][py][px]=='2'
 assert sum(row.count('2') for row in app.blocks[app.name][0])==1
 app.undo();assert app.blocks[app.name][0]==['G'*128]*64
 app.tool.set('Bucket');app.begin(center,'4');app.end();assert app.blocks[app.name][0]==['4'*128]*64
 app.undo();app.set_zoom(32,center);app.update();qx,qy=app.point(center);assert abs(px-qx)<=1 and abs(py-qy)<=1
 before_zoom=[f[:] for f in app.blocks[app.name]];before_history=len(app.history)
 app.wheel_zoom(SimpleNamespace(x=center.x,y=center.y,delta=120));assert app.cell==48
 app.zoom_by(-1);assert app.cell==32
 app.set_zoom(1);assert app.cell==1;app.zoom_by(-1);assert app.cell==1
 app.set_zoom(64);app.zoom_by(1);assert app.cell==64
 app.set_zoom(None);assert app.zoom is None and app.zoom_choice.get()=='Fit'
 assert app.blocks[app.name]==before_zoom and len(app.history)==before_history
 app.blocks[app.name]=original_frames;app.frame=0;app.history.clear();app.redo_stack.clear();app.tool.set('Pencil');app.redraw();app.refresh_palette()

 before=app.blocks[app.name][0][:]
 for x,c in enumerate('20G.'):
  app.begin(event(x,0),c);app.end()
 assert app.blocks[app.name][0][0][:4]=='20G.'
 app.undo();app.redo();assert app.blocks[app.name][0][0][:4]=='20G.'
 app.pick(event(3,0));assert app.brush=='.';assert gui.NAMES[17]=='ALPHA'
 png=root/'frame.png';gui.filedialog.asksaveasfilename=lambda **_:str(png);app.export_png()
 im=gui.tk.PhotoImage(file=str(png));assert im.transparency_get(3,0) and not im.transparency_get(1,0);assert im.get(1,0)==(0,0,0)
 gui.filedialog.askopenfilename=lambda **_:str(png);app.import_png();assert app.blocks[app.name][0][0][3]=='.'
 gui.colorchooser.askcolor=lambda *_,**kw:((39,91,223),'#275bdf');app.change_rgb(2);assert app.palette[2]==[39,91,223]
 # Save also picks up a newly edited header without needing the Reload button.
 header.write_text(header.read_text().replace('ASTAROK_RGB( 57,  73, 255)','ASTAROK_RGB(68, 90, 112)'))
 hashes[header]=hashlib.sha256(header.read_bytes()).digest()
 assert app.save();assert art.load_art()[app.name]==app.blocks[app.name];assert art.load_bases()=={app.name:1}
 assert app.palette[9]==[68,90,112] and app.palette[2]==[39,91,223]
 reloaded=gui.Editor();reloaded.update();reloaded.name=app.name;reloaded.frame=0;reloaded.refresh_palette();assert reloaded.base_choice.current()==2 and reloaded.resolved('G')=='#ce9263';assert reloaded.palette[9]==[68,90,112] and reloaded.palette[2]==[39,91,223];reloaded.destroy()
 assert all(hashlib.sha256(p.read_bytes()).digest()==hashes[p] for p in frozen)
 # Capture the shipped default editor, rather than the temporary test edits.
 app.dirty=False;app.destroy();art.ROOT=original;app=gui.Editor();app.update();app.search.set('Player_Idle_00');app.list.selection_set(0);app.select();app.update();app.set_brush('.');app.update()
 capture=__import__('os').environ.get('ASTAROK_EDITOR_SCREENSHOT')
 if capture:
  from PIL import ImageGrab
  ImageGrab.grab(xdisplay=__import__('os').environ.get('DISPLAY')).save(capture)
 app.destroy()
print('PASS: bucket boundaries/diagonals/8192-pixel fill, alpha-only brush, ALPHA replacement, per-frame scope, undo/redo, zoom/scroll mapping, header RGB sync/reload/export, saved editor RGB overrides, base colors, PNG roundtrip and safe saves; monochrome hashes unchanged')
