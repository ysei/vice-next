/*
 * tfe.h - TFE ("The final ethernet" emulation.
 *
 * Written by
 *  Spiro Trikaliotis <Spiro.Trikaliotis@gmx.de>
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
    FIXME: this header is currently included from arch dependend code,
           which breaks the privateness of the cart functions :)
*/

#ifdef HAVE_TFE 
#else
  #error TFE.H should not be included if HAVE_TFE is not defined!
#endif /* #ifdef HAVE_TFE */

#ifndef VICE_TFE_H
#define VICE_TFE_H

#include "types.h"

/* define this only if VICE should write each and every frame received
   and send into the VICE log
   WARNING: The log grows very fast!
*/
/** #define TFE_DEBUG_FRAMES **/

struct snapshot_s;

extern int tfe_cart_enabled(void);
extern int tfe_as_rr_net;

extern void tfe_init(void);
extern int tfe_resources_init(void);
extern void tfe_resources_shutdown(void);
extern int tfe_cmdline_options_init(void);

extern void tfe_reset(void);
extern void tfe_shutdown(void); /* FIXME: review and rename to _detach */
extern int tfe_enable(void);

extern int tfe_read_snapshot_module(struct snapshot_s *s);
extern int tfe_write_snapshot_module(struct snapshot_s *s);

extern void tfe_clockport_changed(void);

/*
 These functions let the UI enumerate the available interfaces.

 First, tfe_enumadapter_open() is used to start enumeration.

 tfe_enum_adapter is then used to gather information for each adapter present
 on the system, where:

   ppname points to a pointer which will hold the name of the interface
   ppdescription points to a pointer which will hold the description of the interface

   For each of these parameters, new memory is allocated, so it has to be
   freed with lib_free().

 tfe_enumadapter_close() must be used to stop processing.

 Each function returns 1 on success, and 0 on failure.
 tfe_enumadapter() only fails if there is no more adpater; in this case, 
   *ppname and *ppdescription are not altered.
*/
extern int tfe_enumadapter_open(void);
extern int tfe_enumadapter(char **ppname, char **ppdescription);
extern int tfe_enumadapter_close(void);

#endif
