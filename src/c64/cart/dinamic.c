/*
 * dinamic.c - Cartridge handling, Dinamic cart.
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
#include <string.h>

#include "c64cart.h"
#include "c64cartmem.h"
#include "c64export.h"
#include "c64io.h"
#include "cartridge.h"
#include "maincpu.h"
#include "types.h"
#include "util.h"

/* #define DBGDINAMIC */

#ifdef DBGDINAMIC
#define DBG(x) printf x
#else
#define DBG(x)
#endif

/*
    Dinamic Software Game Cartridge

    - Narco Police (128k, 8k*16)
    - Satan (128k, 8k*16)

    - 16 8k ROM Banks, mapped to $8000 in 8k Game Mode

    io1:
    - banks are switched by read accesses to deXX, where XX is the bank number

*/

static int currbank = 0;

static BYTE REGPARM1 dinamic_io1_read(WORD addr)
{
    DBG(("@ $%04x io1 rd %04x (bank: %02x)\n", reg_pc, addr, addr & 0x0f));
    if ((addr & 0x0f) == addr) {
        cartridge_romlbank_set(addr & 0x0f);
        cartridge_romhbank_set(addr & 0x0f);
        currbank = addr & 0x0f;
    }
    return 0;
}

static BYTE REGPARM1 dinamic_io1_peek(WORD addr)
{
    return currbank;
}

/* ---------------------------------------------------------------------*/

static io_source_t dinamic_io1_device = {
    "Dinamic",
    IO_DETACH_CART,
    NULL,
    0xde00, 0xdeff, 0xff,
    0, /* reads are never valid */
    NULL,
    dinamic_io1_read,
    dinamic_io1_peek,
    NULL, /* dump */
    CARTRIDGE_DINAMIC
};

static io_source_list_t *dinamic_io1_list_item = NULL;

static const c64export_resource_t export_res = {
    "Dinamic", 1, 0, &dinamic_io1_device, NULL, CARTRIDGE_DINAMIC
};

/* ---------------------------------------------------------------------*/

void dinamic_config_init(void)
{
    cartridge_config_changed(0, 0, CMODE_READ);
}

void dinamic_config_setup(BYTE *rawcart)
{
    memcpy(roml_banks, rawcart, 0x2000 * 16);
    cartridge_config_changed(0, 0, CMODE_READ);
}

/* ---------------------------------------------------------------------*/

static int dinamic_common_attach(void)
{
    if (c64export_add(&export_res) < 0) {
        return -1;
    }
    dinamic_io1_list_item = c64io_register(&dinamic_io1_device);
    return 0;
}

int dinamic_bin_attach(const char *filename, BYTE *rawcart)
{
    if (util_file_load(filename, rawcart, 0x20000, UTIL_FILE_LOAD_SKIP_ADDRESS) < 0) {
        return -1;
    }
    return dinamic_common_attach();
}

int dinamic_crt_attach(FILE *fd, BYTE *rawcart)
{
    BYTE chipheader[0x10];
    int bank;

    while (1) {
        if (fread(chipheader, 0x10, 1, fd) < 1) {
            break;
        }
        bank = chipheader[0xb];

        if ((bank >= 16) || (chipheader[0xc] != 0x80)) {
            return -1;
        }
        if (fread(&rawcart[bank << 13], 0x2000, 1, fd) < 1) {
            return -1;
        }
    }

    return dinamic_common_attach();
}

void dinamic_detach(void)
{
    c64io_unregister(dinamic_io1_list_item);
    dinamic_io1_list_item = NULL;
    c64export_remove(&export_res);
}
