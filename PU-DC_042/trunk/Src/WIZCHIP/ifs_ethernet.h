
/******************************************************************************
 *
 * \file    ifs_ethernet.h
 *
 * \author  <������� ������> petrov.av@massa.ru
 *
 * \date    Created 09.06.2017
 *
 * \brief   ������ ���������� Eternet �� ���� �/��. W5500
 *
 * \details
 *
 * ��� "�����_�", 2017. ��� ����� ��������.
 *
 ******************************************************************************/

#ifndef __IFS_ETHERNET_H__
#define __IFS_ETHERNET_H__

/******************************************************************************/
/*                             ������������ �����                             */
/******************************************************************************/

#include "types_defines.h"
#include "socket.h"

/******************************************************************************/
/*                         ���������� ����������������                        */
/******************************************************************************/

// ������� ������� ������/�������� ��� �������
#define ETH_RX_BUF_SIZE                 50
#define ETH_TX_BUF_SIZE                 50

// ���� ������
#define  ERR_NO_ERROR                    0   // ��� ������
#define  ERR_NO_FREE_CONNECTION       -100   // ��������� ��������� ����������
#define  ERR_WRONG_CONNECTION_STATUS  -101   // �������� ��������� ����������
#define  ERR_TIMEOUT                  -102   // �������
#define  ERR_WIZCHIP_INIT_ERROR       -103   // ������ ������������� W5500

/******************************************************************************/
/*                            ���������� ���� ������                          */
/******************************************************************************/

// C�������� ����������
typedef enum
{
    IFS_STATE_IDLE = 0,
    IFS_STATE_OFF,
    IFS_STATE_INIT1,
    IFS_STATE_INIT2,
    IFS_STATE_INIT3,
    IFS_STATE_INITED,
    IFS_STATE_DEINITING,
    //
    IFS_STATE_CNT   // ���-�� ���������
    //
} ifs_state_t;

// ��� ����������
typedef enum
{
    CONN_TCP_SERVER = 1,
    CONN_TCP_CLIENT,
    CONN_UDP
    //
} eth_conn_type_t;

// ��������� ����������
typedef struct   
{	 
    U08             remote_ip[4];      // ������� IP (�.�. ������ ����� ��������� ��-�� ������������)
    U16             remote_port;       // ������� ����
    U16             local_port;        // ��������� ����
    eth_conn_type_t conn_type;         // ��� ���������� (TCP ������, TCP ������, UDP)
    //
    // ���� ��� ����������� ������, ��������� � �������� ������ ����� callback
    //
    U32             reciv_timeout_ms;  // ������� ������ ������ ������ ��� ������ callback
    U32             reciv_packet_len;  // ����� ������ ������ ��� ������ callback
    // callback ��� ������ ������ ������
    S32           (*reciv_callback)(U08 *rx_buf, U16 rx_size, U08 *src_ip, U16 src_port);
    // callback ��� ������, ��������� � ����������� �������� ������ ������
    S32           (*reciv_transmit_callback)(U08 *rx_buf, U32 rx_size, U08 *tx_buf, U32 *tx_size);
    //
} ether_connection_t;

/******************************************************************************/
/*                              ���������� ������                             */
/******************************************************************************/
                                                                              
/******************************************************************************/
/*                         ��������� ���������� �������                       */
/******************************************************************************/

// ������������� ����������
extern void ifs_ethernet_init(wiz_NetInfo *net_inf);

// ��������������� ����������
extern void ifs_ethernet_deinit(void);

// ������ �������� ��������� ����������
extern ifs_state_t ifs_ethernet_interface_status_get(void);

// ��������� ������ �� ������� ����� (����� ��� �������� ������ � ����������)
extern U08* ifs_ethernet_rxbuf_ref_get(void);

// �������� ����������
extern S32 ifs_ethernet_connection_open(ether_connection_t *connection);

// �������� ����������
extern void ifs_ethernet_connection_close(U32 conn_num);

// ������ �������� ��������� ����������
extern U08 ifs_ethernet_connection_status_get(U32 conn_num);

// ������ ������ ���������
extern S32 ifs_ethernet_connection_error_get(U32 conn_num);

// ����� ������ ������ ����� ����������
extern S32 ifs_ethernet_connection_receive(U32 conn_num, U08 *buf, U32 data_len, U32 timeout, U08 *src_ip, U16 *src_port);

// ������� ������ ������ ����� ����������
extern S32 ifs_ethernet_connection_send(U32 conn_num, U08 *buf, U32 data_len, U32 timeout_ms);

// ������� ������ ������ �� �������� ip � port
extern S32 ifs_ethernet_connection_send_to(U32 conn_num, U08 *buf, U32 data_len, U32 timeout_ms, U08 *dest_ip, U16 dest_port);

// ����������� ��������� ������ ��������� ����������
extern void ifs_ethernet_tick(void);

#endif

/******************************************************************************/
/*                                   ����� �����                              */
/******************************************************************************/
