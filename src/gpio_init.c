#include "gpio_config.h"



void gpio_init() {
    //Set P6.3 and P6.6 to output direction
    P6DIR |= BIT6;
    P6OUT |= BIT6;

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings.
    PM5CTL0 &= ~LOCKLPM5;
}
