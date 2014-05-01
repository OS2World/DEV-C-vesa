/* VESA package for emx/gcc --- Copyright (c) 1993 by Johannes Martin */
/* vesa_cl.c                --- Copyright (c) 1994 by Laurent Bartholdi */
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

#include <stdio.h>

static VESAWORD _Modes[] = { 0x003, 0x012, 0x013,
                             0x101, 0x0102, 0x103, 0x104, 0x105,
#ifdef HICOLOR
                             0x110, 0x113, 0x111, 0x114,
                             0x112,
#endif
                             0xFFFF };

static struct _ModeInfo _ModeInfos[] =
    { { 0x1, { 0x0f, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 160,
               640, 400, 8, 16, 1, 4, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0x3, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 80,
               640, 480, 8, 16, 1, 4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0x3, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 320,
               320, 200, 8,  8, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 640,
               640, 480, 8, 16, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 100,
               800, 600, 8, 16, 1, 4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 800,
               800, 600, 8, 16, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 128,
               1024, 768, 8, 16, 1, 4, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
      { 0xb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 1024,
               1024, 768, 8, 16, 1, 8, 1, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
#ifdef HICOLOR
      { 0xfb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 640*2,
               640, 480, 8, 16, 1, 16, 1, 6, 0, 1, 5,10, 5, 5, 5, 0, 1, 15 } },
      { 0xfb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 800*2,
               800, 600, 8, 16, 1, 16, 1, 6, 0, 1, 5,10, 5, 5, 5, 0, 1, 15 } },
      { 0xfb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 640*2,
               640, 480, 8, 16, 1, 16, 1, 6, 0, 1, 5, 11, 6, 5, 5, 0, 0, 0 } },
      { 0xfb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 800*2,
               800, 600, 8, 16, 1, 16, 1, 6, 0, 1, 5, 11, 6, 5, 5, 0, 0, 0 } },
      { 0xfb, { 0x1b, 0x7, 0x0, 64, 64, 0x0, 0x0,
               OS2VesaSetWindow, OS2VesaGetWindow, 640*3,
               640, 480, 8, 16, 1, 24, 1, 6, 0, 1, 8, 16, 8, 8, 8, 0, 0, 0 } },
#endif
    };

VESAWORD *Modes             = _Modes;
struct _ModeInfo *ModeInfos = _ModeInfos;

VESABOOL Initialize(void)
{
  _portaccess(0x3c0,0x3df);
  return TRUE;
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

#ifdef HICOLOR

unsigned char inindex( unsigned port, unsigned char index )
{
  _outp8( port, index );
  return _inp8( port+1 );
}

void outindex( unsigned port, unsigned char index, unsigned char data )
{
  _outp8( port, index );
  _outp8( port+1, data );
}

#define OUT8 0x0
#define OUT16 0x1
#define IN8 0x2
#define IN16 0x3
#define OUTI 0x4
#define INI 0x5

void playback( unsigned long code[] )
{
  while( *code )
  {
    unsigned opcode = (*code >> 28) & 0xf;
    unsigned port = (*code >> 16) & 0xfff;
    unsigned ax = (*code >> 0) & 0xffff;
    unsigned al = (*code >> 8) & 0xff;
    unsigned data = (*code >> 0) & 0xff;

    switch( opcode )
    {
    case OUT8: _outp8( port, al );
#ifdef DEBUG
printf("outp8(0x%X,0x%02X);\n", port, al );
#endif
      break;
    case OUT16: _outp16( port, ax );
#ifdef DEBUG
printf("outp16(0x%X,0x%04X);\n", port, ax );
#endif
      break;
    case IN8: al = _inp8( port );
#ifdef DEBUG
printf("/*%02X=*/inp8(0x%X);\n", al, port );
#endif
      break;
    case IN16: ax = _inp16( port );
#ifdef DEBUG
printf("/*%04X=*/inp16(0x%X);\n", ax, port );
#endif
      break;
    case OUTI: outindex( port, al, data );
#ifdef DEBUG
printf("outindex(0x%X,0x%02X,0x%02X);\n", port, al, data );
#endif
      break;
    case INI: data = inindex( port, al );
#ifdef DEBUG
printf("/*%02X=*/inindex(0x%X,0x%02X);\n", data, port, al );
#endif
    }
    code++;
  }
}

#endif

VESABOOL SetMode(VESAWORD mode)
{
  VIOMODEINFO m;

#ifdef HICOLOR

#include "cirrus/3.c"
  playback( mode3 ); /* dirty: resets text mode (3) to ensure we
                        have the proper 8-bit settings */

#endif

  m.cb     = 12;
  m.fbType = ModeInfos[mode].fbtype & 0x0f;
  m.color  = ModeInfos[mode].Vesa.NumberOfBitsPerPixel;
  m.col    = ModeInfos[mode].Vesa.Width / ModeInfos[mode].Vesa.CharacterWidth;
  m.row    = ModeInfos[mode].Vesa.Height/ ModeInfos[mode].Vesa.CharacterHeight;
  m.hres   = ModeInfos[mode].Vesa.Width;
  m.vres   = ModeInfos[mode].Vesa.Height;

  if( m.color > 8 )
    m.color = 8;

  return VioSetMode(&m, 0) == 0;
}

void SetSpecial(VESAWORD mode)
{
#ifdef HICOLOR

#include "cirrus/110.c"
#include "cirrus/111.c"
#include "cirrus/112.c"
#include "cirrus/113.c"
#include "cirrus/114.c"
  switch( _Modes[mode] )
  {
  case 0x110: playback( mode110 ); break;
  case 0x111: playback( mode111 ); break;
  case 0x112: playback( mode112 ); break;
  case 0x113: playback( mode113 ); break;
  case 0x114: playback( mode114 ); break;
  }

#endif
}

VESABOOL OS2VesaSetWindow(VESACHAR Window, VESAWORD Address)
{
  _outp16(GRA_I, (Address << 12) | 0x09);
  return(TRUE);
}

VESABOOL OS2VesaGetWindow(VESACHAR Window, PVESAWORD Address)
{
  _outp8(GRA_I, 0x09);
  *Address = _inp8(GRA_D);
  return(TRUE);
}
