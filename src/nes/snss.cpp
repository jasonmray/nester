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

#include "types.h"
#include <iostream>
#include <string.h>
#include "debug.h"
#include "SNSS.h"
#include "NES_6502.h"
#include "nes.h"

#include "libsnss.h"

// these functions apply a SNSS block to the current emulated NES

static void adopt_BASR(SnssBaseBlock* block, NES* nes)
{
  // BASR - Base Registers
  NES_6502::Context context;

  nes->cpu->GetContext(&context);

  context.a_reg = block->regA;
  context.x_reg = block->regX;
  context.y_reg = block->regY;
  context.p_reg = block->regFlags;
  context.s_reg = block->regStack;
  context.pc_reg = block->regPc;

  context.int_pending = 0;
  context.jammed = 0;
  context.burn_cycles = 0;
  //context.total_cycles = 0;
  nes->cpu->SetContext(&context);

  // registers $2000 and $2001
  nes->MemoryWrite(0x2000, block->reg2000);
  nes->MemoryWrite(0x2001, block->reg2001);

  // RAM
  memcpy(nes->RAM, block->cpuRam, 0x800);

  // SPR-RAM
  memcpy(nes->ppu->spr_ram, block->spriteRam, 0x100);

  // PPU $2000-$2FFF (Name Tables/Attrib Tables)
  memcpy(nes->ppu->PPU_nametables, block->ppuRam, 4*0x400);

  // palettes
  memcpy(nes->ppu->bg_pal,  &block->palette[0x00], 0x10);
  memcpy(nes->ppu->spr_pal, &block->palette[0x10], 0x10);

  // mirroring
  nes->ppu->set_mirroring((uint32)block->mirrorState[0]&0x03,
                          (uint32)block->mirrorState[1]&0x03,
                          (uint32)block->mirrorState[2]&0x03,
                          (uint32)block->mirrorState[3]&0x03);

  // VRAM address
  nes->ppu->loopy_t = block->vramAddress;

  // OAM (spr) address
  nes->ppu->spr_ram_rw_ptr = block->spriteRamAddress;

  // tile X offset
  nes->ppu->loopy_x = block->tileXOffset;
}

static void adopt_VRAM(SnssVramBlock* block, NES* nes)
{
  // VRAM
  int i;

  // read 8K
  for(i = 0; i < 8; i++)
  {
    memcpy(nes->ppu->PPU_VRAM_banks[i], &block->vram[i*0x400], 0x400);
  }

  if(block->vramSize > 0x2000)
  {
    LOG("SNSS VRAM size greater than 8K; unsupported" << endl);
  }
}

static void adopt_SRAM(SnssSramBlock* block, NES* nes)
{
  // Save-RAM
  NES_6502::Context context;

  // read SRAM
  nes->cpu->GetContext(&context);
  memcpy(context.mem_page[3], block->sram, (block->sramSize <= 0x2000) ? block->sramSize : 0x2000);

  if(block->sramSize > 0x2000)
  {
    LOG("SNSS SRAM size greater than 8K; unsupported" << endl);
  }
}

