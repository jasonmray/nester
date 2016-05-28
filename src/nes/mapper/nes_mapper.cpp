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

#define _NES_MAPPER_CPP_

#include "NES_mapper.h"
#include "NES.h"

#include "debug.h"

#define MASK_BANK(bank,mask) (bank) = ((bank) & (mask))

#ifdef NESTER_DEBUG
  #define VALIDATE_ROM_BANK(bank) \
    MASK_BANK(bank,ROM_mask); \
    ASSERT((bank) < num_8k_ROM_banks) \
    if((bank) >= num_8k_ROM_banks) \
    { \
      LOG("Illegal ROM bank switch: " << (int)(bank) << "/" << (int)num_8k_ROM_banks << endl); \
      return; \
    }

  #define VALIDATE_VROM_BANK(bank) \
    MASK_BANK(bank,VROM_mask); \
    ASSERT((bank) < num_1k_VROM_banks) \
    if((bank) >= num_1k_VROM_banks) \
    { \
      LOG("Illegal VROM bank switch: " << (int)(bank) << "/" << (int)num_1k_VROM_banks << endl); \
      return; \
    }
#else
  #define VALIDATE_ROM_BANK(bank) \
    MASK_BANK(bank,ROM_mask); \
    if((bank) >= num_8k_ROM_banks) return;
  #define VALIDATE_VROM_BANK(bank) \
    MASK_BANK(bank,VROM_mask); \
    if((bank) >= num_1k_VROM_banks) return;
#endif

/////////////////////////////////////////////////////////////////////
// Mapper virtual base class
NES_mapper::NES_mapper(NES* parent) : parent_NES(parent)
{
  uint32 probe;

  //num_16k_ROM_banks = parent_NES->ROM->get_num_16k_ROM_banks();
  num_8k_ROM_banks = 2 * parent_NES->ROM->get_num_16k_ROM_banks();
  num_1k_VROM_banks = 8 * parent_NES->ROM->get_num_8k_VROM_banks();

  ROM_banks  = parent_NES->ROM->get_ROM_banks();
  VROM_banks = parent_NES->ROM->get_VROM_banks();

  ROM_mask  = 0xFFFF;
  VROM_mask = 0xFFFF;

  for(probe = 0x8000; probe; probe >>= 1)
  {
    if((num_8k_ROM_banks-1) & probe) break;
    ROM_mask >>= 1;
  }
  for(probe = 0x8000; probe; probe >>= 1)
  {
    if((num_1k_VROM_banks-1) & probe) break;
    VROM_mask >>= 1;
  }

//  LOG(HEX(ROM_mask,2) << " " << HEX(VROM_mask,2) << endl);
}

void NES_mapper::set_CPU_banks(uint32 bank4_num, uint32 bank5_num,
                               uint32 bank6_num, uint32 bank7_num)
{
  NES_6502::Context context;

  VALIDATE_ROM_BANK(bank4_num);
  VALIDATE_ROM_BANK(bank5_num);
  VALIDATE_ROM_BANK(bank6_num);
  VALIDATE_ROM_BANK(bank7_num);

/*
  LOG("Setting CPU banks " << bank4_num << " " << bank5_num << " " <<
                              bank6_num << " " << bank7_num << endl);
*/

  parent_NES->cpu->GetContext(&context);
  context.mem_page[4] = ROM_banks + (bank4_num << 13); // * 0x2000
  context.mem_page[5] = ROM_banks + (bank5_num << 13);
  context.mem_page[6] = ROM_banks + (bank6_num << 13);
  context.mem_page[7] = ROM_banks + (bank7_num << 13);
  parent_NES->cpu->SetContext(&context);
}

void NES_mapper::set_CPU_bank4(uint32 bank_num)
{
  NES_6502::Context context;

  VALIDATE_ROM_BANK(bank_num);

  parent_NES->cpu->GetContext(&context);
  context.mem_page[4] = ROM_banks + (bank_num << 13); // * 0x2000
  parent_NES->cpu->SetContext(&context);
}

void NES_mapper::set_CPU_bank5(uint32 bank_num)
{
  NES_6502::Context context;

  VALIDATE_ROM_BANK(bank_num);

  parent_NES->cpu->GetContext(&context);
  context.mem_page[5] = ROM_banks + (bank_num << 13); // * 0x2000
  parent_NES->cpu->SetContext(&context);
}

