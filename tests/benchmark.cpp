#include <chrono>
#include <cassert>
#define main chsim_unused_main
#include "main.cpp"
#undef main
#include "src/utils/Arduboy2Ext.h"
#include "src/entities/Entities.h"
#include "src/utils/Structs.h"
#include "GameDeclarations.h"
extern Arduboy2Ext arduboy;
extern PlayGameStateVars playGameVars;
extern GameStateType gameState;
int main() {
 setup();playGame_Activate();gameState=GameStateType::PlayGame;
 const int positions[]={255,120,-150};
 for(int i=0;i<4;++i) {
  playGame_Activate();
  if(i<3)playGameVars.world.buildingXPos=positions[i];
  else {playGame_InitEnenmy(9);playGameVars.world.dungeon=10;playGame_ChangeViewState(ViewState::InDungeon_PlayersTurn);}
  for(int n=0;n<80;++n){++arduboy.frameCount;playGame_Render();}
  uint64_t best=~uint64_t(0);
  for(int rep=0;rep<3;++rep) {
   uint64_t start=sim_hostNanos();
   for(int n=0;n<1000;++n){++arduboy.frameCount;playGame_Render();}
   uint64_t elapsed=sim_hostNanos()-start;if(elapsed<best)best=elapsed;
  }
  printf("%s%d %.3f us CPU/frame\n",i<3?"town ":"dungeon ",i<3?positions[i]:10,best/1000.0/1000.0);
 }
}
