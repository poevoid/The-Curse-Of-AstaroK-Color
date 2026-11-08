#pragma once

struct Tree {
	int16_t offset;
	TreeType tree;
};

namespace Data {

  constexpr Tree Trees[] PROGMEM {
    { -175, TreeType::Tree_01 },
    { -135, TreeType::Tree_02 },
    { -40, TreeType::Tree_01 },
    { 94, TreeType::Tree_01 },
    { 210, TreeType::Tree_01 },
  };

}

namespace Images {

  #ifdef COMPRESSED_TREES
    #ifdef TWO_TREE_TYPES
      #ifndef DEBUG
      extern const uint8_t PROGMEM Tree_01[];
      #else
      extern const uint8_t PROGMEM Tree_01[];
      #endif 

      #ifndef DEBUG
      extern const uint8_t PROGMEM Tree_02[];
      #else
      extern const uint8_t PROGMEM Tree_02[];
      #endif 
      
      extern const uint8_t * const Trees[] PROGMEM;
    #else
      extern const uint8_t PROGMEM Tree[];
    #endif
    
  #else
  
  extern const uint8_t PROGMEM Trees[];

  #endif

}