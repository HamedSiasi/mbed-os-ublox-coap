#include "mbed.h"

#include "ns_types.h"
#include "sn_nsdl.h"
#include "sn_coap_header.h"
#include "sn_coap_protocol.h"
#include "sn_nsdl_lib.h"


#include "ublox/modem_driver.h"
#include <stdlib.h>
#include <string.h>

#define DESTINATION "coap.me:5683"
#define TOKEN "12345"

static uint8_t*  gMsgPacket;
static uint32_t  gMsgPacketSize;
static int counter;
static uint8_t readingData;


static void* myMalloc(uint16_t size)
{
	return malloc(size);
}

static void myFree(void* addr)
{
    if( addr )
    {
        free(addr);
    }
}

static uint8_t tx_callback(uint8_t *a, uint16_t b, sn_nsdl_addr_s *c, void *d)
{
    return 0;
}

static int8_t rx_callback(sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c)
{
	return 0;
}


static bool softRadio(void){

	bool status = false;
	bool usingSoftRadio = true;

	Nbiot *pModem = NULL;
	if( !(pModem = new Nbiot()) )
	{
		printf ("[blinky->softRadio]  Out of Memory !!! \r\n");
	}
	else
	{
		status = pModem->connect(usingSoftRadio);
		if (status)
		{
			status = pModem->send ( (char*)gMsgPacket, (uint32_t)gMsgPacketSize);
			if(status)
			{
				char reply[128];
				uint32_t replySize;

				replySize = (uint32_t)pModem->receive ( reply, (uint32_t)128 );
				if (replySize > 0)
				{
					printf ("\n ---> RX (%d bytes): \"%.*s\" \r\n\n", (int)replySize, (int)replySize, reply);
				}
			}
			else
			{
				printf ("[blinky->softRadio]  Failed to send datagram !!!\r\n");
			}
		}
		else
		{
			printf ("[blinky->softRadio]  Failed to connect to the network !!! \r\n");
		}
	}
	delete (pModem);
	return status;
}


static bool coapMsgBuild(
		uint8_t              msgPayload,
		uint16_t             msgPayloadSize,
		sn_coap_msg_type_e   msgType,
		sn_coap_msg_code_e   msgCode)
{
	bool status = false;
	struct coap_s *handle = sn_coap_protocol_init(myMalloc, myFree, tx_callback, rx_callback);
	if(handle)
	{
		// (2 arg)
		sn_nsdl_addr_s dst_addr_ptr;
		memset(&dst_addr_ptr, 0, sizeof(sn_nsdl_addr_s));
		uint8_t temp_addr[4] = {0};
		dst_addr_ptr.addr_ptr = temp_addr;
		dst_addr_ptr.addr_len = (uint8_t)sizeof(temp_addr);
		dst_addr_ptr.type = SN_NSDL_ADDRESS_TYPE_IPV4;

		// (3 arg)
		uint8_t dst_packet_data_ptr[32];

		// (4 arg)
		sn_coap_hdr_s hdr;
		memset(&hdr, 0, sizeof(sn_coap_hdr_s));
		hdr.msg_id = counter++;
		hdr.payload_ptr = (uint8_t*)malloc(8);
		memset(hdr.payload_ptr, msgPayload, 8);
		hdr.payload_len = msgPayloadSize;
		hdr.msg_type = msgType;
		hdr.msg_code = msgCode;

		//Here are most often used Options
		char *myToken = TOKEN;
		hdr.token_ptr = (uint8_t*)myToken;
		hdr.token_len = strlen(myToken);

		char *destination = DESTINATION;
		hdr.uri_path_len = (uint16_t)strlen(destination);
		hdr.uri_path_ptr = (uint8_t*)destination;


		int16_t  msgPacketBytes = sn_coap_protocol_build(handle, &dst_addr_ptr, dst_packet_data_ptr, &hdr, NULL);
		if( (int)msgPacketBytes == -1)
		{
			 printf("[blinky->msgCoAP] Failure in CoAP header structure\r\n");
		}
		else if( (int)msgPacketBytes == -2)
		{
			 printf("[blinky->msgCoAP] Failure in given pointer (= NULL)\r\n");
		}
		else if( (int)msgPacketBytes == -3)
		{
			 printf("[blinky->msgCoAP] Failure in Reset message\r\n");
		}
		else
		{
			 gMsgPacket     = (uint8_t*) dst_packet_data_ptr;
			 gMsgPacketSize = (uint32_t) msgPacketBytes;
			 status = true;
		}
		sn_coap_protocol_destroy(handle);
	}
	return status;
}

static bool coapMsg(void)
{
	bool status = false;
	if(coapMsgBuild((uint8_t) ++ readingData,(uint16_t)sizeof(readingData),COAP_MSG_TYPE_CONFIRMABLE/*CoapMessageType*/,COAP_MSG_CODE_REQUEST_POST/*CoapMessageCode*/))
	{
		if(softRadio())
		{
			status = true;
		}
	}
	return status;
}

int main(void){
int x = 0;
    while (true)
    {
    	printf("*** %d HERE\r\n", x);

    	coapMsg();
    	wait_ms(10000);

    	printf("*** %d AND HERE\r\n", x);
    	x++;
    }
    return -1;
}
