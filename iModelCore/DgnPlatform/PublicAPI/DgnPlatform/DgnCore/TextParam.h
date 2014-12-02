/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/TextParam.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <Bentley/Bentley.h>
#include "../DgnPlatform.r.h"

enum
{
    TXTFONT_NORMAL      = 0,       //!< Text Symbol Type: Normal font
    TXTFONT_SYMBOL      = 1,       //!< Text Symbol Type: Symbol font

    TXTFONT_BYTEVEC     = 1,       //!< Text Vector Size: 8-bits used for bvector
    TXTFONT_WORDVEC     = 2,       //!< Text Vector Size: 16-bits used for bvector

    TXTFONT_BYTEREP     = 0,       //!< Text Representation Size: 8-bits represents font
    TXTFONT_WORDREP     = 1,       //!< Text Representation Size: 16-bits represents font

    TXTFONT_VECTOR      = 0,       //!< Text Type: Vector font
    TXTFONT_RASTER      = 1,       //!< Text Type: Raster font

    TXTDIR_HORIZONTAL   = 0x0,     //!< Text Draw Direction: normal horizontal left-to-right
    TXTDIR_VERTICAL     = 0x4,     //!< Text Draw Direction: a string vertical
    TXTDIR_RIGHTLEFT    = 0x8,     //!< Text Draw Direction: a string right to left
    TXTDIR_UPSIDEDOWN   = 0x10,    //!< Text Draw Direction: text upside down
    TXTDIR_BACKWARDS    = 0x20,    //!< Text Draw Direction: text backwards

    TXTFIT_VARIABLESIZE = 0,       //!< Text Fitting Mode
    TXTFIT_FIXEDSIZE    = 1,       //!< Text Fitting Mode
};

//__PUBLISH_SECTION_END__

#define ACAD_ATLEASTLINESPACING_FACTOR (2.0 / 3.0)

//__PUBLISH_SECTION_START__

DGNPLATFORM_TYPEDEFS (TextSizeParam)
DGNPLATFORM_TYPEDEFS (TextEDField)
DGNPLATFORM_TYPEDEFS (TextStyleInfo)
DGNPLATFORM_TYPEDEFS (TextEDParam)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Justification types for MicroStation text (elements and element-targeted APIs).
//! Unless otherwise noted, the justification is based on a cell box, but without right-side bearing (the "justification range").
//! @note 'Center' is used for horizontal midpoint; 'Middle' is used for vertical midpoint.
//! @note Margin justification is deprecated and support will be removed at some point; use word wrap length instead to achieve the same effect
//! @note Descender justifications are supported for DWG
//! @note Cap justification is no longer supported, and will act like its non-cap equivalent; kept so old data can be interpreted
// @bsiclass                                                    BentleySystems
//=======================================================================================
enum class  TextElementJustification
    {
    LeftTop                 = 0,    //!< Left Top
    LeftMiddle              = 1,    //!< Left Middle
    LeftBaseline            = 2,    //!< Left Bottom (baseline)
    LeftDescender           = 16,   //!< Left Descender
    CenterTop               = 6,    //!< Center Top
    CenterMiddle            = 7,    //!< Center Middle
    CenterBaseline          = 8,    //!< Center Bottom (baseline)
    CenterDescender         = 20,   //!< Center Descender
    RightTop                = 12,   //!< Right Top
    RightMiddle             = 13,   //!< Right Middle
    RightBaseline           = 14,   //!< Right Bottom (baseline)
    RightDescender          = 24,   //!< Right Descender
    
    LeftMarginTop           = 3,    //!< (deprecated) Left Margin Top
    LeftMarginMiddle        = 4,    //!< (deprecated) Left Margin Middle
    LeftMarginBaseline      = 5,    //!< (deprecated) Left Margin Bottom
    LeftMarginDescender     = 18,   //!< (deprecated) Left Margin Descender
    RightMarginTop          = 9,    //!< (deprecated) Right Margin Top
    RightMarginMiddle       = 10,   //!< (deprecated) Right Margin Middle
    RightMarginBaseline     = 11,   //!< (deprecated) Right Margin Bottom
    RightMarginDescender    = 22,   //!< (deprecated) Right Margin Descender
    
    LeftCap                 = 15,   //!< (not supported) Left Cap
    LeftMarginCap           = 17,   //!< (not supported) Left Margin Cap
    CenterCap               = 19,   //!< (not supported) Center Cap
    RightCap                = 23,   //!< (not supported) Right Cap
    RightMarginCap          = 21,   //!< (not supported) Right Margin Cap
    
    Invalid                 = 127   //!< (not supported) Can be used internally as a 'none' state, but don't pass around expecting anything to act in a predictable way
    }; // TextElementJustification

