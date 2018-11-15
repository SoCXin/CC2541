/*
CC254x test demo [��Ī��Ƭ�� www.AmoMcu.com ]  
by AMO
20130106
*/

/*
˵����

*/
#include<ioCC2540.h>
#include "stdio.h"
//#include"Common.h"

#define uint16      unsigned short

/**************************************************
  �ӿڶ��壬��ֲ�˳���ֻ���޸����к궨�����ʱ����
**************************************************/
#define DQ            P0_0                //DS18B20����IO��
#define DQ_PIN        0                //DS18B20����IO��
#define DQ_PORT       P0DIR

/*
 *    ��ʱ����
 *    ����΢��
 */
void delay_nus(uint16 timeout)
{
    while (timeout--)
    {
        asm("NOP");
        asm("NOP");
        asm("NOP");
    }
}


/**************************************************
  ���¶���ΪDS18B20֧�ֵ���������
***************************************************/

#define SEARCH_ROM      0xF0              //����ROM
#define READ_ROM        0x33              //��ROM
#define MATCH_ROM       0x55              //ƥ��ROM(�Ҷ��DS18B20ʱʹ��)
#define SKIP_ROM        0xCC              //����ƥ��ROM(����DS18B20ʱ����)
#define ALARM_SEARCH    0xEC              //��������

#define CONVERT_T       0x44              //��ʼת���¶�
#define WR_SCRATCHPAD   0x4E              //д�ڲ�ram3��4�ֽ�
#define RD_SCRATCHPAD   0xBE              //���ڲ�ram��9�ֽڵ�����
#define CPY_CCTATCHPAD  0x48              //���Ʊ��
#define RECALL_EE       0xB8              //δ����
#define RD_PWR_SUPPLY   0xB4              //����Դ��Ӧ

#define HIGH            1                 //�ߵ�ƽ
#define LOW             0                 //�͵�ƽ

#define CL_DQ()     DQ = LOW              //�������
#define SET_DQ()    DQ = HIGH             //��������
//#define SET_OUT()   DQ_PORT |=  BV(DQ_PIN);  //����IO����,out����IO����Ϊ���
//#define SET_IN()    DQ_PORT &= ~(BV(DQ_PIN));  //����IO����,in�豸IO����Ϊ����

#define SET_OUT()   DQ_PORT |=  1;  //����IO����,out����IO����Ϊ���
#define SET_IN()    DQ_PORT &= ~(1);  //����IO����,in�豸IO����Ϊ����

unsigned char FRACTION_INDEX[16] = {0, 1, 1, 2, 2, 3, 4, 4, 5, 6, 6, 7, 7, 8, 9, 9 };//С��ֵ��ѯ��

/*
������������¶�ֵ�洢������������
��������1Ϊ�������֣�����0ΪС������
*/
unsigned char sensor_data_value[2];  //����������

extern void delay_nus(uint16 n);            //��ʱn us����
extern void DS18B20_Write(unsigned char x); //DS18B20д����
extern unsigned char DS18B20_Read(void);    //DS18B20������
extern int DS18B20_Init(void);             //DS18B20��ʼ��/��λ
extern void DS18B20_SendConvert(void);      //����ת���¶�����
extern void DS18B20_GetTem(void);           //DS18B20��ȡ�¶�


/*
 *    д�����
 *    ���������  ���DS18B20.H�ж��壩
 *    ���������  ��
 *    ���ز�����  ��
 *
 */
void DS18B20_Write(unsigned char cmd)
{
    unsigned char i;
    SET_OUT();                  //����IOΪ�����2530->DS18B20
    
    /*ÿ��һλ��ѭ��8��*/
    for(i=0; i<8; i++)
    {
        CL_DQ();              //IOΪ��    
        if( cmd & (1<<i) )    //д���ݴӵ�λ��ʼ
        {
          SET_DQ();           //IO����ߵ�ƽ
        }
        else
        {
          CL_DQ();            //IO����͵�ƽ
        }
        delay_nus(40);        //����15~60us
        SET_DQ();             //IO������
    }
    SET_DQ();                 //IO������
}


/*
 *    �����ݺ���
 *    ���������  ��
 *    ���������  ��
 *    ���ز�����  ��ȡ������
 *
 */
