
#include "common.h"


quint16 CRC16Table[16] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF
};

quint16 GetCRC16(quint8 *pdata, quint16 len){

    quint16 val;
    quint8 tmp;
    quint16 i;

    val = 0xFFFF;
    for (i = 0; i < len; i++)
    {
        tmp = val >> 12;
        val <<= 4;
        val ^= CRC16Table[tmp ^ (*pdata / 16)];
        tmp = val >> 12;
        val <<= 4;
        val ^= CRC16Table[tmp ^ (*pdata & 0x0F)];
        pdata++;
    }
    return val;
}
