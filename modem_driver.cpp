// NB-IoT modem driver for NB-IoT example application

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "ublox/utilities.h"
#include "ublox/serial_driver.h"
#include "ublox/modem_driver.h"

// ----------------------------------------------------------------
// COMPILE-TIME CONSTANTS
// ----------------------------------------------------------------

// At the end of all AT strings there is a...
#define AT_TERMINATOR "\r\n"

// Wait between polling the NB-IoT module AT interface
#define AT_RX_POLL_TIMER_MS 100

// OK
#define AT_OK "OK\r\n"

// ERROR
#define AT_ERROR "ERROR\r\n"

Timer timer;
// ----------------------------------------------------------------
// PROTECTED FUNCTIONS
// ----------------------------------------------------------------


// variadicFunction
bool Nbiot::sendPrintf(const char * pFormat, ...)
{
	bool success = false;
    va_list args;
    uint32_t len = 0;
    if (gInitialised)
    {
        va_start(args, pFormat);
        len = vsnprintf(gTxBuf, sizeof(gTxBuf), pFormat, args);
        va_end(args);
        success = gpSerialPort->transmitBuffer((const char *) gTxBuf, len);
    }
    return success;
}

// Get characters (up to lenBuf of them) from the NB-IoT module into pBuf.
// If an AT terminator is found, or lenBuf characters have been read,
// return a count of the number of characters (including the AT terminator),
// otherwise return 0.
uint32_t Nbiot::getLine(char * pBuf, uint32_t lenBuf)
{
    int32_t x;
    uint32_t returnLen = 0;
    if (gInitialised)
    {
        if (gLenRx < lenBuf)
        {
            do{
                x = gpSerialPort->receiveChar();
                if (x >= 0)
                {
                    *(pBuf + gLenRx) = (char) x;
                    gLenRx++;

                    if (x == AT_TERMINATOR[gMatched])
                    {
                        gMatched++;
                    }
                    else
                    {
                        gMatched = 0;
                    }
                }
            } while ((x >= 0) && (gLenRx < lenBuf) && (gMatched < sizeof(AT_TERMINATOR) - 1));        
        }
        if ((gMatched == sizeof(AT_TERMINATOR) - 1) || (gLenRx == lenBuf)) // -1 to omit 0 of string
        {
            returnLen = gLenRx;
            gLenRx = 0;
            gMatched = 0;
        }    
    }
    return returnLen;
}

// Callback to handle AT stuff received from the NBIoT module
void Nbiot::rxTick()
{
    uint32_t len = getLine (gRxBuf, sizeof (gRxBuf));
    if (len > sizeof(AT_TERMINATOR) - 1)
    {
        printf ("(%d bytes) \"%.*s\".\r\n", (int) (len - (sizeof(AT_TERMINATOR) - 1)), (int) (len - (sizeof(AT_TERMINATOR) - 1)), gRxBuf);
        if (gpResponse == NULL)
        {
           gLenResponse = len;
           gpResponse = gRxBuf;
        }
    }
}


Nbiot::AtResponse Nbiot::waitResponse(
		const char  *pExpected        /*NULL*/,
		time_t       timeoutSeconds   /*DEFAULT_RESPONSE_TIMEOUT_SECONDS*/,
		char        *pResponseBuf     /*NULL*/,
		uint32_t     responseBufLen   /*0*/)
{
	AtResponse response = AT_RESPONSE_NONE;
	timer.reset();
	timer.start();

    if (gpResponse != NULL)
    {
        printf ("[modem->waitResponse]  ERROR: response from module not cleared from last time.\r\n");
        gpResponse = NULL;
    }
    do {
        rxTick();
        if (gpResponse != NULL)
        {
            if ((strncmp(gpResponse, AT_OK, gLenResponse) == 0) && (gLenResponse == (sizeof (AT_OK) - 1))) // -1 to omit 0 of string
            {
                response = AT_RESPONSE_OK;
            }
            else if ((strncmp(gpResponse, AT_ERROR, gLenResponse) == 0) && (gLenResponse == (sizeof (AT_ERROR) - 1))) // -1 to omit 0 of string
            {
                response = AT_RESPONSE_ERROR;
            }
            else if ((pExpected != NULL) && (gLenResponse >= strlen (pExpected)) && (strcmp(gpResponse, pExpected) >= 0))
            {
                response = AT_RESPONSE_STARTS_AS_EXPECTED;
                if (pResponseBuf != NULL)
                {
                    if (gLenResponse > responseBufLen - 1)
                    {
                        gLenResponse = responseBufLen - 1;
                    }
                    memcpy (pResponseBuf, gpResponse, gLenResponse);
                    pResponseBuf[gLenResponse] = 0;
                }
            }
            else
            {
                if (pExpected != NULL)
                {
                    printf ("[modem->waitResponse]  WARNING: unexpected response from module.\n");
                    printf ("[modem->waitResponse]  Expected: %s... Received: %.*s \r\n", pExpected, (int) gLenResponse, gpResponse);
                }
                gpResponse = NULL;
            }
        }
        if (gpResponse == NULL)
        {
            wait_ms(AT_RX_POLL_TIMER_MS);
        }
    } while ( (response == AT_RESPONSE_NONE) && ((timeoutSeconds == 0) || (timer.read() < timeoutSeconds)) );

    gpResponse = NULL;
    return response;
}

