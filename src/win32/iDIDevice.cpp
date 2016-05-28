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

#include "iDIDevice.h"
#include "debug.h"

iDIDevice::iDIDevice(HINSTANCE hInstance, GUID *guid)
{
  LPDIRECTINPUT lpDI;

  device8 = NULL;

  lpDI = iDirectX::getDirectInput(hInstance);
  if(!lpDI) throw "Error creating DirectInput interface in iDIDevice::iDIDevice";

  device8 = iDirectX::DI_CreateDevice8(lpDI, guid);
  if (!device8) {
    throw "Error creating DirectInputDevice";
  }
}

iDIDevice::~iDIDevice()
{
  device8->Release();
}

HRESULT iDIDevice::SetDataFormat(LPCDIDATAFORMAT lpdf)
{
  return device8->SetDataFormat(lpdf);
}

HRESULT iDIDevice::GetObjectInfo(LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow)
{
  return device8->GetObjectInfo(pdidoi, dwObj, dwHow);
}

HRESULT iDIDevice::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
{
  return device8->SetCooperativeLevel(hwnd, dwFlags);
}

HRESULT iDIDevice::Acquire()
{
  return device8->Acquire();
}

HRESULT iDIDevice::Unacquire()
{
  return device8->Unacquire();
}

HRESULT iDIDevice::Poll()
{
  return device8->Poll();
}

HRESULT iDIDevice::GetDeviceState(DWORD cbData, LPVOID lpvData)
{
  return device8->GetDeviceState(cbData, lpvData);
}

HRESULT iDIDevice::SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
  return device8->SetProperty(rguidProp, pdiph);
}

HRESULT iDIDevice::GetDeviceInfo(LPDIDEVICEINSTANCE pdidi)
{
  return device8->GetDeviceInfo(pdidi);
}
