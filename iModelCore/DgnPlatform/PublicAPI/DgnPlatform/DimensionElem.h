/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DimensionElem.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */
#include "DimensionStyleProps.r.h"

DGNPLATFORM_TYPEDEFS (DimText)
DGNPLATFORM_TYPEDEFS (DimensionElm)
DGNPLATFORM_TYPEDEFS (DimUnitBlock)
DGNPLATFORM_TYPEDEFS (DimDerivedData)

BEGIN_BENTLEY_DGN_NAMESPACE


//__PUBLISH_SECTION_END__

// Options for updating dimension with style
#define ADIM_PARAMS_CREATE                 1
#define ADIM_PARAMS_CHANGE                 2
#define ADIM_PARAMS_UPDATE_FROMDIMSTYLE    3
#define ADIM_PARAMS_UPDATE_FROMTEXTSTYLE   4
#define ADIM_PARAMS_CREATE_FROMDWG         5

/*----------------------------------------------------------------------+
|                                                                       |
| Dimension text justification options                                  |
|                                                                       |
+----------------------------------------------------------------------*/
#define DIMTEXT_OFFSET     0
#define DIMTEXT_LEFT       1
#define DIMTEXT_START      DIMTEXT_LEFT
#define DIMTEXT_CENTER     2
#define DIMTEXT_RIGHT      3
#define DIMTEXT_END        DIMTEXT_RIGHT

/*---------------------------------------------------------------------------+
|                                                                            |
|   Dimension string configuration flags                                     |
|                                                                            |
+---------------------------------------------------------------------------*/
#define DIMSTRING_DUAL           0x1   /* Dual dimensions                   */
#define DIMSTRING_TOLERANCE      0x2   /* Tolerance mode on                 */
#define DIMSTRING_LIMIT          0x4   /* Limit type tolerance              */

#define MAX_DIMSTRING             80   /* Max dimension string length       */

/*----------------------------------------------------------------------+
|                                                                       |
|   Dimension primitive part names and macros                           |
|                                                                       |
+----------------------------------------------------------------------*/
#define ADIM_GETTYPE(dimVar)                    (DimensionPartType)((Byte)((dimVar & 0x000000f0) >> 4))
#define ADIM_GETSUB(dimVar)                     (DimensionPartSubType)((Byte)(dimVar & 0x0000000f))
#define ADIM_GETSEG(dimVar)                     ((Byte)((dimVar & 0x0000ff00) >> 8))

/*----------------------------------------------------------------------+
|                                                                       |
|       Dimension Style                                                 |
|                                                                       |
+----------------------------------------------------------------------*/
#define DIMSTYLE_ApplyOverrides             0
#define DIMSTYLE_RemoveOverrides            1

#define DIMSTYLE_NoStyle                    0
#define DIMSTYLE_ByLevel                    -1

/*----------------------------------------------------------------------+
|                                                                       |
|       Note linkage                                                    |
|                                                                       |
+----------------------------------------------------------------------*/
#define MLNOTE_USERATTR_SIGNATURE          22062 /* BSI-assigned signature */

