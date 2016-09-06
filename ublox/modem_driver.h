// NB-IoT modem driver for NB-IoT example application

#ifndef _MODEM_DRIVER_H_
#define _MODEM_DRIVER_H_

#include "ublox/serial_driver.h"

// ----------------------------------------------------------------
// COMPILE-TIME CONSTANTS
// ----------------------------------------------------------------

// 	Maximum length of a string to be sent with the send() function
#define MAX_LEN_SEND_STRING 128

// 	The default interrupt buffer for received data
#define DEFAULT_RX_INT_STORAGE 128

// 	Default timeout when connecting to the network
#define DEFAULT_CONNECT_TIMEOUT_SECONDS 5

// 	Default timeout when sending a message to the network
#define DEFAULT_SEND_TIMEOUT_SECONDS 5

// 	Default timeout when receiving a message from the network
#define DEFAULT_RECEIVE_TIMEOUT_SECONDS 5

// 	Default timeout when waiting for a response from the CIoT modem
#define DEFAULT_RESPONSE_TIMEOUT_SECONDS 5

// 	Default timeout when flushing the modem at the outset
#define DEFAULT_FLUSH_TIMEOUT_SECONDS 1




// ----------------------------------------------------------------
// CLASSES
// ----------------------------------------------------------------

// Nbiot interface
class Nbiot
{
public:
    // Constructor.  pPortname is a string that defines the serial port where the
    // NB-IoT modem is connected.  On Windows the form of a properly escaped string
    // must be as follows:
    //
    // "\\\\.\\COMx"
    //
    // ...where x is replaced by the COM port number.  So, for COM17, the string
    // would be:
    //
    // "\\\\.\\COM17"    
    Nbiot (const char * pPortname = "\\\\.\\COM10");
    ~Nbiot ();


    // Connect to the NB-IoT network with optional timeoutSeconds.  If usingSoftRadio
    // is true then the connect behaviour is matched to that of SoftRadio, otherwise
    // it is matched to that of a real radio.  If timeoutSeconds is zero this function
    // will block indefinitely until a connection has been achieved.
    bool connect (
    		bool    usingSoftRadio = false,
			time_t  timeoutSeconds = DEFAULT_CONNECT_TIMEOUT_SECONDS);

    

    // Send the contents of the buffer pMsg, length msgSize, to the NB-IoT network with
    // optional timeoutSeconds, waiting for confirmation that the message has been sent.
    // If timeoutSeconds is zero this function will block indefinitely until the message
    // has been sent.
    bool send (
    		char     *pMsg,
			uint32_t  msgSize,
			time_t    timeoutSeconds = DEFAULT_SEND_TIMEOUT_SECONDS);

    

    // Poll the NB-IoT modem for received data with optional timeoutSeconds.  If data
    // has been received the return value will be non-zero, representing the number of
    // bytes received.  Up to msgSize bytes of returned data will be stored at pMsg; any
    // data beyond that will be lost.  If timeoutSeconds is zero this function will block
    // indefinitely until a message has been received.
    uint32_t receive (
    		char      *pMsg,
			uint32_t   msgSize,
			time_t     timeoutSeconds = DEFAULT_RECEIVE_TIMEOUT_SECONDS);


protected:
    typedef enum
    {
        AT_RESPONSE_NONE,
        AT_RESPONSE_STARTS_AS_EXPECTED,
        AT_RESPONSE_OK,
        AT_RESPONSE_ERROR,
        AT_RESPONSE_OTHER
    } AtResponse;


    // Intermediate buffer used during hex string converstion.
    char gHexBuf[MAX_LEN_SEND_STRING];
    

    // Intermediate buffer for storing the hex version of AT strings
    // to be transmitted.
    char gTxBuf[MAX_LEN_SEND_STRING];
    

    // Intermediate buffer for storing the hex version of AT strings
    // received.
    char gRxBuf[DEFAULT_RX_INT_STORAGE];
    

    // Length of the data stored in rxBuf
    uint32_t gLenRx;
    

    // Pointer to a response string from the modem, used during
    // transmit operations.
    const char * gpResponse;
    

    // Length of the string pointed to by pResponse.
    uint32_t gLenResponse;
    

    // Index into matching progress for the AT string terminator.
    uint8_t gMatched;
    

    // Pointer to serial port instance.
    SerialPort *gpSerialPort = NULL;
    

    // Flag to indicate that this driver has been succesfully initialised.
    bool gInitialised;


    // Send a string, printf()-style to the serial port
    bool sendPrintf (const char * pFormat, ...);

    
    // Check the modem interface for received characters, copying each one into
    // pBuf.  If an AT_TERMINATOR is found, or lenBuf characters have been
    // received, return the number of characters (including the AT_TERMINATOR),
    // otherwise return 0.  No NULL terminator is added to pBuf.
    uint32_t getLine (char * pBuf, uint32_t lenBuf);

    
    // Tick along the process of receiving characters from the modem AT interface.
    void rxTick();
    
    // Wait for a response from the modem, used during transmit operations.
    // If pExpected is not NULL, AtResponse will indicate if the received string
    // starts with the characters at pExpected (which must be a NULL terminated
    // string), otherwise it will indicate if the standard strings "OK" and
    // "ERROR" have been received, otherwise it will indicate that something
    // other has been received. Up to responseBufLen received characters are
    // stored at pResponseBuf and a NULL terminator is added.  The number of
    // includes the AT terminator (AT_TERMINATOR).  Any characters over responseBufLen
    // are discarded.
    AtResponse waitResponse (
    		const char  *pExpected       = NULL,
    		time_t       timeoutSeconds  = DEFAULT_RESPONSE_TIMEOUT_SECONDS,
            char        *pResponseBuf    = NULL,
			uint32_t     responseBufLen  = 0);



    void charToTchar(
    		const char *pIn,
			char       *pOut,
			uint32_t    size);
};


#endif

// End Of File
