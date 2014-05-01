/* VESA package for emx/gcc --- Copyright (c) 1993 by Johannes Martin */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/hw.h>

#define INCL_WIN
#define INCL_DOSQUEUES
#define INCL_DOSPROCESS
#define INCL_DOSFILEMGR
#define INCL_DOSSEMAPHORES
#define INCL_VIO
#define INCL_AVIO

#define PAL_COUNT 256
#define TEXT_COLS 80
#define TEXT_ROWS 25

#include <os2emx.h>
#include <os2thunk.h>

#include "vesa_pm.h"
#include "pmscreen.h"

typedef struct
  {
    BOOL   UsePalette;
    BOOL   UseFullPalette;
    BOOL   OriginalSize;
    USHORT Intervall;
    USHORT x, y;
  } PRFDATA;

static DATABUFFER *DataBuffer;
static HPIPE      hpipe1, hpipe2, hpipe3, hpipe4;
static BOOL       UsePalette;
static BOOL       UseFullPalette;
static BOOL       OriginalSize;
static BOOL       HasPalMan;
static USHORT     Intervall;
static USHORT     XChar, YChar;

BOOL ReadIniData(BOOL General)
{
  PRFDATA PrfData;
  ULONG   size;

  size = sizeof(PRFDATA);
  if (PrfQueryProfileData(HINI_PROFILE, "PMSCREEN",
                          General ? "GENERAL" : DataBuffer->ProgName,
                          &PrfData, &size) && (size == sizeof(PRFDATA)))
    {
      UsePalette     = PrfData.UsePalette;
      UseFullPalette = PrfData.UseFullPalette;
      OriginalSize   = PrfData.OriginalSize;
      Intervall      = PrfData.Intervall;
      XChar          = PrfData.x;
      YChar          = PrfData.y;
      return(TRUE);
    }
  return(FALSE);
}

MRESULT SettingsDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg)
    {
      case WM_INITDLG:
        {
          WinSendMsg(WinWindowFromID(hwnd, DID_XSIZE), EM_SETTEXTLIMIT, MPFROMSHORT(2), NULL);
          WinSendMsg(WinWindowFromID(hwnd, DID_YSIZE), EM_SETTEXTLIMIT, MPFROMSHORT(2), NULL);
          if (DataBuffer)
            {
              CHAR str[3];

              WinEnableWindow(WinWindowFromID(hwnd, DID_USEPAL), HasPalMan && (DataBuffer->bits <= 8));
              WinEnableWindow(WinWindowFromID(hwnd, DID_FULLPAL), HasPalMan && UsePalette);
              WinSetDlgItemShort(hwnd, DID_INTERVALL, Intervall, FALSE);
              WinCheckButton(hwnd, DID_USEPAL, UsePalette);
              WinCheckButton(hwnd, DID_FULLPAL, UseFullPalette);
              WinCheckButton(hwnd, DID_ORGSIZE, OriginalSize);
              _itoa(YChar, str, 10);
              WinSetDlgItemText(hwnd, DID_YSIZE, str);
              _itoa(XChar, str, 10);
              WinSetDlgItemText(hwnd, DID_XSIZE, str);
            }
          else
            {
              WinSetDlgItemShort(hwnd, DID_INTERVALL, 3000, FALSE);
              WinEnableWindow(WinWindowFromID(hwnd, DID_FULLPAL), FALSE);
              WinSetDlgItemText(hwnd, DID_YSIZE, "1");
              WinSetDlgItemText(hwnd, DID_XSIZE, "1");
              WinEnableWindow(WinWindowFromID(hwnd, DID_OK), FALSE);
              WinEnableWindow(WinWindowFromID(hwnd, DID_APPDEF), FALSE);
            }
          return(0);
        }
      case WM_CONTROL:
        switch(SHORT2FROMMP(mp1))
          {
            case BN_CLICKED:
              if (SHORT1FROMMP(mp1) == DID_USEPAL)
                {
                  ULONG State = !WinQueryButtonCheckstate(hwnd, DID_USEPAL);
                  WinCheckButton(hwnd, DID_USEPAL, State);
                  WinEnableWindow(WinWindowFromID(hwnd, DID_FULLPAL), State);
                }
              break;
          }
        break;
      case WM_COMMAND:
        switch (COMMANDMSG(&msg)->cmd)
          {
            case DID_OK:
              {
                CHAR str[3];

                WinQueryDlgItemShort(hwnd, DID_INTERVALL, &Intervall, FALSE);
                UsePalette     = WinQueryButtonCheckstate(hwnd, DID_USEPAL);
                UseFullPalette = WinQueryButtonCheckstate(hwnd, DID_FULLPAL);
                OriginalSize   = WinQueryButtonCheckstate(hwnd, DID_ORGSIZE);
                WinQueryDlgItemText(hwnd, DID_YSIZE, 3, str);
                YChar = atoi(str);
                WinQueryDlgItemText(hwnd, DID_XSIZE, 3, str);
                XChar = atoi(str);
                WinDismissDlg(hwnd, TRUE);
                return(0);
              }
            case DID_CANCEL:
              WinDismissDlg(hwnd, FALSE);
              return(0);
            case DID_APPDEF:
            case DID_SYSDEF:
              {
                PRFDATA PrfData;
                CHAR str[3];

                WinQueryDlgItemShort(hwnd, DID_INTERVALL, &PrfData.Intervall, FALSE);
                PrfData.UsePalette     = WinQueryButtonCheckstate(hwnd, DID_USEPAL);
                PrfData.UseFullPalette = WinQueryButtonCheckstate(hwnd, DID_FULLPAL);
                PrfData.OriginalSize   = WinQueryButtonCheckstate(hwnd, DID_ORGSIZE);
                WinQueryDlgItemText(hwnd, DID_YSIZE, 3, str);
                PrfData.y = atoi(str);
                WinQueryDlgItemText(hwnd, DID_XSIZE, 3, str);
                PrfData.x = atoi(str);
                PrfWriteProfileData(HINI_PROFILE, "PMSCREEN",
                                    COMMANDMSG(&msg)->cmd == DID_SYSDEF ? "GENERAL" : DataBuffer->ProgName,
                                    &PrfData, sizeof(PRFDATA));
              }
              if (!DataBuffer)
                WinDismissDlg(hwnd, TRUE);
              return(0);
          }
        break;
    }
  return(WinDefDlgProc(hwnd, msg, mp1, mp2));
}

