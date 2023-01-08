#!/bin/sh
./dcbuild9.sh clean
./dcbuild9.sh make
./dcbuild9.sh bin main.elf
./dcbuild9.sh ip
./dcbuild9.sh cdi