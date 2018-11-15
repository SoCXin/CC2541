#include <ioCC2540.h>
#include "bcomdef.h"
#include "OSAL.h"
#include "pwm.h"

/*
    此资料为amomcu 调试出来的

    http://amomcu.taobao.com/

*/    

//---------- 注意pwm 工作时不可以睡眠， 因为需要32M 时钟


/*
---------- 注意pwm 工作时不可以睡眠， 因为需要32M 时钟

默认 120 Hz 这里要固定成 120
pwm_p03  pwm_p04 pwm_p05 pwm_p06
取值 0~0xff 表示高电平脉宽占空比
0xff 全高， 0x00 为全低

---------- 注意pwm 工作时不可以睡眠， 因为需要32M 时钟
*/
/*
pwm_p11 表示p1.1 输出pwm信号占空比， 注意这个占空比是相对于  0xff的占空比
pwm_p10 表示p1.0 输出pwm信号占空比， 注意这个占空比是相对于  0xff的占空比
pwm_p07 表示p0.7 输出pwm信号占空比， 注意这个占空比是相对于  0xff的占空比
pwm_p06 表示p0.6 输出pwm信号占空比， 注意这个占空比是相对于  0xff的占空比
*/
void PWM_Start(uint8 pwm_p11, uint8 pwm_p10, uint8 pwm_p07, uint8 pwm_p06)
{
    uint16 radio;
    uint32 pwm_value[4];
        
    EA=0; 
    
    //设置pwm端口为输出
    //P0DIR|= BV(3)|BV(4)|BV(5)|BV(6);
    //设置pwm端口为外设端口，非gpio
    //P0SEL|= BV(3)|BV(4)|BV(5)|BV(6);

    //设置pwm端口为输出
    P1DIR|= BV(0)|BV(1);
    //设置pwm端口为外设端口，非gpio
    P1SEL|= BV(0)|BV(1);

    //设置pwm端口为输出
    P0DIR|= BV(6)|BV(7);
    //设置pwm端口为外设端口，非gpio
    P0SEL|= BV(6)|BV(7);

    //由于uart等会占用我们当前使用的pwm端口，因此需要将uart等重映射到别的端口去。
    //PERCFG |= 0x03;             // Move USART1&2 to alternate2 location so that T1 is visible
    
    //PERCFG = (PERCFG & ~0x40) | 0x03; // Select Timer 1 Alternative 0 location, set U1CFG and U0CFG to Alternative 1
    PERCFG |= 0x40;// Select Timer 1 Alternative 2 location   , p88

    //P2DIR |= 0xC0; // Give priority to Timer 1
    //P0SEL |= 0x78; // Set  P0.3, P0.4, P0.5 and P0.6 to peripheral
    
    T1CNTH = 0xff;
    T1CNTL = 0xff;

#if 1// 120Hz    
/*
    00: Tick frequency/1
    01: Tick frequency/8
    10: Tick frequency/32
    11: Tick frequency/128
    mode: modulo bit[1:0]=10, div: tick/128 bit[3:2]=11 
*/
    T1CTL = 0x0e; 

    // 频露 = 32M / 128 分频 / T1CC0
    // 例如 32000000/128/2500 = 120Hz

    radio = 32000000L / 128L / 120L;
//---------- 注意pwm 工作时不可以睡眠， 因为需要32M 时钟
    
    pwm_value[0] = pwm_p11;
    pwm_value[0] = pwm_value[0]*radio/255;
    pwm_value[1] = pwm_p10;
    pwm_value[1] = pwm_value[1]*radio/255;
    pwm_value[2] = pwm_p07;
    pwm_value[2] = pwm_value[2]*radio/255;
    pwm_value[3] = pwm_p06;
    pwm_value[3] = pwm_value[3]*radio/255;

    T1CC0L = radio%256;
    T1CC0H = radio/256; 

#endif    
    
    T1CCTL0 = 0x3c;
    T1CCTL1 = 0x24;
    T1CCTL2 = 0x24;
    T1CCTL3 = 0x24; 
    T1CCTL4 = 0x24; 

    T1CC1L = pwm_value[0]%256; 
    T1CC1H = pwm_value[0]/256; /* white */
    T1CC2L = pwm_value[1]%256; 
    T1CC2H = pwm_value[1]/256; /* green */
    T1CC3L = pwm_value[2]%256; 
    T1CC3H = pwm_value[2]/256; /* blue */
    T1CC4L = pwm_value[3]%256; 
    T1CC4H = pwm_value[3]/256; /* red */

    EA=1; 
    //IEN1 |= 0x02; // Enable T1 cpu interrupt
}
/*
//#pragma register_bank=2
#pragma vector = T1_VECTOR
__interrupt void pwmISR (void) 
{
//---------- 注意pwm 工作时不可以睡眠， 因为需要32M 时钟
    T1STAT = ~T1STAT;
}
*/
