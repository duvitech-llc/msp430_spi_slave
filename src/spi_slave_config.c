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

volatile uint8_t spiBuffer[PACKET_BUFFER_MAX_SIZE];
volatile unsigned int pSpiSend;

void spi_slave_Init()
{

  P1SEL0 |= BIT4 | BIT5 | BIT6 | BIT7;      // set 4-SPI pin as second function

  UCA0CTLW0 |= UCSWRST;                     // **Put state machine in reset**
                                            // 4-pin, 8-bit SPI slave
  UCA0CTLW0 |= UCSYNC | UCCKPL | UCSTEM; // Synchronous mode, CPOL=1, CPHA=0
                                            // Clock polarity high, MSB

  UCA0CTLW0 |= UCSSEL__SMCLK; // Use SMCLK as clock source
  UCA0CTLW0 &= ~UCMST;        // Set as SPI slave
  
  PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings
  UCA0TXBUF = 0x00;                         // set to ready (use start packet byte)
  pSpiSend = 0;
  memset((void *)spiBuffer, 0, PACKET_BUFFER_MAX_SIZE);
  UCA0CTLW0 &= ~UCSWRST;                    // **Initialize USCI state machine**
  UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

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
                
                UCA0CTLW0 |= UCSWRST;
                UCA0CTLW0 &= ~UCSWRST;

                UCA0IE |= UCRXIE;   // Re-enable RX interrupts for the next packet.
            }
            break;
        default:
            break;
    }
}
