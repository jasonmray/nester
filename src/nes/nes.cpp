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

#include <string.h>
#include <math.h>
#include "NES.h"
#include "NES_screen_mgr.h"
#include "NES_ROM.h"
#include "NES_PPU.h"
#include "pixmap.h"
#include "SNSS.h"
#include "win32_directory.h"

#include "debug.h"

const uint8 NES::NES_preset_palette[NES_NUM_COLORS][3] = 
{
// include the NES palette
#include "NES_pal.h"
};

//const float NES::CYCLES_PER_LINE = (float)113.6;
//const float NES::CYCLES_PER_LINE = (float)113.852;
//const float NES::CYCLES_PER_LINE = (float)113.75;
//const float NES::CYCLES_PER_LINE = (float)113.66666666666666666667;
const float NES::CYCLES_PER_LINE = ((float)((double)1364.0/(double)12.0));

NES::NES(const char* ROM_name, NES_screen_mgr* _screen_mgr, sound_mgr* _sound_mgr)
{
  scr_mgr = _screen_mgr;
  snd_mgr = _sound_mgr;

  scr_mgr->setParentNES(this);

  LOG("nester - NES emulator by Darren Ranalli, (c) 2000\n");

  cpu = NULL;
  ppu = NULL;
  apu = NULL;

  try {
    LOG("Creating NES CPU...");
    cpu = new NES_6502(this);
    if(!cpu) throw "error allocating cpu";
    LOG("Done.\n");

    LOG("Creating NES PPU...");
    ppu = new NES_PPU(this);
    if(!ppu) throw "error allocating ppu";
    LOG("Done.\n");

    LOG("Creating NES APU...");
    apu = new NES_APU(this);
    if(!apu) throw "error allocating apu";
    LOG("Done.\n");

    loadROM(ROM_name);
  } catch(...) {
    if(cpu) delete cpu;
    if(ppu) delete ppu;
    if(apu) delete apu;
    throw;
  }

  // set up palette and assert it
  calculate_palette();
  scr_mgr->assert_palette();

  pad1 = NULL;
  pad2 = NULL;
}

NES::~NES()
{
  freeROM();

  if(cpu) delete cpu;
  if(ppu) delete ppu;
  if(apu) delete apu;
}

void NES::new_snd_mgr(sound_mgr* _sound_mgr)
{
  snd_mgr = _sound_mgr;
  apu->snd_mgr_changed();
}

void NES::loadROM(const char* fn)
{
  LOG("Loading ROM...");

  ROM = new NES_ROM(fn);

  // set up the mapper
  mapper = GetMapper(this, ROM);
  if(!mapper)
  {
    // unsupported mapper
    LOG("mapper #" << (int)ROM->get_mapper_num() << " not supported" << endl);

    delete ROM;
    ROM = NULL;

    throw "unsupported mapper";
  }

  LOG("Done\n");

  LOG(ROM->GetRomName() << ".nes: #" << (int)ROM->get_mapper_num() << " ");
  switch(ROM->get_mirroring())
  {
    case NES_PPU::MIRROR_HORIZ:
      LOG("H ");
      break;
    case NES_PPU::MIRROR_VERT:
      LOG("V ");
      break;
    case NES_PPU::MIRROR_FOUR_SCREEN:
      LOG("F ");
      break;
  }

  if(ROM->has_save_RAM())
  {
    LOG("S ");
  }
  if(ROM->has_trainer())
  {
    LOG("T ");
  }

  LOG(16*ROM->get_num_16k_ROM_banks() << "K/" << 8*ROM->get_num_8k_VROM_banks() << "K " << endl);

  Load_SaveRAM();

  reset();

  LOG("Starting emulation...\n");
}

void NES::freeROM()
{
  Save_SaveRAM();

  LOG("Freeing ROM...");
  if(ROM)
  {
    delete ROM;
    ROM = NULL;
  }
  if(mapper)
  {
    delete mapper;
    mapper = NULL;
  }

  scr_mgr->clear(0x00);
  scr_mgr->blt();

  LOG("Done\n");
  LOG(endl);
}

const char* NES::getROMname()
{
  return ROM->GetRomName();
}

const char* NES::getROMnameExt()
{
  return ROM->GetRomNameExt();
}

const char* NES::getROMpath()
{
  return ROM->GetRomPath();
}

