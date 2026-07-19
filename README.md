# HypervWOL

A native C++ Windows Service for managing Wake-on-LAN functionality for Hyper-V virtual machines.

## This project is currently under development

This is developed with the "it works approach".

This will change in the future but for now it is a simple service that does what it is supposed to do.

In this phase of development no care is taken for backwards compatibility.
It is not likely but posible that you have to reconfigure your service after each update.

## Features

- Start any Hyper-V virtual machine with a static MAC address using Wake-on-LAN Packets
- Can service multiple interfaces and ports
- Console mode for testing and debugging
- Runs as a Windows background service
- AGPLv3 licensed, See [LICENSE.txt](LICENSE.txt) for more details

## Prerequisites

- Windows Server 2016+ or Windows 10+
- Administrator privileges for service installation and running console mode

## Install
- From Binary Release

see [INSTALL.md](INSTALL.md) for pre-built binary installation instructions.

- From Source
 
see [INSTALLFROMSOURCE.md](INSTALLFROMSOURCE.md) for building and installing from source instructions.

## Support Me
If you find this project useful, please consider supporting it 
by buying me a coffee: [Buy Me a Coffee](https://ko-fi.com/motwok)