static void adopt_MPRD(SnssMapperBlock* block, NES* nes)
{
  // Mapper Data

  // set PRG pages
  {
    NES_6502::Context context;
    nes->cpu->GetContext(&context);
    context.mem_page[4] = nes->ROM->get_ROM_banks() + ((uint32)block->prgPages[0] << 13);
    context.mem_page[5] = nes->ROM->get_ROM_banks() + ((uint32)block->prgPages[1] << 13);
    context.mem_page[6] = nes->ROM->get_ROM_banks() + ((uint32)block->prgPages[2] << 13);
    context.mem_page[7] = nes->ROM->get_ROM_banks() + ((uint32)block->prgPages[3] << 13);
    nes->cpu->SetContext(&context);
  }

  // set CHR pages
  if(nes->ROM->get_num_8k_VROM_banks())
  {
    nes->ppu->PPU_VRAM_banks[0] = nes->ROM->get_VROM_banks() + ((uint32)block->chrPages[0] << 10);
    nes->ppu->PPU_VRAM_banks[1] = nes->ROM->get_VROM_banks() + ((uint32)block->chrPages[1] << 10);
    nes->ppu->PPU_VRAM_banks[2] = nes->ROM->get_VROM_banks() + ((uint32)block->chrPages[2] << 10);
    nes->ppu->PPU_VRAM_banks[3] = nes->ROM->get_VROM_banks() + ((uint32)block->chrPages[3] << 10);
    nes->ppu->PPU_VRAM_banks[4] = nes->ROM->get_VROM_banks() + ((uint32)block->chrPages[4] << 10);
    nes->ppu->PPU_VRAM_banks[5] = nes->ROM->get_VROM_banks() + ((uint32)block->chrPages[5] << 10);
    nes->ppu->PPU_VRAM_banks[6] = nes->ROM->get_VROM_banks() + ((uint32)block->chrPages[6] << 10);
    nes->ppu->PPU_VRAM_banks[7] = nes->ROM->get_VROM_banks() + ((uint32)block->chrPages[7] << 10);
  }

  // handle mapper-specific data
  switch(nes->ROM->get_mapper_num())
  {
    case 1:
      {
        NES_mapper1* mapper = (NES_mapper1*)nes->mapper;
        mapper1Data* mapper_data = (mapper1Data*)&block->extraData;

        // last values written to the 4 registers
        memcpy(mapper->regs, mapper_data->registers, 4);

        // latch register
        mapper->bits = mapper_data->latch;

        // number of bits written to unfinished reg
        mapper->write_count = mapper_data->numberOfBits;
      }
      break;

    case 4:
      {
        NES_mapper4* mapper = (NES_mapper4*)nes->mapper;
        mapper4Data* mapper_data = (mapper4Data*)&block->extraData;

        // IRQ counter
        mapper->irq_counter = mapper_data->irqCounter;

        // IRQ latch
        mapper->regs[4] = mapper_data->irqLatchCounter;

        // IRQ enabled
        mapper->irq_enabled = mapper_data->irqCounterEnabled;

        // last value written to $8000 (reg 0)
        mapper->regs[0] = mapper_data->last8000Write;

        mapper->SNSS_fixup();
      }
      break;

    case 9:
      {
        NES_mapper9* mapper = (NES_mapper9*)nes->mapper;
        mapper9Data* mapper_data = (mapper9Data*)&block->extraData;

        // 2 latch registers
        mapper->latch_0000 = mapper_data->latch[0];
        mapper->latch_1000 = mapper_data->latch[1];

        // regs (B/C/D/E000)
        mapper->regs[1] = mapper_data->lastB000Write;
        mapper->regs[2] = mapper_data->lastC000Write;
        mapper->regs[3] = mapper_data->lastD000Write;
        mapper->regs[4] = mapper_data->lastE000Write;

        mapper->SNSS_fixup();
      }
      break;

    case 24:
      {
        NES_mapper24* mapper = (NES_mapper24*)nes->mapper;
        mapper24Data* mapper_data = (mapper24Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        //mapper->irq_latch = mapper_data->irqLatchCounter;
      }
      break;

    case 40:
      {
        NES_mapper40* mapper = (NES_mapper40*)nes->mapper;
        mapper40Data* mapper_data = (mapper40Data*)&block->extraData;

        // IRQ counter
        mapper->lines_to_irq = mapper_data->irqCounter;

        // IRQ enabled
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
      }
      break;
  }
}

static void adopt_CNTR(SnssControllersBlock* block, NES* nes)
{
}

static void adopt_SOUN(SnssSoundBlock* block, NES* nes)
{
  // Sound Data

  // give them to the apu
  nes->apu->load_regs(block->soundRegisters);
}

boolean LoadSNSS(const char* fn, NES* nes)
{
  SNSS_FILE* snssFile = NULL;
  SNSS_BLOCK_TYPE blockType;

  try {
    if(SNSS_OK != SNSS_OpenFile(&snssFile, fn, SNSS_OPEN_READ))
      throw -1;

    // at this point, it's too late to go back, and the NES must be reset on failure
    try {
      for(int i = 0; i < (int)snssFile->headerBlock.numberOfBlocks; i++)
      {
        if(SNSS_OK != SNSS_GetNextBlockType(&blockType, snssFile))
          throw -1;

        if(SNSS_OK != SNSS_ReadBlock(snssFile, blockType))
          throw -1;

        switch(blockType)
        {
          case SNSS_BASR:
            adopt_BASR(&snssFile->baseBlock, nes);
            break;

          case SNSS_VRAM:
            adopt_VRAM(&snssFile->vramBlock, nes);
            break;

          case SNSS_SRAM:
            adopt_SRAM(&snssFile->sramBlock, nes);
            break;

          case SNSS_MPRD:
            adopt_MPRD(&snssFile->mapperBlock, nes);
            break;

          case SNSS_CNTR:
            adopt_CNTR(&snssFile->contBlock, nes);
            break;

          case SNSS_SOUN:
            adopt_SOUN(&snssFile->soundBlock, nes);
            break;

          case SNSS_UNKNOWN_BLOCK:
            break;

          default:
            throw -1;
            break;

        }
      }
    } catch(...) {
      nes->reset();
      throw;
    }

    SNSS_CloseFile(&snssFile);

    LOG("Loaded state from " << fn << endl);

  } catch(...) {
    LOG("Error reading " << fn << endl);
    if(snssFile) SNSS_CloseFile(&snssFile);
    return FALSE;
  }

  return TRUE;
}


// these functions create a SNSS block from the current emulated NES
// return 0 if block is valid

static int extract_BASR(SnssBaseBlock* block, NES* nes)
{
  NES_6502::Context context;

  // get the CPU context
  nes->cpu->GetContext(&context);

  // CPU data
  block->regA = context.a_reg;
  block->regX = context.x_reg;
  block->regY = context.y_reg;
  block->regFlags = context.p_reg;
  block->regStack = context.s_reg;
  block->regPc = context.pc_reg;

  // $2000 and $2001
  block->reg2000 = nes->ppu->LowRegs[0];
  block->reg2001 = nes->ppu->LowRegs[1];

  // RAM
  memcpy(block->cpuRam, nes->RAM, 0x800);

  // SPR-RAM
  memcpy(block->spriteRam, nes->ppu->spr_ram, 0x100);

  // PPU $2000-$2FFF (Name Tables/Attrib Tables)
  memcpy(block->ppuRam, nes->ppu->PPU_nametables, 4*0x400);

  // palettes
  memcpy(&block->palette[0x00], nes->ppu->bg_pal,  0x10);
  memcpy(&block->palette[0x10], nes->ppu->spr_pal, 0x10);

  // mirroring
  block->mirrorState[0] = (nes->ppu->PPU_VRAM_banks[0x08] - nes->ppu->PPU_nametables) >> 10;
  block->mirrorState[1] = (nes->ppu->PPU_VRAM_banks[0x09] - nes->ppu->PPU_nametables) >> 10;
  block->mirrorState[2] = (nes->ppu->PPU_VRAM_banks[0x0A] - nes->ppu->PPU_nametables) >> 10;
  block->mirrorState[3] = (nes->ppu->PPU_VRAM_banks[0x0B] - nes->ppu->PPU_nametables) >> 10;
  ASSERT(block->mirrorState[0] < 4); ASSERT(block->mirrorState[1] < 4);
  ASSERT(block->mirrorState[2] < 4); ASSERT(block->mirrorState[3] < 4);

  // VRAM address
  block->vramAddress = nes->ppu->loopy_t;

  // OAM (sprite) address
  block->spriteRamAddress = nes->ppu->spr_ram_rw_ptr;

  // tile X offset
  block->tileXOffset = nes->ppu->loopy_x;

  return 0;
}

static int extract_VRAM(SnssVramBlock* block, NES* nes)
{
  int i;

  // if cart has VROM, don't write any VRAM
  if(nes->ROM->get_num_8k_VROM_banks()) return -1;

  // 8K of VRAM data
  block->vramSize = 8 * 0x400;

  for(i = 0; i < 8; i++)
  {
    memcpy(&block->vram[i*0x400], nes->ppu->PPU_VRAM_banks[i], 0x400);
  }

  return 0;
}

static int extract_SRAM(SnssSramBlock* block, NES* nes)
{
  NES_6502::Context context;

  uint32 i;

  // if nothing has been written to SRAM, don't write it out
  // has anything been written to Save RAM?
  for(i = 0; i < sizeof(nes->SaveRAM); i++)
  {
    if(nes->SaveRAM[i] != 0x00) break;
  }
  if(i == sizeof(nes->SaveRAM)) return -1;

  // SRAM writeable flag
  block->sramEnabled = 1;

  // SRAM size (8k)
  block->sramSize = 0x2000;

  // SRAM data
  nes->cpu->GetContext(&context);
  memcpy(block->sram, context.mem_page[3], 0x2000);

  return 0;
}

static int extract_MPRD(SnssMapperBlock* block, NES* nes)
{
  NES_6502::Context context;

  if(0 == nes->ROM->get_mapper_num()) return -1;

  // 8K PRG page numbers
  nes->cpu->GetContext(&context);
  block->prgPages[0] = (context.mem_page[4] - nes->ROM->get_ROM_banks()) >> 13;
  block->prgPages[1] = (context.mem_page[5] - nes->ROM->get_ROM_banks()) >> 13;
  block->prgPages[2] = (context.mem_page[6] - nes->ROM->get_ROM_banks()) >> 13;
  block->prgPages[3] = (context.mem_page[7] - nes->ROM->get_ROM_banks()) >> 13;

  // 1K CHR page numbers
  if(nes->ROM->get_num_8k_VROM_banks())
  {
    block->chrPages[0] = (nes->ppu->PPU_VRAM_banks[0] - nes->ROM->get_VROM_banks()) >> 10;
    block->chrPages[1] = (nes->ppu->PPU_VRAM_banks[1] - nes->ROM->get_VROM_banks()) >> 10;
    block->chrPages[2] = (nes->ppu->PPU_VRAM_banks[2] - nes->ROM->get_VROM_banks()) >> 10;
    block->chrPages[3] = (nes->ppu->PPU_VRAM_banks[3] - nes->ROM->get_VROM_banks()) >> 10;
    block->chrPages[4] = (nes->ppu->PPU_VRAM_banks[4] - nes->ROM->get_VROM_banks()) >> 10;
    block->chrPages[5] = (nes->ppu->PPU_VRAM_banks[5] - nes->ROM->get_VROM_banks()) >> 10;
    block->chrPages[6] = (nes->ppu->PPU_VRAM_banks[6] - nes->ROM->get_VROM_banks()) >> 10;
    block->chrPages[7] = (nes->ppu->PPU_VRAM_banks[7] - nes->ROM->get_VROM_banks()) >> 10;
  }
  else
  {
    block->chrPages[0] = 0;
    block->chrPages[1] = 1;
    block->chrPages[2] = 2;
    block->chrPages[3] = 3;
    block->chrPages[4] = 4;
    block->chrPages[5] = 5;
    block->chrPages[6] = 6;
    block->chrPages[7] = 7;
  }

  switch(nes->ROM->get_mapper_num())
  {
    case 1:
      {
        NES_mapper1* mapper = (NES_mapper1*)nes->mapper;
        mapper1Data* mapper_data = (mapper1Data*)&block->extraData;

        // last values written to the 4 registers
        memcpy(mapper_data->registers, mapper->regs, 4);

        // latch register
        mapper_data->latch = mapper->bits;

        // number of bits written to unfinished reg
        mapper_data->numberOfBits = mapper->write_count;
      }
      break;

    case 4:
      {
        NES_mapper4* mapper = (NES_mapper4*)nes->mapper;
        mapper4Data* mapper_data = (mapper4Data*)&block->extraData;

        // IRQ counter
        mapper_data->irqCounter = mapper->irq_counter;

        // IRQ latch
        mapper_data->irqLatchCounter = mapper->regs[4];

        // IRQ enabled
        mapper_data->irqCounterEnabled = mapper->irq_enabled;

        // last value written to $8000 (reg 0)
        mapper_data->last8000Write = mapper->regs[0];
      }
      break;

    case 9:
      {
        NES_mapper9* mapper = (NES_mapper9*)nes->mapper;
        mapper9Data* mapper_data = (mapper9Data*)&block->extraData;

        // 2 latch registers
        mapper_data->latch[0] = mapper->latch_0000;
        mapper_data->latch[1] = mapper->latch_1000;

        // regs (B/C/D/E000)
        mapper_data->lastB000Write = mapper->regs[1];
        mapper_data->lastC000Write = mapper->regs[2];
        mapper_data->lastD000Write = mapper->regs[3];
        mapper_data->lastE000Write = mapper->regs[4];
      }
      break;

    case 24:
      {
        NES_mapper24* mapper = (NES_mapper24*)nes->mapper;
        mapper24Data* mapper_data = (mapper24Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        //mapper_data->irqLatchCounter = mapper->irq_latch;
      }
      break;

    case 40:
      {
        NES_mapper40* mapper = (NES_mapper40*)nes->mapper;
        mapper40Data* mapper_data = (mapper40Data*)&block->extraData;

        // IRQ counter
        mapper_data->irqCounter = mapper->lines_to_irq;

        // IRQ enabled
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
      }
      break;
  }

  return 0;
}

static int extract_CNTR(SnssControllersBlock* block, NES* nes)
{
  return -1;
}

static int extract_SOUN(SnssSoundBlock* block, NES* nes)
{
  // get sound registers
  nes->apu->get_regs(block->soundRegisters);

  return 0;
}

boolean SaveSNSS(const char* fn, NES* nes)
{
  SNSS_FILE* snssFile;

  try {
    if(SNSS_OK != SNSS_OpenFile(&snssFile, fn, SNSS_OPEN_WRITE))
      throw -1;

    // write BASR
    if(!extract_BASR(&snssFile->baseBlock, nes))
    {
      if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_BASR))
        throw -1;
    }

    // write VRAM
    if(!extract_VRAM(&snssFile->vramBlock, nes))
    {
      if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_VRAM))
        throw -1;
    }

    // write SRAM
    if(!extract_SRAM(&snssFile->sramBlock, nes))
    {
      if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_SRAM))
        throw -1;
    }

    // write MPRD
    if(!extract_MPRD(&snssFile->mapperBlock, nes))
    {
      if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_MPRD))
        throw -1;
    }

    // write CNTR
    if(!extract_CNTR(&snssFile->contBlock, nes))
    {
      if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_CNTR))
        throw -1;
    }

    // write SOUN
    if(!extract_SOUN(&snssFile->soundBlock, nes))
    {
      if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_SOUN))
        throw -1;
    }

    if(SNSS_OK != SNSS_CloseFile(&snssFile))
      throw -1;

    LOG("Saved state to " << fn << endl);

  } catch(...) {
    LOG("Error writing " << fn << endl);
    if(snssFile) SNSS_CloseFile(&snssFile);
    return FALSE;
  }

  return TRUE;
}