void Nbiot::charToTchar(const char *pIn, char *pOut, uint32_t size)
{
    memset (pOut, 0, size);
#ifdef _MSC_VER
    uint32_t size_needed = MultiByteToWideChar(CP_UTF8, 0, pIn, (int)strlen(pIn), NULL, 0);
    if (size_needed > size)
    {
        size_needed = size;
    }
    MultiByteToWideChar(CP_UTF8, 0, pIn, (int)strlen(pIn), pOut, size_needed);
#else
    strncpy(pOut, pIn, size);
#endif
}


// Constructor
Nbiot::Nbiot(const char * pPortname)
{
    gpResponse   = NULL;
    gpSerialPort = NULL;
    gLenResponse = 0;
    gMatched = 0;
    gLenRx = 0;
    gInitialised = false;

    gpSerialPort = new SerialPort();
    if (gpSerialPort)
    {
        if (gpSerialPort->connect(pPortname))
        {
            gInitialised = true;
        }
        else
        {
            delete gpSerialPort;
            printf ("[modem->Constructor]  unable to connect to port %s. \r\n", pPortname);
        }
    }
}

Nbiot::~Nbiot (){
	delete (gpSerialPort);
}

// Connect to the network
bool Nbiot::connect(bool usingSoftRadio, time_t timeoutSeconds)
{
    bool success = false;
    AtResponse response;
    timer.reset();
    timer.start();
    if (gInitialised)
    {
        if (timeoutSeconds > 0)
        {
            //printf ("Checking for connection to network for up to %d seconds...\r\n", (int) timeoutSeconds);
        }
        else
        {
            printf ("Checking for connection to network...\r\n");
        }

        do {
            if (usingSoftRadio)
            {
                sendPrintf("AT+RAS%s", AT_TERMINATOR);
                response = waitResponse("+RAS:CONNECTED\r\n");
            }
            else
            {
                sendPrintf("AT+NAS%s", AT_TERMINATOR);
                response = waitResponse("+NAS: Connected (activated)\r\n");
            }

            if (response == AT_RESPONSE_STARTS_AS_EXPECTED)
            {
                waitResponse();
                sendPrintf("AT+SMI=1%s", AT_TERMINATOR);
                response = waitResponse("+SMI:OK\r\n");
                if (response == AT_RESPONSE_STARTS_AS_EXPECTED)
                {
                    waitResponse();
                    success = true;
                }
            }
            else
            {
                wait_ms(1000);
            }
        } while ((!success) && ((timeoutSeconds == 0) || (timer.read() < timeoutSeconds)));
    }

    return success;
}

// Send a message to the network
bool Nbiot::send (char * pMsg, uint32_t msgSize, time_t timeoutSeconds)
{
    bool success = false;
    AtResponse response;
    uint32_t   charCount = 0;

    if ((msgSize * 2) <= sizeof(gHexBuf))
    {
    	printf("\n <--- TX (%d bytes):  %.*d\r\n\n", (int) msgSize, (int) msgSize, pMsg);
        charCount = bytesToHexString (pMsg, msgSize, gHexBuf, sizeof(gHexBuf));
        sendPrintf("AT+MGS=%d, %.*s%s", msgSize, charCount, gHexBuf, AT_TERMINATOR);

        response = waitResponse("+MGS:OK\r\n");
        if (response == AT_RESPONSE_STARTS_AS_EXPECTED)
        {
            waitResponse();
            response = waitResponse("+SMI:SENT\r\n", timeoutSeconds);
            if (response == AT_RESPONSE_STARTS_AS_EXPECTED)
            {
                success = true;
            }
        }
    }
    else
    {
        printf ("[modem->send]  datagram is too long (%d characters when only %d bytes can be sent).\r\n", msgSize, (int) (sizeof (gHexBuf) / 2));
    }
    return success;
}

// Receive a message from the network
uint32_t Nbiot::receive (char * pMsg, uint32_t msgSize, time_t timeoutSeconds)
{
    int bytesReceived = 0;
    AtResponse response;
    char * pHexStart = NULL;
    char * pHexEnd = NULL;

    sendPrintf("AT+MGR%s", AT_TERMINATOR);
    response = waitResponse("+MGR:", timeoutSeconds, gHexBuf, sizeof (gHexBuf));
    if (response == AT_RESPONSE_STARTS_AS_EXPECTED)
    {
        if (sscanf(gHexBuf, " +MGR:%d,", &bytesReceived) == 1)
        {
            pHexStart = strchr (gHexBuf, ',');
            if (pHexStart != NULL)
            {
                pHexStart++;
                pHexEnd = strstr (pHexStart, AT_TERMINATOR);
                if ((pHexEnd != NULL) && (pMsg != NULL))
                {
                    hexStringToBytes (pHexStart, pHexEnd - pHexStart, pMsg, bytesReceived/*(uint32_t)msgSize*/);
                }
            }
            response = waitResponse("+MGR:OK\r\n");
        }
    }
    return (uint32_t) bytesReceived;
}

// End Of File
