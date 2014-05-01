/* VESA package for emx/gcc --- Copyright (c) 1993 by Johannes Martin */
#include <string.h>
#include <sys/hw.h>

#define INCL_VIO
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_DOSMISC

#include <os2emx.h>
#include <os2thunk.h>

#include "vgaports.h"
#include "vesa.h"
#include "vesadll.h"
#include "common.h"

/* VGA indexes max counts */
#define CRT_C   24      /* 24 CRT Controller Registers */
#define ATT_C   21      /* 21 Attribute Controller Registers */
#define GRA_C   9       /* 9  Graphics Controller Registers */
#define SEQ_C   5       /* 5  Sequencer Registers */
#define MIS_C   1       /* 1  Misc Output Register */
#define EXT_C   11      /* 10 SVGA Extended Registers */
#define HIC_C   1       /* 1  Hicolor Register     */

/* VGA registers saving indexes */
#define CRT     0               /* CRT Controller Registers start */
#define ATT     CRT+CRT_C       /* Attribute Controller Registers start */
#define GRA     ATT+ATT_C       /* Graphics Controller Registers start */
#define SEQ     GRA+GRA_C       /* Sequencer Registers */
#define MIS     SEQ+SEQ_C       /* General Registers */
#define EXT     MIS+MIS_C       /* SVGA Extended Registers */
#define HIC     EXT+EXT_C       /* Hicolor Register */

#define SEG_SELECT 0x3CD

#define NUM_REGS   72

VESAWORD *Modes                     = NULL;
struct _ModeInfo *ModeInfos         = NULL;
VESACHAR (*ModeRegisters)[NUM_REGS] = NULL;

VESABOOL Initialize(void)
{
  static VESAWORD NoModes = 0xFFFF;

  VESAWORD count, i;
  CHAR     FileName[260];
  HFILE    File;
  ULONG    dummy;
  PVOID    mem;

  _portaccess(ATT_IW, IS1_R);
  Modes = &NoModes;
  if (DosSearchPath(3, "DPATH", "VESAMODE.DAT", FileName, sizeof(FileName)) ||
      DosOpen(FileName, &File, &dummy, 0, 0,
              OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
              OPEN_SHARE_DENYWRITE | OPEN_ACCESS_READONLY, NULL))
    {
      Error("VESA.DLL: Cannot find VESAMODE.DAT.");
      return(FALSE);
    }
  if (DosRead(File, &count, sizeof(count), &dummy) ||
      (dummy != sizeof(count)) || (count == 0))
    {
      DosClose(File);
      Error("VESA.DLL: Invalid VESAMODE.DAT.");
      return(FALSE);
    }
  if (DosAllocMem(&mem, (count + 1) * sizeof(VESAWORD) +
                        count * sizeof(struct _ModeInfo) +
                        count * NUM_REGS * sizeof(VESACHAR),
                  PAG_COMMIT | PAG_READ | PAG_WRITE))
    {
      DosClose(File);
      Error("VESA.DLL: Not enough memory.");
      return(FALSE);
    }
  ModeInfos     = mem;
  Modes         = mem + count * sizeof(struct _ModeInfo);
  ModeRegisters = mem + count * sizeof(struct _ModeInfo)
                      + (count + 1) * sizeof(VESAWORD);
  for (i = 0; i < count; i++)
    {
      if (DosRead(File, &Modes[i], sizeof(VESAWORD), &dummy)                    ||
          (dummy != sizeof(VESAWORD))                                           ||
          DosRead(File, &ModeInfos[i].Vesa, sizeof(VESAMODEINFO), &dummy)       ||
          (dummy != sizeof(VESAMODEINFO))                                       ||
          DosRead(File, &ModeRegisters[i], NUM_REGS * sizeof(VESACHAR), &dummy) ||
          (dummy != NUM_REGS * sizeof(VESACHAR)))
        {
          DosClose(File);
          DosFreeMem(mem);
          Modes = &NoModes;
          Error("VESA.DLL: Invalid VESAMODE.DAT.");
          return(FALSE);
        }
      ModeInfos[i].fbtype = 0;
      ModeInfos[i].Vesa.SetWindowFunc = OS2VesaSetWindow;
      ModeInfos[i].Vesa.GetWindowFunc = OS2VesaGetWindow;
    }
  DosClose(File);
  Modes[i] = 0xFFFF;
  DosSetMem(mem, (count + 1) * sizeof(VESAWORD) +
                 count * sizeof(struct _ModeInfo) +
                 count * NUM_REGS * sizeof(VESACHAR), PAG_READ);
  return(TRUE);
}

