/*
 * digimax.c - Digimax DAC cartridge emulation.
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

#include "cmdline.h"
#include "digiblaster.h"
#include "lib.h"
#include "maincpu.h"
#include "resources.h"
#include "sound.h"
#include "uiapi.h"
#include "translate.h"

/* Flag: Do we enable the DIGIBLASTER add-on?  */
int digiblaster_enabled;

static int set_digiblaster_enabled(int val, void *param)
{
    digiblaster_enabled = val;

    return 0;
}

static const resource_int_t resources_int[] = {
    { "DIGIBLASTER", 0, RES_EVENT_STRICT, (resource_value_t)0,
      &digiblaster_enabled, set_digiblaster_enabled, NULL },
    { NULL }
};

int digiblaster_resources_init(void)
{
    return resources_register_int(resources_int);
}

static const cmdline_option_t cmdline_options[] =
{
    { "-digiblaster", SET_RESOURCE, 0,
      NULL, NULL, "DIGIBLASTER", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDCLS_ENABLE_DIGIBLASTER,
      NULL, NULL },
    { "+digiblaster", SET_RESOURCE, 0,
      NULL, NULL, "DIGIBLASTER", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDCLS_DISABLE_DIGIBLASTER,
      NULL, NULL },
    { NULL }
};

int digiblaster_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ---------------------------------------------------------------------*/

static BYTE digiblaster_sound_data;

struct digiblaster_sound_s
{
    BYTE voice0;
};

static struct digiblaster_sound_s snd;

int digiblaster_sound_machine_calculate_samples(sound_t *psid, SWORD *pbuf, int nr,
                                                int interleave, int *delta_t)
{
    int i;

    if (digiblaster_enabled)
    {
        for (i = 0; i < nr; i++)
        {
            pbuf[i * interleave] = sound_audio_mix(pbuf[i * interleave],snd.voice0 << 8);
        }
    }
    return 0;
}

int digiblaster_sound_machine_init(sound_t *psid, int speed, int cycles_per_sec)
{
    snd.voice0 = 0;

    return 1;
}

void digiblaster_sound_machine_store(sound_t *psid, WORD addr, BYTE val)
{
    snd.voice0 = val;
}

BYTE digiblaster_sound_machine_read(sound_t *psid, WORD addr)
{
    return 0;
}

void digiblaster_sound_reset(void)
{
    snd.voice0 = 0;
    digiblaster_sound_data = 0;
}

/* ---------------------------------------------------------------------*/

void REGPARM2 digiblaster_store(WORD addr, BYTE value)
{
    digiblaster_sound_data = value;
    sound_store((WORD)(0x40), value, 0);
}

BYTE REGPARM1 digiblaster_read(WORD addr)
{
    return sound_read((WORD)(addr + 0x40), 0);
}
