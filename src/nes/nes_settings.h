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

#ifndef NES_SETTINGS_H_
#define NES_SETTINGS_H_

#include "types.h"

#include "OSD_NES_graphics_settings.h"
#include "OSD_ButtonSettings.h"

class NES_preferences_settings
{
public:
  enum SAVE_DIR_TYPE { ROM_DIR=0, NESTER_DIR=1 };
  enum NES_PRIORITY  { PRI_NORMAL=0, PRI_HIGH=1, PRI_REALTIME=2 };

  uint32 run_in_background;
  uint32 speed_throttling;
  uint32 auto_frameskip;
  NES_PRIORITY priority;
  SAVE_DIR_TYPE saveRamDirType;
  char saveRamDir[_MAX_PATH];
  SAVE_DIR_TYPE saveStateDirType;
  char saveStateDir[_MAX_PATH];


  void SetDefaults()
  {
    run_in_background = FALSE;
    speed_throttling = TRUE;
    auto_frameskip = TRUE;
    priority = PRI_HIGH;
    saveRamDirType = ROM_DIR;
    strcpy(saveRamDir, "");
    saveStateDirType = ROM_DIR;
    strcpy(saveStateDir, "");
  }

  NES_preferences_settings()
  {
    SetDefaults();
  }
};

class NES_graphics_settings
{
public:
  uint32 black_and_white;
  uint32 show_more_than_8_sprites;
  uint32 show_all_scanlines;
  uint32 draw_overscan;
  uint32 fullscreen_on_load;
  uint32 fullscreen_scaling;
  uint32 calculate_palette;
  uint8 tint;
  uint8 hue;
  OSD_NES_graphics_settings osd;

  void reset_palette()
  {
    tint = 0x86;
    hue  = 0x9d;
  }

  void SetDefaults()
  {
    black_and_white = FALSE;
    show_more_than_8_sprites = FALSE;
    show_all_scanlines = FALSE;
    draw_overscan = FALSE;
    fullscreen_on_load = FALSE;
    fullscreen_scaling = FALSE;
    calculate_palette = FALSE;
    reset_palette();
    osd.Init();
  }

  NES_graphics_settings()
  {
    SetDefaults();
  }
};

class NES_sound_settings
{
public:
  uint32 enabled;
  uint32 sample_rate;
  uint32 sample_size;

  uint32 rectangle1_enabled;
  uint32 rectangle2_enabled;
  uint32 triangle_enabled;
  uint32 noise_enabled;
  uint32 dpcm_enabled;
  uint32 external_enabled;

  enum { LENGTH_MIN = 1, LENGTH_MAX = 10 };
  uint32 buffer_len;

  enum filter_type_t { FILTER_NONE, FILTER_LOWPASS, FILTER_LOWPASS_WEIGHTED };
  filter_type_t filter_type;

  void SetDefaults()
  {
    enabled = TRUE;
    sample_rate = 44100;
    sample_size = 8;
    buffer_len = 3;

    filter_type = FILTER_LOWPASS_WEIGHTED;

    rectangle1_enabled = TRUE;
    rectangle2_enabled = TRUE;
    triangle_enabled = TRUE;
    noise_enabled = TRUE;
    dpcm_enabled = TRUE;
    external_enabled = TRUE;
  }

  NES_sound_settings()
  {
    SetDefaults();
  }
};

class NES_controller_input_settings
{
public:
  OSD_ButtonSettings btnUp;
	OSD_ButtonSettings btnDown;
	OSD_ButtonSettings btnLeft;
	OSD_ButtonSettings btnRight;
	OSD_ButtonSettings btnSelect;
	OSD_ButtonSettings btnStart;
	OSD_ButtonSettings btnB;
	OSD_ButtonSettings btnA;

  // OS-specific
  void OSD_SetDefaults(int num); // 0 == first player

  void Clear()
  {
    btnUp.Clear();
    btnDown.Clear();
    btnLeft.Clear();
    btnRight.Clear();
    btnSelect.Clear();
    btnStart.Clear();
    btnB.Clear();
    btnA.Clear();
  }

  NES_controller_input_settings(int num)
  {
    Clear();
    OSD_SetDefaults(num);
  }
};

class NES_input_settings
{
public:
  NES_controller_input_settings player1;
  NES_controller_input_settings player2;

  void SetDefaults()
  {
    player1.Clear();
    player2.Clear();
    player1.OSD_SetDefaults(0);
    player2.OSD_SetDefaults(1);
  }

  NES_input_settings() : player1(0), player2(1)
  {
  }
};

class NES_settings
{
public:
  NES_preferences_settings  preferences;
  NES_graphics_settings     graphics;
  NES_sound_settings        sound;
  NES_input_settings        input;

  NES_settings() : preferences(), graphics(), sound(), input()
  {
  }
};

#endif
