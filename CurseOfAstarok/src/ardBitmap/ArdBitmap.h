#pragma once
// Original compatibility names. COLOR checks the independent SD art pack;
// MONO always uses the original native bitmaps and masks in flash.
#include "../platform/ArduboyCompat.h"
#define MIRROR_NONE 0
#define MIRROR_HORIZONTAL 1
#define MIRROR_VERTICAL 2
#define MIRROR_HOR_VER 3
template<int16_t W,int16_t H> class ArdBitmap {
public:
 void drawCompressed(int16_t x,int16_t y,const uint8_t *b,uint8_t c,uint8_t mirror) {
   if(x+b[0]<=0 || x>=W || y+b[1]<=0 || y>=H) return;
   if(colorArtDraw(b,0,x,y,mirror))return;
   uint8_t color=spriteIsMask(b)?COLOR_INK:(videoOptions.monochrome ? (c?COLOR_TEXT:COLOR_INK) : spriteColor(b,0,c?COLOR_TEXT:COLOR_INK));
   portBitmap(x,y,b+2,b[0],b[1],color,1,mirror);
 }
 void drawCompressed_WithMask(int16_t x,int16_t y,const uint8_t *b,const uint8_t *m,uint8_t mirror) {
   if(x+b[0]<=0 || x>=W || y+b[1]<=0 || y>=H) return;
   if(colorArtDraw(b,0,x,y,mirror))return;
   // Dimensions are retained independently, as in the original two-pass draw.
   portBitmap(x,y,m+2,m[0],m[1],COLOR_INK,1,mirror);
   portBitmap(x,y,b+2,b[0],b[1],spriteColor(b,0),1,mirror);
 }
};
