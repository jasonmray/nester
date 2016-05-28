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

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "NES_ROM.h"

#include "debug.h"

NES_ROM::NES_ROM(const char* fn)
{
  FILE* fp;

  fp         = NULL;

  trainer    = NULL;
  ROM_banks  = NULL;
  VROM_banks = NULL;

  rom_name = NULL;
  rom_path = NULL;

  try 
  {
    // store filename and path
    rom_name = (char*)malloc(strlen(fn)+1);
    rom_name_ext = (char*)malloc(strlen(fn)+1);
    rom_path = (char*)malloc(strlen(fn)+1);
    if(!rom_name || !rom_name_ext || !rom_path)
      throw "Error loading ROM: out of memory";

    GetPathInfo(fn);

    fp = fopen(fn, "rb");
    if(fp == NULL)
      throw "Error opening ROM file";

    if(fread((void*)&header, sizeof(struct NES_header), 1, fp) != 1)
      throw "Error reading from NES ROM";

    if(strncmp((const char*)header.id, "NES", 3) || (header.ctrl_z != 0x1A))
      throw "Invalid NES file";

    // allocate memory
    ROM_banks = (uint8*)malloc(header.num_16k_rom_banks * (16*1024));
    if(!ROM_banks) throw "Out of memory";

    VROM_banks = (uint8*)malloc(header.num_8k_vrom_banks * (8*1024));
    if(!VROM_banks) throw "Out of memory";

    // load trainer if present
    if(has_trainer())
    {
      trainer = (uint8*)malloc(TRAINER_LEN);
      if(!trainer) throw "Out of memory";

      if(fread(trainer, TRAINER_LEN, 1, fp) != 1)
        throw "Error reading trainer from NES ROM";
    }

    if(fread(ROM_banks,(16*1024),header.num_16k_rom_banks,fp) != header.num_16k_rom_banks) 
      throw "Error reading ROM banks from NES ROM";

    if(fread(VROM_banks,(8*1024),header.num_8k_vrom_banks,fp) != header.num_8k_vrom_banks) 
      throw "Error reading VROM banks from NES ROM";

    fclose(fp);

  } catch(...) {
    if(fp)          fclose(fp);

    if(VROM_banks)  free(VROM_banks);
    if(ROM_banks)   free(ROM_banks);
    if(trainer)     free(trainer);

    if(rom_name)     free(rom_name);
    if(rom_name_ext) free(rom_name_ext);
    if(rom_path)     free(rom_path);
    throw;
  }

  // figure out mapper number
  mapper = (header.flags_1 >> 4);

  // if there is anything in the reserved bytes,
  // don't trust the high nybble of the mapper number
  for(uint32 i = 0; i < sizeof(header.reserved); i++)
  {
    if(header.reserved[i] != 0x00) return;
  }
  mapper |= (header.flags_2 & 0xF0);

}

NES_ROM::~NES_ROM()
{
  if(VROM_banks)  free(VROM_banks);
  if(ROM_banks)   free(ROM_banks);
  if(trainer)     free(trainer);
  if(rom_name)     free(rom_name);
  if(rom_name_ext) free(rom_name_ext);
  if(rom_path)     free(rom_path);
}

void NES_ROM::GetPathInfo(const char* fn)
{
  // find index of first letter of actual ROM file name (after path)
  uint32 i = strlen(fn); // start at end of string

  while(1)
  {
    // look for directory delimiter
    if((fn[i] == '\\') || (fn[i] == '/'))
    {
      i++;
      break;
    }

    i--;
    if(!i) break;
  }

  // copy rom name w/o extension
  {
    uint32 j = i;
    uint32 a = 0;

    // copy up to period
    while(1)
    {
      if(!fn[j]) break;
      if(fn[j] == '.') break;

      rom_name[a] = fn[j];

      a++;
      j++;
    }

    // terminate rom name string
    rom_name[a] = '\0';
  }

  // copy rom name w/ extension
  {
    uint32 j = i;
    uint32 a = 0;

    // copy up to period
    while(1)
    {
      if(!fn[j]) break;

      rom_name_ext[a] = fn[j];

      a++;
      j++;
    }

    // terminate rom name string
    rom_name_ext[a] = '\0';
  }

  // copy rom path
  {
    uint32 j = 0;

    // copy up to rom file name
    while(j < i)
    {
      rom_path[j] = fn[j];
      j++;
    }

    // terminate rom path string
    rom_path[i] = '\0';
  }

}