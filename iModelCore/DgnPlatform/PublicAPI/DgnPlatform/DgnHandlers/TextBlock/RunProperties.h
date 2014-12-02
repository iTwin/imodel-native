/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/RunProperties.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnCore/RunPropertiesBase.h>
#include <DgnPlatform/DgnHandlers/TextBlock/IDgnTextStyleApplyable.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//=======================================================================================
//! Describes the formatting and spacing paramters for a single run of text that TextBlock supports.
//!
//! To create a new instance, use the static Create methods, or Clone an existing instance.<br>
//! <br>
//! This class stores property override flags and an optional text style. If a text style is present, override flags will be enabled when you call a Set... method. This means that if changes are made to the underlying text style, said modified properties will not propogate, but remain as-is on a given element because they were overridden. You can clear an override by calling the appropriate Clear...Override method (does nothing if there is no text style); you can also test to see if an override was set by calling the appropriate Is...Overridden method (always returns false if there is no text style). ApplyTextStyle provides the option to maintain and respect overrides, or zero them out and ignore them.<br>
//! <br>
//! If created from a text style, this object is a snapshot; it will not automatically update if you change the text style via other means.<br>
//! <br>
//! Since RunProperties can be used to round trip and clone elements among files, and the base class stores things that are at most per-cache (e.g. sizes and offsets in UORs), this class stores (and requires at all times) an associated cache, so effective values can be computed.
//! <br>
//! This class also effectively implements the IDgnTextStyleApplyable interface; use AsIDgnTextStyleApplyable and AsIDgnTextStyleApplyableR to get direct access.
//!
// @bsiclass                                                    Venkat.Kalyan   10/2007
//=======================================================================================
struct RunProperties :  public RunPropertiesBase
//__PUBLISH_SECTION_END__
                        ,public IDgnTextStyleApplyable
