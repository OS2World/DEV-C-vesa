/* VESA package for emx/gcc --- Copyright (c) 1993 by Johannes Martin */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define INCL_WIN
#define INCL_DOSPROCESS
#define INCL_DOSFILEMGR
#define INCL_DOSQUEUES
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_DOSERRORS
#define INCL_DOSSESMGR

#include <os2emx.h>
#include <os2thunk.h>

#include "vesa.h"
#include "vesadll.h"
#include "common.h"
#include "vesa_pm.h"

static VESAWORD _Modes[] = { 0x003, 0x013, 0x100, 0x101, 0x103,
                             0x105, 0x107,
                             0x10F, 0x112, 0x115, 0x118, 0x11b,
                             0xFFFF };

static struct _ModeInfo _ModeInfos[] =
    { { 0x1, { 0x0f, 0x7, 0x0, 1024, 1024, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 160,
               640, 400, 8, 16, 1, 4, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0x3, { 0x1f, 0x7, 0x0, 1024, 1024, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 320,
               320, 200, 8,  8, 1, 8, 1, 0xf4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1f, 0x7, 0x0, 1024, 1024, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 640,
               640, 400, 8, 16, 1, 8, 1, 0xf4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1f, 0x7, 0x0, 1024, 1024, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 640,
               640, 480, 8, 16, 1, 8, 1, 0xf4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1f, 0x7, 0x0, 1024, 1024, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 800,
               800, 600, 8, 16, 1, 8, 1, 0xf4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1f, 0x7, 0x0, 1024, 1024, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 1024,
               1024, 768, 8, 16, 1, 8, 1, 0xf4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1f, 0x7, 0x0, 2048, 2048, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 1280,
               1280, 1024, 8, 16, 1, 8, 1, 0xf4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1f, 0x7, 0x0, 1024, 1024, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 960,
               320, 200, 8, 8, 1, 24, 1, 0xf6, 0, 1, 8, 16, 8, 8, 8, 0, 0, 0 } },
      { 0xb, { 0x1f, 0x7, 0x0, 1024, 1024, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 1920,
               640, 480, 8, 16, 1, 24, 1, 0xf6, 0, 1, 8, 16, 8, 8, 8, 0, 0, 0 } },
      { 0xb, { 0x1f, 0x7, 0x0, 2048, 2048, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 2400,
               800, 600, 8, 16, 1, 24, 1, 0xf6, 0, 1, 8, 16, 8, 8, 8, 0, 0, 0 } },
      { 0xb, { 0x1f, 0x7, 0x0, 4096, 4096, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 3072,
               1024, 768, 8, 16, 1, 24, 1, 0xf6, 0, 1, 8, 16, 8, 8, 8, 0, 0, 0 } },
      { 0xb, { 0x1f, 0x7, 0x0, 4096, 4096, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 3840,
               1280, 1024, 8, 16, 1, 24, 1, 0xf6, 0, 1, 8, 16, 8, 8, 8, 0, 0, 0 } }
    };

static VESAWORD *Modes             = _Modes;
static struct _ModeInfo *ModeInfos = _ModeInfos;

static DATABUFFER *DataBuffer;
static HPIPE      hpipe1;
static HPIPE      hpipe2;
static HPIPE      hpipe3;
static HPIPE      hpipe4;

static int activeindex   = 0;
static int screenlocked  = 0;

static VESACHAR palette[256][3]
   = { {  0,  0,  0 }, {  0,  0, 42 }, {  0, 42,  0 }, {  0, 42, 42 },
       { 42,  0,  0 }, { 42,  0, 42 }, { 42, 21,  0 }, { 42, 42, 42 },
       { 21, 21, 21 }, { 21, 21, 63 }, { 21, 63, 21 }, { 21, 63, 63 },
       { 63, 21, 21 }, { 63, 21, 63 }, { 63, 63, 21 }, { 63, 63, 63 },
       {  0,  0,  0 }, {  5,  5,  5 }, {  8,  8,  8 }, { 11, 11, 11 },
       { 14, 14, 14 }, { 17, 17, 17 }, { 20, 20, 20 }, { 24, 24, 24 },
       { 28, 28, 28 }, { 32, 32, 32 }, { 36, 36, 36 }, { 40, 40, 40 },
       { 45, 45, 45 }, { 50, 50, 50 }, { 56, 56, 56 }, { 63, 63, 63 },
       {  0,  0, 63 }, { 16,  0, 63 }, { 31,  0, 63 }, { 47,  0, 63 },
       { 63,  0, 63 }, { 63,  0, 47 }, { 63,  0, 31 }, { 63,  0, 16 },
       { 63,  0,  0 }, { 63, 16,  0 }, { 63, 31,  0 }, { 63, 47,  0 },
       { 63, 63,  0 }, { 47, 63,  0 }, { 31, 63,  0 }, { 16, 63,  0 },
       {  0, 63,  0 }, {  0, 63, 16 }, {  0, 63, 31 }, {  0, 63, 47 },
       {  0, 63, 63 }, {  0, 47, 63 }, {  0, 31, 63 }, {  0, 16, 63 },
       { 31, 31, 63 }, { 39, 31, 63 }, { 47, 31, 63 }, { 55, 31, 63 },
       { 63, 31, 63 }, { 63, 31, 55 }, { 63, 31, 47 }, { 63, 31, 39 },
       { 63, 31, 31 }, { 63, 39, 31 }, { 63, 47, 31 }, { 63, 55, 31 },
       { 63, 63, 31 }, { 55, 63, 31 }, { 47, 63, 31 }, { 39, 63, 31 },
       { 31, 63, 31 }, { 31, 63, 39 }, { 31, 63, 47 }, { 31, 63, 55 },
       { 31, 63, 63 }, { 31, 55, 63 }, { 31, 47, 63 }, { 31, 39, 63 },
       { 45, 45, 63 }, { 49, 45, 63 }, { 54, 45, 63 }, { 58, 45, 63 },
       { 63, 45, 63 }, { 63, 45, 58 }, { 63, 45, 54 }, { 63, 45, 49 },
       { 63, 45, 45 }, { 63, 49, 45 }, { 63, 54, 45 }, { 63, 58, 45 },
       { 63, 63, 45 }, { 58, 63, 45 }, { 54, 63, 45 }, { 49, 63, 45 },
       { 45, 63, 45 }, { 45, 63, 49 }, { 45, 63, 54 }, { 45, 63, 58 },
       { 45, 63, 63 }, { 45, 58, 63 }, { 45, 54, 63 }, { 45, 49, 63 },
       {  0,  0, 28 }, {  7,  0, 28 }, { 14,  0, 28 }, { 21,  0, 28 },
       { 28,  0, 28 }, { 28,  0, 21 }, { 28,  0, 14 }, { 28,  0,  7 },
       { 28,  0,  0 }, { 28,  7,  0 }, { 28, 14,  0 }, { 28, 21,  0 },
       { 28, 29,  0 }, { 21, 28,  0 }, { 14, 28,  0 }, {  7, 28,  0 },
       {  0, 28,  0 }, {  0, 28,  7 }, {  0, 28, 14 }, {  0, 28, 21 },
       {  0, 28, 28 }, {  0, 21, 28 }, {  0, 14, 28 }, {  0,  7, 28 },
       { 14, 14, 28 }, { 17, 14, 28 }, { 21, 14, 28 }, { 24, 14, 28 },
       { 28, 14, 28 }, { 28, 14, 24 }, { 28, 14, 21 }, { 28, 14, 17 },
       { 28, 14, 14 }, { 28, 17, 14 }, { 28, 21, 14 }, { 28, 24, 14 },
       { 28, 28, 14 }, { 24, 28, 14 }, { 21, 28, 14 }, { 17, 28, 14 },
       { 14, 28, 14 }, { 14, 28, 17 }, { 14, 28, 21 }, { 14, 28, 24 },
       { 14, 28, 28 }, { 14, 24, 28 }, { 14, 21, 28 }, { 14, 17, 28 },
       { 20, 20, 28 }, { 22, 20, 28 }, { 24, 20, 28 }, { 26, 20, 28 },
       { 28, 20, 28 }, { 28, 20, 26 }, { 28, 20, 24 }, { 28, 20, 22 },
       { 28, 20, 20 }, { 28, 22, 20 }, { 28, 24, 20 }, { 28, 26, 20 },
       { 28, 28, 20 }, { 26, 28, 20 }, { 24, 28, 20 }, { 22, 28, 20 },
       { 20, 28, 20 }, { 20, 28, 22 }, { 20, 28, 24 }, { 20, 28, 26 },
       { 20, 28, 28 }, { 20, 26, 28 }, { 20, 24, 28 }, { 20, 22, 28 },
       {  0,  0, 16 }, {  4,  0, 16 }, {  8,  0, 16 }, { 12,  0, 16 },
       { 16,  0, 16 }, { 16,  0, 12 }, { 16,  0,  8 }, { 16,  0,  4 },
       { 16,  0,  0 }, { 16,  4,  0 }, { 16,  8,  0 }, { 16, 12,  0 },
       { 16, 16,  0 }, { 12, 16,  0 }, {  8, 16,  0 }, {  4, 16,  0 },
       {  0, 16,  0 }, {  0, 16,  4 }, {  0, 16,  8 }, {  0, 16, 12 },
       {  0, 16, 16 }, {  0, 12, 16 }, {  0,  8, 16 }, {  0,  4, 16 },
       {  8,  8, 16 }, { 10,  8, 16 }, { 12,  8, 16 }, { 14,  8, 16 },
       { 16,  8, 16 }, { 16,  8, 14 }, { 16,  8, 12 }, { 16,  8, 10 },
       { 16,  8,  8 }, { 16, 10,  8 }, { 16, 12,  8 }, { 16, 14,  8 },
       { 16, 16,  8 }, { 14, 16,  8 }, { 12, 16,  8 }, { 10, 16,  8 },
       {  8, 16,  8 }, {  8, 16, 10 }, {  8, 16, 12 }, {  8, 16, 14 },
       {  8, 16, 16 }, {  8, 14, 16 }, {  8, 12, 16 }, {  8, 10, 16 },
       { 11, 11, 16 }, { 12, 11, 16 }, { 13, 11, 16 }, { 15, 11, 16 },
       { 16, 11, 16 }, { 16, 11, 15 }, { 16, 11, 13 }, { 16, 11, 12 },
       { 16, 11, 11 }, { 16, 12, 11 }, { 16, 13, 11 }, { 16, 15, 11 },
       { 16, 16, 11 }, { 15, 16, 11 }, { 13, 16, 11 }, { 12, 16, 11 },
       { 11, 16, 11 }, { 11, 16, 12 }, { 11, 16, 13 }, { 11, 16, 15 },
       { 11, 16, 16 }, { 11, 15, 16 }, { 11, 13, 16 }, { 11, 12, 16 },
       {  0,  0,  0 }, {  0,  0,  0 }, {  0,  0,  0 }, {  0,  0,  0 },
       {  0,  0,  0 }, {  0,  0,  0 }, {  0,  0,  0 }, {  0,  0,  0 } };

VESABOOL OS2VesaSetWindow(VESACHAR Window, VESAWORD Address)
{
  return(TRUE);
}

VESABOOL OS2VesaGetWindow(VESACHAR Window, PVESAWORD Address)
{
  *Address = 0;
  return(TRUE);
}

VESABOOL OS2VesaGetInfo(PVESAINFO pVesaInfo)
{
  pVesaInfo->Version = 0x102;
  pVesaInfo->OEMName = "OS/2 VESA Driver by Johannes Martin";
  pVesaInfo->Modes   = Modes;
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
  pVesaModeInfo->WindowAStart = DataBuffer->GraphBuffer;
  pVesaModeInfo->WindowBStart = DataBuffer->GraphBuffer;
  return(TRUE);
}

VESABOOL OS2VesaSetMode(VESAWORD Mode)
{
  ULONG count;

  DosResetEventSem(DataBuffer->hev, &count);
  WinPostMsg(DataBuffer->hwndGraph, WM_DONTUPDATE, NULL, NULL);
  for (activeindex = 0; Modes[activeindex] != (Mode & 0x7fff); activeindex++)
    if (Modes[activeindex] == 0xFFFF)
      {
        activeindex = 0;
        break;
      }
  DosWaitEventSem(DataBuffer->hev, SEM_INDEFINITE_WAIT);
  if (DataBuffer->GraphBuffer != NULL)
    DosFreeMem(DataBuffer->GraphBuffer);
  if (activeindex)
    DosAllocSharedMem(&DataBuffer->GraphBuffer, NULL,
                      ModeInfos[activeindex].Vesa.Width
                      * ModeInfos[activeindex].Vesa.Height
                      * ModeInfos[activeindex].Vesa.NumberOfBitsPerPixel / 8,
                      OBJ_GETTABLE | PAG_READ | PAG_WRITE | PAG_COMMIT);
  else
    DataBuffer->GraphBuffer = NULL;
  DataBuffer->pointl[2].x = 0;
  DataBuffer->pointl[2].y = 0;
  DataBuffer->pointl[3].x = ModeInfos[activeindex].Vesa.Width;
  DataBuffer->pointl[3].y = ModeInfos[activeindex].Vesa.Height;
  if ((DataBuffer->bits = ModeInfos[activeindex].Vesa.NumberOfBitsPerPixel) <= 8)
    OS2VesaSetPalette(0, 1 << ModeInfos[activeindex].Vesa.NumberOfBitsPerPixel,
                      (PVESACHAR) palette, 0);
  WinPostMsg(DataBuffer->hwndGraph, WM_NEWMODE, NULL, NULL);
  return(activeindex != 0);
}

VESABOOL OS2VesaGetMode(PVESAWORD Mode)
{
  *Mode = Modes[activeindex];
  return(TRUE);
}

VESABOOL OS2VesaSetScanlineLength(VESAWORD NumberOfPixels)
{
  return(NumberOfPixels == ModeInfos[activeindex].Vesa.BytesPerScanline * 8 /
                           ModeInfos[activeindex].Vesa.NumberOfBitsPerPixel);
}

VESABOOL OS2VesaGetScanlineLength(PVESAWORD BytesPerScanline,
                                  PVESAWORD NumberOfPixels,
                                  PVESAWORD NumberOfScanlines)
{
  *BytesPerScanline *= ModeInfos[activeindex].Vesa.BytesPerScanline;
  *NumberOfPixels    = ModeInfos[activeindex].Vesa.Width;
  *NumberOfScanlines = ModeInfos[activeindex].Vesa.Height;
  return(TRUE);
}

VESABOOL OS2VesaSetSwitchFunc(VESASWITCH SwitchFunc)
{
  return(TRUE);
}

VESABOOL OS2VesaLockScreen(VESACHAR wait)
{
  if (screenlocked == 0)
    {
      ULONG count;

      DosResetEventSem(DataBuffer->hev, &count);
      WinPostMsg(DataBuffer->hwndGraph, WM_DONTUPDATE, NULL, NULL);
      DosWaitEventSem(DataBuffer->hev, SEM_INDEFINITE_WAIT);
    }
  screenlocked++;
  return(TRUE);
}

VESABOOL OS2VesaUnlockScreen(void)
{
  if (screenlocked)
    {
      screenlocked--;
      if (screenlocked == 0)
        WinPostMsg(DataBuffer->hwndGraph, WM_UPDATE, NULL, NULL);
      return(TRUE);
    }
  else
    return(FALSE);
}

VESABOOL OS2VesaSetModified(void)
{
  DataBuffer->modified = TRUE;
  return(TRUE);
}

VESABOOL OS2VesaSetPalette(VESAWORD   StartIndex,
                           VESAWORD   NumberOfEntries,
                           PCVESACHAR Palette,
                           VESABOOL   Wait)
{
  if ((ModeInfos[activeindex].Vesa.NumberOfBitsPerPixel <= 8) &&
      (NumberOfEntries >= 1) &&
      (StartIndex + NumberOfEntries <= 1 << ModeInfos[activeindex].Vesa.NumberOfBitsPerPixel))
    {
      int   i;
      RGB2  *pal_entry;
      ULONG count;

      pal_entry = &DataBuffer->bmi.argbColor[StartIndex];
      for (i = 0; i < NumberOfEntries; i++)
        {
          pal_entry->bRed      = Palette[i * 3 + 0] << 2;
          pal_entry->bGreen    = Palette[i * 3 + 1] << 2;
          pal_entry->bBlue     = Palette[i * 3 + 2] << 2;
          pal_entry->fcOptions = 0;
          pal_entry++;
        }
      DosResetEventSem(DataBuffer->hev, &count);
      WinPostMsg(DataBuffer->hwndGraph, WM_CHGPALETTE, NULL, NULL);
      DosWaitEventSem(DataBuffer->hev, SEM_INDEFINITE_WAIT);
      return(TRUE);
    }
  return(FALSE);
}

VESABOOL OS2VesaGetPalette(VESAWORD  StartIndex,
                           VESAWORD  NumberOfEntries,
                           PVESACHAR Palette)
{
  if ((ModeInfos[activeindex].Vesa.NumberOfBitsPerPixel <= 8) &&
      (NumberOfEntries >= 1) &&
      (StartIndex + NumberOfEntries <= 1 << ModeInfos[activeindex].Vesa.NumberOfBitsPerPixel))
    {
      int i;
      RGB2 *pal_entry;

      pal_entry = &DataBuffer->bmi.argbColor[StartIndex];
      for (i = 0; i < NumberOfEntries; i++)
        {
          Palette[i * 3 + 0] = pal_entry->bRed   >> 2;
          Palette[i * 3 + 1] = pal_entry->bGreen >> 2;
          Palette[i * 3 + 2] = pal_entry->bBlue  >> 2;
          pal_entry++;
        }
      return(TRUE);
    }
  return(FALSE);
}

VESABOOL OS2VesaGetCharacter(PVESACHAR pchar)
{
  ULONG read;

  DosRead(hpipe2, pchar, sizeof(VESACHAR), &read);
  return(read == sizeof(VESACHAR));
}

VESABOOL OS2VesaKeyboardHit(void)
{
  VESACHAR  byte;
  ULONG     read, status;
  AVAILDATA avail;

  DosPeekNPipe(hpipe2, &byte, sizeof(VESACHAR), &read, &avail, &status);
  return(read == sizeof(VESACHAR));
}

VESABOOL OS2MouseOpen(void)
{
  return(TRUE);
}

VESABOOL OS2MouseClose(void)
{
  return(TRUE);
}

VESABOOL OS2MouseShow(void)
{
  return(TRUE);
}

VESABOOL OS2MouseHide(void)
{
  return(TRUE);
}

VESABOOL OS2MouseSetPosition(VESAWORD x, VESAWORD y)
{
  WinPostMsg(DataBuffer->hwndGraph, WM_SETMOUSEPOS, MPFROM2SHORT(x, y), NULL);
  return(FALSE);
}

VESABOOL OS2MouseGetPosition(PVESAWORD x, PVESAWORD y)
{
  *x = DataBuffer->mousex;
  *y = DataBuffer->mousey;
  return(TRUE);
}

VESABOOL OS2MouseGetEvent(PVESAWORD x, PVESAWORD y, PVESAWORD status, VESABOOL wait)
{
  static VESAWORD oldx = 0;
  static VESAWORD oldy = 0;
  static VESAWORD oldstatus = 0;
  VESABOOL rc;

  do
    {
      *x      = DataBuffer->mousex;
      *y      = DataBuffer->mousey;
      *status = DataBuffer->mousestatus;
      rc = (*x != oldx) || (*y != oldy) || (*status != oldstatus);
      if (wait && !rc)
        DosSleep(100);
    }
  while (wait && !rc);
  oldx      = *x;
  oldy      = *y;
  oldstatus = *status;
  return(rc);
}

int readdec(PSZ *s)
{
  int i;

  i = 0;
  while ((**s >= '0') && (**s <= '9'))
    {
      i = i * 10 + **s - '0';
      ++*s;
    }
  return(i);
}

int readhex(PSZ *s)
{
  int i;

  i = 0;
  while (TRUE)
    {
      if ((**s >= '0') && (**s <= '9'))
        i = i * 16 + **s - '0';
      else if ((**s >= 'A') && (**s <= 'F'))
        i = i * 16 + **s - 'A' + 10;
      else if ((**s >= 'a') && (**s <= 'f'))
        i = i * 16 + **s - 'a' + 10;
      else
        break;
      ++*s;
    }
  return(i);
}

int BuildModeTable(void)
{
  PSZ s;

  if (DosScanEnv("PMSCRMOD", &s) == 0)
    {
      int i;
      PCSZ d;

      i = 0;
      d = s;
      while (*d != '\0')
        if (*d++ == ';')
          i++;
      if (DosAllocMem((PPVOID) &Modes, (i + 3) * sizeof(VESAWORD) +
                                       (i + 2) * sizeof(struct _ModeInfo),
                                       PAG_READ | PAG_WRITE | PAG_COMMIT))
        return(FALSE);
      ModeInfos = (void *) &Modes[i + 3];
      Modes[0]  = _Modes[0];
      memcpy(&ModeInfos[0], &_ModeInfos[0], sizeof(struct _ModeInfo));
      i = 1;
      while (*s != '\0')
        {
          int x, y, c;

          Modes[i] = readhex(&s);
          if ((*s != ',') || (Modes[i] == 0x3) || (Modes[i] == 0xFFFF))
            return(FALSE);
          s++;
          x = readdec(&s);
          if ((*s != ',') || (x <= 0) || (x >= 32768))
            return(FALSE);
          s++;
          y = readdec(&s);
          if ((*s != ',') || (y <= 0) || (y >= 32768))
            return(FALSE);
          s++;
          c = readdec(&s);
          switch (c)
            {
              case 8:
                memcpy(&ModeInfos[i], &_ModeInfos[2], sizeof(struct _ModeInfo));
                break;
              case 24:
                memcpy(&ModeInfos[i], &_ModeInfos[7], sizeof(struct _ModeInfo));
                break;
              default:
                return(FALSE);
            }
          ModeInfos[i].Vesa.WindowGranularity = (x * y * c / 8 + 1023) / 1024;
          ModeInfos[i].Vesa.WindowSize        = ModeInfos[i].Vesa.WindowGranularity;
          ModeInfos[i].Vesa.BytesPerScanline  = x * c / 8;
          ModeInfos[i].Vesa.Width             = x;
          ModeInfos[i].Vesa.Height            = y;
          if ((*s != ';') && (*s != '\0'))
            return(FALSE);
          s++;
          i++;
        }
      Modes[i] = 0xFFFF;
    }
  return(TRUE);
}

int _CRT_init(void);
void _CRT_term(void);

unsigned long _DLL_InitTerm(unsigned long modhandle, unsigned long flag)
{
  ULONG count;

  if (!flag)
    {
      STARTDATA StartData;
      char  params[20], prgname[256], *c, *d;
      ULONG sid, rc;
      PID   pid;
      HFILE std;
      PPIB  ppib;
      PTIB  ptib;

      if (_CRT_init())
        return(FALSE);
      if (!BuildModeTable())
        {
          static char message[] = "Invalid mode description in environment variable PMSCRMOD.\r\n";
          ULONG written;

          DosWrite(2, message, strlen(message), &written);
          if (Modes != _Modes)
            DosFreeMem((PVOID) Modes);
          return(FALSE);
        }
      DosAllocSharedMem((PPVOID) &DataBuffer, NULL, sizeof(DATABUFFER),
                        OBJ_GETTABLE | PAG_READ | PAG_WRITE | PAG_COMMIT);
      memset(DataBuffer, 0, sizeof(DATABUFFER));
      DosCreateEventSem(NULL, &DataBuffer->hev, DC_SEM_SHARED, 0);
      DosGetInfoBlocks(&ptib, &ppib);
      strcpy(DataBuffer->Pipe1Name, "\\PIPE\\VESA_PM1.");
      _itoa(ppib->pib_ulpid, DataBuffer->Pipe1Name + 15, 16);
      strcpy(DataBuffer->Pipe2Name, DataBuffer->Pipe1Name);
      DataBuffer->Pipe2Name[14] = '2';
      strcpy(DataBuffer->Pipe3Name, DataBuffer->Pipe1Name);
      DataBuffer->Pipe3Name[14] = '3';
      strcpy(DataBuffer->Pipe4Name, DataBuffer->Pipe1Name);
      DataBuffer->Pipe4Name[14] = '4';
      c = ppib->pib_pchenv;
      while (*c != '\0')
        c += strlen(c) + 1;
      if ((d = strrchr(++c, '\\')) != NULL)
        c = d + 1;
      strncpy(DataBuffer->ProgName, c, sizeof(DataBuffer->ProgName) - 1);
      DataBuffer->ProgName[sizeof(DataBuffer->ProgName) - 1] = '\0';
      DosCreateNPipe(DataBuffer->Pipe1Name, &hpipe1, NP_ACCESS_INBOUND, 1, 0, 256, 0);
      DosCreateNPipe(DataBuffer->Pipe2Name, &hpipe2, NP_ACCESS_INBOUND, 1, 0, 256, 0);
      DosCreateNPipe(DataBuffer->Pipe3Name, &hpipe3, NP_ACCESS_OUTBOUND, 1, 256, 0, 0);
      DosCreateNPipe(DataBuffer->Pipe4Name, &hpipe4, NP_ACCESS_OUTBOUND, 1, 256, 0, 0);
      memset(&StartData, 0, sizeof(StartData));
      StartData.Length      = sizeof(StartData);
      StartData.Related     = 0;
      StartData.FgBg        = 0;
      StartData.TraceOpt    = 0;
      StartData.PgmTitle    = NULL;
      if (DosScanEnv("PMSCREEN", &StartData.PgmName))
        {
          if (DosSearchPath(SEARCH_IGNORENETERRS | SEARCH_ENVIRONMENT | SEARCH_CUR_DIRECTORY,
                            "PATH", "PMSCREEN.EXE", prgname, sizeof(prgname)))
            StartData.PgmName = "PMSCREEN.EXE";
          else
            StartData.PgmName = prgname;
        }
      StartData.PgmInputs   = _itoa((ULONG) DataBuffer, params, 10);
      StartData.TermQ       = NULL;
      StartData.Environment = NULL;
      StartData.InheritOpt  = 1;
      StartData.SessionType = SSF_TYPE_PM;
      StartData.IconFile    = 0;
      StartData.PgmHandle   = 0;
      if (((rc = DosStartSession(&StartData, &sid, &pid)) != 0) &&
          (rc != ERROR_SMG_START_IN_BACKGROUND))
        {
          static char message[] = " not found.\r\n";
          ULONG written;

          DosWrite(2, StartData.PgmName, strlen(StartData.PgmName), &written);
          DosWrite(2, message, strlen(message), &written);
          return(FALSE);
        }
      DosWaitEventSem(DataBuffer->hev, SEM_INDEFINITE_WAIT);
      DosConnectNPipe(hpipe1);
      DosConnectNPipe(hpipe2);
      DosConnectNPipe(hpipe3);
      DosConnectNPipe(hpipe4);
      std = 0;
      DosDupHandle(hpipe1, &std);
      std = 1;
      DosDupHandle(hpipe3, &std);
      std = 2;
      DosDupHandle(hpipe4, &std);
      OS2VesaSetMode(0);
      return(TRUE);
    }
  if (DataBuffer->hwndGraph != 0)
    {
      DosResetEventSem(DataBuffer->hev, &count);
      WinPostMsg(DataBuffer->hwndGraph, WM_FINISH, NULL, NULL);
      DosWaitEventSem(DataBuffer->hev, 10000);
    }
  DosCloseEventSem(DataBuffer->hev);
  DosDisConnectNPipe(hpipe1);
  DosDisConnectNPipe(hpipe2);
  DosDisConnectNPipe(hpipe3);
  DosDisConnectNPipe(hpipe4);
  DosClose(hpipe1);
  DosClose(hpipe2);
  DosClose(hpipe3);
  DosClose(hpipe4);
  if (DataBuffer->GraphBuffer != NULL)
    DosFreeMem(DataBuffer->GraphBuffer);
  DosFreeMem(DataBuffer);
  if (Modes != _Modes)
    DosFreeMem((PVOID) Modes);
  _CRT_term();
  return(TRUE);
}
