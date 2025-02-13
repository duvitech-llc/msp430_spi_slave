#include "clock_config.h"

void clock_init(void)
{
    // Configure FRAM wait states for 24MHz operation
    FRCTL0 = FRCTLPW | NWAITS_2;

    // Configure XT1 pins
    P2SEL1 |= BIT6 | BIT7;  // P2.6 and P2.7 as crystal pins

    // Clear oscillator fault flags
    do
    {
        CSCTL7 &= ~(XT1OFFG | DCOFFG);
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG);

    // Configure DCO for 24MHz
    __bis_SR_register(SCG0);  // Disable FLL
    CSCTL3 |= SELREF__XT1CLK; // Set XT1 as FLL reference
    CSCTL0 = 0;               // Clear DCO and MOD registers
    CSCTL1 |= DCORSEL_7;      // Set DCO = 24MHz
    CSCTL2 = FLLD_0 + 731;    // Set FLL multiplier for 24MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);  // Enable FLL

    // Wait for FLL lock
    while (CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));

    // Set clock sources
    CSCTL4 = SELMS__DCOCLKDIV | SELA__XT1CLK; // MCLK & SMCLK = DCO, ACLK = XT1
}