void NES_mapper::set_CPU_bank6(uint32 bank_num)
{
  NES_6502::Context context;

  VALIDATE_ROM_BANK(bank_num);

  parent_NES->cpu->GetContext(&context);
  context.mem_page[6] = ROM_banks + (bank_num << 13); // * 0x2000
  parent_NES->cpu->SetContext(&context);
}

void NES_mapper::set_CPU_bank7(uint32 bank_num)
{
  NES_6502::Context context;

  VALIDATE_ROM_BANK(bank_num);

  parent_NES->cpu->GetContext(&context);
  context.mem_page[7] = ROM_banks + (bank_num << 13); // * 0x2000
  parent_NES->cpu->SetContext(&context);
}

// for mapper 40 /////////////////////////////////////////////////////////
void NES_mapper::set_CPU_banks(uint32 bank3_num,
                               uint32 bank4_num, uint32 bank5_num,
                               uint32 bank6_num, uint32 bank7_num)
{
  NES_6502::Context context;

  VALIDATE_ROM_BANK(bank3_num);
  VALIDATE_ROM_BANK(bank4_num);
  VALIDATE_ROM_BANK(bank5_num);
  VALIDATE_ROM_BANK(bank6_num);
  VALIDATE_ROM_BANK(bank7_num);

  parent_NES->cpu->GetContext(&context);
  context.mem_page[3] = ROM_banks + (bank3_num << 13); // * 0x2000
  context.mem_page[4] = ROM_banks + (bank4_num << 13);
  context.mem_page[5] = ROM_banks + (bank5_num << 13);
  context.mem_page[6] = ROM_banks + (bank6_num << 13);
  context.mem_page[7] = ROM_banks + (bank7_num << 13);
  parent_NES->cpu->SetContext(&context);
}

void NES_mapper::set_CPU_bank3(uint32 bank_num)
{
  NES_6502::Context context;

  VALIDATE_ROM_BANK(bank_num);

  parent_NES->cpu->GetContext(&context);
  context.mem_page[3] = ROM_banks + (bank_num << 13); // * 0x2000
  parent_NES->cpu->SetContext(&context);
}
//////////////////////////////////////////////////////////////////////////


void NES_mapper::set_PPU_banks(uint32 bank0_num, uint32 bank1_num,
                               uint32 bank2_num, uint32 bank3_num,
                               uint32 bank4_num, uint32 bank5_num,
                               uint32 bank6_num, uint32 bank7_num)
{
  VALIDATE_VROM_BANK(bank0_num);
  VALIDATE_VROM_BANK(bank1_num);
  VALIDATE_VROM_BANK(bank2_num);
  VALIDATE_VROM_BANK(bank3_num);
  VALIDATE_VROM_BANK(bank4_num);
  VALIDATE_VROM_BANK(bank5_num);
  VALIDATE_VROM_BANK(bank6_num);
  VALIDATE_VROM_BANK(bank7_num);

  parent_NES->ppu->PPU_VRAM_banks[0] = VROM_banks + (bank0_num << 10); // * 0x400
  parent_NES->ppu->PPU_VRAM_banks[1] = VROM_banks + (bank1_num << 10);
  parent_NES->ppu->PPU_VRAM_banks[2] = VROM_banks + (bank2_num << 10);
  parent_NES->ppu->PPU_VRAM_banks[3] = VROM_banks + (bank3_num << 10);
  parent_NES->ppu->PPU_VRAM_banks[4] = VROM_banks + (bank4_num << 10);
  parent_NES->ppu->PPU_VRAM_banks[5] = VROM_banks + (bank5_num << 10);
  parent_NES->ppu->PPU_VRAM_banks[6] = VROM_banks + (bank6_num << 10);
  parent_NES->ppu->PPU_VRAM_banks[7] = VROM_banks + (bank7_num << 10);
}

void NES_mapper::set_PPU_bank0(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[0] = VROM_banks + (bank_num << 10); // * 0x400
}

void NES_mapper::set_PPU_bank1(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[1] = VROM_banks + (bank_num << 10); // * 0x400
}

void NES_mapper::set_PPU_bank2(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[2] = VROM_banks + (bank_num << 10); // * 0x400
}

