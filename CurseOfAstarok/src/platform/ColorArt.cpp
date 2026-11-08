#include "ArduboyCompat.h"
#include "ColorArt.h"
#include <Fat.h>
#include <SdSpi.h>
// Sector cache only: big/many color sprites do not consume firmware flash.
static fat::Run runs[16];
static uint8_t nRuns, directory[COLOR_ART_COUNT*3];
static uint16_t palette[16];
static uint8_t cache[7][512] __attribute__((aligned(4)));
static uint32_t tag[7], age[7], clockTag, fileBytes;
static bool ready, paletteReady;
static uint32_t unpack24(const uint8_t *p){return p[0]|uint32_t(p[1])<<8|uint32_t(p[2])<<16;}
static uint32_t unpack32(const uint8_t *p){return unpack24(p)|uint32_t(p[3])<<24;}
bool colorArtReady(){return ready;}
uint16_t colorArtRgb(uint8_t i){return paletteReady?palette[i&15]:ASTAROK_PALETTE[i&15];}
void colorArtBegin(){
 ready=paletteReady=false;Gfx.wait();
 uint8_t *scratch=gfx_chunkScratch();fat::File file;
 if(!sd::init() || fat::mount(scratch)!=fat::OK || fat::find("ASTAROK ART",file,scratch)!=fat::OK)return;
 int8_t n=fat::runs(file,runs,16,scratch);if(n<=0)return;nRuns=n;
 if(!fat::read(runs,nRuns,0,scratch))return;
 const uint8_t *p=gfx_chunkScratch();
 if(memcmp(p,"ACA2",4) || unpack32(p+4)!=COLOR_ART_SCHEMA || p[12]!=COLOR_ART_COUNT || p[13])return;
 fileBytes=unpack32(p+8);if(fileBytes<512 || fileBytes>=0x1000000 || fileBytes!=file.size)return;
 memcpy(palette,p+16,32);palette[0]=0;memcpy(directory,p+48,sizeof(directory));
 for(uint8_t i=0;i<7;++i){tag[i]=0xffffffffu;age[i]=0;}
 clockTag=0;ready=paletteReady=true;
}
static const uint8_t *block(uint32_t k){
 uint8_t oldest=0;
 for(uint8_t i=0;i<7;++i){
  if(tag[i]==k){age[i]=++clockTag;return cache[i];}
  if(age[i]<age[oldest])oldest=i;
 }
 Gfx.wait();if(!fat::read(runs,nRuns,k,cache[oldest])){ready=false;return nullptr;}
 tag[oldest]=k;age[oldest]=++clockTag;return cache[oldest];
}
struct Reader{
 uint32_t pos, page=0xffffffffu;const uint8_t *data=nullptr;
 int next(){
  if(pos>=fileBytes)return -1;
  if((pos>>9)!=page){page=pos>>9;data=block(page);if(!data)return -1;}
  return data[(pos++)&511];
 }
};
static uint32_t read24(uint32_t at){
 Reader r{at};int a=r.next(),b=r.next(),c=r.next();
 if(a<0 || b<0 || c<0){ready=false;return 0;}
 return uint32_t(a)|(uint32_t(b)<<8)|(uint32_t(c)<<16);
}
static void span(int x,int y,int n,uint8_t c){
 if(unsigned(y)>=64)return;
 int end=min(x+n,128);x=max(x,0);if(x>=end)return;
 uint8_t *p=gfx_fb+(y+32)*64+(x>>1);
 if(x&1){*p=(*p&15)|(c<<4);++p;++x;}
 const uint8_t both=c|(c<<4);
 for(;x+1<end;x+=2)*p++=both;
 if(x<end)*p=(*p&240)|c;
}
bool colorArtDraw(const uint8_t *image,uint8_t frame,int x,int y,uint8_t mirror){
 if(videoOptions.monochrome || !ready)return false;
 const int w=image[0],h=image[1];if(x+w<=0 || x>=128 || y+h<=0 || y>=64)return false;
 const uint8_t id=colorArtId(image);if(id>=COLOR_ART_COUNT)return false;
 uint32_t at=unpack24(directory+id*3);if(!at)return false;
 Reader head{at};int dw=head.next(),dh=head.next(),frames=head.next(),baseOverride=head.next();
 if(dw!=w || dh!=h || frames<=frame || frames>255 || baseOverride<0 || baseOverride>16){ready=false;return false;}
 at=read24(at+4+frame*3);if(!at)return false;
 Reader r{at};unsigned out=0;const unsigned count=w*h;const uint8_t base=spriteColor(image,frame,COLOR_TEXT,baseOverride);
 while(out<count){
  int b=r.next();if(b<0 || (b&31)>17){ready=false;return false;}
  unsigned n=(b>>5)+1;if(out+n>count){ready=false;return false;}
  uint8_t c=b&31;if(c==16)c=base;
  while(n){
   unsigned row=out/w,col=out%w,take=min(n,unsigned(w)-col);
   if(c!=17)span(x+((mirror&1)?w-int(col)-int(take):int(col)),y+((mirror&2)?h-1-int(row):int(row)),take,c);
   out+=take;n-=take;
  }
 }
 return true;
}
