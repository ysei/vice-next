/******************************************************************************* 
 *  -- kbd.cpp      Keyboard driver for Playstation 3
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
 *  Copyright (C) 2010
 *
 *      Author    TimRex
 *
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ********************************************************************************/

#include "vice.h"

#include <stdio.h>
#include <stdbool.h>
#include <cell/keyboard.h>

#include "kbd.h"
#include "keyboard.h"

#define MAX_KEYBD 2

uint8_t old_status[MAX_KEYBD];

static CellKbInfo status; 
uint32_t old_info = 0;

#define MAX_KEYS 8

//Unlikely we'll ever want to register more than 8 keys at a time
static uint16_t keysdown[MAX_KEYS];
static unsigned int mkey;

#define L_CTRL  300
#define L_SHIFT 301
#define L_ALT   302
#define L_WIN   303

#define R_CTRL  304
#define R_SHIFT 305
#define R_ALT   306
#define R_WIN   307

void kbd_arch_init(void)
{
	if (cellKbInit(1) != CELL_OK)
	{
		#ifdef CELL_DEBUG
		printf("WARNING: Keyboard failed to initialise\n");
		#endif
		return;
	}

	if (cellKbSetCodeType (0, CELL_KB_CODETYPE_ASCII) != CELL_KB_OK)
	{
		#ifdef CELL_DEBUG
		printf("WARNING: Unable to set KB_CODETYPE_ASCII\n");
		#endif
		return;
	}

	// Packet mode or Character mode?
	//CELL_KB_RMODE_PACKET
	if (cellKbSetReadMode (0, CELL_KB_RMODE_INPUTCHAR) != CELL_KB_OK)
	{
		#ifdef CELL_DEBUG
		printf("WARNING: Unable to set CELL_KB_RMODE_INPUTCHAR\n");
		#endif
		return;
	}
}

void kbd_arch_destroy(void)
{
	cellKbEnd();
	return;
}

