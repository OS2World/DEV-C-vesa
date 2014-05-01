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

#define STANDARD_MODES 4

static VESAWORD _Modes[] = { 0x003, 0x012, 0x013, 0x06A,
                             0x210, 0x211, 0x212, 0xFFFF };

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
               OS2VesaSetWindow, OS2VesaGetWindow, 100,
               800, 600, 8, 16, 4, 4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0x13, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 80,
               320, 240, 8,  8, 4, 8, 1, 5, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0x13, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 80,
               320, 400, 8,  8, 4, 8, 1, 5, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0x13, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 90,
               360, 480, 8,  8, 4, 8, 1, 5, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } }
    };

VESAWORD *Modes             = _Modes;
struct _ModeInfo *ModeInfos = _ModeInfos;

#define CRT_C   24      /* 24 CRT Controller Registers    */
#define CRT     0       /* CRT Controller Registers start */
#define SEQ     24      /* Sequencer Register 4 index     */
#define MIS     25      /* Misc.Output Register index     */

static char moderegs[3][60] =
    { /* Register: Index  0..23: CRT Controller Registers 0..23  */
      /*           Index     24: Sequencer Controller Register 4 */
      /*           Index     25: Misc Output Register            */
      /* non-BIOS mode - 320x240x256 */
      { 0x5F,0x4F,0x50,0x82,0x54,0x80,0x0D,0x3E,0x00,0x41,0x00,0x00,
        0x00,0x00,0x00,0x00,0xEA,0xAC,0xDF,0x28,0x00,0xE7,0x06,0xE3,
        0x06,
        0xE3 },
      /* non-BIOS mode - 320x400x256 */
      { 0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,0x00,0x40,0x00,0x00,
        0x00,0x00,0x00,0x00,0x9C,0x8E,0x8F,0x28,0x00,0x96,0xB9,0xE3,
        0x06,
        0x63 },
      /* non-BIOS mode - 360x480x256 */
      { 0x6B,0x59,0x5A,0x8E,0x5E,0x8A,0x0D,0x3E,0x00,0x40,0x00,0x00,
        0x00,0x00,0x00,0x00,0xEA,0xAC,0xDF,0x2D,0x00,0xE7,0x06,0xE3,
        0x06,
        0xE7 }
    };

VESABOOL Initialize(void)
{
  _portaccess(CRT_I, CRT_D);
  _portaccess(SEQ_I, SEQ_D);
  _portaccess(MIS_W, MIS_W);
  _portaccess(IS1_R, IS1_R);
  _portaccess(ATT_IW, ATT_IW);
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
    mode = 2;
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
  int i;

  if (mode < STANDARD_MODES)
    return;

  mode -= STANDARD_MODES;

  /* disable video */
  _inp8(IS1_R);
  _outp8(ATT_IW, 0x00);

  /* update misc output register */
  _outp8(MIS_W, moderegs[mode][MIS]);

  /* write sequencer registers 4 */
  _outp8(SEQ_I, 0x04);
  _outp8(SEQ_D, moderegs[mode][SEQ]);

  /* deprotect CRT registers 0-7 */
  _outp8(CRT_I, 0x11);
  _outp8(CRT_D, _inp8(CRT_D)&0x7F);

  /* write CRT registers */
  for (i = 0; i < CRT_C; i++)
    {
      _outp8(CRT_I, i);
      _outp8(CRT_D, moderegs[mode][CRT+i]);
    }

  /* enable video */
  _inp8(IS1_R);
  _outp8(ATT_IW, 0x20);
}

VESABOOL OS2VesaSetWindow(VESACHAR Window, VESAWORD Address)
{
  return(TRUE);
}

VESABOOL OS2VesaGetWindow(VESACHAR Window, PVESAWORD Address)
{
  *Address = 0;
  return(TRUE);
}
