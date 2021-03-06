/*
 * sfx_soundexpander.c - SFX soundexpnader cartridge emulation.
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

#include "c64export.h"
#include "c64io.h"
#include "cartridge.h"
#include "cmdline.h"
#include "fmopl.h"
#include "lib.h"
#include "maincpu.h"
#include "resources.h"
#include "sfx_soundexpander.h"
#include "sid.h"
#include "sound.h"
#include "uiapi.h"
#include "translate.h"

/* Flag: Do we enable the SFX soundexpander cartridge?  */
static int sfx_soundexpander_enabled = 0;

/* Flag: What type of ym chip is used?  */
int sfx_soundexpander_chip = 3526;

static FM_OPL *YM3526_chip = NULL;
static FM_OPL *YM3812_chip = NULL;

/* ------------------------------------------------------------------------- */

/* some prototypes are needed */
static void REGPARM2 sfx_soundexpander_sound_store(WORD addr, BYTE value);
static BYTE REGPARM1 sfx_soundexpander_sound_read(WORD addr);
static BYTE REGPARM1 sfx_soundexpander_piano_read(WORD addr);

static io_source_t sfx_soundexpander_sound_device = {
    "SFX Sound Expander",
    IO_DETACH_RESOURCE,
    "SFXSoundExpander",
    0xdf00, 0xdfff, 0x7f,
    0,
    sfx_soundexpander_sound_store,
    sfx_soundexpander_sound_read,
    NULL, /* FIXME: peek */
    NULL, /* FIXME: dump */
    CARTRIDGE_SFX_SOUND_EXPANDER
};

static io_source_t sfx_soundexpander_piano_device = {
    "SFX Sound Expander",
    IO_DETACH_RESOURCE,
    "SFXSoundExpander",
    0xdf00, 0xdfff, 0x1f,
    0,
    NULL,
    sfx_soundexpander_piano_read,
    NULL, /* FIXME: peek */
    NULL, /* FIXME: dump */
    CARTRIDGE_SFX_SOUND_EXPANDER
};

static io_source_list_t *sfx_soundexpander_sound_list_item = NULL;
static io_source_list_t *sfx_soundexpander_piano_list_item = NULL;

static const c64export_resource_t export_res_sound= {
    "SFX Sound Expander", 0, 0, NULL, &sfx_soundexpander_sound_device, CARTRIDGE_SFX_SOUND_SAMPLER
};

static const c64export_resource_t export_res_piano= {
    "SFX Sound Expander", 0, 0, NULL, &sfx_soundexpander_piano_device, CARTRIDGE_SFX_SOUND_SAMPLER
};

/* ------------------------------------------------------------------------- */

int sfx_soundexpander_cart_enabled(void)
{
    return sfx_soundexpander_enabled;
}

static int set_sfx_soundexpander_enabled(int val, void *param)
{
    if (sfx_soundexpander_enabled != val) {
        if (val) {
            if (c64export_add(&export_res_sound) < 0) {
                return -1;
            }
            if (c64export_add(&export_res_piano) < 0) {
                return -1;
            }
            sfx_soundexpander_sound_list_item = c64io_register(&sfx_soundexpander_sound_device);
            sfx_soundexpander_piano_list_item = c64io_register(&sfx_soundexpander_piano_device);
            sfx_soundexpander_enabled = 1;
        } else {
            c64export_remove(&export_res_sound);
            c64export_remove(&export_res_piano);
            c64io_unregister(sfx_soundexpander_sound_list_item);
            c64io_unregister(sfx_soundexpander_piano_list_item);
            sfx_soundexpander_sound_list_item = NULL;
            sfx_soundexpander_piano_list_item = NULL;
            sfx_soundexpander_enabled = 0;
        }
    }
    return 0;
}

static int set_sfx_soundexpander_chip(int val, void *param)
{
    int newval;

    if (val != 3526 && val != 3812) {
        newval = 3526;
    } else {
        newval = val;
    }

    if (newval != sfx_soundexpander_chip) {
        sid_state_changed = 1;
        sfx_soundexpander_chip = newval;
    }
    return 0;
}

void sfx_soundexpander_reset(void)
{
    /* FIXME: do nothing ? */
}

int sfx_soundexpander_enable(void)
{
    return resources_set_int("SFXSoundExpander", 1);
}
void sfx_soundexpander_detach(void)
{
    resources_set_int("SFXSoundExpander", 0);
}

/* ------------------------------------------------------------------------- */

static const resource_int_t resources_int[] = {
    { "SFXSoundExpander", 0, RES_EVENT_STRICT, (resource_value_t)0,
      &sfx_soundexpander_enabled, set_sfx_soundexpander_enabled, NULL },
    { "SFXSoundExpanderChip", 0, RES_EVENT_STRICT, (resource_value_t)3526,
      &sfx_soundexpander_chip, set_sfx_soundexpander_chip, NULL },
    { NULL }
};

