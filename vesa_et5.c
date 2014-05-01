/* VESA package for emx/gcc --- Copyright (c) 1993 by Johannes Martin */
#include <string.h>
#include <sys/hw.h>

#define INCL_VIO
#define INCL_DOSPROCESS

#include <os2emx.h>
#include <os2thunk.h>

#include "vgaports.h"
#include "vesa.h"
#include "vesadll.h"
#include "common.h"

static VESAWORD _Modes[] = { 0x003, 0x012, 0x013, 0x101, 0x102, 0x103,
                             0x104, 0x105, 0x200, 0x201, 0x202, 0xFFFF };

static struct _ModeInfo _ModeInfos[] =
    { { 0x1, { 0x0f, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 160,
               640, 400, 8, 16, 1, 4, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0x3, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 80,
               640, 480, 8, 16, 4, 4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0x3, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 320,
               320, 200, 8,  8, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x5, 0x3, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 640,
               640, 480, 8, 16, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x5, 0x3, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 100,
               800, 600, 8, 16, 4, 4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x5, 0x3, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 800,
               800, 600, 8, 16, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x5, 0x3, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 128,
               1024, 768, 8, 16, 4, 4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x5, 0x3, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 1024,
               1024, 768, 8, 16, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xab, { 0x1b, 0x5, 0x3, 64, 64, 0x0, 0x0,
                OS2VesaSetWindow, OS2VesaGetWindow, 1024,
                512, 768, 8, 16, 1, 16, 1, 6, 0, 1, 5, 10, 5, 5, 5, 0, 1, 15 } },
      { 0xcb, { 0x1b, 0x5, 0x3, 64, 64, 0x0, 0x0,
                OS2VesaSetWindow, OS2VesaGetWindow, 1024,
                512, 768, 8, 16, 1, 16, 1, 6, 0, 1, 5, 11, 6, 5, 5, 0, 0, 0 } },
      { 0xeb, { 0x1b, 0x5, 0x3, 64, 64, 0x0, 0x0,
                OS2VesaSetWindow, OS2VesaGetWindow, 1024,
                341, 768, 8, 16, 1, 24, 1, 6, 0, 1, 8, 16, 8, 8, 8, 0, 0, 0 } }
    };

VESAWORD *Modes             = _Modes;
struct _ModeInfo *ModeInfos = _ModeInfos;

VESABOOL Initialize(void)
{
  _portaccess(CRT_I, CRT_D);
  _portaccess(SEG_SELECT, SEG_SELECT);
  _portaccess(IS1_R, IS1_R);
  _portaccess(ATT_IW, ATT_IW);
  _portaccess(0x3c6, 0x3c8);
  return(TRUE);
}

VOID ClearUp(void)
{
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

  if (ModeInfos[mode].fbtype & 0xf0)
    mode = 7;
  ModeInfo.cb     = 12;
  ModeInfo.fbType = ModeInfos[mode].fbtype;
  ModeInfo.color = ModeInfos[mode].Vesa.NumberOfBitsPerPixel;
  ModeInfo.col    = ModeInfos[mode].Vesa.Width  / ModeInfos[mode].Vesa.CharacterWidth;
  ModeInfo.row    = ModeInfos[mode].Vesa.Height / ModeInfos[mode].Vesa.CharacterHeight;
  ModeInfo.hres   = ModeInfos[mode].Vesa.Width;
  ModeInfo.vres   = ModeInfos[mode].Vesa.Height;
  return(VioSetMode(&ModeInfo, 0) == 0);
}

void SetSpecial(VESAWORD mode)
{
  /* disable video */
  _inp8(IS1_R);
  _outp8(ATT_IW, 0x00);

  /* (re)set hicolor */

  _inp8(0x3c8);
  _inp8(0x3c6);
  _inp8(0x3c6);
  _inp8(0x3c6);
  _inp8(0x3c6);
  _outp8(0x3c6, ModeInfos[mode].fbtype & 0xf0);
  _inp8(0x3c8);

  if (ModeInfos[mode].fbtype & 0xf0)
    {
      /* Word Mode, etc */

      _outp8(CRT_I, 0x14);
      _outp8(CRT_D, 0x60);

      _outp8(CRT_I, 0x17);
      _outp8(CRT_D, 0xab);
    }
  /* enable video */
  _inp8(IS1_R);
  _outp8(ATT_IW, 0x20);
}

VESABOOL OS2VesaSetWindow(VESACHAR Window, VESAWORD Address)
{
  if (Window == 0)
    _outp8(SEG_SELECT, (_inp8(SEG_SELECT) & 0xF0) | Address);
  else
    _outp8(SEG_SELECT, (_inp8(SEG_SELECT) & 0x0F) | (Address << 4));
  return(TRUE);
}

VESABOOL OS2VesaGetWindow(VESACHAR Window, PVESAWORD Address)
{
  if (Window == 0)
    *Address = _inp8(SEG_SELECT) & 0x0F;
  else
    *Address = _inp8(SEG_SELECT) >> 4;
  return(TRUE);
}
