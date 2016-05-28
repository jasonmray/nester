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

#ifndef _NES_H_
#define _NES_H_

#include <stdio.h>

#include "types.h"
#include "emulator.h"
#include "NES_6502.h"
#include "NES_mapper.h"
#include "NES_ROM.h"
#include "NES_PPU.h"
#include "NES_APU_wrapper.h"
#include "NES_pad.h"
#include "NES_settings.h"

#include "libsnss.h"

class NES_screen_mgr;

class NES : public emulator
{
  // friend classes
  friend NES_screen_mgr;
  friend NES_6502;
  friend NES_PPU;
  friend NES_APU;
  friend NES_mapper;
  friend NES_mapper4;
  friend NES_mapper24;
  friend NES_mapper40;

  // SNSS friend functions
  friend void adopt_BASR(SnssBaseBlock* block, NES* nes);
  friend void adopt_VRAM(SnssVramBlock* block, NES* nes);
  friend void adopt_SRAM(SnssSramBlock* block, NES* nes);
  friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
  friend void adopt_SOUN(SnssSoundBlock* block, NES* nes);
  friend int extract_BASR(SnssBaseBlock* block, NES* nes);
  friend int extract_VRAM(SnssVramBlock* block, NES* nes);
  friend int extract_SRAM(SnssSramBlock* block, NES* nes);
  friend int extract_MPRD(SnssMapperBlock* block, NES* nes);
  friend int extract_SOUN(SnssSoundBlock* block, NES* nes);

public:
  NES(const char* ROM_name, NES_screen_mgr* _screen_mgr, sound_mgr* _sound_mgr);
  ~NES();

  void new_snd_mgr(sound_mgr* _sound_mgr);

  void set_pad1(controller* c) { pad1 = (NES_pad*)c; }
  void set_pad2(controller* c) { pad2 = (NES_pad*)c; }

  boolean emulate_frame(boolean draw);

  void reset();

  const char* getROMname();
  const char* getROMnameExt();
  const char* getROMpath();
  NES_ROM* get_NES_ROM();

  boolean loadState(const char* fn);
  boolean saveState(const char* fn);

  void calculate_palette();

  uint8 getBGColor() { return ppu->getBGColor(); }

  enum {
    NES_NUM_VBLANK_LINES = 20,
    NES_NUM_FRAME_LINES = 240,

    // these are 0-based, and actions occur at start of line
    NES_NMI_LINE = 241,
    NES_VBLANK_FLAG_SET_LINE = 241,
    NES_VBLANK_FLAG_RESET_LINE = 261,
    NES_SPRITE0_FLAG_RESET_LINE = 261,

    NES_COLOR_BASE = 0x40, // NES palette is set starting at color 0x40 (64)
    NES_NUM_COLORS = 64    // 64 colors in the NES palette
  };

  static const float CYCLES_PER_LINE;

protected:
  uint8 NES_RGB_pal[NES_NUM_COLORS][3];
  static const uint8 NES_preset_palette[NES_NUM_COLORS][3];

  NES_screen_mgr* scr_mgr;
  sound_mgr* snd_mgr;
  NES_6502* cpu;
  NES_PPU* ppu;
  NES_APU* apu;
  NES_ROM* ROM;
  NES_mapper* mapper;

  NES_settings settings;

  float  ideal_cycle_count;   // number of cycles that should have executed so far
  uint32 emulated_cycle_count;  // number of cycles that have executed so far

  // internal memory
  uint8 RAM[0x800];
  uint8 SaveRAM[0x2000];

  // joypad stuff
  NES_pad* pad1;
  NES_pad* pad2;
  boolean  pad_strobe;
  uint8 pad1_bits;
  uint8 pad2_bits;

  void loadROM(const char* fn);
  void freeROM();

  void onFreeze();
  void onThaw();

  // these are called by the CPU
  uint8 MemoryRead(uint32 addr);
  void  MemoryWrite(uint32 addr, uint8 data);

  // internal read/write functions
  uint8 ReadRAM(uint32 addr);
  void  WriteRAM(uint32 addr, uint8 data);
  
  uint8 ReadLowRegs(uint32 addr);
  void  WriteLowRegs(uint32 addr, uint8 data);
  
  uint8 ReadHighRegs(uint32 addr);
  void  WriteHighRegs(uint32 addr, uint8 data);

  void  emulate_CPU_cycles(float num_cycles);
  void  trim_cycle_counts();

  // file stuff
  void Save_SaveRAM();
  void Load_SaveRAM();

private:
};

#endif
