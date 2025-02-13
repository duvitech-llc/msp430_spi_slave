
#include "utils.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Helper function to convert a nibble (4 bits) to its hexadecimal character.
static char nibbleToHex(uint8_t nibble) {
    nibble &= 0x0F;  // Ensure only the lowest 4 bits are used.
    return (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
}

void randomizeData( uint8_t *pBuff, uint16_t buff_size) 
{
  int i = 0;
  memset(pBuff, 0, buff_size);
  for (i = 0; i < buff_size; i++) {
    pBuff[i] = rand() % 256; // Generate a random 8-bit number
  }
}


// Print the contents of a buffer in hexadecimal (each byte printed as two hex digits)
void printBuffer(const uint8_t* buffer, uint16_t size) {
  uint16_t i = 0;
  for (i = 0; i < size; i++) {
      uint8_t byte = buffer[i];
      char high = nibbleToHex(byte >> 4);      // Extract and convert the high nibble.
      char low  = nibbleToHex(byte & 0x0F);      // Extract and convert the low nibble.

      putchar(high);
      putchar(low);
      putchar(' ');  // Print a space between bytes.
        // Insert a newline every 16 bytes.
        if ((i + 1) % 16 == 0) {
            putchar('\r');
            putchar('\n');
        }
  }
  // Print a final newline if the last line isn't complete.
  if (size % 16 != 0) {
    putchar('\r');
    putchar('\n');
  }
}
