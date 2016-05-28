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

#include "win32_directsound_sound_mgr.h"
#include "debug.h"
#include "iDirectX.h"

win32_directsound_sound_mgr::win32_directsound_sound_mgr(HWND window_handle, int sample_rate, int sample_size, int buffer_length_in_frames)
   : sound_mgr(sample_rate, sample_size, buffer_length_in_frames)
{
  WAVEFORMATEX  pcmwf; // generic waveformat structure
  DSBUFFERDESC  dsbd;  // directsound buffer description

  wnd_handle = window_handle;
  lpDS = NULL;

  buffer_len = (int)((get_sample_rate()*(get_sample_size()/8)*buffer_length_in_frames)/60.0);

  // make SURE the buffer length is even for 16-bit samples
  if((sample_size > 8) && (buffer_len & 1))
  {
    buffer_len++;
  }

  // there are two halves to the buffer
  buffer_len *= 2;

  lpDS = iDirectX::getDirectSound();
  if(!lpDS)
  {
    throw "Error initializing DirectSound";
  }

  if(FAILED(lpDS->SetCooperativeLevel(wnd_handle, DSSCL_PRIORITY/*DSSCL_NORMAL*/)))
    throw "Error setting DirectSound cooperative level";

  // set up the format data structure
  memset(&pcmwf, 0, sizeof(pcmwf));

  pcmwf.wFormatTag	    = WAVE_FORMAT_PCM;
  pcmwf.nChannels		    = 1;
  pcmwf.nSamplesPerSec  = get_sample_rate();
  pcmwf.wBitsPerSample  = get_sample_size();
  pcmwf.nBlockAlign	    = (pcmwf.wBitsPerSample/8)*pcmwf.nChannels;
  pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;

  // create the secondary buffer (no need for a primary)
  memset(&dsbd,0,sizeof(dsbd));
  dsbd.dwSize        = sizeof(DSBUFFERDESC);
  dsbd.dwFlags       = DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2 |
                       /*DSBCAPS_CTRLPOSITIONNOTIFY | */DSBCAPS_GLOBALFOCUS
                       /*| DSBCAPS_PRIMARYBUFFER*/;
  dsbd.dwBufferBytes = get_buffer_len();
  dsbd.lpwfxFormat   = &pcmwf;

  if(FAILED(lpDS->CreateSoundBuffer(&dsbd,&lpDSbSecondary,NULL)))
  {
    throw "Error creating DirectSound buffer";
  }

  // clear out sound buffer
  clear_buffer();

  buffer_locked = FALSE;

  // play the sound in looping mode
  if(FAILED(lpDSbSecondary->Play(0,0,DSBPLAY_LOOPING)))
    throw "Error playing DirectSound buffer";

  playing = TRUE;

  LOG("Directsound initialized, " << get_sample_rate() << " Hz, " <<
      get_sample_size() << "-bit" << endl);
}

win32_directsound_sound_mgr::~win32_directsound_sound_mgr()
{
  if(lpDSbSecondary)
    lpDSbSecondary->Release();
}

void win32_directsound_sound_mgr::reset()
{
  clear_buffer();
  playing = 0;
}

// lock down for a period of inactivity
void win32_directsound_sound_mgr::freeze()
{
  if(playing)
  {
    lpDSbSecondary->Stop();
    playing = 0;
  }
}

void win32_directsound_sound_mgr::thaw()
{
  if(!playing) 
  {
    clear_buffer();
    lpDSbSecondary->Play(0,0,DSBPLAY_LOOPING);
    playing = 1;
  }
}

void win32_directsound_sound_mgr::clear_buffer()
{
  unsigned char *audio_ptr_1 = NULL,   // used to lock memory
                *audio_ptr_2 = NULL;
  DWORD	audio_length_1 = 0,  // length of locked memory
        audio_length_2 = 0;

  if(FAILED(lpDSbSecondary->Lock(0,					 
					                       get_buffer_len(),			
    				                     (void**)&audio_ptr_1, 
					                       &audio_length_1,
					                       (void**)&audio_ptr_2, 
					                       &audio_length_2,
					                       DSBLOCK_FROMWRITECURSOR)))
  {
    LOG("Error locking DirectSound buffer in win32_directsound_sound_mgr::clear_buffer()" << endl);
    return;
  }

  // clear out to silence
  if(get_sample_size() == 8)
  {
    memset(audio_ptr_1, 0x80, audio_length_1);
  
    if(audio_ptr_2)
    {
      memset(audio_ptr_2, 0x80, audio_length_2);
    }
  } else {
    memset(audio_ptr_1, 0x00, audio_length_1);
  
    if(audio_ptr_2)
    {
      memset(audio_ptr_2, 0x00, audio_length_2);
    }
  }

  // unlock the buffer
  if(FAILED(lpDSbSecondary->Unlock((void*)audio_ptr_1, 
                                   audio_length_1, 
                                   (void*)audio_ptr_2, 
                                   audio_length_2)))
  {
    LOG("Error unlocking DirectSound buffer in win32_directsound_sound_mgr::clear_buffer()" << endl);
    return;
  }
}

boolean win32_directsound_sound_mgr::lock(sound_buf_pos which, void** buf, uint32* buf_len)
{
  HRESULT result;

  if(buffer_locked) return FALSE;

  if(get_currently_playing_half() == which) return FALSE;

  // lock the backbuffer surface
  while(1)
  {
    result = lpDSbSecondary->Lock((which == SOUND_BUF_LOW) ? 0 : (get_buffer_len()/2),
              (which == SOUND_BUF_LOW) ? (get_buffer_len()/2) : (get_buffer_len()-(get_buffer_len()/2)),
    				  (void**)&temp_buf, 
					    (unsigned long*)&temp_buf_len,
    				  (void**)&temp_buf2,
					    (unsigned long*)&temp_buf2_len,
					    0);
    if(!FAILED(result))
    {
      break;
    }
    if(result == DSERR_BUFFERLOST)
    {
      result = lpDSbSecondary->Restore();
      if(FAILED(result))
      {
        break;
      }
      continue; // this fixes a bug where while() was
                // terminating before lock() was called
    }
    break;
  }
  if(FAILED(result))
  {
    LOG("Error locking DirectSound buffer in win32_directsound_sound_mgr::lock()" << endl);
    return FALSE;
  }

  *buf = (VOID*)temp_buf;
  *buf_len = (uint32)temp_buf_len;

  buffer_locked = TRUE;

  return TRUE;
}

void win32_directsound_sound_mgr::unlock()
{
  if(!buffer_locked) return;

  lpDSbSecondary->Unlock(temp_buf,
                         temp_buf_len,
                         temp_buf2,
                         temp_buf2_len);
  buffer_locked = FALSE;
}

// returns SOUND_BUF_LOW or SOUND_BUF_HIGH
sound_mgr::sound_buf_pos win32_directsound_sound_mgr::get_currently_playing_half()
{
  DWORD play_pos;
  DWORD write_pos;

  // both positions must be retrieved in DX3
  lpDSbSecondary->GetCurrentPosition(&play_pos, &write_pos);

  if(write_pos >= (DWORD)(get_buffer_len()/2))
  {
    return SOUND_BUF_HIGH;
  }

  return SOUND_BUF_LOW;
}