void NES_mapper::set_PPU_bank3(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[3] = VROM_banks + (bank_num << 10); // * 0x400
}

void NES_mapper::set_PPU_bank4(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[4] = VROM_banks + (bank_num << 10); // * 0x400
}

void NES_mapper::set_PPU_bank5(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[5] = VROM_banks + (bank_num << 10); // * 0x400
}

void NES_mapper::set_PPU_bank6(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[6] = VROM_banks + (bank_num << 10); // * 0x400
}

void NES_mapper::set_PPU_bank7(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[7] = VROM_banks + (bank_num << 10); // * 0x400
}


void NES_mapper::set_mirroring(uint32 nt0, uint32 nt1, uint32 nt2, uint32 nt3)
{
  ASSERT(nt0 < 4);
  ASSERT(nt1 < 4);
  ASSERT(nt2 < 4);
  ASSERT(nt3 < 4);

  parent_NES->ppu->set_mirroring(nt0,nt1,nt2,nt3);
}

void NES_mapper::set_mirroring(NES_PPU::mirroring_type m)
{
  parent_NES->ppu->set_mirroring(m);
}
/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
// Mapper 0
void NES_mapper0::Reset()
{
  // set CPU bank pointers
  if(num_8k_ROM_banks > 2)
  {
    set_CPU_banks(0,1,2,3);
  }
  else if(num_8k_ROM_banks > 1)
  {
    set_CPU_banks(0,1,0,1);
  }
  else
  {
    set_CPU_banks(0,0,0,0);
  }

  if(num_1k_VROM_banks)
  {
    set_PPU_banks(0,1,2,3,4,5,6,7);
  }
}
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper 1
void NES_mapper1::Reset()
{
  write_count = 0;
  bits = 0x00;
  regs[0] = 0x0C; // reflects initial ROM state
  regs[1] = 0x00;
  regs[2] = 0x00;
  regs[3] = 0x00;

  {
    uint32 size_in_K = num_8k_ROM_banks * 8;

    if(size_in_K == 1024)
    {
      MMC1_Size = MMC1_1024K;
    }
    else if(size_in_K == 512)
    {
      MMC1_Size = MMC1_512K;
    }
    else
    {
      MMC1_Size = MMC1_SMALL;
    }
  }
  MMC1_256K_base = 0; // use first 256K
  MMC1_swap = 0;

  if(MMC1_Size == MMC1_SMALL)
  {
    // set two high pages to last two banks
    MMC1_HI1 = num_8k_ROM_banks-2;
    MMC1_HI2 = num_8k_ROM_banks-1;
  }
  else
  {
    // set two high pages to last two banks of current 256K region
    MMC1_HI1 = (256/8)-2;
    MMC1_HI2 = (256/8)-1;
  }

  // set CPU bank pointers
  MMC1_bank1 = 0;
  MMC1_bank2 = 1;
  MMC1_bank3 = MMC1_HI1;
  MMC1_bank4 = MMC1_HI2;

  MMC1_set_CPU_banks();
}

void NES_mapper1::MMC1_set_CPU_banks()
{
  set_CPU_banks((MMC1_256K_base << 5) + (MMC1_bank1 & ((256/8)-1)),
                (MMC1_256K_base << 5) + (MMC1_bank2 & ((256/8)-1)),
                (MMC1_256K_base << 5) + (MMC1_bank3 & ((256/8)-1)),
                (MMC1_256K_base << 5) + (MMC1_bank4 & ((256/8)-1)));
}

