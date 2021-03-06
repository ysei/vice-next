/*
 * mmc64.h - Cartridge handling, MMC64 cart.
 *
 * Written by
 *  Markus Stehr <bastetfurry@ircnet.de>
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

#ifndef VICE_MMC64_H
#define VICE_MMC64_H

#include "types.h"

extern int mmc64_clockport_enabled;
extern int mmc64_hw_clockport;

extern int mmc64_cart_enabled(void);
extern void mmc64_init_card_config(void);
extern BYTE REGPARM1 mmc64_roml_read(WORD addr);
extern void REGPARM2 mmc64_roml_store(WORD addr, BYTE byte);
extern BYTE REGPARM1 mmc64_peek_mem(WORD addr);

extern void mmc64_config_setup(BYTE *rawcart);

extern int mmc64_crt_attach(FILE *fd, BYTE *rawcart);
extern int mmc64_bin_attach(const char *filename, BYTE *rawcart);
extern int mmc64_bin_save(const char *filename);
extern int mmc64_crt_save(const char *filename);
extern int mmc64_flush_image(void);

extern int mmc64_resources_init(void);
extern void mmc64_resources_shutdown(void);
extern int mmc64_cmdline_options_init(void);
extern void mmc64_init(void);
extern void mmc64_detach(void);
extern void mmc64_reset(void);
extern int mmc64_enable(void);
extern const char *mmc64_get_file_name(void);

#endif
