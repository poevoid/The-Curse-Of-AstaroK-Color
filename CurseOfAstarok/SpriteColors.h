#pragma once
#include <stdint.h>
// Named palette slots and inherited sprite/frame BASE assignments (0..15).
// Recompile after changing these assignments. The color-only editor paints
// explicit slots, BASE (G) and ALPHA (.) into independent SD artwork.
// The RGB values here are the fallback palette and MONO tint palette.
// When ASTAROK.ART is loaded, COLOR uses art/ColorPalette.json instead.
#define COLOR_INK         0  // RGB565 0x0000
#define COLOR_LIGHT_BROWN 1  // RGB565 0xCC8C
#define COLOR_RED         2  // RGB565 0xF986
#define COLOR_ORANGE      3  // RGB565 0xFBA0
#define COLOR_GOLD        4  // RGB565 0xFE49
#define COLOR_GREEN       5  // RGB565 0x5549
#define COLOR_MINT        6  // RGB565 0x8F55
#define COLOR_CYAN        7  // RGB565 0x4EFF
#define COLOR_SKY         8  // RGB565 0x8499
#define COLOR_BLUE        9  // RGB565 0x3A5F
#define COLOR_PURPLE     10  // RGB565 0x9A9D
#define COLOR_PINK       11  // RGB565 0xFBD7
#define COLOR_BROWN      12  // RGB565 0x9309
#define COLOR_SLATE      13  // RGB565 0x6B8F
#define COLOR_SILVER     14  // RGB565 0xBDF7
#define COLOR_WHITE      15  // RGB565 0xFFFF
#define COLOR_IVORY COLOR_WHITE  // Compatibility alias; shares the white slot

#define ASTAROK_RGB(r,g,b) uint16_t((((r)&248)<<8) | (((g)&252)<<3) | ((b)>>3))

static const uint16_t ASTAROK_PALETTE[16] = {
  ASTAROK_RGB(  0,   0,   0), // INK
  ASTAROK_RGB(206, 146,  99), // LIGHT_BROWN
  ASTAROK_RGB(255,  48,  49), // RED
  ASTAROK_RGB(255, 117,   0), // ORANGE
  ASTAROK_RGB(255, 203,  74), // GOLD
  ASTAROK_RGB( 82, 170,  74), // GREEN
  ASTAROK_RGB(140, 235, 173), // MINT
  ASTAROK_RGB( 74, 223, 255), // CYAN
  ASTAROK_RGB(132, 146, 206), // SKY
  ASTAROK_RGB( 57,  73, 255), // BLUE
  ASTAROK_RGB(156,  81, 239), // PURPLE
  ASTAROK_RGB(255, 121, 189), // PINK
  ASTAROK_RGB(148,  97,  74), // BROWN
  ASTAROK_RGB(107, 113, 123), // SLATE
  ASTAROK_RGB(189, 190, 189), // SILVER
  ASTAROK_RGB(255, 255, 255), // WHITE
};

#define COLOR_TEXT COLOR_IVORY
#define COLOR_DAY_BACKGROUND COLOR_SKY

// Animation families: a single edit recolors all of their frames.
#define SPRITE_PLAYER             COLOR_IVORY
#define SPRITE_SKELETON           COLOR_SILVER
#define SPRITE_SMALL_WEREWOLF     COLOR_BROWN
#define SPRITE_LARGE_WEREWOLF     COLOR_ORANGE
#define SPRITE_ASTAROK            COLOR_PURPLE
#define SPRITE_CUBE               COLOR_MINT
#define SPRITE_HELMET             COLOR_GOLD
#define SPRITE_TREE               COLOR_GREEN
#define SPRITE_BUILDING           COLOR_SILVER
#define SPRITE_RUNE               COLOR_GOLD
#define SPRITE_SHOPKEEPER         COLOR_PINK
#define SPRITE_NPC                COLOR_BROWN