void NES_mapper1::MemoryWrite(uint32 addr, uint8 data)
{
  uint32 reg_num;

  // if write is to a different reg, reset
  if((addr & 0x6000) != (last_write_addr & 0x6000))
  {
    write_count = 0;
    bits = 0x00;
  }
  last_write_addr = addr;

  // if bit 7 set, reset and return
  if(data & 0x80)
  {
    write_count = 0;
    bits = 0x00;
    return;
  }

  if(data & 0x01) bits |= (1 << write_count);
  write_count++;
  if(write_count < 5) return;

  reg_num = (addr & 0x7FFF) >> 13;
  regs[reg_num] = bits;

  write_count = 0;
  bits = 0x00;

//  LOG("MAP1 REG" << reg_num << ": " << HEX(regs[reg_num],2) << endl);

  switch(reg_num)
  {
    case 0:
      {
//        LOG("REG0: " << HEX(reg[0],2) << endl);

        // set mirroring
        if(regs[0] & 0x02)
        {
          if(regs[0] & 0x01)
          {
            set_mirroring(NES_PPU::MIRROR_HORIZ);
          }
          else
          {
            set_mirroring(NES_PPU::MIRROR_VERT);
          }
        }
        else
        {
          // one-screen mirroring
          if(regs[0] & 0x01)
          {
            set_mirroring(1,1,1,1);
          }
          else
          {
            set_mirroring(0,0,0,0);
          }
        }
      }
      break;

    case 1:
      {
        uint8 bank_num = regs[1];

//        LOG("REG1: " << HEX(reg[1],2) << endl);

        if(MMC1_Size == MMC1_1024K)
        {
          if(regs[0] & 0x10)
          {
            if(MMC1_swap)
            {
              MMC1_256K_base = (regs[1] & 0x10) >> 4;
              if(regs[0] & 0x08)
              {
                MMC1_256K_base |= ((regs[2] & 0x10) >> 3);
              }
              MMC1_set_CPU_banks();
              MMC1_swap = 0;
            }
            else
            {
              MMC1_swap = 1;
            }
          }
          else
          {
            // use 1st or 4th 256K banks
            MMC1_256K_base = (regs[1] & 0x10) ? 3 : 0;
            MMC1_set_CPU_banks();
          }
        }
        else if((MMC1_Size == MMC1_512K) && (!num_1k_VROM_banks))
        {
          MMC1_256K_base = (regs[1] & 0x10) >> 4;
          MMC1_set_CPU_banks();
        }
        else if(num_1k_VROM_banks)
        {
          // set VROM bank at $0000
          if(regs[0] & 0x10)
          {
            // swap 4K
            bank_num <<= 2;
            set_PPU_bank0(bank_num+0);
            set_PPU_bank1(bank_num+1);
            set_PPU_bank2(bank_num+2);
            set_PPU_bank3(bank_num+3);
          }
          else
          {
            // swap 8K
            bank_num <<= 2;
            set_PPU_banks(bank_num+0,bank_num+1,bank_num+2,bank_num+3,
                          bank_num+4,bank_num+5,bank_num+6,bank_num+7);
          }
        }
      }
      break;

    case 2:
      {
        uint8 bank_num = regs[2];

//        LOG("REG2: " << HEX(reg[2],2) << endl);

        if((MMC1_Size == MMC1_1024K) && (regs[0] & 0x08))
        {
          if(MMC1_swap)
          {
            MMC1_256K_base =  (regs[1] & 0x10) >> 4;
            MMC1_256K_base |= ((regs[2] & 0x10) >> 3);
            MMC1_set_CPU_banks();
            MMC1_swap = 0;
          }
          else
          {
            MMC1_swap = 1;
          }
        }

        if(!num_1k_VROM_banks) break;

        // set 4K VROM bank at $1000
        if(regs[0] & 0x10)
        {
          // swap 4K
          bank_num <<= 2;
          set_PPU_bank4(bank_num+0);
          set_PPU_bank5(bank_num+1);
          set_PPU_bank6(bank_num+2);
          set_PPU_bank7(bank_num+3);
        }
      }
      break;

    case 3:
      {
        uint8 bank_num = regs[3];

//        LOG("REG3: " << HEX(reg[3],2) << endl);

        // set ROM bank
        if(regs[0] & 0x08)
        {
          // 16K of ROM
          bank_num <<= 1;

          if(regs[0] & 0x04)
          {
            // 16K of ROM at $8000
            MMC1_bank1 = bank_num;
            MMC1_bank2 = bank_num+1;
            MMC1_bank3 = MMC1_HI1;
            MMC1_bank4 = MMC1_HI2;
          }
          else
          {
            // 16K of ROM at $C000
            if(MMC1_Size == MMC1_SMALL)
            {
              MMC1_bank1 = 0;
              MMC1_bank2 = 1;
              MMC1_bank3 = bank_num;
              MMC1_bank4 = bank_num+1;
            }
          }
        }
        else
        {
          // 32K of ROM at $8000
          bank_num <<= 2;

          MMC1_bank1 = bank_num;
          MMC1_bank2 = bank_num+1;
          if(MMC1_Size == MMC1_SMALL)
          {
            MMC1_bank3 = bank_num+2;
            MMC1_bank4 = bank_num+3;
          }
        }

        MMC1_set_CPU_banks();
      }
      break;
  }
}
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper 2
void NES_mapper2::Reset()
{
  // set CPU bank pointers
  set_CPU_banks(0,1,num_8k_ROM_banks-2,num_8k_ROM_banks-1);
}

