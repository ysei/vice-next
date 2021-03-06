/*
 * capture.c - Cartridge handling, Capture cart.
 *
 * Written by
 *  Groepaz <groepaz@gmx.net>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <stdio.h>
#include <string.h>

#include "c64cart.h"
#include "c64cartmem.h"
#include "c64export.h"
#include "c64io.h"
#include "c64mem.h"
#include "c64meminit.h"
#include "c64memrom.h"
#include "c64tpi.h"
#include "capture.h"
#include "cartridge.h"
#include "machine.h"
#include "maincpu.h"
#include "types.h"
#include "util.h"

/*
    Jason Ranheim "Capture" Cartridge

    - 8K ROM (mapped to e000)
    - 8K RAM (mapped to 6000)

    7474  - 2 D-Flipflops (cart_enabled, register_enabled)
    74125 - 4 Tri-State Buffers
    74133 - NAND with 13 Inputs (adress decoder)

    the cartridge is disabled after a reset.

    when the freeze button is pressed the following happens:
    - an NMI is generated
    - as soon as the current adress is in bank 0xfe the cart switches
      to ultimax mode. the cart rom then contains one page full of
      "jmp $eaea", which ultimatively calls the freezer code.
    - the fff7/fff8 "register" logic is enabled

    after that any access (read or write) to $fff7 will turn the cart_enabled
    off (leave ultimax mode), and an access to $fff8 will turn the cart back
    on (enter ultimax mode). the "register logic" that causes this can only
    be disabled again by a hardware reset.
*/

/*
... program is loaded to $0800, then:

.C:fad3   A0 00      LDY #$00
.C:fad5   B9 00 08   LDA $0800,Y
.C:fad8   C0 03      CPY #$03
.C:fada   B0 05      BCS $FAE1

.C:fadc   D9 F8 FF   CMP $FFF8,Y  ; "4a 59 43" "JYC"
.C:fadf   D0 0C      BNE $FAED

.C:fae1   59 00 FE   EOR $FE00,Y
.C:fae4   99 00 08   STA $0800,Y
.C:fae7   C8         INY
.C:fae8   D0 EB      BNE $FAD5
.C:faea   6C 04 08   JMP ($0804)
.C:faed   60         RTS
*/

/* #define DBGCAPTURE */

#ifdef DBGCAPTURE
#define DBG(x) printf x
#else
#define DBG(x)
#endif

static const c64export_resource_t export_res = {
    "Capture", 1, 1, NULL, NULL, CARTRIDGE_CAPTURE
};

static unsigned int cart_enabled = 0;
static unsigned int freeze_pressed = 0;
static unsigned int register_enabled = 0;
static unsigned int romh_enabled = 0;

void capture_reg(WORD addr)
{
    if (register_enabled) {
        if ((addr & 0xffff) == 0xfff7) {
            cart_enabled = 0;
            DBG(("CAPTURE: enable: %d\n", cart_enabled));
        } else if ((addr & 0xffff) == 0xfff8) {
            cart_enabled = 1;
            DBG(("CAPTURE: enable: %d\n", cart_enabled));
        } else if ((addr & 0xffff) == 0xfff9) {
            if ((!freeze_pressed) && (!romh_enabled)) {
                /* HACK: this one is needed to survive the ram clearing loop */
                cart_enabled = 0;
                DBG(("CAPTURE: enable: %d\n", cart_enabled));
            }
        }
    }
}

void capture_romhflip(WORD addr)
{
    if (freeze_pressed) {
        if ((addr & 0xff00) == 0xfe00) {
            freeze_pressed = 0;
            romh_enabled = 1;
            DBG(("CAPTURE: romh enable: %d\n", romh_enabled));
        }
    }
}

BYTE REGPARM1 capture_romh_read(WORD addr)
{
    capture_reg(addr);
    capture_romhflip(addr);

    if (cart_enabled) {
        if (romh_enabled) {
            return romh_banks[(addr & 0x1fff)];
        }
    }
    return mem_read_without_ultimax(addr);
}

void REGPARM2 capture_romh_store(WORD addr, BYTE value)
{
    capture_reg(addr);
    /* capture_romhflip(addr); */
    if (cart_enabled == 0) {
        mem_store_without_ultimax(addr, value);
    }
}

/*
    there is Cartridge RAM at 0x6000..0x7fff
*/
BYTE REGPARM1 capture_1000_7fff_read(WORD addr)
{
    if (cart_enabled) {
        if (addr>=0x6000) {
            return export_ram0[addr-0x6000];
        }
    }

    return mem_read_without_ultimax(addr);
}

void REGPARM2 capture_1000_7fff_store(WORD addr, BYTE value)
{
    if (cart_enabled) {
        if (addr>=0x6000) {
            export_ram0[addr-0x6000] = value;
        }
    } else {
        mem_store_without_ultimax(addr, value);
    }
}

/******************************************************************************/

void capture_freeze(void)
{
    DBG(("CAPTURE: freeze\n"));
    if (freeze_pressed == 0) {
        cartridge_config_changed(2, 3, CMODE_READ | CMODE_RELEASE_FREEZE);
        cart_enabled = 1;
        freeze_pressed = 1;
        register_enabled = 1;
        romh_enabled = 0;
    }
}

void capture_config_init(void)
{
    DBG(("CAPTURE: config init\n"));
    cartridge_config_changed(2, 2, CMODE_READ);
}

void capture_reset(void)
{
    DBG(("CAPTURE: reset\n"));
    cart_enabled = 0;
    register_enabled = 0;
    freeze_pressed = 0;
    cartridge_config_changed(2, 2, CMODE_READ);
}

void capture_config_setup(BYTE *rawcart)
{
    DBG(("CAPTURE: config setup\n"));
    memcpy(romh_banks, rawcart, 0x2000);
    memset(export_ram0, 0, 0x2000);
    cartridge_config_changed(2, 2, CMODE_READ);
}

static int capture_common_attach(void)
{
    DBG(("CAPTURE: attach\n"));
    if (c64export_add(&export_res) < 0) {
        return -1;
    }
    return 0;
}

int capture_bin_attach(const char *filename, BYTE *rawcart)
{
    if (util_file_load(filename, rawcart, 0x2000, UTIL_FILE_LOAD_SKIP_ADDRESS) < 0) {
        return -1;
    }
    return capture_common_attach();
}

int capture_crt_attach(FILE *fd, BYTE *rawcart)
{
    BYTE chipheader[0x10];

    if (fread(chipheader, 0x10, 1, fd) < 1) {
        return -1;
    }

    if (fread(rawcart, 0x2000, 1, fd) < 1) {
        return -1;
    }

    return capture_common_attach();
}

void capture_detach(void)
{
    c64export_remove(&export_res);
}
