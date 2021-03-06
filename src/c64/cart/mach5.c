/*
 * mach5.c - Cartridge handling, Mach 5 cart.
 *
 * Written by
 * Groepaz <groepaz@gmx.net>
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
#include "mach5.h"
#include "maincpu.h"
#include "types.h"
#include "util.h"

/*
    This cart has 8Kb ROM mapped at $8000-$9FFF.

    The $9E00-$9EFF range is mirrored at $DE00-$DEFF.
    The $9F00-$9FFF range is mirrored at $DF00-$DFFF.
*/

/* #define DEBUGMACH5 */

#ifdef DEBUGMACH5
#define DBG(x) printf x
#else
#define DBG(x)
#endif

static BYTE REGPARM1 mach5_io1_read(WORD addr)
{
/*    DBG(("io1 rd %04x\n", addr)); */
    return roml_banks[0x1e00 + (addr & 0xff)];
}

static void REGPARM2 mach5_io1_store(WORD addr, BYTE value)
{
    DBG(("io1 st %04x %02x\n", addr, value));
    cartridge_config_changed(0, 0, CMODE_WRITE);
}

static BYTE REGPARM1 mach5_io2_read(WORD addr)
{
/*    DBG(("io2 rd %04x\n", addr)); */
    return roml_banks[0x1f00 + (addr & 0xff)];
}

static void REGPARM2 mach5_io2_store(WORD addr, BYTE value)
{
    DBG(("%04x io2 st %04x %02x\n", reg_pc, addr, value));
    cartridge_config_changed(2, 2, CMODE_WRITE);
}

/* ---------------------------------------------------------------------*/

static io_source_t mach5_io1_device = {
    "Mach 5",
    IO_DETACH_CART,
    NULL,
    0xde00, 0xdeff, 0xff,
    1, /* read is always valid */
    mach5_io1_store,
    mach5_io1_read,
    NULL, /* TODO: peek */
    NULL, /* TODO: dump */
    CARTRIDGE_MACH5
};

static io_source_t mach5_io2_device = {
    "Mach 5",
    IO_DETACH_CART,
    NULL,
    0xdf00, 0xdfff, 0xff,
    1, /* read is always valid */
    mach5_io2_store,
    mach5_io2_read,
    NULL, /* TODO: peek */
    NULL, /* TODO: dump */
    CARTRIDGE_MACH5
};

static io_source_list_t *mach5_io1_list_item = NULL;
static io_source_list_t *mach5_io2_list_item = NULL;

static const c64export_resource_t export_res = {
    "Mach 5", 1, 0, &mach5_io1_device, &mach5_io2_device, CARTRIDGE_MACH5
};

/* ---------------------------------------------------------------------*/

void mach5_config_init(void)
{
    cartridge_config_changed(0, 0, CMODE_READ);
}

void mach5_config_setup(BYTE *rawcart)
{
    memcpy(roml_banks, rawcart, 0x2000);
    cartridge_config_changed(0, 0, CMODE_READ);
}

/* ---------------------------------------------------------------------*/

static int mach5_common_attach(void)
{
    if (c64export_add(&export_res) < 0) {
        return -1;
    }
    mach5_io1_list_item = c64io_register(&mach5_io1_device);
    mach5_io2_list_item = c64io_register(&mach5_io2_device);
    return 0;
}

int mach5_bin_attach(const char *filename, BYTE *rawcart)
{
    if (util_file_load(filename, rawcart, 0x2000, UTIL_FILE_LOAD_SKIP_ADDRESS) < 0) {
        return -1;
    }

    return mach5_common_attach();
}

int mach5_crt_attach(FILE *fd, BYTE *rawcart)
{
    BYTE chipheader[0x10];

    if (fread(chipheader, 0x10, 1, fd) < 1) {
        return -1;
    }

    if (fread(rawcart, 0x2000, 1, fd) < 1) {
        return -1;
    }

    return mach5_common_attach();
}

void mach5_detach(void)
{
    c64export_remove(&export_res);
    c64io_unregister(mach5_io1_list_item);
    c64io_unregister(mach5_io2_list_item);
    mach5_io1_list_item = NULL;
    mach5_io2_list_item = NULL;
}
