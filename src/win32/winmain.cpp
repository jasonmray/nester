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

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <commdlg.h> // for open dialog
#include <cderr.h>

#include <stdlib.h>

#include "iDirectX.h"

#include "resource.h"
#include "win32_dialogs.h"
#include "win32_emu.h"
#include "win32_directory.h"
#include "settings.h"

#include "version.h"

#include "debug.h"

// this will ignore every incoming 0x0118 message
// windows will continue to send it regularly when ignored
#define TOOLTIP_HACK

// WM_* in "winuser.h"
//#define DUMP_WM_MESSAGES

#ifdef DUMP_WM_MESSAGES
#define DUMP_WM_MESSAGE(NUM,MSG) \
  LOG("WM" << (NUM) << ":" << HEX((MSG),4) << ",")
#else
#define DUMP_WM_MESSAGE(NUM,MSG)
#endif


#define WINCLASS_NAME "WinClass_nester"

#ifdef OFFICIAL_NESTER
#define PROG_NAME "nester"
#else
#define PROG_NAME "nester (unofficial)"
#endif

int savestate_slot = 0;

#define SCREEN_WIDTH_WINDOWED   (NESTER_settings.nes.graphics.osd.double_size ? \
                                  2*NES_PPU::NES_SCREEN_WIDTH_VIEWABLE : \
                                  NES_PPU::NES_SCREEN_WIDTH_VIEWABLE)
#define SCREEN_HEIGHT_WINDOWED  (NESTER_settings.nes.graphics.osd.double_size ? \
                                  2*NES_PPU::getViewableHeight() : \
                                  NES_PPU::getViewableHeight())

// used for centering
#define APPROX_WINDOW_WIDTH  (SCREEN_WIDTH_WINDOWED + 2*GetSystemMetrics(SM_CXFIXEDFRAME))
#define APPROX_WINDOW_HEIGHT (SCREEN_HEIGHT_WINDOWED + GetSystemMetrics(SM_CYMENU) \
                         + GetSystemMetrics(SM_CYSIZE) \
                         + 2*GetSystemMetrics(SM_CYFIXEDFRAME) \
                         + 1)

#define STYLE_WINDOWED (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | \
      WS_MINIMIZEBOX /*| *//*WS_MAXIMIZEBOX | *//*WS_POPUP*//* | WS_VISIBLE*/)
#define STYLE_FULLSCREEN (WS_VISIBLE | WS_EX_TOPMOST)

#define INACTIVE_PRIORITY   NORMAL_PRIORITY_CLASS

#define TIMER_ID_MOUSE_HIDE       1
#define MOUSE_HIDE_DELAY_SECONDS  1
static int hide_mouse;
static UINT mouse_timer = 0;

#define TIMER_ID_UNSTICK_MESSAGE_PUMP 2
#define TIMER_UNSTICK_MILLISECONDS 1
static UINT unstick_timer = 0;

// use this after every thaw()
// sends a one-shot timer message
// needed due to the structure of the main message loop
// lets nester achieve <1% CPU usage when idle
#define UNSTICK_MESSAGE_PUMP \
  unstick_timer = SetTimer(main_window_handle, TIMER_ID_UNSTICK_MESSAGE_PUMP, \
                           TIMER_UNSTICK_MILLISECONDS, NULL)

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle = NULL; // save the window handle
HINSTANCE g_main_instance = NULL; // save the instance

win32_emu* emu;

static int g_foreground;
static int g_minimized;

static int g_Paused = 0;

int ncbuttondown_flag = 0;
///////////////////////////////////////////////////////////

