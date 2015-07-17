//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* disfnc.h                                            aec    31-Dec-1992     */
/*----------------------------------------------------------------------------*/
/* Display function prototypes and utilities.                                 */
/*----------------------------------------------------------------------------*/
#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files.                                                   */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

#if !defined (LS_MAXNAME)
#define	LS_MAXNAME      64	       /* Maximum line style name length      */
#endif

#if !defined (SYM_MAXNAME)
#define	SYM_MAXNAME      256	  /* Maximum named symbology length      */
#endif

#if !defined (LYR_MAXNAME)
#define	LYR_MAXNAME      MAX_LEVEL_NAME_BYTES	  /* Maximum named symbology length      */
#endif

#if !defined (FNT_MAXNAME)
#define	FNT_MAXNAME      MAX_FONT_NAME_BYTES	  /* Maximum font length      */
#endif

#if !defined (STY_MAXNAME)
#define	STY_MAXNAME      MAX_TEXTSTYLE_NAME_LENGTH	  /* Maximum text style length      */
#endif

/*----------------------------------------------------------------------------*/
/* Data structures                                                            */
/*----------------------------------------------------------------------------*/

typedef struct CADlinpar
{
  unsigned long opt;                    /* internal use only                  */
  unsigned long opt1;                   /* for byte alignment                 */
  UINT lvl;                           /* level                              */
  UINT clr;                           /* color                              */
  UINT wgt;                           /* weight                             */
  long bitField;                        /* for bit masking fill, etc.         */
  double lineStyleScale;                /* scale for line style               */
  wchar_t lineStyle[LS_MAXNAME];        /* custom line style                  */
  wchar_t symbologyName[SYM_MAXNAME];   /* Named symbology style name         */
  //wchar_t layerName[LYR_MAXNAME];       /* Layer name                         */
  //wchar_t material[MAX_MaterialName];   /* Material name                      */
} CADlinpar;

typedef struct CADtxtpar
{
  unsigned long opt;                    /* internal use only                  */
  UINT lvl;                           /* level                              */
  UINT clr;                           /* color                              */
  UINT wgt;                           /* weight                             */
  long fnt;                             /* font                               */
  int jst;                             /* justification for single-line text */
  int justMulti;                       /* justification for multi-line text  */
  long lineLength;                      /* max length of multiline [0,255]    */
  long paperUnits;                      /* specifies units for text size      */
  unsigned long bitField;               /* fraction|vertical|underline| etc   */
  double slant;                         /* slant of text (-90,90) degrees     */
  double angle;                         /* angle of text                      */
  double txh;                           /* text height                        */
  double txw;                           /* text width                         */
  double charSpacing;                   /* intercharacter spacing             */
  double lineSpacing;                   /* line spacing                       */
  double horOffset;                     /* parallel offset                    */
  double vrtOffset;                     /* perpendicular offset               */
  wchar_t symbologyName[SYM_MAXNAME];   /* Named symbology style name         */
  //wchar_t layerName[LYR_MAXNAME];       /* Layer name                         */
  //wchar_t fontName[FNT_MAXNAME];        /* font name                          */
  //wchar_t textStyleName[STY_MAXNAME];   /* text style                         */
} CADtxtpar;