NES_ROM* NES::get_NES_ROM()
{
  return ROM;
}

void NES::reset()
{
  LOG("Resetting NES\n");

  // make sure saveRAM is saved
  Save_SaveRAM();

  // RAM
  memset(RAM, 0x00, sizeof(RAM));

  // SaveRAM
  Load_SaveRAM();

  // set up CPU
  {
    NES_6502::Context context;

    memset((void*)&context, 0x00, sizeof(context));
    cpu->GetContext(&context);

    context.mem_page[0] = RAM;
    context.mem_page[3] = SaveRAM;

    cpu->SetContext(&context);
  }

  // set up the trainer if present
  if(ROM->has_trainer())
  {
    LOG("  Setting trainer...");
    // trainer is located at 0x7000; SaveRAM is 0x2000 bytes at 0x6000
    memcpy(&SaveRAM[0x1000], ROM->get_trainer(), NES_ROM::TRAINER_LEN);
    LOG("OK\n");
  }

  LOG("  PPU reset...");
  // reset the PPU
  ppu->reset();
  LOG("OK\n");

  LOG("  APU reset...");
  // reset the APU
  apu->reset();
  LOG("OK\n");

  if(mapper)
  {
    LOG("  mapper reset...");
    // reset the mapper
    mapper->Reset();
    LOG("OK\n");
  }

  LOG("  CPU reset...");
  // reset the CPU
  cpu->Reset();
  LOG("OK\n");

  ideal_cycle_count  = 0.0;
  emulated_cycle_count = 0;

  pad_strobe = FALSE;
  pad1_bits = 0x00;
  pad2_bits = 0x00;
}


boolean NES::emulate_frame(boolean draw)
{
  uint32 i;
  pixmap p;
  uint8* cur_line; // ptr to screen buffer
  boolean retval = draw;

  trim_cycle_counts();

  // do frame
  ppu->start_frame();

  if(retval)
  {
    if(!scr_mgr->lock(p))
      retval = FALSE;
    else
      cur_line = p.data;
  }

  // LINES 0-239
  for(i = 0; i < NES_NUM_FRAME_LINES; i++)
  {
    // do one line's worth of CPU cycles
    emulate_CPU_cycles(CYCLES_PER_LINE);
    mapper->HSync(i);

    if(retval)
    {
      // render line
      ppu->do_scanline_and_draw(cur_line);
      // point to next line
      cur_line += p.pitch;
    }
    else
    {
      ppu->do_scanline_and_dont_draw();
    }
  }

  if(retval)
  {
    scr_mgr->unlock();
  }

  ppu->end_frame();

  for(i = 240; i <= 261; i++)
  {
    if(i == 241)
    {
      // do v-blank
      ppu->start_vblank();
      mapper->VSync();
    }
    else if(i == 261)
    {
      ppu->end_vblank();
    }

    if(i == 241)
    {
      // 1 instruction between vblank flag and NMI
      emulate_CPU_cycles(1.0);
      if(ppu->NMI_enabled()) cpu->DoNMI();
      emulate_CPU_cycles((float)CYCLES_PER_LINE-(float)1.0);
      mapper->HSync(i);
      continue;
    }

    emulate_CPU_cycles(CYCLES_PER_LINE);
    mapper->HSync(i);
  }


/*
  // NES DOES NOT DO THIS
  // HALF-LINE 262.5
  emulate_CPU_cycles(CYCLES_PER_LINE/2);
*/

  apu->DoFrame();

  return retval;
}

void NES::onFreeze()
{
  apu->freeze();
}

void NES::onThaw()
{
  apu->thaw();
}

uint8 NES::MemoryRead(uint32 addr)
{
//  LOG("Read " << HEX(addr,4) << endl);

  if(addr < 0x2000) // RAM
  {
    return ReadRAM(addr);
  }
  else if(addr < 0x4000) // low registers
  {
    return ReadLowRegs(addr);
  }
  else if(addr < 0x4018) // high registers
  {
    return ReadHighRegs(addr);
  }
  else if(addr < 0x6000) // mapper low
  {
    //LOG("MAPPER LOW READ: " << HEX(addr,4) << endl);
    return((uint8)(addr >> 8)); // what's good for conte is good for me
  }
  else // save RAM, or ROM (mapper 40)
  {
    return cpu->GetByte(addr);
  }
}

