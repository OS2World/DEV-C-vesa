/* VESA package for emx/gcc --- Copyright (c) 1993 by Johannes Martin */
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <sys/hw.h>
#include <conio.h>

#include "vesa.h"
#include "vesa_dos.h"
#include "vgaports.h"

#define STANDARD_MODES 3
#define DEFAULT_MODES 23

static VESABOOL textmode = TRUE;

static VESAMODEINFO standard_infos[STANDARD_MODES]
 = { { 0x1f, 0x7, 0x0, 64, 64, 0x0, 0x0, DosVesaSetWindow, DosVesaGetWindow,
       80, 640,  480, 8, 16, 4,  4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
     { 0x1f, 0x7, 0x0, 64, 64, 0x0, 0x0, DosVesaSetWindow, DosVesaGetWindow,
       320, 320,  200, 8,  8, 1,  8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
     { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0, DosVesaSetWindow, DosVesaGetWindow,
       100, 800, 600, 8, 16, 4, 4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } };

struct _defaultinfo
  {
    VESAWORD Mode;
    VESAWORD Width;
    VESAWORD Height;
    VESACHAR CharacterWidth;
    VESACHAR CharacterHeight;
    VESACHAR NumberOfMemoryPlanes;
    VESACHAR NumberOfBitsPerPixel;
    VESACHAR NumberOfBanks;
    VESACHAR MemoryModelType;
    VESACHAR SizeOfBank;
    VESACHAR NumberofPages;
    VESACHAR RedMaskSize;
    VESACHAR RedMaskPosition;
    VESACHAR GreenMaskSize;
    VESACHAR GreenMaskPosition;
    VESACHAR BlueMaskSize;
    VESACHAR BlueMaskPosition;
    VESACHAR ReservedMaskSize;
    VESACHAR ReservedMaskPosition;
  };

static struct _defaultinfo default_infos[DEFAULT_MODES]
  = { { 0x100,  640,  400, 8, 16, 1,  8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0x101,  640,  480, 8, 16, 1,  8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0x102,  800,  600, 8, 16, 1,  4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0x103,  800,  600, 8, 16, 1,  8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0x104, 1024,  768, 8, 16, 1,  4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0x105, 1024,  768, 8, 16, 1,  8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0x106, 1280, 1024, 8, 16, 1,  4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0x107, 1280, 1024, 8, 16, 1,  8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0x10D,  320,  200, 8,  8, 1, 16, 1, 6, 0, 1, 5, 10, 5, 5, 5, 0, 1, 15 },
      { 0x10E,  320,  200, 8,  8, 1, 16, 1, 6, 0, 1, 5, 11, 6, 5, 5, 0, 0, 0 },
      { 0x10F,  320,  200, 8,  8, 1, 24, 1, 6, 0, 1, 8, 16, 8, 8, 8, 0, 0, 0 },
      { 0x110,  640,  480, 8, 16, 1, 16, 1, 6, 0, 1, 5, 10, 5, 5, 5, 0, 1, 15 },
      { 0x111,  640,  480, 8, 16, 1, 16, 1, 6, 0, 1, 5, 11, 6, 5, 5, 0, 0, 0 },
      { 0x112,  640,  480, 8, 16, 1, 24, 1, 6, 0, 1, 8, 16, 8, 8, 8, 0, 0, 0 },
      { 0x113,  800,  600, 8, 16, 1, 16, 1, 6, 0, 1, 5, 10, 5, 5, 5, 0, 1, 15 },
      { 0x114,  800,  600, 8, 16, 1, 16, 1, 6, 0, 1, 5, 11, 6, 5, 5, 0, 0, 0 },
      { 0x115,  800,  600, 8, 16, 1, 24, 1, 6, 0, 1, 8, 16, 8, 8, 8, 0, 0, 0 },
      { 0x116, 1024,  768, 8, 16, 1, 16, 1, 6, 0, 1, 5, 10, 5, 5, 5, 0, 1, 15 },
      { 0x117, 1024,  768, 8, 16, 1, 16, 1, 6, 0, 1, 5, 11, 6, 5, 5, 0, 0, 0 },
      { 0x118, 1024,  768, 8, 16, 1, 24, 1, 6, 0, 1, 8, 16, 8, 8, 8, 0, 0, 0 },
      { 0x119, 1280, 1024, 8, 16, 1, 16, 1, 6, 0, 1, 5, 10, 5, 5, 5, 0, 1, 15 },
      { 0x11A, 1280, 1024, 8, 16, 1, 16, 1, 6, 0, 1, 5, 11, 6, 5, 5, 0, 0, 0 },
      { 0x11B, 1280, 1024, 8, 16, 1, 24, 1, 6, 0, 1, 8, 16, 8, 8, 8, 0, 0, 0 } };

static struct __access
  {
    VESAWORD Segment, Offset;
    int      Size, flag;
    VESAPTR  Mem;
    struct __access *next;
  } *access_root = NULL;


static VESAPTR GetMemAccess(VESAWORD Segment, VESAWORD Offset, int Size, int flag)
{
  struct __access *ptr;

  ptr = access_root;
  while ((ptr != NULL) &&
         ((ptr->Segment != Segment) ||
          (ptr->Offset  != Offset)  ||
          (ptr->Size    != Size)    ||
          (ptr->flag    != flag)))
    ptr = ptr->next;
  if (ptr != NULL)
    return(ptr->Mem);
  else
    {
      VESAPTR Mem;
      int     first, last, offs;

      offs  = (Segment << 4) + Offset;
      first = (offs / 4096) * 4096;
      last  = ((offs + Size) / 4096) * 4096 + 4096 - 1;
      if ((Mem = _memaccess(first, last, flag)) == NULL)
        return(NULL);
      if ((ptr = malloc(sizeof(struct __access))) != NULL)
        {
          ptr->Segment = Segment;
          ptr->Offset  = Offset;
          ptr->Size    = Size;
          ptr->flag    = flag;
          ptr->Mem     = Mem + offs - first;
          ptr->next    = access_root;
          access_root  = ptr;
        }
      return(Mem + offs - first);
    }
}

VESABOOL DosVesaGetInfo(PVESAINFO pVesaInfo)
{
  union REGS r;
  VESAWORD *p, i;
#pragma pack(1)
  union
    {
      struct
        {
          VESACHAR  signature[4];
          VESAWORD  version;
          PVESACHAR oem;
          VESACHAR  capabilities[4];
	  PVESAWORD modes;
	  VESAWORD  banks;
	} info;
      char fill[256];
    } Info;
#pragma pack(4)

  r.x.ax  = 0x4f00;
  r.e.edi = (unsigned long) &Info;
  _int86(0x10, &r, &r);
  if ((r.x.ax != 0x004f) || (memcmp(Info.info.signature, "VESA", 4) != 0))
    return(FALSE);
  pVesaInfo->Version     = Info.info.version;
  pVesaInfo->OEMName     = Info.info.oem;
  pVesaInfo->MemoryBanks = Info.info.banks;
  p = Info.info.modes;
  for (i = 0; p[i] != 0xFFFF; i++);
  if ((pVesaInfo->Modes = malloc((i + 4) * sizeof(VESAWORD))) == NULL)
    pVesaInfo->Modes = p;
  else
    {
      memcpy(pVesaInfo->Modes, p, i * sizeof(VESAWORD));
      pVesaInfo->Modes[i + 0] = 0x12;
      pVesaInfo->Modes[i + 1] = 0x13;
      pVesaInfo->Modes[i + 2] = 0x6A;
      pVesaInfo->Modes[i + 3] = 0xFFFF;
    }
  return(TRUE);
}

VESABOOL DosVesaGetModeInfo(VESAWORD Mode, PVESAMODEINFO pVesaModeInfo)
{
  union REGS r;
  int  i;
#pragma pack(1)
  union
    {
      struct
	{
	  VESAWORD mode_attr;
	  VESACHAR window_a_attr;
	  VESACHAR window_b_attr;
	  VESAWORD window_gran;
	  VESAWORD window_size;
	  VESAWORD window_a_start;
	  VESAWORD window_b_start;
	  VESAPTR  window_fun;
	  VESAWORD bytes_per_line;
	  VESAWORD width;
	  VESAWORD height;
	  VESACHAR char_width;
	  VESACHAR char_height;
	  VESACHAR num_planes;
	  VESACHAR num_bits;
	  VESACHAR num_banks;
	  VESACHAR mem_model;
	  VESACHAR bank_size;
	  VESACHAR num_pages;
	  VESACHAR reserved;
	  VESACHAR red_mask_size;
	  VESACHAR red_mask_pos;
	  VESACHAR green_mask_size;
	  VESACHAR green_mask_pos;
	  VESACHAR blue_mask_size;
	  VESACHAR blue_mask_pos;
	  VESACHAR res_mask_size;
	  VESACHAR res_mask_pos;
	  VESACHAR direct_mode_info;
	} info;
      char fill[256];
    } Info;
#pragma pack(4)

  if ((Mode == 0x12) || (Mode == 0x13) || (Mode == 0x6A))
    {
      if (Mode == 0x6A)
	*pVesaModeInfo = standard_infos[2];
      else
	*pVesaModeInfo = standard_infos[Mode - 0x12];
      pVesaModeInfo->WindowAStart = GetMemAccess(0xA000, 0, 64 * 1024, 1);
      pVesaModeInfo->WindowBStart = pVesaModeInfo->WindowAStart;
      return(TRUE);
    }
  r.x.ax  = 0x4f01;
  r.x.cx  = Mode;
  r.e.edi = (unsigned long) &Info;
  _int86(0x10, &r, &r);
  if (r.x.ax != 0x004f)
    {
      pVesaModeInfo->ModeAttributes = 0;
      return(FALSE);
    }
  pVesaModeInfo->ModeAttributes       = Info.info.mode_attr;
  pVesaModeInfo->WindowAAttributes    = Info.info.window_a_attr;
  pVesaModeInfo->WindowBAttributes    = Info.info.window_b_attr;
  pVesaModeInfo->WindowGranularity    = Info.info.window_gran;
  pVesaModeInfo->WindowSize           = Info.info.window_size;
  pVesaModeInfo->WindowAStart         = GetMemAccess(Info.info.window_a_start, 0, pVesaModeInfo->WindowSize * 1024, pVesaModeInfo->WindowAAttributes & 4 ? 1 : 0);
  pVesaModeInfo->WindowBStart         = GetMemAccess(Info.info.window_b_start, 0, pVesaModeInfo->WindowSize * 1024, pVesaModeInfo->WindowBAttributes & 4 ? 1 : 0);
  pVesaModeInfo->SetWindowFunc        = DosVesaSetWindow;
  pVesaModeInfo->GetWindowFunc        = DosVesaGetWindow;
  pVesaModeInfo->BytesPerScanline     = Info.info.bytes_per_line;
  if (pVesaModeInfo->ModeAttributes & 2)
    {
      pVesaModeInfo->Width                = Info.info.width;
      pVesaModeInfo->Height               = Info.info.height;
      pVesaModeInfo->CharacterWidth       = Info.info.char_width;
      pVesaModeInfo->CharacterHeight      = Info.info.char_height;
      pVesaModeInfo->NumberOfMemoryPlanes = Info.info.num_planes;
      pVesaModeInfo->NumberOfBitsPerPixel = Info.info.num_bits;
      pVesaModeInfo->NumberOfBanks        = Info.info.num_banks;
      pVesaModeInfo->MemoryModelType      = Info.info.mem_model;
      pVesaModeInfo->SizeOfBank           = Info.info.bank_size;
      pVesaModeInfo->NumberofPages        = Info.info.num_pages;
      pVesaModeInfo->RedMaskSize          = Info.info.red_mask_size;
      pVesaModeInfo->RedMaskPosition      = Info.info.red_mask_pos;
      pVesaModeInfo->GreenMaskSize        = Info.info.green_mask_size;
      pVesaModeInfo->GreenMaskPosition    = Info.info.green_mask_pos;
      pVesaModeInfo->BlueMaskSize         = Info.info.blue_mask_size;
      pVesaModeInfo->BlueMaskPosition     = Info.info.blue_mask_pos;
      pVesaModeInfo->ReservedMaskSize     = Info.info.res_mask_size;
      pVesaModeInfo->ReservedMaskPosition = Info.info.res_mask_pos;
      switch (pVesaModeInfo->NumberOfBitsPerPixel)
	{
	  case 15:
	    pVesaModeInfo->NumberOfBitsPerPixel = 16;
	  case 16:
	    if ((pVesaModeInfo->RedMaskSize == 0) ||
		(pVesaModeInfo->RedMaskSize   +
		 pVesaModeInfo->GreenMaskSize +
		 pVesaModeInfo->BlueMaskSize  +
		 pVesaModeInfo->ReservedMaskPosition != 16))
	      memcpy(&pVesaModeInfo->RedMaskSize, &default_infos[11].RedMaskSize,
		     &default_infos[11].ReservedMaskPosition -
		     &default_infos[11].RedMaskSize + sizeof(VESACHAR));
	    pVesaModeInfo->MemoryModelType = 6;
	    break;
	  case 24:
	    if ((pVesaModeInfo->RedMaskSize == 0) ||
		(pVesaModeInfo->RedMaskSize   +
		 pVesaModeInfo->GreenMaskSize +
		 pVesaModeInfo->BlueMaskSize  +
		 pVesaModeInfo->ReservedMaskPosition != 24))
	      memcpy(&pVesaModeInfo->RedMaskSize, &default_infos[13].RedMaskSize,
		     &default_infos[13].ReservedMaskPosition -
		     &default_infos[13].RedMaskSize + sizeof(VESACHAR));
	    pVesaModeInfo->MemoryModelType = 6;
	    break;
	  default:
	    if (pVesaModeInfo->NumberOfBitsPerPixel <= 8)
	      memset(&pVesaModeInfo->RedMaskSize, 0,
		     &default_infos[13].ReservedMaskPosition -
		     &default_infos[13].RedMaskSize + sizeof(VESACHAR));
	}
      return(TRUE);
    }

  for (i = 0; i < DEFAULT_MODES; i++)
    if (default_infos[i].Mode == Mode)
      {
	pVesaModeInfo->ModeAttributes |= 2;
	memcpy(&pVesaModeInfo->Width, &default_infos[i].Width,
	       sizeof(default_infos[i]) - sizeof(default_infos[i].Mode));
	return(TRUE);
      }
  return(FALSE);
}

VESABOOL DosVesaSetMode(VESAWORD Mode)
{
  union REGS r;

  r.x.ax = 0x4f02;
  r.x.bx = Mode;
  _int86(0x10, &r, &r);
  return(r.x.ax == 0x004f);
}

VESABOOL DosVesaGetMode(PVESAWORD Mode)
{
  union REGS r;

  r.x.ax = 0x4f03;
  _int86(0x10, &r, &r);
  *Mode = r.x.bx;
  return(r.x.ax == 0x004f);
}

VESABOOL DosVesaSetWindow(VESACHAR Window, VESAWORD Address)
{
  union REGS r;

  r.x.ax = 0x4f05;
  r.h.bh = 0x00;
  r.h.bl = Window;
  r.x.dx = Address;
  _int86(0x10, &r, &r);
  return(r.x.ax == 0x004f);
}

VESABOOL DosVesaGetWindow(VESACHAR Window, PVESAWORD Address)
{
  union REGS r;

  r.x.ax = 0x4f05;
  r.h.bh = 0x01;
  r.h.bl = Window;
  _int86(0x10, &r, &r);
  *Address = r.x.dx;
  return(r.x.ax == 0x004f);
}

VESABOOL DosVesaSetScanlineLength(VESAWORD NumberOfPixels)
{
  union REGS r;

  r.x.ax = 0x4f06;
  r.h.bl = 0x00;
  r.x.cx = NumberOfPixels;
  _int86(0x10, &r, &r);
  return(r.x.ax == 0x004f);
}

VESABOOL DosVesaGetScanlineLength(PVESAWORD BytesPerScanline,
                                  PVESAWORD NumberOfPixels,
                                  PVESAWORD NumberOfScanlines)
{
  union REGS r;

  r.x.ax = 0x4f06;
  r.h.bl = 0x01;
  _int86(0x10, &r, &r);
  *BytesPerScanline  = r.x.bx;
  *NumberOfPixels    = r.x.cx;
  *NumberOfScanlines = r.x.dx;
  return(r.x.ax == 0x004f);
}

VESABOOL DosVesaSetPalette(VESAWORD   StartIndex,
                           VESAWORD   NumberOfEntries,
                           PCVESACHAR Palette,
                           VESABOOL   Wait)
{
  if ((NumberOfEntries >= 1) && (StartIndex + NumberOfEntries <= 256))
    {
      if (Wait)
        _wait01 (IS1_R, 0x08);
      _outp8 (PEL_IW, StartIndex);
      _outps8dac (PEL_D, Palette, 3 * NumberOfEntries);
      return(TRUE);
    }
  return(FALSE);
}

VESABOOL DosVesaGetPalette(VESAWORD  StartIndex,
                           VESAWORD  NumberOfEntries,
                           PVESACHAR Palette)
{
  if ((NumberOfEntries >= 1) && (StartIndex + NumberOfEntries <= 256))
    {
      int i, j;

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
      return(TRUE);
    }
  return(FALSE);
}

VESABOOL DosVesaGetCharacter(PVESACHAR pchar)
{
  *pchar = getch();
  return(TRUE);
}

VESABOOL DosVesaKeyboardHit(void)
{
  union REGS r;

  r.h.ah = 1;
  _int86(0x16, &r, &r);
  return((r.e.eflags & 0x40) == 0);
}

VESABOOL DosNoVesaGetInfo(PVESAINFO pVesaInfo)
{
  static VESAWORD Modes[4] = { 0x12, 0x13, 0x6A, 0xFFFF };

  pVesaInfo->Version = 0x102;
  pVesaInfo->OEMName = "DOS VESA Replacement by Johannes Martin";
  pVesaInfo->Modes   = Modes;
  return(TRUE);
}

VESABOOL DosNoVesaGetModeInfo(VESAWORD Mode, PVESAMODEINFO pVesaModeInfo)
{
  if ((Mode == 0x12) || (Mode == 0x13) || (Mode == 0x6A))
    {
      if (Mode == 0x6A)
        *pVesaModeInfo = standard_infos[2];
      else
        *pVesaModeInfo = standard_infos[Mode - 0x12];
      pVesaModeInfo->WindowAStart = GetMemAccess(0xA000, 0, 64 * 1024, 1);
      pVesaModeInfo->WindowBStart = pVesaModeInfo->WindowAStart;
      return(TRUE);
    }
  return(FALSE);
}

VESABOOL DosNoVesaSetMode(VESAWORD Mode)
{
  union REGS r;

  if (Mode & 0xFF00)
    return(FALSE);
  r.x.ax = Mode;
  _int86(0x10, &r, &r);
  r.h.ah = 0x0f;
  _int86(0x10, &r, &r);
  return(r.h.al == Mode);
}

VESABOOL DosNoVesaGetMode(PVESAWORD Mode)
{
  union REGS r;

  r.h.ah = 0x0f;
  _int86(0x10, &r, &r);
  *Mode = r.h.al;
  return(TRUE);
}

VESABOOL DosNoVesaSetWindow(VESACHAR Window, VESAWORD Address)
{
  return(TRUE);
}

VESABOOL DosNoVesaGetWindow(VESACHAR Window, PVESAWORD Address)
{
  *Address = 0;
  return(TRUE);
}

VESABOOL DosNoVesaSetScanlineLength(VESAWORD NumberOfPixels)
{
  return(FALSE);
}

VESABOOL DosNoVesaGetScanlineLength(PVESAWORD BytesPerScanline,
                                    PVESAWORD NumberOfPixels,
                                    PVESAWORD NumberOfScanlines)
{
  union REGS r;

  r.h.ah = 0x0f;
  _int86(0x10, &r, &r);
  switch (r.h.al)
    {
      case 0x12:
        *BytesPerScanline  = 80;
        *NumberOfPixels    = 640;
        *NumberOfScanlines = 480;
        return(TRUE);
      case 0x13:
        *BytesPerScanline  = 320;
        *NumberOfPixels    = 320;
        *NumberOfScanlines = 200;
        return(TRUE);
      case 0x6A:
        *BytesPerScanline  = 100;
        *NumberOfPixels    = 800;
        *NumberOfScanlines = 600;
        return(TRUE);
    }
  return(FALSE);
}

VESABOOL DosMouseOpen(void)
{
  union REGS   r;
  VESAWORD     Mode, Width, Height;
  VESAMODEINFO Info;
  VESABOOL     rc;
  VESACHAR     BiosMode, *BiosAddr;

  VesaGetMode(&Mode);
  if (VesaGetModeInfo(Mode, &Info))
    if (Info.ModeAttributes & 0x10)
      {
        textmode = FALSE;
        Width    = Info.Width - 1;
        Height   = Info.Height - 1;
      }
    else
      {
        textmode = TRUE;
        Width    = (Info.Width - 1) * 8;
        Height   = (Info.Height - 1) * 8;
      }
  else
    {
      textmode = TRUE;
      r.h.ah   = 0x0f;
      _int86(0x10, &r, &r);
      Width    = (r.h.ah - 1) * 8;
      Height   = 24 * 8;
/*      r.x.ax   = 0x1130;
      r.h.bh   = 0;
      r.h.dl   = 0;
      _int86(0x10, &r, &r);
      if (r.h.dl == 0)
        Height = 24 * 8;
      else
        Height = r.h.dl * 8;
*/    }
  if (!textmode)
    {
      BiosAddr  = GetMemAccess(0x40, 0x49, 1, 1);
      BiosMode  = *BiosAddr;
      *BiosAddr = 0x6;
    }
  r.x.ax = 0x0000;
  _int86(0x33, &r, &r);
  if (!textmode)
    *BiosAddr = BiosMode;
  rc = r.x.ax == 0xffff;
  r.x.ax   = 0x0007;
  r.x.cx   = 0x0000;
  r.x.dx   = Width;
  _int86(0x33, &r, &r);
  r.x.ax   = 0x0008;
  r.x.cx   = 0x0000;
  r.x.dx   = Height;
  _int86(0x33, &r, &r);
  return(rc);
}

VESABOOL DosMouseClose(void)
{
  union REGS r;

  r.x.ax = 0x0000;
  _int86(0x33, &r, &r);
  return(TRUE);
}

VESABOOL DosMouseShow(void)
{
  union REGS r;

  r.x.ax = 0x0001;
  _int86(0x33, &r, &r);
  return(TRUE);
}

VESABOOL DosMouseHide(void)
{
  union REGS r;

  r.x.ax = 0x0002;
  _int86(0x33, &r, &r);
  return(TRUE);
}

VESABOOL DosMouseSetPosition(VESAWORD x, VESAWORD y)
{
  union REGS r;

  if (textmode)
    {
      r.x.cx *= 8;
      r.x.dx *= 8;
    }
  r.x.ax = 0x0004;
  r.x.cx = x;
  r.x.dx = y;
  _int86(0x33, &r, &r);
  return(TRUE);
}

VESABOOL DosMouseGetPosition(PVESAWORD x, PVESAWORD y)
{
  union REGS r;

  r.x.ax = 0x0003;
  _int86(0x33, &r, &r);
  *x = r.x.cx;
  *y = r.x.dx;
  if (textmode)
    {
      *x /= 8;
      *y /= 8;
    }
  return(TRUE);
}

VESABOOL DosMouseGetEvent(PVESAWORD x, PVESAWORD y, PVESAWORD status, VESABOOL wait)
{
  static VESAWORD oldx      = 0;
  static VESAWORD oldy      = 0;
  static VESAWORD oldstatus = 0;
  union REGS r;
  VESABOOL event;

  do
    {
      r.x.ax = 0x0003;
      _int86(0x33, &r, &r);
      event = (r.x.cx != oldx) || (r.x.dx != oldy) || (r.x.bx != oldstatus);
    }
  while (wait && !event);
  *x      = oldx      = r.x.cx;
  *y      = oldy      = r.x.dx;
  *status = oldstatus = r.x.bx;
  if (textmode)
    {
      *x /= 8;
      *y /= 8;
    }
  return(event);
}
