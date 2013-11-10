##### File Definition ####
PRJ = 2410test
INIT= 2410init
AM1 = 2410slib
AM2 = 2410swis
AM3 = speed2003
AM4 = games
CM1 = 2410lib
CM2 = mmu
CM3 = 2410iis
#CM4 = iis_dual
CM5 = timer
CM6 = 2410RTC
CM7 = 2410IIC
CM8 = 2410int
CM9 = 2410swi
CM10 = 2410etc
CM11 = adc
CM12 = dma
CM13 = dma2
CM14 = eint
CM15 = extdma
CM16 = k9s1208
CM17 = nwait
CM18 = sdi
CM19 = stone
CM20 = ts_auto
CM21 = ts_sep
CM22 = lcdlib
CM23 = lcd
#CM24 = pll
CM25 = power
CM26 = idle
CM27 = PowerOff
CM28 = flash
CM29 = pwr_c
CM30 = etc
CM31 = pd6710
CM32 = glib
CM33 = am29f800
CM34 = palette
CM35 = IrDA
CM36 = uart0
CM37 = uart1
CM38 = uart2
CM39 = spi
CM40 = usbfifo
CM41 = strata32
CM42 = cache
CM43 = cpuspeed
CM44 = game


#### Destination path Definition ####
OBJ=.\obj
ERR=.\err

#### ARM tool Definition ####
ARMLINK= armlink
ARMASM = armasm
ARMCC  = armcc

#### Option Definition ####
LFLAGS = -ro-base 0x30000000 -elf -map -xref \
	 -list list.txt -first $(INIT).o(Init)
AFLAGS = -li -apcs /noswst -cpu ARM920T
CFLAGS = -c -g+ -li -apcs /noswst -cpu ARM920T
#AFLAGS = -bi -apcs /noswst -cpu ARM920T
#CFLAGS = -c -g+ -bi -apcs /noswst -cpu ARM920T


#If you doesn't debug,use following CFLAGS for more faster operation.

#CFLAGS = -c -g- -li -apcs /noswst -cpu ARM920T

#### Object combine Definition ####

OBJS =	$(OBJ)\$(INIT).o $(OBJ)\$(AM1).o $(OBJ)\$(AM2).o $(OBJ)\$(AM3).o $(OBJ)\$(AM4).o \
	$(OBJ)\$(PRJ).o $(OBJ)\$(CM1).o $(OBJ)\$(CM2).o $(OBJ)\$(CM3).o\
	$(OBJ)\$(CM5).o $(OBJ)\$(CM6).o $(OBJ)\$(CM7).o $(OBJ)\$(CM8).o \
	$(OBJ)\$(CM9).o $(OBJ)\$(CM10).o $(OBJ)\$(CM11).o $(OBJ)\$(CM12).o \
	$(OBJ)\$(CM13).o $(OBJ)\$(CM14).o $(OBJ)\$(CM15).o $(OBJ)\$(CM16).o \
	$(OBJ)\$(CM17).o $(OBJ)\$(CM18).o $(OBJ)\$(CM19).o $(OBJ)\$(CM20).o \
	$(OBJ)\$(CM21).o $(OBJ)\$(CM22).o $(OBJ)\$(CM23).o \
	$(OBJ)\$(CM25).o $(OBJ)\$(CM26).o $(OBJ)\$(CM27).o $(OBJ)\$(CM28).o \
	$(OBJ)\$(CM29).o $(OBJ)\$(CM30).o $(OBJ)\$(CM31).o $(OBJ)\$(CM32).o \
	$(OBJ)\$(CM33).o $(OBJ)\$(CM34).o $(OBJ)\$(CM35).o $(OBJ)\$(CM36).o \
	$(OBJ)\$(CM37).o $(OBJ)\$(CM38).o $(OBJ)\$(CM39).o $(OBJ)\$(CM40).o \
	$(OBJ)\$(CM41).o $(OBJ)\$(CM42).o $(OBJ)\$(CM43).o $(OBJ)\$(CM44).o		


all: $(PRJ).axf

clean:
	del $(OBJ)\*.o
	del $(ERR)\*.err
	del $(PRJ).axf
	del $(PRJ).bin
	
$(PRJ).axf: $(OBJS) 
	$(ARMLINK) $(LFLAGS) -o $(PRJ).axf $(OBJS)
	fromelf -nodebug -nozeropad $(PRJ).axf -bin $(PRJ).bin

#	fromelf -nodebug -bin -output $(PRJ).bin $(PRJ).axf
    
