// Serial port driver for NB-IoT example application

#include <stdint.h>
#include <stdio.h>
#include "mbed.h"
#include "ublox/serial_driver.h"


// Constructor.
SerialPort::SerialPort(PinName tx/*UART1_TX*/,  PinName rx/*UART1_RX*/,  int baudrate/*9600*/)
{
    pgUart = new Serial(tx, rx);
	pgUart->baud(baudrate);
	pgUart->format(8, SerialBase::None, 1);
}

// Destructor.
SerialPort::~SerialPort()
{
	delete (pgUart);
}

// CallBack function to call whenever a serial interrupt is generated.
void SerialPort::uartCallBack(void)
{
}

// Disconnect from the port.
void SerialPort::disconnect(void)
{
}

void SerialPort::clear()
{
}

// Make a connection to a named port.
bool SerialPort::connect(const char * pPortName)
{
    bool success = true;
    return success;
}


// Send lenBuf bytes from pBuf over the serial port, returning true
// in the case of success.
bool SerialPort::transmitBuffer(const char *pBuf, uint32_t lenBuf)
{
	unsigned long result = 0;
    if(pgUart->writeable())
    {
    	result = pgUart->printf(pBuf);
        if (!result)
        {
            printf ("[serial->transmitBuffer]  Transmit failed !!! \r\n");
        }
    }
    return (bool) result;
}


// Get up to lenBuf bytes into pBuf from the serial port,
// returning the number of characters actually read.
uint32_t SerialPort::receiveBuffer (char *pBuf, uint32_t lenBuf)
{
	unsigned long result = 0;
    //if(pgUart->readable())
    //{
    result = pgUart->scanf("%s",pBuf);
    if (!result)
    {
    	printf ("[serial->receiveBuffer] Receive failed !!!.\r\n");
    }
    //}
    return (uint32_t) result;
}

// Read a single character from the serial port, returning
// -1 if no character is read.
int32_t SerialPort::receiveChar()
{
	int32_t returnChar = -1;
	//if( pgUart->readable() )
    //{
    returnChar = (int32_t) pgUart->getc();
    printf("%c", returnChar);
    //}
    return (int32_t)returnChar;
}









