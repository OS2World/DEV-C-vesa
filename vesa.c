#define INCL_DOSMODULEMGR
#define INCL_DOSMISC

#include <os2.h>
#include <stdlib.h>
#include <sys/hw.h>

#include "vesa.h"
#include "vesa_dos.h"
#include "vgaports.h"

static HMODULE hmod;

static VESABOOL dummy1(void)
{
  return(TRUE);
}

static VESABOOL dummy2(void)
{
  return(FALSE);
}

static void MyExit(void)
{
  DosFreeModule(hmod);
}

VESABOOL VesaInitialize(void)
{
  if (_osmode == OS2_MODE)
    {
      CHAR Error[100];
      PSZ  EnvVar;

      if (DosScanEnv("VESADLL", &EnvVar))
        EnvVar = "VESA_PM";
      if (DosLoadModule(Error, sizeof(Error), EnvVar, &hmod))
        return(FALSE);
      atexit(MyExit);
      if ((DosQueryProcAddr(hmod,  1, NULL, (PFN*) &VesaGetInfo)) ||
          (DosQueryProcAddr(hmod,  2, NULL, (PFN*) &VesaGetModeInfo)) ||
          (DosQueryProcAddr(hmod,  3, NULL, (PFN*) &VesaSetMode)) ||
          (DosQueryProcAddr(hmod,  4, NULL, (PFN*) &VesaGetMode)) ||
          (DosQueryProcAddr(hmod,  5, NULL, (PFN*) &VesaSetWindow)) ||
          (DosQueryProcAddr(hmod,  6, NULL, (PFN*) &VesaGetWindow)) ||
          (DosQueryProcAddr(hmod,  7, NULL, (PFN*) &VesaSetScanlineLength)) ||
          (DosQueryProcAddr(hmod,  8, NULL, (PFN*) &VesaGetScanlineLength)) ||
          (DosQueryProcAddr(hmod,  9, NULL, (PFN*) &VesaSetSwitchFunc)) ||
          (DosQueryProcAddr(hmod, 10, NULL, (PFN*) &VesaLockScreen)) ||
          (DosQueryProcAddr(hmod, 11, NULL, (PFN*) &VesaUnlockScreen)) ||
          (DosQueryProcAddr(hmod, 12, NULL, (PFN*) &VesaSetModified)) ||
          (DosQueryProcAddr(hmod, 13, NULL, (PFN*) &VesaSetPalette)) ||
          (DosQueryProcAddr(hmod, 14, NULL, (PFN*) &VesaGetPalette)) ||
          (DosQueryProcAddr(hmod, 15, NULL, (PFN*) &VesaGetCharacter)) ||
          (DosQueryProcAddr(hmod, 16, NULL, (PFN*) &VesaKeyboardHit)) ||
          (DosQueryProcAddr(hmod, 20, NULL, (PFN*) &MouseOpen)) ||
          (DosQueryProcAddr(hmod, 21, NULL, (PFN*) &MouseClose)) ||
          (DosQueryProcAddr(hmod, 22, NULL, (PFN*) &MouseShow)) ||
          (DosQueryProcAddr(hmod, 23, NULL, (PFN*) &MouseHide)) ||
          (DosQueryProcAddr(hmod, 24, NULL, (PFN*) &MouseSetPosition)) ||
          (DosQueryProcAddr(hmod, 25, NULL, (PFN*) &MouseGetPosition)) ||
          (DosQueryProcAddr(hmod, 26, NULL, (PFN*) &MouseGetEvent)))
        return(FALSE);
      else
        return(TRUE);
    }
  else
    {
      VESAINFO VesaInfo;

      _portaccess(PEL_IR, PEL_D);
      _portaccess(IS1_R, IS1_R);
      if (DosVesaGetInfo(&VesaInfo))
        {
          VesaGetInfo           = DosVesaGetInfo;
          VesaGetModeInfo       = DosVesaGetModeInfo;
          VesaSetMode           = DosVesaSetMode;
          VesaGetMode           = DosVesaGetMode;
          VesaSetWindow         = DosVesaSetWindow;
          VesaGetWindow         = DosVesaGetWindow;
          VesaSetScanlineLength = DosVesaSetScanlineLength;
          VesaGetScanlineLength = DosVesaGetScanlineLength;
        }
      else
        {
          VesaGetInfo           = DosNoVesaGetInfo;
          VesaGetModeInfo       = DosNoVesaGetModeInfo;
          VesaSetMode           = DosNoVesaSetMode;
          VesaGetMode           = DosNoVesaGetMode;
          VesaSetWindow         = DosNoVesaSetWindow;
          VesaGetWindow         = DosNoVesaGetWindow;
          VesaSetScanlineLength = DosNoVesaSetScanlineLength;
          VesaGetScanlineLength = DosNoVesaGetScanlineLength;
        }
      VesaSetSwitchFunc     = (void *) dummy1;
      VesaLockScreen        = (void *) dummy1;
      VesaUnlockScreen      = (void *) dummy1;
      VesaSetModified       = (void *) dummy1;
      VesaSetPalette        = DosVesaSetPalette;
      VesaGetPalette        = DosVesaGetPalette;
      VesaGetCharacter      = DosVesaGetCharacter;
      VesaKeyboardHit       = DosVesaKeyboardHit;
      if (DosMouseOpen())
        {
          MouseOpen        = DosMouseOpen;
          MouseClose       = DosMouseClose;
          MouseShow        = DosMouseShow;
          MouseHide        = DosMouseHide;
          MouseSetPosition = DosMouseSetPosition;
          MouseGetPosition = DosMouseGetPosition;
          MouseGetEvent    = DosMouseGetEvent;
        }
      else
        {
          MouseOpen        = (void *) dummy2;
          MouseClose       = (void *) dummy2;
          MouseShow        = (void *) dummy2;
          MouseHide        = (void *) dummy2;
          MouseSetPosition = (void *) dummy2;
          MouseGetPosition = (void *) dummy2;
          MouseGetEvent    = (void *) dummy2;
        }
    }
  return(TRUE);
}

