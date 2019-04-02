#include"DHT11.h"

/**************************************************
  �ӿڶ��壬��ֲ�˳���ֻ���޸����к궨�����ʱ����
**************************************************/
#define IN_DQ		P0_0  //�������ݶ˿�
#define DQ_PIN		0     //��������IN_DQһ��
#define DQ_PORT		P0DIR


#define SET_OUT DQ_PORT|=BV(DQ_PIN);asm("NOP");asm("NOP")
#define SET_IN  DQ_PORT&=~(BV(DQ_PIN));asm("NOP");asm("NOP")

#define CL_DQ  IN_DQ=0;asm("NOP");asm("NOP")
#define SET_DQ IN_DQ=1;asm("NOP");asm("NOP") 


uint8  CheckSum;
uint8  tmp8BitValue;

/*��������*/
void Read8Bit(void);
static void Delay_nus(uint16 s);


static void Delay_nus(uint16 s) 
{
  while (s--)
  {
#if 0
    asm("NOP");
    asm("NOP");
    asm("NOP");
#else
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
#endif
  }
}

void Read8Bit(void)
{
  static uint8  OverTimeCnt = 0;
  uint8 i,tmpBit;
  
  for(i=0;i<8;i++)
  {
    OverTimeCnt = 2;
    while((IN_DQ == 0)&&OverTimeCnt++);
    //while(IN_DQ == 0);
    Delay_nus(19);//12
    if(IN_DQ == 1)
      tmpBit = 1;
    else
      tmpBit = 0;
    OverTimeCnt = 2;
    while((IN_DQ == 1)&&OverTimeCnt++);
    //while(IN_DQ == 1);
    //��ʱ������forѭ��		  
    if(OverTimeCnt==1)
      break;
    
    tmp8BitValue<<=1;
    tmp8BitValue|=tmpBit;        //0
  
  }

}

void Dht11_Powor(bool onoff)
{
    if(onoff)
    {
        SET_OUT;
        SET_DQ;
    }
    else
    {
        //io ����Ϊ���0 ʡ��
        SET_OUT;    
        CL_DQ;
    }
}

/*
//how to use Dht11_ReadValue
uint8 Sensor[4];
Dht11_ReadValue(Sensor);

Sensor[0]
-->ʪ������ֵ
Sensor[1]
-->ʪ��С��ֵ

Sensor[2]
-->�¶�����ֵ
Sensor[3]
-->�¶�С��ֵ
*/
#if 0
int Dht11_ReadValue(uint8 *sv)
{
  static uint8  i= 0;
  sv[0] = sv[1] = sv[2] = sv[3] = i++;
  return 1;
}
#else
int Dht11_ReadValue(uint8 *sv)
{
  static uint8  OverTimeCnt = 0;
  int ret = 0;

  SET_OUT;
  SET_DQ;  
  
  CL_DQ; 
  Delay_nus(20000);//������������18ms  
  SET_DQ;
  Delay_nus(20);//������������������ ������ʱ20us-40us
  SET_IN;
  Delay_nus(20);//������������������ ������ʱ20us-40us
  if(IN_DQ == 0)
  {
    OverTimeCnt = 2;
    while((IN_DQ == 0)&&OverTimeCnt++);

    OverTimeCnt = 2;
    while((IN_DQ == 1)&&OverTimeCnt++);
    //���ݽ���״̬	
    Read8Bit();
    sv[0]=tmp8BitValue;
    Read8Bit();
    sv[1]=tmp8BitValue;
    Read8Bit();
    sv[2]=tmp8BitValue;
    Read8Bit();
    sv[3]=tmp8BitValue;
	
    Read8Bit();
    CheckSum = tmp8BitValue;
    
    if(CheckSum == sv[0]+sv[1]+sv[2]+sv[3])
    {
      CheckSum = 0xff;    
    }

    ret = 1;
  }

  return ret;
}
#endif

/*
//how to use Dht11_ReadValue
uint8 Sensor[4];
Dht11_ReadValue(Sensor);

Sensor[0]
-->ʪ������ֵ
Sensor[1]
-->ʪ��С��ֵ

Sensor[2]
-->�¶�����ֵ
Sensor[3]
-->�¶�С��ֵ
*/
int Dht11_ReadValueInconnect(uint8 *sv)
{
  static uint8  OverTimeCnt = 0;
  int ret = 0;
/*
  Dht11_Powor(TRUE); // Ҫ�ϵ� 1s ���ٶ�ȡ�� ������ʱ100ms Ҳ����
  
  Delay_nus(20000);//������������18ms  
  Delay_nus(20000);//������������18ms  
  Delay_nus(20000);//������������18ms  
  Delay_nus(20000);//������������18ms  
  Delay_nus(20000);//������������18ms  
*/
  CL_DQ; 
  Delay_nus(20000);//������������18ms  
  SET_DQ;
  Delay_nus(20);//������������������ ������ʱ20us-40us
  SET_IN;
  Delay_nus(20);//������������������ ������ʱ20us-40us
  if(IN_DQ == 0)
  {
    OverTimeCnt = 2;
    while((IN_DQ == 0)&&OverTimeCnt++);

    OverTimeCnt = 2;
    while((IN_DQ == 1)&&OverTimeCnt++);
    //���ݽ���״̬	
    Read8Bit();
    sv[0]=tmp8BitValue;
    Read8Bit();
    sv[1]=tmp8BitValue;
    Read8Bit();
    sv[2]=tmp8BitValue;
    Read8Bit();
    sv[3]=tmp8BitValue;
	
    Read8Bit();
    CheckSum = tmp8BitValue;
    
    if(CheckSum == sv[0]+sv[1]+sv[2]+sv[3])
    {
      CheckSum = 0xff;    
    }

    ret = 1;
  }

  Dht11_Powor(FALSE);

  return ret;
}


#if 0
int main()
{
  HAL_BOARD_INIT();
  HalLcd_HW_Init();
  
  HalLcd_HW_WriteLine(HAL_LCD_LINE_1, "      CC2530EM");
  HalLcd_HW_WriteLine(HAL_LCD_LINE_3, "-->DHT11_TEST"); 
  HalLcd_HW_WriteLine(HAL_LCD_LINE_4, "Current hum & temp:");
  
  uint8 hum[10];
  uint8 temp[10];
  uint8 Sensor[4];
  while(1){
    ReadValue(Sensor);
    IntToStr(hum,Sensor[0]);
    IntToStr(temp,Sensor[2]);
    HalLcd_HW_WriteLine(HAL_LCD_LINE_5, hum);
    HalLcd_HW_WriteLine(HAL_LCD_LINE_6, temp);
    HalHW_WaitMS(500);
  }
  return 0;
}
#endif
