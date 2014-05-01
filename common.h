extern int activeindex;
extern int screenlocked;
extern VESASWITCH swfunc;

extern VESAWORD *Modes;

extern struct _ModeInfo
  {
    UCHAR        fbtype;
    VESAMODEINFO Vesa;
  } *ModeInfos;

/* functions implemented in common.c                                         */
VOID Error(char *s);             /* Print error message using OS/2 DOS-Calls */

/* functions to implement in vesa_xxx.c                                      */
VOID *GetPhysBuf(void);          /* Get access to physical display buffer    */
VESABOOL Initialize(void);       /* Initialize VESA dll                      */
VOID     ClearUp(void);          /* Prepare to exit VESA dll                 */
VESABOOL SetMode(VESAWORD mode); /* Set video mode - routine called both, if */
                                 /* program in foreground or in background   */
                                 /* don't do any register access             */
void SetSpecial(VESAWORD mode);  /* Set video mode - routine only called, if */
                                 /* program in foreground,                   */
                                 /* register access allowed                  */
