#define WM_NEWMODE       WM_USER + 1
#define WM_DONTUPDATE    WM_USER + 2
#define WM_UPDATE        WM_USER + 3
#define WM_FINISH        WM_USER + 4
#define WM_NEWCHAR       WM_USER + 5
#define WM_CHGPALETTE    WM_USER + 6
#define WM_SETMOUSEPOS   WM_USER + 7

#define ID_TIMER         1

#pragma pack(1)

typedef struct
  {
    HEV   hev;
    HWND  hwndGraph;
    HWND  hwndText;
    PVOID GraphBuffer;
    CHAR  Pipe1Name[25];
    CHAR  Pipe2Name[25];
    CHAR  Pipe3Name[25];
    CHAR  Pipe4Name[25];
    CHAR  ProgName[25];
    BOOL  modified;
    struct
      {
        ULONG  cbFix;
        ULONG  cx;
        ULONG  cy;
        USHORT cPlanes;
        USHORT cBitCount;
        ULONG  ulCompression;
        ULONG  cbImage;
        ULONG  cxResolution;
        ULONG  cyResolution;
        ULONG  cclrUsed;
        ULONG  cclrImportant;
        USHORT usUnits;
        USHORT usReserved;
        USHORT usRecording;
        USHORT usRendering;
        ULONG  cSize1;
        ULONG  cSize2;
        ULONG  ulColorEncoding;
        ULONG  ulIdentifier;
        RGB2   argbColor[256];
      } bmi;
    POINTL pointl[4];
    UCHAR  bits;
    USHORT mousex;
    USHORT mousey;
    USHORT mousestatus;
  } DATABUFFER;
