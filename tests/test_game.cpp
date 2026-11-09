// Built against the official CHGame host simulator by run_game_tests.py.
#include <cassert>
#define main chsim_unused_main
#include "main.cpp"
#undef main
#include "src/utils/Arduboy2Ext.h"
#include "src/entities/Entities.h"
#include "src/utils/Structs.h"
#include "GameDeclarations.h"
#include "src/images/Images.h"
#include "src/ardBitmap/ArdBitmap.h"
extern void testAllSpritePixels();
extern Arduboy2Ext arduboy;
extern PlayGameStateVars playGameVars;
extern GameStateType gameState;
static void press(uint8_t b) {
 arduboy.previousButtonState=0; arduboy.currentButtonState=b;
 playGame_Update(); arduboy.previousButtonState=b; arduboy.currentButtonState=0;
}
static void render(int frames=70) {
 for(int n=0;n<frames;++n) { ++arduboy.frameCount; playGame_Render(); }
}
static void capture(const char *name) {
 const char *dir=getenv("ASTAROK_SCREENSHOTS");if(!dir)return;
 char path[1024];snprintf(path,sizeof(path),"%s/%s.ppm",dir,name);
 FILE *f=fopen(path,"wb");assert(f);fprintf(f,"P6\n128 128\n255\n");
 for(int i=0;i<128*128;++i) {uint8_t rgb[]={uint8_t(sim_panel[i]>>16),uint8_t(sim_panel[i]>>8),uint8_t(sim_panel[i])};fwrite(rgb,1,3,f);}fclose(f);
}
// Exercise the original world-coordinate path, not only direct sprite draws.
// These objects have negative positions in the village and must remain signed
// through C++ integer promotion on the 32-bit target.
static void testViewportTransfer() {
 Gfx.clear(COLOR_INK);arduboy.display(false); // one-time full panel initialization
 uint32_t start=micros();arduboy.display(false);uint32_t elapsed=micros()-start;
 assert(elapsed>5000 && elapsed<6000); // 16 KB at the simulator's SPI model rate
 EEPROM.ok=false;arduboy.display(false);int red=0;
 for(int y=104;y<120;++y)for(int x=0;x<128;++x)
   if(Gfx.getPixel(x,y)==COLOR_RED)++red;
 assert(red>0);
 EEPROM.ok=true;arduboy.display(false);
 for(int y=104;y<120;++y)for(int x=0;x<128;++x)assert(Gfx.getPixel(x,y)==COLOR_INK);
 fprintf(stderr,"PASS: 64-row frame transfer (%u us) and save-error border updates\n",unsigned(elapsed));
}
static void testTownCoordinates() {
 ArdBitmap<128,64> bitmap;
 struct TownCase {ItemType type;int x,offset,left,top,width,height;const uint8_t *image;};
 const TownCase cases[]={
   {ItemType::NPC,-80,120,35,34,13,16,Images::NPC},
   {ItemType::Tombstone,-190,255,61,34,11,16,Images::Tombstone},
   {ItemType::SavingPost,-110,120,10,34,16,16,Images::SavingPost},
   {ItemType::Crates,-256,255,-7,33,22,24,Images::Crates}
 };
 for(const auto &c:cases) {
   TownItem item(c.type,c.x,RenderPosition::FrontOfBuildings);
   assert(item.getX()==c.x);
   Gfx.clear(COLOR_DAY_BACKGROUND);arduboy.frameCount=1;
   item.render(arduboy,bitmap,c.offset,RenderPosition::FrontOfBuildings,false);
   int visible=0;uint8_t color=spriteColor(c.image,0);
   for(int y=c.top;y<c.top+c.height;++y)for(int x=max(c.left,0);x<min(c.left+c.width,128);++x)
     if(Gfx.getPixel(x,y+32)==color)++visible;
   assert(visible>0);
 }
 TownItem moved(ItemType::HP,0);moved.setX(-53);assert(moved.getX()==-53);
 fprintf(stderr,"PASS: signed negative NPC/tombstone/save-post/crate coordinates and moved pickup\n");
}
static void menuPress(uint8_t button) {
 chgame.injected=0;sim_advance(30000);loop();
 chgame.injected=button;sim_advance(30000);loop();
 chgame.injected=0;
}
static uint32_t rgb888(uint16_t c) {
 uint32_t r=c>>11,g=(c>>5)&63,b=c&31;
 return (((r<<3)|(r>>2))<<16)|(((g<<2)|(g>>4))<<8)|((b<<3)|(b>>2));
}
static void testOptionsAndRumble() {
 // Palette revision: an old ivory choice becomes white, while other settings
 // and progress survive. New slot-1 choices must remain light brown after save.
 struct PreviousSave {PlayGameStateVars game;uint8_t sound;VideoOptions video;};
 PreviousSave prior={playGameVars,1,{1,1,1,1}};prior.game.player.setHP(68);
 Gfx.wait();assert(save::store(save::magic("CAST"),2,prior));assert(save::store(save::magic("CAST"),2,prior));
 EEPROM.begin();PlayGameStateVars migrated;EEPROM.get(101,migrated);
 assert(videoOptions.monoColor==COLOR_WHITE && videoOptions.playerColor==COLOR_WHITE && videoOptions.rumble && migrated.player.getHP()==68);
 videoOptions.monoColor=COLOR_LIGHT_BROWN;videoOptions.playerColor=COLOR_LIGHT_BROWN;
 assert(portSaveVideoOptions());EEPROM.begin();
 assert(videoOptions.monoColor==COLOR_LIGHT_BROWN && videoOptions.playerColor==COLOR_LIGHT_BROWN);
 // Simulate a pre-options save in BOTH pages, then upgrade it without losing
 // the player's progress or sound preference.
 struct OldSave {PlayGameStateVars game;uint8_t sound;};
 OldSave old={playGameVars,0};old.game.hasBeenSaved='c';old.game.player.setHP(73);old.game.player.setGP(87);
 Gfx.wait();assert(save::store(save::magic("CAST"),1,old));assert(save::store(save::magic("CAST"),1,old));
 EEPROM.begin();PlayGameStateVars restored;EEPROM.get(101,restored);
 assert(restored.hasBeenSaved=='c' && restored.player.getHP()==73 && restored.player.getGP()==87 && !PortAudio::enabled());
 assert(!videoOptions.monochrome && videoOptions.monoColor==COLOR_WHITE && !videoOptions.playerColor && !videoOptions.rumble);
 memset(&playGameVars,0,sizeof(playGameVars));playGame_Activate();gameState=GameStateType::PlayGame;
 render();playGameVars.player.setHP(1); // unsaved game progress must stay unsaved
 chgame.injected=0;sim_advance(30000);loop();
 PlayGameStateVars frozen=playGameVars;uint16_t frame=arduboy.frameCount;
 chgame.injected=START_BUTTON;sim_advance(30000);loop();chgame.injected=0;
 assert(portOptionsOpen() && arduboy.frameCount==frame);capture("options-color");
 menuPress(RIGHT_BUTTON);assert(videoOptions.monochrome);
 menuPress(DOWN_BUTTON);menuPress(RIGHT_BUTTON);assert(videoOptions.monoColor==COLOR_LIGHT_BROWN); // wraps white -> light brown
 for(int i=0;i<6;++i)menuPress(RIGHT_BUTTON);
 assert(videoOptions.monoColor==COLOR_CYAN);
 menuPress(DOWN_BUTTON);menuPress(LEFT_BUTTON);assert(videoOptions.playerColor==COLOR_WHITE);
 menuPress(RIGHT_BUTTON);assert(videoOptions.playerColor==0);menuPress(RIGHT_BUTTON);menuPress(RIGHT_BUTTON);assert(videoOptions.playerColor==COLOR_RED);
 menuPress(DOWN_BUTTON);menuPress(RIGHT_BUTTON);assert(videoOptions.rumble);
 assert(arduboy.frameCount==frame && memcmp(&frozen,&playGameVars,sizeof(frozen))==0);capture("options-mono");
 menuPress(B_BUTTON);assert(!portOptionsOpen());
 // Reload as if booting again. The options persist, but the last manual game
 // save remains at 73 HP, rather than being replaced by the unsaved 1 HP game.
 videoOptions={0,COLOR_WHITE,0,0};EEPROM.begin();EEPROM.get(101,restored);
 assert(videoOptions.monochrome && videoOptions.monoColor==COLOR_CYAN && videoOptions.playerColor==COLOR_RED && videoOptions.rumble);
 assert(restored.hasBeenSaved=='c' && restored.player.getHP()==73 && restored.player.getGP()==87);
 assert(!PortAudio::enabled());
 render(2);capture("town-mono-cyan");
 uint32_t tint=rgb888(ASTAROK_PALETTE[COLOR_CYAN]);int lit=0,black=0;
 for(int y=32;y<96;++y)for(int x=0;x<128;++x){uint32_t c=sim_panel[y*128+x];assert(c==0 || c==tint);if(c)++lit;else ++black;}
 assert(lit>0 && black>0 && spriteColor(Images::Player_Idle_00,0)==COLOR_TEXT);
 EEPROM.ok=false;arduboy.display(false);videoOptions.monoColor=COLOR_GREEN;portApplyVideoPalette();arduboy.display(false);
 for(int y=104;y<120;++y)for(int x=0;x<128;++x)assert(sim_panel[y*128+x]==0 || sim_panel[y*128+x]==rgb888(ASTAROK_PALETTE[COLOR_GREEN]));
 EEPROM.ok=true;videoOptions.monoColor=COLOR_CYAN;portApplyVideoPalette();arduboy.display(false);

 // Dark village silhouettes must remain visible against the tinted day sky.
 ArdBitmap<128,64> bitmap;TownItem npc(ItemType::NPC,-80,RenderPosition::FrontOfBuildings);
 Gfx.clear(COLOR_DAY_BACKGROUND);npc.render(arduboy,bitmap,120,RenderPosition::FrontOfBuildings,false);
 int silhouette=0;for(int y=34;y<50;++y)for(int x=35;x<48;++x)if(Gfx.getPixel(x,y+32)==COLOR_INK)++silhouette;
 assert(silhouette>30);
 menuPress(START_BUTTON);menuPress(RIGHT_BUTTON);assert(!videoOptions.monochrome);capture("options-custom");menuPress(START_BUTTON);
 assert(spriteColor(Images::Player_Idle_00,0)==COLOR_RED && spriteColor(Images::Player_Walking_03,0)==COLOR_RED && spriteColor(Images::Recast,0)==COLOR_RED);
 assert(spriteColor(Images::Player_Idle_00_Mask,0)==COLOR_INK);
 assert(spriteColor(Images::Helmet_00,0)==ART_HELMET_00);render(2);capture("town-player-red");
 // Low and high actual HP loss produce increasingly strong/long pulses.
 Player p;p.setHP(100);p.setDef(0);p.setHelmetType(HelmetType::None);
 videoOptions.rumble=0;p.inflictDMG(10);assert(!portRumbleDuty());
 videoOptions.rumble=1;p.setHP(100);p.setDef(20);p.inflictDMG(10);assert(p.getHP()==100 && !portRumbleDuty());
 p.setDef(0);p.inflictDMG(5);assert(p.getHP()==95 && portRumbleDuty()==255);
 uint16_t smallTime=portRumbleDuration();delay(25);portRumbleUpdate();uint8_t weak=portRumbleDuty();assert(weak>0 && weak<255);
 p.setHP(100);p.inflictDMG(20);uint16_t bigTime=portRumbleDuration();delay(25);portRumbleUpdate();assert(portRumbleDuty()>weak && bigTime>smallTime);
 p.setHP(3);p.inflictDMG(250);assert(p.getHP()==0 && portRumbleDuration()==50+3*12);
 portRumbleStop();p.setHP(50);p.setDef(10);p.inflictDMG(14);assert(p.getHP()==46 && portRumbleDuration()==50+4*12);
 portRumbleStop();p.decHP(12);assert(p.getHP()==34 && portRumbleDuration()==50+12*12); // Astarok drain path
 delay(1000);portRumbleUpdate();assert(!portRumbleDuty());p.incHP(5);assert(!portRumbleDuty());
 p.decHP(5);videoOptions.rumble=0;portRumbleUpdate();assert(!portRumbleDuty());
 videoOptions.rumble=1;p.decHP(3);assert(portRumbleDuty());menuPress(START_BUTTON);assert(!portRumbleDuty());p.decHP(5);assert(!portRumbleDuty());menuPress(B_BUTTON);
 videoOptions={0,COLOR_WHITE,0,0};portVideoInvert(false);portRumbleStop();chgame.injected=0;
 fprintf(stderr,"PASS: four-row START menu, pause/input isolation, color/tint/player controls, legacy save migration, persistent options and proportional rumble (blocked/armor/lethal/drain/off/timeout/pause)\n");
}

