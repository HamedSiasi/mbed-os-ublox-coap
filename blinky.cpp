/*
 * PackageLicenseDeclared: Apache-2.0
 * Copyright (c) 2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "errno.h"

// ----------------------------------------------------------------
// GENERAL COMPILE-TIME CONSTANTS
// ----------------------------------------------------------------

// Things to do with the processing system
#define SYSTEM_CONTROL_BLOCK_START_ADDRESS ((uint32_t *) 0xe000ed00)
#define SYSTEM_RAM_SIZE_BYTES 16384

// ----------------------------------------------------------------
// TYPES
// ----------------------------------------------------------------

// Tick callback
typedef void (*TickCallback_t)(uint32_t count);

// ----------------------------------------------------------------
// GLOBAL VARIABLES
// ----------------------------------------------------------------

// GPIO to toggle
static DigitalOut gGpio(LED1);

// Flipper to test uS delays
static Ticker gFlipper;

// ----------------------------------------------------------------
// FUNCTION PROTOTYPES
// ----------------------------------------------------------------

static void checkCpu(void);
static size_t checkHeapSize(void);
static uint32_t * checkRam(uint32_t * pMem, size_t memorySize);
static void flip(void);

// ----------------------------------------------------------------
// STATIC FUNCTIONS
// ----------------------------------------------------------------

// Check-out the characteristics of the CPU we're running on
static void checkCpu()
{
    uint32_t x = 0x01234567;

    printf("\n*** Printing stuff of interest about the CPU.\n");
    if ((*(uint8_t *) &x) == 0x67)
    {
        printf("Little endian.\n");
    }
    else
    {
        printf("Big endian.\n");
    }

    // Read the system control block
    // CPU ID register
    printf("CPUID: 0x%08lx.\n", *(SYSTEM_CONTROL_BLOCK_START_ADDRESS));
    // Interrupt control and state register
    printf("ICSR: 0x%08lx.\n", *(SYSTEM_CONTROL_BLOCK_START_ADDRESS + 1));
    // VTOR is not there, skip it
    // Application interrupt and reset control register
    printf("AIRCR: 0x%08lx.\n", *(SYSTEM_CONTROL_BLOCK_START_ADDRESS + 3));
    // SCR is not there, skip it
    // Configuration and control register
    printf("CCR: 0x%08lx.\n", *(SYSTEM_CONTROL_BLOCK_START_ADDRESS + 5));
    // System handler priority register 2
    printf("SHPR2: 0x%08lx.\n", *(SYSTEM_CONTROL_BLOCK_START_ADDRESS + 6));
    // System handler priority register 3
    printf("SHPR3: 0x%08lx.\n", *(SYSTEM_CONTROL_BLOCK_START_ADDRESS + 7));
    // System handler control and status register
    printf("SHCSR: 0x%08lx.\n", *(SYSTEM_CONTROL_BLOCK_START_ADDRESS + 8));

    printf("Last stack entry was at 0x%08lx.\n", (uint32_t) &x);
    printf("A static variable is at 0x%08lx.\n", (uint32_t) &gFlipper);
}

// Check how much RAM can be malloc'ed
static size_t checkHeapSize(void)
{
    int32_t memorySize = SYSTEM_RAM_SIZE_BYTES;
    void * pMem = NULL;

    while ((pMem == NULL) && (memorySize > 0))
    {
        pMem = malloc(memorySize);
        if (pMem == NULL)
        {
            memorySize -= sizeof(uint32_t);
        }
    }

    if (pMem != NULL)
    {
        free(pMem);
    }

    if (memorySize < 0)
    {
        memorySize = 0;
    }

    return (size_t) memorySize;
}

// Check that the given area of RAM is good.
static uint32_t * checkRam(uint32_t *pMem, size_t memorySize)
{
    uint32_t * pLocation = NULL;
    uint32_t value;

    if (pMem != NULL)
    {
        // Write a walking 1 pattern
        value = 1;
        for (pLocation = pMem; pLocation < pMem + memorySize / sizeof (*pLocation); pLocation++)
        {
            *pLocation = value;
            value <<= 1;
            if (value == 0)
            {
                value = 1;
            }
        }

        // Read the walking 1 pattern
        value = 1;
        for (pLocation = pMem; pLocation < pMem + memorySize / sizeof (*pLocation); pLocation++)
        {
            value <<= 1;
            if (value == 0)
            {
                value = 1;
            }
        }

        if (pLocation >= pMem + memorySize / sizeof (uint32_t))
        {
            // Write an inverted walking 1 pattern
            value = 1;
            for (pLocation = pMem; pLocation < pMem + memorySize / sizeof (*pLocation); pLocation++)
            {
                *pLocation = ~value;
                value <<= 1;
                if (value == 0)
                {
                    value = 1;
                }
            }

            // Read the inverted walking 1 pattern
            value = 1;
            for (pLocation = pMem; (pLocation < pMem + memorySize / sizeof (*pLocation)) && (*pLocation == ~value); pLocation++)
            {
                value <<= 1;
                if (value == 0)
                {
                    value = 1;
                }
            }
        }

        if (pLocation >= pMem + memorySize / sizeof (*pLocation))
        {
            pLocation = NULL;
        }
    }

    return pLocation;
}

// Flip
static void flip()
{
    gGpio = !gGpio;
}

// ----------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------

int main(void)
{
    Serial usb (USBTX, USBRX);
    size_t memorySize;
    uint32_t * pMem;
    uint32_t * pRamResult;

    //usb.baud (115200);
    usb.baud (9600);

    checkCpu();

    printf("*** Checking heap size available.\n");
    memorySize = checkHeapSize();

    printf("    %d byte(s) available.\n", memorySize);

    if (memorySize >= sizeof (uint32_t))
    {
        pMem = (uint32_t *) malloc(memorySize);
        printf("*** Checking available heap RAM, from 0x%08lx to 0x%08lx.\n", (uint32_t) pMem, (uint32_t) pMem + memorySize);
        printf("    (the last variable pushed onto the stack is at 0x%08lx, MSP is at 0x%08lx, errno is %d).\n", (uint32_t) &pRamResult, __get_MSP(), errno);
        if (pMem != NULL)
        {
            pRamResult = checkRam(pMem, memorySize);
            if (pRamResult != NULL)
            {
                printf("!!! RAM check failure at location 0x%08lx (contents 0x%08lx).\n", (uint32_t) pRamResult, *pRamResult);
                while(1) {};
            }
        }
        else
        {
            printf("!!! Unable to malloc() %d byte(s).\n", memorySize);
        }
    }

    printf("*** Running us_ticker at 100 usecond intervals for 2 seconds...\n");

    /* Use a usecond delay function to check-out the us_ticker at high speed for a little while */
    gFlipper.attach_us(&flip, 100);

    wait(2);

    gFlipper.attach_us(NULL, 0);

    printf("*** Echoing received characters forever.\n");

    while (1)
    {
        if (usb.readable() && usb.writeable())
        {
            char c = usb.getc();
            usb.putc(c);
        }
    }
    
    return -1;
}
