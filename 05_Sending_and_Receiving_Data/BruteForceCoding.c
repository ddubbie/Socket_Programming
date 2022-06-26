#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

const uint8_t val8 = 101;
const uint16_t val16 = 10001;
const uint32_t val32 = 100000001;
const uint64_t val64 = 1000000000001L;
const int MESSAGELENGTH = sizeof(uint8_t) + sizeof(uint16_t) + 
                            sizeof(uint32_t)+ sizeof(uint64_t);

static char stringBuf[8192];

char *BytesToDecString(uint8_t *byteArray, int arrayLenght)
{
    // TODO
}

// Warning: Untested preconditions (e.g., 0 <= size <= 8)
int EncodeIntBigEndian(uint8_t dst[], uint64_t val, int offset, int size)
{
    // TODO
}

// Warning: Untested preconditions (e.g., 0 <= size <= 8)
uint64_t DecodeIntBigEndian(uint8_t val[], int offset, int size)
{
    // TODO
}

int
main(int argc, char *argv[])
{
    
}