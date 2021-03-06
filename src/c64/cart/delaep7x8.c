/*
 * delaep7x8.c - Cartridge handling, Dela EP7x8kb cart.
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
#include "cartridge.h"
#include "delaep7x8.h"
#include "types.h"
#include "util.h"

/* This eprom system by DELA seems to be a bit more advanced
   than the EP64 and EP256. It can handle what the EP64 can
   handle, plus the following features :

   - Alternate rom at $8000
   - Alternate basic at $A000
   - Extended basic
   - Basic programs
   - Simulated 16kb roms
   - Kernel replacement

   The system uses 8 8kb eproms, of which the first is used for
   the base menu. 7 8kb banks are used for above named features.

   Because of the fact that this system supports switching in a
   different eprom at $8000 (followed by a reset) it is possible
   to place generic 8kb carts (games/tools) in the eproms and
   use them.

   The bank selecting is done by writing to $DE00. Each low bit is used to
   bank in the respective eprom. If all bits are high then  the  EXROM  is
   switched off.

   The bit values for each eprom bank is:

   eprom bank 1 : 11111110 ($FE) (base eprom)
   eprom bank 2 : 11111101 ($FD)
   eprom bank 3 : 11111011 ($FB)
   eprom bank 4 : 11110111 ($F7)
   eprom bank 5 : 11101111 ($EF)
   eprom bank 6 : 11011111 ($DF)
   eprom bank 7 : 10111111 ($BF)
   eprom bank 8 : 01111111 ($7F)

   EXROM off    : 11111111 ($FF)
*/

/* ---------------------------------------------------------------------*/
static int currbank = 0;

static void REGPARM2 delaep7x8_io1_store(WORD addr, BYTE value)
{
    BYTE bank, config, test_value;

    /* Each bit of the register set to low activates a
       respective EPROM, $FF switches off EXROM */
    config = (value == 0xff) ? 2 : 0;

    cartridge_config_changed(config, config, CMODE_WRITE);

    bank = 0;
    test_value = (~value);
    while (test_value != 0) {
        bank++;
        test_value = (test_value >> 1);
    }
    if (bank != 0) {
        cartridge_romlbank_set(bank - 1);
        currbank = bank - 1;
    }
}

static BYTE REGPARM1 delaep7x8_io1_peek(WORD addr)
{
    return currbank;
}

/* ---------------------------------------------------------------------*/

static io_source_t delaep7x8_device = {
    "DELA EP7x8",
    IO_DETACH_CART,
    NULL,
    0xde00, 0xdeff, 0xff,
    0,
    delaep7x8_io1_store,
    NULL,
    delaep7x8_io1_peek,
    NULL, /* TODO: dump */
    CARTRIDGE_DELA_EP7x8
};

static io_source_list_t *delaep7x8_list_item = NULL;

static const c64export_resource_t export_res = {
    "Dela EP7x8", 1, 0, &delaep7x8_device, NULL, CARTRIDGE_DELA_EP7x8
};

/* ---------------------------------------------------------------------*/

void delaep7x8_config_init(void)
{
    cartridge_config_changed(0, 0, CMODE_READ);
    cartridge_romlbank_set(0);
}

/* FIXME: should copy rawcart to roml_banks ! */
void delaep7x8_config_setup(BYTE *rawcart)
{
    cartridge_config_changed(0, 0, CMODE_READ);
    cartridge_romlbank_set(0);
}

/* ---------------------------------------------------------------------*/
static int delaep7x8_common_attach(void)
{
    if (c64export_add(&export_res) < 0) {
        return -1;
    }
    delaep7x8_list_item = c64io_register(&delaep7x8_device);
    return 0;
}

/* FIXME: this function should setup rawcart instead of copying to roml_banks ! */
/* FIXME: handle the various combinations / possible file lengths */
int delaep7x8_bin_attach(const char *filename, BYTE *rawcart)
{
    if (util_file_load(filename, roml_banks, 0x2000, UTIL_FILE_LOAD_SKIP_ADDRESS) < 0) {
        return -1;
    }
    return delaep7x8_common_attach();
}

/* FIXME: this function should setup rawcart instead of copying to roml_banks ! */
int delaep7x8_crt_attach(FILE *fd, BYTE *rawcart)
{
    WORD chip;
    WORD size;
    BYTE chipheader[0x10];

    memset(roml_banks, 0xff, 0x10000);

    while (1) {
        if (fread(chipheader, 0x10, 1, fd) < 1) {
            break;
        }

        chip = (chipheader[0x0a] << 8) + chipheader[0x0b];
        size = (chipheader[0x0e] << 8) + chipheader[0x0f];

        if (size != 0x2000) {
            return -1;
        }

        if (chip > 7) {
            return -1;
        }

        if (fread(roml_banks + (chip<<13), 0x2000, 1, fd)<1) {
            return -1;
        }
    }
    return delaep7x8_common_attach();
}

void delaep7x8_detach(void)
{
    c64export_remove(&export_res);
    c64io_unregister(delaep7x8_list_item);
    delaep7x8_list_item = NULL;
}
