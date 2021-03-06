/*
 * c64io.h -- C64 io handling.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#ifndef VICE_C64IO_H
#define VICE_C64IO_H
 
#include "types.h"

#define IO_DETACH_CART     0
#define IO_DETACH_RESOURCE 1

#define CPU_LINES_C64_256K 1
#define CPU_LINES_PLUS60K  2
#define CPU_LINES_PLUS256K 3

extern BYTE REGPARM1 c64io1_read(WORD addr);
extern BYTE REGPARM1 c64io1_peek(WORD addr);
extern void REGPARM2 c64io1_store(WORD addr, BYTE value);
extern BYTE REGPARM1 c64io2_read(WORD addr);
extern BYTE REGPARM1 c64io2_peek(WORD addr);
extern void REGPARM2 c64io2_store(WORD addr, BYTE value);

struct mem_ioreg_list_s;
extern void c64io_ioreg_add_list(struct mem_ioreg_list_s **mem_ioreg_list);

extern int get_cpu_lines_lock(void);
extern void set_cpu_lines_lock(int device, char *name);
extern void remove_cpu_lines_lock(void);
extern char *get_cpu_lines_lock_name(void);

typedef struct io_source_s {
    char *name; /* literal name of this i/o device */
    int detach_id;
    char *resource_name;
    WORD start_address;
    WORD end_address;
    BYTE address_mask;
    int  io_source_valid;
    void REGPARM2 (*store)(WORD address, BYTE data);
    BYTE REGPARM1 (*read)(WORD address);
    BYTE REGPARM1 (*peek)(WORD address); /* read without side effects (used by monitor) */
    int REGPARM1 (*dump)(void); /* print detailed state for this i/o device (used by monitor) */
    int cart_id; /* id of associated cartridge */
} io_source_t;

typedef struct io_source_list_s {
    struct io_source_list_s *previous;
    io_source_t *device;
    struct io_source_list_s *next;
} io_source_list_t;

typedef struct io_source_detach_s {
    int det_id;
    char *det_devname;
    char *det_name;
    int det_cartid;
} io_source_detach_t;

extern io_source_list_t *c64io_register(io_source_t *device);
extern void c64io_unregister(io_source_list_t *device);

#endif
