"""Compile the actual PWM backend against fake registers, without CHSIM.
Checks GPIO1/PC0 selection, TIM2_CH4 mapping, PWM period/duty, and preservation
of the speaker's TIM1 remap and the remaining GPIOC pin configuration.
"""
from pathlib import Path
import subprocess,tempfile,shutil
root=Path(__file__).resolve().parents[1];sk=root/'CurseOfAstarok'
with tempfile.TemporaryDirectory() as temp:
 d=Path(temp);p=d/'src/platform';p.mkdir(parents=True)
 for n in ['Rumble.cpp','Rumble.h','VideoOptions.h']:shutil.copy(sk/'src/platform'/n,p/n)
 shutil.copy(sk/'RumbleConfig.h',d/'RumbleConfig.h')
 (p/'ArduboyCompat.h').write_text('''#pragma once
#include <stdint.h>
#include <algorithm>
#include "VideoOptions.h"
#define F_CPU 48000000u
using std::min;
uint32_t millis();
bool portOptionsOpen();
''')
 (d/'ch32x035.h').write_text('''#pragma once
#include <stdint.h>
struct TestTimer {uint32_t CTLR1,CTLR2,SMCFGR,DMAINTENR,CCER,CHCTLR1,CHCTLR2,PSC,RPTCR,ATRLR,CH4CVR,CNT,BDTR,SWEVGR,INTFR;};
struct TestGPIO {uint32_t CFGLR,CFGHR,BCR;};
struct TestRCC {uint32_t APB1PCENR,APB2PCENR;};
struct TestAFIO {uint32_t PCFR1;};
extern TestTimer testTim1,testTim2;
extern TestGPIO testGpioC;
extern TestRCC testRcc;
extern TestAFIO testAfio;
#define TIM1 (&testTim1)
#define TIM2 (&testTim2)
#define GPIOC (&testGpioC)
#define RCC (&testRcc)
#define AFIO (&testAfio)
#define RCC_APB2Periph_AFIO 1u
#define RCC_APB2Periph_GPIOC 16u
#define RCC_APB1Periph_TIM2 1u
#define GPIO_Mode_AF_PP 0x18u
#define GPIO_Speed_50MHz 1u
''')
 (d/'main.cpp').write_text('''#include <cassert>
#include <cstring>
#include <cstdio>
#include "ch32x035.h"
#include "src/platform/ArduboyCompat.h"
#include "src/platform/Rumble.h"
TestTimer testTim1{},testTim2{};
TestGPIO testGpioC{0x44444444u,0x88888888u,0};
TestRCC testRcc{};
TestAFIO testAfio{0xa5a5a5a5u};
VideoOptions videoOptions{0,15,0,1};
static uint32_t now;
uint32_t millis(){return now;}
bool portOptionsOpen(){return false;}
int main(){
 const uint32_t remap=testAfio.PCFR1, pins=testGpioC.CFGLR, highPins=testGpioC.CFGHR;
 testTim1.CTLR1=0x81;testTim1.PSC=47;testTim1.ATRLR=999;testTim2.RPTCR=7;
 TestTimer speaker=testTim1;
 portRumbleBegin();assert(testGpioC.BCR==1 && (testGpioC.CFGLR&15)==1);
 assert((testGpioC.CFGLR&~15u)==(pins&~15u));
 portRumbleDamage(5);
 assert(testTim2.CCER==0x1000 && testTim2.CHCTLR1==0 && testTim2.CHCTLR2==0x6800);
 assert(testTim2.PSC==0 && testTim2.RPTCR==0 && testTim2.ATRLR==2399 && testTim2.CH4CVR==2400);
 assert((testGpioC.CFGLR&15)==9 && testGpioC.CFGHR==highPins);
 assert((testAfio.PCFR1&~(7u<<18))==(remap&~(7u<<18)));
 assert(((testAfio.PCFR1>>18)&7)==6 && memcmp(&speaker,&testTim1,sizeof speaker)==0);
 now=25;portRumbleUpdate();assert(portRumbleDuty()==101 && testTim2.CH4CVR==950);
 now=111;portRumbleUpdate();assert(testTim2.CH4CVR==0);
 portRumbleDamage(20);videoOptions.rumble=0;portRumbleUpdate();assert(testTim2.CH4CVR==0);
 puts("PASS: actual PWM backend register configuration, 20 kHz CH4 duty, stop, GPIO neighbors and speaker remap preservation");
}
''')
 exe=d/'rumble';subprocess.run(['g++','-std=c++14','-O2','-I'+str(d),str(p/'Rumble.cpp'),str(d/'main.cpp'),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
