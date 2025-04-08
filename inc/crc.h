// crc.h
#ifndef CRC_H
#define CRC_H

#include <stdint.h>

uint16_t crc16(const uint8_t *data, uint16_t length);
uint16_t crc16_ccitt_hw(const uint8_t *data, uint16_t length);

#endif // CRC_H