//=======================================================================================
//! Line spacing types for MicroStation text elements.
// @bsiclass
//=======================================================================================
enum class DgnLineSpacingType
    {
    Exact             = 0,    //!< Displacement of subsequent line is the existing line's max cell box displacement above origin + value
    Automatic         = 1,    //!< DO NOT USE; same as exact - exists to interpret old data
    ExactFromLineTop  = 2,    //!< Emulates the DWG "exactly" / "exact from line top" line spacing type
    AtLeast           = 3     //!< Emulates the DWG "at least" line spacing type

    }; // DgnLineSpacingType

//=======================================================================================
//! Character spacing types for MicroStation text elements.
// @bsiclass                                                    BentleySystems   07/2007
//=======================================================================================
enum class CharacterSpacingType
    {
    Absolute                = 0,    //!< Treat value as an absolute distance beyond the normal pen movement
    FixedWidth              = 1,    //!< Treat value as an absolute distance from one glyph origin to the next (ignores normal pen movement)
    Factor                  = 2     //!< Treat (value * text height) as an absolute distance beyond the normal pen movement
    }; // CharacterSpacingType

//=======================================================================================
//! Describes what kind of a fraction to display.
// @bsiclass                                                    Venkat.Kalyan   07/07
//=======================================================================================
enum class StackedFractionType
    {
    None                    = 0,    //!< Not part of a fraction. DO NOT USE when acutally specifying a fraction. It is used as an internal none/unitialized state; you could use it for similar, but don't pass around expecting anything predictable to happen
    NoBar                   = 1,    //!< A numerator simply stacked on top of a denominator; they are left-aligned, and are NOT separated by a horizontal line
    DiagonalBar             = 2,    //!< A diagonal stacked fraction; the numerator and denominator are horizontally shifted, and separated by a diagonal line
    HorizontalBar           = 3     //!< A horizontal stacked fraction; the numerator and denominator are centered, and separated by a horizontal line
    }; // StackedFractionType

//=======================================================================================
//! Describes which part of the fraction a run belongs to.
// @bsiclass
//=======================================================================================
enum class StackedFractionSection
    {
    None                    = 0,    //!< Not part of a fraction
    Numerator               = 1,    //!< The top (numerator)
    Denominator             = 2     //!< The bottom (denominator)
    }; // StackedFractionSection

#if !defined(__midl) // For a MIDL compile, all we care about are the values of the #define constants.

//=======================================================================================
//! A subset of TextParamWide flags.
// unused  => ?
// unused2 => reverseMline
// unused3 => exText
// @bsiclass
//=======================================================================================
struct TextDrawFlags
    {
    UShort interCharSpacing         :1; //!< Whether to honor character spacing values
    UShort fixedWidthSpacing        :1; //!< Use fixed character spacing
    UShort underline                :1; //!< Display an underline
    UShort slant                    :1; //!< Display as italics
    UShort vertical                 :1; //!< Draw from top-down; note that this does not also rotate glyphs (and hence is inadequate for @ TrueType fonts)
    UShort rightToLeft_deprecated   :1; //!< (deprecated) This value has been deprecated, but should be maintained when writing to the file for previous versions. This flag is no longer honored when drawing; text services analyzes the string and re-orders as appropriate based on context.
    UShort unused2                  :1; //!< (deprecated)
    UShort unused                   :1; //!< (deprecated)
    UShort offset                   :1; //!< Whether to honor run offset values
    UShort codePage_deprecated      :1; //!< (deprecated) This value has been deprecated, but should be maintained when writing to the file for previous versions. Any code that needs to rely on a proper code page should call methods like DgnFontManager::GetEffectiveCodePage, and RunPropertiesBase::GetFontForCodePage.
    UShort shxBigFont               :1; //!< Whether to honor big font ID value
    UShort bgColor                  :1; //!< Show the background and border
    UShort subscript                :1; //!< Scale and shift so as to look like subscript
    UShort superscript              :1; //!< Scale and shift to as to look superscipt
    UShort unused3                  :1; //!< (deprecated)
    UShort textStyle                :1; //!< Whether to honor the text style ID value

    //! True if any non-deprecated values are non-zero; false otherwise.
    public: DGNPLATFORM_EXPORT bool AreAnyFlagsSet () const;

    }; // TextDrawFlags

