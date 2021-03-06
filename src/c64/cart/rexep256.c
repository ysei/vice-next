/*
 * rexep256.c - Cartridge handling, REX EP256 cart.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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
#include <stdlib.h>
#include <string.h>

#include "c64cart.h"
#include "c64cartmem.h"
#include "c64export.h"
#include "c64io.h"
#include "c64mem.h"
#include "cartridge.h"
#include "rexep256.h"
#include "types.h"
#include "util.h"

/* This eprom system by REX is similair to the EP64. It can handle
   what the EP64 can handle, plus the following features :

   - Alternate rom at $8000
   - 8kb or 16kb or 32kb eprom support

   The system uses 9 eproms, of which the first (8kb eprom) is used
   for the main menu. 8 eprom banks, populated with either an 8kb,
   16kb or 32kb eprom, are used for above named features.

   When a 16kb or 32kb is used only 8kb blocks of it can be switched.

   Because of the fact that this system supports switching in a
   different eprom at $8000 (followed by a reset) it is possible
   to place other 8kb carts in the eproms and use them.

   The bank selecting is done by  writing  to  $DFA0:

   bit   meaning
   ---   -------
    0    socket selection bit 0
    1    socket selection bit 1
    2    socket selection bit 2
    3    unused
    4    8kb bank selection bit 0
    5    8kb bank selection bit 1
   6-7   unused

    Reading from $DFC0 switches off the EXROM.

    Reading from $DFE0 switches on the EXROM.
*/

/* ---------------------------------------------------------------------*/

static WORD rexep256_eprom[8];
static BYTE rexep256_eprom_roml_bank_offset[8];

/* ---------------------------------------------------------------------*/

static void REGPARM2 rexep256_io2_store(WORD addr, BYTE value)
{
    BYTE eprom_bank, test_value, eprom_part = 0;

    if (addr == 0xdfa0) {
        eprom_bank = (value & 0xf);
        if (eprom_bank > 7) {
            return;
        }

        test_value = (value & 0xf0) >> 4;
        if (test_value > 3) {
            return;
        }

        if (rexep256_eprom[eprom_bank] == 0x2000) {
            eprom_part = 0;
        }
        if (rexep256_eprom[eprom_bank] == 0x4000) {
            eprom_part = test_value & 1;
        }
        if (rexep256_eprom[eprom_bank] == 0x8000) {
            eprom_part = test_value;
        }

        cartridge_romlbank_set(rexep256_eprom_roml_bank_offset[eprom_bank]+eprom_part+1);
    }
}

/* I'm unsure whether the register is write-only,
   but in this case it is assumed to be. */
static BYTE REGPARM1 rexep256_io2_read(WORD addr)
{
    if (addr == 0xdfc0) {
        export.exrom = 0;
        mem_pla_config_changed();
    }
    if (addr == 0xdfe0) {
        export.exrom = 1;
        mem_pla_config_changed();
    }
    return 0;
}

static BYTE REGPARM1 rexep256_io2_peek(WORD addr)
{
    return 0;
}

/* ---------------------------------------------------------------------*/

static io_source_t rexep256_device = {
    "REX EP256",
    IO_DETACH_CART,
    NULL,
    0xdf00, 0xdfff, 0xff,
    0, /* read is never valid */
    rexep256_io2_store,
    rexep256_io2_read,
    rexep256_io2_peek,
    NULL, /* TODO: dump */
    CARTRIDGE_REX_EP256
};

static io_source_list_t *rexep256_list_item = NULL;

static const c64export_resource_t export_res = {
    "REX EP256", 1, 0, NULL, &rexep256_device, CARTRIDGE_REX_EP256
};

/* ---------------------------------------------------------------------*/

void rexep256_config_init(void)
{
    cartridge_config_changed(0, 0, CMODE_READ);
    cartridge_romlbank_set(0);
}

/* FIXME: should copy rawcart to roml_banks ! */
void rexep256_config_setup(BYTE *rawcart)
{
    cartridge_config_changed(0, 0, CMODE_READ);
    cartridge_romlbank_set(0);
}

/* ---------------------------------------------------------------------*/
static int rexep256_common_attach(void)
{
    if (c64export_add(&export_res) < 0) {
        return -1;
    }
    rexep256_list_item = c64io_register(&rexep256_device);
    return 0;
}

/* FIXME: this function should setup rawcart instead of copying to roml_banks ! */
/* FIXME: handle the various combinations / possible file lengths */
int rexep256_bin_attach(const char *filename, BYTE *rawcart)
{
    if (util_file_load(filename, roml_banks, 0x2000, UTIL_FILE_LOAD_SKIP_ADDRESS) < 0) {
        return -1;
    }
    return rexep256_common_attach();
}

/* FIXME: this function should setup rawcart instead of copying to roml_banks ! */
int rexep256_crt_attach(FILE *fd, BYTE *rawcart)
{
    WORD chip;
    WORD size;
    BYTE chipheader[0x10];
    int rexep256_total_size = 0;
    int i;

    memset(roml_banks, 0xff, 0x42000);

    for (i = 0; i < 8; i++) {
        rexep256_eprom[i] = 0x2000;
        rexep256_eprom_roml_bank_offset[i] = 0x1f;
    }

    if (fread(chipheader, 0x10, 1, fd) < 1) {
        return -1;
    }

    chip = (chipheader[0x0a] << 8) + chipheader[0x0b];
    size = (chipheader[0x0e] << 8) + chipheader[0x0f];

    if (size != 0x2000) {
        return -1;
    }

    if (fread(roml_banks, 0x2000, 1, fd) < 1) {
        return -1;
    }

    while (1) {
        if (fread(chipheader, 0x10, 1, fd) < 1) {
            break;
        }

        chip = (chipheader[0x0a] << 8) + chipheader[0x0b];
        size = (chipheader[0x0e] << 8) + chipheader[0x0f];

        if (size != 0x2000 && size != 0x4000 && size != 0x8000) {
            return -1;
        }

        if (chip > 8) {
            return -1;
        }

        rexep256_eprom[chip - 1] = size;
        rexep256_eprom_roml_bank_offset[chip - 1] = rexep256_total_size >> 13;

        if (fread(roml_banks + 0x2000 + rexep256_total_size, size, 1, fd) < 1) {
            return -1;
        }

        rexep256_total_size=rexep256_total_size+size;
    }
    return rexep256_common_attach();
}

void rexep256_detach(void)
{
    c64export_remove(&export_res);
    c64io_unregister(rexep256_list_item);
    rexep256_list_item = NULL;
}
