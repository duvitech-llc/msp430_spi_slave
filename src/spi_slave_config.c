/* spi_slave.c
 *
 *  Created on: Dec 3, 2023
 *      Author: gvigelet
 */

#include "common.h"
#include "sys/cdefs.h"
#include "spi_slave_config.h"
#include <stdio.h>
#include <string.h>

#define SS_PIN BIT4

volatile uint8_t spiTxBuffer[SPI_SLAVE_REPLY_LENGTH] = {0};
volatile uint8_t spiRxBuffer[SPI_SLAVE_CMD_LENGTH] = {0};
volatile uint8_t spiRxIndex = 0;
volatile uint8_t spiTxIndex = 0;

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
  pSpiSend = 0;
  memset((void *)spiBuffer, 0, PACKET_BUFFER_MAX_SIZE);

  UCA0CTLW0 &= ~UCSWRST;  // Release reset
  
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch (__even_in_range(UCA0IV, 4))
    {
        case USCI_NONE:
            break; // No interrupt
        case USCI_SPI_UCRXIFG:
            spiBuffer[pSpiSend++] = UCA0RXBUF;
            if (pSpiSend >= PACKET_BUFFER_MAX_SIZE) {
                UCA0IE &= ~UCRXIE;  // Disable further RX interrupts.
                // Fill in temperature data into the last two bytes.
                //spiBuffer[PACKET_BUFFER_MAX_SIZE-2] = (uint8_t)(rawTemp >> 8);  // High byte
                //spiBuffer[PACKET_BUFFER_MAX_SIZE-1] = (uint8_t)(rawTemp & 0xFF);  // Low byte
                pSpiSend = 0;
                
                UCA0IE |= UCTXIE;   // Enable TX interrupt for transmission.
            }
            break;
        case USCI_SPI_UCTXIFG:
            if (pSpiSend < PACKET_BUFFER_MAX_SIZE) {
                UCA0TXBUF = spiBuffer[pSpiSend++];
            } else {
                UCA0IE &= ~UCTXIE;  // Disable TX interrupts.
                memset((void *)spiBuffer, 0, PACKET_BUFFER_MAX_SIZE);
                pSpiSend = 0;
                // Preload TXBUF with a default value for the next transaction.
                UCA0TXBUF = 0x00;  // Set this to whatever default is appropriate.
                
                UCA0IFG &= ~UCRXIFG; // Clear stale
                UCA0IE |= UCRXIE;   // Re-enable RX interrupts for the next packet.
            }
            break;
        default:
            break;
    }
}