//=======================================================================================
//! A subset of TextParamWide flags.
// @bsiclass
//=======================================================================================
struct TextExFlags
    {
    UInt32 overline                 :1; //!< Draw an overline
    UInt32 stackedFractionSection   :2; //!< Identifies if this run is part of a fraction, and whether it's the numerator or denominator
    UInt32 stackedFractionAlign     :2; //!< If this run is a fraction, how it aligns to other runs in the line
    UInt32 stackedFractionType      :2; //!< If this run is a fraction, the type (e.g. stacked, stacked and centered, or diagonal)
    UInt32 crCount                  :8; //!< How many carriage returns occur after this run
    UInt32 bold                     :1; //!< Display as bold
    UInt32 underlineStyle           :1; //!< Whether to honor underline style settings (color/line style/weight)
    UInt32 overlineStyle            :1; //!< Whether to honor overline style settings (color/line style/weight)
    UInt32 styleOverrides           :1; //!< Whether any properties in the whole of TextParamWide override the associated text style
    UInt32 wordWrapTextNode         :1; //!< Whether to honor the word wrap length (not line length)
    UInt32 fullJustification        :1; //!< Use full justification (space out words such that each line effectively occupies the same width)
    UInt32 color                    :1; //!< Whether to use the color specified in TextParamWide (vs. the color on the display header)
    UInt32 acadInterCharSpacing     :1; //!< Use factor character spacing (for DWG support)
    UInt32 acadLineSpacingType      :2; //!< The line spacing type to use (can be exact/automatic (same, DGN), at-least (DWG), and exactly(DWG))
    UInt32 backwards                :1; //!< Mirrors this run horizontally; note that layout support is needed to achieve full effect (i.e. you can't simply toggle this value)
    UInt32 upsidedown               :1; //!< Mirrors this run vertically; note that layout support is needed to achieve full effect (i.e. you can't simply toggle this value)
    UInt32 acadFittedText           :1; //!< Not supported
    UInt32 annotationScale          :1; //!< Whether to honor the annotation scale value
    UInt32 renderPercentsAsMText    :1; //!< Whether to process % character-based escape sequences
    UInt32 bitMaskContainsTabCRLF   :1; //!< Whether the whitespace bitmask linkage is 1 or 2 bits-per value (e.g. whether each value is CR/LF, or CR/LF/tab)
    UInt32 isField                  :1; //!< Whether this run is part of a field (affects display; field data is on another linkage)

    //! True if any non-deprecated values are non-zero; false otherwise.
    public: DGNPLATFORM_EXPORT bool AreAnyFlagsSet () const;

    }; // TextExFlags

//=======================================================================================
//! A subset of TextParamWide flags.
// @bsiclass
//=======================================================================================
struct TextRenderingFlags
    {
    UInt32 alignEdge        :1;     //!< Ignore left-side bearing
    UInt32 lineAlignment    :2;     //!< How this run aligns to other runs in the line (assuming runs of heterogeneous sizes)
    UInt32 documentType     :2;     //!< Determines if this element should follow DGN, DText, or MText rules
    UInt32 unused           :27;    //!< Reserved for future use

    }; // TextRenderingFlags

//=======================================================================================
//! Legacy structure; do not add new APIs that use this, and aggressively replace when found. Use DVec2d or DPoint2d instead.
// @bsiclass
//=======================================================================================
typedef struct mstextsize
    {
    double  width;
    double  height;

    } MSTextSize;

typedef MSTextSize TextSize;

