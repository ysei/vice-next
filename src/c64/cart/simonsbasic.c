/*
 * simonsbasic.c - Cartridge handling, Simons Basic cart.
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
#include <stdlib.h>
#include <string.h>

#include "c64cart.h"
#include "c64cartmem.h"
#include "c64export.h"
#include "c64io.h"
#include "cartridge.h"
#include "simonsbasic.h"
#include "types.h"
#include "util.h"

/*
    Simon's Basic Cartridge

    - 16kb ROM

    - reading io1 switches to 8k game config (bank 0 at $8000)
    - writing io1 switches to 16k game config (bank 0 at $8000, bank 1 at $a000)
*/

static BYTE REGPARM1 simon_io1_read(WORD addr)
{
    cartridge_config_changed(0, 0, CMODE_READ);
    return 0;
}

static BYTE REGPARM1 simon_io1_peek(WORD addr)
{
    return 0;
}

static void REGPARM2 simon_io1_store(WORD addr, BYTE value)
{
    cartridge_config_changed(1, 1, CMODE_WRITE);
}

/* ---------------------------------------------------------------------*/

static io_source_t simon_device = {
    "Simon's Basic",
    IO_DETACH_CART,
    NULL,
    0xde00, 0xdeff, 0xff,
    0, /* read is never valid */
    simon_io1_store,
    simon_io1_read,
    simon_io1_peek,
    NULL, /* TODO: dump */
    CARTRIDGE_SIMONS_BASIC
};

static io_source_list_t *simon_list_item = NULL;

static const c64export_resource_t export_res_simon = {
    "Simon's Basic", 1, 1, &simon_device, NULL, CARTRIDGE_SIMONS_BASIC
};

/* ---------------------------------------------------------------------*/

void simon_config_init(void)
{
    cartridge_config_changed(1, 1, CMODE_READ);
}

void simon_config_setup(BYTE *rawcart)
{
    memcpy(roml_banks, rawcart, 0x2000);
    memcpy(romh_banks, &rawcart[0x2000], 0x2000);
    cartridge_config_changed(1, 1, CMODE_READ);
}

static int simon_common_attach(void)
{
    if (c64export_add(&export_res_simon) < 0) {
        return -1;
    }
    simon_list_item = c64io_register(&simon_device);
    return 0;
}

int simon_bin_attach(const char *filename, BYTE *rawcart)
{
    if (util_file_load(filename, rawcart, 0x4000, UTIL_FILE_LOAD_SKIP_ADDRESS) < 0) {
        return -1;
    }
    return simon_common_attach();
}

int simon_crt_attach(FILE *fd, BYTE *rawcart)
{
    BYTE chipheader[0x10];
    int i;

    for (i = 0; i <= 1; i++) {
        if (fread(chipheader, 0x10, 1, fd) < 1) {
            return -1;
        }

        if (chipheader[0xc] != 0x80 && chipheader[0xc] != 0xa0) {
            return -1;
        }

        if (fread(&rawcart[(chipheader[0xc] << 8) - 0x8000], 0x2000, 1, fd) < 1) {
            return -1;
        }
    }

    return simon_common_attach();
}

void simon_detach(void)
{
    c64export_remove(&export_res_simon);
    c64io_unregister(simon_list_item);
    simon_list_item = NULL;
}
