/**
////////////////////////////////////////////////////////////////////////////
Copyright 2016 - 2020 Yuenjee Tech. All rights reserved.


/////////////////////////////////////////////////////////////////////////////

@headerfile:       ble_transmit.h
$Date: 2016-12-15 $

Description:    ���ļ������˴����ͷ


**************************************************************************************************/

#ifndef _BLE_TRANSMIT_H_

//(F֡)������֡ͷ����
typedef struct {
    unsigned short mark;        //֡��־λ
    unsigned short crc;         //crcУ���
    unsigned char len;          //data���ֳ���
    unsigned char opcode;       //�����룬0����
    unsigned char data[0];      //�û�����
}YJ_BLE_F_HEAD;

//(T֡)С����֡ͷ����
typedef struct {
    unsigned char len;          //data���ֳ���: 0~18
    unsigned char opcode;       //�����룬0����
    unsigned char data[0];      //�û�����
}YJ_BLE_T_HEAD;

#endif//_BLE_TRANSMIT_H_
