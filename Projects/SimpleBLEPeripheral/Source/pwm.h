#ifndef _PWM_H_
#define _PWM_H_

/*
pwm_p11 ��ʾp1.1 ���pwm�ź�ռ�ձȣ� ע�����ռ�ձ��������  0xff��ռ�ձ�
pwm_p10 ��ʾp1.0 ���pwm�ź�ռ�ձȣ� ע�����ռ�ձ��������  0xff��ռ�ձ�
pwm_p07 ��ʾp0.7 ���pwm�ź�ռ�ձȣ� ע�����ռ�ձ��������  0xff��ռ�ձ�
pwm_p06 ��ʾp0.6 ���pwm�ź�ռ�ձȣ� ע�����ռ�ձ��������  0xff��ռ�ձ�
*/
void PWM_Start(uint8 pwm_p11, uint8 pwm_p10, uint8 pwm_p07, uint8 pwm_p06);

#endif