void NES::MemoryWrite(uint32 addr, uint8 data)
{
//  LOG("Write " << HEX(addr,4) << " " << HEX(data,2) << endl);

  if(addr < 0x2000) // RAM
  {
    WriteRAM(addr, data);
  }
  else if(addr < 0x4000) // low registers
  {
    WriteLowRegs(addr, data);
  }
  else if(addr < 0x4018) // high registers
  {
    WriteHighRegs(addr, data);
  }
  else if(addr < 0x6000) // mapper low
  {
    mapper->MemoryWriteLow(addr, data);
  }
  else if(addr < 0x8000) // save RAM
  {
    SaveRAM[addr - 0x6000] = data;
    mapper->MemoryWriteSaveRAM(addr, data);
  }
  else // mapper
  {
    mapper->MemoryWrite(addr, data);
  }
}


uint8 NES::ReadRAM(uint32 addr)
{
  return RAM[addr & 0x7FF];
}

void NES::WriteRAM(uint32 addr, uint8 data)
{
  RAM[addr & 0x7FF] = data;
}


uint8 NES::ReadLowRegs(uint32 addr)
{
  return ppu->ReadLowRegs(addr & 0xE007);
}

void NES::WriteLowRegs(uint32 addr, uint8 data)
{
  ppu->WriteLowRegs(addr & 0xE007, data);
}


uint8 NES::ReadHighRegs(uint32 addr)
{
  if(addr == 0x4014) // SPR-RAM DMA
  {
    LOG("Read from SPR-RAM DMA reg??" << endl);
    return ppu->Read0x4014();
  }
  else if(addr < 0x4016) // APU
  {
//    LOG("APU READ:" << HEX(addr,4) << endl);
    return apu->Read(addr);
  }
  else // joypad regs
  {
    uint8 retval;

    if(addr == 0x4016)
    {
      // joypad 1
      retval = pad1_bits & 0x01;
      pad1_bits >>= 1;
    }
    else
    {
      // joypad 2
      retval = pad2_bits & 0x01;
      pad2_bits >>= 1;
    }
    return retval;
  }
}

void NES::WriteHighRegs(uint32 addr, uint8 data)
{
  if(addr == 0x4014) // SPR-RAM DMA
  {
    ppu->Write0x4014(data);
    // sprite DMA takes XXX cycles
    cpu->SetDMA(512);
  }
  else if(addr < 0x4016) // APU
  {
//    LOG("APU WRITE" << endl);
    apu->Write(addr, data);
  }
  else // joypad regs
  {
    // bit 0 == joypad strobe
    if(data & 0x01)
    {
      pad_strobe = TRUE;
    }
    else
    {
      if(pad_strobe)
      {
        pad_strobe = FALSE;
        // get input states
        if(pad1) pad1_bits = pad1->get_inp_state();
        if(pad2) pad2_bits = pad2->get_inp_state();
      }
    }
  }
}

void NES::emulate_CPU_cycles(float num_cycles)
{
  uint32 cycle_deficit;

  ideal_cycle_count += num_cycles;
  cycle_deficit = ((uint32)ideal_cycle_count) - emulated_cycle_count;
  if(cycle_deficit > 0)
  {
    emulated_cycle_count += cpu->Execute(cycle_deficit);
  }
}

// call every once in a while to avoid cycle count overflow
void NES::trim_cycle_counts()
{
  uint32 trim_amount;

  trim_amount = (uint32)floor(ideal_cycle_count);
  if(trim_amount > emulated_cycle_count) trim_amount = emulated_cycle_count;

  ideal_cycle_count  -= (float)trim_amount;
  emulated_cycle_count -= trim_amount;
}


void NES::Save_SaveRAM()
{
  // does the ROM use save ram?
  if(!ROM->has_save_RAM()) return;

  uint32 i;
  // has anything been written to Save RAM?
  for(i = 0; i < sizeof(SaveRAM); i++)
  {
    if(SaveRAM[i] != 0x00) break;
  }
  if(i < sizeof(SaveRAM))
  {
    FILE* fp = NULL;
    char fn[_MAX_PATH];

    LOG("Saving Save RAM...");

    DIR_createNesFileName(ROM, fn, NESTER_settings.nes.preferences.saveRamDirType,
      NESTER_settings.nes.preferences.saveRamDir, ROM->GetRomName(), ".sav");

    try
    {
      fp = fopen(fn, "wb");
      if(!fp) throw "can't open save RAM file";

      if(fwrite(SaveRAM, sizeof(SaveRAM), 1, fp) != 1)
        throw "can't open save RAM file";

      fclose(fp);
      LOG("Done." << endl);

    } catch(...) {
      LOG("can't save" << endl);
      if(fp) fclose(fp);
    }
  }
}

