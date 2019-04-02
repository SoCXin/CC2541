#include"DHT11.h"

/**************************************************
  接口定义，移植此程序只需修改下列宏定义和延时函数
**************************************************/
#define IN_DQ		P0_0  //设置数据端口
#define DQ_PIN		0     //这里需与IN_DQ一致
#define DQ_PORT		P0DIR


#define SET_OUT DQ_PORT|=BV(DQ_PIN);asm("NOP");asm("NOP")
#define SET_IN  DQ_PORT&=~(BV(DQ_PIN));asm("NOP");asm("NOP")

#define CL_DQ  IN_DQ=0;asm("NOP");asm("NOP")
#define SET_DQ IN_DQ=1;asm("NOP");asm("NOP") 


uint8  CheckSum;
uint8  tmp8BitValue;

/*函数声明*/
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
    //超时则跳出for循环		  
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
        //io 设置为输出0 省电
        SET_OUT;    
        CL_DQ;
    }
}

/*
//how to use Dht11_ReadValue
uint8 Sensor[4];
Dht11_ReadValue(Sensor);

Sensor[0]
-->湿度整数值
Sensor[1]
-->湿度小数值

Sensor[2]
-->温度整数值
Sensor[3]
-->温度小数值
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
  Delay_nus(20000);//主机拉低至少18ms  
  SET_DQ;
  Delay_nus(20);//总线由上拉电阻拉高 主机延时20us-40us
  SET_IN;
  Delay_nus(20);//总线由上拉电阻拉高 主机延时20us-40us
  if(IN_DQ == 0)
  {
    OverTimeCnt = 2;
    while((IN_DQ == 0)&&OverTimeCnt++);

    OverTimeCnt = 2;
    while((IN_DQ == 1)&&OverTimeCnt++);
    //数据接收状态	
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
-->湿度整数值
Sensor[1]
-->湿度小数值

Sensor[2]
-->温度整数值
Sensor[3]
-->温度小数值
*/
int Dht11_ReadValueInconnect(uint8 *sv)
{
  static uint8  OverTimeCnt = 0;
  int ret = 0;
/*
  Dht11_Powor(TRUE); // 要上电 1s 后再读取， 这里延时100ms 也可以
  
  Delay_nus(20000);//主机拉低至少18ms  
  Delay_nus(20000);//主机拉低至少18ms  
  Delay_nus(20000);//主机拉低至少18ms  
  Delay_nus(20000);//主机拉低至少18ms  
  Delay_nus(20000);//主机拉低至少18ms  
*/
  CL_DQ; 
  Delay_nus(20000);//主机拉低至少18ms  
  SET_DQ;
  Delay_nus(20);//总线由上拉电阻拉高 主机延时20us-40us
  SET_IN;
  Delay_nus(20);//总线由上拉电阻拉高 主机延时20us-40us
  if(IN_DQ == 0)
  {
    OverTimeCnt = 2;
    while((IN_DQ == 0)&&OverTimeCnt++);

    OverTimeCnt = 2;
    while((IN_DQ == 1)&&OverTimeCnt++);
    //数据接收状态	
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
