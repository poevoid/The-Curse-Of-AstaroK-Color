#pragma once
#include <stdint.h>
void portRumbleBegin();
void portRumbleDamage(uint8_t hpLost);
void portRumbleUpdate();
void portRumbleStop();
uint8_t portRumbleDuty();
uint16_t portRumbleDuration();
