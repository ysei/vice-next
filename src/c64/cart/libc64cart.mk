
include common.mk

PPU_INCDIRS 	+= 	-I./c64/cart/

#PPU_SRCS	= 	c64/cart/actionreplay3.c c64/cart/actionreplay4.c c64/cart/actionreplay.c c64/cart/atomicpower.c c64/cart/c64cart.c c64/cart/c64cartmem.c c64/cart/comal80.c c64/cart/crt.c c64/cart/delaep256.c c64/cart/delaep64.c c64/cart/delaep7x8.c c64/cart/epyxfastload.c c64/cart/expert.c c64/cart/final.c c64/cart/generic.c c64/cart/ide64.c c64/cart/kcs.c c64/cart/magicformel.c c64/cart/mikroass.c c64/cart/retroreplay.c c64/cart/rexep256.c c64/cart/ross.c c64/cart/stardos.c c64/cart/stb.c c64/cart/supergames.c c64/cart/supersnapshot.c  c64/cart/zaxxon.c
#c64/cart/spi-sdcard.c



PPU_SRCS	=	c64/cart/actionreplay2.c c64/cart/c64-midi.c c64/cart/digimax.c c64/cart/finalplus.c c64/cart/ide64.c c64/cart/mmcreplay.c c64/cart/ser-eeprom.c c64/cart/supergames.c c64/cart/actionreplay3.c c64/cart/c64tpi.c c64/cart/dinamic.c c64/cart/fmopl.c c64/cart/isepic.c c64/cart/ocean.c c64/cart/sfx_soundexpander.c c64/cart/supersnapshot4.c c64/cart/actionreplay4.c c64/cart/capture.c c64/cart/dqbb.c c64/cart/freezeframe.c c64/cart/kcs.c c64/cart/prophet64.c c64/cart/sfx_soundsampler.c c64/cart/supersnapshot.c c64/cart/actionreplay.c c64/cart/comal80.c c64/cart/easyflash.c c64/cart/freezemachine.c c64/cart/mach5.c c64/cart/ramcart.c c64/cart/simonsbasic.c c64/cart/tfe.c c64/cart/atomicpower.c c64/cart/crt.c c64/cart/epyxfastload.c c64/cart/funplay.c c64/cart/magicdesk.c c64/cart/retroreplay.c c64/cart/snapshot64.c c64/cart/warpspeed.c c64/cart/c64acia1.c c64/cart/delaep256.c c64/cart/exos.c c64/cart/gamekiller.c c64/cart/magicformel.c c64/cart/reu.c c64/cart/spi-sdcard.c c64/cart/westermann.c c64/cart/c64cart.c c64/cart/delaep64.c c64/cart/expert.c c64/cart/generic.c c64/cart/magicvoice.c c64/cart/rexep256.c c64/cart/stardos.c c64/cart/zaxxon.c c64/cart/c64carthooks.c c64/cart/delaep7x8.c c64/cart/final3.c c64/cart/georam.c c64/cart/mikroass.c c64/cart/rexutility.c c64/cart/stb.c c64/cart/c64cartmem.c c64/cart/diashowmaker.c c64/cart/final.c c64/cart/gs.c c64/cart/mmc64.c c64/cart/ross.c c64/cart/superexplode5.c 





PPU_LIB_TARGET	=	libc64cart.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
