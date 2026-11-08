#pragma once

namespace Images {

  extern const unsigned char PROGMEM Helmet_00[];

  extern const unsigned char PROGMEM Helmet_01[];

  extern const unsigned char PROGMEM Helmet_02[];

  extern const uint8_t * const Helmets[] PROGMEM; 

  extern const unsigned char PROGMEM Helmet_00_Mask[];

  extern const unsigned char PROGMEM Helmet_01_Mask[];

  extern const unsigned char PROGMEM Helmet_02_Mask[];

  extern const uint8_t * const Helmets_Mask[] PROGMEM; 

  extern const uint8_t PROGMEM Player_Idle_00[];

  extern const uint8_t PROGMEM Player_Idle_01[];

  extern const uint8_t PROGMEM Player_Idle_02[];
      
  extern const uint8_t PROGMEM Player_Walking_00[];

  extern const uint8_t PROGMEM Player_Walking_01[];

  extern const uint8_t PROGMEM Player_Walking_02[];

  extern const uint8_t PROGMEM Player_Walking_03[];

  extern const uint8_t * const Player_Images[] PROGMEM; 

  extern const uint8_t PROGMEM Player_Idle_00_Mask[];

  extern const uint8_t PROGMEM Player_Idle_01_Mask[];

  extern const uint8_t PROGMEM Player_Idle_02_Mask[];
  
  extern const uint8_t PROGMEM Player_Walking_00_Mask[];

  extern const unsigned char PROGMEM Player_Walking_01_Mask[];

  extern const unsigned char PROGMEM Player_Walking_02_Mask[];

  extern const unsigned char PROGMEM Player_Walking_03_Mask[];

  extern const uint8_t * const Player_Masks[] PROGMEM; 
  
  extern const uint8_t PROGMEM Jump_Effect[];

  extern const uint8_t PROGMEM RollsLeft[];

  #ifndef DEBUG
  extern const uint8_t PROGMEM NPC[];
  #else
  extern const unsigned char PROGMEM NPC[];
  #endif

  #ifndef DEBUG
  extern const unsigned char PROGMEM NPC_EOG[];
  #else
  extern const unsigned char PROGMEM NPC_EOG[];
  #endif

  extern const unsigned char PROGMEM Recast[];

  extern const unsigned char PROGMEM Recast_Mask[];

};