void kbd_process(void)
{
	static CellKbData kdata;

	if (cellKbGetInfo (&status) != CELL_KB_OK)
		return;

	if((status.info & CELL_KB_INFO_INTERCEPTED) && (!(old_info & CELL_KB_INFO_INTERCEPTED)))
	{
		#ifdef CELL_DEBUG
		printf("INFO: Lost Keyboard\n");
		#endif
		old_info = status.info;
	}
	#ifdef CELL_DEBUG
	else if((!(status.info & CELL_KB_INFO_INTERCEPTED)) && (old_info & CELL_KB_INFO_INTERCEPTED))
	{
		printf("INFO: Found Keyboard\n");
		old_info = status.info;
	}
	#endif

	if ( (old_info != CELL_KB_STATUS_DISCONNECTED) && (status.info == CELL_KB_STATUS_DISCONNECTED))
	{
		#ifdef CELL_DEBUG
		printf("Keyboard has disconnected\n");
		#endif
		old_info = status.info;
		return;
	}

	#ifdef CELL_DEBUG
	if (old_status == CELL_KB_STATUS_DISCONNECTED)
	{
		printf("Keyboard has connected\n");
	}
	#endif

	if (cellKbRead (0, &kdata) != CELL_KB_OK)
		return;

	if ( (kdata.len == 0) )   // even the mkey shows kdata.len == 1
		return;

	// First, check for modifier keys and apply them.

	if ( !(mkey & CELL_KB_MKEY_L_CTRL) && (kdata.mkey & CELL_KB_MKEY_L_CTRL))
		keyboard_key_pressed (L_CTRL);

	if ((mkey & CELL_KB_MKEY_L_CTRL) && !(kdata.mkey & CELL_KB_MKEY_L_CTRL))
		keyboard_key_released (L_CTRL);

	if ( !(mkey & CELL_KB_MKEY_L_SHIFT) && (kdata.mkey & CELL_KB_MKEY_L_SHIFT))
		keyboard_key_pressed (L_SHIFT);

	if ((mkey & CELL_KB_MKEY_L_SHIFT) && !(kdata.mkey & CELL_KB_MKEY_L_SHIFT))
		keyboard_key_released (L_SHIFT);

	if ( !(mkey & CELL_KB_MKEY_L_ALT) && (kdata.mkey & CELL_KB_MKEY_L_ALT))
		keyboard_key_pressed (L_ALT);

	if ((mkey & CELL_KB_MKEY_L_ALT) && !(kdata.mkey & CELL_KB_MKEY_L_ALT))
		keyboard_key_released (L_ALT);

	if ( !(mkey & CELL_KB_MKEY_L_WIN) && (kdata.mkey & CELL_KB_MKEY_L_WIN))
		keyboard_key_pressed (L_WIN);

	if ((mkey & CELL_KB_MKEY_L_WIN) && !(kdata.mkey & CELL_KB_MKEY_L_WIN))
		keyboard_key_released (L_WIN);

	if ( !(mkey & CELL_KB_MKEY_R_CTRL) && (kdata.mkey & CELL_KB_MKEY_R_CTRL))
		keyboard_key_pressed (R_CTRL);

	if ((mkey & CELL_KB_MKEY_R_CTRL) && !(kdata.mkey & CELL_KB_MKEY_R_CTRL))
		keyboard_key_released (R_CTRL);

	if ( !(mkey & CELL_KB_MKEY_R_SHIFT) && (kdata.mkey & CELL_KB_MKEY_R_SHIFT))
		keyboard_key_pressed (R_SHIFT);

	if ((mkey & CELL_KB_MKEY_R_SHIFT) && !(kdata.mkey & CELL_KB_MKEY_R_SHIFT))
		keyboard_key_released (R_SHIFT);

	if ( !(mkey & CELL_KB_MKEY_R_ALT) && (kdata.mkey & CELL_KB_MKEY_R_ALT))
		keyboard_key_pressed (R_ALT);

	if ((mkey & CELL_KB_MKEY_R_ALT) && !(kdata.mkey & CELL_KB_MKEY_R_ALT))
		keyboard_key_released (R_ALT);

	if ( !(mkey & CELL_KB_MKEY_R_WIN) && (kdata.mkey & CELL_KB_MKEY_R_WIN))
		keyboard_key_pressed (R_WIN);

	if ((mkey & CELL_KB_MKEY_R_WIN) && !(kdata.mkey & CELL_KB_MKEY_R_WIN))
		keyboard_key_released (R_WIN);

	mkey = kdata.mkey;


	if (kdata.len == 0)
		return;

	int i, j;
	uint16_t kcode;
	for (j = 0; j < kdata.len; j++)
	{
#ifdef CELL_DEBUG
		printf ("kdata.len is %d, mkey is %x\n", kdata.len, kdata.mkey);
#endif

		if (kdata.keycode[j] == 0x8039)    //caps lock
			continue;

		if (kdata.keycode[j] & CELL_KB_KEYPAD)
			kcode = kdata.keycode[j] & ~CELL_KB_KEYPAD;
		else
			kcode = kdata.keycode[j];

#ifdef CELL_DEBUG
		printf ("orig keycode = 0x%x\n", kdata.keycode[j]);
#endif

		if (kcode == 0x00)
		{
			//printf("Detected 0x00. Releasing ALL keys\n");
			// Release all keys
			for (i=0; i<MAX_KEYS; i++)
			{
				if (keysdown[i] != 0x00)
				{
#ifdef CELL_DEBUG
					printf("detected keyUP kcode '%d'\n", keysdown[i]);
#endif
					keyboard_key_released((signed long)keysdown[i]);
					keysdown[i] = 0x00;
				}
			}
			continue;
		}
	}

	// Find the keys that need to be released
	for (i=0; i < MAX_KEYS; i++)
	{
		bool found=false;

		if (keysdown[i] == 0x00)
			continue;

		for (j=0; j < kdata.len; j++)
		{
			if (kdata.keycode[j] & CELL_KB_KEYPAD)
				kcode = kdata.keycode[j] & ~CELL_KB_KEYPAD;
			else 
				kcode = kdata.keycode[j];

			if (keysdown[i] == kcode)
				found=true;
		}

		if (found==false)
		{
			// This key isn't pressed anymore.
#ifdef CELL_DEBUG
			printf("detected keyUP kcode '%d'\n", keysdown[i]);
#endif
			keyboard_key_released((signed long)keysdown[i]);
			keysdown[i] = 0x00;
		}
	}


	// Find the keys that need to be pressed
	for (j = 0; j < kdata.len; j++)
	{
		bool found=false;
		if (kdata.keycode[j] & CELL_KB_KEYPAD)
			kcode = kdata.keycode[j] & ~CELL_KB_KEYPAD;
		else 
			kcode = kdata.keycode[j];

		for (i=0; i < MAX_KEYS; i++)
		{
			if (kcode == keysdown[i])
			{
				found=true;
				break;
			}
		}

		if (found)
			continue;
		else
		{
			// find a slot to record it, and press the key
			for (i=0; i < MAX_KEYS; i++)
			{
				if (keysdown[i]==0)
				{
					keysdown[i] = kcode;
#ifdef CELL_DEBUG
					printf("detected keyDOWN kcode '%d'\n", keysdown[i]);
#endif
					keyboard_key_pressed((signed long)keysdown[i]);
					break;
				}
			}
		}
	}

}

