// Utilitiy functions NB-IoT example application

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

// ----------------------------------------------------------------
// COMPILE-TIME CONSTANTS
// ----------------------------------------------------------------

// ----------------------------------------------------------------
// FUNCTIONS
// ----------------------------------------------------------------

uint32_t bytesToHexString (const char * pInBuf, uint32_t size, char * pOutBuf, uint32_t lenOutBuf);
uint32_t hexStringToBytes (const char * pInBuf, uint32_t lenInBuf, char * pOutBuf, uint32_t lenOutBuf);

#endif

// End Of File
