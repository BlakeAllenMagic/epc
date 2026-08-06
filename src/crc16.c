#include "crc16.h"

uint16_t crc16_ccitt_false(const uint8_t *p, size_t len)
{
    uint16_t crc = 0xFFFF; //initialize crc to FFFF (prevents leading zeros issue)

    for (size_t i = 0; i < len; i++) //for length of data (in bytes)
    {
        crc ^= p[i] << 8; //shift 8 bits so it lands on top half of accumulator as it needs to process MSB first
        
        for (int j = 0; j < 8; j++) //iterate over the 8 bits in the byte
        {
            if(crc & 0x8000)
        }

    }

}