/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/MdlTextInternal.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnCore/TextParam.h>
#include "TextBlock/TextAPICommon.h"

#define MAX_TEXT_LENGTH     64000
#define MAX_LINES           65535
#define MAX_CHARS           2048
#define MAX_NEWLINES        50

#define MIN_TXT_SIZE_UORS   0.01

#define MAX_FieldSize       1024
#define MAX_FieldResultSize 1024

DGNPLATFORM_TYPEDEFS (TextLen)
DGNPLATFORM_TYPEDEFS (TextTileSize)
DGNPLATFORM_TYPEDEFS (TextMultFactor)
DGNPLATFORM_TYPEDEFS (TextNodeLinkage)
DGNPLATFORM_TYPEDEFS (TextLinkage)

DGNPLATFORM_TYPEDEFS (TextFormattingLinkage)
DGNPLATFORM_TYPEDEFS (TextAnnotationOverrideFlags)
DGNPLATFORM_TYPEDEFS (TextAnnotationLinkage)
DGNPLATFORM_TYPEDEFS (TextRenderingLinkage)
DGNPLATFORM_TYPEDEFS (IndentationLinkageData)
DGNPLATFORM_TYPEDEFS (TextIndentationLinkage)

// Enter data field delimiters
#define ED_LEFT_DELIMITER   0x3c
#define ED_RIGHT_DELIMITER  0x3e

// Text placement modes
#define PLACETEXT       1
#define FITTEXT         2
#define PLACEVITEXT     3
#define FITVITEXT       4
#define PLACENODE       5
#define PLACEVINODE     6
#define MODIFYTEXT      7
#define INCLUDEFILE     8
#define ABOVETEXT       9
#define BELOWTEXT       10
#define ONTEXT          11
#define MATRIXTEXT      12
#define MATRIXNODE      13
#define ALONGTEXT       14
#define ENTERSINGLE     15
#define ENTERAUTO       16
#define PLACENOTE       17
#define COPYEDFIELD     18
#define COPYINCREDFIELD 19
#define EDITTEXT        20
#define WORDWRAPTEXT    21
#define WORDWRAPVITEXT  22

// Dependancy linkage app values
#define TEXTDEPENDENCYAPPVALUE_AlongElement 1

// Hook ids to resolve functions in textedit.ma from mdltext.c
#define HOOKITEMID_TextEditMDL_EditCommand  0
#define HOOKITEMID_TextEditMDL_SetupEditor  1
#define HOOKITEMID_TextEditMDL_Keyin        2

// User messages from textset.c and userpref.c to text editors/tools
#define TEXTEDIT_USERMSG_FONTCHANGED        1
#define TEXTEDIT_USERMSG_EDITORSTYLECHANGED 2
#define TEXTEDIT_USERMSG_GETTEXTFIELD       3
#define TEXTEDIT_USERMSG_SETTEXTFIELD       4
#define TEXTEDIT_USERMSG_RESETTOSTYLE       5
#define TEXTEDIT_USERMSG_LOCKCHANGED        6

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* Values for text whitespace bitmask linkage.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum WhiteSpaceBitmaskValue
    {
    WhiteSpaceBitmaskValue_Tab              = 0,
    WhiteSpaceBitmaskValue_ParagraphBreak   = 1,
    WhiteSpaceBitmaskValue_LineBreak        = 2

    }; // WhitespaceBitmaskValue

END_BENTLEY_DGNPLATFORM_NAMESPACE

#if !defined (__midl) /* During MIDL compile, all we care about are the values of the #define constants. */

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TextLen
    {
    double  w;
    double  h;

    }; //  TextLen

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TextTileSize
    {
    double  w;
    double  h;

    }; // TextTileSize

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TextMultFactor
    {
    double  w;
    double  h;

    }; // TextMultFactor