#	fromelf  $(PRJ).axf -text/s  syms.sym
#	fromelf  $(PRJ).axf -text/c  symc.sym

#For SDT2.5 fromelf -nodebug -nozeropad $(PRJ).elf -bin $(PRJ).bin 
#For ADS1.0 fromelf -nodebug -bin -output $(PRJ).bin $(PRJ).elf


$(OBJ)\$(PRJ).o :  $(PRJ).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(PRJ).c -o $(OBJ)\$(PRJ).o -Errors $(ERR)\$(PRJ).err
 
$(OBJ)\$(INIT).o:  $(INIT).s  option.inc  memcfg.inc  makefile
	$(ARMASM) $(AFLAGS)  $(INIT).s -o $(OBJ)\$(INIT).o -Errors $(ERR)\$(INIT).err

$(OBJ)\$(AM1).o:  $(AM1).s  makefile
	$(ARMASM) $(AFLAGS)  $(AM1).s -o $(OBJ)\$(AM1).o -Errors $(ERR)\$(AM1).err

$(OBJ)\$(AM2).o:  $(AM2).s  makefile
	$(ARMASM) $(AFLAGS)  $(AM2).s -o $(OBJ)\$(AM2).o -Errors $(ERR)\$(AM2).err

$(OBJ)\$(AM3).o:  $(AM3).s  makefile
	$(ARMASM) $(AFLAGS)  $(AM3).s -o $(OBJ)\$(AM3).o -Errors $(ERR)\$(AM3).err

$(OBJ)\$(AM4).o:  $(AM4).s  makefile
	$(ARMASM) $(AFLAGS)  $(AM4).s -o $(OBJ)\$(AM4).o -Errors $(ERR)\$(AM4).err

$(OBJ)\$(CM1).o:  $(CM1).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM1).c -o $(OBJ)\$(CM1).o -Errors $(ERR)\$(CM1).err

$(OBJ)\$(CM2).o:  $(CM2).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM2).c -o $(OBJ)\$(CM2).o -Errors $(ERR)\$(CM2).err

$(OBJ)\$(CM3).o:  $(CM3).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM3).c -o $(OBJ)\$(CM3).o -Errors $(ERR)\$(CM3).err

#$(OBJ)\$(CM4).o:  $(CM4).c  2410addr.h  2410lib.h  makefile
#	$(ARMCC) $(CFLAGS)  $(CM4).c -o $(OBJ)\$(CM4).o -Errors $(ERR)\$(CM4).err

$(OBJ)\$(CM5).o:  $(CM5).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM5).c -o $(OBJ)\$(CM5).o -Errors $(ERR)\$(CM5).err

$(OBJ)\$(CM6).o:  $(CM6).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM6).c -o $(OBJ)\$(CM6).o -Errors $(ERR)\$(CM6).err

$(OBJ)\$(CM7).o:  $(CM7).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM7).c -o $(OBJ)\$(CM7).o -Errors $(ERR)\$(CM7).err

$(OBJ)\$(CM8).o:  $(CM8).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM8).c -o $(OBJ)\$(CM8).o -Errors $(ERR)\$(CM8).err

$(OBJ)\$(CM9).o:  $(CM9).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM9).c -o $(OBJ)\$(CM9).o -Errors $(ERR)\$(CM9).err

$(OBJ)\$(CM10).o:  $(CM10).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM10).c -o $(OBJ)\$(CM10).o -Errors $(ERR)\$(CM10).err

$(OBJ)\$(CM11).o:  $(CM11).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM11).c -o $(OBJ)\$(CM11).o -Errors $(ERR)\$(CM11).err

$(OBJ)\$(CM12).o:  $(CM12).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM12).c -o $(OBJ)\$(CM12).o -Errors $(ERR)\$(CM12).err

$(OBJ)\$(CM13).o:  $(CM13).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM13).c -o $(OBJ)\$(CM13).o -Errors $(ERR)\$(CM13).err

$(OBJ)\$(CM14).o:  $(CM14).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM14).c -o $(OBJ)\$(CM14).o -Errors $(ERR)\$(CM14).err

$(OBJ)\$(CM15).o:  $(CM15).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM15).c -o $(OBJ)\$(CM15).o -Errors $(ERR)\$(CM15).err

$(OBJ)\$(CM16).o:  $(CM16).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM16).c -o $(OBJ)\$(CM16).o -Errors $(ERR)\$(CM16).err

$(OBJ)\$(CM17).o:  $(CM17).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM17).c -o $(OBJ)\$(CM17).o -Errors $(ERR)\$(CM17).err

