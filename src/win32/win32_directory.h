/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

#ifndef _WIN32_DIRECTORY_H_
#define _WIN32_DIRECTORY_H_

#include "NES_settings.h"
#include "NES_ROM.h"

extern int DIR_createPath(const char* path);
extern void DIR_createFileName(char* buf, const char* basePath,
                               const char* relativePath,
                               const char* fileName, const char* fileExtension);
extern void DIR_createNesFileName(NES_ROM* rom, char* buf, NES_preferences_settings::SAVE_DIR_TYPE dirType,
                                  const char* relativePath, const char* fileName,
                                  const char* fileExtension);


#endif
