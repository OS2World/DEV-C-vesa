VESABOOL OS2VesaGetInfo(PVESAINFO pVesaInfo);
VESABOOL OS2VesaGetModeInfo(VESAWORD Mode, PVESAMODEINFO pVesaModeInfo);

VESABOOL OS2VesaSetMode(VESAWORD Mode);
VESABOOL OS2VesaGetMode(PVESAWORD Mode);

VESABOOL OS2VesaSetWindow(VESACHAR Window, VESAWORD Address);
VESABOOL OS2VesaGetWindow(VESACHAR Window, PVESAWORD Address);

VESABOOL OS2VesaSetScanlineLength(VESAWORD NumberOfPixels);
VESABOOL OS2VesaGetScanlineLength(PVESAWORD BytesPerScanline,
                                  PVESAWORD NumberOfPixels,
                                  PVESAWORD NumberOfScanlines);

VESABOOL OS2VesaSetSwitchFunc(VESASWITCH SwitchFunc);
VESABOOL OS2VesaLockScreen   (VESACHAR wait);
VESABOOL OS2VesaUnlockScreen (void);
VESABOOL OS2VesaSetModified  (void);

VESABOOL OS2VesaSetPalette(VESAWORD   StartIndex,
                           VESAWORD   NumberOfEntries,
                           PCVESACHAR Palette,
                           VESABOOL   Wait);
VESABOOL OS2VesaGetPalette(VESAWORD  StartIndex,
                           VESAWORD  NumberOfEntries,
                           PVESACHAR Palette);

VESABOOL OS2VesaGetCharacter(PVESACHAR pchar);
VESABOOL OS2VesaKeyboardHit(void);

VESABOOL OS2MouseOpen(void);
VESABOOL OS2MouseClose(void);
VESABOOL OS2MouseShow(void);
VESABOOL OS2MouseHide(void);
VESABOOL OS2MouseSetPosition(VESAWORD x, VESAWORD y);
VESABOOL OS2MouseGetPosition(PVESAWORD x, PVESAWORD y);
VESABOOL OS2MouseGetEvent(PVESAWORD x, PVESAWORD y, PVESAWORD status, VESABOOL wait);
