#include <ioCC2540.h>
#include "bcomdef.h"
#include "OSAL.h"
#include "pwm.h"

/*
    ������Ϊamomcu ���Գ�����

    http://amomcu.taobao.com/

*/    

//---------- ע��pwm ����ʱ������˯�ߣ� ��Ϊ��Ҫ32M ʱ��


/*
---------- ע��pwm ����ʱ������˯�ߣ� ��Ϊ��Ҫ32M ʱ��

Ĭ�� 120 Hz ����Ҫ�̶��� 120
pwm_p03  pwm_p04 pwm_p05 pwm_p06
ȡֵ 0~0xff ��ʾ�ߵ�ƽ����ռ�ձ�
0xff ȫ�ߣ� 0x00 Ϊȫ��

---------- ע��pwm ����ʱ������˯�ߣ� ��Ϊ��Ҫ32M ʱ��
*/
/*
pwm_p11 ��ʾp1.1 ���pwm�ź�ռ�ձȣ� ע�����ռ�ձ��������  0xff��ռ�ձ�
pwm_p10 ��ʾp1.0 ���pwm�ź�ռ�ձȣ� ע�����ռ�ձ��������  0xff��ռ�ձ�
pwm_p07 ��ʾp0.7 ���pwm�ź�ռ�ձȣ� ע�����ռ�ձ��������  0xff��ռ�ձ�
pwm_p06 ��ʾp0.6 ���pwm�ź�ռ�ձȣ� ע�����ռ�ձ��������  0xff��ռ�ձ�
*/
void PWM_Start(uint8 pwm_p11, uint8 pwm_p10, uint8 pwm_p07, uint8 pwm_p06)
{
    uint16 radio;
    uint32 pwm_value[4];
        
    EA=0; 
    
    //����pwm�˿�Ϊ���
    //P0DIR|= BV(3)|BV(4)|BV(5)|BV(6);
    //����pwm�˿�Ϊ����˿ڣ���gpio
    //P0SEL|= BV(3)|BV(4)|BV(5)|BV(6);

    //����pwm�˿�Ϊ���
    P1DIR|= BV(0)|BV(1);
    //����pwm�˿�Ϊ����˿ڣ���gpio
    P1SEL|= BV(0)|BV(1);

    //����pwm�˿�Ϊ���
    P0DIR|= BV(6)|BV(7);
    //����pwm�˿�Ϊ����˿ڣ���gpio
    P0SEL|= BV(6)|BV(7);

    //����uart�Ȼ�ռ�����ǵ�ǰʹ�õ�pwm�˿ڣ������Ҫ��uart����ӳ�䵽��Ķ˿�ȥ��
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

    // Ƶ¶ = 32M / 128 ��Ƶ / T1CC0
    // ���� 32000000/128/2500 = 120Hz

    radio = 32000000L / 128L / 120L;
//---------- ע��pwm ����ʱ������˯�ߣ� ��Ϊ��Ҫ32M ʱ��
    
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
//---------- ע��pwm ����ʱ������˯�ߣ� ��Ϊ��Ҫ32M ʱ��
    T1STAT = ~T1STAT;
}
*/
