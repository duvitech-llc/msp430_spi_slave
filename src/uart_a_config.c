#include "uart_a_config.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>


void uint16ToHexStr(uint16_t value, char* hexStr, size_t hexStrSize) {
    int i = 0;
    if (hexStr == 0 || hexStrSize < 5) {
        // Handle error, the output buffer is either not provided or too small.
        return;
    }

    const char hexDigits[] = "0123456789ABCDEF";

    // Convert each nibble to a hexadecimal character, starting with the most significant nibble
    for (i = 3; i >= 0; --i) {
        hexStr[3 - i] = hexDigits[(value >> (i * 4)) & 0x0F];
    }

    // Null-terminate the string
    hexStr[4] = '\0';
}


void uart_a_init(void)
{

    /* Setup the IO pins for the UART */
    P4SEL0 |= BIT2 | BIT3;
    P4SEL1 &= ~(BIT2 | BIT3);

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings.
    PM5CTL0 &= ~LOCKLPM5;

    /* Disable UART for Configuration */
    UCA1CTLW0 = UCSWRST;
    UCA1CTLW0 |= UCSSEL__SMCLK; // Use 24MHz SMCLK
    UCA1CTLW0 &= ~(UCPEN | UCMSB | UC7BIT | UCSPB); // 8N1, No Parity

    /* 115200 Baud at 24MHz */
    UCA1BRW = 13;  // Clock prescaler
    UCA1MCTLW = UCOS16 | (37 << 8) | (0 << 4); // Modulation settings

    UCA1CTLW0 &= ~UCSWRST; // Enable UART
                                              
    UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt
    UCA1IE |= UCTXIE;                         // Enable USCI_A1 TX interrupt
}

int putchar( int ch)
{

    // Load the data onto the buffer
    UCA1TXBUF = (uint8_t)ch;
    // Wait for the transmit buffer to be ready
    while (!(UCA1IFG & UCTXIFG));
    return ch;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A1_VECTOR)))
#endif
void EUSCI_A1_ISR(void)
{
    switch(__even_in_range(UCA1IV,USCI_UART_UCTXCPTIFG))
    {
        case USCI_NONE: break;
        case USCI_UART_UCRXIFG:
            break;
       case USCI_UART_UCTXIFG:
           UCA1IE &= ~UCTXIE;
           break;
       case USCI_UART_UCSTTIFG: break;
       case USCI_UART_UCTXCPTIFG: break;
    }
}
