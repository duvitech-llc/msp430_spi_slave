#ifndef CORE_INC_SPI_COMMON_H_
#define CORE_INC_SPI_COMMON_H_

#include <stdint.h>

// Buffer size definition
#define PACKET_BUFFER_MAX_SIZE  32

#define START_BYTE 0xAA
#define END_BYTE 0xDD

#define DUMMY_BYTE 0xDB
#define SPI_SLAVE_DATA_LENGTH 6
#define SPI_SLAVE_CMD_LENGTH (SPI_SLAVE_DATA_LENGTH + 3)

#define SPI_SLAVE_REPLY_LENGTH 6

typedef enum {
    ACK_READY = 0x70,
    ACK_WAITING_NEXT = 0x71,
    ACK_OK = 0x72,
    ACK_BUSY = 0x73,
    ACK_REPLY_START = 0x74,
    ACK_BAD_CRC = 0x75,
    ACK_ERROR_START = 0x77
} Ack;

typedef enum {
    N6_SPI_MODE_NORMAL = 0x00,
    N6_SPI_MODE_RECEIVING = 0x01,
    N6_SPI_MODE_TRANSMITTING = 0x02,
} N6_SPI_Mode;

// Enum for SPI states
typedef enum {
    N6_SPI_READY = 0x00,
    N6_SPI_READING = 0x01,
    N6_SPI_WRITING = 0x02,
    N6_SPI_BUSY = 0x03,
    N6_SPI_ERROR = 0xFF
} N6_SPI_State;

// Enum for SPI errors
typedef enum {
    N6_SPI_NO_ERROR = 0x00,
    N6_SPI_COMMS_ERROR = 0x01,
    N6_SPI_DATA_NOT_READY = 0x02,
    N6_SPI_INVALID_COMMAND = 0x03,
    N6_SPI_UNKNOWN_ERROR = 0xFF
} N6_SPI_Error;

// SPI Data Structure
// TODO: +1 is for string to have a null terminator
typedef struct {
    N6_SPI_Mode mode;
    N6_SPI_State state;
    N6_SPI_Error error;
    uint8_t txBuffer[PACKET_BUFFER_MAX_SIZE];
    uint8_t pTX;
    uint8_t rxBuffer[PACKET_BUFFER_MAX_SIZE];
    uint8_t pRX;
} N6_SPI_Data;

// SPI Command Enum
typedef enum {
    N6_CMD_READ_STATE = 0x00,
    N6_CMD_READ_ERROR = 0x01,
    N6_CMD_READ_DATA = 0x02,
    N6_CMD_WRITE_DATA = 0x03,
} N6_SPI_Commands;

#endif /* CORE_INC_SPI_COMMON_H_ */
