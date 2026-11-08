#pragma once
#include <Arduino.h>
#include <CHGame.h>
#include <string.h>
#include "../../SpriteColors.h"
#include "VideoOptions.h"
#include "Rumble.h"
#include "ColorArt.h"
extern bool portDeferredPresent;
void portOptionsBackground();
void portSmallText(int x,int y,const char *text,uint8_t color);
#define WIDTH 128
#define HEIGHT 64
#define WHITE 1
#define BLACK 0
#define ARDUBOY_NO_USB
#ifndef pgm_read_ptr
#define pgm_read_ptr(p) (*(const void *const *)(p))
#endif
#ifndef memcpy_P
#define memcpy_P memcpy
#endif
#ifdef CHSIM
class __FlashStringHelper;
#endif
uint8_t spriteColor(const uint8_t *image, uint8_t frame, uint8_t fallback = COLOR_TEXT, uint8_t baseOverride = 0);
bool spriteIsMask(const uint8_t *image);
void portPixel(int x, int y, uint8_t color);
void portFill(int x, int y, int w, int h, uint8_t color);
void portAudioUpdate();
// A vertical 1-bit bitmap to the packed color view, with clipping done once.
void portBitmap(int x,int y,const uint8_t *data,int w,int h,uint8_t color,uint8_t mode,uint8_t mirror=0,const uint8_t *mask=nullptr);
class PortAudio {
public:
 static bool enabled();
 void begin();
 void on();
 void off();
 void saveOnOff();
};
class Arduboy2Base {
public:
 PortAudio audio;
 uint16_t frameCount = 0;
 uint8_t currentButtonState = 0, previousButtonState = 0;
 void boot();
 void systemButtons() {}
 void setFrameRate(uint8_t fps) { chgame.setFrameRate(fps); }
 bool nextFrame();
 void pollButtons() {
   previousButtonState = currentButtonState;
   chgame.pollButtons();
   currentButtonState = chgame.buttons();
 }
 bool everyXFrames(uint8_t n) const { return frameCount % n == 0; }
 void display(bool clear = false);
 void clearDay() { portFill(0, 0, WIDTH, HEIGHT, COLOR_DAY_BACKGROUND); }
 void drawPixel(int x,int y,uint8_t c=WHITE) { portPixel(x,y,c ? COLOR_TEXT : COLOR_INK); }
 void drawFastHLine(int x,int y,int w,uint8_t c=WHITE) { portFill(x,y,w,1,c ? COLOR_TEXT : COLOR_INK); }
 void drawFastVLine(int x,int y,int h,uint8_t c=WHITE) { portFill(x,y,1,h,c ? COLOR_TEXT : COLOR_INK); }
 void fillRect(int x,int y,int w,int h,uint8_t c=WHITE) { portFill(x,y,w,h,c ? COLOR_TEXT : COLOR_INK); }
 void drawRect(int x,int y,int w,int h,uint8_t c=WHITE) {
   drawFastHLine(x,y,w,c); drawFastHLine(x,y+h-1,w,c);
   drawFastVLine(x,y,h,c); drawFastVLine(x+w-1,y,h,c);
 }
 void writePageByte(int x,int page,uint8_t value);
 void invert(bool enabled);
 [[noreturn]] void exitToBootloader() { chgame.exitToMenu(); }
};
class SpritesB {
public:
 static void drawOverwrite(int x,int y,const uint8_t *b,uint8_t f);
 static void drawSelfMasked(int x,int y,const uint8_t *b,uint8_t f);
 static void drawErase(int x,int y,const uint8_t *b,uint8_t f);
 static void drawExternalMask(int x,int y,const uint8_t *b,const uint8_t *m,uint8_t f,uint8_t mf);
};
// The game's EEPROM calls are redirected to a checked, versioned flash record.
// Full state is copied only when saving/restoring; no writes in the frame loop.
class PortSave {
public:
 void begin();
 bool ok = true;
 template<class T> void get(int, T &value) {
   const uint8_t *p = savedData();
   if (p) memcpy(&value,p,sizeof(T)); else memset(&value,0,sizeof(T));
 }
 template<class T> void put(int, const T &value) { ok = storeData(&value,sizeof(T)); }
 const uint8_t *savedData();
 bool storeData(const void *data,uint16_t size);
};
extern PortSave EEPROM;
