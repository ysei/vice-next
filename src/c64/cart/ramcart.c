/*
 * ramcart.c - RAMCART emulation.
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

/*
 * RamCart is a memory expansion module with battery backup for C64/128
 * that was designed and produced in Poland. At start there was only Atari
 * version, in 1993 a C64/128 cartridge appeared. It was produced in two
 * flavours: 64KB and 128KB.
 *
 * The memory is seen in a 256-byte window, placed at $df00-$dfff. The upper
 * bits of the address are set by writing page number to $de00 (and the
 * lowest bit of $de01 in 128KB version).
 *
 * Additionaly, there is a switch that protects the memory contents from
 * overwriting. If the switch is set to Read-Only and bit 7 of $de01 is
 * cleared (default), then contents of memory window are also visible in
 * the $8000-$80ff area. This allows to emulate usual cartridge takeover
 * after hardware reset by placing boot code with magic CBM80 string in
 * the very first page of RamCart memory.
 *
 * There was some firmware on a floppy that allowed the RamCart to be
 * used as a memory disk, as device number 7. You could load and save
 * files, load directory and delete files. Note that only LOAD and SAVE
 * worked. It wasn't possible to use BASIC command OPEN to create a file.
 * The firmware took over control right after hardware reset and presented
 * the user with a list of stored files. By pressing a letter key it was
 * possible to quickload a file and execute it. Hence RamCart was ideal for
 * storing frequently used tools.
 *
 * The register at $de01 only exists in the 128KB version.
 *
 * Register | bits
 * -------------------
 * $de01    | 7xxxxxx0
 *
 * x = unused, not connected.
 *
 * bit 7 is used in combination with the read-only switch to mirror
 *       $df00-$dfff into $8000-$80ff, when set to 1 and switch is
 *       on, area is mirrored.
 *
 * bit 0 is used as 64k bank selector.
 *
 * The current emulation has support for both 64k and 128k flavors,
 * the unused bits of the $de01 register is assumed to be not
 * connected.
 */

#include "vice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c64_256k.h"
#include "c64cart.h"
#include "c64cartsystem.h"
#include "c64export.h"
#include "c64io.h"
#include "c64mem.h"
#include "cartridge.h"
#include "cmdline.h"
#include "lib.h"
#include "machine.h"
#include "mem.h"
#include "plus256k.h"
#include "plus60k.h"
#include "resources.h"
#include "ramcart.h"
#include "snapshot.h"
#include "translate.h"
#include "types.h"
#include "util.h"
#include "vicii-phi1.h"

/* RAMCART registers */
static BYTE ramcart[2];

/* RAMCART image.  */
static BYTE *ramcart_ram = NULL;
static int old_ramcart_ram_size = 0;

static int ramcart_activate(void);
static int ramcart_deactivate(void);

/* Flag: Do we enable the external RAMCART?  */
static int ramcart_enabled;

/* Flag: Is the RAMCART readonly ?  */
int ramcart_readonly = 0;

/* Size of the RAMCART.  */
static int ramcart_size = 0;

/* Size of the RAMCART in KB.  */
static int ramcart_size_kb = 0;

/* Filename of the RAMCART image.  */
static char *ramcart_filename = NULL;

/* ------------------------------------------------------------------------- */
int ramcart_cart_enabled(void)
{
	return ramcart_enabled;
}

static BYTE REGPARM1 ramcart_reg_read(WORD addr)
{
	BYTE retval;

	if (addr == 1 && ramcart_size_kb == 128) 
	{
		retval = vicii_read_phi1() & 0x7e;
		retval += ramcart[addr];
	}
	else
		retval = ramcart[addr];

	return retval;
}

static void REGPARM2 ramcart_reg_store(WORD addr, BYTE byte)
{
	if (addr == 1 && ramcart_size_kb == 128)
		ramcart[1] = byte & 0x81;

	if (addr == 0)
		ramcart[0] = byte;
}

static BYTE REGPARM1 ramcart_window_read(WORD addr)
{
	BYTE retval;

	retval = ramcart_ram[((ramcart[1] & 1) << 16) + (ramcart[0] * 256) + (addr & 0xff)];

	return retval;
}

static void REGPARM2 ramcart_window_store(WORD addr, BYTE byte)
{
	ramcart_ram[((ramcart[1] & 1) * 65536) + (ramcart[0] * 256) + (addr & 0xff)] = byte;
}

/* ------------------------------------------------------------------------- */

static io_source_t ramcart_io1_device = {
    "RAMCART",
    IO_DETACH_RESOURCE,
    "RAMCART",
    0xde00, 0xdeff, 0x01,
    1, /* read is always valid */
    ramcart_reg_store,
    ramcart_reg_read,
    NULL,
    NULL,
    CARTRIDGE_RAMCART
};