void NES_mapper2::MemoryWrite(uint32 addr, uint8 data)
{
  data &= num_8k_ROM_banks-1;
  set_CPU_banks(data*2,(data*2)+1,num_8k_ROM_banks-2,num_8k_ROM_banks-1);
}
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper 3
void NES_mapper3::Reset()
{
  // set CPU bank pointers
  if(num_8k_ROM_banks > 2)
  {
    set_CPU_banks(0,1,2,3);
  }
  else
  {
    set_CPU_banks(0,1,0,1);
  }

  // set VROM banks
  set_PPU_banks(0,1,2,3,4,5,6,7);
}

void NES_mapper3::MemoryWrite(uint32 addr, uint8 data)
{
  uint32 base;

  data &= (num_1k_VROM_banks>>1)-1;

  base = ((uint32)data) << 3;
  set_PPU_banks(base+0,base+1,base+2,base+3,base+4,base+5,base+6,base+7);
}
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper 4
// much of this is based on the DarcNES source. thanks, nyef :)
void NES_mapper4::Reset()
{
  // clear registers FIRST!!!
  for(int i = 0; i < 8; i++) regs[i] = 0x00;

  // set CPU bank pointers
  prg0 = 0;
  prg1 = 1;
  MMC3_set_CPU_banks();

  // set VROM banks
  if(num_1k_VROM_banks)
  {
    chr01 = 0;
    chr23 = 2;
    chr4  = 4;
    chr5  = 5;
    chr6  = 6;
    chr7  = 7;
    MMC3_set_PPU_banks();
  }
  else
  {
    chr01 = chr23 = chr4 = chr5 = chr6 = chr7 = 0;
  }

  irq_enabled = 0;
  irq_counter = 0;
  irq_latch = 0;
}

void NES_mapper4::MemoryWrite(uint32 addr, uint8 data)
{
//  LOG("Write: " << HEX((addr & 0xE001),4) << "=" << HEX(data,2) << endl);
  switch(addr & 0xE001)
  {
    case 0x8000:
      {
        regs[0] = data;
        MMC3_set_PPU_banks();
        MMC3_set_CPU_banks();
      }
      break;

    case 0x8001:
      {
        uint32 bank_num;

        regs[1] = data;

        bank_num = regs[1];

//        LOG("CMD" << (int)(regs[0]&0x07) << ": " << HEX(bank_num,2) << endl);

        switch(regs[0] & 0x07)
        {
          case 0x00:
            {
              if(num_1k_VROM_banks)
              {
                bank_num &= 0xfe;
                chr01 = bank_num;
                MMC3_set_PPU_banks();
              }
            }
            break;

          case 0x01:
            {
              if(num_1k_VROM_banks)
              {
                bank_num &= 0xfe;
                chr23 = bank_num;
                MMC3_set_PPU_banks();
              }
            }
            break;

          case 0x02:
            {
              if(num_1k_VROM_banks)
              {
                chr4 = bank_num;
                MMC3_set_PPU_banks();
              }
            }
            break;

          case 0x03:
            {
              if(num_1k_VROM_banks)
              {
                chr5 = bank_num;
                MMC3_set_PPU_banks();
              }
            }
            break;

          case 0x04:
            {
              if(num_1k_VROM_banks)
              {
                chr6 = bank_num;
                MMC3_set_PPU_banks();
              }
            }
            break;

          case 0x05:
            {
              if(num_1k_VROM_banks)
              {
                chr7 = bank_num;
                MMC3_set_PPU_banks();
              }
            }
            break;

          case 0x06:
            {
              prg0 = bank_num;
              MMC3_set_CPU_banks();
            }
            break;

          case 0x07:
            {
              prg1 = bank_num;
              MMC3_set_CPU_banks();
            }
            break;
        }
      }
      break;

    case 0xA000:
      {
        regs[2] = data;

        if(data & 0x40)
        {
          LOG("MAP4 MIRRORING: 0x40 ???" << endl);
        }

        if(parent_NES->ROM->get_mirroring() != NES_PPU::MIRROR_FOUR_SCREEN)
        {
          if(data & 0x01)
          {
            set_mirroring(NES_PPU::MIRROR_HORIZ);
          }
          else
          {
            set_mirroring(NES_PPU::MIRROR_VERT);
          }
        }
      }
      break;

    case 0xA001:
      {
        regs[3] = data;

        if(data & 0x80)
        {
          // enable save RAM $6000-$7FFF
        }
        else
        {
          // disable save RAM $6000-$7FFF
        }
      }
      break;

    case 0xC000:
//  LOG("counter = " << HEX(data,2) << endl);
      regs[4] = data;
      irq_counter = regs[4];
      break;

    case 0xC001:
//  LOG("latch = " << HEX(data,2) << endl);
      regs[5] = data;
      irq_latch = regs[5];
      break;

    case 0xE000:
//  LOG("enabled = 0" << endl);
      regs[6] = data;
      irq_enabled = 0;
      break;

    case 0xE001:
//  LOG("enabled = 1" << endl);
      regs[7] = data;
      irq_enabled = 1;
      break;

    default:
      LOG("MAP4: UNKNOWN: " << HEX(addr,4) << " = " << HEX(data) << endl);
      break;

  }
}

