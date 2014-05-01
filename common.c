/* VESA package for emx/gcc --- Copyright (c) 1993 by Johannes Martin */
#include <sys/hw.h>
#include <stdlib.h>
#include <string.h>

#define INCL_VIO
#define INCL_MOU
#define INCL_DOSPROCESS
#define INCL_DOSFILEMGR

#define VESADLL

#include <os2emx.h>
#include <os2thunk.h>

#include "vgaports.h"
#include "vesa.h"
#include "vesadll.h"
#include "common.h"

int        activeindex    = 0;
VESASWITCH swfunc         = NULL;
VESAWORD   ScanlineLength = 0;
int        screenlocked   = 0;
HMOU       MouseHandle    = 0;

void Error(char *s)
{
  ULONG dummy;

  DosWrite(2, s, strlen(s), &dummy);
}

VESABOOL OS2VesaGetInfo(PVESAINFO pVesaInfo)
{
  pVesaInfo->Version     = 0x102;
  pVesaInfo->OEMName     = "OS/2 VESA Driver by Johannes Martin";
  pVesaInfo->Modes       = Modes;
  pVesaInfo->MemoryBanks = 0;
  return(TRUE);
}

VESABOOL OS2VesaGetModeInfo(VESAWORD Mode, PVESAMODEINFO pVesaModeInfo)
{
  int i;

  Mode &= 0x7fff;
  for (i = 0; Modes[i] != Mode; i++)
    if (Modes[i] == 0xFFFF)
      return(FALSE);
  *pVesaModeInfo = ModeInfos[i].Vesa;
  if ((i == activeindex) && OS2VesaLockScreen(0))
    {
      pVesaModeInfo->BytesPerScanline = ScanlineLength;
      pVesaModeInfo->WindowAStart     = GetPhysBuf();
      pVesaModeInfo->WindowBStart     = pVesaModeInfo->WindowAStart;
      OS2VesaUnlockScreen();
    }
  if (pVesaModeInfo->NumberOfBitsPerPixel == 15)
    pVesaModeInfo->NumberOfBitsPerPixel = 16;
  return(TRUE);
}

static void SetScanlineLength(void)
{
  if ((ScanlineLength <= 512) &&
      ((ModeInfos[activeindex].Vesa.NumberOfBitsPerPixel == 4) ||
       (ModeInfos[activeindex].Vesa.MemoryModelType == 5)))
    {
      _outp8(CRT_I, 0x17);
      _outp8(CRT_D, _inp8(CRT_D) | 0x40);
      _outp8(CRT_I, 0x13);
      _outp8(CRT_D, ScanlineLength >> 1);
    }
  else
    {
      _outp8(CRT_I, 0x17);
      _outp8(CRT_D, (_inp8(CRT_D) | 0x20) & ~0x40);
      _outp8(CRT_I, 0x14);
      _outp8(CRT_D, _inp8(CRT_D) | 0x40);
      _outp8(CRT_I, 0x13);
      _outp8(CRT_D, ScanlineLength >> 3);
    }
}

VESABOOL OS2VesaSetMode(VESAWORD Mode)
{
  USHORT status;
  int i;

  if (OS2VesaLockScreen(0))
    {
      OS2VesaSetWindow(0, 0);
      OS2VesaSetWindow(1, 0);
      OS2VesaUnlockScreen();
    }
  for (i = 0; Modes[i] != (Mode & 0x7fff); i++)
    if (Modes[i] == 0xFFFF)
      return(FALSE);
  status = 0x100;
  MouSetDevStatus(&status, MouseHandle);
  if (!SetMode(i))
    return(FALSE);
  activeindex = i;
  ScanlineLength = ModeInfos[i].Vesa.BytesPerScanline;
  if (OS2VesaLockScreen(0))
    {
      SetSpecial(i);
      if (ModeInfos[i].Vesa.ModeAttributes & 0x10)
        {
          if ((Mode & 0x8000) == 0)
            {
              PVOID Buffer   = GetPhysBuf();
              int   LPWindow = ModeInfos[i].Vesa.WindowSize * 1024 / ModeInfos[i].Vesa.BytesPerScanline;
              int   WinCount = ModeInfos[i].Vesa.Height / LPWindow
                            + (ModeInfos[i].Vesa.Height % LPWindow != 0);
              int   WinFact  = ModeInfos[i].Vesa.WindowSize / ModeInfos[i].Vesa.WindowGranularity;

              for (i = 0; i < WinCount; i++)
                {
                  OS2VesaSetWindow(0, i * WinFact);
                  memset(Buffer, 0, ModeInfos[activeindex].Vesa.WindowSize * 1024);
                }
            }
          OS2VesaSetWindow(0, 0);
          OS2VesaSetWindow(1, 0);
          SetScanlineLength();
        }
      OS2VesaUnlockScreen();
    }
  if (activeindex == 0)
    {
      status = 0;
      MouSetDevStatus(&status, MouseHandle);
    }
  return(TRUE);
}

