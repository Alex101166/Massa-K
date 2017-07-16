
/******************************************************************************
 *
 * \file    ifs_ethernet.c
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

/******************************************************************************/
/*                             ЗАГОЛОВОЧНЫЕ ФАЙЛЫ                             */
/******************************************************************************/

#include "ifs_ethernet.h"
#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_spi.h"

/******************************************************************************/
/*                       ЛОКАЛЬНЫЕ МАКРООПРЕДЕЛЕНИЯ                           */
/******************************************************************************/

// Таймаут посылки данных
#define DATA_SEND_TIMEOUT_MS    200

/******************************************************************************/
/*                            ЛОКАЛЬНЫЕ ТИПЫ ДАННЫХ                           */
/******************************************************************************/

// Требуемое состояние соединения
typedef enum
{
    CONN_UNUSED = 0,
    CONN_DO_OPEN,
    CONN_DO_CLOSE
    //
} conn_state_t;

// Служебные параметры соединения
typedef struct   
{	 
    S32             conn_error;        // Поле ошибки
    U08             conn_status;       // Статус соединения
    conn_state_t    conn_do;           // Требуемое состояние сокета
    //
} conn_aux_flds_t;

// Стуктура соединения
typedef struct   
{	 
    conn_aux_flds_t     aux_flds;      // Служебные поля
    ether_connection_t  public_flds;   // Публичные поля
    //
} connection_t;

/******************************************************************************/
/*                               ГЛОБАЛЬНЫЕ ДАННЫЕ                            */
/******************************************************************************/

extern SPI_HandleTypeDef hspi2;

/******************************************************************************/
/*                              ЛОКАЛЬНЫЕ ДАННЫЕ                              */
/******************************************************************************/

// Текущее состояние интерфейса
static ifs_state_t ifs_state = IFS_STATE_IDLE;

// Признак реинициализации интерфейса
static bool ifs_reiniting = false;

// Массив соединений
static connection_t connections[_WIZCHIP_SOCK_NUM_];

// Вх. и вых буфера для сокетов (одна пара буферов на все сокеты)
static U08 rx_buff[ETH_RX_BUF_SIZE], tx_buff[ETH_TX_BUF_SIZE];

// Указатель на структуру иниц. пар-ров
static wiz_NetInfo *net_info;

/******************************************************************************/
/*                         ПРОТОТИПЫ ЛОКАЛЬНЫХ ФУНКЦИЙ                        */
/******************************************************************************/

/******************************************************************************/
/*                              ЛОКАЛЬНЫЕ ФУНКЦИИ                             */
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////
//                     callback функции для управления выводом SCSn W5500                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
static void wizchip_select(void)
{
    HAL_GPIO_WritePin(ETH_SCSn_GPIO_Port, ETH_SCSn_Pin, GPIO_PIN_RESET);
}

