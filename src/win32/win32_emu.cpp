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

#include "win32_emu.h"
#include "win32_directinput_input_mgr.h"
#include "win32_timing.h"
#include "debug.h"
#include "NES_settings.h"
#include "dinput_headers.h"

#define PROFILE

#define SPEED_THROTTLE

#define SPEED_THROTTLE_KEY  VK_ADD

// these read the keyboard asynchronously
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

win32_emu::win32_emu(HWND parent_window_handle, HINSTANCE parent_instance_handle, const char* ROM_name)
{
  parent_wnd_handle = parent_window_handle;
  scr_mgr = NULL;
  inp_mgr = NULL;
  snd_mgr = &local_null_snd_mgr;
  emu = NULL;

  win32_pad1 = NULL;
  win32_pad2 = NULL;

  SYS_TimeInit();

  try {
    try {
      scr_mgr = new win32_NES_screen_mgr(parent_wnd_handle);
    } catch(...) {
      throw;// "error creating screen manager";
    }

    try {
      inp_mgr = new win32_directinput_input_mgr(parent_wnd_handle, parent_instance_handle);
    } catch(...) {
      throw;// "error creating input manager";
    }

    // get a null sound mgr
		snd_mgr = &local_null_snd_mgr;

    try {
      emu = new NES(ROM_name,scr_mgr,snd_mgr);
    } catch(...) {
      throw;// "error creating emulated NES";
    }

    scr_mgr->setParentNES((NES*)emu);

    CreateWin32Pads();

    // start the timer off right
    reset_last_frame_time();

    // try to init dsound if appropriate
    enable_sound(NESTER_settings.nes.sound.enabled);

    // set up control pads
    emu->set_pad1(&pad1);
    emu->set_pad2(&pad2);

  } catch(...) {
    // careful of the order here
    DeleteWin32Pads();
    if(emu) delete emu;
    if(scr_mgr) delete scr_mgr;
    if(inp_mgr) delete inp_mgr;
  	if(snd_mgr != &local_null_snd_mgr) delete snd_mgr;
    throw;
  }

}

win32_emu::~win32_emu()
{
  DeleteWin32Pads();
  if(emu) delete emu;
  if(scr_mgr) delete scr_mgr;
	if(inp_mgr) delete inp_mgr;
	if(snd_mgr != &local_null_snd_mgr) delete snd_mgr;
}

void win32_emu::PollInput()
{
  // if we don't have the input focus, release all buttons
  if(GetForegroundWindow() != parent_wnd_handle)
  {
	  pad1.release_all_buttons();
	  pad2.release_all_buttons();
    return;
  }

	inp_mgr->Poll();

  if(win32_pad1) win32_pad1->Poll();
  if(win32_pad2) win32_pad2->Poll();
}

void win32_emu::input_settings_changed()
{
  DeleteWin32Pads();
  CreateWin32Pads();
}

void win32_emu::CreateWin32Pads()
{
  win32_directinput_input_mgr* win32_inp_mgr;

  DeleteWin32Pads();

  win32_inp_mgr = (win32_directinput_input_mgr*)inp_mgr; // naughty

  try {
    win32_pad1 = new win32_NES_pad(&NESTER_settings.nes.input.player1, &pad1, win32_inp_mgr);
  } catch(const char* IFDEBUG(s)) {
    LOG("Error creating pad 1 - " << s << endl);
    win32_pad1 = NULL;
  }

  try {
    win32_pad2 = new win32_NES_pad(&NESTER_settings.nes.input.player2, &pad2, win32_inp_mgr);
  } catch(const char* IFDEBUG(s)) {
    LOG("Error creating pad 2 - " << s << endl);
    win32_pad2 = NULL;
  }
}

void win32_emu::DeleteWin32Pads()
{
  if(win32_pad1)
  {
    delete win32_pad1;
    win32_pad1 = NULL;
  }
  if(win32_pad2)
  {
    delete win32_pad2;
    win32_pad2 = NULL;
  }
}

boolean win32_emu::emulate_frame(boolean draw)
{
  return emu->emulate_frame(draw);
}

void win32_emu::onFreeze()
{
  emu->freeze();
}

void win32_emu::onThaw()
{
  emu->thaw();
  reset_last_frame_time();
}

void win32_emu::reset_last_frame_time()
{
  last_frame_time = SYS_TimeInMilliseconds();

#ifdef PROFILE
  last_profile_sec_time = cur_time;
  frames_this_sec = 0;
#endif

}

const char* win32_emu::getROMname()
{
  return emu->getROMname();
}

const char* win32_emu::getROMnameExt()
{
  return emu->getROMnameExt();
}

const char* win32_emu::getROMpath()
{
  return emu->getROMpath();
}

