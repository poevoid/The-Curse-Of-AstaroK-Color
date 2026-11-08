#include "Arduboy2Ext.h"
#include "Enums.h"

Arduboy2Ext::Arduboy2Ext() : Arduboy2Base() { }

uint8_t Arduboy2Ext::justPressedButtons() const {

  return (~previousButtonState & currentButtonState);

}

uint8_t Arduboy2Ext::pressedButtons() const {

  return currentButtonState;

}

void Arduboy2Ext::clearButtonState() {

  currentButtonState = previousButtonState = 0;

}


void Arduboy2Ext::resetFrameCount() {

  frameCount = 0;

}

uint16_t Arduboy2Ext::getFrameCount() const {

  return frameCount;

}

uint8_t Arduboy2Ext::getFrameCount(uint8_t mod) const {

  return frameCount % mod;

}

bool Arduboy2Ext::getFrameCountHalf(uint8_t mod) const {

	return getFrameCount(mod) > (mod / 2);

}

bool Arduboy2Ext::isFrameCount(uint8_t mod) const {

  return (frameCount % mod) == 0;

}

bool Arduboy2Ext::isFrameCount(uint8_t mod, uint8_t val) const {

  return (frameCount % mod) == val;

}


void Arduboy2Ext::displayWithBackground(TimeOfDay timeOfDay) {

  if(portDeferredPresent)return;
  display(false);
  // Clear directly to the next view's background, rather than clearing black
  // and repainting the same 4 KB a second time for a daytime town frame.
  portFill(0,0,WIDTH,HEIGHT,timeOfDay==TimeOfDay::Day ? COLOR_DAY_BACKGROUND : COLOR_INK);

}


/* ----------------------------------------------------------------------------
 *  Draw a horizontal dotted line. 
 */
void Arduboy2Ext::drawHorizontalDottedLine(uint8_t x1, uint8_t x2, uint8_t y, uint8_t colour) {

	uint8_t diff = (x2 - x1);

	for (uint8_t x = 0; x <= diff; x += 2) {

		drawPixel(x1 + x, y, colour);

  }

}


/* ----------------------------------------------------------------------------
 *  Draw a vertical dotted line. 
 */
void Arduboy2Ext::drawVerticalDottedLine(uint8_t y1, uint8_t y2, uint8_t x, uint8_t colour) {

	uint8_t diff = (y2 - y1);

	for (uint8_t y = 0; y <= diff; y += 2) {

		drawPixel(x, y1 + y, colour);

  }
  
}


uint16_t rnd = 0xACE1;


uint8_t Arduboy2Ext::randomLFSR(uint8_t min, uint8_t max) {
  if (max <= min) return min; // final boss has a 0..0 SP reward range
  uint16_t r = rnd;
  r ^= static_cast<uint16_t>(micros()); // add some extra timing randomness
  (r & 1) ? r = (r >> 1) ^ 0xB400 : r >>= 1;
  rnd = r;
  return r % (max - min) + min;
}