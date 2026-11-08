#include "ArduboyCompat.h"
#include "../entities/Entities.h"
#include "../utils/Structs.h"
PortSave EEPROM;
static bool soundOn = true;
static constexpr uint32_t SAVE_MAGIC = save::magic("CAST");
struct LegacySaveRecord { PlayGameStateVars game; uint8_t sound; };
struct SaveRecord { PlayGameStateVars game; uint8_t sound; VideoOptions video; };
static_assert(sizeof(SaveRecord)<=save::MAX_DATA,"Curse save must fit in flash record");
static const uint8_t *storedGame;
static SaveRecord record;
VideoOptions videoOptions={0,COLOR_WHITE,0,0};
static bool videoInverted, paletteDirty=true;
bool portDeferredPresent=false;
void portApplyVideoPalette() {
 paletteDirty=true;
 for(uint8_t i=0;i<16;++i) {
   uint16_t rgb;
   if(videoOptions.monochrome)rgb=((i!=0)!=videoInverted)?ASTAROK_PALETTE[videoOptions.monoColor]:0;
   else rgb=videoInverted ? uint16_t(~colorArtRgb(i)) : colorArtRgb(i);
   Gfx.setPaletteEntry(i,rgb);
 }
}
void portVideoInvert(bool on) { videoInverted=on;portApplyVideoPalette(); }
bool portVideoInverted() { return videoInverted; }
static bool writeRecord() {
 record.sound=soundOn;record.video=videoOptions;
 Gfx.wait();
 if(!save::store(SAVE_MAGIC,3,record))return false;
 storedGame=(const uint8_t *)save::read(SAVE_MAGIC,3,sizeof(record));
 return true;
}
bool portSaveVideoOptions() { return writeRecord(); }
static_assert(COLOR_INK == 0,"Palette slot zero is the erasing ink");
void portPixel(int x,int y,uint8_t c) {
 if ((unsigned)x >= WIDTH || (unsigned)y >= HEIGHT) return;
 // CHGfx packs the left pixel in the LOW nibble. Flushes are blocking, so
 // the completed previous frame is no longer reading this byte.
 uint8_t &p = gfx_fb[((y+32)*128+x)>>1];
 if(x&1) p=(p&15)|(c<<4); else p=(p&240)|(c&15);
}
void portFill(int x,int y,int w,int h,uint8_t c) {
 int x0 = max(x,0), y0 = max(y,0), x1 = min(x+w,WIDTH), y1 = min(y+h,HEIGHT);
 if (x1>x0 && y1>y0) Gfx.fillRect(x0,y0+32,x1-x0,y1-y0,c);
}
bool PortAudio::enabled() { return soundOn; }
static const audio::Effect effectTable[] = {{nullptr,0,0}};
void PortAudio::begin() { audio::begin(effectTable,0,soundOn); }
void PortAudio::on() { soundOn=true; audio::setOn(true); }
void PortAudio::off() { soundOn=false; audio::setOn(false); }
void PortAudio::saveOnOff() { EEPROM.ok=writeRecord(); }
void PortSave::begin() {
 memset(&record,0,sizeof(record));record.sound=1;record.video={0,COLOR_WHITE,0,0};
 const SaveRecord *current=(const SaveRecord *)save::read(SAVE_MAGIC,3,sizeof(record));
 storedGame=(const uint8_t *)current;
 if(current)record=*current;
 else if(const SaveRecord *previous=(const SaveRecord *)save::read(SAVE_MAGIC,2,sizeof(record))) {
   record=*previous;storedGame=(const uint8_t *)previous;
   // Slot 1 was ivory. Preserve saved choices when it becomes light brown.
   if(record.video.monoColor==1)record.video.monoColor=COLOR_WHITE;
   if(record.video.playerColor==1)record.video.playerColor=COLOR_WHITE;
 } else {
   // Older ports saved the same game payload plus sound, without video settings.
   const LegacySaveRecord *old=(const LegacySaveRecord *)save::read(SAVE_MAGIC,1,sizeof(LegacySaveRecord));
   if(old){record.game=old->game;record.sound=old->sound;storedGame=(const uint8_t *)old;}
 }
 videoOptions=record.video;
 videoOptions.monochrome=videoOptions.monochrome?1:0;
 if(videoOptions.monoColor<1 || videoOptions.monoColor>15)videoOptions.monoColor=COLOR_WHITE;
 if(videoOptions.playerColor>15)videoOptions.playerColor=0;
 videoOptions.rumble=videoOptions.rumble?1:0;
 soundOn=record.sound!=0;portApplyVideoPalette();portRumbleStop();
}
const uint8_t *PortSave::savedData() { return storedGame; }
bool PortSave::storeData(const void *data,uint16_t size) {
 if(size!=sizeof(record.game))return false;
 memcpy(&record.game,data,size);return writeRecord();
}
void Arduboy2Base::boot() {
 chgame.boot();
#ifdef CHSIM
 chgame.lockstep=-1;
#endif
 Gfx.begin();
 Gfx.setPalette(ASTAROK_PALETTE,16); Gfx.clear(COLOR_INK);
 colorArtBegin();EEPROM.begin();portRumbleBegin();
 currentButtonState=previousButtonState=chgame.buttons();
}
bool Arduboy2Base::nextFrame() {
 portAudioUpdate();portRumbleUpdate();
 if (!chgame.nextFrame()) return false;
 ++frameCount; return true;
}
void Arduboy2Base::display(bool clear) {
 if(portDeferredPresent)return;
 static bool first=true, errorShown=false;
 if(first) {
   Gfx.fillRect(0,0,128,32,COLOR_INK);
   Gfx.fillRect(0,96,128,32,COLOR_INK);
 }
 if(first || errorShown!=!EEPROM.ok || (!EEPROM.ok && paletteDirty)) {
   Gfx.fillRect(0,104,128,16,COLOR_INK);
   if(!EEPROM.ok)portSmallText(10,108,"SAVE FAILED",COLOR_RED);
   if(!first)Gfx.displayRect(0,104,128,16);
   errorShown=!EEPROM.ok;
 }
 if(first) { Gfx.display();first=false; }
 else Gfx.displayRect(0,32,128,64);
 paletteDirty=false;
 if(clear)portFill(0,0,WIDTH,HEIGHT,COLOR_INK);
}
void Arduboy2Base::writePageByte(int x,int page,uint8_t v) {
 for (int bit=0;bit<8;++bit) portPixel(x,page*8+bit,(v&(1<<bit))?COLOR_TEXT:COLOR_INK);
}
void Arduboy2Base::invert(bool on) { portVideoInvert(on); }
// Modes: 0 overwrite, 1 transparent ink, 3 external mask. Mode 2 (the
// original erase operation) is transparent ink in a sprite's assigned color.
#if defined(__GNUC__)
__attribute__((optimize("O2")))
#endif
void portBitmap(int x,int y,const uint8_t *data,int w,int h,uint8_t color,uint8_t mode,uint8_t mirror,const uint8_t *mask) {
 const int x0=max(x,0),x1=min(x+w,WIDTH),y0=max(y,0),y1=min(y+h,HEIGHT);
 if(x0>=x1 || y0>=y1) return;
 const uint8_t packed[4]={0,color,uint8_t(color<<4),uint8_t(color|(color<<4))};
 static const uint8_t cover[4]={0,15,240,255};
 const bool horizontal=mirror&1,vertical=mirror&2;
 const int step=horizontal?-1:1;
 for(int dy=y0;dy<y1;++dy) {
   const int sy=vertical ? h-1-(dy-y) : dy-y;
   const uint8_t shift=sy&7;
   int src=(sy>>3)*w+(horizontal ? w-1-(x0-x) : x0-x);
   uint8_t *dst=gfx_fb+(dy+32)*64+(x0>>1);
   int dx=x0;
   if(dx&1) {
     const unsigned v=(data[src]>>shift)&1;
     const bool paint=mode==0 || v || (mode==3 && ((mask[src]>>shift)&1));
     if(paint)*dst=(*dst&15)|(v ? packed[2] : 0);
     ++dst;src+=step;++dx;
   }
   for(;dx+1<x1;dx+=2,++dst,src+=2*step) {
     unsigned bits=((data[src]>>shift)&1)|(((data[src+step]>>shift)&1)<<1);
     if(mode==0) *dst=packed[bits];
     else {
       unsigned coverage=bits;
       if(mode==3)coverage|=((mask[src]>>shift)&1)|(((mask[src+step]>>shift)&1)<<1);
       *dst=(*dst&~cover[coverage])|packed[bits];
     }
   }
   if(dx<x1) {
     const unsigned v=(data[src]>>shift)&1;
     const bool paint=mode==0 || v || (mode==3 && ((mask[src]>>shift)&1));
     if(paint)*dst=(*dst&240)|(v ? color : 0);
   }
 }
}
static void draw(int x,int y,const uint8_t *b,const uint8_t *m,uint8_t f,uint8_t mf,uint8_t mode) {
 const int w=b[0],h=b[1];
 if(x+w<=0 || x>=WIDTH || y+h<=0 || y>=HEIGHT) return;
 if(colorArtDraw(b,f,x,y))return;
 const unsigned size=w*((h+7)/8);
 const uint8_t color=(videoOptions.monochrome && mode==2)?COLOR_INK:spriteColor(b,f,mode==2?COLOR_INK:COLOR_TEXT);
 portBitmap(x,y,b+2+unsigned(f)*size,w,h,color,mode,0,m?m+unsigned(mf)*size:nullptr);
}
void SpritesB::drawOverwrite(int x,int y,const uint8_t *b,uint8_t f) { draw(x,y,b,nullptr,f,0,0); }
void SpritesB::drawSelfMasked(int x,int y,const uint8_t *b,uint8_t f) { draw(x,y,b,nullptr,f,0,1); }
void SpritesB::drawErase(int x,int y,const uint8_t *b,uint8_t f) { draw(x,y,b,nullptr,f,0,2); }
void SpritesB::drawExternalMask(int x,int y,const uint8_t *b,const uint8_t *m,uint8_t f,uint8_t mf) { draw(x,y,b,m,f,mf,3); }