$(OBJ)\$(CM18).o:  $(CM18).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM18).c -o $(OBJ)\$(CM18).o -Errors $(ERR)\$(CM18).err

$(OBJ)\$(CM19).o:  $(CM19).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM19).c -o $(OBJ)\$(CM19).o -Errors $(ERR)\$(CM19).err

$(OBJ)\$(CM20).o:  $(CM20).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM20).c -o $(OBJ)\$(CM20).o -Errors $(ERR)\$(CM20).err

$(OBJ)\$(CM21).o:  $(CM21).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM21).c -o $(OBJ)\$(CM21).o -Errors $(ERR)\$(CM21).err

$(OBJ)\$(CM22).o:  $(CM22).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM22).c -o $(OBJ)\$(CM22).o -Errors $(ERR)\$(CM22).err

$(OBJ)\$(CM23).o:  $(CM23).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM23).c -o $(OBJ)\$(CM23).o -Errors $(ERR)\$(CM23).err

#$(OBJ)\$(CM24).o:  $(CM24).c  2410addr.h  2410lib.h  makefile
#	$(ARMCC) $(CFLAGS)  $(CM24).c -o $(OBJ)\$(CM24).o -Errors $(ERR)\$(CM24).err

$(OBJ)\$(CM25).o:  $(CM25).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM25).c -o $(OBJ)\$(CM25).o -Errors $(ERR)\$(CM25).err

$(OBJ)\$(CM26).o:  $(CM26).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM26).c -o $(OBJ)\$(CM26).o -Errors $(ERR)\$(CM26).err

$(OBJ)\$(CM27).o:  $(CM27).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM27).c -o $(OBJ)\$(CM27).o -Errors $(ERR)\$(CM27).err

$(OBJ)\$(CM28).o:  $(CM28).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM28).c -o $(OBJ)\$(CM28).o -Errors $(ERR)\$(CM28).err

$(OBJ)\$(CM29).o:  $(CM29).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM29).c -o $(OBJ)\$(CM29).o -Errors $(ERR)\$(CM29).err

$(OBJ)\$(CM30).o:  $(CM30).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM30).c -o $(OBJ)\$(CM30).o -Errors $(ERR)\$(CM30).err

$(OBJ)\$(CM31).o:  $(CM31).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM31).c -o $(OBJ)\$(CM31).o -Errors $(ERR)\$(CM31).err

$(OBJ)\$(CM32).o:  $(CM32).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM32).c -o $(OBJ)\$(CM32).o -Errors $(ERR)\$(CM32).err

$(OBJ)\$(CM33).o:  $(CM33).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM33).c -o $(OBJ)\$(CM33).o -Errors $(ERR)\$(CM33).err

$(OBJ)\$(CM34).o:  $(CM34).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM34).c -o $(OBJ)\$(CM34).o -Errors $(ERR)\$(CM34).err

$(OBJ)\$(CM35).o:  $(CM35).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM35).c -o $(OBJ)\$(CM35).o -Errors $(ERR)\$(CM35).err

$(OBJ)\$(CM36).o:  $(CM36).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM36).c -o $(OBJ)\$(CM36).o -Errors $(ERR)\$(CM36).err

$(OBJ)\$(CM37).o:  $(CM37).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM37).c -o $(OBJ)\$(CM37).o -Errors $(ERR)\$(CM37).err

$(OBJ)\$(CM38).o:  $(CM38).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM38).c -o $(OBJ)\$(CM38).o -Errors $(ERR)\$(CM38).err

$(OBJ)\$(CM39).o:  $(CM39).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM39).c -o $(OBJ)\$(CM39).o -Errors $(ERR)\$(CM39).err
	
$(OBJ)\$(CM40).o:  $(CM40).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM40).c -o $(OBJ)\$(CM40).o -Errors $(ERR)\$(CM40).err
	
$(OBJ)\$(CM41).o:  $(CM41).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM41).c -o $(OBJ)\$(CM41).o -Errors $(ERR)\$(CM41).err	

$(OBJ)\$(CM42).o:  $(CM42).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM42).c -o $(OBJ)\$(CM42).o -Errors $(ERR)\$(CM42).err
	
$(OBJ)\$(CM43).o:  $(CM43).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM43).c -o $(OBJ)\$(CM43).o -Errors $(ERR)\$(CM43).err
	
$(OBJ)\$(CM44).o:  $(CM44).c  2410addr.h  2410lib.h  makefile
	$(ARMCC) $(CFLAGS)  $(CM44).c -o $(OBJ)\$(CM44).o -Errors $(ERR)\$(CM44).err
