#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "vesa.h"

void main(int argc, char *argv[])
{
  VESAWORD x, y, status;
  VESACHAR c;

  if (argc != 3)
    {
      printf("Usage: moustest <mode-number> <wait flag>\n");
      printf("       where <wait flag> is - 0: do not wait for mouse events\n");
      printf("                            - 1: wait for mouse events\n");
      return;
    }
  if (!VesaSetMode(atoi(argv[1])))
    {
      VesaSetMode(3);
      printf("Could not activate mode 0x%x", atoi(argv[1]));
      return;
    }
  if (!MouseOpen())
    {
      VesaSetMode(3);
      printf("Could not initialize mouse.\n");
      return;
    }
  MouseShow();
  do
    {
      if (MouseGetEvent(&x, &y, &status, atoi(argv[2])))
        printf("  ");
      else
        printf("No");
      printf(" Mouse Event: x = %i, y = %i, status = %i (press any key, ESC to abort)\n", x, y, status);
      VesaGetCharacter(&c);
    }
  while (c != 0x1b);
  MouseHide();
  MouseClose();
}
