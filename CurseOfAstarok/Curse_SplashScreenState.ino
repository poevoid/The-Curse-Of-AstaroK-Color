#include "src/utils/Arduboy2Ext.h"

static constexpr uint8_t ppotFrames = (1.6 *40); // 1.6s animation loop


// ----------------------------------------------------------------------------
//  Initialise state ..
//
void splashScreen_Activate() {

  splashScreenVars.counter = 0;

}


// ----------------------------------------------------------------------------
//  Handle state updates .. 
//
void splashScreen_Update() { 

  auto justPressed = arduboy.justPressedButtons();

  if (justPressed > 0) {
      
      gameState = GameStateType::TitleScreen_Activate;

  }

}


// ----------------------------------------------------------------------------
//  Render the state .. 
//
void splashScreen_Render() {


    SpritesB::drawOverwrite(32, 16, Images::PPOT, 0);

    uint8_t p = 0x11; // Default pixel position 0 is hidden in the image

    switch (arduboy.getFrameCount(ppotFrames)) {

        // 2nd quarter of frames
        case (ppotFrames*1/4) ... (ppotFrames*2/4)-1:
            p = 0x31; // Move pixel down to position 1
            /*-fallthrough*/

        // 1st quarter of frames
        case 0 ... (ppotFrames*1/4)-1:
            // Flash 'Play' arrow by clearing the image
            // Overwrite directly to the screen buffer 
            arduboy.writePageByte(91, 3, 0x00);
            arduboy.writePageByte(92, 3, 0x00);
            arduboy.writePageByte(93, 3, 0x00);
            arduboy.writePageByte(94, 3, 0x00);
            arduboy.writePageByte(95, 3, 0x00);
            break;

        // 3rd quarter of frames
        case (ppotFrames*2/4) ... (ppotFrames*3/4)-1:
            p = 0x51; // Move pixel down to position 2
            break;

        // 4th quarter of frames
        default:
            p = 0x91; // Move pixel down to position 3
            break;

    }

    // Draw pixel to represent the tape spooling
    // Render directly to the screen buffer 
    arduboy.writePageByte(52, 3, p); // Values 0x11, 0x31, 0x51, 0x91

    // On even steps of pixel movement, update the spindle image
    if ((p & 0xA0) == 0) {
        // Render directly to the screen buffer 
        arduboy.writePageByte(45, 3, 0xA5);
        arduboy.writePageByte(46, 3, 0x95);
    }

    arduboy.display(true);

}