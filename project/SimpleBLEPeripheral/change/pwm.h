#ifndef _PWM_H_
#define _PWM_H_

/*
pwm_p11 表示p1.1 输出pwm信号占空比， 注意这个占空比是相对于  0xff的占空比
pwm_p10 表示p1.0 输出pwm信号占空比， 注意这个占空比是相对于  0xff的占空比
pwm_p07 表示p0.7 输出pwm信号占空比， 注意这个占空比是相对于  0xff的占空比
pwm_p06 表示p0.6 输出pwm信号占空比， 注意这个占空比是相对于  0xff的占空比
*/
void PWM_Start(uint8 pwm_p11, uint8 pwm_p10, uint8 pwm_p07, uint8 pwm_p06);

#endif
