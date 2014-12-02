/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/TextStringProperties.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "RunPropertiesBase.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//! Smart pointer wrapper for TextStringProperties.
typedef RefCountedPtr<TextStringProperties> TextStringPropertiesPtr;

//=======================================================================================
//! Describes the formatting and spacing parameters for a single run of text that TextString supports.
//! You must use one of the static Create methods to create instances of this class.
// @bsiclass                                                    BentleySystems   05/2007
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TextStringProperties : public RunPropertiesBase
    {
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(RunPropertiesBase)

    friend struct TextString;
    friend struct DgnTrueTypeFontManager;
    friend struct DgnShxFont;
    friend struct DgnRscFont;
    friend struct PersistableTextStringHelper;

    private:    DgnGlyphRunLayoutFlags      m_layoutFlags;
    private:    bool                        m_isPartOfField;
    private:    bool                        m_isViewIndependent;
    private:    bool                        m_is3d;
    private:    TextElementJustification    m_justification;
    private:    StackedFractionType         m_stackedFractionType;
    private:    StackedFractionSection      m_stackedFractionSection;

    public:     DGNPLATFORM_EXPORT                                  TextStringProperties    ();
    public:     DGNPLATFORM_EXPORT                                  TextStringProperties    (DgnFontCR font, DgnFontCP shxBigFont, DPoint2dCR fontSize);
    public:     DGNPLATFORM_EXPORT                                  TextStringProperties    (TextParamWideCR, DPoint2dCR fontSize, bool is3d, DgnProjectCR);
    public:     DGNPLATFORM_EXPORT                                  TextStringProperties    (RunPropertiesBaseCR, DgnGlyphRunLayoutFlags, bool isPartOfField, bool isViewIndependent, bool is3d, TextElementJustification, StackedFractionType, StackedFractionSection);
    public:     DGNPLATFORM_EXPORT                                  TextStringProperties    (TextStringPropertiesCR);

    private:                            void                    InitDefaults            ();

    protected:                  virtual void                    _FromElementData        (TextParamWideCR, DPoint2dCR fontSize, DgnProjectCR) override;
    private:                            void                    FromElementDataInternal (TextParamWideCR, DPoint2dCR fontSize);
    protected:                  virtual void                    _ToElementData          (TextParamWideR, DgnProjectR) const override;
    private:                            void                    ToElementDataInternal   (TextParamWideR) const;
    public:                             DgnGlyphRunLayoutFlags  GetRunLayoutFlags       () const;
    public:                             void                    SetRunLayoutFlags       (DgnGlyphRunLayoutFlags);
    public:     DGNPLATFORM_EXPORT          void                    GetAxes                 (DVec3dR xAxis, DVec3dR yAxis) const;

    public:     DGNPLATFORM_EXPORT          bool                    IsPartOfField           () const;
    public:     DGNPLATFORM_EXPORT          void                    SetIsPartOfField        (bool value);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Creates a new instance from the provided parameters; the rest of the structure is zeroed.
    public: DGNPLATFORM_EXPORT static TextStringPropertiesPtr Create (DgnFontCR font, DgnFontCP shxBigFont, DPoint2dCR fontSize);

    //! Creates a new instance as a deep copy of this instance.
    public: DGNPLATFORM_EXPORT TextStringPropertiesPtr Clone () const;

    //! Determines if an underline should be drawn for the associated piece of text.
    //! This not only takes into account the underline flag, but any other relevant flags like IsVertical.
    public: DGNPLATFORM_EXPORT bool ShouldShowUnderline () const;

    //! Determines if an overline should be drawn for the associated piece of text.
    //! This not only takes into account the overline flag, but any other relevant flags like IsVertical.
    public: DGNPLATFORM_EXPORT bool ShouldShowOverline () const;

    //! Gets the vertical flag.
    //! @note   This controls MicroStation vertical text layout logic, which simply takes the glyphs from a normal font as-is, and effectively assumes soft line feeds after each character. This does not work with TrueType fonts, which have special glyphs for more correct vertical text.
    public: DGNPLATFORM_EXPORT bool IsVertical () const;

    //! Sets the vertical flag.
    //! @note   This controls MicroStation vertical text layout logic, which simply takes the glyphs from a normal font as-is, and effectively assumes soft line feeds after each character. This does not work with TrueType fonts, which have special glyphs for more correct vertical text.
    public: DGNPLATFORM_EXPORT void SetIsVertical (bool);

    //! Gets the backwards flag.
    //! @note   This is essentially a mirror about the vertical axis.
    public: DGNPLATFORM_EXPORT bool IsBackwards () const;

    //! Sets the backwards flag.
    //! @see IsBackwards for additional notes
    public: DGNPLATFORM_EXPORT void SetIsBackwards (bool);

    //! Gets the upside down flag.
    //! @note   This is essentially a mirror about the horizontal axis.
    public: DGNPLATFORM_EXPORT bool IsUpsideDown () const;

    //! Sets the upside down flag.
    //! @see IsUpsideDown for additional notes
    public: DGNPLATFORM_EXPORT void SetIsUpsideDown (bool);

    //! True if text should be view independent.
    public: DGNPLATFORM_EXPORT bool IsViewIndependent () const;

    //! True if the text is being drawn in a 3d view.
    public: DGNPLATFORM_EXPORT bool Is3d () const;

    //! Sets the Is3d flag.
    //! @see Is3d for additional notes
    public: DGNPLATFORM_EXPORT void SetIs3d (bool);

    //! Gets the justification.
    public: DGNPLATFORM_EXPORT TextElementJustification GetJustification () const;

    //! Sets the justification.
    public: DGNPLATFORM_EXPORT void SetJustification (TextElementJustification);

    //! Gets the stacked fraction type (e.g. none, stacked with no divider, diagonal, stacked with a divider).
    public: DGNPLATFORM_EXPORT StackedFractionType GetStackedFractionType () const;

    //! Sets the stacked fraction type.
    //! @see GetStackedFractionType for additional notes
    public: DGNPLATFORM_EXPORT void SetStackedFractionType(StackedFractionType);

    //! Gets the stacked fraction section (e.g. none, top, bottom).
    public: DGNPLATFORM_EXPORT StackedFractionSection GetStackedFractionSection () const;

    //! Sets the stacked fraction section.
    //! @see GetStackedFractionSection for additional notes
    public: DGNPLATFORM_EXPORT void SetStackedFractionSection(StackedFractionSection);

    //! Scales all scalar values in this instance.
    //! This includes text size, as well as spacing and offset attributes.
    public: DGNPLATFORM_EXPORT void ApplyScale (DPoint2dCR);

    }; // TextStringProperties

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
