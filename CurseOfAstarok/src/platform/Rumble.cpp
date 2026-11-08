#include "ArduboyCompat.h"
#include "Rumble.h"
#include "../../RumbleConfig.h"
static_assert(RUMBLE_FULL_DAMAGE>0 && RUMBLE_MIN_DUTY<=255,"invalid rumble strength range");
static_assert(RUMBLE_MAX_MS>0 && RUMBLE_MAX_MS<=65535 && RUMBLE_MIN_MS<=RUMBLE_MAX_MS,"invalid rumble duration");
static_assert(RUMBLE_PWM_HZ>0 && F_CPU/RUMBLE_PWM_HZ>0 && F_CPU/RUMBLE_PWM_HZ<=65535,"invalid rumble PWM period");
static uint32_t startedAt;
static uint16_t duration;
static uint8_t strength, duty;
#ifndef CHSIM
extern "C" {
#include "ch32x035.h"
}
static bool ready;
static void pwmBegin() {
 RCC->APB2PCENR |= RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC;
 RCC->APB1PCENR |= RCC_APB1Periph_TIM2;
 // The WCH remap constant selects TIM2 CH4 on PC0. Other channels stay off;
 // the PC14/PC15 buttons remain inputs. TIM1's speaker remap is preserved.
 // CH32X035 TIM2 remap bits are [20:18], value 6 for the PC0 channel-4 map.
 AFIO->PCFR1=(AFIO->PCFR1&~(7u<<18))|(6u<<18);
 TIM2->CTLR1=0;TIM2->CTLR2=0;TIM2->SMCFGR=0;TIM2->DMAINTENR=0;
 TIM2->CCER=0;TIM2->CHCTLR1=0;TIM2->CHCTLR2=0x6800;
 TIM2->PSC=0;TIM2->RPTCR=0;TIM2->ATRLR=F_CPU/RUMBLE_PWM_HZ-1;
 TIM2->CH4CVR=0;TIM2->CNT=0;TIM2->BDTR=0x8000;
 TIM2->CCER=0x1000;TIM2->SWEVGR=1;TIM2->INTFR=0;
 const uint32_t mode=(GPIO_Mode_AF_PP&15u)|GPIO_Speed_50MHz;
 GPIOC->CFGLR=(GPIOC->CFGLR&~15u)|mode;
 TIM2->CTLR1=0x81;ready=true;
}
#endif
static void output(uint8_t value) {
 if(duty==value)return;
 duty=value;
#ifndef CHSIM
 if(value && !ready)pwmBegin();
 if(ready)TIM2->CH4CVR=(F_CPU/RUMBLE_PWM_HZ)*uint32_t(value)/255u;
#endif
}
void portRumbleBegin() {
 portRumbleStop();
#ifndef CHSIM
 if(!ready) {
   RCC->APB2PCENR|=RCC_APB2Periph_GPIOC;
   GPIOC->BCR=1u; // GPIO1/PC0 starts LOW, before switching it to output
   GPIOC->CFGLR=(GPIOC->CFGLR&~15u)|GPIO_Speed_50MHz;
 }
#endif
}
void portRumbleStop() { duration=0;strength=0;output(0); }
void portRumbleDamage(uint8_t hpLost) {
 if(!videoOptions.rumble || !hpLost || portOptionsOpen())return;
 unsigned damage=min(unsigned(hpLost),RUMBLE_FULL_DAMAGE);
 strength=RUMBLE_MIN_DUTY+(255u-RUMBLE_MIN_DUTY)*damage/RUMBLE_FULL_DAMAGE;
 duration=min(RUMBLE_MAX_MS,RUMBLE_MIN_MS+unsigned(hpLost)*RUMBLE_MS_PER_HP);
 if(!duration){portRumbleStop();return;}
 startedAt=millis();output(255); // brief full-duty spin-up, then scaled strength
}
void portRumbleUpdate() {
 if(!videoOptions.rumble || portOptionsOpen()){portRumbleStop();return;}
 if(!duration)return;
 uint32_t elapsed=millis()-startedAt;
 if(elapsed>=duration)portRumbleStop();
 else output(elapsed<RUMBLE_KICK_MS?255:strength);
}
uint8_t portRumbleDuty() { return duty; }
uint16_t portRumbleDuration() { return duration; }