void NES_mapper4::HSync(uint32 scanline)
{
  if(irq_enabled)
  {
    if((scanline >= 0) && (scanline <= 239))
    {
      if(parent_NES->ppu->spr_enabled() || parent_NES->ppu->bg_enabled())
      {
        if(!(irq_counter--))
        {
          irq_counter = irq_latch;
          parent_NES->cpu->DoIRQ();
        }
      }
    }
  }
}

#define MAP4_ROM(ptr)  (((ptr)-parent_NES->ROM->get_ROM_banks())  >> 13)
#define MAP4_VROM(ptr) (((ptr)-parent_NES->ROM->get_VROM_banks()) >> 10)

void NES_mapper4::SNSS_fixup() // HACK HACK HACK HACK
{
  NES_6502::Context context;
  parent_NES->cpu->GetContext(&context);

  prg0 = MAP4_ROM(context.mem_page[prg_swap() ? 6 : 4]);
  prg1 = MAP4_ROM(context.mem_page[5]);
  if(num_1k_VROM_banks)
  {
    if(chr_swap())
    {
      chr01 = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[4]);
      chr23 = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[6]);
      chr4  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[0]);
      chr5  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[1]);
      chr6  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[2]);
      chr7  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[3]);
    }
    else
    {
      chr01 = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[0]);
      chr23 = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[2]);
      chr4  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[4]);
      chr5  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[5]);
      chr6  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[6]);
      chr7  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[7]);
    }
  }
}

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper 7
void NES_mapper7::Reset()
{
  // set CPU bank pointers
  set_CPU_banks(0,1,2,3);
}

void NES_mapper7::MemoryWrite(uint32 addr, uint8 data)
{
  uint32 bank;

  bank = (data & 0x07) << 2;
  set_CPU_banks(bank+0,bank+1,bank+2,bank+3);

  if(data & 0x10)
  {
    set_mirroring(1,1,1,1);
  }
  else
  {
    set_mirroring(0,0,0,0);
  }
}
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper 9
void NES_mapper9::Reset()
{
  // set CPU bank pointers
  set_CPU_banks(0,num_8k_ROM_banks-3,num_8k_ROM_banks-2,num_8k_ROM_banks-1);

  for(int i = 0; i < sizeof(regs)/sizeof(regs[0]); i++)
    regs[i] = 0;

  regs[2] = 4;

  latch_0000 = 0xFE;
  latch_1000 = 0xFE;

  set_VROM_0000();
  set_VROM_1000();
}

void NES_mapper9::PPU_Latch_FDFE(uint32 addr)
{
  if(addr & 0x1000)
  {
    latch_1000 = (addr & 0x0FF0) >> 4;
    set_VROM_1000();
  }
  else
  {
    latch_0000 = (addr & 0x0FF0) >> 4;
    set_VROM_0000();
  }
}