VOID ClearUp(void)
{
  DosFreeMem(ModeInfos);
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

  if (ModeInfos[mode].Vesa.ModeAttributes & 0x10)
    {
      ModeInfo.cb     = 12;
      ModeInfo.fbType = 3;
      ModeInfo.color  = 8;
      ModeInfo.col    = 40;
      ModeInfo.row    = 25;
      ModeInfo.hres   = 320;
      ModeInfo.vres   = 200;
    }
  else
    {
      ModeInfo.cb     = 8;
      ModeInfo.fbType = 1;
      ModeInfo.color  = 4;
      ModeInfo.col    = 80;
      ModeInfo.row    = 25;
    }
  return(VioSetMode(&ModeInfo, 0) == 0);
}

static void hicolor_set_regs(char regs[])
{
  /* (re)set hicolor */

  _inp8(0x3c8);
  _inp8(0x3c6);
  _inp8(0x3c6);
  _inp8(0x3c6);
  _inp8(0x3c6);
  _outp8(0x3c6, regs[HIC]);
  _inp8(0x3c8);
}

static void et4000_set_regs(char regs[])
{
  int i;

  /* write some ET4000 specific registers */
  _outp8(0x3c3, regs[EXT+7]);
  _outp8(0x3cd, regs[EXT+8]);

  /* write extended sequencer register */
  _outp8(SEQ_I, 7);
  _outp8(SEQ_D, regs[EXT+6]);

  /* write extended CRT registers */
  for (i = 0; i < 6; i++)
    {
      _outp8(CRT_I, 0x32+i);
      _outp8(CRT_D, regs[EXT+i]);
    }

  /* write extended attribute register */
  _inp8(IS1_R);    /* reset flip flop */
  _outp8(ATT_IW, 0x16);
  _outp8(ATT_IW, regs[EXT+9]);
  /* write another ET4000 CRT register */
  _outp8(CRT_I, 0x3f);
  _outp8(CRT_D, regs[EXT+10]);
}

static void set_regs(char regs[])
{
  int i;

  /* update misc output register */
  _outp8(MIS_W, regs[MIS]);

  /* synchronous reset on */
  _outp8(SEQ_I, 0x00);
  _outp8(SEQ_D, 0x01);

  /* write sequencer registers */
  for (i = 1; i < SEQ_C; i++)
    {
      _outp8(SEQ_I, i);
      _outp8(SEQ_D, regs[SEQ+i]);
    }

  /* synchronous reset off */
  _outp8(SEQ_I, 0x00);
  _outp8(SEQ_D, 0x03);

  /* deprotect CRT registers 0-7 */
  _outp8(CRT_I, 0x11);
  _outp8(CRT_D, _inp8(CRT_D)&0x7F);

  /* write CRT registers */
  for (i = 0; i < CRT_C; i++)
    {
      _outp8(CRT_I, i);
      _outp8(CRT_D, regs[CRT+i]);
    }

  /* write graphics controller registers */
  for (i = 0; i < GRA_C; i++)
    {
      _outp8(GRA_I, i);
      _outp8(GRA_D, regs[GRA+i]);
    }

  /* write attribute controller registers */
  for (i = 0; i < ATT_C; i++)
    {
      _inp8(IS1_R);   /* reset flip-flop */
      _outp8(ATT_IW, i);
      _outp8(ATT_IW, regs[ATT+i]);
    }
}

void SetSpecial(VESAWORD mode)
{
  /* disable video */
  _inp8(IS1_R);
  _outp8(ATT_IW, 0x00);

  /* unlock extended registers */
  _outp8(0x3bf, 3);
  _outp8(0x3d8, 0xa0);

  set_regs(ModeRegisters[mode]);
  et4000_set_regs(ModeRegisters[mode]);
  hicolor_set_regs(ModeRegisters[mode]);

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