static void setWindowedWindowStyle()
{
  SetWindowLong(main_window_handle, GWL_STYLE, STYLE_WINDOWED | (emu ? WS_MAXIMIZEBOX : 0));
  SetWindowPos(main_window_handle, HWND_NOTOPMOST, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}

static void setFullscreenWindowStyle()
{
  SetWindowLong(main_window_handle, GWL_STYLE, STYLE_FULLSCREEN);
  SetWindowPos(main_window_handle, HWND_NOTOPMOST, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}

static void assertWindowStyle()
{
  if(emu)
  {
    if(emu->is_fullscreen())
    {
      setFullscreenWindowStyle();
      return;
    }
  }

  setWindowedWindowStyle();
}

static void set_priority(DWORD priority)
{
  // set process priority
  HANDLE pid;

  pid = GetCurrentProcess();

  SetPriorityClass(pid, priority);
}

static void assert_priority()
{
  switch(NESTER_settings.nes.preferences.priority)
  {
    case NES_preferences_settings::PRI_NORMAL:
      set_priority(NORMAL_PRIORITY_CLASS);
      break;

    case NES_preferences_settings::PRI_HIGH:
      set_priority(HIGH_PRIORITY_CLASS);
      break;

    case NES_preferences_settings::PRI_REALTIME:
      set_priority(REALTIME_PRIORITY_CLASS);
      break;
  }
}

static boolean GetROMFileName(char* filenamebuf, const char* path)
{
  OPENFILENAME OpenFileName;
  char szFilter[1024] = {
    "NES ROM (*.nes)\0*.nes\0" \
/*
    "GameBoy ROM (*.gb;*.gbc)\0*.gb;*.gbc\0" \
    "SNES ROM (*.smc)\0*.smc\0" \
    "N64 ROM (*.n64)\0*.n64\0" \
*/
    ""
  };
  char szFile[1024] = "";
  char szFileTitle[1024];

  int ret;

  memset(&OpenFileName, 0x00, sizeof(OpenFileName));
  OpenFileName.lStructSize = sizeof (OPENFILENAME);
  OpenFileName.hwndOwner = main_window_handle;
  OpenFileName.hInstance = g_main_instance;
  OpenFileName.lpstrFilter = szFilter;
  OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
  OpenFileName.nMaxCustFilter = 0L;
  OpenFileName.nFilterIndex = 1L;
  OpenFileName.lpstrFile = szFile;
  OpenFileName.nMaxFile = sizeof(szFile);
  OpenFileName.lpstrFileTitle = szFileTitle;
  OpenFileName.nMaxFileTitle = sizeof(szFileTitle);
  if(path)
    OpenFileName.lpstrInitialDir = path;
  else
    OpenFileName.lpstrInitialDir = ".";
  OpenFileName.lpstrTitle = "Open ROM";
  OpenFileName.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;
  OpenFileName.nFileOffset = 0;
  OpenFileName.nFileExtension = 0;
  OpenFileName.lpstrDefExt = "nes";
  OpenFileName.lCustData = 0;

  if((ret = GetOpenFileName(&OpenFileName)) == 0)
  {
    return FALSE;
  }

  if(OpenFileName.lpstrFileTitle)
  {
    strcpy(filenamebuf, OpenFileName.lpstrFileTitle);
    return TRUE;
  }

  return FALSE;
}

static boolean GetLoadSavestateFileName(char* filenamebuf, const char* path, const char* RomName)
{
  OPENFILENAME OpenFileName;
  char szFilter[1024];
  char szFile[1024];
  char szFileTitle[1024];

  int ret;

  // create the default filename
  strcpy(szFile, filenamebuf);

  // create the filter
  {
    char* p = szFilter;
    sprintf(p, "savestate (%s.ss?)", RomName);
    p += strlen(p)+1;
    sprintf(p, "%s.ss?", RomName);
    p += strlen(p)+1;
    strcpy(p, "All Files (*.*)");
    p += strlen(p)+1;
    strcpy(p, "*.*");
    p += strlen(p)+1;
    *p = '\0';
  }

  memset(&OpenFileName, 0x00, sizeof(OpenFileName));
  OpenFileName.lStructSize = sizeof (OPENFILENAME);
  OpenFileName.hwndOwner = main_window_handle;
  OpenFileName.hInstance = g_main_instance;
  OpenFileName.lpstrFilter = szFilter;
  OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
  OpenFileName.nMaxCustFilter = 0L;
  OpenFileName.nFilterIndex = 1L;
  OpenFileName.lpstrFile = szFile;
  OpenFileName.nMaxFile = sizeof(szFile);
  OpenFileName.lpstrFileTitle = szFileTitle;
  OpenFileName.nMaxFileTitle = sizeof(szFileTitle);
  OpenFileName.lpstrInitialDir = path;
  OpenFileName.lpstrTitle = "Load State";
  OpenFileName.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;
  OpenFileName.nFileOffset = 0;
  OpenFileName.nFileExtension = 0;
  OpenFileName.lpstrDefExt = NULL;
  OpenFileName.lCustData = 0;

  if((ret = GetOpenFileName(&OpenFileName)) == 0)
  {
    return FALSE;
  }

  if(OpenFileName.lpstrFileTitle)
  {
    strcpy(filenamebuf, OpenFileName.lpstrFileTitle);
    return TRUE;
  }

  return FALSE;
}

static boolean GetSaveSavestateFileName(char* filenamebuf, const char* path, const char* RomName)
{
  OPENFILENAME OpenFileName;
  char szFilter[1024];
  char szFile[1024];
  char szFileTitle[1024];

  int ret;

  // create the default filename
  strcpy(szFile, filenamebuf);

  // create the filter
  {
    char* p = szFilter;
    sprintf(p, "save state (%s.ss?)", RomName);
    p += strlen(p)+1;
    sprintf(p, "%s.ss?", RomName);
    p += strlen(p)+1;
    strcpy(p, "All Files (*.*)");
    p += strlen(p)+1;
    strcpy(p, "*.*");
    p += strlen(p)+1;
    *p = '\0';
  }

  memset(&OpenFileName, 0x00, sizeof(OpenFileName));
  OpenFileName.lStructSize = sizeof (OPENFILENAME);
  OpenFileName.hwndOwner = main_window_handle;
  OpenFileName.hInstance = g_main_instance;
  OpenFileName.lpstrFilter = szFilter;
  OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
  OpenFileName.nMaxCustFilter = 0L;
  OpenFileName.nFilterIndex = 1L;
  OpenFileName.lpstrFile = szFile;
  OpenFileName.nMaxFile = sizeof(szFile);
  OpenFileName.lpstrFileTitle = szFileTitle;
  OpenFileName.nMaxFileTitle = sizeof(szFileTitle);
  OpenFileName.lpstrInitialDir = path;
  OpenFileName.lpstrTitle = "Save State";
  OpenFileName.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;
  OpenFileName.nFileOffset = 0;
  OpenFileName.nFileExtension = 0;
  OpenFileName.lpstrDefExt = NULL; //"ss0";
  OpenFileName.lCustData = 0;

  if((ret = GetSaveFileName(&OpenFileName)) == 0)
  {
    return FALSE;
  }

  if(OpenFileName.lpstrFileTitle)
  {
    strcpy(filenamebuf, OpenFileName.lpstrFileTitle);
    return TRUE;
  }

  return FALSE;
}

// returns -1 if not found
int GetMenuItemIndex(HMENU hMenu, const char* ItemName)
{
  int index = 0;
  char buf[256];

  while(index < GetMenuItemCount(hMenu))
  {
    if(GetMenuString(hMenu, index, buf, sizeof(buf)-1, MF_BYPOSITION))
    {
      if(!strcmp(ItemName, buf))
        return index;
    }
    index++;
  }
  return -1;
}

void UpdateSaveStateSlotMenu(HWND hMainWindow)
{
  HMENU hMainMenu;
  HMENU hFileMenu;
  HMENU hSlotMenu;
  int index;

  // get main menu handle
  hMainMenu = GetMenu(hMainWindow);
  if(!hMainMenu) return;

  // get file menu index
  index = GetMenuItemIndex(hMainMenu, "&File");
  if(index < 0) return;
  // get file menu handle
  hFileMenu = GetSubMenu(hMainMenu, index);
  if(!hFileMenu) return;

  // get slot menu index
  index = GetMenuItemIndex(hFileMenu, "Savestate Slot");
  if(index < 0) return;
  // get slot menu handle
  hSlotMenu = GetSubMenu(hFileMenu, index);
  if(!hSlotMenu) return;

  CheckMenuRadioItem(hSlotMenu, 0, 9, savestate_slot, MF_BYPOSITION);
}

void UpdateRecentROMMenu(HWND hMainWindow)
{
  HMENU hMainMenu;
  HMENU hFileMenu;
  HMENU hRecentMenu;
  int index;

  // get main menu handle
  hMainMenu = GetMenu(hMainWindow);
  if(!hMainMenu) return;

  // get file menu index
  index = GetMenuItemIndex(hMainMenu, "&File");
  if(index < 0) return;
  // get file menu handle
  hFileMenu = GetSubMenu(hMainMenu, index);
  if(!hFileMenu) return;

  // get recent menu index
  index = GetMenuItemIndex(hFileMenu, "Recent ROMs");
  if(index < 0) return;
  // get recent menu handle
  hRecentMenu = GetSubMenu(hFileMenu, index);
  if(!hRecentMenu) return;

  // clear out the menu
  while(GetMenuItemCount(hRecentMenu))
  {
    DeleteMenu(hRecentMenu, 0, MF_BYPOSITION);
  }

  // if no recent files, add a dummy
  if(!NESTER_settings.recent_ROMs.get_num_entries())
  {
    AppendMenu(hRecentMenu, MF_GRAYED | MF_STRING, ID_FILE_RECENT_0, "None");
  }
  else
  {
    int i = 0;
    char text[256];

    for(i = 0; i < NESTER_settings.recent_ROMs.get_num_entries(); i++)
    {
      sprintf(text, "&%u ", i);
      // handle filenames with '&' in them
      for(uint32 k = 0; k < strlen(NESTER_settings.recent_ROMs.get_entry(i)); k++)
      {
        char temp[2] = " ";
        temp[0] = NESTER_settings.recent_ROMs.get_entry(i)[k];
        strcat(text, temp);
        if(temp[0] == '&') strcat(text, temp);
      }
      AppendMenu(hRecentMenu, MF_STRING, ID_FILE_RECENT_0+i, text);
    }
  }

  DrawMenuBar(hMainWindow);
}

///////////////////////////////////////////////////////
void freeze()
{
  if(emu)
  {
    emu->freeze();
  }
}

void thaw()
{
  if(emu)
  {
    if(emu->thaw())
    {
      assert_priority();
      UNSTICK_MESSAGE_PUMP;
    }
  }
}

static char orig_titlebar[256];

static void Pause()
{
  char new_titlebar[256];
  if(!g_Paused)
  {
    g_Paused = 1;
    freeze();
    GetWindowText(main_window_handle, orig_titlebar, sizeof(orig_titlebar));
    strcpy(new_titlebar, orig_titlebar);
    strcat(new_titlebar, " -- PAUSED");
    SetWindowText(main_window_handle, new_titlebar);
  }
}

static void UnPause()
{
  if(g_Paused)
  {
    g_Paused = 0;
    SetWindowText(main_window_handle, orig_titlebar);
    thaw();
  }
}

///////////////////////////////////////////////////////

void init()
{
  emu = NULL;

  g_foreground = 1;
  g_minimized = 0;
}

void shutdown()
{
  if(emu)
  {
    delete emu;
    emu = NULL;
  }
}

void toggle_fullscreen()
{
  static RECT win_rect;
  static HMENU win_menu;

  if(!emu) return;

  freeze();

  if(emu->is_fullscreen())
  {
    setWindowedWindowStyle();

    SetMenu(main_window_handle, win_menu);

    emu->toggle_fullscreen();

    SetWindowPos(main_window_handle, HWND_NOTOPMOST,
      win_rect.left, win_rect.top,
      (win_rect.right - win_rect.left),
      (win_rect.bottom - win_rect.top), SWP_SHOWWINDOW);
  }
  else
  {
    GetWindowRect(main_window_handle, &win_rect);

    if(emu->toggle_fullscreen())
    {
      setFullscreenWindowStyle();

      win_menu = GetMenu(main_window_handle);
      SetMenu(main_window_handle, NULL);
    }
    else
    {
      // recover gracefully
      SetWindowPos(main_window_handle, HWND_NOTOPMOST,
        win_rect.left, win_rect.top,
        (win_rect.right - win_rect.left),
        (win_rect.bottom - win_rect.top), SWP_SHOWWINDOW);
    }
  }

  // update the palette
  SendMessage(main_window_handle, WM_QUERYNEWPALETTE, 0, 0);

  // make sure cursor is hidden in fullscreen mode
  SendMessage(main_window_handle, WM_SETCURSOR, 0, 0);

  thaw();
}

void assertWindowSize()
{
  RECT rct;

  // set rc to client size
  SetRect(&rct, 0, 0, SCREEN_WIDTH_WINDOWED, SCREEN_HEIGHT_WINDOWED);

  // adjust rc to full window size
  AdjustWindowRectEx(&rct,
                     GetWindowStyle(main_window_handle),
                     GetMenu(main_window_handle) != NULL,
                     GetWindowExStyle(main_window_handle));

  SetWindowPos(main_window_handle, HWND_TOP, 0, 0,
    rct.right-rct.left, rct.bottom-rct.top, SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);
}

void CenterWindow()
{
  RECT rct;

  // set rc to client size
  SetRect(&rct, 0, 0, SCREEN_WIDTH_WINDOWED, SCREEN_HEIGHT_WINDOWED);

  // adjust rc to full window size
  AdjustWindowRectEx(&rct,
                     GetWindowStyle(main_window_handle),
                     GetMenu(main_window_handle) != NULL,
                     GetWindowExStyle(main_window_handle));

  SetWindowPos(main_window_handle, HWND_TOP,
               GetSystemMetrics(SM_CXFULLSCREEN)/2 - (rct.right-rct.left)/2,
               GetSystemMetrics(SM_CYFULLSCREEN)/2 - (rct.bottom-rct.top)/2,
               0, 0, SWP_NOCOPYBITS | SWP_NOSIZE | SWP_NOZORDER);
}

void MakeSaveStateFilename(char* buf)
{
  char extension[5];

  if(emu)
  {
    sprintf(extension, ".ss%i", savestate_slot);
    DIR_createNesFileName(emu->get_NES_ROM(), buf,
      NESTER_settings.nes.preferences.saveStateDirType,
      NESTER_settings.nes.preferences.saveStateDir,
      emu->getROMname(), extension);
/*
    strcpy(buf, emu->getROMpath());
    strcat(buf, emu->getROMname());
    sprintf(extension, ".ss%i", savestate_slot);
    strcat(buf, extension);
*/
  }
  else
  {
    strcpy(buf, "");
  }
}

void MakeShortSaveStateFilename(char* buf)
{
  char extension[5];

  if(emu)
  {
    strcpy(buf, emu->getROMname());
    sprintf(extension, ".ss%i", savestate_slot);
    strcat(buf, extension);
  }
  else
  {
    strcpy(buf, "");
  }
}

void LoadROM(const char* rom_name)
{
  UnPause();

  if(emu)
  {
    delete emu;
    emu = NULL;
  }
  try {
    char full_name_with_path[_MAX_PATH];

    if(!_fullpath(full_name_with_path, rom_name, sizeof(full_name_with_path)))
    {
      throw "error loading ROM: path not found";
    }

    emu = new win32_emu(main_window_handle, g_main_instance, full_name_with_path);

    // add the ROM to the recent ROM list
    NESTER_settings.recent_ROMs.add_entry(full_name_with_path);

    // set the Open directory
    strcpy(NESTER_settings.OpenPath, emu->getROMpath());

    // assert the priority
    assert_priority();

    assertWindowStyle();

    // update the palette
    SendMessage(main_window_handle, WM_QUERYNEWPALETTE, 0, 0);

    if(NESTER_settings.nes.graphics.fullscreen_on_load)
    {
      if(!emu->is_fullscreen())
      {
        toggle_fullscreen();
      }
    }

  } catch(const char* m) {
    MessageBox(main_window_handle, m, "ERROR", MB_OK);
    LOG(m << endl);
  } catch(...) {
    char *err = "unknown error while loading ROM";
    MessageBox(main_window_handle, err, "ERROR", MB_OK);
    LOG(err << endl);
  }
}

void FreeROM()
{
  UnPause();

  if(emu)
  {
    delete emu;
    emu = NULL;
  }

  assertWindowStyle();
}

void LoadCmdLineROM(char* rom_name)
{
  if(!strlen(rom_name)) return;

  // are there quotes?
  if(rom_name[0] == '"')
  {
    rom_name++;
    if(rom_name[strlen(rom_name)-1] == '"')
    {
      rom_name[strlen(rom_name)-1] = '\0';
    }
  }

  if(!strlen(rom_name)) return;

  // try to load up a ROM from the command line
  LoadROM(rom_name);
}

void ShowAboutDialog(HWND hWnd)
{
  freeze();
  DialogBox(g_main_instance, MAKEINTRESOURCE(IDD_HELP_ABOUT),
            hWnd, AboutNester_DlgProc);
  thaw();
}

LRESULT CALLBACK WindowProc(HWND hwnd, 
                            UINT msg, 
                            WPARAM wparam, 
                            LPARAM lparam)
{
  // this is the main message handler of the system
  PAINTSTRUCT  ps;      // used in WM_PAINT
  HDC          hdc;     // handle to a device context

  DUMP_WM_MESSAGE(3,msg);

  // what is the message 
  switch(msg)
  {
    case WM_COMMAND:
      // Handle all menu and accelerator commands 
      switch (LOWORD(wparam))
      {
        case ID_FILE_EXIT:
          if(emu)
          {
            SendMessage(main_window_handle, WM_COMMAND, ID_FILE_CLOSE_ROM, 0L);
          }
          PostMessage(main_window_handle, WM_CLOSE, 0, 0L);
          return 0L;
          break;

        case ID_FILE_OPEN_ROM:
          if((!emu) || (emu && (!emu->is_fullscreen())))
          {
            char filename[_MAX_PATH];
            freeze();
            if(GetROMFileName(filename, NESTER_settings.OpenPath))
            {
              LoadROM(filename);
            }
            else
            {
              thaw();
            }
          }
          return 0;
          break;

        case ID_FILE_CLOSE_ROM:
          FreeROM();
          return 0;
          break;

        case ID_FILE_RESET:
          UnPause();
          emu->reset();
          return 0;
          break;

        case ID_FILE_ROMINFO:
          if((!emu) || (emu && (!emu->is_fullscreen())))
          {
            freeze();
            DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_ROMINFO),
                           hwnd, ROMInfo_DlgProc, (LPARAM)emu->get_NES_ROM());
            thaw();
          }
          return 0;
          break;

        case ID_FILE_PAUSE:
          if(g_Paused)
          {
            UnPause();
          }
          else
          {
            Pause();
          }
          return 0;
          break;

        case ID_FILE_LOAD_STATE:
          freeze();
          {
            char savestate_filename[_MAX_PATH];
            MakeShortSaveStateFilename(savestate_filename);
            if(GetLoadSavestateFileName(savestate_filename, emu->getROMpath(), emu->getROMname()))
            {
              UnPause();
              emu->loadState(savestate_filename);
            }
          }
          thaw();
          return 0;
          break;

        case ID_FILE_SAVE_STATE:
          freeze();
          {
            char savestate_filename[_MAX_PATH];
            MakeShortSaveStateFilename(savestate_filename);
            if(GetSaveSavestateFileName(savestate_filename, emu->getROMpath(), emu->getROMname()))
            {
              emu->saveState(savestate_filename);
            }
          }
          thaw();
          return 0;
          break;

        case ID_FILE_QUICK_LOAD:
          {
            char savestate_filename[_MAX_PATH];
            MakeSaveStateFilename(savestate_filename);
            UnPause();
            emu->loadState(savestate_filename);
          }
          return 0;
          break;

        case ID_FILE_QUICK_SAVE:
          {
            char savestate_filename[_MAX_PATH];
            MakeSaveStateFilename(savestate_filename);
            emu->saveState(savestate_filename);
          }
          return 0;
          break;

        case ID_FILE_SLOT_0:
        case ID_FILE_SLOT_1:
        case ID_FILE_SLOT_2:
        case ID_FILE_SLOT_3:
        case ID_FILE_SLOT_4:
        case ID_FILE_SLOT_5:
        case ID_FILE_SLOT_6:
        case ID_FILE_SLOT_7:
        case ID_FILE_SLOT_8:
        case ID_FILE_SLOT_9:
          {
            int index = LOWORD(wparam) - ID_FILE_SLOT_0;
            savestate_slot = index;
          }
          return 0;
          break;

        case ID_FILE_RECENT_0:
        case ID_FILE_RECENT_1:
        case ID_FILE_RECENT_2:
        case ID_FILE_RECENT_3:
        case ID_FILE_RECENT_4:
        case ID_FILE_RECENT_5:
        case ID_FILE_RECENT_6:
        case ID_FILE_RECENT_7:
        case ID_FILE_RECENT_8:
        case ID_FILE_RECENT_9:
          {
            int index = LOWORD(wparam) - ID_FILE_RECENT_0;
            if(NESTER_settings.recent_ROMs.get_entry(index))
              LoadROM(NESTER_settings.recent_ROMs.get_entry(index));
          }
          return 0;
          break;

        case ID_OPTIONS_PREFERENCES:
          if((!emu) || (emu && (!emu->is_fullscreen())))
          {
            int run_in_background = NESTER_settings.nes.preferences.run_in_background;

            freeze();

            DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_OPTIONS_PREFERENCES),
                           hwnd, PreferencesOptions_DlgProc, (LPARAM)&NESTER_settings);

            // if run in background setting has changed, we need to adjust freeze state
            if(run_in_background && 
               !NESTER_settings.nes.preferences.run_in_background)
            {
              freeze();
            }
            else if(!run_in_background && 
                    NESTER_settings.nes.preferences.run_in_background)
            {
              thaw();
            }

            thaw();
          }
          return 0;
          break;

        case ID_OPTIONS_GRAPHICS:
          if((!emu) || (emu && (!emu->is_fullscreen())))
          {
            freeze();
            DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_OPTIONS_GRAPHICS),
                              hwnd, GraphicsOptions_DlgProc, (LPARAM)&NESTER_settings);
            SendMessage(main_window_handle, WM_QUERYNEWPALETTE, 0, 0);
            SendMessage(main_window_handle, WM_PAINT, 0, 0);
            assertWindowSize();
            thaw();
          }
          return 0;
          break;

        case ID_OPTIONS_SOUND:
          if((!emu) || (emu && (!emu->is_fullscreen())))
          {
            freeze();
            if(DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_OPTIONS_SOUND),
                              hwnd, SoundOptions_DlgProc, (LPARAM)&NESTER_settings))
            {
              if(emu)
              {
                emu->enable_sound(NESTER_settings.nes.sound.enabled);
              }
            }
            thaw();
          }
          return 0;
          break;

        case ID_OPTIONS_CONTROLLERS:
          if((!emu) || (emu && (!emu->is_fullscreen())))
          {
            freeze();
            if(DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_OPTIONS_CONTROLLERS),
                              hwnd, ControllersOptions_DlgProc, (LPARAM)&NESTER_settings))
            {
              if(emu) emu->input_settings_changed();
            }
            thaw();
          }
          return 0;
          break;

        case ID_OPTIONS_DOUBLESIZE:
          if(!emu || (emu && !emu->is_fullscreen()))
          {
            NESTER_settings.nes.graphics.osd.double_size = !NESTER_settings.nes.graphics.osd.double_size;
            assertWindowSize();
          }
          return 0;
          break;

        case ID_OPTIONS_FULLSCREEN:
          if(emu)
          {
            toggle_fullscreen();
          }
          return 0;
          break;

        case ID_HELP_ABOUT:
          if(emu)
          {
            if(!emu->is_fullscreen())
            {
              ShowAboutDialog(hwnd);
            }
          }
          else
          {
            ShowAboutDialog(hwnd);
          }
          return 0;
          break;
      }
      break;

    case WM_INITMENUPOPUP:
      switch(LOWORD(lparam))
        {
        case 0: // file menu
          {
            UINT flag;

            flag = emu ? MF_ENABLED : MF_GRAYED;
            EnableMenuItem((HMENU)wparam, ID_FILE_CLOSE_ROM,  flag);
            EnableMenuItem((HMENU)wparam, ID_FILE_RESET,      flag);
            EnableMenuItem((HMENU)wparam, ID_FILE_ROMINFO,    flag);
            EnableMenuItem((HMENU)wparam, ID_FILE_LOAD_STATE, flag);
            EnableMenuItem((HMENU)wparam, ID_FILE_SAVE_STATE, flag);
            EnableMenuItem((HMENU)wparam, ID_FILE_QUICK_LOAD, flag);
            EnableMenuItem((HMENU)wparam, ID_FILE_QUICK_SAVE, flag);

            // handle pause check mark
            {
              MENUITEMINFO menuItemInfo;

              memset((void*)&menuItemInfo, 0x00, sizeof(menuItemInfo));
              menuItemInfo.cbSize = sizeof(menuItemInfo);
              menuItemInfo.fMask = MIIM_STATE;
              menuItemInfo.fState = g_Paused ? MFS_CHECKED : MFS_UNCHECKED;
              menuItemInfo.fState |= emu ? MF_ENABLED : MF_GRAYED;
              menuItemInfo.hbmpChecked = NULL;
              SetMenuItemInfo((HMENU)wparam, ID_FILE_PAUSE, FALSE, &menuItemInfo);
            }

            UpdateSaveStateSlotMenu(main_window_handle);
            UpdateRecentROMMenu(main_window_handle);
          }
          break;

        case 1: // options menu
          {
            UINT flag;

            flag = emu ? MF_ENABLED : MF_GRAYED;

            EnableMenuItem((HMENU)wparam, ID_OPTIONS_PREFERENCES, MF_ENABLED);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_GRAPHICS, MF_ENABLED);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_SOUND, MF_ENABLED);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_CONTROLLERS, MF_ENABLED);

            // handle double-size check mark
            {
              MENUITEMINFO menuItemInfo;

              memset((void*)&menuItemInfo, 0x00, sizeof(menuItemInfo));
              menuItemInfo.cbSize = sizeof(menuItemInfo);
              menuItemInfo.fMask = MIIM_STATE;
              menuItemInfo.fState = NESTER_settings.nes.graphics.osd.double_size ? MFS_CHECKED : MFS_UNCHECKED;
              menuItemInfo.hbmpChecked = NULL;
              SetMenuItemInfo((HMENU)wparam, ID_OPTIONS_DOUBLESIZE, FALSE, &menuItemInfo);
            }

            EnableMenuItem((HMENU)wparam, ID_OPTIONS_FULLSCREEN, flag);
          }
          break;
      }
      break;

    case WM_KEYDOWN:
      switch(wparam)
      {
        case VK_ESCAPE:
          if(emu)
          {
            // if ESC is pressed in fullscreen mode, go to windowed mode
            if(emu->is_fullscreen())
            {
              PostMessage(main_window_handle, WM_COMMAND,ID_OPTIONS_FULLSCREEN,0);
            }
          }
          break;
      }
      break;

    case WM_DROPFILES:
      if ((!emu) || (emu && !emu->is_fullscreen()))
      {
        char filename[_MAX_PATH];

        freeze();

        // file has been dropped onto window
        strcpy(filename, "");
        DragQueryFile((HDROP)wparam,0,filename, sizeof(filename));

        SetForegroundWindow(main_window_handle);

        // if emulator active, try and load a savestate
        if(emu)
        {
          if(!emu->loadState(filename))
          {
            // not a savestate, try loading as a ROM
            LoadROM(filename);
          }
        }
        else
        {
          // try loading as a ROM
          LoadROM(filename);
        }

        thaw();
      }
      return 0;
      break;

    case WM_CREATE:
      // do initialization stuff here
      init();

      DragAcceptFiles(hwnd, TRUE);

      return(0);
      break;

    case WM_DESTROY:
      shutdown();

      DragAcceptFiles(hwnd, FALSE);

      // kill the application      
      PostQuitMessage(0);
      return(0);
      break;

    case WM_KILLFOCUS:
      if(g_foreground)
      {
        g_foreground = 0;
        if(!NESTER_settings.nes.preferences.run_in_background)
        {
          freeze();
        }
      }
      return 0;
      break;
    case WM_SETFOCUS:
      if(!g_foreground)
      {
        g_foreground = 1;
        if(!NESTER_settings.nes.preferences.run_in_background)
        {
          thaw();
        }
      }
      return 0;
      break;

    // user is using menu
    case WM_ENTERMENULOOP:
      freeze();
      DefWindowProc(hwnd, msg, wparam, lparam);
      return 0;
      break;
    case WM_EXITMENULOOP:
      DefWindowProc(hwnd, msg, wparam, lparam);
      thaw();
      return 0;
      break;

    // user is moving window
    case WM_ENTERSIZEMOVE:
      freeze();
      DefWindowProc(hwnd, msg, wparam, lparam);
      return 0;
      break;
    case WM_EXITSIZEMOVE:
      if(ncbuttondown_flag)
      {
        ncbuttondown_flag = 0;
        thaw();
      }
      DefWindowProc(hwnd, msg, wparam, lparam);
      thaw();
      return 0;
      break;


    // user has clicked the title bar
    case WM_NCLBUTTONDOWN:
      {
        if(wparam == HTCAPTION)
        {
          ncbuttondown_flag = 1;
          freeze();
          DefWindowProc(hwnd, msg, wparam, lparam);
          return 0;
        }
      }
      break;
    // this only comes if WM_NCLBUTTONDOWN and SC_MOVE both return 0
    // not even... I'll leave it in anyway, with a guard around thaw()
    case WM_NCLBUTTONUP:
      {
        if(wparam == HTCAPTION)
        {
          DefWindowProc(hwnd, msg, wparam, lparam);
          if(ncbuttondown_flag)
          {
            ncbuttondown_flag = 0;
            thaw();
          }
          return 0;
        }
      }
      break;
    // this is sent after WM_NCLBUTTONDOWN, when the button is released, or a move cycle starts
    case WM_CAPTURECHANGED:
      DefWindowProc(hwnd, msg, wparam, lparam);
      if(ncbuttondown_flag)
      {
        ncbuttondown_flag = 0;
        thaw();
      }
      return 0;
      break;


    case WM_MOVE:
      if(emu)
      {
        if(!emu->is_fullscreen())
        {
          emu->blt();
        }
      }
      break;

    case WM_PAINT:
      // start painting
      hdc = BeginPaint(hwnd, &ps);
      // end painting
      EndPaint(hwnd, &ps);
      if(emu)
      {
        if(!emu->is_fullscreen())
          emu->blt();
      }
      return(0);
      break;

    case WM_QUERYNEWPALETTE:
      if(emu)
      {
        if(!emu->is_fullscreen())
        {
          try {
            emu->assert_palette();
          } catch(...) {
            return FALSE;
          }
          return TRUE;
        }
      }
      else
      {
        return FALSE;
      }
      break;

    case WM_MOUSEMOVE:
      {
        static LPARAM last_pos;

        if(lparam != last_pos)
        {
          last_pos = lparam;
          hide_mouse = 0;
          mouse_timer = SetTimer(main_window_handle, TIMER_ID_MOUSE_HIDE, 1000*MOUSE_HIDE_DELAY_SECONDS, NULL);
        }
      }
      break;

    case WM_NCMOUSEMOVE:
      hide_mouse = 0;
      if(mouse_timer)
      {
        KillTimer(main_window_handle, mouse_timer);
        mouse_timer = 0;
      }
      break;

    case WM_TIMER:
      switch(wparam)
      {
        case TIMER_ID_MOUSE_HIDE:
          if(mouse_timer)
          {
            KillTimer(main_window_handle, mouse_timer);
            mouse_timer = 0;
          }
          hide_mouse = 1;
          if(emu)
          {
            SetCursor(NULL); // hide the mouse cursor
          }
          return 0;
          break;
        case TIMER_ID_UNSTICK_MESSAGE_PUMP:
          if(unstick_timer)
          {
            KillTimer(main_window_handle, unstick_timer);
            unstick_timer = 0;
          }
          return 0;
          break;
      }
      break;

    case WM_SETCURSOR:
      if(emu)
      {
        if(emu->is_fullscreen())
        {
          SetCursor(NULL); // hide the mouse cursor
          return TRUE;
        }
        else
        {
          if(hide_mouse)
          {
            if(!emu->frozen())
            {
              SetCursor(NULL); // hide the mouse cursor
              return TRUE;
            }
          }
        }
      }
      break;

    case WM_SYSCOMMAND:
      switch(LOWORD(wparam & 0xfff0)) // & to catch double-click on title bar maximize
      {
        case SC_CLOSE:
          if(emu)
          {
            if(emu->is_fullscreen())
              return 0;
          }
          break;

        case SC_MAXIMIZE:
          if(emu)
          {
            // if window is minimized, restore it first
            SendMessage(main_window_handle, WM_SYSCOMMAND, SC_RESTORE, 0);

            if(!emu->is_fullscreen())
              toggle_fullscreen();
          }
          return 0;
          break;

        case SC_MINIMIZE:
          if(!NESTER_settings.nes.preferences.run_in_background)
          {
            if(!g_minimized) freeze();
          }
          g_minimized = 1;
          // make the minimize happen
          freeze();
          DefWindowProc(hwnd, msg, wparam, lparam);
          thaw();
          return 0;
          break;

        case SC_RESTORE:
          // make the restore happen
          freeze();
          DefWindowProc(hwnd, msg, wparam, lparam);
          thaw();
          if(!NESTER_settings.nes.preferences.run_in_background)
          {
            if(g_minimized) thaw();
          }
          g_minimized = 0;
          return 0;
          break;
      }
      break;

    default:
      break;

  } // end switch

  // process any messages that we didn't take care of 
  return(DefWindowProc(hwnd, msg, wparam, lparam));

} // end WinProc

