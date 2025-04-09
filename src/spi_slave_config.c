/* spi_slave.c
 *
 *  Created on: Dec 3, 2023
 *      Author: gvigelet
 */

#include "common.h"
#include "sys/cdefs.h"
#include "spi_slave_config.h"
#include "crc.h"
#include <stdio.h>
#include <string.h>

#define SS_PIN BIT4

volatile uint8_t spiTxBuffer[SPI_SLAVE_REPLY_LENGTH] = {0};
volatile uint8_t spiRxBuffer[SPI_SLAVE_CMD_LENGTH] = {0};
volatile uint8_t spiRxIndex = 0;
volatile uint8_t spiTxIndex = 0;

static void execute_command(uint8_t cmd, const uint8_t* data)
{
    printf("Execute command: %d\r\n", cmd);
}

void spi_slave_Init()
{

  P1SEL0 |= BIT5 | BIT6 | BIT7;      // set 4-SPI pin as second function

  // Configure Slave Select pin
  P1DIR &= ~SS_PIN;       // Input
  P1REN |= SS_PIN;        // Enable pull-up
  P1OUT |= SS_PIN;

  UCA0CTLW0 |= UCSWRST;                     // **Put state machine in reset**
                                            // 4-pin, 8-bit SPI slave
  UCA0CTLW0 |= UCSYNC | UCCKPL | UCSTEM; // Synchronous mode, CPOL=1, CPHA=0
                                            // Clock polarity high, MSB

  UCA0CTLW0 |= UCSSEL__SMCLK; // Use SMCLK as clock source
  UCA0CTLW0 &= ~UCMST;        // Set as SPI slave

  UCA0TXBUF = 0x00;                         // set to ready (use start packet byte)
  UCA0CTLW0 &= ~UCSWRST;  // Release reset  
}

void spi_slave_poll_blocking(void) {
    int i = 0;

    // Step 1: Wait for CS to go low (master asserts)
    while (P1IN & SS_PIN);  // Wait until SS is LOW (active)

    spiRxIndex = 0;
    spiTxIndex = 0;

    // Step 2: Receive full SPI_SLAVE_CMD_LENGTH
    while (spiRxIndex < SPI_SLAVE_CMD_LENGTH) {
        while (!(UCA0IFG & UCRXIFG));                 // Wait for RX byte
        spiRxBuffer[spiRxIndex++] = UCA0RXBUF;

        while (!(UCA0IFG & UCTXIFG));                 // Wait for TX ready
        UCA0TXBUF = ACK_BUSY;                         // Send BUSY until response is ready
    }

    // Step 3: Check CRC
    uint16_t received_crc = (spiRxBuffer[SPI_SLAVE_CMD_LENGTH - 1] << 8) |
                             spiRxBuffer[SPI_SLAVE_CMD_LENGTH - 2];
    uint16_t calc_crc = crc16_ccitt_hw((uint8_t *)spiRxBuffer, SPI_SLAVE_CMD_LENGTH - 2);

    if (received_crc != calc_crc) {
        spiTxBuffer[0] = ACK_BAD_CRC;
        for (i = 1; i < SPI_SLAVE_REPLY_LENGTH; i++) spiTxBuffer[i] = 0x00;
    } else {
        // Step 4: Process command
        execute_command((uint8_t)spiRxBuffer[0], (uint8_t *)&spiRxBuffer[1]);
        // The reply is assumed to be filled into spiTxBuffer
    }

    // Step 5: Wait for CS to go high then low again for transmit phase
    while (!(P1IN & SS_PIN));  // Wait for CS HIGH
    while (P1IN & SS_PIN);     // Wait for CS LOW again

    spiTxIndex = 0;

    // Step 6: Transmit SPI_SLAVE_REPLY_LENGTH bytes
    while (spiTxIndex < SPI_SLAVE_REPLY_LENGTH) {
        while (!(UCA0IFG & UCRXIFG));                 // Wait for dummy byte from master
        volatile uint8_t dummy = UCA0RXBUF;

        while (!(UCA0IFG & UCTXIFG));
        UCA0TXBUF = spiTxBuffer[spiTxIndex++];
    }

    // Final state: optional reset
    spiRxIndex = 0;
    spiTxIndex = 0;
}
