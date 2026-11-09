"""Header RGB parsing/sync/export, including existing editor overrides."""
from pathlib import Path
import sys,tempfile,shutil,json,struct
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import color_art as a
original=a.ROOT
with tempfile.TemporaryDirectory() as work:
 a.ROOT=Path(work);shutil.copytree(original/'art',a.ROOT/'art');(a.ROOT/'CurseOfAstarok').mkdir()
 header=a.ROOT/'CurseOfAstarok/SpriteColors.h';original_header=(original/'CurseOfAstarok/SpriteColors.h').read_text();header.write_text(original_header)
 (a.ROOT/'art/ColorPaletteHeader.json').unlink(missing_ok=True)
 baseline=a.header_palette();assert baseline==a.LEGACY_HEADER_PALETTE
 saved=[c[:] for c in baseline];saved[2]=[244,19,22];saved[9]=[11,22,33]
 (a.ROOT/'art/ColorPalette.json').write_text(json.dumps(saved))
 # Existing GUI choices survive untouched header slots; a changed header wins.
 header.write_text(original_header.replace('ASTAROK_RGB(255,  48,  49)','ASTAROK_RGB( /* RGB */ 0x0D,\n201u, 92)'))
 parsed=a.header_palette();assert parsed[2]==[13,201,92]
 merged=a.load_palette();assert merged[2]==[13,201,92] and merged[9]==[11,22,33]
 merged[2]=[24,80,190];a.save_palette(merged,parsed)
 assert a.load_palette()[2]==[24,80,190] # saved GUI override, same header
 header.write_text(original_header)
 assert a.load_palette()[2]==baseline[2] and a.load_palette()[9]==[11,22,33] # reverting header wins too
 # Raw RGB565 literals can be mixed with RGB macros.
 header.write_text(original_header.replace('ASTAROK_RGB(255,  48,  49)','0x07E0u'))
 assert a.header_palette()[2]==[0,255,0]
 a.rebuild();pack=(a.ROOT/'sdcard/ASTAROK.ART').read_bytes()
 assert struct.unpack_from('<H',pack,20)[0]==0x07e0
 assert json.loads((a.ROOT/'art/ColorPalette.json').read_text())[2]==[0,255,0]
 assert json.loads((a.ROOT/'art/ColorPaletteHeader.json').read_text())[2]==[0,255,0]
 # Header is a read-only input and malformed values never execute code/fall back silently.
 assert header.read_text()==original_header.replace('ASTAROK_RGB(255,  48,  49)','0x07E0u')
 for bad in ['ASTAROK_RGB(256, 0, 0)','ASTAROK_RGB(1, 2)','execute_something()']:
  header.write_text(original_header.replace('ASTAROK_RGB(255,  48,  49)',bad))
  try:a.header_palette()
  except ValueError:pass
  else:raise AssertionError(bad)
 header.write_text(original_header.replace('ASTAROK_RGB(  0,   0,   0)','ASTAROK_RGB(1, 0, 0)'))
 try:a.header_palette()
 except ValueError:pass
 else:raise AssertionError('Black slot must remain black')
 a.ROOT=original
print('PASS: header RGB/hex/comments, header-change precedence, preserved editor overrides, header reverts, RGB565 export, invalid input and read-only header')
