#ifndef CRC16_H
#define CRC16_H

#include <stdint.h> 
#include <stddef.h>

uint16_t crc16_ccitt_false(const uint8_t *bufferpointer, size_t len);

#endif