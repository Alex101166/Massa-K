
/******************************************************************************
 *
 * \file    ifs_ethernet.h
 *
 * \author  <Алексей Петров> petrov.av@massa.ru
 *
 * \date    Created 09.06.2017
 *
 * \brief   Модуль интерфейса Eternet на базе м/сх. W5500
 *
 * \details
 *
 * ЗАО "МАССА_К", 2017. Все права защищены.
 *
 ******************************************************************************/

#ifndef __IFS_ETHERNET_H__
#define __IFS_ETHERNET_H__

/******************************************************************************/
/*                             ЗАГОЛОВОЧНЫЕ ФАЙЛЫ                             */
/******************************************************************************/

#include "types_defines.h"
#include "socket.h"

/******************************************************************************/
/*                         ГЛОБАЛЬНЫЕ МАКРООПРЕДЕЛЕНИЯ                        */
/******************************************************************************/

// Размеры буферов приема/передачи для сокетов
#define ETH_RX_BUF_SIZE                 50
#define ETH_TX_BUF_SIZE                 50

// Коды ошибок
#define  ERR_NO_ERROR                    0   // Нет ошибок
#define  ERR_NO_FREE_CONNECTION       -100   // Исчерпаны свободные соединения
#define  ERR_WRONG_CONNECTION_STATUS  -101   // Неверное состояние соединения
#define  ERR_TIMEOUT                  -102   // Таймаут
#define  ERR_WIZCHIP_INIT_ERROR       -103   // Ошибка инициализации W5500

/******************************************************************************/
/*                            ГЛОБАЛЬНЫЕ ТИПЫ ДАННЫХ                          */
/******************************************************************************/

// Cостояния интерфейса
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
    IFS_STATE_CNT   // Кол-во состояний
    //
} ifs_state_t;

// Тип соединения
typedef enum
{
    CONN_TCP_SERVER = 1,
    CONN_TCP_CLIENT,
    CONN_UDP
    //
} eth_conn_type_t;

// Параметры соединения
typedef struct   
{	 
    U08             remote_ip[4];      // Внешний IP (д.б. первым полем структуры из-за выравнивания)
    U16             remote_port;       // Внешний порт
    U16             local_port;        // Локальный порт
    eth_conn_type_t conn_type;         // Тип соединения (TCP клиент, TCP сервер, UDP)
    //
    // Поля для организации приема, обработки и отправки данных через callback
    //
    U32             reciv_timeout_ms;  // Таймаут приема пакета данных при вызове callback
    U32             reciv_packet_len;  // Длина пакета данных при вызове callback
    // callback для приема пакета данных
    S32           (*reciv_callback)(U08 *rx_buf, U16 rx_size, U08 *src_ip, U16 src_port);
    // callback для приема, обработки и последующей передачи пакета данных
    S32           (*reciv_transmit_callback)(U08 *rx_buf, U32 rx_size, U08 *tx_buf, U32 *tx_size);
    //
} ether_connection_t;

/******************************************************************************/
/*                              ГЛОБАЛЬНЫЕ ДАННЫЕ                             */
/******************************************************************************/
                                                                              
/******************************************************************************/
/*                         ПРОТОТИПЫ ГЛОБАЛЬНЫХ ФУНКЦИЙ                       */
/******************************************************************************/

// Инициализация интерфейса
extern void ifs_ethernet_init(wiz_NetInfo *net_inf);

// Деинициализация интерфейса
extern void ifs_ethernet_deinit(void);

// Чтение текущего состояния интерфейса
extern ifs_state_t ifs_ethernet_interface_status_get(void);

// Получение ссылка на входной буфер (нужна для привязки буфера к интерфейсу)
extern U08* ifs_ethernet_rxbuf_ref_get(void);

// Открытие соединения
extern S32 ifs_ethernet_connection_open(ether_connection_t *connection);

// Закрытие соединения
extern void ifs_ethernet_connection_close(U32 conn_num);

// Чтение текущего состояния соединения
extern U08 ifs_ethernet_connection_status_get(U32 conn_num);

// Чтение ошибки соединеия
extern S32 ifs_ethernet_connection_error_get(U32 conn_num);

// Прием пакета данных через соединение
extern S32 ifs_ethernet_connection_receive(U32 conn_num, U08 *buf, U32 data_len, U32 timeout, U08 *src_ip, U16 *src_port);

// Посылка пакета данных через соединение
extern S32 ifs_ethernet_connection_send(U32 conn_num, U08 *buf, U32 data_len, U32 timeout_ms);

// Посылка пакета данных на заданный ip и port
extern S32 ifs_ethernet_connection_send_to(U32 conn_num, U08 *buf, U32 data_len, U32 timeout_ms, U08 *dest_ip, U16 dest_port);

// Циклическая процедура опроса состояния соединения
extern void ifs_ethernet_tick(void);

#endif

/******************************************************************************/
/*                                   КОНЕЦ ФАЙЛА                              */
/******************************************************************************/
