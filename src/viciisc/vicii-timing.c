/*
 * vicii-timing.c - Timing related settings for the MOS 6569 (VIC-II) emulation.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Gunnar Ruthenberg <Krill.Plush@gmail.com>
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

#include "c64.h"
#include "machine.h"
#include "resources.h"
#include "vicii-timing.h"
#include "vicii.h"
#include "viciitypes.h"

/* Number of cycles per line.  */
#define VICII_PAL_CYCLES_PER_LINE      C64_PAL_CYCLES_PER_LINE
#define VICII_NTSC_CYCLES_PER_LINE     C64_NTSC_CYCLES_PER_LINE
#define VICII_NTSCOLD_CYCLES_PER_LINE  C64_NTSCOLD_CYCLES_PER_LINE
#define VICII_PALN_CYCLES_PER_LINE     C64_PALN_CYCLES_PER_LINE

void vicii_timing_set(machine_timing_t *machine_timing, int border_mode)
{
    int mode;

    resources_get_int("MachineVideoStandard", &mode);

    switch (mode) {
      case MACHINE_SYNC_NTSC:
        vicii.screen_height = VICII_NTSC_SCREEN_HEIGHT;
        switch (border_mode) {
          default:
          case VICII_NORMAL_BORDERS:
            vicii.screen_leftborderwidth = VICII_SCREEN_NTSC_NORMAL_LEFTBORDERWIDTH;
            vicii.screen_rightborderwidth = VICII_SCREEN_NTSC_NORMAL_RIGHTBORDERWIDTH;
            vicii.first_displayed_line = VICII_NTSC_NORMAL_FIRST_DISPLAYED_LINE;
            vicii.last_displayed_line = VICII_NTSC_NORMAL_LAST_DISPLAYED_LINE;
            break;       
          case VICII_FULL_BORDERS:
            vicii.screen_leftborderwidth = VICII_SCREEN_NTSC_FULL_LEFTBORDERWIDTH;
            vicii.screen_rightborderwidth = VICII_SCREEN_NTSC_FULL_RIGHTBORDERWIDTH;
            vicii.first_displayed_line = VICII_NTSC_FULL_FIRST_DISPLAYED_LINE;
            vicii.last_displayed_line = VICII_NTSC_FULL_LAST_DISPLAYED_LINE;
            break;
          case VICII_DEBUG_BORDERS:
            vicii.screen_leftborderwidth = VICII_SCREEN_NTSC_DEBUG_LEFTBORDERWIDTH;
            vicii.screen_rightborderwidth = VICII_SCREEN_NTSC_DEBUG_RIGHTBORDERWIDTH;
            vicii.first_displayed_line = VICII_NTSC_DEBUG_FIRST_DISPLAYED_LINE;
            vicii.last_displayed_line = VICII_NTSC_DEBUG_LAST_DISPLAYED_LINE;
            break;
        }
        vicii.cycles_per_line = VICII_NTSC_CYCLES_PER_LINE;
        break;
      case MACHINE_SYNC_NTSCOLD:
        vicii.screen_height = VICII_NTSCOLD_SCREEN_HEIGHT;
        switch (border_mode) {
          default:
          case VICII_NORMAL_BORDERS:
            vicii.screen_leftborderwidth = VICII_SCREEN_NTSCOLD_NORMAL_LEFTBORDERWIDTH;
            vicii.screen_rightborderwidth = VICII_SCREEN_NTSCOLD_NORMAL_RIGHTBORDERWIDTH;
            vicii.first_displayed_line = VICII_NTSCOLD_NORMAL_FIRST_DISPLAYED_LINE;
            vicii.last_displayed_line = VICII_NTSCOLD_NORMAL_LAST_DISPLAYED_LINE;
            break;
          case VICII_FULL_BORDERS:
            vicii.screen_leftborderwidth = VICII_SCREEN_NTSCOLD_FULL_LEFTBORDERWIDTH;
            vicii.screen_rightborderwidth = VICII_SCREEN_NTSCOLD_FULL_RIGHTBORDERWIDTH;
            vicii.first_displayed_line = VICII_NTSCOLD_FULL_FIRST_DISPLAYED_LINE;
            vicii.last_displayed_line = VICII_NTSCOLD_FULL_LAST_DISPLAYED_LINE;
            break;
          case VICII_DEBUG_BORDERS:
            vicii.screen_leftborderwidth = VICII_SCREEN_NTSCOLD_DEBUG_LEFTBORDERWIDTH;
            vicii.screen_rightborderwidth = VICII_SCREEN_NTSCOLD_DEBUG_RIGHTBORDERWIDTH;
            vicii.first_displayed_line = VICII_NTSCOLD_DEBUG_FIRST_DISPLAYED_LINE;
            vicii.last_displayed_line = VICII_NTSCOLD_DEBUG_LAST_DISPLAYED_LINE;
            break;
        }
        vicii.cycles_per_line = VICII_NTSCOLD_CYCLES_PER_LINE;
        break;
      case MACHINE_SYNC_PALN:
        vicii.screen_height = VICII_PALN_SCREEN_HEIGHT;
        switch (border_mode) {
          default:
          case VICII_NORMAL_BORDERS:
            vicii.screen_leftborderwidth = VICII_SCREEN_PALN_NORMAL_LEFTBORDERWIDTH;
            vicii.screen_rightborderwidth = VICII_SCREEN_PALN_NORMAL_RIGHTBORDERWIDTH;
            vicii.first_displayed_line = VICII_PALN_NORMAL_FIRST_DISPLAYED_LINE;
            vicii.last_displayed_line = VICII_PALN_NORMAL_LAST_DISPLAYED_LINE;
            break;
          case VICII_FULL_BORDERS:
            vicii.screen_leftborderwidth = VICII_SCREEN_PALN_FULL_LEFTBORDERWIDTH;
            vicii.screen_rightborderwidth = VICII_SCREEN_PALN_FULL_RIGHTBORDERWIDTH;
            vicii.first_displayed_line = VICII_PALN_FULL_FIRST_DISPLAYED_LINE;
            vicii.last_displayed_line = VICII_PALN_FULL_LAST_DISPLAYED_LINE;
            break;
          case VICII_DEBUG_BORDERS:
            vicii.screen_leftborderwidth = VICII_SCREEN_PALN_DEBUG_LEFTBORDERWIDTH;
            vicii.screen_rightborderwidth = VICII_SCREEN_PALN_DEBUG_RIGHTBORDERWIDTH;
            vicii.first_displayed_line = VICII_PALN_DEBUG_FIRST_DISPLAYED_LINE;
            vicii.last_displayed_line = VICII_PALN_DEBUG_LAST_DISPLAYED_LINE;
            break;
        }
        vicii.cycles_per_line = VICII_PALN_CYCLES_PER_LINE;
        break;
      case MACHINE_SYNC_PAL:
      default:
        vicii.screen_height = VICII_PAL_SCREEN_HEIGHT;
        switch (border_mode) {
          default:
          case VICII_NORMAL_BORDERS:
            vicii.screen_leftborderwidth = VICII_SCREEN_PAL_NORMAL_LEFTBORDERWIDTH;
            vicii.screen_rightborderwidth = VICII_SCREEN_PAL_NORMAL_RIGHTBORDERWIDTH;
            vicii.first_displayed_line = VICII_PAL_NORMAL_FIRST_DISPLAYED_LINE;
            vicii.last_displayed_line = VICII_PAL_NORMAL_LAST_DISPLAYED_LINE;
            break;
          case VICII_FULL_BORDERS:
            vicii.screen_leftborderwidth = VICII_SCREEN_PAL_FULL_LEFTBORDERWIDTH;
            vicii.screen_rightborderwidth = VICII_SCREEN_PAL_FULL_RIGHTBORDERWIDTH;
            vicii.first_displayed_line = VICII_PAL_FULL_FIRST_DISPLAYED_LINE;
            vicii.last_displayed_line = VICII_PAL_FULL_LAST_DISPLAYED_LINE;
            break;
          case VICII_DEBUG_BORDERS:
            vicii.screen_leftborderwidth = VICII_SCREEN_PAL_DEBUG_LEFTBORDERWIDTH;
            vicii.screen_rightborderwidth = VICII_SCREEN_PAL_DEBUG_RIGHTBORDERWIDTH;
            vicii.first_displayed_line = VICII_PAL_DEBUG_FIRST_DISPLAYED_LINE;
            vicii.last_displayed_line = VICII_PAL_DEBUG_LAST_DISPLAYED_LINE;
            break;
        }
        vicii.cycles_per_line = VICII_PAL_CYCLES_PER_LINE;
        break;
    }

}
