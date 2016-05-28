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

#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include <stdio.h>
#include "types.h"
#include "screen_mgr.h"
#include "sound_mgr.h"
#include "controller.h"
#include "debug.h"

// @!#?@!
class NES_ROM;

class emulator
{
public:
  emulator(const char* ROM_name = NULL) { num_freezes = 0; };
  virtual ~emulator() {};

  virtual const char* getROMname() = 0;    // returns ROM name without extension
  virtual const char* getROMnameExt() = 0; // returns ROM name with extension
  virtual const char* getROMpath() = 0;

  // oh so dirty.
  virtual NES_ROM* get_NES_ROM() { return NULL; }

  virtual boolean loadState(const char* fn) = 0;
  virtual boolean saveState(const char* fn) = 0;

  virtual boolean emulate_frame(boolean draw) = 0;

  virtual void reset() = 0;

  virtual void set_pad1(controller* c) {}
  virtual void set_pad2(controller* c) {}

  virtual void input_settings_changed() {}

  // sound
  virtual void enable_sound(boolean enable) {};
  virtual boolean sound_enabled() { return FALSE; };
  virtual boolean set_sample_rate(int sample_rate) { return FALSE; };
  virtual int get_sample_rate() { return 0; };

  // freeze() is called when the emulator should
  // shut down for a period of inactivity;
  void freeze()
  {
    if(!num_freezes) onFreeze();
    num_freezes++;
  }
  // thaw() signals the end of the inactive period
  int thaw()
  {
    num_freezes--;
    if(num_freezes < 0)
    {
      LOG("Too many calls to thaw() (emulator.h)" << endl);
      num_freezes = 0;
    }
    else if(!num_freezes)
    {
      onThaw();
    }
    return(0 == num_freezes);
  }

  boolean frozen()
  {
    return(num_freezes != 0);
  }

protected:
  int num_freezes;
  virtual void onFreeze() {}
  virtual void onThaw()   {}
  
private:
};

#endif
