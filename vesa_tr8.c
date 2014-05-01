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
                             0x104, 0x105, 0xFFFF };

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
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 640,
               640, 480, 8, 16, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 100,
               800, 600, 8, 16, 4, 4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 800,
               800, 600, 8, 16, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 128,
               1024, 768, 8, 16, 4, 4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 1024,
               1024, 768, 8, 16, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } }
    };

VESAWORD *Modes             = _Modes;
struct _ModeInfo *ModeInfos = _ModeInfos;

VESABOOL Initialize(void)
{
  _portaccess(CRT_I, CRT_D);
  _portaccess(SEG_SELECT, SEG_SELECT);
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
  _outp8(SEQ_I, 0x0B);
  _outp8(SEQ_D, 0x00);
  _inp8(SEQ_D);
}

VESABOOL OS2VesaSetWindow(VESACHAR Window, VESAWORD Address)
{
  _outp8(SEQ_I, 0x0B);
  _outp8(SEQ_D, 0);
  _inp8(SEQ_D);
  _outp8(SEQ_I, 0x0E);
  _outp8(SEQ_D, Address ^ 2);
  return(TRUE);
}

VESABOOL OS2VesaGetWindow(VESACHAR Window, PVESAWORD Address)
{
  _outp8(SEQ_I, 0x0E);
  *Address = _inp8(SEQ_D) & 0x0F;
  return(TRUE);
}