void WriteChar(USHORT usCode)
{
  ULONG written;

  if ((usCode & 0x00FF) == 0)
    {
      DosWrite(hpipe2, &usCode, sizeof(CHAR), &written);
      usCode >>= 8;
      DosWrite(hpipe2, &usCode, sizeof(CHAR), &written);
    }
  else
    {
      DosWrite(hpipe1, &usCode, sizeof(CHAR), &written);
      DosWrite(hpipe2, &usCode, sizeof(CHAR), &written);
    }
}

void InstallMenu(HWND hwnd)
{
  CHAR *szMenuText[3] = { NULL, "~Update screen", "~PM-Screen settings" };
  MENUITEM mi[3]      = {{ MIT_END, MIS_SEPARATOR, 0, 0, (HWND) NULL, (ULONG) NULL },
                         { MIT_END, MIS_TEXT, 0, IDM_UPDATE, (HWND) NULL, (ULONG) NULL },
                         { MIT_END, MIS_TEXT, 0, IDM_SETTINGS, (HWND) NULL, (ULONG) NULL }};
  HWND     hwndSysMenu, hwndSysSubMenu;
  MENUITEM miSysMenu;
  SHORT    sItem, idSysMenu;

  hwndSysMenu = WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT), FID_SYSMENU);
  idSysMenu   = SHORT1FROMMR(WinSendMsg(hwndSysMenu, MM_ITEMIDFROMPOSITION, NULL, NULL));
  WinSendMsg(hwndSysMenu, MM_QUERYITEM,  MPFROM2SHORT(idSysMenu, FALSE), MPFROMP(&miSysMenu));
  hwndSysSubMenu = miSysMenu.hwndSubMenu;
  for (sItem = 0; sItem < 3; sItem++)
     WinSendMsg(hwndSysSubMenu, MM_INSERTITEM, MPFROMP(mi+sItem), MPFROMP(szMenuText[sItem]));
}

BOOL TestPalMan(HDC hdc)
{
  ULONG caps;

  DevQueryCaps(hdc, CAPS_ADDITIONAL_GRAPHICS, 1, &caps);
  return((caps & CAPS_PALETTE_MANAGER) != 0);
}

VOID MakePalette(HWND hwnd, HPS hps, HPAL *hPal)
{
  ULONG i, color;
  ULONG alTable[PAL_COUNT];

  for (i = 0; i < PAL_COUNT; i++)
    alTable[i] = PC_RESERVED * 16777216
               + DataBuffer->bmi.argbColor[i].bRed * 65536
               + DataBuffer->bmi.argbColor[i].bGreen * 256
               + DataBuffer->bmi.argbColor[i].bBlue;

  *hPal = GpiCreatePalette(WinQueryAnchorBlock(hwnd),
                           LCOL_PURECOLOR | (UseFullPalette ? LCOL_OVERRIDE_DEFAULT_COLORS : 0),
                           LCOLF_CONSECRGB, PAL_COUNT, alTable);
  GpiSelectPalette(hps, *hPal);
  WinRealizePalette(hwnd, hps, &color);
}

