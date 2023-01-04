#!/bin/sh
dcbuild clean
dcbuild make
dcbuild bin main.elf
dcbuild ip
dcbuild cdi