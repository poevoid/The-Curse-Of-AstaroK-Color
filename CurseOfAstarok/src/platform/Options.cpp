#include "ArduboyCompat.h"
#include "../utils/Arduboy2Ext.h"
#include "../fonts/Font3x6.h"
#include "../images/Images.h"
void drawMessageBox(int8_t x,int8_t y,uint8_t width,uint8_t height);
extern Arduboy2Ext arduboy;
static bool opened, priorInvert;
static uint8_t row, background;
static VideoOptions before;
static const char *const names[]={"DEFAULT","L.BROWN","RED","ORANGE","GOLD","GREEN","MINT","CYAN","SKY","BLUE","PURPLE","PINK","BROWN","SLATE","SILVER","WHITE"};
bool portOptionsOpen() { return opened; }
static void drawOptions() {
 portFill(0,0,128,64,background);portOptionsBackground();
 // Same corner artwork and font as the inventory, centered in the game view.
 drawMessageBox(12,5,104,54);Font3x6 font;
 font.setCursor(51,8);font.print("OPTIONS");
 const char *const labels[]={"Display","Mono tint","Player","Rumble"};
 for(uint8_t i=0;i<4;++i){font.setCursor(24,21+i*9);font.print(labels[i]);}
 SpritesB::drawSelfMasked(18,25+row*9,Images::Inventory_Marker,0);
 font.setCursor(81,21);font.print(videoOptions.monochrome?"MONO":"COLOR");
 font.setCursor(81,30);font.print(names[videoOptions.monoColor]);
 font.setCursor(81,39);font.print(names[videoOptions.playerColor]);
 font.setCursor(81,48);font.print(videoOptions.rumble?"ON":"OFF");
 arduboy.display(false);
}

bool portOptionsTick(uint8_t pressed) {
 if(!opened) {
   if(!(pressed&START_BUTTON))return false;
   opened=true;row=0;before=videoOptions;portRumbleStop();
   background=Gfx.getPixel(0,32);priorInvert=portVideoInverted();portVideoInvert(false);
 } else if(pressed&(START_BUTTON|B_BUTTON)) {
   opened=false;
   if(memcmp(&before,&videoOptions,sizeof(before)))EEPROM.ok=portSaveVideoOptions();
   portVideoInvert(priorInvert);
   portFill(0,0,128,64,background);
   arduboy.previousButtonState=arduboy.currentButtonState;
   return true;
 } else {
   if(pressed&UP_BUTTON)row=row?row-1:3;
   else if(pressed&DOWN_BUTTON)row=(row+1)%4;
   int step=pressed&LEFT_BUTTON ? -1 : ((pressed&(RIGHT_BUTTON|A_BUTTON))?1:0);
   if(step) {
     if(row==0)videoOptions.monochrome=!videoOptions.monochrome;
     else if(row==1)videoOptions.monoColor=uint8_t((int(videoOptions.monoColor)-1+step+15)%15+1);
     else if(row==2)videoOptions.playerColor=uint8_t((int(videoOptions.playerColor)+step+16)%16);
     else {videoOptions.rumble=!videoOptions.rumble;portRumbleStop();}
     portApplyVideoPalette();
   }
 }
 drawOptions();return true;
}
