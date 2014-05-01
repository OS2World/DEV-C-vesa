/* VESA package for emx/gcc --- Copyright (c) 1993 by Johannes Martin */
#include <string.h>
#include <sys/hw.h>

#define INCL_VIO
#define INCL_DOSPROCESS
#define INCL_DOSFILEMGR
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL

#include <os2emx.h>
#include <os2thunk.h>

#include "vgaports.h"
#include "vesa.h"
#include "vesadll.h"
#include "common.h"

VESABOOL OS2VesaSetWindowDummy(VESACHAR Window, VESAWORD Address);
VESABOOL OS2VesaGetWindowDummy(VESACHAR Window, PVESAWORD Address);
VESABOOL OS2VesaSetWindowLinear(VESACHAR Window, VESAWORD Address);
VESABOOL OS2VesaGetWindowLinear(VESACHAR Window, PVESAWORD Address);
VESABOOL OS2VesaSetWindowPlanar(VESACHAR Window, VESAWORD Address);
VESABOOL OS2VesaGetWindowPlanar(VESACHAR Window, PVESAWORD Address);

static VESAWORD _Modes[] = { 0x003, 0x012, 0x013, 0x101, 0x102, 0x103,
                             0x104, 0x105, 0x106, 0x107, 0x110, 0x111,
                             0x112, 0x113, 0x114, 0x115, 0x116, 0x117,
                             0xFFFF };

static struct _ModeInfo _ModeInfos[] =
    { { 0x1, { 0x0f, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowDummy, OS2VesaGetWindowDummy, 160,
               640, 400, 8, 16, 1, 4, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0x3, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowDummy, OS2VesaGetWindowDummy, 80,
               640, 480, 8, 16, 4, 4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0x3, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowDummy, OS2VesaGetWindowDummy, 320,
               320, 200, 8,  8, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowLinear, OS2VesaGetWindowLinear, 640,
               640, 480, 8, 16, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowDummy, OS2VesaGetWindowDummy, 100,
               800, 600, 8, 16, 4, 4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowLinear, OS2VesaGetWindowLinear, 800,
               800, 600, 8, 16, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowPlanar, OS2VesaGetWindowPlanar, 128,
               1024, 768, 8, 16, 4, 4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowLinear, OS2VesaGetWindowLinear, 1024,
               1024, 768, 8, 16, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowPlanar, OS2VesaGetWindowPlanar, 160,
               1280, 1024, 8, 16, 4, 4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowLinear, OS2VesaGetWindowLinear, 1280,
               1280, 1024, 8, 16, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowLinear, OS2VesaGetWindowLinear, 1280,
               640, 480, 8, 16, 1, 15, 1, 6, 0, 1, 5, 10, 5, 5, 5, 0, 1, 15 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowLinear, OS2VesaGetWindowLinear, 1280,
               640, 480, 8, 16, 1, 16, 1, 6, 0, 1, 5, 11, 6, 5, 5, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowLinear, OS2VesaGetWindowLinear, 1920,
               640, 480, 8, 16, 1, 24, 1, 6, 0, 1, 8, 16, 8, 8, 8, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowLinear, OS2VesaGetWindowLinear, 1600,
               800, 600, 8, 16, 1, 15, 1, 6, 0, 1, 5, 10, 5, 5, 5, 0, 1, 15 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowLinear, OS2VesaGetWindowLinear, 1600,
               800, 600, 8, 16, 1, 16, 1, 6, 0, 1, 5, 11, 6, 5, 5, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowLinear, OS2VesaGetWindowLinear, 2400,
               800, 600, 8, 16, 1, 24, 1, 6, 0, 1, 8, 16, 8, 8, 8, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowLinear, OS2VesaGetWindowLinear, 2048,
               1024, 768, 8, 16, 1, 15, 1, 6, 0, 1, 5, 10, 5, 5, 5, 0, 1, 15 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindowLinear, OS2VesaGetWindowLinear, 2048,
               1024, 768, 8, 16, 1, 16, 1, 6, 0, 1, 5, 11, 6, 5, 5, 0, 0, 0 } }
    };

VESAWORD *Modes             = _Modes;
struct _ModeInfo *ModeInfos = _ModeInfos;

HFILE ScreenHandle = 0;

VESABOOL Initialize(void)
{
  ULONG action;

  return(DosOpen("SCREEN$", &ScreenHandle, &action, 0, 0,
                 OPEN_ACTION_OPEN_IF_EXISTS, OPEN_FLAGS_NOINHERIT |
                 OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, NULL) == 0);
  return(TRUE);
}

VOID ClearUp(void)
{
  if (ScreenHandle != 0)
    DosClose(ScreenHandle);
}

VOID *GetPhysBuf(void)
{
  VIOMODEINFO ModeInfo;
  VIOPHYSBUF  PhysBuf;

  ModeInfo.cb  = sizeof(ModeInfo);
  VioGetMode(&ModeInfo, 0);
  PhysBuf.pBuf = (PBYTE) ModeInfo.buf_addr;
  PhysBuf.cb   = 0x10000;
  if (VioGetPhysBuf(&PhysBuf, 0) != 0)
    return(NULL);
  else
    return(MAKEP(PhysBuf.asel[0], 0));
}

