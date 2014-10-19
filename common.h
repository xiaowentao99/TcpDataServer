#ifndef COMMON_H
#define COMMON_H
#define MINI_PACKET_LENGTH 21

#include <qglobal.h>

static quint16 g_server_port=60000;
static quint16 g_node_id=0x0301;

//unsigned short int g_server_port=60004;

//quint16 g_node_id=0x0301;

quint16 GetCRC16(quint8 *pdata, quint16 len);
//quint16 GetCRC16(quint8 *pdata, quint16 len)

#endif // COMMON_H
