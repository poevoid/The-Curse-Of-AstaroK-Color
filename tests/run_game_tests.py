"""Usage: python tests/run_game_tests.py PATH_TO_OFFICIAL_CHGAME_REPO
Requires g++ and the CHGame tools/chsim host harness. No device required.
"""
from pathlib import Path
import sys,os,subprocess,tempfile,json
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
import art_tools as art
repo=Path(sys.argv[1]).resolve();root=Path(__file__).resolve().parents[1];sk=root/'CurseOfAstarok'
sys.path.insert(0,str(repo/'tools/chsim'));import chsim
host=repo/'tools/chsim/host';gfx=chsim.chgfx_dir();game=chsim.chgame_dir()
os.environ['CHSIM_CXX']='g++';os.environ['CHSIM_FLAGS']='-fpermissive -include chrono -fsanitize=address,undefined -fno-omit-frame-pointer'
with tempfile.TemporaryDirectory() as td:
 td=Path(td);unit=td/'game.cpp';ino=[sk/'CurseOfAstarok.ino']+sorted(p for p in sk.glob('*.ino') if p.name!='CurseOfAstarok.ino')
 unit.write_text('#include <Arduino.h>\n'+'\n'.join(p.read_text() for p in ino))
 # Expected pixels come from the readable art, independently of the C++ blitter.
 manifest=json.loads((root/'art/manifest.json').read_text());blocks=art.load_art(root/'art/MonochromeSprites.txt')
 render=td/'render.cpp'
 text='#include "src/utils/Arduboy2Ext.h"\n#include "src/utils/Constants.h"\n#include "src/images/Images.h"\n#include "src/ardBitmap/ArdBitmap.h"\n#include <cassert>\n'
 cases=[]
 for name,info in manifest.items():
  if info['kind']=='mask':continue
  data=art.encode(blocks[name],'native')[2:]
  text+='static const uint8_t e_'+name+'[]={'+','.join(map(str,data))+'};\n'
  cases.append('{Images::'+name+',e_'+name+','+','.join(map(str,[info['width'],info['height'],info['frames'],int(info.get('bitmap_call',False))]))+'}')
 text+='struct Case {const uint8_t *image,*expect; int w,h,frames;bool bitmapCall;};\nstatic const Case cases[]={'+','.join(cases)+'};\n'
 text+=r'''void testAllSpritePixels() {
  ArdBitmap<128,64> bitmap;int checked=0;
  const int positions[][2]={{0,0},{-3,-3},{120,60}};
  for(const auto &c:cases)for(int frame=0;frame<c.frames;++frame)
   for(const auto &pos:positions)for(int mirror=0;mirror<(c.bitmapCall?4:1);++mirror) {
    Gfx.clear(COLOR_PINK);
    if(c.bitmapCall)bitmap.drawCompressed(pos[0],pos[1],c.image,WHITE,mirror);
    else SpritesB::drawSelfMasked(pos[0],pos[1],c.image,frame);
    int bytes=c.w*((c.h+7)/8);uint8_t ink=spriteColor(c.image,frame);
    for(int y=0;y<64;++y)for(int x=0;x<128;++x) {
      int sx=x-pos[0],sy=y-pos[1];uint8_t expect=COLOR_PINK;
      if(sx>=0 && sx<c.w && sy>=0 && sy<c.h) {
       if(mirror&1)sx=c.w-1-sx;if(mirror&2)sy=c.h-1-sy;
       if(c.expect[frame*bytes+(sy/8)*c.w+sx]&(1<<(sy&7)))expect=ink;
      }
      assert(Gfx.getPixel(x,y+32)==expect);
    }
    ++checked;
   }
  fprintf(stderr,"PASS: %d sprite/frame/clip/mirror pixel comparisons\n",checked);
}
'''
 render.write_text(text)
 sources=[unit,render]+sorted((sk/'src').rglob('*.cpp'))+sorted(p for p in gfx.glob('*.cpp') if p.name!='CHGfx.cpp')+sorted(game.rglob('*.cpp'))+[host/'chgfx_host.cpp',root/'tests/test_game.cpp']
 sd=repo/'platform/board/arduino/CHGame/libraries/CHSd'
 sources+=sorted((sd/'src').glob('*.cpp'))+[sd/'host/sd_host.cpp']
 exe=chsim.compile_exe(sources,td/'test_game',includes=[host,sk,gfx,game,sd/'src',sd/'host']);subprocess.run([str(exe)],check=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0','UBSAN_OPTIONS':'halt_on_error=1'})