//
static void wizchip_deselect(void)
{
    HAL_GPIO_WritePin(ETH_SCSn_GPIO_Port, ETH_SCSn_Pin, GPIO_PIN_SET);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//                            callback SPI функции чтения/записи W5500                           //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
static void wizchip_write(U08 wb)
{
    HAL_SPI_Transmit(&hspi2, &wb, 1, HAL_MAX_DELAY);
}

//
static U08 wizchip_read(void)
{
    U08 result;
    //
    HAL_SPI_Receive(&hspi2, &result, 1, HAL_MAX_DELAY);
    //
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//                                   Аппаратный сброс W5500                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
static void w5500_reset(void)
{
    HAL_GPIO_WritePin(ETH_RST_GPIO_Port, ETH_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(ETH_RST_GPIO_Port, ETH_RST_Pin, GPIO_PIN_SET);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//                                  Intialize the WIZCHIP                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
static S32 w5500_init(void)
{
    U08 tx_mem_size[8] = {2,2,2,2,2,2,2,2};
    U08 rx_mem_size[8] = {2,2,2,2,2,2,2,2};
    //
    ///////// Register callback functions implemented by user for accessing WIZCHIP ///////
    
    // Chip selection callback
#if   _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
#elif _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_select);  // CS must be tried with LOW.
#else
  #if (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SIP_) != _WIZCHIP_IO_MODE_SIP_
    #error "Unknown _WIZCHIP_IO_MODE_"
  #else
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
  #endif
#endif
        
    // SPI Read & Write callback
    reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);
    
    ///////////////////////////////////////////////////////////////////////////////////////

    // Аппаратный сброс w5500   
    w5500_reset();
    
    // WIZCHIP SOCKET Buffer initialize
    return (wizchip_init(tx_mem_size, rx_mem_size) == 0) ? ERR_NO_ERROR : ERR_WIZCHIP_INIT_ERROR;
}
                
///////////////////////////////////////////////////////////////////////////////////////////////////
//                       Обработка текущего состояния соединения                                 //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
static void connection_process(U32 conn_num)
{
    S32 ret;
    //
    connections[conn_num].aux_flds.conn_status = getSn_SR(conn_num);
    //
    switch (connections[conn_num].aux_flds.conn_status)
    {
        //////////////////// Сокет закрыт /////////////////////
        //
        case SOCK_CLOSED:
            //
            // Сокет нужно открыть
            if (connections[conn_num].aux_flds.conn_do == CONN_DO_OPEN)
            {
                // Открытие сокета с заданным типом протокола
                U08 prot = (connections[conn_num].public_flds.conn_type == CONN_UDP) ? Sn_MR_UDP : Sn_MR_TCP;
                ret = socket(conn_num, prot, connections[conn_num].public_flds.local_port, SF_IO_NONBLOCK);
                connections[conn_num].aux_flds.conn_error = (ret == conn_num) ? ERR_NO_ERROR : ret;
            }
            // Сокет закрылся. Перевод сокета в состояние "Неиспользуемый"
            else if (connections[conn_num].aux_flds.conn_do == CONN_DO_CLOSE)
            {
                connections[conn_num].aux_flds.conn_do = CONN_UNUSED;
            }
            break;
        
        ////////////////// Инициализация сокета ///////////////
        //
        case SOCK_INIT:
            //
            // Если это TCP сервер, прослушивание порта
            if (connections[conn_num].public_flds.conn_type == CONN_TCP_SERVER)
            {
                ret = listen(conn_num);
            }
            // Если это TCP клиент, отправка запроса на соединение
            else
            {
                ret = connect(conn_num, connections[conn_num].public_flds.remote_ip, connections[conn_num].public_flds.remote_port);
            }
            connections[conn_num].aux_flds.conn_error = (ret == SOCK_OK) ? ERR_NO_ERROR : ret;
            break;

        //////////// TCP сервер прослушивает порт /////////////
        //
        case SOCK_LISTEN:
            //
            // Сокет нужно закрыть
            if (connections[conn_num].aux_flds.conn_do == CONN_DO_CLOSE)
            {
                ret = close(conn_num);
                connections[conn_num].aux_flds.conn_error = (ret == SOCK_OK) ? ERR_NO_ERROR : ret;
            }
            break;
        
        ///////////// TCP соединение установлено //////////////
        //
        case SOCK_ESTABLISHED:
            //
            // Проверка входного буфера и вызов callback, если пришли данные
            if ( (connections[conn_num].public_flds.reciv_callback != NULL) ||
                 (connections[conn_num].public_flds.reciv_transmit_callback != NULL) )
            {
                S32 rcvsize;
                //
                if (getSn_RX_RSR(conn_num) > 0)  // >0 - начало пакета данных
                {
                    rcvsize = ifs_ethernet_connection_receive(conn_num, rx_buff, 
                                                                        connections[conn_num].public_flds.reciv_packet_len, 
                                                                        connections[conn_num].public_flds.reciv_timeout_ms, 
                                                                        NULL, NULL);
                    if (rcvsize > 0)
                    {
                        // Вызов reciv_callback()
                        if (connections[conn_num].public_flds.reciv_callback != NULL)
                        {
                            connections[conn_num].aux_flds.conn_error = connections[conn_num].public_flds.reciv_callback(rx_buff, rcvsize, NULL, 0);
                        }
                        // Вызов reciv_transmit_callback()
                        else if (connections[conn_num].public_flds.reciv_transmit_callback != NULL)
                        {
                            U32 sendsize;
                            //
                            connections[conn_num].aux_flds.conn_error = connections[conn_num].public_flds.reciv_transmit_callback(rx_buff, rcvsize, tx_buff, &sendsize);
                            ifs_ethernet_connection_send(conn_num, tx_buff, sendsize, DATA_SEND_TIMEOUT_MS);
                        }
                    }
                }
            }
            //
            // Сокет нужно закрыть
            if (connections[conn_num].aux_flds.conn_do == CONN_DO_CLOSE)
            {
                ret = disconnect(conn_num);
                connections[conn_num].aux_flds.conn_error = (ret == SOCK_OK) ? ERR_NO_ERROR : ret;
            }
            break;

        ///////////////// UDP сокет открыт ////////////////
        //
        case SOCK_UDP:
            //
            // Проверка входного буфера и вызов callback, если пришли данные
            if ( (connections[conn_num].public_flds.reciv_callback != NULL) ||
                 (connections[conn_num].public_flds.reciv_transmit_callback != NULL) )
            {
                U08 src_ip[4]; 
                U16 src_port;
                S32 rcvsize;
                //
                if (getSn_RX_RSR(conn_num) > 0)  // >0 - начало пакета данных
                {
                    rcvsize = ifs_ethernet_connection_receive(conn_num, rx_buff, 
                                                                        connections[conn_num].public_flds.reciv_packet_len, 
                                                                        connections[conn_num].public_flds.reciv_timeout_ms, 
                                                                        src_ip, &src_port);
                    if (rcvsize > 0)
                    {
                        // Вызов reciv_callback()
                        if (connections[conn_num].public_flds.reciv_callback != NULL)
                        {
                            connections[conn_num].public_flds.reciv_callback(rx_buff, rcvsize, src_ip, src_port);
                        }
                        // Вызов reciv_transmit_callback()
                        else if (connections[conn_num].public_flds.reciv_transmit_callback != NULL)
                        {
                            U32 sendsize;
                            //
                            connections[conn_num].public_flds.reciv_transmit_callback(rx_buff, rcvsize, tx_buff, &sendsize);
                            ifs_ethernet_connection_send(conn_num, tx_buff, sendsize, DATA_SEND_TIMEOUT_MS);
                        }
                    }
                }
            }
            //
            // Сокет нужно закрыть
            if (connections[conn_num].aux_flds.conn_do == CONN_DO_CLOSE)
            {
                ret = close(conn_num);
                connections[conn_num].aux_flds.conn_error = (ret == SOCK_OK) ? ERR_NO_ERROR : ret;
            }
            break;
        
        ////////////// Сокет получил запрос на закрытие //////////////
        //
        case SOCK_CLOSE_WAIT:
            // 
            ret = disconnect(conn_num);
            connections[conn_num].aux_flds.conn_error = (ret == SOCK_OK) ? ERR_NO_ERROR : ret;
            break;
        
        //////////////// Остальные состояния //////////////////
        //
        default:     
            ;
            break;
    }
}

/******************************************************************************/
/*                             ГЛОБАЛЬНЫЕ ФУНКЦИИ                             */
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////
//                                  Инициализация интерфейса                                     //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
void ifs_ethernet_init(wiz_NetInfo *net_inf)
{
    net_info = net_inf;
    ifs_reiniting = true;
    ifs_state = IFS_STATE_DEINITING;  
}    
    
///////////////////////////////////////////////////////////////////////////////////////////////////
//                                Деинициализация интерфейса                                     //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
void ifs_ethernet_deinit(void)
{
    ifs_reiniting = false;
    ifs_state = IFS_STATE_DEINITING;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//                               Чтение текущего состояния интерфейса                            //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
ifs_state_t ifs_ethernet_interface_status_get(void)
{
    return ifs_state;
}
            
///////////////////////////////////////////////////////////////////////////////////////////////////
//           Получение ссылки на входной буфер (нужна для привязки буфера к интерфейсу)          //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
U08* ifs_ethernet_rxbuf_ref_get(void)
{
    return rx_buff;     // Один буфер на все соединения
}
            
///////////////////////////////////////////////////////////////////////////////////////////////////
//                                     Открытие соединения                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
S32 ifs_ethernet_connection_open(ether_connection_t *connection)
{
    U32 conn_num;
    S32 result = ERR_NO_FREE_CONNECTION;
    //
    for (conn_num=0; conn_num<countof(connections); conn_num++)
    {
        // Поиск свободного элемента в массиве соединений
        if (connections[conn_num].aux_flds.conn_do == CONN_UNUSED)
        {
            connections[conn_num].public_flds = *connection;
            connections[conn_num].aux_flds.conn_do = CONN_DO_OPEN;
            connections[conn_num].aux_flds.conn_error = ERR_NO_ERROR;
            result = conn_num;
            break; 
        }
    }
    //
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//                                  Закрытие соединения                                          //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
void ifs_ethernet_connection_close(U32 conn_num)
{
    connections[conn_num].aux_flds.conn_do = CONN_DO_CLOSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//                               Чтение текущего состояния соединения                            //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
U08 ifs_ethernet_connection_status_get(U32 conn_num)
{
    return connections[conn_num].aux_flds.conn_status;
}
            
///////////////////////////////////////////////////////////////////////////////////////////////////
//                                  Чтение кода ошибки соединения                                //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
S32 ifs_ethernet_connection_error_get(U32 conn_num)
{
    return connections[conn_num].aux_flds.conn_error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//                            Прием пакета данных через соединение                               //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Возвращает реально принятое кол-во байт (если ret >= 0) или код ошибки (если ret < 0)
//
S32 ifs_ethernet_connection_receive(U32 conn_num, U08 *buf, U32 data_len, U32 timeout_ms, U08 *src_ip, U16 *src_port)
{
    S32 recivsize, ret;
    U32 tickstart;
    //
    tickstart = HAL_GetTick();
    
    // Чтение данных, если UDP сокет открыт или TCP соединение установлено, иначе выход с ошибкой
    if ( ((connections[conn_num].public_flds.conn_type == CONN_UDP) && (getSn_SR(conn_num) == SOCK_UDP)) ||
         ((connections[conn_num].public_flds.conn_type != CONN_UDP) && (getSn_SR(conn_num) == SOCK_ESTABLISHED)) )
    {
        do
        {                
            recivsize = getSn_RX_RSR(conn_num);
            if (recivsize >= data_len) break;
        } while ((HAL_GetTick() - tickstart) < timeout_ms);          
        //
        if (recivsize > 0)
        {
            if (connections[conn_num].public_flds.conn_type == CONN_UDP)
            {
                // Для UDP данные принимаются с адреса (remote_ip : remote_port)
                ret = recvfrom(conn_num, buf, MIN(recivsize, data_len), src_ip, src_port);
            }
            else
            {
                // Для TCP данные принимаются по ранее установленному соединению
                ret = recv(conn_num, buf, MIN(recivsize, data_len));
            }
            if (ret < 0)
            {
                //close(conn_num); //??
            }
        }
        else
        {
            ret = 0;  // Данные не пришли за время timeout_ms
        }
    }
    else
    {
        ret = ERR_WRONG_CONNECTION_STATUS;
    }
    connections[conn_num].aux_flds.conn_error = (ret < 0) ? ret : ERR_NO_ERROR;
    //
    return ret;
}
    
///////////////////////////////////////////////////////////////////////////////////////////////////
//                           Посылка пакета данных через соединение                              //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
S32 ifs_ethernet_connection_send(U32 conn_num, U08 *buf, U32 data_len, U32 timeout_ms)
{
    S32 sentsize, ret;
    U32 tickstart;
    //
    tickstart = HAL_GetTick();
    
    // Посылка данных, если UDP сокет открыт или TCP соединение установлено, иначе выход с ошибкой
    if ( ((connections[conn_num].public_flds.conn_type == CONN_UDP) && (getSn_SR(conn_num) == SOCK_UDP)) ||
         ((connections[conn_num].public_flds.conn_type != CONN_UDP) && (getSn_SR(conn_num) == SOCK_ESTABLISHED)) )
    {
        sentsize = 0;
        while (data_len != sentsize)
        {
            if (connections[conn_num].public_flds.conn_type == CONN_UDP)
            {
                // Для UDP данные посылаются по адресу (remote_ip : remote_port)
                ret = sendto(conn_num, buf + sentsize, data_len - sentsize, connections[conn_num].public_flds.remote_ip, connections[conn_num].public_flds.remote_port);
            }
            else
            {
                // Для TCP данные посылаются по ранее установленному соединению
                ret = send(conn_num, buf + sentsize, data_len - sentsize);
            }
            //
            if (ret < 0)
            {
                //close(conn_num); //??
                break;
            }
            //
            if ((HAL_GetTick() - tickstart) > timeout_ms) 
            {
                ret = ERR_TIMEOUT;
                break;
            }
            //
            sentsize += ret;
        }
        // Успешное завершение передачи данных
        if (ret > 0) 
        {
            ret = ERR_NO_ERROR;
        }
    }
    else
    {
        ret = ERR_WRONG_CONNECTION_STATUS;
    }
    connections[conn_num].aux_flds.conn_error = ret;
    //
    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//                          Посылка пакета данных на заданный ip и port                          //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Данные посылаются по адресу (dest_ip : dest_port) без проверки типа и открытия сокета
//
S32 ifs_ethernet_connection_send_to(U32 conn_num, U08 *buf, U32 data_len, U32 timeout_ms, U08 *dest_ip, U16 dest_port)
{
    S32 sentsize, ret;
    U32 tickstart;
    //
    tickstart = HAL_GetTick();
    
    sentsize = 0;
    while (data_len != sentsize)
    {
        ret = sendto(conn_num, buf + sentsize, data_len - sentsize, dest_ip, dest_port);
        if (ret < 0)
        {
            //close(conn_num); //??
            break;
        }
        //
        if ((HAL_GetTick() - tickstart) > timeout_ms) 
        {
            ret = ERR_TIMEOUT;
            break;
        }
        //
        sentsize += ret;
    }
    // Успешное завершение передачи данных
    if (ret > 0) 
    {
        ret = ERR_NO_ERROR;
    }
    connections[conn_num].aux_flds.conn_error = ret;
    //
    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//                               Обработка состояний интерфейса                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Должна периодически вызываться из главного или низкоприоритетного потока
//
void ifs_ethernet_tick(void)
{
    U32 tmp;
    bool btmp;
    wiz_PhyConf phyconf;
    //
    switch (ifs_state)
    {
        case IFS_STATE_IDLE:
            //
            break;
        
        case IFS_STATE_OFF:
            //
            // Перевод интерфейса в режим POWER_DOWN
            phyconf.by = PHY_CONFBY_SW;
            wizphy_setphyconf(&phyconf);
            wizphy_setphypmode(PHY_POWER_DOWN);
            ifs_state = IFS_STATE_IDLE;
            break;
        
        case IFS_STATE_INIT1:
            //
            // Инициализация массива соединений
            memset(&connections, 0, sizeof(connections));   
            // Инициализация W5500
            switch (w5500_init())
            {
                case ERR_NO_ERROR:    
                    ifs_state = IFS_STATE_INIT2; 
                    break;
                
                default:
                    ifs_state = IFS_STATE_IDLE;
                    break;
            }
            break;

        case IFS_STATE_INIT2:
            //
            // Ожидание PHY_LINK
            switch (wizphy_getphylink())
            {
                case PHY_LINK_ON:   
                    ifs_state = IFS_STATE_INIT3; 
                    break;
                
                case PHY_LINK_OFF:
                    ;                    
                    break;
                
                default:            
                    ifs_state = IFS_STATE_IDLE;   
                    break;
            }
            break;
        
        case IFS_STATE_INIT3:
            //
            // Инициализация сетевых параметров
            if (net_info != NULL)
            {                
                wizchip_setnetinfo(net_info);
                ifs_state = IFS_STATE_INITED;
            }
            else
            {
                ifs_state = IFS_STATE_OFF;   
            }
            break;
        
        case IFS_STATE_INITED:
            //
            // Рабочий режим - опрос соединений
            for (tmp=0; tmp<countof(connections); tmp++)
            {
                if (connections[tmp].aux_flds.conn_do != CONN_UNUSED)
                {
                    connection_process(tmp);
                }
            }
            break;
        
        case IFS_STATE_DEINITING:
            //
            // Закрытие всех соединений, далее интерфейс переводится в реж. POWER_DOWN
            btmp = true;
            for (tmp=0; tmp<countof(connections); tmp++)
            {
                if (connections[tmp].aux_flds.conn_do != CONN_UNUSED)
                {
                    connections[tmp].aux_flds.conn_do = CONN_DO_CLOSE;
                    connection_process(tmp);
                    btmp = false;
                }
            }
            if (btmp) ifs_state = ifs_reiniting ? IFS_STATE_INIT1 : IFS_STATE_OFF;
            break;
        
        default:
            //
            break;
    }
}

/******************************************************************************/
/*                                   КОНЕЦ ФАЙЛА                              */
/******************************************************************************/
