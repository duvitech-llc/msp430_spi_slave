#include "gpio_config.h"



void gpio_init() {
    //Set P6.3 and P6.6 to output direction
    P6DIR |= BIT6;
    P6OUT |= BIT6;
}
