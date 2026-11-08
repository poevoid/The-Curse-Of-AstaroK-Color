#pragma once
#include <stdint.h>
struct VideoOptions {
 uint8_t monochrome;  // 0 color, 1 original two-tone drawing
 uint8_t monoColor;   // palette index 1..15; black is reserved for ink
 uint8_t playerColor; // 0 uses SpriteColors.h, 1..15 overrides the body
 uint8_t rumble;      // 0 disabled, 1 GPIO1 driver enabled
};
extern VideoOptions videoOptions;
void portApplyVideoPalette();
void portVideoInvert(bool inverted);
bool portVideoInverted();
bool portSaveVideoOptions();
bool portOptionsTick(uint8_t justPressed);
bool portOptionsOpen();
