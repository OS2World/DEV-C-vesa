/* VESA package for emx/gcc --- Copyright (c) 1993 by Johannes Martin */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/hw.h>

#include "vesa.h"
#include "vgaports.h"

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

static int et4000_test(void)
{
  char new, old, val;
  int  base;

  /* test for Tseng clues */
  old = _inp8(0x3cd);
  _outp8(0x3cd, 0x55);
  new = _inp8(0x3cd);
  _outp8(0x3cd, old);

  /* return false if not Tseng */
  if (new != 0x55)
    return 0;

  /* test for ET4000 clues */
  if (_inp8(0x3cc) & 1)
    base = 0x3d4;
  else
    base = 0x3b4;
  _outp8(base, 0x33);
  old = _inp8(base+1);
  new = old ^ 0xf;
  _outp8(base+1, new);
  val = _inp8(base+1);
  _outp8(base+1, old);

  /* return true if ET4000 */
  return (val == new);
}

static int getdac(void)
{
  _inp8(0x3c8);
  _inp8(0x3c6);
  _inp8(0x3c6);
  _inp8(0x3c6);
  _inp8(0x3c6);
  return(_inp8(0x3c6));
}

static void et4000_save_regs(VESACHAR regs[72])
{
  int i;

  /* save extended CRT registers */
  for (i = 0; i < 6; i++)
    {
      _outp8(CRT_I, 0x32+i);
      regs[EXT+i] = _inp8(CRT_D);
    }

  /* save extended sequencer register */
  _outp8(SEQ_I, 7);
  regs[EXT+6] = _inp8(SEQ_D);

  /* save some other ET4000 specific registers */
  regs[EXT+7] = _inp8(0x3c3);
  regs[EXT+8] = _inp8(0x3cd);

  /* save extended attribute register */
  _inp8(IS1_R);    /* reset flip flop */
  _outp8(ATT_IW, 0x16);
  regs[EXT+9] = _inp8(ATT_R);
  _outp8(CRT_I, 0x3f);
  regs[EXT+10] = _inp8(CRT_D);
}

static void saveregs(VESACHAR regs[72])
{
  int i;

  /* save VGA registers */
  for (i = 0; i < CRT_C; i++)
    {
      _outp8(CRT_I, i);
      regs[CRT+i] = _inp8(CRT_D);
    }
  for (i = 0; i < ATT_C; i++)
    {
      _inp8(IS1_R);
      _outp8(ATT_IW, i);
      regs[ATT+i] = _inp8(ATT_R);
    }
  for (i = 0; i < GRA_C; i++)
    {
      _outp8(GRA_I, i);
      regs[GRA+i] = _inp8(GRA_D);
    }
  for (i = 0; i < SEQ_C; i++)
    {
      _outp8(SEQ_I, i);
      regs[SEQ+i] = _inp8(SEQ_D);
    }
  regs[MIS] = _inp8(MIS_R);

  et4000_save_regs(regs);
  regs[71] = getdac();
}

static VESABOOL dumpmode(FILE *file, VESAWORD mode)
{
  VESAMODEINFO info;
  VESACHAR     regs[72];

  if (!VesaSetMode(mode))
    {
      fprintf(stderr, "cannot activate mode 0x%x\n", mode);
      return(FALSE);
    }
  if (mode == 0x03)
    {
      memset(&info, 0, sizeof(info));
      info.ModeAttributes       = 0xf;
      info.WindowAAttributes    = 0x7;
      info.WindowGranularity    = 64;
      info.WindowSize           = 64;
      info.BytesPerScanline     = 160;
      info.Width                = 80;
      info.Height               = 25;
      info.CharacterWidth       = 8;
      info.CharacterHeight      = 16;
      info.NumberOfBitsPerPixel = 4;
    }
  else
    {
      if (!VesaGetModeInfo(mode, &info))
        {
          fprintf(stderr, "VesaGetModeInfo failed, mode = 0x%x\n", mode);
          return(FALSE);
        }
      if ((info.ModeAttributes & 1) == 0)
        {
          fprintf(stderr, "mode 0x%x is said to be \"not supported\"\n", mode);
          fprintf(stderr, "but in fact, it seems to be supported...\n");
          info.ModeAttributes |= 1;
        }
    }
  saveregs(regs);
  if ((fwrite(&mode, sizeof(mode), 1, file) != 1) ||
      (fwrite(&info, sizeof(info), 1, file) != 1) ||
      (fwrite(regs,  sizeof(regs), 1, file) != 1))
    {
      VesaSetMode(0x03);
      fprintf(stderr, "error writing vesamode.dat\n");
      exit(1);
    }
  return(TRUE);
}

void main(void)
{
  VESAWORD count;
  VESAINFO info;
  FILE     *file;

  _portaccess(ATT_IW, IS1_R);

  if (!et4000_test())
    {
      fprintf(stderr, "No ET4000 detected\n");
      exit(1);
    }
  if (!VesaGetInfo(&info))
    {
      fprintf(stderr, "VesaGetInfo failed\n");
      exit(1);
    }
  if ((file = fopen("vesamode.dat", "wb")) == NULL)
    {
      fprintf(stderr, "cannot create vesamode.dat\n");
      exit(1);
    }
  count = 0;
  if (fwrite(&count, sizeof(count), 1, file) != 1)
    {
      fprintf(stderr, "error writing vesamode.dat\n");
      exit(1);
    }
  if (!dumpmode(file, 0x03) ||
      !dumpmode(file, 0x12) ||
      !dumpmode(file, 0x13))
    {
      fprintf(stderr, "fatal: could not activate standard modes\n");
      exit(1);
    }
  count = 3;
  while (*info.Modes != 0xffff)
    {
      if ((*info.Modes != 0x03) &&
          (*info.Modes != 0x12) &&
          (*info.Modes != 0x13) &&
          dumpmode(file, *info.Modes))
        count++;
      info.Modes++;
    }
  VesaSetMode(0x03);
  fseek(file, 0, SEEK_SET);
  if (fwrite(&count, sizeof(count), 1, file) != 1)
    {
      fprintf(stderr, "error writing vesamode.dat\n");
      exit(1);
    }
  fclose(file);
  exit(0);
}