VESABOOL OS2VesaGetMode(PVESAWORD Mode)
{
  *Mode = Modes[activeindex];
  return(TRUE);
}

VESABOOL OS2VesaSetScanlineLength(VESAWORD NumberOfPixels)
{
  ScanlineLength = NumberOfPixels * ModeInfos[activeindex].Vesa.NumberOfBitsPerPixel
                    / (8 * ModeInfos[activeindex].Vesa.NumberOfMemoryPlanes);
  if ((ScanlineLength <= 512) &&
      ((ModeInfos[activeindex].Vesa.NumberOfBitsPerPixel == 4) ||
       (ModeInfos[activeindex].Vesa.MemoryModelType == 5)))
    ScanlineLength = (ScanlineLength + 1) & 0xFFFE;
  else
    ScanlineLength = (ScanlineLength + 7) & 0xFFF8;
  if (OS2VesaLockScreen(0))
    {
      SetScanlineLength();
      OS2VesaUnlockScreen();
    }
  return(TRUE);
}

VESABOOL OS2VesaGetScanlineLength(PVESAWORD BytesPerScanline,
                                  PVESAWORD NumberOfPixels,
                                  PVESAWORD NumberOfScanlines)
{
  *BytesPerScanline  = ScanlineLength;
  *NumberOfPixels    = ScanlineLength * ModeInfos[activeindex].Vesa.NumberOfMemoryPlanes
                       * 8 / ModeInfos[activeindex].Vesa.NumberOfBitsPerPixel;
  *NumberOfScanlines = ModeInfos[activeindex].Vesa.Height;
  return(TRUE);
}

static VOID APIENTRY RedrawWaitThread(ULONG Param)
{
  static VESAWORD WindowAStat = 0;
  static VESAWORD WindowBStat = 0;

  USHORT notifytype;

  while (1)
    {
      VioSavRedrawWait(0, &notifytype, 0);
      screenlocked++;
      if (notifytype)
        {
          SetMode(activeindex);
          SetSpecial(activeindex);
          SetScanlineLength();
          OS2VesaSetWindow(0, WindowAStat);
          OS2VesaSetWindow(1, WindowBStat);
        }
      swfunc(notifytype);
      if (!notifytype)
        {
          OS2VesaGetWindow(0, &WindowAStat);
          OS2VesaGetWindow(1, &WindowBStat);
          SetMode(0);
          SetSpecial(0);
        }
      screenlocked--;
    }
}

VESABOOL OS2VesaSetSwitchFunc(VESASWITCH SwitchFunc)
{
  static TID tid = 0;

  if ((VioSavRedrawUndo(1, 1, 0) != 0) && (tid != 0))
    DosKillThread(tid);
  if ((swfunc = SwitchFunc) != NULL)
    return(DosCreateThread(&tid, RedrawWaitThread, 0, 0, 32 * 1024) == 0);
  else
    {
      tid = 0;
      return(TRUE);
    }
}

VESABOOL OS2VesaLockScreen(VESACHAR wait)
{
  UCHAR Success;

  if ((screenlocked) ||
      ((VioScrLock(wait, &Success, 0) == 0) && (wait || (Success == 0))))
    {
      screenlocked++;
      return(TRUE);
    }
  else
    return(FALSE);
}

VESABOOL OS2VesaUnlockScreen(void)
{
  if (screenlocked)
    if (--screenlocked)
      return(TRUE);
    else
      return(VioScrUnLock(0) == 0);
  else
    return(FALSE);
}

VESABOOL OS2VesaSetModified(void)
{
  return(TRUE);
}

VESABOOL OS2VesaSetPalette(VESAWORD   StartIndex,
                           VESAWORD   NumberOfEntries,
                           PCVESACHAR Palette,
                           VESABOOL   Wait)
{
  if ((NumberOfEntries >= 1) && (StartIndex + NumberOfEntries <= 256))
    {
      if (!OS2VesaLockScreen(0))
        return(FALSE);
      if (Wait)
        _wait01 (IS1_R, 0x08);
      _outp8 (PEL_IW, StartIndex);
      _outps8dac (PEL_D, Palette, 3 * NumberOfEntries);
      OS2VesaUnlockScreen();
      return(TRUE);
    }
  return(FALSE);
}

