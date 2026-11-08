#pragma once
// CHGame Rev0 expansion GPIO1 = PC0 = H1 pin 8, TIM2 channel 4.
// This is a 3.3 V active-high DRIVER CONTROL output, not motor power.
// Connect a suitable rumble/motor driver; do not attach a motor to the GPIO.
#define RUMBLE_PWM_HZ          20000u
#define RUMBLE_FULL_DAMAGE        40u  // HP lost for maximum duty
#define RUMBLE_MIN_DUTY            80u // minimum running duty, 0..255
#define RUMBLE_MIN_MS              50u
#define RUMBLE_MS_PER_HP           12u
#define RUMBLE_MAX_MS             650u
#define RUMBLE_KICK_MS             20u // startup kick for a small ERM motor
