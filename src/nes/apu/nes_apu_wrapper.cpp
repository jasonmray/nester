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
#include <stdlib.h>
#include "NES_APU_wrapper.h"
#include "NES.h"
#include "sound_mgr.h"
#include "debug.h"
#include "settings.h"

NES_APU::NES_APU(NES* parent)
{
  parent_NES = parent;
  parent_NES->snd_mgr->clear_buffer();

  apu = NULL;
  Init();
  AssertParams();
  reset();
}

NES_APU::~NES_APU()
{
  ShutDown();
}

void NES_APU::update_local_params()
{
  if(parent_NES->snd_mgr->IsNull())
  {
    // kinda filthy.
    _local_sample_rate = 11025;
    _local_sample_size = 8;
  }
  else
  {
    _local_sample_rate = parent_NES->snd_mgr->get_sample_rate();
    _local_sample_size = parent_NES->snd_mgr->get_sample_size();
  }
}

void NES_APU::Init()
{
  if(apu)
  {
    ShutDown();
  }

  update_local_params();
  apu = apu_create(_local_sample_rate, 60, 0, _local_sample_size);
  if(!apu) throw "Error creating NES APU";
}

void NES_APU::ShutDown()
{
  if(apu)
  {
    apu_destroy(&apu);
    apu = NULL;
  }
}

void NES_APU::hasExt_VRC6()
{
  if(apu)
  {
    apu_setext(apu, &vrcvi_ext);
    apu_reset();
  }
}

void NES_APU::AssertParams()
{
  if(apu)
  {
    apu_setchan(0, NESTER_settings.nes.sound.rectangle1_enabled);
    apu_setchan(1, NESTER_settings.nes.sound.rectangle2_enabled);
    apu_setchan(2, NESTER_settings.nes.sound.triangle_enabled);
    apu_setchan(3, NESTER_settings.nes.sound.noise_enabled);
    apu_setchan(4, NESTER_settings.nes.sound.dpcm_enabled);
    apu_setchan(5, NESTER_settings.nes.sound.external_enabled);

    switch(NESTER_settings.nes.sound.filter_type)
    {
      case NES_sound_settings::FILTER_NONE:
        apu_setfilter(APU_FILTER_NONE);
        break;
      case NES_sound_settings::FILTER_LOWPASS:
        apu_setfilter(APU_FILTER_LOWPASS);
        break;
      case NES_sound_settings::FILTER_LOWPASS_WEIGHTED:
        apu_setfilter(APU_FILTER_WEIGHTED);
        break;
    }
  }
}

void NES_APU::reset()
{
  if(apu)
  {
    apu_setext(apu, NULL);
    apu_reset();
  }
  memset(regs, 0x00, sizeof(regs));
}

void NES_APU::snd_mgr_changed()
{
  if(apu)
  {
    update_local_params();
    apu_setparams(_local_sample_rate, 60, 0, _local_sample_size);
    AssertParams();
  }
}

uint8 NES_APU::Read(uint32 addr)
{
  if(apu)
  {
    return apu_read(addr);
  }
  else
  {
    return 0x00;
  }
}

void NES_APU::Write(uint32 addr, uint8 data)
{
  if(apu)
  {
    if((addr > 0x4000) && (addr <= 0x4015))
    {
      regs[addr - 0x4000] = data;
    }
    apu_write(addr, data);
  }
}

void NES_APU::DoFrame()
{
  sound_mgr* snd_mgr = parent_NES->snd_mgr;
  sound_mgr::sound_buf_pos cur_ph;  // cur playing half
  sound_mgr::sound_buf_pos cur_nph; // cur not-playing half

  uint8* buf;
  uint32 buf_len;

  if(apu)
  {
    if(parent_NES->snd_mgr->IsNull())
    {
      apu_process(NULL, 0);
    }
    else
    {
      cur_ph = snd_mgr->get_currently_playing_half();

      if(cur_ph == currently_playing_half) return;

      cur_nph = currently_playing_half;
      currently_playing_half = cur_ph;

      if(!snd_mgr->lock(cur_nph, (void**)&buf, &buf_len))
      {
        LOG("couldn't lock sound buffer" << endl);
        return;
      }

      apu_process(buf, buf_len/(_local_sample_size/8));

      snd_mgr->unlock();
    }
  }
}

void NES_APU::freeze()
{
  parent_NES->snd_mgr->clear_buffer();
}

void NES_APU::thaw()
{
  currently_playing_half = parent_NES->snd_mgr->get_currently_playing_half();
}

void NES_APU::load_regs(const uint8 new_regs[0x16])
{
  int i;

  if(!apu) return;

  for(i = 0; i < 0x16; i++)
  {
    // reg 0x14 not used
    if(i == 0x14) continue;

    // write the DMC regs directly
    if((i >= 0x10) && (i <= 0x13))
    {
      apu->dmc.regs[i - 0x10] = new_regs[i];
    }
    else
    {
      apu_write(0x4000 + i, new_regs[i]);
    }
  }
}

void NES_APU::get_regs(uint8 reg_array[0x16])
{
  // copy last written values
  memcpy(reg_array, regs, 0x16);

  if(apu)
  {
    // copy in the per-channel stored values
    regs[APU_WRA0-0x4000] = apu->rectangle[0].regs[0];
    regs[APU_WRA1-0x4000] = apu->rectangle[0].regs[1];
    regs[APU_WRA2-0x4000] = apu->rectangle[0].regs[2];
    regs[APU_WRA3-0x4000] = apu->rectangle[0].regs[3];
    regs[APU_WRB0-0x4000] = apu->rectangle[1].regs[0];
    regs[APU_WRB1-0x4000] = apu->rectangle[1].regs[1];
    regs[APU_WRB2-0x4000] = apu->rectangle[1].regs[2];
    regs[APU_WRB3-0x4000] = apu->rectangle[1].regs[3];
    regs[APU_WRC0-0x4000] = apu->triangle.regs[0];
    regs[APU_WRC2-0x4000] = apu->triangle.regs[1];
    regs[APU_WRC3-0x4000] = apu->triangle.regs[2];
    regs[APU_WRD0-0x4000] = apu->noise.regs[0];
    regs[APU_WRD2-0x4000] = apu->noise.regs[1];
    regs[APU_WRD3-0x4000] = apu->noise.regs[2];
    regs[APU_WRE0-0x4000] = apu->dmc.regs[0];
    regs[APU_WRE1-0x4000] = apu->dmc.regs[1];
    regs[APU_WRE2-0x4000] = apu->dmc.regs[2];
    regs[APU_WRE3-0x4000] = apu->dmc.regs[3];
  }
}

