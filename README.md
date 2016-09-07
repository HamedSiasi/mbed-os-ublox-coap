# mbed-os: u-blox Test Application for CoAP Protocol

This repo contains a mini application that pulls together:

- u-blox integration of the mbed 5 (https://github.com/u-blox/mbed-os-ublox-app)
- CoAP stack which works with SoftRadio (running on host) via UART

finally builds something runnable(approximate immage size 55k) which send data to the golang Broker/Router server
(https://github.com/HamedSiasi/go-router-server)

# Prerequisites
To fetch and build the code in this repository you need first to install the [mbed CLI tools](https://github.com/ARMmbed/mbed-cli#installation) and their prerequisites.

# Building This Code
(1) Clone this repo (`git clone https://github.com/HamedSiasi/mbed-os-ublox-coap`)

(2) Enter to the repo directory (`cd mbed-os-ublox-coap`)

(3) `add` necessary libraries:
`mbed add https://github.com/ARMmbed/mbed-client-c`
`mbed add https://github.com/ARMmbed/mbed-trace`
`mbed add https://github.com/ARMmbed/nanostack-libservice`
don't clone directly the libraries use `mbed add`.

(4) Set the target (`mbed target SARA_NBIOT_EVK`)
(5) Set the toolchain (`mbed toolchain GCC_ARM`)

(6) `mbed update`
This will fetch the latest code from https://github.com/ARMmbed/mbed-ublox

(7) Once this is done, build the code with:
`mbed compile`

You will find the output files (bin,elf,..) in the sub-directory `.build\SARA_NBIOT_EVK\GCC_ARM\`.


