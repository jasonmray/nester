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

#include "win32_directory.h"
#include "debug.h"

static int createPathRecurse(const char* path)
{
  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char fname[_MAX_FNAME];
  char ext[_MAX_EXT];
  char new_path[_MAX_PATH];
  char directory_to_make[_MAX_DIR];

//  LOG("createPathRecurse " << path << endl);

  // can we get to the directory?
  if(0 != SetCurrentDirectory(path))
  {
//    LOG("dir " << path << " exists" << endl);
    return 0;
  }

  // chop off a directory and try to get there
  _splitpath(path, drive, dir, fname, ext);

  // if dir is empty, fail
  if(!strcmp(dir, "") || !strcmp(dir, "\\")) return -1;

  int i = strlen(dir)-1;
  // get rid of trailing slash
  if(i >= 0) if(dir[i] == '\\') dir[i--] = 0;
  while(i >= 0)
  {
    if(dir[i] == '\\') break;
    i--;
  }
  strcpy(directory_to_make, &dir[i+1]);
  dir[i+1] = 0;

  _makepath(new_path, drive, dir, "", "");

  if(0 != createPathRecurse(new_path)) return -1;

//  LOG("createPathRecurse create directory " << directory_to_make << endl);
  if(0 == CreateDirectory(directory_to_make, NULL)) return -1;
  if(0 == SetCurrentDirectory(directory_to_make)) return -1;
  return 0;
}

int DIR_createPath(const char* path)
{
  char currentDir[_MAX_PATH];
  char temp[_MAX_PATH];
  int retval = 0;

  GetCurrentDirectory(sizeof(currentDir), currentDir);

  strcpy(temp, path);
  if(temp[strlen(temp)-1] != '\\') strcat(temp, "\\");
  retval = createPathRecurse(temp);

  SetCurrentDirectory(currentDir);

  return retval;
}

void DIR_createFileName(char* buf, const char* basePath, const char* relativePath,
                        const char* fileName, const char* fileExtension)
{
  char currentDir[_MAX_PATH];
  GetCurrentDirectory(sizeof(currentDir), currentDir);

  strcpy(buf, basePath);
  if(!DIR_createPath(buf))
  {
    // process relative path
    SetCurrentDirectory(buf);
    if(_fullpath(buf, relativePath, _MAX_PATH) == NULL)
    {
//      LOG("_fullpath() failed" << endl);
      strcpy(buf, basePath);
    }
    else
    {
      if(DIR_createPath(buf))
      {
//        LOG("createPath failed" << endl);
        strcpy(buf, basePath);
      }
    }
  }

  if(buf[strlen(buf)-1] != '\\') strcat(buf, "\\");

  strcat(buf, fileName);
  strcat(buf, fileExtension);

  SetCurrentDirectory(currentDir);
}

void DIR_createNesFileName(NES_ROM* rom, char* buf, NES_preferences_settings::SAVE_DIR_TYPE dirType,
                           const char* relativePath, const char* fileName,
                           const char* fileExtension)
{
  const char* basePath = "";
  char temp[_MAX_PATH];

  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char fname[_MAX_FNAME];
  char ext[_MAX_EXT];

  // figure out which directory we're working off of
  switch(dirType)
  {
    case NES_preferences_settings::ROM_DIR:
      basePath = rom->GetRomPath();
      break;
    case NES_preferences_settings::NESTER_DIR:
      GetModuleFileName(NULL, temp, sizeof(temp));
      // strip off filename
      _splitpath(temp, drive, dir, fname, ext);
      _makepath(temp, drive, dir, "", "");
      basePath = temp;
      break;
  }

  DIR_createFileName(buf, basePath, relativePath, fileName, fileExtension);

  LOG("created filename " << buf << endl);
}