static VESABOOL _VesaGetInfo(PVESAINFO pVesaInfo)
{
  return(VesaInitialize() && VesaGetInfo(pVesaInfo));
}

static VESABOOL _VesaGetModeInfo(VESAWORD Mode, PVESAMODEINFO pVesaModeInfo)
{
  return(VesaInitialize() && VesaGetModeInfo(Mode, pVesaModeInfo));
}

static VESABOOL _VesaSetMode(VESAWORD Mode)
{
  return(VesaInitialize() && VesaSetMode(Mode));
}

static VESABOOL _VesaGetMode(PVESAWORD Mode)
{
  return(VesaInitialize() && VesaGetMode(Mode));
}

static VESABOOL _VesaSetWindow(VESACHAR Window, VESAWORD Address)
{
  return(VesaInitialize() && VesaSetWindow(Window, Address));
}

static VESABOOL _VesaGetWindow(VESACHAR Window, PVESAWORD Address)
{
  return(VesaInitialize() && VesaGetWindow(Window, Address));
}

static VESABOOL _VesaSetScanlineLength(VESAWORD NumberOfPixels)
{
  return(VesaInitialize() && VesaSetScanlineLength(NumberOfPixels));
}

static VESABOOL _VesaGetScanlineLength(PVESAWORD BytesPerScanline,
                                       PVESAWORD NumberOfPixels,
                                       PVESAWORD NumberOfScanlines)
{
  return(VesaInitialize() && VesaGetScanlineLength(BytesPerScanline,
                             NumberOfPixels, NumberOfScanlines));
}

static VESABOOL _VesaSetSwitchFunc(VESASWITCH SwitchFunc)
{
  return(VesaInitialize() && VesaSetSwitchFunc(SwitchFunc));
}

static VESABOOL _VesaLockScreen (VESACHAR wait)
{
  return(VesaInitialize() && VesaLockScreen(wait));
}

static VESABOOL _VesaUnlockScreen (void)
{
  return(VesaInitialize() && VesaUnlockScreen());
}

static VESABOOL _VesaSetModified (void)
{
  return(VesaInitialize() && VesaSetModified());
}

static VESABOOL _VesaSetPalette(VESAWORD   StartIndex,
                                VESAWORD   NumberOfEntries,
                                PCVESACHAR Palette,
                                VESABOOL   Wait)
{
  return(VesaInitialize() && VesaSetPalette(StartIndex, NumberOfEntries,
                                            Palette, Wait));
}

