# MCUXpresso SDK : memfault

## Overview
This folder is for MCUXpresso SDK memfault delivery. This repository is part of the MCUXpresso SDK overall delivery which is composed of several sub-repositories/projects. Navigate to the top/parent repository (mcuxsdk-manifests) for the complete delivery of MCUXpresso SDK.

## Documentation
MCUXpresso SDK overall details can be reviewed here: [MCUXpresso SDK Online Documentation](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/introduction/README.html)

Memfault can be used for device debugging, products analytics and so on.
Visit [memfault website](https://memfault.com) for more defails.

### MCUXpresso SDK Integration
The integration code is in the folder *sdk_port*. Currently only flash backend is implemented.
The dumped data is saved in flash, it can be used for later analysis.

There is project shows how to use memfault in SDK, the project is in the folder
*mcuxsdk/mcuxsdk/examples/component_examples/memfault*

## Setup
Instructions on how to install the MCUXpresso SDK provided from GitHub via west manifest
[Getting Started with SDK - Detailed Installation Instructions](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/gsd/installation.html#installation)
