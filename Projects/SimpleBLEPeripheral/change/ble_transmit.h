/**
////////////////////////////////////////////////////////////////////////////
Copyright 2016 - 2020 Yuenjee Tech. All rights reserved.


/////////////////////////////////////////////////////////////////////////////

@headerfile:       ble_transmit.h
$Date: 2016-12-15 $

Description:    此文件定义了传输层头


**************************************************************************************************/

#ifndef _BLE_TRANSMIT_H_

//(F帧)大数据帧头定义
typedef struct {
    unsigned short mark;        //帧标志位
    unsigned short crc;         //crc校验和
    unsigned char len;          //data部分长度
    unsigned char opcode;       //操作码，0保留
    unsigned char data[0];      //用户数据
}YJ_BLE_F_HEAD;

//(T帧)小数据帧头定义
typedef struct {
    unsigned char len;          //data部分长度: 0~18
    unsigned char opcode;       //操作码，0保留
    unsigned char data[0];      //用户数据
}YJ_BLE_T_HEAD;

#endif//_BLE_TRANSMIT_H_
