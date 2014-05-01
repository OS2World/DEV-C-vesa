VESABOOL DosVesaGetInfo(PVESAINFO pVesaInfo);
VESABOOL DosVesaGetModeInfo(VESAWORD Mode, PVESAMODEINFO pVesaModeInfo);
VESABOOL DosVesaSetMode(VESAWORD Mode);
VESABOOL DosVesaGetMode(PVESAWORD Mode);
VESABOOL DosVesaSetWindow(VESACHAR Window, VESAWORD Address);
VESABOOL DosVesaGetWindow(VESACHAR Window, PVESAWORD Address);
VESABOOL DosVesaSetScanlineLength(VESAWORD NumberOfPixels);
VESABOOL DosVesaGetScanlineLength(PVESAWORD BytesPerScanline,
                                  PVESAWORD NumberOfPixels,
                                  PVESAWORD NumberOfScanlines);
VESABOOL DosVesaSetPalette(VESAWORD   StartIndex,
                           VESAWORD   NumberOfEntries,
                           PCVESACHAR Palette,
                           VESABOOL   Wait);
VESABOOL DosVesaGetPalette(VESAWORD  StartIndex,
                           VESAWORD  NumberOfEntries,
                           PVESACHAR Palette);
VESABOOL DosVesaGetCharacter(PVESACHAR pchar);
VESABOOL DosVesaKeyboardHit(void);

VESABOOL DosNoVesaGetInfo(PVESAINFO pVesaInfo);
VESABOOL DosNoVesaGetModeInfo(VESAWORD Mode, PVESAMODEINFO pVesaModeInfo);
VESABOOL DosNoVesaSetMode(VESAWORD Mode);
VESABOOL DosNoVesaGetMode(PVESAWORD Mode);
VESABOOL DosNoVesaSetWindow(VESACHAR Window, VESAWORD Address);
VESABOOL DosNoVesaGetWindow(VESACHAR Window, PVESAWORD Address);
VESABOOL DosNoVesaSetScanlineLength(VESAWORD NumberOfPixels);
VESABOOL DosNoVesaGetScanlineLength(PVESAWORD BytesPerScanline,
                                    PVESAWORD NumberOfPixels,
                                    PVESAWORD NumberOfScanlines);
VESABOOL DosMouseOpen(void);
VESABOOL DosMouseClose(void);
VESABOOL DosMouseShow(void);
VESABOOL DosMouseHide(void);
VESABOOL DosMouseSetPosition(VESAWORD x, VESAWORD y);
VESABOOL DosMouseGetPosition(PVESAWORD x, PVESAWORD y);
VESABOOL DosMouseGetEvent(PVESAWORD x, PVESAWORD y, PVESAWORD status, VESABOOL wait);
