/* VESA package for emx/gcc --- Copyright (c) 1993 by Johannes Martin */
#include <stdio.h>
#include <stdlib.h>

#include "vesa.h"

static void print_mode(int mode)
{
  VESAMODEINFO info;
  VESACHAR     c;

  printf("-------------------------------------\n");
  if (!VesaGetModeInfo(mode, &info))
    {
      printf("VesaGetModeInfo failed, mode = %i\n", mode);
      return;
    }
  printf("Mode:                 %x\n", mode);
  printf("ModeAttributes:       %x\n", info.ModeAttributes);
  printf("WindowAAttributes:    %x\n", info.WindowAAttributes);
  printf("WindowBAttributes:    %x\n", info.WindowBAttributes);
  printf("WindowGranularity:    %i\n", info.WindowGranularity);
  printf("WindowSize:           %i\n", info.WindowSize);
  printf("WindowAStart:         %p\n", info.WindowAStart);
  printf("WindowBStart:         %p\n", info.WindowBStart);
  printf("BytesPerScanline:     %i\n", info.BytesPerScanline);
  printf("Width:                %i\n", info.Width);
  printf("Height:               %i\n", info.Height);
  printf("CharacterWidth:       %i\n", info.CharacterWidth);
  printf("CharacterHeight:      %i\n", info.CharacterHeight);
  printf("NumberOfMemoryPlanes: %i\n", info.NumberOfMemoryPlanes);
  printf("NumberOfBitsPerPixel: %i\n", info.NumberOfBitsPerPixel);
  printf("NumberOfBanks:        %i\n", info.NumberOfBanks);
  printf("MemoryModelType:      %x\n", info.MemoryModelType);
  printf("SizeOfBank:           %i\n", info.SizeOfBank);
  printf("NumberofPages:        %i\n", info.NumberofPages);
  printf("RedMaskSize:          %i\n", info.RedMaskSize);
  printf("RedMaskPosition:      %i\n", info.RedMaskPosition);
  printf("GreenMaskSize:        %i\n", info.GreenMaskSize);
  printf("GreenMaskPosition:    %i\n", info.GreenMaskPosition);
  printf("BlueMaskSize:         %i\n", info.BlueMaskSize);
  printf("BlueMaskPosition:     %i\n", info.BlueMaskPosition);
  printf("ReservedMaskSize:     %i\n", info.ReservedMaskSize);
  printf("ReservedMaskPosition: %i\n", info.ReservedMaskPosition);
  printf("Weiter...");
  VesaGetCharacter(&c);
  printf("\n");
}

void main(void)
{
  VESAINFO info;
  VESAWORD *modes;

  if (!VesaInitialize())
    {
      printf("VesaInitialize failed\n");
      return;
    }
  if (!VesaGetInfo(&info))
    {
      printf("VesaGetInfo failed\n");
      return;
    }
  printf("Version: %u.%u\n", info.Version >> 8, info.Version & 0xff);
  printf("OEM    : %s\n", info.OEMName);
  printf("Modes  : ");
  modes = info.Modes;
  while (*modes != 0xffff)
    printf("%04x, ", *modes++);
  printf("\n\n");
  modes = info.Modes;
  while (*modes != 0xffff)
    print_mode(*modes++);
}
