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

#ifndef _NES_MAPPER_H_
#define _NES_MAPPER_H_

#include "NES_ROM.h"
#include "debug.h"

class NES;  // class prototypes
class NES_mapper;
class NES_mapper4;

/////////////////////////////////////////////////////////////////////
// mapper factory
NES_mapper* GetMapper(NES* parent, NES_ROM* rom);
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper virtual base class
class NES_mapper
{
public:
  NES_mapper(NES* parent);
  virtual ~NES_mapper() {};

  virtual void  Reset() = 0;

  virtual void  MemoryWrite(uint32 addr, uint8 data)        {}
  virtual void  MemoryWriteLow(uint32 addr, uint8 data)     {}
  virtual void  MemoryWriteSaveRAM(uint32 addr, uint8 data) {}

  virtual void  HSync(uint32 scanline)  {}
  virtual void  VSync()  {}

  // for mmc2
  virtual void  PPU_Latch_FDFE(uint32 addr) {}

protected:
  NES* parent_NES;

  uint32 num_16k_ROM_banks;
  uint32 num_8k_ROM_banks;
  uint32 num_1k_VROM_banks;

  uint8* ROM_banks;
  uint8* VROM_banks;

  uint32 ROM_mask;
  uint32 VROM_mask;

  void set_CPU_banks(uint32 bank4_num, uint32 bank5_num,
                     uint32 bank6_num, uint32 bank7_num);
  void set_CPU_bank4(uint32 bank_num);
  void set_CPU_bank5(uint32 bank_num);
  void set_CPU_bank6(uint32 bank_num);
  void set_CPU_bank7(uint32 bank_num);

  // for mapper 40
  void set_CPU_banks(uint32 bank3_num,
                     uint32 bank4_num, uint32 bank5_num,
                     uint32 bank6_num, uint32 bank7_num);
  void set_CPU_bank3(uint32 bank_num);

  void set_PPU_banks(uint32 bank0_num, uint32 bank1_num,
                     uint32 bank2_num, uint32 bank3_num,
                     uint32 bank4_num, uint32 bank5_num,
                     uint32 bank6_num, uint32 bank7_num);
  void set_PPU_bank0(uint32 bank_num);
  void set_PPU_bank1(uint32 bank_num);
  void set_PPU_bank2(uint32 bank_num);
  void set_PPU_bank3(uint32 bank_num);
  void set_PPU_bank4(uint32 bank_num);
  void set_PPU_bank5(uint32 bank_num);
  void set_PPU_bank6(uint32 bank_num);
  void set_PPU_bank7(uint32 bank_num);

  void set_mirroring(uint32 nt0, uint32 nt1, uint32 nt2, uint32 nt3);
  void set_mirroring(NES_PPU::mirroring_type m);

private:
};
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper 0
class NES_mapper0 : public NES_mapper
{
public:
  NES_mapper0(NES* parent) : NES_mapper(parent) {}
  ~NES_mapper0() {}

  void  Reset();

protected:
private:
};
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper 1
class NES_mapper1 : public NES_mapper
{
  friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
  friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
  NES_mapper1(NES* parent) : NES_mapper(parent) {}
  ~NES_mapper1() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);
protected:
  // this uses MMC1_256K_base and MMC1_bankX
  void MMC1_set_CPU_banks();

  uint32 write_count;
  uint8  bits;
  uint8  regs[4];
  uint32 last_write_addr;

  enum MMC1_Size_t
  {
    MMC1_SMALL,
    MMC1_512K,
    MMC1_1024K
  };

  MMC1_Size_t MMC1_Size;
  uint32 MMC1_256K_base;
  uint32 MMC1_swap;

  // these are the 4 ROM banks currently selected
  uint32 MMC1_bank1;
  uint32 MMC1_bank2;
  uint32 MMC1_bank3;
  uint32 MMC1_bank4;

  uint32 MMC1_HI1;
  uint32 MMC1_HI2;
private:
};
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper 2
class NES_mapper2 : public NES_mapper
{
public:
  NES_mapper2(NES* parent) : NES_mapper(parent) {}
  ~NES_mapper2() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);
protected:
private:
};
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper 3
class NES_mapper3 : public NES_mapper
{
public:
  NES_mapper3(NES* parent) : NES_mapper(parent) {}
  ~NES_mapper3() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);
protected:
private:
};
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper 4

class NES_mapper4 : public NES_mapper
{
  friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
  friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
  NES_mapper4(NES* parent) : NES_mapper(parent) {}
  ~NES_mapper4() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);

  void  HSync(uint32 scanline);
protected:
  uint8  regs[8];

  uint32 prg0,prg1;
  uint32 chr01,chr23,chr4,chr5,chr6,chr7;

  uint32 chr_swap() { return regs[0] & 0x80; }
  uint32 prg_swap() { return regs[0] & 0x40; }

  uint8 irq_enabled; // IRQs enabled
  uint8 irq_counter; // IRQ scanline counter, decreasing
  uint8 irq_latch;   // IRQ scanline counter latch

  void MMC3_set_CPU_banks()
  {
    if(prg_swap())
    {
      set_CPU_banks(num_8k_ROM_banks-2,prg1,prg0,num_8k_ROM_banks-1);
    }
    else
    {
      set_CPU_banks(prg0,prg1,num_8k_ROM_banks-2,num_8k_ROM_banks-1);
    }
  }

  void MMC3_set_PPU_banks()
  {
    if(num_1k_VROM_banks)
    {
      if(chr_swap())
      {
        set_PPU_banks(chr4,chr5,chr6,chr7,chr01,chr01+1,chr23,chr23+1);
      }
      else
      {
        set_PPU_banks(chr01,chr01+1,chr23,chr23+1,chr4,chr5,chr6,chr7);
      }
    }
  }

  void SNSS_fixup(); // HACK HACK HACK HACK

private:
};

/////////////////////////////////////////////////////////////////////
// Mapper 7
class NES_mapper7 : public NES_mapper
{
public:
  NES_mapper7(NES* parent) : NES_mapper(parent) {}
  ~NES_mapper7() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);
protected:
private:
};
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper 9
class NES_mapper9 : public NES_mapper
{
  friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
  friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
  NES_mapper9(NES* parent) : NES_mapper(parent) {}
  ~NES_mapper9() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);

  void  PPU_Latch_FDFE(uint32 addr);

protected:
  uint8 regs[6];
  uint8 latch_0000;
  uint8 latch_1000;

  void set_VROM_0000();
  void set_VROM_1000();

  void SNSS_fixup(); // HACK HACK HACK HACK

private:
};
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper 40 (smb2j)
class NES_mapper40 : public NES_mapper
{
  friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
  friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
  NES_mapper40(NES* parent) : NES_mapper(parent) {}
  ~NES_mapper40() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);

  void  HSync(uint32 scanline);
protected:
  uint8 irq_enabled;
  uint32 lines_to_irq;
private:
};
/////////////////////////////////////////////////////////////////////

#include "NES_mapper_Konami.h"

/////////////////////////////////////////////////////////////////////

#endif
