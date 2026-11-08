#include "Tones.h"
static const uint16_t *startScore, *nextNote;
static uint32_t deadline;
static uint16_t heldHz, remaining;
ArduboyTones::ArduboyTones(bool (*)()) {}
static void silence() { audio::setOn(false); audio::setOn(PortAudio::enabled()); audio::blip(0,2); }
void ArduboyTones::tones(const uint16_t *score) {
 silence(); startScore=nextNote=score; remaining=0; deadline=millis(); portAudioUpdate();
}
void ArduboyTones::noTone() { startScore=nextNote=nullptr; remaining=0; silence(); }
void portAudioUpdate() {
 if(!nextNote || int32_t(millis()-deadline)<0) return;
 if(!remaining) {
   uint16_t hz=*nextNote++;
   if(hz==TONES_REPEAT) { nextNote=startScore; hz=*nextNote++; }
   if(hz==TONES_END) { nextNote=nullptr; return; }
   heldHz=hz; remaining=*nextNote++;
 }
 // CHGame's effect steps store at most 510 ms. Split long notes so original
 // 750/1000/1100 ms durations do not overflow the 8-bit effect duration.
 uint16_t duration=remaining>500 ? 500 : remaining;
 if(PortAudio::enabled() && heldHz) audio::blip(heldHz,duration);
 else silence();
 remaining-=duration; deadline=millis()+duration;
}