VESABOOL OS2VesaGetPalette(VESAWORD  StartIndex,
                           VESAWORD  NumberOfEntries,
                           PVESACHAR Palette)
{
  if ((NumberOfEntries >= 1) && (StartIndex + NumberOfEntries <= 256))
    {
      int i, j;

      if (!OS2VesaLockScreen(0))
        return(FALSE);
      /* select palette register */
      _outp8(PEL_IR, StartIndex);

      for(j = 0; j < NumberOfEntries; j++)
        {
          for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */
          *(Palette++) = (int) _inp8(PEL_D);
          for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */
          *(Palette++) = (int) _inp8(PEL_D);
          for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */
          *(Palette++) = (int) _inp8(PEL_D);
        }
      OS2VesaUnlockScreen();
      return(TRUE);
    }
  return(FALSE);
}

static ULONG LastCode = -1;

VESABOOL OS2VesaGetCharacter(PVESACHAR pchar)
{
  if (LastCode != -1)
    {
      *pchar   = LastCode;
      LastCode = -1;
    }
  else
    *pchar = _read_kbd(0, 1, 0);
  return(TRUE);
}

VESABOOL OS2VesaKeyboardHit(void)
{
  if (LastCode == -1)
    LastCode = _read_kbd(0, 0, 0);
  return(LastCode != -1);
}

VESABOOL OS2MouseOpen(void)
{
  if (MouseHandle)
    {
      USHORT mask = 0x7F;

      MouFlushQue(MouseHandle);
      MouSetEventMask(&mask, MouseHandle);
      return(TRUE);
    }
  return(FALSE);
}

VESABOOL OS2MouseClose(void)
{
  return(MouseHandle != 0);
}

VESABOOL OS2MouseShow(void)
{
  if (MouseHandle && !activeindex)
    {
      MouDrawPtr(MouseHandle);
      return(TRUE);
    }
  return(FALSE);
}

VESABOOL OS2MouseHide(void)
{
  if (MouseHandle && !activeindex)
    {
      NOPTRRECT area;

      area.col  = 0;
      area.row  = 0;
      area.cCol = 65535;
      area.cRow = 65535;
      MouRemovePtr(&area, MouseHandle);
      return(TRUE);
    }
  return(FALSE);
}

VESABOOL OS2MouseSetPosition(VESAWORD x, VESAWORD y)
{
  if (MouseHandle)
    {
      PTRLOC location;

      location.row = y;
      location.col = x;
      MouSetPtrPos(&location, MouseHandle);
      return(TRUE);
    }
  return(FALSE);
}

VESABOOL OS2MouseGetPosition(PVESAWORD x, PVESAWORD y)
{
  if (MouseHandle)
    {
      PTRLOC location;

      MouGetPtrPos(&location, MouseHandle);
      *x = location.col;
      *y = location.row;
      return(TRUE);
    }
  return(FALSE);
}

VESABOOL OS2MouseGetEvent(PVESAWORD x, PVESAWORD y, PVESAWORD status, VESABOOL wait)
{
  static VESAWORD sstatus = 0;

  if (MouseHandle)
    {
      MOUEVENTINFO event;
      USHORT uswait, rc;

      uswait = wait;
      rc     = MouReadEventQue(&event, &uswait, MouseHandle);
      if ((rc == 0) && (event.time != 0))
        {
          *x = event.col;
          *y = event.row;
          if (event.fs & 0x06)
            sstatus = 1;
          else
            sstatus = 0;
          if (event.fs & 0x18)
            sstatus |= 2;
          if (event.fs & 0x60)
            sstatus |= 4;
          *status = sstatus;
          return(TRUE);
        }
    }
  OS2MouseGetPosition(x, y);
  *status = sstatus;
  return(FALSE);
}

int _CRT_init(void);
void _CRT_term(void);

unsigned long _DLL_InitTerm(unsigned long modhandle, unsigned long flag)
{
  if (flag)
    {
      if (activeindex)
        OS2VesaSetMode(0x03);
      if (MouseHandle)
        MouClose(MouseHandle);
      ClearUp();
      _CRT_term();
      return(TRUE);
    }
  if (MouOpen(NULL, &MouseHandle) && MouOpen("POINTER$", &MouseHandle))
    MouseHandle = 0;
  return((_CRT_init() == 0) && Initialize());
}