int sfx_soundexpander_resources_init(void)
{
    return resources_register_int(resources_int);
}
void sfx_soundexpander_resources_shutdown(void)
{
}

/* ------------------------------------------------------------------------- */

static const cmdline_option_t cmdline_options[] =
{
    { "-sfxse", SET_RESOURCE, 0,
      NULL, NULL, "SFXSoundExpander", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDCLS_ENABLE_SFX_SE,
      NULL, NULL },
    { "+sfxse", SET_RESOURCE, 0,
      NULL, NULL, "SFXSoundExpander", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDCLS_DISABLE_SFX_SE,
      NULL, NULL },
    { "-sfxsetype", SET_RESOURCE, 1,
      NULL, NULL, "SFXSoundExpanderChip", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_TYPE, IDCLS_SET_YM_CHIP_TYPE,
      NULL, NULL },
    { NULL }
};

int sfx_soundexpander_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ---------------------------------------------------------------------*/

struct sfx_soundexpander_sound_s
{
    BYTE command;
};

static struct sfx_soundexpander_sound_s snd;

int sfx_soundexpander_sound_machine_calculate_samples(sound_t *psid, SWORD *pbuf, int nr, int interleave, int *delta_t)
{
    int i;
    SWORD *buffer;

    if (sfx_soundexpander_enabled) {
        buffer = lib_malloc(nr * 2);
        if (sfx_soundexpander_chip == 3812) {
            ym3812_update_one(YM3812_chip, buffer, nr);
        } else {
            ym3526_update_one(YM3526_chip, buffer, nr);
        }

        for (i = 0; i < nr; i++) {
            pbuf[i * interleave] = sound_audio_mix(pbuf[i * interleave], buffer[i]);
        }
        lib_free(buffer);
    }
    return 0;
}

int sfx_soundexpander_sound_machine_init(sound_t *psid, int speed, int cycles_per_sec)
{
    if (sfx_soundexpander_chip == 3812) {
        if (YM3812_chip != NULL) {
            ym3812_shutdown(YM3812_chip);
        }
        YM3812_chip = ym3812_init((UINT32)3579545, (UINT32)speed);
    } else {
        if (YM3526_chip != NULL) {
            ym3526_shutdown(YM3526_chip);
        }
        YM3526_chip = ym3526_init((UINT32)3579545, (UINT32)speed);
    }
    snd.command = 0;

    return 1;
}

void sfx_soundexpander_sound_machine_close(sound_t *psid)
{
    if (YM3526_chip != NULL) {
        ym3526_shutdown(YM3526_chip);
        YM3526_chip = NULL;
    }
    if (YM3812_chip != NULL) {
        ym3812_shutdown(YM3812_chip);
        YM3526_chip = NULL;
    }
}

void sfx_soundexpander_sound_machine_store(sound_t *psid, WORD addr, BYTE val)
{
    snd.command = val;

    if (sfx_soundexpander_chip == 3812) {
        ym3812_write(YM3812_chip, 1, val);
    } else {
        ym3526_write(YM3526_chip, 1, val);
    }
}

BYTE sfx_soundexpander_sound_machine_read(sound_t *psid, WORD addr)
{
    if (sfx_soundexpander_chip == 3812) {
        return ym3812_read(YM3812_chip, 1);
    }
    return ym3526_read(YM3526_chip, 1);
}

void sfx_soundexpander_sound_reset(void)
{
    if (sfx_soundexpander_chip == 3812) {
        ym3812_reset_chip(YM3812_chip);
    } else {
        ym3526_reset_chip(YM3526_chip);
    }
}

/* ---------------------------------------------------------------------*/

static void REGPARM2 sfx_soundexpander_sound_store(WORD addr, BYTE value)
{
    if (addr == 0xdf40) {
        if (sfx_soundexpander_chip == 3812) {
            ym3812_write(YM3812_chip, 0, value);
        } else {
            ym3526_write(YM3526_chip, 0, value);
        }
    }
    if (addr == 0xdf50) {
        sound_store((WORD)0x60, value, 0);
    }
}

static BYTE REGPARM1 sfx_soundexpander_sound_read(WORD addr)
{
    BYTE value = 0;

    sfx_soundexpander_sound_device.io_source_valid = 0;

    if (addr == 0xdf60) {
        sfx_soundexpander_sound_device.io_source_valid = 1;
        value=sound_read((WORD)0x60, 0);
    }
    return value;
}

/* No piano keyboard is emulated currently, so we return 0xff */
static BYTE REGPARM1 sfx_soundexpander_piano_read(WORD addr)
{
  sfx_soundexpander_piano_device.io_source_valid = 0;
  if ((addr & 16) == 0 && (addr & 8) == 8) {
      sfx_soundexpander_piano_device.io_source_valid = 1;
  }
  return (BYTE)0xff;
}
