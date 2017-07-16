
/******************************************************************************
 *
 * \file    xxx.h
 * 
 * \author  
 *
 * \date
 * 
 * \brief   
 * 
 * \details           
 *
 * ��� "�����_�", 2016. ��� ����� ��������.
 *
 ******************************************************************************/

#ifndef __Coders_H
#define __Coders_H

/******************************************************************************/
/*                             ������������ �����                             */
/******************************************************************************/

/******************************************************************************/
/*                         ���������� ����������������                        */
/******************************************************************************/

#define CMD_UDP_POLL	0x00 // Ok?
#define CMD_TCP_GET_SERIAL	0x90 //NASK
#define CMD_TCP_SET_TARE	0xA3
#define CMD_TCP_GET_WEIGHT	0xA0 // Ok
#define CMD_TCP_GET_SCALE_INFO	0x20
#define CMD_TCP_SET_SCALE_NAME 0x25 // Ok
#define CMD_TCP_GET_SCALE_IDS	0x23 // Ok
#define CMD_TCP_SET_WIFI	0x31 // Ok
#define CMD_TCP_GET_WIFI 0x33 //Ok
#define CMD_TCP_SET_ETHERNET 0x26 // Ok
#define CMD_TCP_GET_ETHERNET	0x29 //Ok
#define CMD_TCP_SET_RS232 	0x2C
#define CMD_TCP_GET_RS232	0x2F
#define CMD_USB_SET_SCALE_INFO	0x32
#define CMD_TCP_GET_WIFI_SSIDS 0x2B //Ok?
#define CMD_TCP_SET_STATE 0x35
#define CMD_TCP_GET_STATE 0x36

#define CMD_UDP_RES_ID	0x01  //Ok?
#define CMD_TCP_ACK_SERIAL	0x50 //NASK
#define CMD_TCP_ACK_TARE	0x12
#define CMD_TCP_NACK_TARE	0x15
#define CMD_TCP_ACK_WEIGHT	0x10 // Ok
#define CMD_TCP_NACK_WEIGHT	0x14 //Ok?
#define CMD_TCP_ACK_SCALE_INFO 	0x21
#define CMD_TCP_NACK_SCALE_INFO 	0x22
#define CMD_TCP_ACK_SCALE_IDS 	0x24 //Ok?
#define CMD_TCP_ACK_GET_ETHERNET	0x2A //Ok
#define CMD_TCP_ACK_GET_WIFI 0x34 //Ok
#define CMD_TCP_ACK_GET_RS232	0x30
#define CMD_TCP_ACK_WIFI_SSIDS 0x2D //Ok?
#define CMD_TCP_ACK_GET_STATE 0x37

#define CMD_TCP_ERROR 	0x28
#define CMD_TCP_ACK_SET 	0x27
#define CMD_TCP_NACK	0xF0

#define TCP_ERROR_SAVE 11
#define TCP_ERROR_ESP_START 14
#define TCP_ERROR_ESP_BUSY 15

/******************************************************************************/
/*                            ���������� ���� ������                          */
/******************************************************************************/

typedef enum
{
    IFS_RS232,
    IFS_USB,
    IFS_ETHERNET,
    IFS_WIFI
    //
} interface_type_t;

/******************************************************************************/
/*                              ���������� ������                             */
/******************************************************************************/
                                                                              
/******************************************************************************/
/*                         ��������� ���������� �������                       */
/******************************************************************************/

// ������������� ������
extern void CodersInit(void);

//�������� �������� ������ � ����������
extern void CoderBuffLink(unsigned char* buffer, interface_type_t Interface);

// �������� ������������� ������
// ��������� �� �����������
extern int IsDataReady(unsigned char *RxDecodeData, unsigned int FullLenght);
// RxDecodeData - �����, ���������� ������
// FullLenght - ���������� �������� ������
// ���������� ����� ���� ���������
// � ������ ������:
//  0 - ������ �� ������
// -1 - �� ��������� ��
// -2 - ������ ��������� ������
// -3 - ������ ��������� (������ ����)

// ��������� ������
extern int DecodeData(unsigned char *RxDecodeData, unsigned int RxDataLen, unsigned char *TxCodeData, unsigned int* TxDataLen);
// ��� ���������� ���������� ���������������� ������ RxDecodeData � TxCodeData
// !!! � �������� ���������� �������� � ��� �������� !!!
// RxDecodeData - �����, ���������� ������
// TxCodeData - �����, ���������� ������ ��� ��������
// FullLenght - ���������� �������� ������
// ���������� ���������� ������ ��� ��������
// � ������ ������:
//  0 - ������� �� �������

#endif

/******************************************************************************/
/*                                   ����� �����                              */
/******************************************************************************/
