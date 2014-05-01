#if !defined (_VESA_H)
#define _VESA_H

#if defined (__cplusplus)
extern "C" {
#endif

#ifndef FALSE
  #define FALSE 0
#endif

#ifndef TRUE
  #define TRUE 1
#endif

#define VESA_ModeSupported     1
#define VESA_Block2Info        2
#define VESA_BIOSSupport       4
#define VESA_ColorMode         8
#define VESA_GrafMode         16

#define VESA_Text_Model        0
#define VESA_CGA_Model         1
#define VESA_Hercules_Model    2
#define VESA_Plane_Model       3
#define VESA_PackedPixel_Model 4
#define VESA_NonChain4_Model   5
#define VESA_DirectColor_Model 6
#define VESA_YUV_Model         7

typedef unsigned char  VESACHAR, *PVESACHAR;
typedef __const__ unsigned char *PCVESACHAR;
typedef unsigned short VESAWORD, *PVESAWORD;
typedef int            VESABOOL;
typedef void           *VESAPTR;

typedef VESABOOL (*VESASET)(VESACHAR Window, VESAWORD Address);
typedef VESABOOL (*VESAGET)(VESACHAR Window, PVESAWORD Address);
typedef void  (*VESASWITCH)(int Restore);

typedef struct
  {
    VESAWORD Version;
    VESACHAR *OEMName;
    VESAWORD *Modes;
    VESAWORD MemoryBanks;
  } VESAINFO, *PVESAINFO;

typedef struct
  {
    VESAWORD ModeAttributes;
    VESACHAR WindowAAttributes;
    VESACHAR WindowBAttributes;
    VESAWORD WindowGranularity;
    VESAWORD WindowSize;
    VESAPTR  WindowAStart;
    VESAPTR  WindowBStart;
    VESASET  SetWindowFunc;
    VESAGET  GetWindowFunc;
    VESAWORD BytesPerScanline;
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
    /* ... */
    VESACHAR RedMaskSize;
    VESACHAR RedMaskPosition;
    VESACHAR GreenMaskSize;
    VESACHAR GreenMaskPosition;
    VESACHAR BlueMaskSize;
    VESACHAR BlueMaskPosition;
    VESACHAR ReservedMaskSize;
    VESACHAR ReservedMaskPosition;
  } VESAMODEINFO, *PVESAMODEINFO;

VESABOOL VesaInitialize(void);

extern VESABOOL (*VesaGetInfo)(PVESAINFO pVesaInfo);
extern VESABOOL (*VesaGetModeInfo)(VESAWORD Mode, PVESAMODEINFO pVesaModeInfo);

extern VESABOOL (*VesaSetMode)(VESAWORD Mode);
extern VESABOOL (*VesaGetMode)(PVESAWORD Mode);

extern VESABOOL (*VesaSetWindow)(VESACHAR Window, VESAWORD Address);
extern VESABOOL (*VesaGetWindow)(VESACHAR Window, PVESAWORD Address);

extern VESABOOL (*VesaSetScanlineLength)(VESAWORD NumberOfPixels);
extern VESABOOL (*VesaGetScanlineLength)(PVESAWORD BytesPerScanline,
                                         PVESAWORD NumberOfPixels,
                                         PVESAWORD NumberOfScanlines);

extern VESABOOL (*VesaSetSwitchFunc)(VESASWITCH SwitchFunc);
extern VESABOOL (*VesaLockScreen)   (VESACHAR wait);
extern VESABOOL (*VesaUnlockScreen) (void);
extern VESABOOL (*VesaSetModified)  (void);

extern VESABOOL (*VesaSetPalette)(VESAWORD   StartIndex,
                                  VESAWORD   NumberOfEntries,
                                  PCVESACHAR Palette,
                                  VESABOOL   Wait);
extern VESABOOL (*VesaGetPalette)(VESAWORD  StartIndex,
                                  VESAWORD  NumberOfEntries,
                                  PVESACHAR Palette);

extern VESABOOL (*VesaGetCharacter)(PVESACHAR pchar);
extern VESABOOL (*VesaKeyboardHit)(void);

extern VESABOOL (*MouseOpen)(void);
extern VESABOOL (*MouseClose)(void);
extern VESABOOL (*MouseShow)(void);
extern VESABOOL (*MouseHide)(void);
extern VESABOOL (*MouseSetPosition)(VESAWORD x, VESAWORD y);
extern VESABOOL (*MouseGetPosition)(PVESAWORD x, PVESAWORD y);
extern VESABOOL (*MouseGetEvent)(PVESAWORD x, PVESAWORD y, PVESAWORD status, VESABOOL wait);

#if defined (__cplusplus)
}
#endif

#endif /* !defined (_VESA_H) */
