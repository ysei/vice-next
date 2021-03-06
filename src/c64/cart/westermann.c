/*
 * westermann.c - Cartridge handling, Westermann cart.
 *
 * Written by
 *  groepaz <groepaz@gmx.net>
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
#include "westermann.h"
#include "types.h"
#include "util.h"

/*
    Westermann Utility Cartridge

    16kb ROM

    - starts in 16k game config
    - any read access to io2 switches to 8k game config
*/

/* some prototypes are needed */
static BYTE REGPARM1 westermann_io2_read(WORD addr);
static BYTE REGPARM1 westermann_io2_peek(WORD addr);

static io_source_t westermann_device = {
    "Westermann",
    IO_DETACH_CART,
    NULL,
    0xdf00, 0xdfff, 0xff,
    0, /* read is never valid */
    NULL,
    westermann_io2_read,
    westermann_io2_peek,
    NULL, /* TODO: dump */
    CARTRIDGE_WESTERMANN
};

static io_source_list_t *westermann_list_item = NULL;

static const c64export_resource_t export_res_westermann = {
    "Westermann", 1, 0, NULL, &westermann_device, CARTRIDGE_WESTERMANN
};

/* ---------------------------------------------------------------------*/

static BYTE REGPARM1 westermann_io2_read(WORD addr)
{
    cartridge_config_changed(0, 0, CMODE_READ);
    return 0;
}

static BYTE REGPARM1 westermann_io2_peek(WORD addr)
{
    return 0;
}

/* ---------------------------------------------------------------------*/

void westermann_config_init(void)
{
    cartridge_config_changed(1, 1, CMODE_READ);
}

void westermann_config_setup(BYTE *rawcart)
{
    memcpy(roml_banks, rawcart, 0x2000);
    memcpy(romh_banks, &rawcart[0x2000], 0x2000);
    cartridge_config_changed(1, 1, CMODE_READ);
}

static int westermann_common_attach(void)
{
    if (c64export_add(&export_res_westermann) < 0) {
        return -1;
    }
    westermann_list_item = c64io_register(&westermann_device);

    return 0;
}

int westermann_bin_attach(const char *filename, BYTE *rawcart)
{
    if (util_file_load(filename, rawcart, 0x4000, UTIL_FILE_LOAD_SKIP_ADDRESS) < 0) {
        return -1;
    }
    return westermann_common_attach();
}

int westermann_crt_attach(FILE *fd, BYTE *rawcart)
{
    BYTE chipheader[0x10];

    if (fread(chipheader, 0x10, 1, fd) < 1) {
        return -1;
    }

    if (chipheader[0xc] != 0x80 || chipheader[0xe] != 0x40) {
        return -1;
    }

    if (fread(rawcart, chipheader[0xe] << 8, 1, fd) < 1) {
        return -1;
    }

    return westermann_common_attach();
}

void westermann_detach(void)
{
    c64export_remove(&export_res_westermann);
    c64io_unregister(westermann_list_item);
    westermann_list_item = NULL;
}

