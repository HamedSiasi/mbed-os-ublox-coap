// Serial port driver for NB-IoT example application

#ifndef _SERIAL_DRIVER_H_
#define _SERIAL_DRIVER_H_
#include "stdint.h"
#include "stdio.h"
#include "mbed.h"

// ----------------------------------------------------------------
// CLASSES
// ----------------------------------------------------------------

// Serial port interface
class SerialPort {
public:

    SerialPort(PinName tx    =  UART1_TX,
    		   PinName rx    =  UART1_RX,
			   int baudrate  =  9600);
    ~SerialPort();


    bool connect(const char * pPortName);


    // Disconnect from the current serial port.
    void disconnect(void);

    // Transmit lenBuf characters from pBuf over the serial port.
    // Returns TRUE on success, otherwise FALSE.
    bool transmitBuffer(const char * pBuf, uint32_t lenBuf);
    
    // Receive up to lenBuf characters into pBuf over the serial port.
    // Returns the number of characters received.
    uint32_t receiveBuffer(char * pBuf, uint32_t lenBuf);
    
    // Receive a single character from the serial port.
    // Returns -1 if there are no characters, otherwise it
    // returns the character (i.e. it can be cast to char).
    int32_t receiveChar();
    
    // Clear the serial port buffers, both transmit and receive.
    void clear();


protected:
    Serial *pgUart = NULL;
    static void uartCallBack(void);


};

#endif

// End Of File
