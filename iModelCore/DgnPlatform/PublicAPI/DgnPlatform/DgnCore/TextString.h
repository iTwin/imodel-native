/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/TextString.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "IViewDraw.h"
#include "TextStringProperties.h"

//__PUBLISH_SECTION_END__
DGNPLATFORM_TYPEDEFS (PersistableTextStringHelper)
DGNPLATFORM_TYPEDEFS (IDgnGlyphLayoutListener);
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//__PUBLISH_SECTION_END__

struct  DgnRscFont;
struct  DgnTrueTypeFontManager;
struct  DgnShxFont;

//__PUBLISH_SECTION_START__

//! Smart pointer wrapper for TextString.
typedef RefCountedPtr<TextString> TextStringPtr;

//=======================================================================================
//! Used to draw a single-line collection of like-formatted characters; this class is targeted at performance, is immutable, and cannot create elements.
//! To create a TextString from a text element, see TextElemHandler::InitTextString. You must use one of the static Create methods to create instances of this class.<br>
//! <br>
//! This class should only be used for purposes of rendering; as such, it is essentially immutable, and cannot generate a persistent format. If you want to work with text data for any other purpose (e.g. modifying the string or formatting and writing back to the file), see TextBlock.
// @bsiclass
//=======================================================================================
struct TextString : public RefCountedBase
    {
//__PUBLISH_SECTION_END__
    friend struct DgnShxFont;
    friend struct DgnTrueTypeFontManager;
    friend struct DgnRscFont;
    friend struct PersistableTextStringHelper;

private:
    enum
        {
        PREALLOC_TEXTLEN = 128,                       //!< Number of characters (and associated data) we pre-allocate with this object
        PER_CHAR_SIZE    = (
                            sizeof (FontChar)    +    // m_fontchars
                            sizeof (TextEDField) +    // m_edFields
                            sizeof (Byte)        +    // m_enterDataFlags
                            sizeof (UInt16)      +    // m_glyphCodes
                            sizeof (DPoint3d)         // m_glyphOrigins
                            )
        };

    byte m_preAlloc[PREALLOC_TEXTLEN * PER_CHAR_SIZE];
    byte* m_alloced;
    FontCharP m_fontChars; //!< Characters in locale of font
    TextEDField* m_edFields; //!< Start and length of each EDF
    mutable Byte* m_enterDataFlags; //!< Justification of each EDF
    UInt16* m_glyphCodes; //!< Glyph IDs (from font system)
    DPoint3dP m_glyphOrigins; //!< Lower-left origin of each glyph
    size_t m_bufferCount;
    size_t m_numFontChars;
    size_t m_numEdFields;
    mutable size_t m_numGlyphCodes;
    TextStringProperties m_props;
    UInt32 m_lineWeight;
    DPoint3d m_lowerLeft;
    mutable DRange2d m_extents;
    RotMatrix m_rMatrix;
    mutable bool m_extentsValid;
    bool m_shouldIgnorePercentEscapes;
    mutable UInt32 m_loadedFontNumber;

    void SetEDFlags () const;
    void EnsureInternalBuffer (size_t newCount, bool shouldCopyData);
    DGNPLATFORM_EXPORT BentleyStatus GenerateBoundingShape (DPoint3dP, size_t offset, size_t length, DPoint2dCP expandLL = NULL, DPoint2dCP expandUR = NULL) const;

public:
    DGNPLATFORM_EXPORT TextString ();
    DGNPLATFORM_EXPORT TextString (WCharCP, DPoint3dCP lowerLeft, RotMatrixCP, TextStringPropertiesCR);
    DGNPLATFORM_EXPORT ~TextString ();
    DGNPLATFORM_EXPORT void Init (WCharCP, DPoint3dCR origin, RotMatrixCR, TextStringPropertiesCR, size_t numEDFields, TextEDFieldCP, int symbLineWeight);
    DGNPLATFORM_EXPORT void Init (FontCharCP, size_t numFontChars, DgnModelR, DPoint3dCR, RotMatrixCR, DPoint2dCR scale, TextParamWideCR, size_t numEDFields, TextEDFieldCP, int symbLineWeight);
    DGNPLATFORM_EXPORT void Init (FontCharCP, size_t numFontChars, DPoint3dCR, RotMatrixCR, TextStringPropertiesCR, size_t numEDFields, TextEDFieldCP, int symbLineWeight);
    bool IsEmptyString () const;
    DGNPLATFORM_EXPORT BentleyStatus LoadGlyphs (IDgnGlyphLayoutListenerP) const;
    UInt32 GetLoadedFontNumber () const {return m_loadedFontNumber;}
    size_t GetNumGlyphCodes () const {return m_numGlyphCodes;}
    UInt16 const* GetGlyphCodes () const {return m_glyphCodes;}
    FontCharCP GetFontChars () const {return m_fontChars;}
    size_t GetNumFontChars () const {return m_numFontChars;}
    DPoint3dCP GetGlyphOrigins () const {return m_glyphOrigins;}
    Byte const* GetEdFlags () const {return (0 == m_numEdFields) ? NULL : m_enterDataFlags;}
    void SetFontSize (DPoint2dCR fontSize) {m_props.SetFontSize (fontSize); m_extentsValid = false;}
    DPoint3d GetLowerLeft() const {return m_lowerLeft;}
    void SetLowerLeft (DPoint3dCR newLL) {m_lowerLeft = newLL;}
    bool Is3d () const {return m_props.m_is3d;}
    DGNPLATFORM_EXPORT bool IsBlankString () const;
    DGNPLATFORM_EXPORT void GetGlyphSymbology (ElemDisplayParamsR) const;
    DGNPLATFORM_EXPORT void GetUnderlineSymbology (ElemDisplayParamsR) const;
    DGNPLATFORM_EXPORT void GetOverlineSymbology (ElemDisplayParamsR) const;
    DGNPLATFORM_EXPORT void GetFractionSymbology (ElemDisplayParamsR) const;
    DGNPLATFORM_EXPORT void GetBackgroundSymbology (ElemDisplayParamsR) const;
    DGNPLATFORM_EXPORT void GetOutlineSymbology (ElemDisplayParamsR) const;
    DGNPLATFORM_EXPORT void GetDrawTransform (Transform& trans, bool includeShear) const;
    DGNPLATFORM_EXPORT bool HasFraction () const;
    DGNPLATFORM_EXPORT BentleyStatus GetFontDescription (WStringR) const;
    DGNPLATFORM_EXPORT bool MakeExtentsValid () const;
    DGNPLATFORM_EXPORT void DrawTextAdornments (ViewContextR) const;
    DGNPLATFORM_EXPORT void DrawTextBackground (ViewContextR) const;
    void ReCookDisplayParams (ElemDisplayParamsP, ViewContextP) const;
    DGNPLATFORM_EXPORT void Clear ();
    DGNPLATFORM_EXPORT BentleyStatus GenerateBoundingShape (DPoint3dP, DPoint2dCP expandLL = NULL, DPoint2dCP expandUR = NULL) const;
    DGNPLATFORM_EXPORT static void ComputeUserOriginOffset (DPoint3dR computedOffset, DPoint2dCR textExtents, double wordWrapLength, TextElementJustification justification, bool isVertical, DgnFontCP font);
    DGNPLATFORM_EXPORT static void TransformOrientationAndGetScale (DPoint2dR scaleFactor, RotMatrixR orientation, double* rotationAngle, TransformCR transform, bool is3d);
    DGNPLATFORM_EXPORT void SetShouldIgnorePercentEscapes (bool);
    DGNPLATFORM_EXPORT static void SortAndValidateEdfs (bvector<TextEDField>&, size_t elementStringLength);
    DGNPLATFORM_EXPORT UInt32 GetLineWeight () const;
    DGNPLATFORM_EXPORT void SetLineWeight (UInt32);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    //! Constructs a new TextString with an empty string. This instance is invalid until an Init method is called on it (e.g. TextElemHandler::InitTextString).
    DGNPLATFORM_EXPORT static TextStringPtr Create ();

    //! Constructs a new TextString from a NULL-terminated Unicode string and format parameters.
    DGNPLATFORM_EXPORT static TextStringPtr Create (WCharCP, DPoint3dCP lowerLeft, RotMatrixCP, TextStringPropertiesCR);

    //! Gets the bounds of this entire instance in local coordinates.
    //! \note   The bounds computed are known as the "justification range", and describe the bounds that are used for positioning and snapping. This is opposed to pure exact (black box) and nominal (cell box) ranges.
    DGNPLATFORM_EXPORT DRange2dCR GetExtents () const;

    //! Gets the width of the extents.
    DGNPLATFORM_EXPORT double GetWidth () const;

    //! Gets the height of the extents.
    DGNPLATFORM_EXPORT double GetHeight () const;

    //! Gets the lower-left origin of the text.
    DGNPLATFORM_EXPORT DPoint3dCR GetOrigin () const;

    //! Gets the user origin of this instance.
    DGNPLATFORM_EXPORT BentleyStatus ComputeUserOrigin (DPoint3dR) const;

    //! Sets the user origin of this instance.
    //! \note   The internal cached data is independent of user origin, so this can be changed after initialization.
    DGNPLATFORM_EXPORT BentleyStatus SetOriginFromUserOrigin (DPoint3dCR);

    //! Gets the formatting properties.
    DGNPLATFORM_EXPORT TextStringPropertiesCR GetProperties () const;

    //! Determines if the properties' font is an RSC font.
    DGNPLATFORM_EXPORT bool IsRscFont () const;

    //! Determines if the properties' font is an SHX font.
    DGNPLATFORM_EXPORT bool IsShxFont () const;

    //! Determines if the properties' font is a TrueType font.
    DGNPLATFORM_EXPORT bool IsTrueTypeFont () const;

    //! Determines if the font is effectively Unicode (e.g. 'normal' font is Unicode, and no SHX big font exists).
    DGNPLATFORM_EXPORT bool IsUnicodeFont () const;

    //! Gets the Unicode string.
    DGNPLATFORM_EXPORT WString GetString () const;

    //! Gets the orientation.
    DGNPLATFORM_EXPORT RotMatrixCR GetRotMatrix () const;
    };

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