void NES::Load_SaveRAM()
{
  memset(SaveRAM, 0x00, sizeof(SaveRAM));

  // does the ROM use save ram?
  if(!ROM->has_save_RAM()) return;

  {
    FILE* fp = NULL;
    char fn[_MAX_PATH];

    DIR_createNesFileName(ROM, fn, NESTER_settings.nes.preferences.saveRamDirType,
      NESTER_settings.nes.preferences.saveRamDir, ROM->GetRomName(), ".sav");

    try
    {
      fp = fopen(fn, "rb");
      if(!fp) throw "none found.";

      LOG("Loading Save RAM...");

      if(fread(SaveRAM, sizeof(SaveRAM), 1, fp) != 1)
      {
        LOG("error reading Save RAM file" << endl);
        throw "error reading Save RAM file";
      }

      fclose(fp);
      LOG("Done." << endl);

    } catch(...) {
      if(fp) fclose(fp);
    }
  }
}

boolean NES::loadState(const char* fn)
{
  return LoadSNSS(fn, this);
}

boolean NES::saveState(const char* fn)
{
  return SaveSNSS(fn, this);
}

void NES::calculate_palette()
{
  if(NESTER_settings.nes.graphics.calculate_palette)
  {
    int x,z;
    float tint = ((float)NESTER_settings.nes.graphics.tint) / 256.0f;
    float hue = 332.0f + (((float)NESTER_settings.nes.graphics.hue - (float)0x80) * (20.0f / 256.0f));
    float s,y;
    int cols[16] = {0,240,210,180,150,120,90,60,30,0,330,300,270,0,0,0};
    float theta;
    float br1[4] = {0.5f, 0.75f, 1.0f, 1.0f};
    float br2[4] = {0.29f, 0.45f, 0.73f, 0.9f};
    float br3[4] = {0.0f, 0.24f, 0.47f, 0.77f};
    float r,g,b;

    for(x = 0; x <= 3; x++)
    {
      for(z = 0; z <= 15; z++)
      {
        s = tint;
        y = br2[x];
        if(z == 0)
        {
          s = 0;
          y = br1[x];
        }
        else if(z == 13)
        {
          s = 0;
          y = br3[x];
        }
        else if((z == 14) || (z == 15))
        {
          s = 0;
          y = 0;
        }

        theta = 3.14159265f * (((float)(cols[z] + hue)) / 180.0f);

        r = y + (s * (float)sin(theta));
        g = y - ((27.0f / 53.0f) * s * (float)sin(theta)) + ((10.0f / 53.0f) * s * (float)cos(theta));
        b = y - (s * (float)cos(theta));

        r = r * 256.0f;
        g = g * 256.0f;
        b = b * 256.0f;

        if(r > 255.0f) r = 255.0f;
        if(g > 255.0f) g = 255.0f;
        if(b > 255.0f) b = 255.0f;

        if(r < 0.0f) r = 0.0;
        if(g < 0.0f) g = 0.0;
        if(b < 0.0f) b = 0.0;

        NES_RGB_pal[(x*16) + z][0] = (uint8)r;
        NES_RGB_pal[(x*16) + z][1] = (uint8)g;
        NES_RGB_pal[(x*16) + z][2] = (uint8)b;
      }
    }
  }
  else
  {
    memcpy(NES_RGB_pal, NES_preset_palette, sizeof(NES_RGB_pal));
  }

  if(NESTER_settings.nes.graphics.black_and_white)
  {
    int i;

    for(i = 0; i < NES_NUM_COLORS; i++)
    {
      uint8 Y;
      Y = (uint8)(((float)NES_RGB_pal[i][0] * 0.299) +
                  ((float)NES_RGB_pal[i][1] * 0.587) +
                  ((float)NES_RGB_pal[i][2] * 0.114));
      NES_RGB_pal[i][0] = Y;
      NES_RGB_pal[i][1] = Y;
      NES_RGB_pal[i][2] = Y;
    }
  }
}
