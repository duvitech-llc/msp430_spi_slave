
#include <msp430.h>

#include "clock_config.h"
#include "common.h"
#include "gpio_config.h"
#include "spi_slave_config.h"
#include "timer_b_config.h"
#include "uart_a_config.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

volatile bool send_receive_slave = false;

volatile uint16_t bFlag = 1;
static unsigned int count = 0;

int main(void) {
  WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
  clock_init();
  gpio_init();
  uart_a_init();
  spi_slave_Init();  

  // Set up Timer_A (TA0) using ACLK in continuous mode.
  // ACLK is typically sourced from a 32.768kHz crystal or the internal VLO.
  TB0CTL = TBSSEL__ACLK | MC__CONTINUOUS;

  bFlag = 1;  
  // need to init spi master also conflict with spi master peripheral
  printf("\033cMSP430 Slave\r\n");
  
  __delay_cycles(240000); // allow printf output

  __delay_cycles(2400); // small delay to let the pin stabilize
  __bis_SR_register(GIE); // Enter LPM0, enable interrupts

  // 500ms interval in ticks (ACLK @ 32768Hz)
  const uint16_t interval = 16384;
  uint16_t lastTick = TB0R;

  
  while (1) {    
    uint16_t currentTick = TB0R;

    // Unsigned arithmetic correctly handles timer rollover.
    if ((uint16_t)(currentTick - lastTick) >= interval) {
      P6OUT ^= BIT6;          // Toggle LED on P6.6
      lastTick = currentTick; // Reset the timer reference point
    }

  }
}