VOID RemovePalette(HPS hps, HPAL *hPal)
{
  GpiSelectPalette(hps, 0);
  GpiDeletePalette(*hPal);
  *hPal = 0;
}

MRESULT GraphWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static BOOL do_update;
  static HMTX hmtxPS;
  static HPS  hpsBuffer;
  static HPAL hPal;
  static HDC  hdc;

  switch (msg)
    {
      case WM_CREATE:
        {
          SIZEL sizl = { 0L, 0L };
          HAB   hab;

          DosCreateMutexSem(NULL, &hmtxPS, 0, FALSE);
          hab       = WinQueryAnchorBlock(hwnd);
          hdc       = WinOpenWindowDC(hwnd);
          hpsBuffer = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIF_DEFAULT |
                                  GPIT_MICRO | GPIA_ASSOC );
          DataBuffer->hwndGraph = hwnd;
          DataBuffer->modified  = TRUE;
          HasPalMan             = TestPalMan(hdc);
          if (!ReadIniData(FALSE) && !ReadIniData(TRUE))
            {
              UsePalette            = FALSE;
              UseFullPalette        = FALSE;
              OriginalSize          = FALSE;
              Intervall             = 3000;
              XChar                 = 1;
              YChar                 = 1;
            }
          do_update             = TRUE;
          hPal                  = 0;
          InstallMenu(hwnd);
          WinStartTimer(hab, hwnd, ID_TIMER, Intervall);
          WinPostMsg(DataBuffer->hwndText, WM_FONTSIZECHANGED, NULL, NULL);
          break;
        }
      case WM_CHAR:
        {
          if (CHARMSG(&msg)->fs & KC_KEYUP)
            return(0);
          if (CHARMSG(&msg)->fs & KC_CHAR)
            {
              WriteChar(CHARMSG(&msg)->chr);
              return(0);
            }
          if (CHARMSG(&msg)->fs & KC_VIRTUALKEY)
            {
              static USHORT keytable[][5] = {
                /* VK-Keyword  normal- shifted- ctrl-  alt- Code */
                { VK_ESC,      0x001b, 0x001b, 0x001b, 0x001b },
                { VK_BACKTAB,  0x0f00, 0x0f00, 0x0f00, 0x0f00 },
                { VK_PAGEUP,   0x4900, 0x0039, 0x8400, 0x9900 },
                { VK_PAGEDOWN, 0x5100, 0x0033, 0x7600, 0xa100 },
                { VK_END,      0x4f00, 0x0031, 0x7500, 0x9f00 },
                { VK_HOME,     0x4700, 0x0037, 0x7700, 0x9700 },
                { VK_LEFT,     0x4b00, 0x0034, 0x7300, 0x9b00 },
                { VK_UP,       0x4800, 0x0038, 0x8D00, 0x9800 },
                { VK_RIGHT,    0x4d00, 0x0036, 0x7400, 0x9d00 },
                { VK_DOWN,     0x5000, 0x0032, 0x9100, 0xa000 },
                { VK_INSERT,   0x5200, 0x0030, 0x9200, 0xa200 },
                { VK_DELETE,   0x5300, 0x002e, 0x9300, 0xa300 },
                { VK_F1,       0x3b00, 0x5400, 0x5e00, 0x6800 },
                { VK_F2,       0x3c00, 0x5500, 0x5f00, 0x6900 },
                { VK_F3,       0x3d00, 0x5600, 0x6000, 0x6a00 },
                { VK_F4,       0x3e00, 0x5700, 0x6100, 0x6b00 },
                { VK_F5,       0x3f00, 0x5800, 0x6200, 0x6c00 },
                { VK_F6,       0x4000, 0x5900, 0x6300, 0x6d00 },
                { VK_F7,       0x4100, 0x5a00, 0x6400, 0x6e00 },
                { VK_F8,       0x4200, 0x5b00, 0x6500, 0x6f00 },
                { VK_F9,       0x4300, 0x5c00, 0x6600, 0x7000 },
                { VK_F10,      0x4400, 0x5d00, 0x6700, 0x7100 },
                { VK_F11,      0x8500, 0x8700, 0x8900, 0x8b00 },
                { VK_F12,      0x8600, 0x8800, 0x8a00, 0x8c00 },
                { 0,           0,      0,      0,      0      } };
              int i = 0;

              while (keytable[i][0] != 0)
                {
                  if (keytable[i][0] == CHARMSG(&msg)->vkey)
                    {
                      if (CHARMSG(&msg)->fs & KC_ALT)
                        WriteChar(keytable[i][4]);
                      else if (CHARMSG(&msg)->fs & KC_CTRL)
                        WriteChar(keytable[i][3]);
                      else if (CHARMSG(&msg)->fs & KC_SHIFT)
                        WriteChar(keytable[i][2]);
                      else
                        WriteChar(keytable[i][1]);
                      return(0);
                    }
                  i++;
                }
              return(0);
            }
          if ((CHARMSG(&msg)->fs & KC_CTRL) && (CHARMSG(&msg)->chr != 0))
            {
              WriteChar(CHARMSG(&msg)->chr & 0x1f);
              return(0);
            }
          if ((CHARMSG(&msg)->fs & KC_SCANCODE) &&
              (CHARMSG(&msg)->fs & KC_ALT))
            {
              WriteChar(CHARMSG(&msg)->scancode << 8);
              return(0);
            }
          return(0);
        }
      case WM_COMMAND:
        switch (COMMANDMSG(&msg) -> cmd)
          {
            case IDM_SETTINGS:
              {
                static BOOL inDlg = 0;
                ULONG rc;

                if (inDlg)
                  return(0);
                inDlg = 1;
                rc = WinDlgBox(HWND_DESKTOP, hwnd, SettingsDlgProc, 0, ID_SETTINGS, NULL);
                inDlg = 0;
                if (rc)
                  {
                    SWP swp;

                    WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, ID_TIMER);
                    WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, ID_TIMER, Intervall);
                    if (hPal)
                      RemovePalette(hpsBuffer, &hPal);

                    WinQueryWindowPos(WinQueryWindow(hwnd, QW_PARENT), &swp);
                    if (((swp.fl & SWP_MINIMIZE) == 0) && OriginalSize &&
                        ((DataBuffer->pointl[1].y != DataBuffer->pointl[3].y - 1) ||
                         (DataBuffer->pointl[1].x != DataBuffer->pointl[3].x - 1)))
                      {
                        RECTL rectl;

                        rectl.xLeft   = 0;
                        rectl.yBottom = 0;
                        rectl.xRight  = DataBuffer->pointl[3].x;
                        rectl.yTop    = DataBuffer->pointl[3].y;
                        WinCalcFrameRect(WinQueryWindow(hwnd, QW_PARENT), &rectl, FALSE);
                        WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT), 0, 0, 0,
                                        rectl.xRight - rectl.xLeft - 1,
                                        rectl.yTop   - rectl.yBottom - 1, SWP_SIZE);
                      }
                    WinPostMsg(DataBuffer->hwndText, WM_FONTSIZECHANGED, NULL, NULL);
                  }
                else
                  return(0);
              }
            case IDM_UPDATE:
              WinInvalidateRect(hwnd, NULL, FALSE);
              return(0);
          }
        break;
      case WM_DONTUPDATE:
        do_update = FALSE;
        DosPostEventSem(DataBuffer->hev);
        return(0);
      case WM_NEWMODE:
        DataBuffer->bmi.cbFix           = 64;
        DataBuffer->bmi.cx              = DataBuffer->pointl[3].x;
        DataBuffer->bmi.cy              = DataBuffer->pointl[3].y;
        DataBuffer->bmi.cPlanes         = 1;
        DataBuffer->bmi.cBitCount       = DataBuffer->bits;
        DataBuffer->bmi.ulCompression   = BCA_UNCOMP;
        DataBuffer->bmi.cbImage         = 0;
        DataBuffer->bmi.cxResolution    = 2000;
        DataBuffer->bmi.cyResolution    = 2000;
        DataBuffer->bmi.cclrUsed        = 0;
        DataBuffer->bmi.cclrImportant   = 0;
        DataBuffer->bmi.usUnits         = BRU_METRIC;
        DataBuffer->bmi.usReserved      = 0;
        DataBuffer->bmi.usRecording     = BRA_BOTTOMUP;
        DataBuffer->bmi.usRendering     = BRH_NOTHALFTONED;
        DataBuffer->bmi.cSize1          = 0;
        DataBuffer->bmi.cSize2          = 0;
        DataBuffer->bmi.ulColorEncoding = BCE_RGB;
        DataBuffer->bmi.ulIdentifier    = 0;
        if (DataBuffer->GraphBuffer != 0)
          {
            HWND temp1, temp2;

            DosGetSharedMem(DataBuffer->GraphBuffer, PAG_READ);
            WinSetWindowPos(temp1 = WinQueryWindow(DataBuffer->hwndText, QW_PARENT),
                            0, 0, 0, 0, 0, SWP_SHOW | SWP_MINIMIZE);
            WinSetWindowPos(temp2 = WinQueryWindow(DataBuffer->hwndGraph, QW_PARENT),
                            0, 0, 0, 0, 0, SWP_SHOW | SWP_RESTORE);
            if (WinQueryActiveWindow(HWND_DESKTOP) == temp1)
              WinSetActiveWindow(HWND_DESKTOP, temp2);
          }
        else
          {
            HWND temp1, temp2;

            WinSetWindowPos(temp1 = WinQueryWindow(DataBuffer->hwndGraph, QW_PARENT),
                            0, 0, 0, 0, 0, SWP_SHOW | SWP_MINIMIZE);
            WinSetWindowPos(temp2 = WinQueryWindow(DataBuffer->hwndText, QW_PARENT),
                            0, 0, 0, 0, 0, SWP_SHOW | SWP_RESTORE);
            if (WinQueryActiveWindow(HWND_DESKTOP) == temp1)
              WinSetActiveWindow(HWND_DESKTOP, temp2);
          }
        if (DataBuffer->bits > 8)
          UsePalette = FALSE;
        DataBuffer->mousex      = 0;
        DataBuffer->mousey      = 0;
        DataBuffer->mousestatus = 0;
      case WM_UPDATE:
        do_update = TRUE;
        DataBuffer->modified = TRUE;
        WinInvalidateRect(hwnd, NULL, FALSE);
        return(0);
      case WM_CHGPALETTE:
        if (hPal)
          {
            ULONG i;
            ULONG alTable[PAL_COUNT];

            for (i = 0; i < PAL_COUNT; i++)
              alTable[i] = PC_RESERVED * 16777216
                         + DataBuffer->bmi.argbColor[i].bRed * 65536
                         + DataBuffer->bmi.argbColor[i].bGreen * 256
                         + DataBuffer->bmi.argbColor[i].bBlue;
            WinRequestMutexSem(hmtxPS, SEM_INDEFINITE_WAIT);
            GpiAnimatePalette(hPal, LCOLF_CONSECRGB, 0, PAL_COUNT, alTable);
            DosReleaseMutexSem(hmtxPS);
          }
        else
          DataBuffer->modified = TRUE;
        DosPostEventSem(DataBuffer->hev);
        return(0);
      case WM_SIZE:
        {
          SWP swp;

          WinQueryWindowPos(WinQueryWindow(hwnd, QW_PARENT), &swp);
          DataBuffer->pointl[1].x = SHORT1FROMMP(mp2);
          DataBuffer->pointl[1].y = SHORT2FROMMP(mp2);
          if (((swp.fl & SWP_MINIMIZE) == 0) && OriginalSize &&
              ((DataBuffer->pointl[1].y != DataBuffer->pointl[3].y - 1) ||
               (DataBuffer->pointl[1].x != DataBuffer->pointl[3].x - 1)))
            {
              RECTL rectl;

              rectl.xLeft   = 0;
              rectl.yBottom = 0;
              rectl.xRight  = DataBuffer->pointl[3].x;
              rectl.yTop    = DataBuffer->pointl[3].y;
              WinCalcFrameRect(WinQueryWindow(hwnd, QW_PARENT), &rectl, FALSE);
              WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT), 0, swp.x,
                              swp.y + swp.cy - rectl.yTop + rectl.yBottom + 1,
                              rectl.xRight - rectl.xLeft - 1,
                              rectl.yTop   - rectl.yBottom - 1, SWP_SIZE | SWP_MOVE);
            }
          else
            WinInvalidateRect(hwnd, NULL, FALSE);
        }
        break;
      case WM_TIMER:
        if (do_update && DataBuffer->modified)
          WinInvalidateRect(hwnd, NULL, FALSE);
        break;
      case WM_PAINT:
        {
          HPS hps;

          WinRequestMutexSem(hmtxPS, SEM_INDEFINITE_WAIT);
          if (UsePalette && !hPal)
            MakePalette(hwnd, hpsBuffer, &hPal);
          else
            if (hPal && !UsePalette)
              RemovePalette(hpsBuffer, &hPal);
          hps = WinBeginPaint(hwnd, hpsBuffer, NULL);
          if (do_update && (DataBuffer->GraphBuffer != NULL))
            {
              GpiDrawBits(hps, DataBuffer->GraphBuffer, (PVOID) &DataBuffer->bmi,
                          4, DataBuffer->pointl, ROP_SRCCOPY, BBO_IGNORE);
              DataBuffer->modified = FALSE;
            }
          else
            GpiErase(hps);
          WinEndPaint(hps);
          DosReleaseMutexSem(hmtxPS);
        }
        return(0);
      case WM_REALIZEPALETTE:
        if (hPal)
          {
            ULONG cclr;

            WinRequestMutexSem(hmtxPS, SEM_INDEFINITE_WAIT);
            WinRealizePalette(hwnd, hpsBuffer, &cclr);
            DosReleaseMutexSem(hmtxPS);
          }
        break;
      case WM_FINISH:
        WinPostMsg(WinQueryWindow(hwnd, QW_PARENT), WM_QUIT, NULL, NULL);
        return(0);
      case WM_CLOSE:
        if (WinMessageBox(HWND_DESKTOP, hwnd, "Are you sure?", "Close Window?",
                          0, MB_YESNO | MB_ICONQUESTION) == MBID_YES)
          WinPostMsg(WinQueryWindow(hwnd, QW_PARENT), WM_QUIT, NULL, NULL);
        return(0);
      case WM_DESTROY:
        WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, ID_TIMER);
        if (hPal)
          RemovePalette(hpsBuffer, &hPal);
        if (hpsBuffer)
          WinReleasePS(hpsBuffer);
        break;
    }
  if (DataBuffer->GraphBuffer != NULL)
    switch (msg)
      {
        case WM_MOUSEMOVE:
          DataBuffer->mousex       =  1 + (ULONG) SHORT1FROMMP(mp1) * (ULONG) (DataBuffer->pointl[3].x) / (ULONG) (DataBuffer->pointl[1].x + 1);
          DataBuffer->mousey       = -1 + (ULONG) (DataBuffer->pointl[1].y + 1 - SHORT2FROMMP(mp1)) * (ULONG) (DataBuffer->pointl[3].y) / (ULONG) (DataBuffer->pointl[1].y + 1);
          break;
        case WM_BUTTON1DOWN:
          DataBuffer->mousex       =  1 + (ULONG) SHORT1FROMMP(mp1) * (ULONG) (DataBuffer->pointl[3].x) / (ULONG) (DataBuffer->pointl[1].x + 1);
          DataBuffer->mousey       = -1 + (ULONG) (DataBuffer->pointl[1].y + 1 - SHORT2FROMMP(mp1)) * (ULONG) (DataBuffer->pointl[3].y) / (ULONG) (DataBuffer->pointl[1].y + 1);
          DataBuffer->mousestatus |= 1;
          break;
        case WM_BUTTON2DOWN:
          DataBuffer->mousex       =  1 + (ULONG) SHORT1FROMMP(mp1) * (ULONG) (DataBuffer->pointl[3].x) / (ULONG) (DataBuffer->pointl[1].x + 1);
          DataBuffer->mousey       = -1 + (ULONG) (DataBuffer->pointl[1].y + 1 - SHORT2FROMMP(mp1)) * (ULONG) (DataBuffer->pointl[3].y) / (ULONG) (DataBuffer->pointl[1].y + 1);
          DataBuffer->mousestatus |= 2;
          break;
        case WM_BUTTON3DOWN:
          DataBuffer->mousex       =  1 + (ULONG) SHORT1FROMMP(mp1) * (ULONG) (DataBuffer->pointl[3].x) / (ULONG) (DataBuffer->pointl[1].x + 1);
          DataBuffer->mousey       = -1 + (ULONG) (DataBuffer->pointl[1].y + 1 - SHORT2FROMMP(mp1)) * (ULONG) (DataBuffer->pointl[3].y) / (ULONG) (DataBuffer->pointl[1].y + 1);
          DataBuffer->mousestatus |= 4;
          break;
        case WM_BUTTON1UP:
          DataBuffer->mousex       =  1 + (ULONG) SHORT1FROMMP(mp1) * (ULONG) (DataBuffer->pointl[3].x) / (ULONG) (DataBuffer->pointl[1].x + 1);
          DataBuffer->mousey       = -1 + (ULONG) (DataBuffer->pointl[1].y + 1 - SHORT2FROMMP(mp1)) * (ULONG) (DataBuffer->pointl[3].y) / (ULONG) (DataBuffer->pointl[1].y + 1);
          DataBuffer->mousestatus &= ~1;
          break;
        case WM_BUTTON2UP:
          DataBuffer->mousex       =  1 + (ULONG) SHORT1FROMMP(mp1) * (ULONG) (DataBuffer->pointl[3].x) / (ULONG) (DataBuffer->pointl[1].x + 1);
          DataBuffer->mousey       = -1 + (ULONG) (DataBuffer->pointl[1].y + 1 - SHORT2FROMMP(mp1)) * (ULONG) (DataBuffer->pointl[3].y) / (ULONG) (DataBuffer->pointl[1].y + 1);
          DataBuffer->mousestatus &= ~2;
          break;
        case WM_BUTTON3UP:
          DataBuffer->mousex       =  1 + (ULONG) SHORT1FROMMP(mp1) * (ULONG) (DataBuffer->pointl[3].x) / (ULONG) (DataBuffer->pointl[1].x + 1);
          DataBuffer->mousey       = -1 + (ULONG) (DataBuffer->pointl[1].y + 1 - SHORT2FROMMP(mp1)) * (ULONG) (DataBuffer->pointl[3].y) / (ULONG) (DataBuffer->pointl[1].y + 1);
          DataBuffer->mousestatus &= ~4;
          break;
      }
  return(WinDefWindowProc(hwnd, msg, mp1, mp2));
}

