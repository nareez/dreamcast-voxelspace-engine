all: rm-elf main.elf

include $(KOS_BASE)/Makefile.rules

export CFLAGS = -std=c99

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