void NES_mapper9::set_VROM_0000()
{
  int bank_num = (latch_0000 == 0xFD) ? regs[1] : regs[2];

  bank_num <<= 2;

  set_PPU_bank0(bank_num+0);
  set_PPU_bank1(bank_num+1);
  set_PPU_bank2(bank_num+2);
  set_PPU_bank3(bank_num+3);
}

void NES_mapper9::set_VROM_1000()
{
  int bank_num = (latch_1000 == 0xFD) ? regs[3] : regs[4];

  bank_num <<= 2;

  set_PPU_bank4(bank_num+0);
  set_PPU_bank5(bank_num+1);
  set_PPU_bank6(bank_num+2);
  set_PPU_bank7(bank_num+3);
}

void NES_mapper9::MemoryWrite(uint32 addr, uint8 data)
{
  switch(addr & 0xF000)
  {
    case 0xA000:
      {
        regs[0] = data;

        // 8K ROM bank at $8000
        uint8 bank_num = regs[0];
        set_CPU_bank4(bank_num);
      }
      break;

    case 0xB000:
      {
        // B000-BFFF: select 4k VROM for (0000) $FD latch
        regs[1] = data;
        set_VROM_0000();
      }
      break;

    case 0xC000:
      {
        // C000-CFFF: select 4k VROM for (0000) $FE latch
        regs[2] = data;
        set_VROM_0000();
      }
      break;

    case 0xD000:
      {
        // D000-DFFF: select 4k VROM for (1000) $FD latch
        regs[3] = data;
        set_VROM_1000();
      }
      break;

    case 0xE000:
      {
        // E000-EFFF: select 4k VROM for (1000) $FE latch
        regs[4] = data;
        set_VROM_1000();
      }
      break;

    case 0xF000:
      {
        regs[5] = data;

        if(regs[5] & 0x01)
        {
          set_mirroring(NES_PPU::MIRROR_HORIZ);
        }
        else
        {
          set_mirroring(NES_PPU::MIRROR_VERT);
        }
      }
      break;
  }
}

void NES_mapper9::SNSS_fixup()
{
  set_VROM_0000();
  set_VROM_1000();
}

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Mapper 40 (smb2j)
void NES_mapper40::Reset()
{
  irq_enabled = 0;
  lines_to_irq = 0;

  // set CPU bank pointers
  set_CPU_banks(6,4,5,0,7);

  // set VROM banks
  if(num_1k_VROM_banks)
  {
    set_PPU_banks(0,1,2,3,4,5,6,7);
  }
}

void NES_mapper40::MemoryWrite(uint32 addr, uint8 data)
{
  switch(addr & 0xE000)
  {
    case 0x8000:
      irq_enabled = 0;
//      LOG("MAP40: [$8000] = " << HEX(data,2) << endl);
      break;

    case 0xA000:
      irq_enabled = 1;
      lines_to_irq = 37;

//      LOG("MAP40: [$A000] = " << HEX(data,2) << endl);
      break;

    case 0xC000:
//      LOG("MAP40: [$C000] = " << HEX(data,2) << endl);
//      LOG("MAP40: INVALID WRITE TO $C000" << endl);
      break;

    case 0xE000:
//      LOG("MAP40: [$E000] = " << HEX(data,2) << endl);

      set_CPU_bank6(data & 0x07);
      break;

  }
}

void NES_mapper40::HSync(uint32 scanline)
{
  if(irq_enabled)
  {
    if((--lines_to_irq) <= 0)
    {
      parent_NES->cpu->DoIRQ();
    }
  }
}
/////////////////////////////////////////////////////////////////////

#include "NES_mapper_Konami.cpp"

/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
// mapper factory
NES_mapper* GetMapper(NES* parent, NES_ROM* rom)
{
  switch(rom->get_mapper_num())
  {
    case 0:
      return new NES_mapper0(parent);

    case 1:
      return new NES_mapper1(parent);

    case 2:
      return new NES_mapper2(parent);

    case 3:
      return new NES_mapper3(parent);

    case 4:
      return new NES_mapper4(parent);

    case 7:
      return new NES_mapper7(parent);

    case 9:
      return new NES_mapper9(parent);

    case 24:
      return new NES_mapper24(parent);

    case 40:
      return new NES_mapper40(parent);

    default:
      return NULL;  // mapper not supported
  }
}
/////////////////////////////////////////////////////////////////////
