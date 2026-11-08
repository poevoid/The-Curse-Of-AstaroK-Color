#pragma once

namespace Images {

  extern const uint8_t PROGMEM Sky[];

  #ifndef DEBUG
  extern const uint8_t PROGMEM Cloud_01[];
  #else
  extern const uint8_t PROGMEM Cloud_01[];
  #endif 

  #ifndef DEBUG
  extern const uint8_t PROGMEM Cloud_02[];
  #else
  extern const uint8_t PROGMEM Cloud_02[];
  #endif 

  #ifndef DEBUG
  extern const uint8_t PROGMEM Crates[];
  #else
  extern const uint8_t PROGMEM Crates[];
  #endif 

  #ifndef DEBUG
  extern const uint8_t PROGMEM Crates_Mask[];
  #else
  extern const uint8_t PROGMEM Crates_Mask[];
  #endif 

  extern const uint8_t PROGMEM Grass[];

  extern const uint8_t PROGMEM Grass_mask[];

  extern const uint8_t PROGMEM Town_Gound[];

  extern const uint8_t PROGMEM Arrow_Up_Dialogue[];

  extern const uint8_t PROGMEM Arrow_Down_Dialogue[];

  extern const uint8_t PROGMEM Arrow_Up[];

  extern const uint8_t PROGMEM Arrow_Up_Small[];

  extern const uint8_t PROGMEM Arrow_Down[];

  extern const uint8_t PROGMEM Arrow_Down_Small[];

  extern const uint8_t PROGMEM Arrow_Left[];

  extern const uint8_t PROGMEM Arrow_Right[];

  extern const uint8_t PROGMEM HandPointer[];

  #ifndef DEBUG
  extern const uint8_t PROGMEM Town_Sign[];
  #else
  extern const uint8_t PROGMEM Town_Sign[];
  #endif 

  extern const uint8_t PROGMEM Tombstone[];

  extern const uint8_t PROGMEM Tombstone2[];

  extern const uint8_t PROGMEM Pumpkin[];

  extern const unsigned char PROGMEM Pumpkin_Mask[];

  extern const uint8_t PROGMEM SavingPost[];

  extern const unsigned char PROGMEM HP[];

  extern const unsigned char PROGMEM MrBlinky[];

  extern const unsigned char PROGMEM MrBlinky_Mask[];

}