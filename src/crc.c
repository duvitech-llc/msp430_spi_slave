#include "crc.h"
#include "msp430fr2355.h"
#include <msp430.h>
#include <stdint.h>

#define CRC16_POLY 0x1021
#define CRC16_INITIAL 0xFFFF

uint16_t crc16(const uint8_t *data, uint16_t length) {
    uint16_t crc = CRC16_INITIAL;
    uint16_t i;
    uint8_t bit;

    for ( i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ CRC16_POLY;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

uint16_t crc16_ccitt_hw(const uint8_t *data, uint16_t length) {
    int16_t i;
    CRCINIRES = CRC16_INITIAL; // Initialize with correct value
    
    for ( i = 0; i < length; i++) {
        CRCDIRB_L  = data[i];
    }
    
    return CRCINIRES;
}