void EmuError(const char* error)
{
  char msg[1024];
  strcpy(msg, "Emulation error:\n");
  strcat(msg, error);
  strcat(msg, "\nFreeing ROM and halting emulation.");
  MessageBox(main_window_handle, msg, "Error", MB_OK);
  LOG(msg << endl);
  FreeROM();
}

void MainWinLoop(MSG& msg, HWND hwnd, HACCEL hAccel)
{
  while(1)
  {
    if(emu && !emu->frozen())
    {
      try {
        emu->do_frame();
      } catch(const char* s) {
        LOG("EXCEPTION: " << s << endl);
        EmuError(s);
      } catch(...) {
        LOG("Caught unknown exception in " << __FILE__ << endl);
        EmuError("unknown error");
      }

      while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
//        DUMP_WM_MESSAGE(1,msg.message);

        if(msg.message == WM_QUIT) return;

#ifdef TOOLTIP_HACK
        if(msg.message != 0x0118)
#endif
        if(!TranslateAccelerator(hwnd, hAccel, &msg))
        {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      }
    }
    else
    {
      if(GetMessage(&msg, NULL, 0, 0))
      {
//        DUMP_WM_MESSAGE(2,msg.message);

#ifdef TOOLTIP_HACK
        if(msg.message != 0x0118)
#endif
        if(!TranslateAccelerator(hwnd, hAccel, &msg))
        {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      }
      else
      {
        return;
      }
    }
  }
}

void InitControlsNStuff()
{
  INITCOMMONCONTROLSEX icce;

  memset((void*)&icce, (int)0, sizeof(icce));
  icce.dwSize = sizeof(icce);
  icce.dwICC = ICC_BAR_CLASSES;
//  InitCommonControlsEx(&icce);
  // win95 barfs on InitCommonControlsEx()
  InitCommonControls();
}

// WINMAIN ////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hinstance,
                   HINSTANCE hprevinstance,
                   LPSTR lpcmdline,
                   int ncmdshow)
{

  WNDCLASSEX  winclass; // window class we create
  HWND     hwnd;      // window handle
  MSG      msg;       // message

  HACCEL   hAccel;  // handle to keyboard accelerators

  try {
    NESTER_settings.Load();
  } catch(const char* IFDEBUG(s)) {
    LOG(s);
  } catch(...) {
  }

  // fill in the window class stucture
  winclass.cbSize     = sizeof(winclass);
  winclass.style      = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT;
  winclass.lpfnWndProc  = WindowProc;
  winclass.cbClsExtra   = 0;
  winclass.cbWndExtra   = 0;
  winclass.hInstance    = hinstance;
  winclass.hIcon        = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_NESTERICON));
  winclass.hCursor      = LoadCursor(NULL, IDC_ARROW);
  winclass.hbrBackground  = GetStockBrush(BLACK_BRUSH);
  winclass.lpszMenuName   = MAKEINTRESOURCE(IDR_MENU1);
  winclass.lpszClassName  = WINCLASS_NAME;
  winclass.hIconSm      = NULL;

  // register the window class
  if(!RegisterClassEx(&winclass))
    return(0);

  InitControlsNStuff();

  hAccel = LoadAccelerators(hinstance, MAKEINTRESOURCE(IDR_MAIN_ACCEL));

  // create the window
  if(!(hwnd = CreateWindowEx(0,
                WINCLASS_NAME, // class
                PROG_NAME,   // title
                STYLE_WINDOWED,
                CW_USEDEFAULT, // x
                CW_USEDEFAULT, // y
                0,  // width
                0, // height
                NULL,     // handle to parent
                NULL,     // handle to menu
                hinstance,// instance
                NULL)))  // creation parms
    return(0);

  // save the window handle and instance in globals
  main_window_handle = hwnd;
  g_main_instance    = hinstance;

#ifdef NESTER_DEBUG
  char new_titlebar[256];
  GetWindowText(main_window_handle, new_titlebar, sizeof(new_titlebar));
  strcat(new_titlebar, " - DEBUG");
  SetWindowText(main_window_handle, new_titlebar);
#endif

  assertWindowSize();
  CenterWindow();

  ShowWindow(hwnd, ncmdshow);
  UpdateWindow(hwnd);
  SetFocus(hwnd);

  try {
    LoadCmdLineROM(lpcmdline);

    // sit and spin
    MainWinLoop(msg, hwnd, hAccel);

    // shut down
    NESTER_settings.Save();

    if(emu) FreeROM();

    // make sure directx is shut down
    iDirectX::releaseAll();

  } catch(const char* IFDEBUG(s)) {
    LOG("EXCEPTION: " << s << endl);
  } catch(...) {
    LOG("Caught unknown exception in " << __FILE__ << endl);
  }

  return(msg.wParam);
}