//__PUBLISH_SECTION_START__
    {
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(RunPropertiesBase)
    friend struct TextBlock; // So ACAD compatibility checks can look at overrides.

    //=======================================================================================
    // @bsiclass                                                    Venkat.Kalyan   10/2007
    //=======================================================================================
    private: struct Overrides
        {
        public: bool    m_font;
        public: bool    m_shxBigFont;
        public: bool    m_hasColor;
        public: bool    m_color;
        public: bool    m_isBold;
        public: bool    m_isItalic;
        public: bool    m_customSlantAngle;
        public: bool    m_isUnderlined;
        public: bool    m_shouldUseUnderlineStyle;
        public: bool    m_underlineOffset;
        public: bool    m_underlineColor;
        public: bool    m_underlineLineStyle;
        public: bool    m_underlineWeight;
        public: bool    m_isOverlined;
        public: bool    m_shouldUseOverlineStyle;
        public: bool    m_overlineOffset;
        public: bool    m_overlineColor;
        public: bool    m_overlineLineStyle;
        public: bool    m_overlineWeight;
        public: bool    m_characterSpacingType;
        public: bool    m_characterSpacingValue;
        public: bool    m_shouldUseBackground;
        public: bool    m_backgroundFillColor;
        public: bool    m_backgroundBorderColor;
        public: bool    m_backgroundBorderLineStyle;
        public: bool    m_backgroundBorderWeight;
        public: bool    m_backgroundBorderPadding;
        public: bool    m_runOffset;
        public: bool    m_isSubScript;
        public: bool    m_isSuperScript;
        public: bool    m_width;
        public: bool    m_height;

        public:         Overrides ();
        public: bool    Equals (Overrides const & rhs) const;
        public: void    Clear ();

        }; // Overrides

    private:    DgnModelP       m_dgnModel;
    private:    Overrides       m_overrides;
    private:    LineAlignment   m_lineAlignment;
    private:    UInt32          m_textStyleId;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    public:     DGNPLATFORM_EXPORT  explicit                    RunProperties               (DgnModelR);
    public:     DGNPLATFORM_EXPORT                              RunProperties               (DgnFontCR font, DgnFontCP shxBigFont, DPoint2dCR fontSize, DgnModelR);
    public:     DGNPLATFORM_EXPORT                              RunProperties               (TextParamWideCR, DPoint2dCR fontSize, DgnModelR);
    public:     DGNPLATFORM_EXPORT                              RunProperties               (DgnTextStyleCR, DgnModelR);
    public:     DGNPLATFORM_EXPORT                              RunProperties               (RunPropertiesCR);

    private:                                    void            InitDefaults                (DgnModelR);

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    protected:                      virtual     bool            _HasTextStyle               () const override;
    protected:                      virtual     UInt32          _GetTextStyleId             () const override;
    protected:                      virtual     DgnTextStylePtr _GetTextStyleInFile         () const override;
    protected:                      virtual     void            _ToStyle                    (DgnTextStyleR) const override;

    protected:                      virtual     void            _ApplyTextStyle             (DgnTextStyleCR, bool respectOverrides) override;
    protected:                      virtual     void            _RemoveTextStyle            () override;
    protected:                      virtual     void            _SetProperties              (DgnTextStyleCR, DgnTextStylePropertyMaskCR) override;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    protected:  DGNPLATFORM_EXPORT  virtual     void            _FromElementData            (TextParamWideCR, DPoint2dCR fontSize, DgnProjectCR) override;
    private:                                    void            FromElementDataInternal     (TextParamWideCR);
    protected:  DGNPLATFORM_EXPORT  virtual     void            _ToElementData              (TextParamWideR, DgnProjectR) const override;
    private:                                    void            ToElementDataInternal       (TextParamWideR) const;
    public:     DGNPLATFORM_EXPORT              bool            Equals                      (RunPropertiesCR) const;
    public:     DGNPLATFORM_EXPORT              bool            Equals                      (RunPropertiesCR, double tolerance, bool shouldIgnoreElementOverhead) const;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetFont                    (DgnFontCR) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetShxBigFont              (DgnFontCP) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetColor                   (UInt32) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _ClearColor                 () override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetIsBold                  (bool) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetIsItalic                (bool) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetCustomSlantAngle        (double) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetIsUnderlined            (bool) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetShouldUseUnderlineStyle (bool) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetUnderlineStyle          (UInt32 const * color, Int32 const * lineStyle, UInt32 const * weight) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetUnderlineOffset         (double) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetIsOverlined             (bool) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetShouldUseOverlineStyle  (bool) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetOverlineStyle           (UInt32 const * color, Int32 const * lineStyle, UInt32 const * weight) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetOverlineOffset          (double) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetCharacterSpacingType    (CharacterSpacingType) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetCharacterSpacingValue   (double) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetShouldUseBackground     (bool) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetBackgroundStyle         (UInt32 const * fillColor, UInt32 const * borderColor, Int32 const * borderLineStyle, UInt32 const * borderWeight, DPoint2d const * borderPadding) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetRunOffset               (DPoint2dCR) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetIsSubScript             (bool) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetIsSuperScript           (bool) override;
    protected:  DGNPLATFORM_EXPORT  virtual     void            _SetFontSize                (DPoint2dCR) override;

    public:     DGNPLATFORM_EXPORT              LineAlignment   GetLineAlignment            () const;
    public:     DGNPLATFORM_EXPORT              void            SetLineAlignment            (LineAlignment);

    private:                                    void            SetPropertiesFromStyle      (DgnTextStylePropertyMaskCR applyMask, DgnTextStyleCR, DgnTextStylePropertyMaskCR overridesMask);

    // DO NOT PUBLISH -- meant to maintain the reference counted pattern internally.
    public: DGNPLATFORM_EXPORT static RunPropertiesPtr Create (TextParamWideCR, DPoint2dCR fontSize, DgnModelR);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Provides access to the IDgnTextStyleApplyable interface that this object effectively implements.
    public: DGNPLATFORM_EXPORT IDgnTextStyleApplyable const & AsIDgnTextStyleApplyable () const;

    //! Provides access to the IDgnTextStyleApplyable interface that this object effectively implements.
    public: DGNPLATFORM_EXPORT IDgnTextStyleApplyable& AsIDgnTextStyleApplyableR ();

    //! Creates a new instance of RunProperties with default (e.g. zero'ed) values.
    public: DGNPLATFORM_EXPORT static RunPropertiesPtr Create (DgnFontCR, DPoint2dCR fontSize, DgnModelR);

    //! Creates a new instance of RunProperties with applicable properties from the provided style.
    public: DGNPLATFORM_EXPORT static RunPropertiesPtr Create (DgnTextStyleCR, DgnModelR);

    //! Creates a new instance as a deep copy of this instance.
    public: DGNPLATFORM_EXPORT RunPropertiesPtr Clone () const;

    //! Gets the DGN cache that this instance is associated with.
    public: DGNPLATFORM_EXPORT DgnModelR GetDgnModelR () const;

    //! True if the font property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsFontOverridden () const;

    //! Clears the font property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearFontOverride ();

    //! True if the big font property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsShxBigFontOverridden () const;

    //! Clears the big font property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearShxBigFontOverride ();

    //! True if the has color property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsHasColorOverridden () const;

    //! Clears the has color property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearHasColorOverride ();

    //! True if the color property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsColorOverridden () const;

    //! Clears the color property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearColorOverride ();

    //! True if the is bold property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsBoldOverridden () const;

    //! Clears the is bold property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearIsBoldOverride ();

    //! True if the is italic property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsItalicOverridden () const;

    //! Clears the is italic property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearIsItalicOverride ();

    //! True if the custom slant angle property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsCustomSlantAngleOverridden () const;

    //! Clears the custom slant angle property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearCustomSlantAngleOverride ();

    //! True if the is underlined property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsUnderlinedOverridden () const;

    //! Clears the is underlined property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearIsUnderlinedOverride ();

    //! True if the should use underline style property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsShouldUseUnderlineStyleOverridden () const;

    //! Clears the should use underline style property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearShouldUseUnderlineStyleOverride ();

    //! True if the underline offset property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsUnderlineOffsetOverridden () const;

    //! Clears the underline offset property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearUnderlineOffsetOverride ();

    //! True if the underline color property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsUnderlineColorOverridden () const;

    //! Clears the underline color property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearUnderlineColorOverride ();

    //! True if the underline line style property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsUnderlineLineStyleOverridden () const;

    //! Clears the underline line style property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearUnderlineLineStyleOverride ();

    //! True if the underline weight property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsUnderlineWeightOverridden () const;

    //! Clears the underline weight property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearUnderlineWeightOverride ();

    //! True if the is overlined property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsOverlinedOverridden () const;

    //! Clears the is overlined property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearIsOverlinedOverride ();

    //! True if the should use overline style property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsShouldUseOverlineStyleOverridden () const;

    //! Clears the should use overline style property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearShouldUseOverlineStyleOverride ();

    //! True if the overline offset property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsOverlineOffsetOverridden () const;

    //! Clears the overline offset property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearOverlineOffsetOverride ();

    //! True if the overline color property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsOverlineColorOverridden () const;

    //! Clears the overline color property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearOverlineColorOverride ();

    //! True if the overline line style property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsOverlineLineStyleOverridden () const;

    //! Clears the overline line style property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearOverlineLineStyleOverride ();

    //! True if the overline weight property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsOverlineWeightOverridden () const;

    //! Clears the overline weight property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearOverlineWeightOverride ();

    //! True if the character spacing type property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsCharacterSpacingTypeOverridden () const;

    //! Clears the character spacing type property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearCharacterSpacingTypeOverride ();

    //! True if the character spacing value property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsCharacterSpacingValueOverridden () const;

    //! Clears the character spacing value property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearCharacterSpacingValueOverride ();

    //! True if the should use background property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsShouldUseBackgroundOverridden () const;

    //! Clears the should use background property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearShouldUseBackgroundOverride ();

    //! True if the background fill color property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsBackgroundFillColorOverridden () const;

    //! Clears the background fill color property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearBackgroundFillColorOverride ();

    //! True if the background border color property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsBackgroundBorderColorOverridden () const;

    //! Clears the background border color property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearBackgroundBorderColorOverride ();

    //! True if the background border line style property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsBackgroundBorderLineStyleOverridden () const;

    //! Clears the background border line style property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearBackgroundBorderLineStyleOverride ();

    //! True if the background border weight property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsBackgroundBorderWeightOverridden () const;

    //! Clears the background border weight property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearBackgroundBorderWeightOverride ();

    //! True if the background border padding property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsBackgroundBorderPaddingOverridden () const;

    //! Clears the background border padding property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearBackgroundBorderPaddingOverride ();

    //! True if the run offset property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsRunOffsetOverridden () const;

    //! Clears the run offset property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearRunOffsetOverride ();

    //! True if the is subscript property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsSubScriptOverridden () const;

    //! Clears the is subscript property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearIsSubScriptOverride ();

    //! True if the is superscript property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsSuperScriptOverridden () const;

    //! Clears the is superscript property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearIsSuperScriptOverride ();

    //! True if the width property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsWidthOverridden () const;

    //! Clears the width property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearWidthOverride ();

    //! True if the height property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsHeightOverridden () const;

    //! Clears the height property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearHeightOverride ();

    }; // RunProperties

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