//__PUBLISH_SECTION_START__
/*=================================================================================**//**
* Describes the strings that are stored on a dimension segment.
* @Remarks      If tolerance is true, there are tolerance strings.
* @Remarks      If limit is true, the tolerance strings are upper and lower limits:
*               DIMTEXTSUBPART_Limit_Upper and DIMTEXTSUBPART_Limit_Lower.
* @Remarks      If config-&gt;limit is false, the tolerances are plus/minus values:
*               DIMTEXTSUBPART_Main, DIMTEXTSUBPART_Tolerance_Plus and DIMTEXTSUBPART_Tolerance_Minus
* @Remarks      If tolerance is false, only a single dimension string is used:
*               DIMTEXTSUBPART_Main.
* @Remarks      If dual is true, primary and secondary strings are present:
*               DIMTEXTPART_Primary and DIMTEXTPART_Secondary
* @Remarks      If dual is true, only primary strings are present:
*               DIMTEXTPART_Primary
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DimStringConfig
    {
    unsigned    dual:1;                //!< true if both primary and secondary strings are used.
    unsigned    tolerance:1;           //!< true if tolerances are used.
    unsigned    limit:1;               //!< true for limit tolerance, false for plus/minus tolerance.
    };

/*=================================================================================**//**
* Holds all the strings that can be stored on a dimension segment.  Usually
* a DimStrings object will be paired with a DimStringConfig that can be used
* to determine which strings are valid.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DimStrings
    {
private:
    WString  m_strings[6];
public:
//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT  WStringP    GetString(size_t iStr);
    WStringP            GetStrings()         {return m_strings;}
    WStringCP           GetStringsCP() const {return m_strings;}

    WString  (& GetPrimaryStrings())[6]{return m_strings;}
    WString*    GetSecondaryStrings ()   {return &m_strings[3];}
//__PUBLISH_SECTION_START__

    //! Get the string value.  Consult the paired DimStringConfig object for the valid arguments.
    DGNPLATFORM_EXPORT WStringP    GetString (DimensionTextPartType, DimensionTextPartSubType);
    };

//__PUBLISH_SECTION_END__

/*----------------------------------------------------------------------+
|                                                                       |
|       Dimension Template structure for using tool templates           |
|                                                                       |
+----------------------------------------------------------------------*/
struct DimToolTemplate
    {
#if !defined (BITFIELDS_REVERSED)
    unsigned short first_term:3;
    unsigned short left_term:3;
    unsigned short right_term:3;
    unsigned short bowtie_symbol:3;
    unsigned short pre_symbol:3;
    unsigned short stacked:1;

    unsigned short post_symbol:3;
    unsigned short above_symbol:3;
    unsigned short left_witness:1;
    unsigned short right_witness:1;
    unsigned short vertical_text:1;
    unsigned short nofit_vertical:1;
    unsigned short centermark:1;
    unsigned short centerLeft:1;
    unsigned short centerRight:1;
    unsigned short centerTop:1;
    unsigned short centerBottom:1;
    unsigned short altExt:1;
#else
    unsigned short altExt:1;
    unsigned short centerBottom:1;
    unsigned short centerTop:1;
    unsigned short centerRight:1;
    unsigned short centerLeft:1;
    unsigned short centermark:1;
    unsigned short nofit_vertical:1;
    unsigned short vertical_text:1;
    unsigned short right_witness:1;
    unsigned short left_witness:1;
    unsigned short above_symbol:3;
    unsigned short post_symbol:3;

    unsigned short stacked:1;
    unsigned short pre_symbol:3;
    unsigned short bowtie_symbol:3;
    unsigned short right_term:3;
    unsigned short left_term:3;
    unsigned short first_term:3;
#endif
    };

/*-----------------------------------------------------------------------
    Text Boxes
-----------------------------------------------------------------------*/
struct AdimRotatedTextBox
    {
    DPoint3d    baseFirstChar;
    DVec3d      baseDir, zvec;
    double      width, height;
    };

struct AdimSegmentTextBoxes
    {
    struct
        {
        unsigned short hasPrimary:1;
        unsigned short hasSecondary:1;
        unsigned short unused:14;
        }   flags;

    AdimRotatedTextBox  primary;
    AdimRotatedTextBox  secondary;
    };

/*---------------------------------------------------------------------------------------
    DimDerivedData should be used to hold data that can only be robustly derived by
    stroking a dimension.  The data structure should consist only of pointers to
    other structures, so that the strokers only collect the requested information.

    DimDerivedData is intended to be *UNPUBLISHED*, if/when we need to include this
    level of info in a public api, an opaque pointer should be published.

                                                                        JS 01/03
---------------------------------------------------------------------------------------*/
struct DimTermDirs
    {
    struct
        {
        unsigned short hasLeftTerm:1;
        unsigned short hasRightTerm:1;
        unsigned short unused:14;
        }   flags;

    DPoint3d        leftDir;
    DPoint3d        rightDir;

    };

