/******************************************************************************* 
 *  -- ui_snapshot.h - Snapshot interface
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
 *
 *  Copyright (C) 2010
 *  Created on: Dec 23, 2010
 *  Author:      TimRex
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


#ifndef UI_SNAPSHOT_H_
#define UI_SNAPSHOT_H_

bool snapshot_exists(const char *romfile);
int  load_snapshot(const char *romfile);
int  save_snapshot(const char *romfile);
  
#endif /* UI_SNAPSHOT_H_ */
