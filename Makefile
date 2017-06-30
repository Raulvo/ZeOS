################################
##############ZeOS #############
################################
########## Makefile ############
################################

# package dev86 is required (Old Debian distros)
# package bin86 is required (Newer Debian distros)
AS86	= as86 -0 -a
LD86	= ld86 -0

# ARCH flags -> flags needed by the host to compile the guest OS architecture
HOSTARCHFLAGS = -m32 -march=i386 -fno-PIC
ASARCHFLAG = --32 -march=i386

HOSTCFLAGS = -Wall -Wstrict-prototypes -g
HOSTCC 	= gcc
CC      = gcc
CPP	= cpp
AS      = as
LD      = ld
OBJCOPY = objcopy -O binary -R .note -R .comment -S

INCLUDEDIR = include

# Define here flags to compile the tests if needed
JP = 

CFLAGS = $(HOSTARCHFLAGS) -O2 -g -ffreestanding $(JP) -Wall -I$(INCLUDEDIR)
CPPFLAGS = $(HOSTARCHFLAGS) -O2 -I$(INCLUDEDIR)
ASMFLAGS = -I$(INCLUDEDIR) 
LINKFLAGS = -m elf_i386

SYSOBJ = interrupt.o entry.o sys_call_table.o io.o sched.o sys.o mm.o devices.o utils.o hardware.o
LIBZEOS = -L . -l zeos

#add to USROBJ the object files required to complete the user program
USROBJ = libc.o # libjp.a

all:zeos.bin

zeos.bin: bootsect system build user
	$(OBJCOPY) system system.out
	$(OBJCOPY) user user.out
	./build bootsect system.out user.out > zeos.bin

build: build.c
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

bootsect.s: bootsect.S
	$(CPP) $(CPPFLAGS) -traditional-cpp $< -o $@

bootsect.o: bootsect.s
	$(AS86) -o $@ $<

bootsect: bootsect.o
	$(LD86) -s -o $@ $<

entry.s: entry.S $(INCLUDEDIR)/asm.h $(INCLUDEDIR)/segment.h
	$(CPP) $(CFLAGS) -o $@ $<

sys_call_table.s: sys_call_table.S $(INCLUDEDIR)/asm.h $(INCLUDEDIR)/segment.h
	$(CPP) $(CFLAGS) -o $@ $<

entry.o: entry.s
	$(AS) $(ASARCHFLAG) $(ASMFLAGS) -o $@ $<

sys_call_table.o: sys_call_table.s
	$(AS) $(ASARCHFLAG) $(ASMFLAGS) -o $@ $<

user.o:user.c $(INCLUDEDIR)/libc.h
	$(CC) $(CFLAGS) -c -o $@ $<

interrupt.o:interrupt.c $(INCLUDEDIR)/interrupt.h $(INCLUDEDIR)/segment.h $(INCLUDEDIR)/types.h
	$(CC) $(CFLAGS) -c -o $@ $<

io.o:io.c $(INCLUDEDIR)/io.h
	$(CC) $(CFLAGS) -c -o $@ $<

sched.o:sched.c $(INCLUDEDIR)/sched.h
	$(CC) $(CFLAGS) -c -o $@ $<

libc.o:libc.c $(INCLUDEDIR)/libc.h
	$(CC) $(CFLAGS) -c -o $@ $<

mm.o:mm.c $(INCLUDEDIR)/types.h $(INCLUDEDIR)/mm.h
	$(CC) $(CFLAGS) -c -o $@ $<

sys.o:sys.c $(INCLUDEDIR)/devices.h
	$(CC) $(CFLAGS) -c -o $@ $<

utils.o:utils.c $(INCLUDEDIR)/utils.h
	$(CC) $(CFLAGS) -c -o $@ $<


system.o:system.c $(INCLUDEDIR)/hardware.h $(INCLUDEDIR)/segment.h $(INCLUDEDIR)/types.h $(INCLUDEDIR)/interrupt.h $(INCLUDEDIR)/system.h $(INCLUDEDIR)/sched.h $(INCLUDEDIR)/mm.h $(INCLUDEDIR)/io.h $(INCLUDEDIR)/mm_address.h 
	$(CC) $(CFLAGS) -c -o $@ $<

system: system.o system.lds $(SYSOBJ)
	$(LD) $(LINKFLAGS) -T system.lds -o $@ $< $(SYSOBJ) $(LIBZEOS)

user: user.o user.lds $(USROBJ) 
	$(LD) $(LINKFLAGS) -T user.lds -o $@ $< $(USROBJ)

clean:
	rm -f *.o *.s bochsout.txt parport.out system.out system bootsect zeos.bin user user.out *~ build 

disk: zeos.bin
	dd if=zeos.bin of=/dev/fd0

emul: zeos.bin
	bochs -q -f .bochsrc

emuldbg: zeos.bin
	bochs-nogdb -q -f .bochsrc-nogdb