struct DimDerivedData
    {
    unsigned short          flags;

    int                     numSegments;
    DimTermDirs            *pTermDirs;      // NULL, or provide a buffer of nSegments * sizeof (DimTermDirs)
    bool                   *pIsTextDocked;  // NULL, or provide a buffer of nSegments * sizeof (bool)
    bool                   *pIsTextOutside; // NULL, or provide a buffer of nSegments * sizeof (bool)
    DPoint3dP               pArcDefPoint;   // NULL, or provide a buffer of sizeof (DPoint3d)
    AdimSegmentTextBoxes   *pTextBoxes;     // NULL, or provide a buffer of nSegments * sizeof (AdimSegmentTextBoxes)
    uint16_t               *pChainType;     // NULL, or provide a buffer of nSegments * sizeof (UInt16)
    double                 *pDimValues;     // NULL, or provide a buffer of nSegments * sizeof (double)
    bool                   *pSuperscripted; // NULL, or provide a buffer of nSegments * sizeof (bool)
    bool                   *pDelimiterOmitted; // NULL, or provide a buffer of nSegments * sizeof (bool)
    };

#define     DIMDERIVEDDATA_FLAGS_TERMDIR            1 << 0
#define     DIMDERIVEDDATA_FLAGS_TEXTDOCKED         1 << 1
#define     DIMDERIVEDDATA_FLAGS_ARCDEFPOINT        1 << 2
#define     DIMDERIVEDDATA_FLAGS_TEXTBOX            1 << 3
#define     DIMDERIVEDDATA_FLAGS_CHAINTYPE          1 << 4
#define     DIMDERIVEDDATA_FLAGS_TEXTOUTSIDE        1 << 5
#define     DIMDERIVEDDATA_FLAGS_DIMVALUES          1 << 6
#define     DIMDERIVEDDATA_FLAGS_SUPERSCRIPTED      1 << 7
#define     DIMDERIVEDDATA_FLAGS_DELIMITEROMITTED   1 << 8

#define DIM_VERSION               11
#define MAX_ADIM_POINTS           50
#define LAST_POINT                -1

#define DIMTYPE_LINEAR_MASK   ( (1L << (int) DimensionType::SizeArrow     ) | \
                                (1L << (int) DimensionType::SizeStroke    ) | \
                                (1L << (int) DimensionType::LocateSingle  ) | \
                                (1L << (int) DimensionType::LocateStacked ) | \
                                (1L << (int) DimensionType::CustomLinear  ) )

#define DIMTYPE_ANGULAR_MASK  ( (1L << (int) DimensionType::AngleSize     ) | \
                                (1L << (int) DimensionType::ArcSize       ) | \
                                (1L << (int) DimensionType::AngleLocation ) | \
                                (1L << (int) DimensionType::ArcLocation   ) | \
                                (1L << (int) DimensionType::AngleLines    ) | \
                                (1L << (int) DimensionType::AngleAxis     ) )

#define DIMTYPE_RADIAL_MASK   ( (1L << (int) DimensionType::Radius                 ) | \
                                (1L << (int) DimensionType::Diameter               ) | \
                                (1L << (int) DimensionType::DiameterParallel       ) | \
                                (1L << (int) DimensionType::DiameterPerpendicular  ) | \
                                (1L << (int) DimensionType::RadiusExtended         ) | \
                                (1L << (int) DimensionType::DiameterExtended       ) | \
                                (1L << (int) DimensionType::Center                 ) )

#if defined (BEIJING_DGNPLATFORM_WIP_DIMENSION)
#endif

#define DIMTYPE_ORDINATE_MASK ( (1L << (int) DimensionType::Ordinate         ) )

/* Macros to classify a dimension element ex. if DIM_LINEAR (elmP->dim.dimcmd) */
#define DIM_LINEAR(type)    ((static_cast<int>(type) < 32) && (0 != (1L << (static_cast<int>(type)) & DIMTYPE_LINEAR_MASK)))
#define DIM_ANGULAR(type)   ( ((static_cast<int>(type) < 32) && (0 != (1L << (static_cast<int>(type)) & DIMTYPE_ANGULAR_MASK))) || (type == DimensionType::AngleAxisX || type == DimensionType::AngleAxisY))
#define DIM_RADIAL(type)    ((static_cast<int>(type) < 32) && (0 != (1L << (static_cast<int>(type)) & DIMTYPE_RADIAL_MASK)))
#define DIM_NOTE(type)      (type == DimensionType::Note)
#define DIM_ORDINATE(type)  (type == DimensionType::Ordinate)
#define DIM_LABELLINE(type) (type == DimensionType::LabelLine)

/*__PUBLISH_SECTION_START__*/

END_BENTLEY_DGN_NAMESPACE