unsigned char DS18B20_Read(void)
{
    unsigned char rdData;     //����������
    unsigned char i, dat;     //��ʱ����
    
    rdData = 0;               //���������ݳ�ʼ��Ϊ0     
    
    /* ÿ�ζ�һλ����8�� */
    for(i=0; i<8; i++)
    {
        CL_DQ();            //IO����
        SET_DQ();           //IO����
        SET_IN();           //����IO����Ϊ���� DS18B20->CC2540
        dat = DQ;           //������,�ӵ�λ��ʼ
        
        if(dat)
        {
          rdData |= (1<<i); //�������������λΪ��
        }
        else
        {
          rdData &= ~(1<<i);//�������������λΪ��
        }
        
        delay_nus(70);      //����60~120us
        SET_OUT();          //����IO����Ϊ��� CC2540->DS18B20

    }
    return (rdData);        //���ض���������
}


/*
 *    DS18B20��ʼ��/��λ����
 *    ���������  ��
 *    ���������  ��
 *    ���ز�����  0, 1 
 *
 */
int DS18B20_Init(void)
{
    int i = 5000;
    
    SET_OUT();
    SET_DQ();         //IO������
    CL_DQ();          //IO������
    delay_nus(550);   //IO���ͺ󱣳�һ��ʱ�� 480-960us
    SET_DQ();         //�ͷ�
    SET_IN();         //IO����Ϊ���� DS18B20->CC2540
    delay_nus(40);    //�ͷ����ߺ�ȴ�15-60us
    
    /* �ȴ�DQ��� */
    while(DQ && --i)
    {
        ; 
    }
    delay_nus(240);   //��⵽DQ ��ͺ���ʱ60-240us
    SET_OUT();        //����IO����Ϊ��� CC2540->DS18B20
    SET_DQ();         //IO����

    if(i == 0)
        return 0;
    else
        return 1;
}


/*
 *    DS18B20 ת���¶Ⱥ���
 *    ���������  ��
 *    ���������  ��
 *    ���ز�����  ��
 *
 */
void DS18B20_SendConvert(void)
{
    DS18B20_Init();               //��λ18B20
    DS18B20_Write(SKIP_ROM);      //��������ROMƥ�����
    DS18B20_Write(CONVERT_T);     //�����¶�ת��
}

#if 0
/*
 *    DS18B20��ȡ�¶Ⱥ���
 *    ���������  ��
 *    ���������  ��
 *    ���ز�����  ��
 *
 */
void DS18B20_GetTem(void)
{
    unsigned char tem_h,tem_l;    //�¶ȸ�λ�ֽڼ���λ�ֽ�,���洢�������ֺ�С�����ֵ�����
    unsigned char flag=0;           //�¶�������ǣ���Ϊ0����Ϊ1
    unsigned short int t;
    
    DS18B20_Init();               //DS18B20��λ       
    DS18B20_Write(SKIP_ROM);      //����ROMƥ��
    
    DS18B20_Write(RD_SCRATCHPAD); //д���9�ֽ�����
    tem_l = DS18B20_Read();       //���¶ȵ�λ����һ�ֽ�
    tem_h = DS18B20_Read();       //���¶ȸ�λ���ڶ��ֽ�

    /* �ж�RAM�д洢���¶����� 
       ����ȡ������λ����ʵ������
    */
    if(tem_h & 0x80)
    {
        flag = 1;                 //�¶�Ϊ��
        t = tem_l+tem_h<<8;
        t = ~(t-1);
        tem_l = t & 0x0f;         //ȡС������
        tem_h = (t & 0x0ff0)>>4;  //ȡ��������
        
    }
    else
    {
        flag = 0;                 //Ϊ��
        tem_h = tem_h<<4;
        tem_h += (tem_l&0xf0)>>4;     //�õ���������ֵ 

        tem_l = tem_l&0x0f;           //�ó�С������ֵ
    }
    sensor_data_value[1] = tem_h| (flag<<7);      //�������֣���������λ
    sensor_data_value[1] = FRACTION_INDEX[tem_l]; //����С��ֵ
}
#endif