void ProcessOutput(HPIPE hpipe, ULONG MsgId)
{
  ULONG read;
  CHAR  buffer[80];

  while (!DosRead(hpipe, buffer, sizeof(buffer), &read))
    WinPostMsg(DataBuffer->hwndText, MsgId, MPFROMLONG(read), MPFROMP(buffer));
}

void StdOutThread(ULONG x)
{
  ProcessOutput(hpipe3, WM_NEWCHAR);
}

void StdErrThread(ULONG x)
{
  ProcessOutput(hpipe4, WM_NEWCHAR);
}

MRESULT TextWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static HPS  hps;
  static HVPS hvps;

  switch (msg)
    {
      case WM_CREATE:
        {
          HDC   hdc;
          SIZEL sizel;

          DataBuffer->hwndText    = hwnd;
          InstallMenu(hwnd);

          hdc = WinOpenWindowDC(hwnd);
          sizel.cx = sizel.cy = 0;
          hps = GpiCreatePS(WinQueryAnchorBlock(hwnd), hdc, &sizel,
                            PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);
          VioCreatePS(&hvps, TEXT_ROWS, TEXT_COLS, 0, 1, 0);
          VioAssociate(hdc, hvps);
          return(0);
        }
      case WM_SIZE:
        WinDefAVioWindowProc(hwnd, msg, LONGFROMMP(mp1), LONGFROMMP(mp2));
        WinPostMsg(hwnd, WM_FONTSIZECHANGED, NULL, NULL);
        return(0);
      case WM_NEWCHAR:
        VioWrtTTY(PVOIDFROMMP(mp2), LONGFROMMP(mp1), hvps);
        return(0);
      case WM_CHAR:
        WinSendMsg(DataBuffer->hwndGraph, msg, mp1, mp2);
        break;
      case WM_COMMAND:
        switch (COMMANDMSG(&msg) -> cmd)
          {
            case IDM_SETTINGS:
              WinSendMsg(DataBuffer->hwndGraph, msg, mp1, mp2);
              break;
            case IDM_UPDATE:
              WinInvalidateRect(hwnd, NULL, FALSE);
              return(0);
          }
        break;
      case WM_PAINT:
        WinBeginPaint(hwnd, hps, NULL);
        VioShowBuf(0, TEXT_ROWS * TEXT_COLS * 2, hvps);
        WinEndPaint(hps);
        return(0);
      case WM_FONTSIZECHANGED:
        {
          SWP   swp;
          RECTL rectl;

          VioSetDeviceCellSize(YChar, XChar, hvps);
          VioGetDeviceCellSize(&YChar, &XChar, hvps);
          WinQueryWindowPos(WinQueryWindow(hwnd, QW_PARENT), &swp);
          WinQueryWindowRect(hwnd, &rectl);
          if (((swp.fl & SWP_MINIMIZE) == 0) &&
              ((rectl.xRight - rectl.xLeft   > XChar * TEXT_COLS) ||
               (rectl.yTop   - rectl.yBottom > YChar * TEXT_ROWS)))
            {
              rectl.xLeft   = 0;
              rectl.yBottom = 0;
              rectl.xRight  = XChar * TEXT_COLS + 4;
              rectl.yTop    = YChar * TEXT_ROWS + 4;
              WinCalcFrameRect(WinQueryWindow(hwnd, QW_PARENT), &rectl, FALSE);
              WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT), 0, 0, 0,
                              rectl.xRight, rectl.yTop, SWP_SIZE);
            }
          WinInvalidateRect(hwnd, NULL, FALSE);
          return(0);
        }
      case WM_CLOSE:
        if (WinMessageBox(HWND_DESKTOP, hwnd, "Are you sure?", "Close Window?",
                          0, MB_YESNO | MB_ICONQUESTION) == MBID_YES)
          WinPostMsg(WinQueryWindow(hwnd, QW_PARENT), WM_QUIT, NULL, NULL);
        return(0);
      case WM_DESTROY:
        VioAssociate(0, hvps);
        VioDestroyPS(hvps);
        GpiDestroyPS(hps);
        return(0);
    }
  return(WinDefWindowProc(hwnd, msg, mp1, mp2));
}

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;

  hab = WinInitialize(0);
  hmq = WinCreateMsgQueue(hab, 0);
  if (argc != 2)
    {
      DataBuffer = NULL;
      WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, SettingsDlgProc,
                NULLHANDLE, ID_SETTINGS, NULL);
    }
  else
    {
      static CHAR  szGraphClass[] = "PMSCREEN_GRAPH";
      static CHAR  szTextClass[]  = "PMSCREEN_TEXT";
      static ULONG flFrameFlags   = FCF_TITLEBAR      | FCF_SYSMENU   |
                                    FCF_SIZEBORDER    | FCF_MINMAX    |
                                    FCF_SHELLPOSITION | FCF_TASKLIST  |
                                    FCF_ICON;
      TID   tid;
      QMSG  qmsg;
      HWND  hwndGraphFrame, hwndGraphClient;
      HWND  hwndTextFrame, hwndTextClient;
      ULONG action;

      DataBuffer = (DATABUFFER *) atoi(argv[1]);
      DosGetSharedMem(DataBuffer, PAG_READ | PAG_WRITE);
      WinRegisterClass(hab, szGraphClass, GraphWndProc, 0, 0);
      WinRegisterClass(hab, szTextClass, TextWndProc, 0, 0);
      hwndTextFrame  = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE, &flFrameFlags,
                                          szTextClass, "PM-Screen Text Window",
                                          0, 0, ID_PMSCREEN, &hwndTextClient);
      hwndGraphFrame = WinCreateStdWindow(HWND_DESKTOP, 0, &flFrameFlags,
                                          szGraphClass, "PM-Screen Graphics Window",
                                          0, 0, ID_PMSCREEN, &hwndGraphClient);
      WinSetWindowPos(hwndGraphFrame, 0, 0, 0, 0, 0, SWP_SHOW | SWP_MINIMIZE);
      DosOpenEventSem(NULL, &DataBuffer->hev);
      DosPostEventSem(DataBuffer->hev);
      DosWaitNPipe(DataBuffer->Pipe1Name, -1);
      DosOpen(DataBuffer->Pipe1Name, &hpipe1, &action, 0, FILE_NORMAL,
              OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
              OPEN_SHARE_DENYNONE | OPEN_ACCESS_WRITEONLY, NULL);
      DosSetNPHState(hpipe1, NP_NOWAIT);
      DosWaitNPipe(DataBuffer->Pipe2Name, -1);
      DosOpen(DataBuffer->Pipe2Name, &hpipe2, &action, 0, FILE_NORMAL,
              OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
              OPEN_SHARE_DENYNONE | OPEN_ACCESS_WRITEONLY, NULL);
      DosSetNPHState(hpipe2, NP_NOWAIT);
      DosWaitNPipe(DataBuffer->Pipe3Name, -1);
      DosOpen(DataBuffer->Pipe3Name, &hpipe3, &action, 0, FILE_NORMAL,
              OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
              OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, NULL);
      DosWaitNPipe(DataBuffer->Pipe4Name, -1);
      DosOpen(DataBuffer->Pipe4Name, &hpipe4, &action, 0, FILE_NORMAL,
              OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
              OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, NULL);
      DosCreateThread(&tid, StdOutThread, 0, 0, 32768);
      DosCreateThread(&tid, StdErrThread, 0, 0, 32768);
      while (WinGetMsg(hab, &qmsg, 0, 0, 0))
        WinDispatchMsg(hab, &qmsg);
      DosPostEventSem(DataBuffer->hev);
      DosCloseEventSem(DataBuffer->hev);
      DosClose(hpipe1);
      DosClose(hpipe2);
      DosClose(hpipe3);
      DosClose(hpipe4);
      WinDestroyWindow(hwndGraphFrame);
      WinDestroyWindow(hwndTextFrame);
    }
  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);
  return(argc != 2);
}
