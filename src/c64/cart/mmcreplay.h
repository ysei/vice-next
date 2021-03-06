/*
 * mmcreplay.h - Cartridge handling, MMCReplay cart.
 *
 * Written by
 *  Groepaz/Hitmen <groepaz@gmx.net>
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

#ifndef VICE_MMCREPLAY_H
#define VICE_MMCREPLAY_H

#include "types.h"

/* FIXME get rid of this */
#define MMCREPLAY_EEPROM_SIZE (1024)

extern BYTE REGPARM1 mmcreplay_roml_read(WORD addr);
extern void REGPARM2 mmcreplay_roml_store(WORD addr, BYTE value);
extern BYTE REGPARM1 mmcreplay_romh_read(WORD addr);
extern void REGPARM2 mmcreplay_romh_store(WORD addr, BYTE value);

extern BYTE REGPARM1 mmcreplay_1000_7fff_read(WORD addr);
extern void REGPARM2 mmcreplay_1000_7fff_store(WORD addr, BYTE value);
extern BYTE REGPARM1 mmcreplay_a000_bfff_read(WORD addr);
extern void REGPARM2 mmcreplay_a000_bfff_store(WORD addr, BYTE value);
extern BYTE REGPARM1 mmcreplay_c000_cfff_read(WORD addr);
extern void REGPARM2 mmcreplay_c000_cfff_store(WORD addr, BYTE value);

extern void mmcreplay_freeze(void);
extern int mmcreplay_freeze_allowed(void);

extern void mmcreplay_config_init(void);
extern void mmcreplay_reset(void);
extern void mmcreplay_config_setup(BYTE *rawcart);
extern int mmcreplay_bin_attach(const char *filename, BYTE *rawcart);
extern int mmcreplay_crt_attach(FILE *fd, BYTE *rawcart, const char *filename);
extern void mmcreplay_detach(void);
extern int mmcreplay_flush_image(void);
extern int mmcreplay_bin_save(const char *filename);
extern int mmcreplay_crt_save(const char *filename);

extern int mmcreplay_resources_init(void);
extern void mmcreplay_resources_shutdown(void);
extern int mmcreplay_cmdline_options_init(void);

extern int mmcr_clockport_enabled; /* FIXME */

#endif

