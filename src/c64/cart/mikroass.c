/*
 * mikroass.c - Cartridge handling, Mikro Assembler cart.
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
#include "mikroass.h"
#include "types.h"
#include "util.h"

/*
    "Mikro Assembler" Cartridge

    This cart has 8Kb ROM mapped at $8000-$9FFF.

    The $9E00-$9EFF range is mirrored at $DE00-$DEFF.

    The $9F00-$9FFF range is mirrored at $DF00-$DFFF.

*/

static BYTE REGPARM1 mikroass_io1_read(WORD addr)
{
    return roml_banks[0x1e00 + (addr & 0xff)];
}

static BYTE REGPARM1 mikroass_io2_read(WORD addr)
{
    return roml_banks[0x1f00 + (addr & 0xff)];
}

/* ---------------------------------------------------------------------*/

static io_source_t mikroass_io1_device = {
    "MIKRO ASSEMBLER",
    IO_DETACH_CART,
    NULL,
    0xde00, 0xdeff, 0xff,
    1, /* read is always valid */
    NULL,
    mikroass_io1_read,
    NULL, /* TODO: peek */
    NULL, /* TODO: dump */
    CARTRIDGE_MIKRO_ASSEMBLER
};

static io_source_t mikroass_io2_device = {
    "MIKRO ASSEMBLER",
    IO_DETACH_CART,
    NULL,
    0xdf00, 0xdfff, 0xff,
    1, /* read is always valid */
    NULL,
    mikroass_io2_read,
    NULL, /* TODO: peek */
    NULL, /* TODO: dump */
    CARTRIDGE_MIKRO_ASSEMBLER
};

static io_source_list_t *mikroass_io1_list_item = NULL;
static io_source_list_t *mikroass_io2_list_item = NULL;

static const c64export_resource_t export_res = {
    "Mikro Assembler", 1, 0, &mikroass_io1_device, &mikroass_io2_device, CARTRIDGE_MIKRO_ASSEMBLER
};

/* ---------------------------------------------------------------------*/

void mikroass_config_init(void)
{
    cartridge_config_changed(0, 0, CMODE_READ);
}

void mikroass_config_setup(BYTE *rawcart)
{
    memcpy(roml_banks, rawcart, 0x2000);
    cartridge_config_changed(0, 0, CMODE_READ);
}

/* ---------------------------------------------------------------------*/

static int mikroass_common_attach(void)
{
    if (c64export_add(&export_res) < 0) {
        return -1;
    }
    mikroass_io1_list_item = c64io_register(&mikroass_io1_device);
    mikroass_io2_list_item = c64io_register(&mikroass_io2_device);
    return 0;
}

int mikroass_bin_attach(const char *filename, BYTE *rawcart)
{
    if (util_file_load(filename, rawcart, 0x2000, UTIL_FILE_LOAD_SKIP_ADDRESS) < 0) {
        return -1;
    }
    return mikroass_common_attach();
}

int mikroass_crt_attach(FILE *fd, BYTE *rawcart)
{
    BYTE chipheader[0x10];

    if (fread(chipheader, 0x10, 1, fd) < 1) {
        return -1;
    }

    if (fread(rawcart, 0x2000, 1, fd) < 1) {
        return -1;
    }

    return mikroass_common_attach();
}

void mikroass_detach(void)
{
    c64export_remove(&export_res);
    c64io_unregister(mikroass_io1_list_item);
    c64io_unregister(mikroass_io2_list_item);
    mikroass_io1_list_item = NULL;
    mikroass_io2_list_item = NULL;
}
