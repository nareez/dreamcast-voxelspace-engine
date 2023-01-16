all: rm-elf main.elf

include $(KOS_BASE)/Makefile.rules

CFLAGS = -std=c99

#KOS_CFLAGS=-O3 -ffast-math -fomit-frame-pointer -ml -m4-single-only -ffunction-sections -fdata-sections  -I/opt/toolchains/dc/kos/include -I/opt/toolchains/dc/kos/kernel/arch/dreamcast/include -I/opt/toolchains/dc/kos/addons/include -I/opt/toolchains/dc/kos/../kos-ports/include -I/opt/toolchains/dc/kos/include -I/opt/toolchains/dc/kos/kernel/arch/dreamcast/include -I/opt/toolchains/dc/kos/addons/include -I/opt/toolchains/dc/kos/../kos-ports/include -D_arch_dreamcast -D_arch_sub_pristine -Wall -fno-builtin -ml -m4-single-only -ffunction-sections -fdata-sections

OBJS = main.o display.o
	
clean:
	-rm -f main.elf $(OBJS)
	-rm -f romdisk_boot.*
	
clean-all:
	-rm -f main.elf $(OBJS) main.iso output.bin Program.cdi 1st_read.bin
	-rm -f romdisk_boot.*

dist:
	-rm -f $(OBJS)
	$(KOS_STRIP) main.elf
	
rm-elf:
	-rm -f main.elf
	-rm -f romdisk_boot.*

main.elf: $(OBJS) romdisk_boot.o
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $@ $(KOS_START) $^ -lm $(KOS_LIBS)  

romdisk_boot.img:
	$(KOS_GENROMFS) -f $@ -d romdisk_boot -v

romdisk_boot.o: romdisk_boot.img
	$(KOS_BASE)/utils/bin2o/bin2o $< romdisk_boot $@