//=======================================================================================
//! Legacy structure; do not add new APIs that use this, and aggressively replace when found. Use DVec2d or DPoint2d instead.
// @bsiclass
//=======================================================================================
struct TextSizeParam
    {
    int         mode;
    MSTextSize  size;
    double      aspectRatio;

    }; // TextSizeParam

//=======================================================================================
//! Legacy structure; do not add new APIs that use this, and aggresively replace when found. If possible, use TextStringProperties or RunProperties; could also consider TextParamWide.
// @bsiclass
//=======================================================================================
typedef struct textParam
    {
    UInt32  font;
    int     just;
    int     lineStyle_deprecated;
    int     viewIndependent;

    } TextParam;

//=======================================================================================
//! File-based structure that stores most text parameters (e.g. those not on the element structure itself).
//!   This structure should be considered deprecated; if possible, use TextStringProperties or RunProperties instead.
// @bsiclass
//=======================================================================================
struct TextParamWide
    {
    UInt32                  font;                   //!< Primary font number
    int                     just;                   //!< Justification (see TextElementJustification)
    int                     lineStyle_deprecated;   //!< Line style ID has not been honored since 8.5, and is no longer even persisted (was 'style')
    int                     viewIndependent;        //!< View independent
    int                     codePage_deprecated;    //!< This value has been deprecated in 8.11, but should be maintained for previous versions. Any code that needs to rely on a proper code page should call DgnFontManager::GetEffectiveCodePage. The code page value here is NOT necessarily the encoding of the text in the element; rather it is the code page that was originally used to create the text.
    UInt32                  nodeNumber;             //!< Unique number associated with a text node (for textnode only)
    UInt32                  shxBigFont;             //!< Big font number (DWG compatibility)
    UInt32                  backgroundFillColor;    //!< Fill color if background is used
    UInt32                  textStyleId;            //!< Text style ID (0 implies no style)
    UInt32                  color;                  //!< Color ID
    double                  slant;                  //!< A custom slant angle, in radians
    double                  lineSpacing;            //!< Line spacing (see DgnLineSpacingType)
    double                  characterSpacing;       //!< Character spacing value; see also CharacterSpacingType
    double                  underlineSpacing;       //!< Offset, in UORs, of the underline
    double                  textnodeWordWrapLength; //!< Length, in UOR's, that a piece of text will wrap to
    TextDrawFlags           flags;                  //!< See TextDrawFlags
    TextExFlags             exFlags;                //!< See TextExFlags
    DPoint2d                lineOffset;             //!< Offset from baseline; also known as run offset
    DPoint2d                backgroundBorder;       //!< Background border offset, in UORs
    Int64                   assocId;                //!< Used to track the dependent element for along-text
    UInt32                  backgroundColor;        //!< Background border color ID
    Int32                   backgroundStyle;        //!< Background border line style ID
    UInt32                  backgroundWeight;       //!< Background border weight
    UInt32                  underlineColor;         //!< Underline color ID
    Int32                   underlineStyle;         //!< Underline line style ID
    UInt32                  underlineWeight;        //!< Underline weight
    UInt32                  overlineColor;          //!< Overline color ID
    Int32                   overlineStyle;          //!< Overline line style ID
    UInt32                  overlineWeight;         //!< Overline weight
    double                  overlineSpacing;        //!< Offset, in UORs, of the overline
    LegacyTextStyleOverrideFlags  overridesFromStyle;     //!< Notes which properties this element overrides from its text style
    double                  widthFactor;            //!< Text width factor
    double                  annotationScale;        //!< Annotation scale factor
    TextRenderingFlags      renderingFlags;         //!< See TextRenderingFlags

    //! Initializes this structure. This essentially means it is zeroed, with the exception of scale values, which are set to 1.0.
    public: DGNPLATFORM_EXPORT TextParamWide ();

    //! Initializes this structure. This essentially means it is zeroed, with the exception of scale values, which are set to 1.0.
    public: DGNPLATFORM_EXPORT void Initialize ();

    //! Applies the provided scale factor to all applicable length and offset values.
    //! @note   Because TextParamWide is compressed when written to the file (e.g. values are only written if non-default), attempting to scale may require a size change, which is not valid for every caller; use allowSizeChange to limit this.
    public: DGNPLATFORM_EXPORT bool ApplyScaleFactor (double uniformScaleFactor, bool isTextNode, bool allowSizeChange);

    //! Applies the provided scale factor to all applicable length and offset values.
    //! @note   Because TextParamWide is compressed when written to the file (e.g. values are only written if non-default), attempting to scale may require a size change, which is not valid for every caller; use allowSizeChange to limit this.
    public: DGNPLATFORM_EXPORT bool ApplyScaleFactor (DPoint2dCR scaleFactor, bool isTextNode, bool allowSizeChange);

    //! Sets the code page value of the given TextParmWide structure, and sets the code page flag to true if the code page value is non-zero (false otherwise).
    //! @note   The code page in the TextParamWide structure is deprecated in 8.11, but should still be maintained for backwards compatibility. This code page value is not actually the code page that the text is stored as in the element, but rather the keyboard code page used to create the text. In other words, this is the original locale code page. As noted, please continue to set this value for backwards compatibility, but if you think you must read this value for dealing with the text of the element, see DgnFontManager::GetEffectiveCodePage.
    public: DGNPLATFORM_EXPORT void SetCodePage (int codePage);

    /*__PUBLISH_SECTION_END__*/

    //! Gets the font most appropriate for determining character encoding.
    public: DGNPLATFORM_EXPORT DgnFontCR GetFontForCodePage (DgnProjectR);

    /*__PUBLISH_SECTION_START__*/
    }; // TextParamWide

