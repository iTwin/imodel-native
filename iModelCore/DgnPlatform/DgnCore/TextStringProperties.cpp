/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/TextStringProperties.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace std;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TextStringProperties::TextStringProperties ()
    : RunPropertiesBase ()
    {
    this->InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TextStringProperties::TextStringProperties (DgnFontCR font, DgnFontCP shxBigFont, DPoint2dCR fontSize)
    : RunPropertiesBase (font, shxBigFont, fontSize)
    {
    this->InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TextStringProperties::TextStringProperties (TextParamWideCR params, DPoint2dCR fontSize, bool is3d, DgnProjectCR project) :
    RunPropertiesBase   (params, fontSize, project),
    m_layoutFlags       (GLYPH_RUN_LAYOUT_FLAG_None),
    m_is3d              (is3d)
    {
    this->FromElementDataInternal (params, fontSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TextStringProperties::TextStringProperties
(
RunPropertiesBaseCR         runProps,
DgnGlyphRunLayoutFlags      layoutFlags,
bool                        isPartOfField,
bool                        isViewIndependent,
bool                        is3d,
TextElementJustification    justification,
StackedFractionType         stackedFractionType,
StackedFractionSection      stackedFractionSection
) :
    RunPropertiesBase           (runProps),
    m_layoutFlags               (layoutFlags),
    m_isPartOfField             (isPartOfField),
    m_isViewIndependent         (isViewIndependent),
    m_is3d                      (is3d),
    m_justification             (justification),
    m_stackedFractionType       (stackedFractionType),
    m_stackedFractionSection    (stackedFractionSection)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TextStringProperties::TextStringProperties (TextStringPropertiesCR rhs) :
    T_Super (rhs),
    m_layoutFlags               (rhs.m_layoutFlags),
    m_isPartOfField             (rhs.m_isPartOfField),
    m_isViewIndependent         (rhs.m_isViewIndependent),
    m_is3d                      (rhs.m_is3d),
    m_justification             (rhs.m_justification),
    m_stackedFractionType       (rhs.m_stackedFractionType),
    m_stackedFractionSection    (rhs.m_stackedFractionSection)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextStringProperties::InitDefaults ()
    {
    m_layoutFlags               = GLYPH_RUN_LAYOUT_FLAG_None;
    m_isPartOfField             = false;
    m_isViewIndependent         = false;
    m_is3d                      = false;
    m_justification             = TextElementJustification::LeftTop;           // Seems as good as any default; TextElementJustification::Invalid is available, but is useless to the point where I want to assume this instead
    m_stackedFractionSection    = StackedFractionSection::None;
    m_stackedFractionType       = StackedFractionType::None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextStringProperties::_FromElementData (TextParamWideCR params, DPoint2dCR fontSize, DgnProjectCR project)
    {
    T_Super::_FromElementData (params, fontSize, project);
    FromElementDataInternal (params, fontSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextStringProperties::FromElementDataInternal (TextParamWideCR params, DPoint2dCR fontSize)
    {
    this->SetIsVertical (params.flags.vertical);
    this->SetIsBackwards (params.exFlags.backwards || (fontSize.x < 0.0));
    this->SetIsUpsideDown (params.exFlags.upsidedown || (fontSize.y < 0.0));
    
    m_isPartOfField             = params.exFlags.isField;
    m_isViewIndependent         = TO_BOOL (params.viewIndependent);
    m_justification             = (TextElementJustification)params.just;
    m_stackedFractionType       = (StackedFractionType)params.exFlags.stackedFractionType;
    m_stackedFractionSection    = (StackedFractionSection)params.exFlags.stackedFractionSection;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextStringProperties::_ToElementData (TextParamWideR params, DgnProjectR project) const
    {
    T_Super::_ToElementData (params, project);
    ToElementDataInternal (params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextStringProperties::ToElementDataInternal (TextParamWideR params) const
    {
    params.flags.vertical                   = this->IsVertical ();
    params.exFlags.backwards                = this->IsBackwards ();
    params.exFlags.upsidedown               = this->IsUpsideDown ();
    params.exFlags.isField                  = m_isPartOfField;
    params.viewIndependent                  = m_isViewIndependent;
    params.just                             = (int)m_justification;
    params.exFlags.stackedFractionType      = (int)m_stackedFractionType;
    params.exFlags.stackedFractionSection   = (int)m_stackedFractionSection;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TextStringPropertiesPtr TextStringProperties::Create (DgnFontCR font, DgnFontCP shxBigFont, DPoint2dCR fontSize)
    {
    return new TextStringProperties (font, shxBigFont, fontSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TextStringPropertiesPtr TextStringProperties::Clone () const
    {
    return new TextStringProperties (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool                        TextStringProperties::ShouldShowUnderline       () const                            { return m_isUnderlined && !this->IsVertical (); }
bool                        TextStringProperties::ShouldShowOverline        () const                            { return m_isOverlined && !this->IsVertical (); }
DgnGlyphRunLayoutFlags      TextStringProperties::GetRunLayoutFlags         () const                            { return m_layoutFlags; }
void                        TextStringProperties::SetRunLayoutFlags         (DgnGlyphRunLayoutFlags value)      { m_layoutFlags = value; }
bool                        TextStringProperties::IsVertical                () const                            { return (GLYPH_RUN_LAYOUT_FLAG_Vertical == (GLYPH_RUN_LAYOUT_FLAG_Vertical & m_layoutFlags)); }
void                        TextStringProperties::SetIsVertical             (bool value)                        { m_layoutFlags = (value ? (DgnGlyphRunLayoutFlags)(m_layoutFlags | GLYPH_RUN_LAYOUT_FLAG_Vertical) : (DgnGlyphRunLayoutFlags)(m_layoutFlags & ~GLYPH_RUN_LAYOUT_FLAG_Vertical)); }
bool                        TextStringProperties::IsBackwards               () const                            { return (GLYPH_RUN_LAYOUT_FLAG_Backwards == (GLYPH_RUN_LAYOUT_FLAG_Backwards & m_layoutFlags)); }
void                        TextStringProperties::SetIsBackwards            (bool value)                        { m_layoutFlags = (value ? (DgnGlyphRunLayoutFlags)(m_layoutFlags | GLYPH_RUN_LAYOUT_FLAG_Backwards) : (DgnGlyphRunLayoutFlags)(m_layoutFlags & ~GLYPH_RUN_LAYOUT_FLAG_Backwards)); }
bool                        TextStringProperties::IsUpsideDown              () const                            { return (GLYPH_RUN_LAYOUT_FLAG_UpsideDown == (GLYPH_RUN_LAYOUT_FLAG_UpsideDown & m_layoutFlags)); }
void                        TextStringProperties::SetIsUpsideDown           (bool value)                        { m_layoutFlags = (value ? (DgnGlyphRunLayoutFlags)(m_layoutFlags | GLYPH_RUN_LAYOUT_FLAG_UpsideDown) : (DgnGlyphRunLayoutFlags)(m_layoutFlags & ~GLYPH_RUN_LAYOUT_FLAG_UpsideDown)); }
bool                        TextStringProperties::IsPartOfField             () const                            { return m_isPartOfField; }
void                        TextStringProperties::SetIsPartOfField          (bool value)                        { m_isPartOfField = value; }
bool                        TextStringProperties::IsViewIndependent         () const                            { return m_isViewIndependent; }
bool                        TextStringProperties::Is3d                      () const                            { return m_is3d; }
void                        TextStringProperties::SetIs3d                   (bool value)                        { m_is3d = value; }
TextElementJustification    TextStringProperties::GetJustification          () const                            { return m_justification; }
void                        TextStringProperties::SetJustification          (TextElementJustification value)    { m_justification = value; }
StackedFractionType         TextStringProperties::GetStackedFractionType    () const                            { return m_stackedFractionType; }
void                        TextStringProperties::SetStackedFractionType    (StackedFractionType value)         { m_stackedFractionType = value; }
StackedFractionSection      TextStringProperties::GetStackedFractionSection () const                            { return m_stackedFractionSection; }
void                        TextStringProperties::SetStackedFractionSection (StackedFractionSection value)      { m_stackedFractionSection = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextStringProperties::ApplyScale (DPoint2dCR scaleFactor)
    {
    T_Super::ApplyScale (scaleFactor, this->IsVertical ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextStringProperties::GetAxes (DVec3dR xAxis, DVec3dR yAxis) const
    {
    DPoint2d size = this->GetDisplaySize ();

    // 2D backward/up-side down text stores flags and negative font sizes; 3D stores flags, positive font sizes, and a mirrored matrix.
    //  At runtime, font size is always positive.
    //  For purposes of glyph layout, we want it to reverse the flow of text only for 2D with backwards/up-side down; it should not for 3D, since the transform will effectively do it.
    
    if (this->IsBackwards () && !m_is3d)
        size.x = -size.x;

    if (this->IsUpsideDown () && !m_is3d)
        size.y = -size.y;
    
    xAxis.init (size.x, 0.0, 0.0);
    yAxis.init (0.0, size.y, 0.0);

    if (IsItalic () && !ShouldUseItalicTypeface ())
        yAxis.x = tan (m_customSlantAngle) * size.y;
    }