/*=================================================================================**//**
* Holds the bulk of a TextParamWide... other sources include the element header, TextAnnotationLinkage, and TextRenderingLinkage.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TextFormattingLinkage
    {
    LinkageHeader   linkHeader;
    TextDrawFlags   flags;
    TextExFlags     exFlags;
    byte            textData[300];

    public:     /* DGNHANDLERS */   static  UInt16  GetLinkageID                ();
    public:     /* DGNHANDLERS */   static  bool    DoesElementHaveThisLinkage  (ElementHandleCR);
    public:     /* DGNHANDLERS */   static  void    FillTextParamsFromLinkage   (ElementHandleCR, TextParamWideR);
    public:     /* DGNHANDLERS */           void    FillTextParams              (TextParamWideR) const;
    public:     /* DGNHANDLERS */   static  void    AppendLinkageFromTextParams (EditElementHandleR, TextParamWideCR);
    public:     /* DGNHANDLERS */   static  void    AppendLinkageFromTextParams (DgnElementR, TextParamWideCR);
    public:     /* DGNHANDLERS */           void    InitializeFromTextParams    (TextParamWideCR);
    public:     /* DGNHANDLERS */   static  void    RemoveLinkages              (EditElementHandleR);
    public:     /* DGNHANDLERS */   static  void    RemoveLinkages              (DgnElementR);

    }; // TextFormattingLinkage

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TextAnnotationOverrideFlags
    {
    UInt16  width               :1;
    UInt16  height              :1;
    UInt16  linespacing         :1;
    UInt16  interCharSpacing    :1;
    UInt16  underlineOffset     :1;
    UInt16  overlineOffset      :1;
    UInt16  lineOffset          :1;
    UInt16  backgroundborder    :1;
    UInt16  reserved            :8;

    }; // TextAnnotationOverrideFlags

/*=================================================================================**//**
* Holds part of a TextParamWide... other sources include the element header, TextFormattingLinkage, and TextRenderingLinkage.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TextAnnotationLinkage
    {
    LinkageHeader               linkHeader;
    double                      annotationScale;
    TextAnnotationOverrideFlags flags;

    public: /* DGNHANDLERS */   static  UInt16  GetLinkageID                ();
    public: /* DGNHANDLERS */   static  bool    DoesElementHaveThisLinkage  (ElementHandleCR);
    public: /* DGNHANDLERS */   static  void    FillTextParamsFromLinkage   (ElementHandleCR, TextParamWideR);
    public: /* DGNHANDLERS */   static  void    AppendLinkageFromTextParams (EditElementHandleR, TextParamWideCR);
    public: /* DGNHANDLERS */   static  void    AppendLinkageFromTextParams (DgnElementR, TextParamWideCR);
    public: /* DGNHANDLERS */   static  void    RemoveLinkages              (EditElementHandleR);
    public: /* DGNHANDLERS */   static  void    RemoveLinkages              (DgnElementR);

    }; // TextAnnotationLinkage

/*=================================================================================**//**
* Holds part of a TextParamWide... other sources include the element header, TextFormattingLinkage, and TextAnnotationLinkage.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TextRenderingLinkage
    {
    LinkageHeader       linkHeader;
    TextRenderingFlags  flags;

    public: /* DGNHANDLERS */   static  UInt16  GetLinkageID                ();
    public: /* DGNHANDLERS */   static  bool    DoesElementHaveThisLinkage  (ElementHandleCR);
    public: /* DGNHANDLERS */   static  void    FillTextParamsFromLinkage   (ElementHandleCR, TextParamWideR);
    public: /* DGNHANDLERS */   static  void    AppendLinkageFromTextParams (EditElementHandleR, TextParamWideCR);
    public: /* DGNHANDLERS */   static  void    AppendLinkageFromTextParams (DgnElementR, TextParamWideCR);
    public: /* DGNHANDLERS */   static  void    RemoveLinkages              (EditElementHandleR);
    public: /* DGNHANDLERS */   static  void    RemoveLinkages              (DgnElementR);

    }; // TextRenderingLinkage

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IndentationLinkageData
    {
    double  firstLineIndent;
    double  paragraphIndent;
    int     tabCount;
    double  tabStops[1];

    }; // IndentationLinkageData

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TextIndentationLinkage
    {
    LinkageHeader           linkHeader;
    IndentationLinkageData  indentationData;

    public: /* DGNHANDLERS */   static  UInt16  GetLinkageID                        ();
    public: /* DGNHANDLERS */   static  bool    DoesElementHaveThisLinkage          (ElementHandleCR);
    public: /* DGNHANDLERS */   static  void    FillIndentationDataFromLinkage      (ElementHandleCR, IndentationDataR);
    public: /* DGNHANDLERS */   static  void    AppendLinkageFromIndentationData    (EditElementHandleR, IndentationDataCR);
    public: /* DGNHANDLERS */   static  void    RemoveLinkages                      (EditElementHandleR);

    }; // TextIndentationLinkage

END_BENTLEY_DGNPLATFORM_NAMESPACE

BEGIN_BENTLEY_API_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT UShort mdlText_getMarginTextJustification (UInt32 justification);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT UShort mdlText_constrainTextJustification (int justification);

END_BENTLEY_API_NAMESPACE

#endif // !defined (__midl)