//=======================================================================================
//! File-based structure that stores enter data field (EDF) information.
// @bsiclass
//=======================================================================================
struct TextEDField
    {
    byte    start;  //!< Start index of the EDF; this is 1-indexed in the file
    byte    len;    //!< Length of the EDF
    byte    just;   //!< Justification; see EdfJustification

    }; // TextEDField

//=======================================================================================
//! Convenience structure to hold multiple EDFs.
// @bsiclass
//=======================================================================================
struct TextEDParam
    {
    int             numEDFields;
    TextEDFieldP    edField;

    }; // TextEDParam

//=======================================================================================
//! Legacy structure; do not add new APIs that use this, and aggressively replace when found.
// @bsiclass
//=======================================================================================
typedef struct textScale
    {
    double  x;
    double  y;

    } TextScale;

//=======================================================================================
//! Legacy structure; do not add new APIs that use this, and aggressively replace when found.
// @bsiclass
//=======================================================================================
typedef struct textFlags
    {
    UShort  upper       :1; //!< upper case
    UShort  lower       :1; //!< lower case
    UShort  fract       :1; //!< fractions
    UShort  intl        :1; //!< international
    UShort  fast        :1; //!< fast
    UShort  space       :1; //!< space char defined?
    UShort  filled      :1; //!< filled font
    UShort  reserved    :9; //!< reserved

    } TextFlags;

//=======================================================================================
//! Legacy structure; do not add new APIs that use this, and aggressively replace when found.
// @bsiclass
//=======================================================================================
struct TextStyleInfo
    {
    UInt32  font;
    double  slant;
    byte    bold;
    byte    italics;
    short   lineStyle_deprecated;
    UInt32  shxBigFont;

    /*MSCORE*/  void FromActive ();
    /*MSCORE*/  void FromElement (ElementHandleCR);

    }; // TextStyleInfo

//=======================================================================================
//! Legacy structure; do not add new APIs that use this, and aggressively replace when found.
// @bsiclass
//=======================================================================================
typedef struct textFontInfo
    {
    TextFlags   lettersType;    //!< upper, lower, fractions, intl
    byte        charType;       //!< symbol or normal
    byte        vectorSize;     //!< byte or word
    byte        graphicType;    //!< raster or vector
    byte        charSize;       //!< 8 or 16 bit representation of font
    short       tileSize;       //!< size of "tile" making up characters
    long        dataSize;       //!< size of file including header
    Point2d     origin;         //!< origin of char in the "tile"
    TextScale   scale;          //!< scale factor of font

    } TextFontInfo;

#endif // !defined(__midl)

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