NES_ROM* win32_emu::get_NES_ROM()
{
  return emu->get_NES_ROM();
}

boolean win32_emu::loadState(const char* fn)
{
  boolean result;

  freeze();
  result = emu->loadState(fn);
  thaw();

  return result;
}

boolean win32_emu::saveState(const char* fn)
{
  boolean result;

  freeze();
  result = emu->saveState(fn);
  thaw();

  return result;
}

void win32_emu::reset()
{
  freeze();
  emu->reset();
  thaw();
}

void win32_emu::blt()
{
  scr_mgr->blt();
}

void win32_emu::flip()
{
  scr_mgr->flip();
}

void win32_emu::assert_palette()
{
  scr_mgr->assert_palette();
}

boolean win32_emu::toggle_fullscreen()
{
  return scr_mgr->toggle_fullscreen();
}

void win32_emu::enable_sound(boolean enable)
{
  freeze();

	if(snd_mgr != &local_null_snd_mgr)
	{
		delete snd_mgr;
		snd_mgr = &local_null_snd_mgr;
	}

	if(enable)
	{
		// try to init dsound
		try {
      try {
				snd_mgr = new win32_directsound_sound_mgr(parent_wnd_handle,
					NESTER_settings.nes.sound.sample_rate, NESTER_settings.nes.sound.sample_size,
          NESTER_settings.nes.sound.buffer_len);
      } catch(const char* IFDEBUG(s)) {
        LOG(s << endl);
        throw;
      }
		} catch(...) {
			LOG("Directsound initialization failed" << endl);
    	snd_mgr = &local_null_snd_mgr;
		}
	}

	((NES*)emu)->new_snd_mgr(snd_mgr);

  thaw();
}

boolean win32_emu::sound_enabled()
{
  return !snd_mgr->IsNull();
}

boolean win32_emu::set_sample_rate(int sample_rate)
{
  if(!sound_enabled()) return FALSE;
  if(get_sample_rate() == sample_rate) return TRUE;
  return TRUE;
}

int win32_emu::get_sample_rate()
{
  return snd_mgr->get_sample_rate();
}

// STATIC FUNCTIONS
static inline void SleepUntil(long time)
{
  long timeleft;

  while(1)
  {
    timeleft = time - long(SYS_TimeInMilliseconds());
    if(timeleft <= 0) break;

    if(timeleft > 2)
    {
      Sleep((timeleft) - 1);
    }
  }
}

/*
When the NTSC standard was designed, certain frequencies involved
in the color subcarrier were interfering with the 60 Hz power lines.  So
the NTSC engineers set the framerate to 60000/1001 Hz.  See also
"drop frame timecode" on any search engine for the full story.
*/
#define NTSC_FRAMERATE (60000.0/1001.0)
#define FRAME_PERIOD   (1000.0/NTSC_FRAMERATE)

#define THROTTLE_SPEED  (NESTER_settings.nes.preferences.speed_throttling && KEY_UP(SPEED_THROTTLE_KEY))
#define SKIP_FRAMES     (NESTER_settings.nes.preferences.auto_frameskip && THROTTLE_SPEED)

void win32_emu::do_frame()
{
  uint32 frames_since_last;

  if(frozen()) return;

  // at this point, last_frame_time is set to the time when the last frame was drawn.

  // get the current time
  cur_time = SYS_TimeInMilliseconds();

  // make up for missed frames
  if(SKIP_FRAMES)
  {
    frames_since_last = (uint32)((cur_time - last_frame_time) / FRAME_PERIOD);

    // are there extra frames?
    if(frames_since_last > 1)
    {
      for(uint32 i = 1; i < frames_since_last; i++)
      {
        if(i == 4) break;
        last_frame_time += FRAME_PERIOD;
        emulate_frame(FALSE);
      }
    }
  }

  // emulate current frame
  PollInput();
  if(emulate_frame(TRUE))
  {
    // display frame
    blt();
    flip();
  }

  // sleep until this frame's target time
  if(THROTTLE_SPEED)
  {
    SleepUntil(long(last_frame_time + FRAME_PERIOD));
  }

#ifdef PROFILE
  frames_this_sec++;
  if((cur_time - last_profile_sec_time) > (2.0*1000.0))
  {
    frames_per_sec =
      (double)frames_this_sec * (1000.0/((double)cur_time - (double)last_profile_sec_time));

    LOG((int)frames_per_sec << " FPS ("
      << (int)(100.0 * ((float)frames_per_sec / 60.0)) << "%)" << endl);

    frames_this_sec = 0;
    last_profile_sec_time = cur_time;
  }
#endif

  // get ready for next frame
  if(THROTTLE_SPEED)
  {
    last_frame_time += FRAME_PERIOD;
  }
  else
  {
    last_frame_time = cur_time;
  }
}