static io_source_t ramcart_io2_device = {
    "RAMCART",
    IO_DETACH_RESOURCE,
    "RAMCART",
    0xdf00, 0xdfff, 0xff,
    1, /* read is always valid */
    ramcart_window_store,
    ramcart_window_read,
    NULL,
    NULL,
    CARTRIDGE_RAMCART
};

static io_source_list_t *ramcart_io1_list_item = NULL;
static io_source_list_t *ramcart_io2_list_item = NULL;

static const c64export_resource_t export_res = {
    "RAMCART", 1, 0, &ramcart_io1_device, &ramcart_io2_device, CARTRIDGE_RAMCART
};

/* ------------------------------------------------------------------------- */

static int ramcart_activate(void)
{
	if (!ramcart_size)
		return 0;

	ramcart_ram = lib_realloc((void *)ramcart_ram, (size_t)ramcart_size);

	/* Clear newly allocated RAM.  */
	if (ramcart_size > old_ramcart_ram_size)
		memset(ramcart_ram, 0, (size_t)(ramcart_size - old_ramcart_ram_size));

	old_ramcart_ram_size = ramcart_size;

	#ifdef CELL_DEBUG
	printf("INFO: %dKB unit installed.\n", ramcart_size >> 10);
	#endif

	if (!util_check_null_string(ramcart_filename))
	{
		if (util_file_load(ramcart_filename, ramcart_ram, (size_t)ramcart_size, UTIL_FILE_LOAD_RAW) < 0)
		{
			#ifdef CELL_DEBUG
			printf("INFO: Reading RAMCART image %s failed.\n", ramcart_filename);
			#endif
			if (util_file_save(ramcart_filename, ramcart_ram, ramcart_size) < 0)
			{
				#ifdef CELL_DEBUG
				printf("INFO: Creating RAMCART image %s failed.\n", ramcart_filename);
				#endif
				return -1;
			}
			#ifdef CELL_DEBUG
			printf("INFO: Creating RAMCART image %s.\n", ramcart_filename);
			#endif
			return 0;
		}
		#ifdef CELL_DEBUG
		printf("INFO: Reading RAMCART image %s.\n", ramcart_filename);
		#endif
	}

	ramcart_reset();
	return 0;
}

static int ramcart_deactivate(void)
{
	if (ramcart_ram == NULL)
		return 0;

	if (!util_check_null_string(ramcart_filename))
	{
		if (util_file_save(ramcart_filename, ramcart_ram, ramcart_size) < 0)
		{
			#ifdef CELL_DEBUG
			printf("INFO: Writing RAMCART image %s failed.\n", ramcart_filename);
			#endif
			return -1;
		}
		#ifdef CELL_DEBUG
		printf("INFO: Writing RAMCART image %s.\n", ramcart_filename);
		#endif
	}

	lib_free(ramcart_ram);
	ramcart_ram = NULL;
	old_ramcart_ram_size = 0;

	return 0;
}

static int set_ramcart_enabled(int val, void *param)
{
	if(!ramcart_enabled && val)
	{
		cart_power_off();
		if (ramcart_activate() < 0)
			return -1;

		if (c64export_add(&export_res) < 0)
			return -1;

		ramcart_io1_list_item = c64io_register(&ramcart_io1_device);
		ramcart_io2_list_item = c64io_register(&ramcart_io2_device);
		ramcart_enabled = 1;
		/* FIXME */
		export.exrom = 1;
		mem_pla_config_changed();
	}
	else if(ramcart_enabled && !val)
	{
		cart_power_off();
		if (ramcart_deactivate() < 0)
			return -1;

		c64io_unregister(ramcart_io1_list_item);
		c64io_unregister(ramcart_io2_list_item);
		ramcart_io1_list_item = NULL;
		ramcart_io2_list_item = NULL;
		c64export_remove(&export_res);
		ramcart_enabled = 0;
		/* FIXME */
		export.exrom = 0;
		mem_pla_config_changed();
	}
	return 0;
}

static int set_ramcart_readonly(int val, void *param)
{
	ramcart_readonly = val;
	return 0;
}

static int set_ramcart_size(int val, void *param)
{
	if (val == ramcart_size_kb)
		return 0;

	switch (val)
	{
		case 64:
		case 128:
			break;
		default:
			#ifdef CELL_DEBUG
			printf("INFO: Unknown RAMCART size %d.\n", val);
			#endif
			return -1;
	}

	if (ramcart_enabled)
	{
		ramcart_deactivate();
		ramcart_size_kb = val;
		ramcart_size = ramcart_size_kb << 10;
		ramcart_activate();
	}
	else
	{
		ramcart_size_kb = val;
		ramcart_size = ramcart_size_kb << 10;
	}

	return 0;
}