int main() {
 setup(); testAllSpritePixels(); testTownCoordinates(); testViewportTransfer(); playGame_Activate(); gameState=GameStateType::PlayGame;
 assert(playGameVars.player.getHP()==100 && playGameVars.player.getGP()==5);
 assert(arduboy.randomLFSR(0,0)==0);
 assert(arduboy.randomLFSR(8,8)==8);
 // The town, inventory, and both directions of player/helmet rendering.
 render(); capture("town");
 playGameVars.world.buildingXPos=255;render(2);capture("graveyard");
 playGameVars.world.buildingXPos=-150;render(2);capture("town-right");
 playGameVars.world.buildingXPos=120;render(2);
 press(B_BUTTON); assert(playGameVars.inventory.show); render(4);
 press(B_BUTTON); assert(!playGameVars.inventory.show);
 for(int h=0;h<3;++h) {
  playGameVars.player.setHelmetType(static_cast<HelmetType>(h));
  playGameVars.player.setPlayerDirection(PlayerDirection::WalkingLeft);render(15);
  playGameVars.player.setPlayerDirection(PlayerDirection::WalkingRight);render(15);
 }
 playGameVars.player.setHelmetType(HelmetType::None);
 // Buy each shop item via its confirmation path, then render its panel.
 for(int i=0;i<6;++i) {
  playGame_ChangeViewState(ViewState::InShopItems);playGameVars.itemIdx=i;playGameVars.player.setGP(200);
  press(A_BUTTON);assert(playGameVars.viewState==ViewState::InShopItems_ConfirmPurchase);render();
  press(A_BUTTON);assert(playGameVars.viewState==ViewState::InShopItems);
  assert(playGameVars.player.getItemCount(i)==1);render(3);if(i==3)capture("item-shop");
 }
 playGameVars.itemIdx=6;render(70);press(A_BUTTON);assert(playGameVars.viewState==ViewState::InTown);
 // Buy all four rune combinations and exercise the Exit row too.
 for(int i=0;i<4;++i) {
  playGame_ChangeViewState(ViewState::InShopRunes);playGameVars.runeIdx=i;playGameVars.player.setSP(200);
  press(A_BUTTON);assert(playGameVars.viewState==ViewState::InShopRunes_ConfirmPurchase);render();
  press(A_BUTTON);assert(playGameVars.player.getRuneCombination(i)>0);render(3);if(i==0)capture("rune-shop");
 }
 playGameVars.runeIdx=4;render(70);press(A_BUTTON);assert(playGameVars.viewState==ViewState::InTown);
 // Save/load round trip, including sound option changes preserving progress.
 playGameVars.player.setHP(73);playGameVars.player.setGP(87);playGame_SaveGame();assert(EEPROM.ok);
 playGameVars.player.setHP(1);EEPROM.get(101,playGameVars);assert(playGameVars.player.getHP()==73 && playGameVars.player.getGP()==87);
 arduboy.audio.off();arduboy.audio.saveOnOff();EEPROM.begin();assert(!PortAudio::enabled());
 EEPROM.get(101,playGameVars);assert(playGameVars.player.getHP()==73);arduboy.audio.on();
 // Standard five-of-a-kind scoring without purchased rune interference.
 playGame_Activate();
 for(int i=0;i<5;++i)playGameVars.runes.getRune(i).setValue(0);
 MatchedRuneCombination match;playGameVars.runes.checkRuneCombination(playGameVars.player,match);assert(match.dmg1==45);
 // Every dungeon's enemy stats, animations, decorations and damage/death path.
 for(int d=0;d<15;++d) {
  playGameVars.world.dungeon=d+1;playGameVars.player.setHP(100);playGameVars.player.setX(5);
  playGame_InitEnenmy(d);playGame_ChangeViewState(ViewState::InDungeon_PlayersTurn);render();
  if(d==0)capture("skeleton");if(d==9)capture("werewolf");if(d==14)capture("astarok");
  uint8_t hp=playGameVars.enemy.getHP();
  uint8_t damage=45-playGameVars.enemy.getDEF();
  playGameVars.matchedRuneCombination=match;playGameVars.counter=Constants::DialogueDelay_Fight_Damage_ThisRoll;
  press(A_BUTTON);assert(playGameVars.enemy.getHP()==(hp>damage?hp-damage:0));render(4);
  // Exercise victory, including Astarok's zero-range reward.
  playGameVars.enemy.setHP(0);playGameVars.invert=1;playGameVars.counter=Constants::DialogueDelay_Fight_Damage_Apply;
  press(0);assert(playGameVars.viewState==ViewState::InDungeon_EnemyDead);render(8);
 }
 assert(playGameVars.gameOver);
 testOptionsAndRumble();
 fprintf(stderr,"PASS: town, inventory, 6 item purchases, 4 rune purchases, exit rows, save/load/options, rune scoring and 15 dungeons\n");
 return 0;
}
