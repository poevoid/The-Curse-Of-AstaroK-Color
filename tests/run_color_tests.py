"""Usage: python tests/run_color_tests.py /path/to/CHGame; requires g++."""
from pathlib import Path
import sys,os,subprocess,tempfile,json,copy
root=Path(__file__).resolve().parents[1];sk=root/'CurseOfAstarok';repo=Path(sys.argv[1]).resolve()
sys.path.insert(0,str(root/'tools'));import color_art as art
sys.path.insert(0,str(repo/'tools/chsim'));import chsim
host=repo/'tools/chsim/host';gfx=chsim.chgfx_dir();game=chsim.chgame_dir();sd=repo/'platform/board/arduino/CHGame/libraries/CHSd'
os.environ['CHSIM_CXX']='g++';os.environ['CHSIM_FLAGS']='-fpermissive -include chrono -fsanitize=address,undefined -fno-omit-frame-pointer'
with tempfile.TemporaryDirectory() as work:
 td=Path(work);blocks=art.load_art();meta=json.loads((root/'art/color_manifest.json').read_text())['sprites']
 # Force every frame to differ; dense patterns cross sector and 8-pixel run boundaries.
 for name,info in meta.items():
  blocks[name]=[[''.join(art.SYMBOLS[(x//3+y*7+f*5)%18] for x in range(info['width'])) for y in range(info['height'])] for f in range(info['frames'])]
 palette=art.load_palette();palette[1]=[31,197,91]
 data,count=art.build_pack(blocks,palette,{'NPC':5,'Player_Idle_00':1});assert count==147
 card=td/'ASTAROK.ART';card.write_bytes(data)
 unit=td/'game.cpp';ino=[sk/'CurseOfAstarok.ino']+sorted(p for p in sk.glob('*.ino') if p.name!='CurseOfAstarok.ino')
 unit.write_text('#include <Arduino.h>\n'+'\n'.join(p.read_text() for p in ino))
 test=td/'test.cpp'
 code=r'''#define main chsim_unused_main
#include "main.cpp"
#undef main
#include "src/utils/Arduboy2Ext.h"
#include "src/utils/Constants.h"
#include "src/images/Images.h"
#include "src/ardBitmap/ArdBitmap.h"
#include <cassert>
extern Arduboy2Ext arduboy;
struct Case {const uint8_t *image;int frames;const char *name;};
static const Case cases[]={'''
 code+=','.join('{Images::'+name+','+str(i['frames'])+',"'+name+'"}' for name,i in meta.items())+'};\n'
 code+=r'''
static uint8_t expectedBase(const uint8_t *image,int frame){return image==Images::NPC?COLOR_GREEN:(image==Images::Player_Idle_00?COLOR_LIGHT_BROWN:spriteColor(image,frame));}
static uint32_t rgb888(uint16_t c){uint32_t r=c>>11,g=(c>>5)&63,b=c&31;return (((r<<3)|(r>>2))<<16)|(((g<<2)|(g>>4))<<8)|((b<<3)|(b>>2));}
int main(int argc,char **argv){
 setup();Gfx.wait();
 if(argc>1){assert(!colorArtReady());assert(!colorArtDraw(Images::NPC,0,0,0));fprintf(stderr,"PASS: missing/malformed SD art falls back to firmware\n");return 0;}
 assert(colorArtReady());assert(colorArtRgb(1)==ASTAROK_RGB(31,197,91));
 const int positions[][2]={{0,0},{-5,-7},{123,61}};int checked=0;
 for(auto c:cases)for(int f=0;f<c.frames;++f)for(auto &p:positions)for(int mirror=0;mirror<4;++mirror){
  Gfx.clear(COLOR_PINK);int w=c.image[0],h=c.image[1];bool visible=p[0]+w>0 && p[0]<128 && p[1]+h>0 && p[1]<64;if(colorArtDraw(c.image,f,p[0],p[1],mirror)!=visible){fprintf(stderr,"FAIL case %s id %d w%d h%d f%d pos%d,%d mirror%d ready%d\n",c.name,colorArtId(c.image),w,h,f,p[0],p[1],mirror,colorArtReady());abort();}
  for(int y=0;y<64;++y)for(int x=0;x<128;++x){
   int sx=x-p[0],sy=y-p[1];uint8_t want=COLOR_PINK;
   if(sx>=0 && sx<w && sy>=0 && sy<h){if(mirror&1)sx=w-1-sx;if(mirror&2)sy=h-1-sy;
    int v=(sx/3+sy*7+f*5)%18;if(v==16)v=expectedBase(c.image,f);if(v!=17)want=v;
   }assert(Gfx.getPixel(x,y+32)==want);
  }++checked;
 }
 // These paths must bypass old masks when a color image supplies its own alpha.
 ArdBitmap<128,64> bitmap;
 for(int mode=0;mode<3;++mode){
  Gfx.clear(COLOR_PINK);
  if(mode==0)SpritesB::drawSelfMasked(0,0,Images::NPC,0);
  if(mode==1)bitmap.drawCompressed(0,0,Images::NPC,WHITE,0);
  if(mode==2)bitmap.drawCompressed_WithMask(0,0,Images::NPC,Images::Crates_Mask,0);
  for(int y=0;y<Images::NPC[1];++y)for(int x=0;x<Images::NPC[0];++x){int v=(x/3+y*7)%18;if(v==16)v=expectedBase(Images::NPC,0);if(v==17)v=COLOR_PINK;assert(Gfx.getPixel(x,y+32)==v);}
 }
 // BASE follows player choice, alpha is skipped, black remains opaque.
 videoOptions.playerColor=COLOR_CYAN;Gfx.clear(COLOR_PINK);assert(colorArtDraw(Images::Player_Idle_00,0,0,0));
 assert(spriteColor(Images::Player_Idle_00,0)==COLOR_CYAN);
 int basePixels=0;for(int y=0;y<Images::Player_Idle_00[1];++y)for(int x=0;x<Images::Player_Idle_00[0];++x)if((x/3+y*7)%18==16){assert(Gfx.getPixel(x,y+32)==COLOR_CYAN);++basePixels;}assert(basePixels>0);
 videoOptions.monochrome=true;Gfx.clear(COLOR_PINK);assert(!colorArtDraw(Images::NPC,0,0,0));
 SpritesB::drawSelfMasked(0,0,Images::NPC,0);
 for(int y=0;y<Images::NPC[1];++y)for(int x=0;x<Images::NPC[0];++x){bool on=Images::NPC[2+(y/8)*Images::NPC[0]+x]&(1<<(y&7));assert(Gfx.getPixel(x,y+32)==(on?spriteColor(Images::NPC,0):COLOR_PINK));}
 videoOptions.monoColor=COLOR_CYAN;portApplyVideoPalette();Gfx.clear(COLOR_WHITE);arduboy.display(false);assert(sim_panel[32*128]==rgb888(ASTAROK_PALETTE[COLOR_CYAN]));
 videoOptions.monochrome=false;portApplyVideoPalette();Gfx.clear(1);arduboy.display(false);assert(sim_panel[32*128]==rgb888(ASTAROK_RGB(31,197,91)));assert(Gfx.getPalette565(COLOR_WHITE)==ASTAROK_PALETTE[COLOR_CYAN]);
 fprintf(stderr,"PASS: %d multicolor/frame/clip/mirror comparisons; black vs ALPHA, BASE, three draw paths, frozen monochrome and independent palettes\n",checked);
}
'''
 # CHGfx does not expose its palette getter; panel RGB verifies palette application instead.
 code=code.replace('assert(Gfx.getPalette565(COLOR_WHITE)==ASTAROK_PALETTE[COLOR_CYAN]);','')
 test.write_text(code)
 sources=[unit,test]+sorted((sk/'src').rglob('*.cpp'))+sorted(p for p in gfx.glob('*.cpp') if p.name!='CHGfx.cpp')+sorted(game.rglob('*.cpp'))+[host/'chgfx_host.cpp']+sorted((sd/'src').glob('*.cpp'))+[sd/'host/sd_host.cpp']
 exe=chsim.compile_exe(sources,td/'color_test',includes=[host,sk,gfx,game,sd/'src',sd/'host'])
 env={**os.environ,'CHSD_CARD':str(card),'ASAN_OPTIONS':'detect_leaks=0','UBSAN_OPTIONS':'halt_on_error=1'}
 subprocess.run([str(exe)],env=env,check=True)
 card.write_bytes(b'invalid art pack')
 subprocess.run([str(exe),'fallback'],env=env,check=True)
 env.pop('CHSD_CARD');subprocess.run([str(exe),'fallback'],env=env,check=True)
