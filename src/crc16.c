#include "crc16.h"

//#define CRC16_CCITT_POLY 0x1021; //CCITT
static const uint16_t CRC16_CCITT_POLY = 0x1021; //better modern practice

uint16_t crc16_ccitt_false(const uint8_t *p, size_t len)
{
    uint16_t crc = 0xFFFF; //initialize crc (0xFFFF prevents leading zeros issue)

    for (size_t i = 0; i < len; i++) //for length of data (in bytes)
    {
        //int promotion to 32b
        crc ^= p[i] << 8; //shift 8 bits so it lands on top half of accumulator as it needs to process MSB first
        
        for (int j = 0; j < 8; j++) //iterate over the 8 bits in the byte
        {
            if(crc & 0x8000) //is CRC MSB set?
            {
                //int promotion to 32b
                crc = (crc << 1) ^ CRC16_CCITT_POLY //shift by one, then XOR
            }
            else
            {
                //int promotion to 32b
                crc = crc << 1; //shift by one
            }
        }
    }
    return crc;
} 