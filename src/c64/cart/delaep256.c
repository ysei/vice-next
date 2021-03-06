/*
 * delaep256.c - Cartridge handling, Dela EP256 cart.
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
#include "delaep256.h"
#include "types.h"
#include "util.h"

/* This eprom system by DELA is similair to the EP64. It can handle
   what the EP64 can handle, plus the following features :

   - Alternate rom at $8000

   The system uses 33 8kb eproms, of which the first is used for
   the main menu. 32 8kb banks are used for above named features.

   Because of the fact that this system supports switching in a
   different eprom at $8000 (followed by a reset) it is possible
   to place other 8kb carts in the eproms and use them.

   The bank selecting is done by writing to $DE00.

   The values for the (extra) eprom banks are:

   eprom banks  1- 8 : $38-3F
   eprom banks  9-16 : $28-2F
   eprom banks 17-24 : $18-1F
   eprom banks 25-32 : $08-0F

   Setting bit 7 high will switch off EXROM.
*/

/* ---------------------------------------------------------------------*/

static int currbank = 0;

static void REGPARM2 delaep256_io1_store(WORD addr, BYTE value)
{
    BYTE bank, config;

    /* D7 switches off EXROM */
    config = (value & 0x80) ? 2 : 0;

    cartridge_config_changed(config, config, CMODE_WRITE);

    bank = ((0x30 - (value & 0x30)) >> 1) + (value & 7) + 1;

    if (bank < 1 || bank > 32) {
        bank = 0;
    }

    cartridge_romlbank_set(bank);
    currbank = bank;
}

static BYTE REGPARM1 delaep256_io1_peek(WORD addr)
{
    return currbank;
}

/* ---------------------------------------------------------------------*/

static io_source_t delaep256_device = {
    "DELA EP256",
    IO_DETACH_CART,
    NULL,
    0xde00, 0xdeff, 0xff,
    0,
    delaep256_io1_store,
    NULL,
    delaep256_io1_peek,
    NULL, /* TODO: dump */
    CARTRIDGE_DELA_EP256
};

static io_source_list_t *delaep256_list_item = NULL;

static const c64export_resource_t export_res = {
    "Dela EP256", 1, 0, &delaep256_device, NULL, CARTRIDGE_DELA_EP256
};

/* ---------------------------------------------------------------------*/

void delaep256_config_init(void)
{
    cartridge_config_changed(0, 0, CMODE_READ);
    cartridge_romlbank_set(0);
}

/* FIXME: should copy rawcart to roml_banks ! */
void delaep256_config_setup(BYTE *rawcart)
{
    cartridge_config_changed(0, 0, CMODE_READ);
    cartridge_romlbank_set(0);
}

/* ---------------------------------------------------------------------*/
static int delaep256_common_attach(void)
{
    if (c64export_add(&export_res) < 0) {
        return -1;
    }
    delaep256_list_item = c64io_register(&delaep256_device);
    return 0;
}

/* FIXME: this function should setup rawcart instead of copying to roml_banks ! */
/* FIXME: handle the various combinations / possible file lengths */
int delaep256_bin_attach(const char *filename, BYTE *rawcart)
{
    if (util_file_load(filename, roml_banks, 0x2000, UTIL_FILE_LOAD_SKIP_ADDRESS) < 0) {
        return -1;
    }
    return delaep256_common_attach();
}

/* FIXME: this function should setup rawcart instead of copying to roml_banks ! */
int delaep256_crt_attach(FILE *fd, BYTE *rawcart)
{
    WORD chip;
    WORD size;
    BYTE chipheader[0x10];

    memset(roml_banks, 0xff, 0x42000);

    while (1) {
        if (fread(chipheader, 0x10, 1, fd) < 1) {
            break;
        }

        chip = (chipheader[0x0a] << 8) + chipheader[0x0b];
        size = (chipheader[0x0e] << 8) + chipheader[0x0f];

        if (size != 0x2000) {
            return -1;
        }

        if (chip > 32) {
            return -1;
        }

        if (fread(roml_banks + (chip << 13), 0x2000, 1, fd) < 1) {
            return -1;
        }
    }
    return delaep256_common_attach();
}

void delaep256_detach(void)
{
    c64export_remove(&export_res);
    c64io_unregister(delaep256_list_item);
    delaep256_list_item = NULL;
}
