#pragma once
#include <stdint.h>
#define COLOR_ART_SCHEMA 3767001318u
#define COLOR_ART_COUNT 147
void colorArtBegin();
bool colorArtReady();
uint16_t colorArtRgb(uint8_t index);
uint8_t colorArtId(const uint8_t *image);
bool colorArtDraw(const uint8_t *image,uint8_t frame,int x,int y,uint8_t mirror=0);