static VESABOOL _VesaGetPalette(VESAWORD StartIndex,
                                VESAWORD NumberOfEntries,
                                PVESACHAR Palette)
{
  return(VesaInitialize() && VesaGetPalette(StartIndex, NumberOfEntries, Palette));
}

static VESABOOL _VesaGetCharacter(PVESACHAR pchar)
{
  return(VesaInitialize() && VesaGetCharacter(pchar));
}

static VESABOOL _VesaKeyboardHit(void)
{
  return(VesaInitialize() && VesaKeyboardHit());
}

static VESABOOL _MouseOpen(void)
{
  return(VesaInitialize() && MouseOpen());
}

static VESABOOL _MouseClose(void)
{
  return(VesaInitialize() && MouseClose());
}

static VESABOOL _MouseShow(void)
{
  return(VesaInitialize() && MouseShow());
}

static VESABOOL _MouseHide(void)
{
  return(VesaInitialize() && MouseHide());
}

static VESABOOL _MouseSetPosition(VESAWORD x, VESAWORD y)
{
  return(VesaInitialize() && MouseSetPosition(x, y));
}

static VESABOOL _MouseGetPosition(PVESAWORD x, PVESAWORD y)
{
  return(VesaInitialize() && MouseGetPosition(x, y));
}

static VESABOOL _MouseGetEvent(PVESAWORD x, PVESAWORD y, PVESAWORD status, VESABOOL wait)
{
  return(VesaInitialize() && MouseGetEvent(x, y, status, wait));
}

VESABOOL (*VesaGetInfo)(PVESAINFO pVesaInfo) = _VesaGetInfo;
VESABOOL (*VesaGetModeInfo)(VESAWORD Mode, PVESAMODEINFO pVesaModeInfo) = _VesaGetModeInfo;

VESABOOL (*VesaSetMode)(VESAWORD Mode)  = _VesaSetMode;
VESABOOL (*VesaGetMode)(PVESAWORD Mode) = _VesaGetMode;

VESABOOL (*VesaSetWindow)(VESACHAR Window, VESAWORD Address)  = _VesaSetWindow;
VESABOOL (*VesaGetWindow)(VESACHAR Window, PVESAWORD Address) = _VesaGetWindow;

VESABOOL (*VesaSetScanlineLength)(VESAWORD NumberOfPixels)     = _VesaSetScanlineLength;
VESABOOL (*VesaGetScanlineLength)(PVESAWORD BytesPerScanline,
                                  PVESAWORD NumberOfPixels,
                                  PVESAWORD NumberOfScanlines) = _VesaGetScanlineLength;

VESABOOL (*VesaSetSwitchFunc)(VESASWITCH SwitchFunc) = _VesaSetSwitchFunc;
VESABOOL (*VesaLockScreen)(VESACHAR wait)            = _VesaLockScreen;
VESABOOL (*VesaUnlockScreen)(void)                   = _VesaUnlockScreen;
VESABOOL (*VesaSetModified)(void)                    = _VesaSetModified;

VESABOOL (*VesaSetPalette)(VESAWORD   StartIndex,
                           VESAWORD   NumberOfEntries,
                           PCVESACHAR Palette,
                           VESABOOL   Wait)          = _VesaSetPalette;
VESABOOL (*VesaGetPalette)(VESAWORD StartIndex,
                           VESAWORD NumberOfEntries,
                           PVESACHAR Palette)        = _VesaGetPalette;

VESABOOL (*VesaGetCharacter)(PVESACHAR pchar)        = _VesaGetCharacter;
VESABOOL (*VesaKeyboardHit)(void)                    = _VesaKeyboardHit;

VESABOOL (*MouseOpen)(void)                          = _MouseOpen;
VESABOOL (*MouseClose)(void)                         = _MouseClose;
VESABOOL (*MouseShow)(void)                          = _MouseShow;
VESABOOL (*MouseHide)(void)                          = _MouseHide;
VESABOOL (*MouseSetPosition)(VESAWORD x, VESAWORD y) = _MouseSetPosition;
VESABOOL (*MouseGetPosition)(PVESAWORD x,
                             PVESAWORD y)            = _MouseGetPosition;
VESABOOL (*MouseGetEvent)(PVESAWORD x, PVESAWORD y,
                          PVESAWORD status,
                          VESABOOL wait)             = _MouseGetEvent;
