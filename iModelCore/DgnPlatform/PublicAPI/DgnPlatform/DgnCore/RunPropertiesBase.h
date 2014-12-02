/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/RunPropertiesBase.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "../DgnPlatform.h"
#include "TextParam.h"

//__PUBLISH_SECTION_END__
DGNPLATFORM_TYPEDEFS (PersistableTextStringHelper)
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//__PUBLISH_SECTION_END__

struct  DgnRscFont;
struct  DgnTrueTypeFontManager;
struct  DgnShxFont;

//__PUBLISH_SECTION_START__

//=======================================================================================
//! Container for the common properties of runs of text.
//! This is a base class with run properties that can be shared between TextString and TextBlock. Both TextString and TextBlock deal with 'runs' of like-formatted characters, but require different subsets of information; this serves as a base class for both systems, containing the properties and accessors that they can share.
//! @note This class is not meant to be used directly; see provided derived classes (i.e. TextStringProperties and RunProperties).
// @bsiclass                                                    BentleySystems   06/2007
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RunPropertiesBase : public RefCountedBase
    {
//__PUBLISH_SECTION_END__

    // We friend struct these so that we don't take all of the Get/Set function call performance hits.
    friend struct TextString;
    friend struct DgnTrueTypeFontManager;
    friend struct DgnShxFont;
    friend struct DgnRscFont;
    friend struct PersistableTextStringHelper;

    public:     DGNPLATFORM_EXPORT static const double SUPERSCRIPT_SUBSCRIPT_SCALE;
    public:     DGNPLATFORM_EXPORT static const double SUPERSCRIPT_SUBSCRIPT_SHIFT;

    protected:  DgnFontCP               m_font;
    protected:  DgnFontCP               m_shxBigFont;
    protected:  bool                    m_hasColor;
    protected:  UInt32                  m_color;
    protected:  bool                    m_isBold;
    protected:  bool                    m_isItalic;
    protected:  double                  m_customSlantAngle;
    protected:  bool                    m_isUnderlined;
    protected:  bool                    m_shouldUseUnderlineStyle;
    protected:  double                  m_underlineOffset;
    protected:  UInt32                  m_underlineColor;
    protected:  Int32                   m_underlineLineStyle;
    protected:  UInt32                  m_underlineWeight;
    protected:  bool                    m_isOverlined;
    protected:  bool                    m_shouldUseOverlineStyle;
    protected:  double                  m_overlineOffset;
    protected:  UInt32                  m_overlineColor;
    protected:  Int32                   m_overlineLineStyle;
    protected:  UInt32                  m_overlineWeight;
    protected:  CharacterSpacingType    m_characterSpacingType;
    protected:  double                  m_characterSpacingValue;
    protected:  bool                    m_shouldUseBackground;
    protected:  UInt32                  m_backgroundFillColor;
    protected:  UInt32                  m_backgroundBorderColor;
    protected:  Int32                   m_backgroundBorderLineStyle;
    protected:  UInt32                  m_backgroundBorderWeight;
    protected:  DPoint2d                m_backgroundBorderPadding;
    protected:  DPoint2d                m_runOffset;
    protected:  bool                    m_isSubScript;
    protected:  bool                    m_isSuperScript;
    protected:  bool                    m_shouldIgnoreLSB;
    protected:  DPoint2d                m_fontSize;

    protected:  DGNPLATFORM_EXPORT                      RunPropertiesBase           ();
    protected:  DGNPLATFORM_EXPORT                      RunPropertiesBase           (DgnFontCR font, DgnFontCP shxBigFont, DPoint2dCR fontSize);
    protected:  DGNPLATFORM_EXPORT                      RunPropertiesBase           (TextParamWideCR, DPoint2dCR fontSize, DgnProjectCR);
    protected:  DGNPLATFORM_EXPORT                      RunPropertiesBase           (RunPropertiesBaseCR);

    private:                            void        InitDefaults                (DgnFontCR font, DgnFontCP shxBigFont, DPoint2dCR fontSize);

    public:     DGNPLATFORM_EXPORT          void        FromElementData             (TextParamWideCR, DPoint2dCR fontSize, DgnProjectCR);
    protected:  DGNPLATFORM_EXPORT  virtual void        _FromElementData            (TextParamWideCR, DPoint2dCR fontSize, DgnProjectCR);
    private:                            void        FromElementDataInternal     (TextParamWideCR, DPoint2dCR fontSize, DgnProjectCR);
    public:     DGNPLATFORM_EXPORT          void        ToElementData               (TextParamWideR, DgnProjectR) const;
    protected:  DGNPLATFORM_EXPORT  virtual void        _ToElementData              (TextParamWideR, DgnProjectR) const;
    private:                            void        ToElementDataInternal       (TextParamWideR, DgnProjectR) const;
    protected:  DGNPLATFORM_EXPORT          bool        Equals                      (RunPropertiesBaseCR) const;
    protected:  DGNPLATFORM_EXPORT          bool        Equals                      (RunPropertiesBaseCR, double tolerance, bool shouldIgnoreElementOverhead) const;

    public:     DGNPLATFORM_EXPORT          DPoint2d    GetDisplaySize              () const;
    public:     DGNPLATFORM_EXPORT          DPoint3d    GetDisplayOffset            () const;

    public:     DGNPLATFORM_EXPORT  virtual void        _SetFont                    (DgnFontCR);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetShxBigFont              (DgnFontCP);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetColor                   (UInt32);
    public:     DGNPLATFORM_EXPORT  virtual void        _ClearColor                 ();
    public:     DGNPLATFORM_EXPORT  virtual void        _SetIsBold                  (bool);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetIsItalic                (bool);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetCustomSlantAngle        (double);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetIsUnderlined            (bool);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetShouldUseUnderlineStyle (bool);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetUnderlineStyle          (UInt32 const * color, Int32 const * lineStyle, UInt32 const * weight);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetUnderlineOffset         (double);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetIsOverlined             (bool);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetShouldUseOverlineStyle  (bool);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetOverlineStyle           (UInt32 const * color, Int32 const * lineStyle, UInt32 const * weight);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetOverlineOffset          (double);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetCharacterSpacingType    (CharacterSpacingType);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetCharacterSpacingValue   (double);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetShouldUseBackground     (bool);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetBackgroundStyle         (UInt32 const * fillColor, UInt32 const * borderColor, Int32 const * borderLineStyle, UInt32 const * borderWeight, DPoint2d const * borderPadding);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetRunOffset               (DPoint2dCR);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetIsSubScript             (bool);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetIsSuperScript           (bool);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetShouldIgnoreLSB         (bool);
    public:     DGNPLATFORM_EXPORT  virtual void        _SetFontSize                (DPoint2dCR);

    public:     DGNPLATFORM_EXPORT          DgnFontCR   GetFontForCodePage          () const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Gets the 'normal' font.
    public: DGNPLATFORM_EXPORT DgnFontCR GetFont () const;

    //! Sets the 'normal' font.
    //! The value is ignored if it is an SHX big font.
    public: DGNPLATFORM_EXPORT void SetFont (DgnFontCR);

    //! Resolves the 'normal' font. This ensures you get a non-missing font object that can be used to graphical operations (e.g. drawing and measuring).
    public: DGNPLATFORM_EXPORT DgnFontCR ResolveFont () const;

    //! Gets the (optional) 'big' font.
    public: DGNPLATFORM_EXPORT DgnFontCP GetShxBigFontCP () const;

    //! Sets the 'big' font; can be NULL.
    //! The value is ignored if it is not an SHX big font.
    public: DGNPLATFORM_EXPORT void SetShxBigFont (DgnFontCP);

    //! Resolves the 'big' font. If a big font is specified, this ensures you get a non-missing font object that can be used to graphical operations (e.g. drawing and measuring).
    public: DGNPLATFORM_EXPORT DgnFontCP ResolveShxBigFontCP () const;

    //! True if a text color is specified.
    //! @see    GetColor for additional notes
    public: DGNPLATFORM_EXPORT bool HasColor () const;

    //! Clears the text color index and indicates that a color is no longer specified.
    //! @see    GetColor for additional notes
    public: DGNPLATFORM_EXPORT void ClearColor ();

    //! Gets the text color index.
    //! @see    HasColor since this value is optional
    //! @note   Color indices are per-file. For TextString, this index will pass through to the view context, and effectively indicates what color to use out of the view root. For TextBlock, this color is in relation to the TextBlock's file (derived from its DGN cache).
    //! @note   If color is not specified, it comes from the display header.
    public: DGNPLATFORM_EXPORT UInt32 GetColor () const;

    //! Sets the text color index.
    //! @see    GetColor for additional notes
    public: DGNPLATFORM_EXPORT void SetColor (UInt32);

    //! True if text should be drawn bold.
    //! @note   This is only valid for TrueType text, and selects the bold typeface when drawing.
    public: DGNPLATFORM_EXPORT bool IsBold () const;

    //! Sets if text should be drawn bold.
    //! @see    IsBold for additional notes
    public: DGNPLATFORM_EXPORT void SetIsBold (bool);

    //! True if text should be drawn italicized.
    //! @note   Italic text is normally drawn with a geometric shear; however, TrueType-based text without a custom slant angle will use its italic typeface instead. TrueType-based text with a custom slant angle will use the nominal typeface with a geometric shear.
    public: DGNPLATFORM_EXPORT bool IsItalic () const;

    //! True if an italic typeface should be selected.
    //! @see    IsItalic for additional notes
    public: DGNPLATFORM_EXPORT bool ShouldUseItalicTypeface () const;

    //! Sets if text should be drawn italic.
    //! @see    IsItalic for additional notes
    public: DGNPLATFORM_EXPORT void SetIsItalic (bool);

    //! Gets the custom slant angle.
    //! @see    IsItalic for additional notes
    //! @note   This is only used if the italic flag is also set.
    public: DGNPLATFORM_EXPORT double GetCustomSlantAngle () const;

    //! Sets the custom slant angle.
    //! @see    GetCustomSlantAngle for additional notes
    public: DGNPLATFORM_EXPORT void SetCustomSlantAngle (double);

    //! True if an underline should be drawn.
    //! @note   If no underline style is specified, the underline color / style / weight will be text color / solid / 0.
    public: DGNPLATFORM_EXPORT bool IsUnderlined () const;

    //! Sets if an underline should be drawn.
    //! @see    IsUnderlined for additional notes
    public: DGNPLATFORM_EXPORT void SetIsUnderlined (bool);

    //! True if an underline style is specified.
    //! @see    IsUnderlined for additional notes
    public: DGNPLATFORM_EXPORT bool ShouldUseUnderlineStyle () const;

    //! Sets if an underline style is specified.
    //! @see    IsUnderlined for additional notes
    public: DGNPLATFORM_EXPORT void SetShouldUseUnderlineStyle (bool);

    //! Gets underline style attributes. Pass NULL for any values you don't wish to receive.
    //! @see    IsUnderlined for additional notes
    public: DGNPLATFORM_EXPORT void GetUnderlineStyle (UInt32* color, Int32* lineStyle, UInt32* weight) const;

    //! Sets underline style attributes. Pass NULL for any values you won't wish to set; they will remain as-is.
    //! @see    IsUnderlined for additional notes
    public: DGNPLATFORM_EXPORT void SetUnderlineStyle (UInt32 const * color, Int32 const * lineStyle, UInt32 const * weight);

    //! Gets the underline offset.
    //! @see    IsUnderlined for additional notes
    public: DGNPLATFORM_EXPORT double GetUnderlineOffset () const;

    //! Sets the underline offset.
    //! @see    IsUnderlined for additional notes
    public: DGNPLATFORM_EXPORT void SetUnderlineOffset (double);

    //! True if an overline should be drawn.
    //! @note   If no overline style is specified, the overline color / style / weight will be text color / solid / 0.
    public: DGNPLATFORM_EXPORT bool IsOverlined () const;

    //! Sets if an overline should be drawn.
    //! @see    IsOverlined for additional notes
    public: DGNPLATFORM_EXPORT void SetIsOverlined (bool);

    //! True if an overline style is specified.
    //! @see    IsOverlined for additional notes
    public: DGNPLATFORM_EXPORT bool ShouldUseOverlineStyle () const;

    //! Sets if an overline style is specified.
    //! @see    IsOverlined for additional notes
    public: DGNPLATFORM_EXPORT void SetShouldUseOverlineStyle (bool);

    //! Gets overline style attributes. Pass NULL for any values you don't wish to receive.
    //! @see    IsOverlined for additional notes
    public: DGNPLATFORM_EXPORT void GetOverlineStyle (UInt32* color, Int32* lineStyle, UInt32* weight) const;

    //! Sets overline style attributes. Pass NULL for any values you don't wish to set; they will retain their current value.
    //! @see    IsOverlined for additional notes
    public: DGNPLATFORM_EXPORT void SetOverlineStyle (UInt32 const * color, Int32 const * lineStyle, UInt32 const * weight);

    //! Gets the overline offset.
    //! @see    IsOverlined for additional notes
    public: DGNPLATFORM_EXPORT double GetOverlineOffset () const;

    //! Sets the overline offset.
    //! @see    IsOverlined for additional notes
    public: DGNPLATFORM_EXPORT void SetOverlineOffset (double);

    //! Gets the character spacing type.
    //! @note   A character spacing of CharacterSpacingType::Absolute with a value of 0.0 is nominal character spacing.
    public: DGNPLATFORM_EXPORT CharacterSpacingType GetCharacterSpacingType () const;

    //! Sets the character spacing type.
    //! @see    GetCharacterSpacingType for additional notes
    public: DGNPLATFORM_EXPORT void SetCharacterSpacingType (CharacterSpacingType);

    //! Gets the character spacing value
    //! @see    GetCharacterSpacingType for additional notes
    public: DGNPLATFORM_EXPORT double GetCharacterSpacingValue () const;

    //! Sets the character spacing value.
    //! @see    GetCharacterSpacingType for additional notes
    public: DGNPLATFORM_EXPORT void SetCharacterSpacingValue (double);

    //! True if a background and border should be drawn behind the text.
    public: DGNPLATFORM_EXPORT bool ShouldUseBackground () const;

    //! Sets if a background and border should be drawn behind the text.
    //! @see    ShouldUseBackground for additional notes
    public: DGNPLATFORM_EXPORT void SetShouldUseBackground (bool);

    //! Gets background style attributes. Pass NULL for any values you don't wish to receive.
    //! @see    ShouldUseBackground for additional notes
    public: DGNPLATFORM_EXPORT void GetBackgroundStyle (UInt32* fillColor, UInt32* borderColor, Int32* borderLineStyle, UInt32* borderWeight, DPoint2d* borderPadding) const;

    //! Sets background style attributes. Pass NULL for any values you don't wish to set; they will retain their current value.
    //! @see    ShouldUseBackground for additional notes
    public: DGNPLATFORM_EXPORT void SetBackgroundStyle (UInt32 const * fillColor, UInt32 const * borderColor, Int32 const * borderLineStyle, UInt32 const * borderWeight, DPoint2d const * borderPadding);

    //! Gets the run offset.
    //! @note   Run offset should not be manually adjusted for fraction runs.
    public: DGNPLATFORM_EXPORT DPoint2dCR GetRunOffset () const;

    //! Sets the run offset.
    //! @see    GetRunOffset for additional notes
    public: DGNPLATFORM_EXPORT void SetRunOffset (DPoint2dCR);

    //! True if the text should be drawn as subscript.
    //! @note   Subscript text is automatically scaled and shifted to appear as subscript.
    public: DGNPLATFORM_EXPORT bool IsSubScript () const;

    //! Sets whether the text should be drawn as subscript.
    //! @see    IsSubScript for additional notes
    public: DGNPLATFORM_EXPORT void SetIsSubScript (bool);

    //! True is the text should be drawn as superscript.
    //! @note   Superscript text is automatically scaled and shifted to appear as superscript.
    public: DGNPLATFORM_EXPORT bool IsSuperScript () const;

    //! Sets whether the text should be drawn as superscript.
    //! @see    IsSuperScript for additional notes
    public: DGNPLATFORM_EXPORT void SetIsSuperScript (bool);

    //! True if the left side bearing should be ignored when measuring the text.
    //! @note   This should not normally be controlled; it is used for DWG compatibility.
    public: DGNPLATFORM_EXPORT bool ShouldIgnoreLSB () const;

    //! Sets whether the left side bearing should be ignored when measuring the text.
    //! @see    ShouldIgnoreLSB for additional notes
    public: DGNPLATFORM_EXPORT void SetShouldIgnoreLSB (bool);

    //! Gets the size, in UORs, that the glyphs will be drawn.
    public: DGNPLATFORM_EXPORT DPoint2dCR GetFontSize () const;

    //! Sets the size, in UORs, that the glyphs will be drawn.
    //! @see    GetFontSize for additional notes
    public: DGNPLATFORM_EXPORT void SetFontSize (DPoint2dCR);

    //! Scales all scalar values in this instace.
    //! This includes text size, as well as spacing and offset attributes.
    public: DGNPLATFORM_EXPORT void ApplyScale (DPoint2dCR, bool isVertical);

    }; // RunPropertiesBase

/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