void DS18B20_GetTem(void)
{
    unsigned char tem_h,tem_l;    //�¶ȸ�λ�ֽڼ���λ�ֽ�
    unsigned char a,b;            //��ʱ����
    unsigned char flag;           //�¶�������ǣ���Ϊ0����Ϊ1
    
    DS18B20_Init();               //DS18B20��λ       
    DS18B20_Write(SKIP_ROM);      //����ROMƥ��
    
    DS18B20_Write(RD_SCRATCHPAD); //д���9�ֽ�����
    tem_l = DS18B20_Read();       //���¶ȵ�λ����һ�ֽ�
    tem_h = DS18B20_Read();       //���¶ȸ�λ���ڶ��ֽ�

    /* �ж�RAM�д洢���¶����� 
       ����ȡ������λ����ʵ������
    */
    if(tem_h & 0x80)
    {
        flag = 1;                 //�¶�Ϊ��
        a = (tem_l>>4);           //ȡ�¶ȵ�4λԭ��
        b = (tem_h<<4)& 0xf0;     //ȡ�¶ȸ�4λԭ��
        /*����-ԭ��ת��
          �����Ĳ���������ԭ��Ļ�����, ����λ����, �����λȡ��, ���+1
        */
        tem_h = ~(a|b) + 1;       //ȡ����������ֵ��������λ
        
        tem_l = ~(a&0x0f) + 1;    //ȡС������ԭֵ����������λ
    }
    else
    {
        flag = 0;                 //Ϊ��
        a = tem_h<<4;
        a += (tem_l&0xf0)>>4;     //�õ���������ֵ 
        b = tem_l&0x0f;           //�ó�С������ֵ
        tem_h = a;                //��������
        tem_l = b&0xff;           //С������
    }
  
    sensor_data_value[0] = FRACTION_INDEX[tem_l]; //����С��ֵ
    sensor_data_value[1] = tem_h| (flag<<7);      //�������֣���������λ
}


float DS18B20_ReadMain(void)
{
    unsigned char tem_h,tem_l;    //�¶ȸ�λ�ֽڼ���λ�ֽ�
    unsigned char a,b;            //��ʱ����
    unsigned char flag;           //�¶�������ǣ���Ϊ0����Ϊ1
    float ft;
    
    if(DS18B20_Init() == 0)               //DS18B20��λ       
    {
        return 0.0;
    }
    else
    {
        DS18B20_Write(SKIP_ROM);      //����ROMƥ��
        
        DS18B20_Write(RD_SCRATCHPAD); //д���9�ֽ�����
        tem_l = DS18B20_Read();       //���¶ȵ�λ����һ�ֽ�
        tem_h = DS18B20_Read();       //���¶ȸ�λ���ڶ��ֽ�

        /* �ж�RAM�д洢���¶����� 
           ����ȡ������λ����ʵ������
        */
        if(tem_h & 0x80)
        {
            flag = 1;                 //�¶�Ϊ��
            a = (tem_l>>4);           //ȡ�¶ȵ�4λԭ��
            b = (tem_h<<4)& 0xf0;     //ȡ�¶ȸ�4λԭ��
            /*����-ԭ��ת��
              �����Ĳ���������ԭ��Ļ�����, ����λ����, �����λȡ��, ���+1
            */
            tem_h = ~(a|b) + 1;       //ȡ����������ֵ��������λ
            
            tem_l = ~(a&0x0f) + 1;    //ȡС������ԭֵ����������λ
        }
        else
        {
            flag = 0;                 //Ϊ��
            a = tem_h<<4;
            a += (tem_l&0xf0)>>4;     //�õ���������ֵ 
            b = tem_l&0x0f;           //�ó�С������ֵ
            tem_h = a;                //��������
            tem_l = b&0xff;           //С������
        }
      
        sensor_data_value[0] = FRACTION_INDEX[tem_l]; //����С��ֵ
        sensor_data_value[1] = tem_h| (flag<<7);      //�������֣���������λ

        ft = sensor_data_value[1] + ((float)sensor_data_value[0])/10.0;


            //��ʼת��
        DS18B20_SendConvert();
    }
    
    return ft;
}


#if  0
int main()
{
  HAL_BOARD_INIT();
  HalLcd_HW_Init();
  
  HalLcd_HW_WriteLine(HAL_LCD_LINE_1, "      CC254xEM");
  HalLcd_HW_WriteLine(HAL_LCD_LINE_3, "-->DS18B20_TEST"); 
  HalLcd_HW_WriteLine(HAL_LCD_LINE_4, "Current Temp:");

  uint8 i;
  uint8 buf[10];
  while(1){
       
    //��ʼת��
    DS18B20_SendConvert();
    //��ʱ1S
    for(i=20; i>0; i--)
      delay_nus(50000);
    DS18B20_GetTem();
    sprintf(buf, "%d", sensor_data_value[1]);
    HalLcd_HW_WriteLine(HAL_LCD_LINE_5, buf);
  }
}
#endif


