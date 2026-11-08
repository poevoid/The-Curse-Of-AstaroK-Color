#pragma once
#include "ArduboyCompat.h"
#define TONES_END 0xffff
#define TONES_REPEAT 0xfffe
#define NOTE_REST 0
#define NOTE_A2 110
#define NOTE_A4 440
#define NOTE_A5 880
#define NOTE_A6 1760
#define NOTE_B4 494
#define NOTE_B5 988
#define NOTE_C3 131
#define NOTE_C5 523
#define NOTE_CS4 277
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS4 311
#define NOTE_DS5 622
#define NOTE_E4 330
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS4 370
#define NOTE_FS5 740
#define NOTE_G4 392
#define NOTE_G5 784
#define NOTE_GS4 415
class ArduboyTones {
public:
 explicit ArduboyTones(bool (*enabled)());
 void tones(const uint16_t *score);
 void noTone();
};