// Individual artwork. Override any entry independently, using COLOR_* above.
#define ART_SKY                                 COLOR_SKY
#define ART_CLOUD_01                            COLOR_WHITE
#define ART_CLOUD_02                            COLOR_WHITE
#define ART_CRATES                              COLOR_BROWN
#define ART_GRASS                               COLOR_GREEN
#define ART_TOWN_GOUND                          COLOR_GREEN
#define ART_ARROW_UP_DIALOGUE                   COLOR_TEXT
#define ART_ARROW_DOWN_DIALOGUE                 COLOR_TEXT
#define ART_ARROW_UP                            COLOR_TEXT
#define ART_ARROW_UP_SMALL                      COLOR_TEXT
#define ART_ARROW_DOWN                          COLOR_TEXT
#define ART_ARROW_DOWN_SMALL                    COLOR_TEXT
#define ART_ARROW_LEFT                          COLOR_TEXT
#define ART_ARROW_RIGHT                         COLOR_TEXT
#define ART_HAND_POINTER                        COLOR_TEXT
#define ART_TOWN_SIGN                           COLOR_TEXT
#define ART_TOMBSTONE                           COLOR_TEXT
#define ART_TOMBSTONE2                          COLOR_TEXT
#define ART_PUMPKIN                             COLOR_ORANGE
#define ART_SAVING_POST                         COLOR_BROWN
#define ART_HP                                  COLOR_RED
#define ART_MR_BLINKY                           COLOR_TEXT
#define ART_TREE                                SPRITE_TREE
#define ART_TITLE_SCREEN                        COLOR_TEXT
#define ART_TITLE_SCREEN_PRESS_A                COLOR_TEXT
#define ART_ENEMY_INFO_PANEL                    COLOR_TEXT
#define ART_GELATINOUS_CUBE_00                  SPRITE_CUBE
#define ART_GELATINOUS_CUBE_01                  SPRITE_CUBE
#define ART_GELATINOUS_CUBE_02                  SPRITE_CUBE
#define ART_GELATINOUS_CUBE_03                  SPRITE_CUBE
#define ART_SMALL_WEREWOLF_00                   SPRITE_SMALL_WEREWOLF
#define ART_SMALL_WEREWOLF_01                   SPRITE_SMALL_WEREWOLF
#define ART_SMALL_WEREWOLF_02                   SPRITE_SMALL_WEREWOLF
#define ART_LARGE_WEREWOLF_HEAD                 SPRITE_LARGE_WEREWOLF
#define ART_LARGE_WEREWOLF_TORSO                SPRITE_LARGE_WEREWOLF
#define ART_LARGE_WEREWOLF_BOTTOM               SPRITE_LARGE_WEREWOLF
#define ART_AZTARO_K_LEFT                       SPRITE_ASTAROK
#define ART_AZTARO_K_MIDDLE                     SPRITE_ASTAROK
#define ART_AZTARO_K_RIGHT                      SPRITE_ASTAROK
#define ART_AZTARO_K_DEAD                       SPRITE_ASTAROK
#define ART_SKELETON_00                         SPRITE_SKELETON
#define ART_SKELETON_01                         SPRITE_SKELETON
#define ART_SKELETON_02                         SPRITE_SKELETON
#define ART_DMGFIRE                             COLOR_ORANGE
#define ART_DMGMIST                             COLOR_PURPLE
#define ART_HP_GAUGE                            COLOR_RED
#define ART_GAUGE                               COLOR_TEXT
#define ART_SP_GAUGE                            COLOR_PURPLE
#define ART_GP_GAUGE                            COLOR_GOLD
#define ART_NUMBERS_4X4                         COLOR_TEXT
#define ART_INVENTORY_TL                        COLOR_TEXT
#define ART_INVENTORY_TR                        COLOR_TEXT
#define ART_INVENTORY_BL                        COLOR_TEXT
#define ART_INVENTORY_BR                        COLOR_TEXT
#define ART_CASTLE_LH_TOP                       COLOR_SLATE
#define ART_CASTLE_RH_TOP                       COLOR_SLATE
#define ART_CASTLE_LH_MID                       COLOR_SLATE
#define ART_CASTLE_LH_BOTTOM                    COLOR_SLATE
#define ART_CASTLE_RH_BOTTOM                    COLOR_SLATE
#define ART_CASTLE_FLOOR                        COLOR_SLATE
#define ART_CASTLE_STAIRS                       COLOR_SLATE
#define ART_CASTLE_DARK_BRICK                   COLOR_SLATE
#define ART_CHAIN                               COLOR_SLATE
#define ART_CHEST                               COLOR_GOLD
#define ART_SKULL                               COLOR_TEXT
#define ART_RUNE_FRAME_00                       SPRITE_RUNE
#define ART_RUNE_FRAME_01                       SPRITE_RUNE
#define ART_RUNE_FRAME_02                       SPRITE_RUNE
#define ART_RUNE_FRAME_03                       SPRITE_RUNE
#define ART_RUNE_FRAME_04                       SPRITE_RUNE
#define ART_RUNE_FRAME_05                       SPRITE_RUNE
#define ART_RUNE_FRAME_06                       SPRITE_RUNE
#define ART_RUNE_FRAME_07                       SPRITE_RUNE
#define ART_RUNE_FRAME_08                       SPRITE_RUNE
#define ART_RUNE_FRAME_09                       SPRITE_RUNE
#define ART_RUNE_FRAME_10                       SPRITE_RUNE
#define ART_RUNE_FRAME_11                       SPRITE_RUNE
#define ART_RUNE_FRAME_12                       SPRITE_RUNE
#define ART_RUNE_FRAME_13                       SPRITE_RUNE
#define ART_RUNE_FRAME_14                       SPRITE_RUNE
#define ART_RUNE_FRAME_15                       SPRITE_RUNE
#define ART_RUNE_FRAME_16                       SPRITE_RUNE
#define ART_COINS                               COLOR_GOLD
#define ART_SPGRAPHIC                           COLOR_PURPLE
#define ART_PPOT                                COLOR_TEXT
#define ART_SOUNDS                              COLOR_TEXT
#define ART_HELMET_00                           SPRITE_HELMET
#define ART_HELMET_01                           SPRITE_HELMET
#define ART_HELMET_02                           SPRITE_HELMET
#define ART_PLAYER_IDLE_00                      SPRITE_PLAYER
#define ART_PLAYER_IDLE_01                      SPRITE_PLAYER
#define ART_PLAYER_IDLE_02                      SPRITE_PLAYER
#define ART_PLAYER_WALKING_00                   SPRITE_PLAYER
#define ART_PLAYER_WALKING_01                   SPRITE_PLAYER
#define ART_PLAYER_WALKING_02                   SPRITE_PLAYER
#define ART_PLAYER_WALKING_03                   SPRITE_PLAYER
#define ART_JUMP_EFFECT                         COLOR_TEXT
#define ART_ROLLS_LEFT                          COLOR_TEXT
#define ART_NPC                                 SPRITE_NPC
#define ART_NPC_EOG                             SPRITE_NPC
#define ART_RECAST                              SPRITE_PLAYER
#define ART_ITEMS_BUILDING_TOP                  SPRITE_BUILDING
#define ART_ITEMS_BUILDING_BOTTOM               SPRITE_BUILDING
#define ART_RUNES_BUILDING_TOP                  SPRITE_BUILDING
#define ART_RUNES_BUILDING_BOTTOM               SPRITE_BUILDING
#define ART_RUNES_BUILDING_SIDE                 SPRITE_BUILDING
#define ART_SHOP_KEEPER_ITEM                    SPRITE_SHOPKEEPER
#define ART_SHOP_KEEPER_COMMON                  SPRITE_SHOPKEEPER
#define ART_SHOP_KEEPER_RUNE                    SPRITE_SHOPKEEPER
#define ART_SHOP_KEEPER_PANEL                   SPRITE_SHOPKEEPER
#define ART_ITEM_SHOP_EFFECT_00                 COLOR_TEXT
#define ART_ITEM_SHOP_EFFECT_01                 COLOR_TEXT
#define ART_ITEM_SHOP_EFFECT_02                 COLOR_TEXT
#define ART_ITEM_SHOP_EFFECT_03                 COLOR_TEXT
#define ART_ITEM_SHOP_EFFECT_04                 COLOR_TEXT
#define ART_ITEM_SHOP_EFFECT_05                 COLOR_TEXT
#define ART_ITEM_SHOP_NAMES_00                  COLOR_TEXT
#define ART_ITEM_SHOP_NAMES_01                  COLOR_TEXT
#define ART_ITEM_SHOP_NAMES_02                  COLOR_TEXT
#define ART_ITEM_SHOP_NAMES_03                  COLOR_TEXT
#define ART_ITEM_SHOP_NAMES_04                  COLOR_TEXT
#define ART_ITEM_SHOP_NAMES_05                  COLOR_TEXT
#define ART_ITEM_SHOP_HELMET_BASE               COLOR_TEXT
#define ART_ITEM_SHOP_POTION_BASE               COLOR_TEXT
#define ART_ITEM_SHOP_HELMET_00                 SPRITE_HELMET
#define ART_ITEM_SHOP_HELMET_01                 SPRITE_HELMET
#define ART_ITEM_SHOP_HELMET_02                 SPRITE_HELMET
#define ART_RUNE_SHOP_EFFECT_00                 COLOR_TEXT
#define ART_RUNE_SHOP_EFFECT_01                 COLOR_TEXT
#define ART_RUNE_SHOP_EFFECT_02                 COLOR_TEXT
#define ART_RUNE_SHOP_EFFECT_03                 COLOR_TEXT
#define ART_RUNE_SHOP_NAMES_00                  COLOR_TEXT
#define ART_RUNE_SHOP_NAMES_01                  COLOR_TEXT
#define ART_RUNE_SHOP_NAMES_02                  COLOR_TEXT
#define ART_RUNE_SHOP_NAMES_03                  COLOR_TEXT
#define ART_RUNE_SHOP_FRAME                     COLOR_TEXT
#define ART_CLOSED_DOOR                         COLOR_SLATE
#define ART_SHOP_NUMBERS                        COLOR_TEXT
#define ART_UOM_GP                              COLOR_GOLD
#define ART_UOM_SP                              COLOR_PURPLE
#define ART_INVENTORY_MARKER                    COLOR_TEXT
#define ART_INVENTORY_HELMET_MARKER             COLOR_TEXT
#define ART_LEAVE_SHOP                          COLOR_TEXT
#define ART_LEAVE_SHOP_IMAGE                    COLOR_TEXT
#define ART_BYE                                 COLOR_TEXT

// Individual frames in shared sheets (these override the sheet assignment).
#define RUNE_SYMBOL_0 COLOR_RED
#define RUNE_SYMBOL_1 COLOR_BLUE
#define RUNE_SYMBOL_2 COLOR_GOLD
#define RUNE_SYMBOL_3 COLOR_MINT
#define RUNE_SYMBOL_4 COLOR_PURPLE
#define RUNE_SYMBOL_5 COLOR_PINK
#define RUNE_SYMBOL_6 COLOR_IVORY
#define RUNE_SYMBOL_7 COLOR_IVORY
#define POTION_FIRE COLOR_ORANGE
#define POTION_HEALTH COLOR_RED
#define POTION_SPEED COLOR_CYAN