signed long kbd_arch_keyname_to_keynum(char *keyname)
{
	return (signed long)atoi(keyname);
}

const char *kbd_arch_keynum_to_keyname(signed long keynum)
{
	static char keyname[20];

	memset(keyname, 0, 20);

	sprintf(keyname, "%li", keynum);

	return keyname;
}

void kbd_initialize_numpad_joykeys(int* joykeys)
{
}

#define MAX_BUFFER 64
static int inputbuffer[MAX_BUFFER];
int osk_active_bufferlen=0;

void osk_kbd_append_buffer(const char *keystring)
{
	for (unsigned int i=0; i < strlen(keystring); i++)
	{
		if (osk_active_bufferlen >= MAX_BUFFER)
		{
#ifdef CELL_DEBUG
			printf("WARNING: Keystring inputbufer overflow\n");
#endif
			return;
		}

		inputbuffer[osk_active_bufferlen++] = keystring[i];
	}
}

void osk_kbd_append_buffer_char (int keycode)
{
#ifdef CELL_DEBUG
	printf("appending char keycode %d\n", keycode);
#endif
	inputbuffer[osk_active_bufferlen++] = keycode;
}


void osk_kbd_type_key(void)
{
	// This will be called once per vsync.

	static int keystroke=0;    // represents the position in the buffer
	static int keydown=0;
	static int countdown=8;    // The number of cycles the key should be held down in order to register a keystroke

	if (keystroke > osk_active_bufferlen)
	{
		// End of the buffer. 
		// Reset the buffer
		osk_active_bufferlen=0;
		keystroke = 0;
	}

	if (osk_active_bufferlen == 0)
		return;

	if (!keydown)
	{
		// Press a key

		switch (inputbuffer[keystroke])
		{
			case '\n':
				keyboard_key_pressed((signed long) 10 );    // Enter
				break;
			default:
				keyboard_key_pressed((signed long) inputbuffer[keystroke] );
				break;
		}
		keydown=1;
	}
	else 
	{
		if (countdown == 0)
		{
			// Release a key
			switch (inputbuffer[keystroke])
			{
				case '\n':
					keyboard_key_released((signed long) 10 );    // Enter
					break;
				default:
					keyboard_key_released((signed long) inputbuffer[keystroke]);
					break;
			}
			keydown=0;
			// Advance to the next keystroke
			keystroke++;

			// Reset the countdown for the next character
			countdown=8;
		}
		else
			countdown--;
	}
}

