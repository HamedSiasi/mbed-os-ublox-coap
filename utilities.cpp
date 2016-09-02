// Utilitiy functions NB-IoT example application

#include <stdint.h>
#include "ublox/utilities.h"

// ----------------------------------------------------------------
// PRIVATE VARIABLES
// ----------------------------------------------------------------

static const char hexTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

// ----------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------

// Convert a hex string of a given length into a sequence of bytes, returning the
// number of bytes written.
uint32_t hexStringToBytes (const char * pInBuf, uint32_t lenInBuf, char * pOutBuf, uint32_t lenOutBuf)
{
    uint32_t x;
    uint32_t y = 0;
    int32_t z;
    uint32_t a = 0;

    for (x = 0; (x < lenInBuf) && (y < lenOutBuf); x++)
    {
        z = *(pInBuf + x);
        if ((z >= '0') && (z <= '9'))
        {
            z = z - '0';
        }
        else
        {
            z &= ~0x20;
            if ((z >= 'A') && (z <= 'F'))
            {
                z = z - 'A' + 10;
            }
            else
            {
                z = -1;
            }
        }

        if (z >= 0)
        {
            if (a % 2 == 0)
            {
                *(pOutBuf + y) = (z << 4) & 0xF0;
            }
            else
            {
                *(pOutBuf + y) += z;
                y++;
            }
            a++;
        }
    }

    return y;
}

// Convert a sequence of bytes into a hex string, returning the number
// of characters written. The hex string is NOT null terminated.
uint32_t bytesToHexString (const char * pInBuf, uint32_t size, char * pOutBuf, uint32_t lenOutBuf)
{
    uint32_t x = 0;
    uint32_t y = 0;

    for (x = 0; (x < size) && (y < lenOutBuf); x++)
    {
        pOutBuf[y] = hexTable[(pInBuf[x] >> 4) & 0x0f]; // upper nibble
        y++;
        if (y < lenOutBuf)
        {
            pOutBuf[y] = hexTable[pInBuf[x] & 0x0f]; // lower nibble
            y++;
        }
    }

    return y;
}

// End Of File