VESABOOL SetMode(VESAWORD mode)
{
  VIOMODEINFO ModeInfo;

  ModeInfo.cb     = 12;
  ModeInfo.fbType = ModeInfos[mode].fbtype;
  ModeInfo.color  = ModeInfos[mode].Vesa.NumberOfBitsPerPixel;
  ModeInfo.col    = ModeInfos[mode].Vesa.Width  / ModeInfos[mode].Vesa.CharacterWidth;
  ModeInfo.row    = ModeInfos[mode].Vesa.Height / ModeInfos[mode].Vesa.CharacterHeight;
  ModeInfo.hres   = ModeInfos[mode].Vesa.Width;
  ModeInfo.vres   = ModeInfos[mode].Vesa.Height;
  return(VioSetMode(&ModeInfo, 0) == 0);
}

void SetSpecial(VESAWORD mode)
{
}

VESABOOL OS2VesaSetWindowDummy(VESACHAR Window, VESAWORD Address)
{
  return(TRUE);
}

VESABOOL OS2VesaGetWindowDummy(VESACHAR Window, PVESAWORD Address)
{
  *Address = 0;
  return(TRUE);
}

VESABOOL OS2VesaSetWindowLinear(VESACHAR Window, VESAWORD Address)
{
  struct
    {
      ULONG  length;
      USHORT bank;
      USHORT modetype;
      USHORT bankmode;
    } parameter;
  ULONG datalen, parmlen;

  datalen = 0;
  parmlen = sizeof(parameter);
  parameter.length   = sizeof(parameter);
  parameter.bank     = Address;
  parameter.modetype = 2;
  parameter.bankmode = 0;
  if (DosDevIOCtl(ScreenHandle, 0x80, 1,
                  &parameter, parmlen, &parmlen, NULL, 0, &datalen))
    DosBeep(440, 500);
  datalen = 0;
  parmlen = sizeof(parameter);
  parameter.length   = sizeof(parameter);
  parameter.bank     = Address;
  parameter.modetype = 2;
  parameter.bankmode = 1;
  if (DosDevIOCtl(ScreenHandle, 0x80, 1,
                  &parameter, parmlen, &parmlen, NULL, 0, &datalen))
    DosBeep(880, 500);
  return(TRUE);
}

VESABOOL OS2VesaGetWindowLinear(VESACHAR Window, PVESAWORD Address)
{
  struct
    {
      ULONG  length;
      USHORT bank;
      USHORT modetype;
      USHORT bankmode;
    } parameter;
  ULONG datalen, parmlen;

  datalen = 0;
  parmlen = sizeof(parameter);
  parameter.length   = sizeof(parameter);
  parameter.modetype = 2;
  parameter.bankmode = 0;
  if (DosDevIOCtl(ScreenHandle, 0x80, 0,
                  &parameter, parmlen, &parmlen, NULL, 0, &datalen))
    DosBeep(220, 500);
  *Address = parameter.bank;
  return(TRUE);
}

VESABOOL OS2VesaSetWindowPlanar(VESACHAR Window, VESAWORD Address)
{
  struct
    {
      ULONG  length;
      USHORT bank;
      USHORT modetype;
      USHORT bankmode;
    } parameter;
  ULONG datalen, parmlen;

  datalen = 0;
  parmlen = sizeof(parameter);
  parameter.length   = sizeof(parameter);
  parameter.bank     = Address;
  parameter.modetype = 1;
  parameter.bankmode = 0;
  if (DosDevIOCtl(ScreenHandle, 0x80, 1,
                  &parameter, parmlen, &parmlen, NULL, 0, &datalen))
    DosBeep(440, 500);
  datalen = 0;
  parmlen = sizeof(parameter);
  parameter.length   = sizeof(parameter);
  parameter.bank     = Address;
  parameter.modetype = 1;
  parameter.bankmode = 1;
  if (DosDevIOCtl(ScreenHandle, 0x80, 1,
                  &parameter, parmlen, &parmlen, NULL, 0, &datalen))
    DosBeep(880, 500);
  return(TRUE);
}

VESABOOL OS2VesaGetWindowPlanar(VESACHAR Window, PVESAWORD Address)
{
  struct
    {
      ULONG  length;
      USHORT bank;
      USHORT modetype;
      USHORT bankmode;
    } parameter;
  ULONG datalen, parmlen;

  datalen = 0;
  parmlen = sizeof(parameter);
  parameter.length   = sizeof(parameter);
  parameter.modetype = 1;
  parameter.bankmode = 0;
  if (DosDevIOCtl(ScreenHandle, 0x80, 0,
                  &parameter, parmlen, &parmlen, NULL, 0, &datalen))
    DosBeep(220, 500);
  *Address = parameter.bank;
  return(TRUE);
}

VESABOOL OS2VesaSetWindow(VESACHAR Window, VESAWORD Address)
{
  return(_ModeInfos[activeindex].Vesa.SetWindowFunc(Window, Address));
}

VESABOOL OS2VesaGetWindow(VESACHAR Window, PVESAWORD Address)
{
  return(_ModeInfos[activeindex].Vesa.GetWindowFunc(Window, Address));
}