static int set_ramcart_filename(const char *name, void *param)
{
	if (ramcart_filename != NULL && name != NULL && strcmp(name, ramcart_filename) == 0)
		return 0;

	if (name != NULL && *name != '\0')
	{
		if (util_check_filename_access(name) < 0)
			return -1;
	}

	if (ramcart_enabled)
	{
		ramcart_deactivate();
		util_string_set(&ramcart_filename, name);
		ramcart_activate();
	}
	else
		util_string_set(&ramcart_filename, name);

	return 0;
}

/* ------------------------------------------------------------------------- */

static const resource_string_t resources_string[] = {
    { "RAMCARTfilename", "", RES_EVENT_NO, NULL,
      &ramcart_filename, set_ramcart_filename, NULL },
    { NULL }
};

static const resource_int_t resources_int[] = {
    { "RAMCART", 0, RES_EVENT_STRICT, (resource_value_t)0,
      &ramcart_enabled, set_ramcart_enabled, NULL },
    { "RAMCART_RO", 0, RES_EVENT_NO, NULL,
      &ramcart_readonly, set_ramcart_readonly, NULL },
    { "RAMCARTsize", 128, RES_EVENT_NO, NULL,
      &ramcart_size_kb, set_ramcart_size, NULL },
    { NULL }
};

int ramcart_resources_init(void)
{
	if (resources_register_string(resources_string) < 0)
		return -1;

	return resources_register_int(resources_int);
}

void ramcart_resources_shutdown(void)
{
	lib_free(ramcart_filename);
	ramcart_filename = NULL;
}

/* ------------------------------------------------------------------------- */

static const cmdline_option_t cmdline_options[] =
{
    { "-ramcart", SET_RESOURCE, 0,
      NULL, NULL, "RAMCART", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDCLS_ENABLE_RAMCART,
      NULL, NULL },
    { "+ramcart", SET_RESOURCE, 0,
      NULL, NULL, "RAMCART", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDCLS_DISABLE_RAMCART,
      NULL, NULL },
    { "-ramcartimage", SET_RESOURCE, 1,
      NULL, NULL, "RAMCARTfilename", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NAME, IDCLS_SPECIFY_RAMCART_NAME,
      NULL, NULL },
    { "-ramcartsize", SET_RESOURCE, 1,
      NULL, NULL, "RAMCARTsize", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_SIZE_IN_KB, IDCLS_RAMCART_SIZE,
      NULL, NULL },
    { NULL }
};

int ramcart_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

const char *ramcart_get_file_name(void)
{
	return ramcart_filename;
}

void ramcart_init_config(void)
{
	if (ramcart_enabled)
	{
		/* FIXME */
		export.exrom = 1;
		mem_pla_config_changed();
	}
}

void ramcart_init(void)
{
}

void ramcart_reset(void)
{
	ramcart[0] = 0;
	ramcart[1] = 0;
}

void ramcart_config_setup(BYTE *rawcart)
{
	memcpy(ramcart_ram, rawcart, ramcart_size); /* FIXME */
}

void ramcart_detach(void)
{
	resources_set_int("RAMCART", 0);
}

int ramcart_enable(void)
{
	if (resources_set_int("RAMCART", 1) < 0)
		return -1;

	return 0;
}

/* ------------------------------------------------------------------------- */

BYTE REGPARM1 ramcart_roml_read(WORD addr)
{
	if (ramcart_readonly == 1 && ramcart_size_kb == 128 && addr >= 0x8000 && addr <= 0x80ff)
		return ramcart_ram[((ramcart[1] & 1) * 65536) + (ramcart[0] * 256) + (addr & 0xff)];

	/* FIXME: intentionally breaking this, this code should be removed and
	   ram extensions should be handled in the generic interface */
#if 0
	if (plus60k_enabled) {
		return plus60k_ram_read(addr);
	}
	if (plus256k_enabled) {
		return plus256k_ram_high_read(addr);
	}
	if (c64_256k_enabled) {
		return c64_256k_ram_segment2_read(addr);
	}
#endif
	return mem_ram[addr];
}

void REGPARM2 ramcart_roml_store(WORD addr, BYTE byte)
{
	/* FIXME: intentionally breaking this, this code should be removed and
	   ram extensions should be handled in the generic interface */
#if 0
	if (plus60k_enabled) {
		plus60k_ram_store(addr, byte);
		return;
	}
	if (plus256k_enabled) {
		plus256k_ram_high_store(addr, byte);
		return;
	}
	if (c64_256k_enabled) {
		c64_256k_ram_segment2_store(addr, byte);
		return;
	}
#endif
	mem_ram[addr] = byte;
}
