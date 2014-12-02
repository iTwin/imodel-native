/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/RunProperties.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace std;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- RunProperties::Overrides ------------------------------------------------------------------------------------------- RunProperties::Overrides --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/08
//---------------------------------------------------------------------------------------
RunProperties::Overrides::Overrides ()
    {
    Clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/08
//---------------------------------------------------------------------------------------
bool RunProperties::Overrides::Equals (RunProperties::Overrides const & rhs) const
    {
    if (m_font                      != rhs.m_font)                      return false;
    if (m_shxBigFont                != rhs.m_shxBigFont)                return false;
    if (m_hasColor                  != rhs.m_hasColor)                  return false;
    if (m_color                     != rhs.m_color)                     return false;
    if (m_isBold                    != rhs.m_isBold)                    return false;
    if (m_isItalic                  != rhs.m_isItalic)                  return false;
    if (m_customSlantAngle          != rhs.m_customSlantAngle)          return false;
    if (m_isUnderlined              != rhs.m_isUnderlined)              return false;
    if (m_shouldUseUnderlineStyle   != rhs.m_shouldUseUnderlineStyle)   return false;
    if (m_underlineOffset           != rhs.m_underlineOffset)           return false;
    if (m_underlineColor            != rhs.m_underlineColor)            return false;
    if (m_underlineLineStyle        != rhs.m_underlineLineStyle)        return false;
    if (m_underlineWeight           != rhs.m_underlineWeight)           return false;
    if (m_isOverlined               != rhs.m_isOverlined)               return false;
    if (m_shouldUseOverlineStyle    != rhs.m_shouldUseOverlineStyle)    return false;
    if (m_overlineOffset            != rhs.m_overlineOffset)            return false;
    if (m_overlineColor             != rhs.m_overlineColor)             return false;
    if (m_overlineLineStyle         != rhs.m_overlineLineStyle)         return false;
    if (m_overlineWeight            != rhs.m_overlineWeight)            return false;
    if (m_characterSpacingType      != rhs.m_characterSpacingType)      return false;
    if (m_characterSpacingValue     != rhs.m_characterSpacingValue)     return false;
    if (m_shouldUseBackground       != rhs.m_shouldUseBackground)       return false;
    if (m_backgroundFillColor       != rhs.m_backgroundFillColor)       return false;
    if (m_backgroundBorderColor     != rhs.m_backgroundBorderColor)     return false;
    if (m_backgroundBorderLineStyle != rhs.m_backgroundBorderLineStyle) return false;
    if (m_backgroundBorderWeight    != rhs.m_backgroundBorderWeight)    return false;
    if (m_backgroundBorderPadding   != rhs.m_backgroundBorderPadding)   return false;
    if (m_runOffset                 != rhs.m_runOffset)                 return false;
    if (m_isSubScript               != rhs.m_isSubScript)               return false;
    if (m_isSuperScript             != rhs.m_isSuperScript)             return false;
    if (m_width                     != rhs.m_width)                     return false;
    if (m_height                    != rhs.m_height)                    return false;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/08
//---------------------------------------------------------------------------------------
void RunProperties::Overrides::Clear ()
    {
    m_font                      = false;
    m_shxBigFont                = false;
    m_hasColor                  = false;
    m_color                     = false;
    m_isBold                    = false;
    m_isItalic                  = false;
    m_customSlantAngle          = false;
    m_isUnderlined              = false;
    m_shouldUseUnderlineStyle   = false;
    m_underlineOffset           = false;
    m_underlineColor            = false;
    m_underlineLineStyle        = false;
    m_underlineWeight           = false;
    m_isOverlined               = false;
    m_shouldUseOverlineStyle    = false;
    m_overlineOffset            = false;
    m_overlineColor             = false;
    m_overlineLineStyle         = false;
    m_overlineWeight            = false;
    m_characterSpacingType      = false;
    m_characterSpacingValue     = false;
    m_shouldUseBackground       = false;
    m_backgroundFillColor       = false;
    m_backgroundBorderColor     = false;
    m_backgroundBorderLineStyle = false;
    m_backgroundBorderWeight    = false;
    m_backgroundBorderPadding   = false;
    m_runOffset                 = false;
    m_isSubScript               = false;
    m_isSuperScript             = false;
    m_width                     = false;
    m_height                    = false;
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- RunProperties ----------------------------------------------------------------------------------------------------------------- RunProperties --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2009
//---------------------------------------------------------------------------------------
template<class T>
static bool clearPropertyOverride (bool& overrideFlag, T& value, DgnTextStylePtr textStyle, DgnTextStyleProperty tsProp)
    {
    overrideFlag = false;
    
    if (!textStyle.IsValid ())
        return false;
    
    textStyle->GetPropertyValue (tsProp, value);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2009
//---------------------------------------------------------------------------------------
static void clearFactorPropertyOverride (bool& overrideFlag, double& value, DgnTextStylePtr textStyle, DgnTextStyleProperty tsProp, DgnModelR dgnModel)
    {
    if (!clearPropertyOverride (overrideFlag, value, textStyle, tsProp))
        return;

    double scale;
    textStyle->GetPropertyValue (DgnTextStyleProperty::Height, scale);
            
    value *= scale;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2009
//---------------------------------------------------------------------------------------
static void clearFactorPropertyOverride (bool& overrideFlag, DPoint2dR value, DgnTextStylePtr textStyle, DgnTextStyleProperty tsProp, DgnModelR dgnModel)
    {
    if (!clearPropertyOverride (overrideFlag, value, textStyle, tsProp))
        return;

    double scale;
    textStyle->GetPropertyValue (DgnTextStyleProperty::Height, scale);
            
    value.x *= scale;
    value.y *= scale;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2009
//---------------------------------------------------------------------------------------
static void clearFontSizePropertyOverride (bool& overrideFlag, double& value, DgnTextStylePtr textStyle, DgnTextStyleProperty tsProp, DgnModelR dgnModel)
    {
    if (!clearPropertyOverride (overrideFlag, value, textStyle, tsProp))
        return;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
IDgnTextStyleApplyable const &  RunProperties::AsIDgnTextStyleApplyable                 () const                { return *this; }
IDgnTextStyleApplyable&         RunProperties::AsIDgnTextStyleApplyableR                ()                      { return *this; }
DgnModelR                       RunProperties::GetDgnModelR                             () const                { return *m_dgnModel; }
LineAlignment                   RunProperties::GetLineAlignment                         () const                { return m_lineAlignment; }
void                            RunProperties::SetLineAlignment                         (LineAlignment value)   { m_lineAlignment = value; }
bool                            RunProperties::IsFontOverridden                         () const                { return m_overrides.m_font; }
void                            RunProperties::ClearFontOverride                        ()                      { clearPropertyOverride (m_overrides.m_font, m_font, this->GetTextStyleInFile (), DgnTextStyleProperty::Font); }
bool                            RunProperties::IsShxBigFontOverridden                   () const                { return m_overrides.m_shxBigFont; }
void                            RunProperties::ClearShxBigFontOverride                  ()                      { clearPropertyOverride (m_overrides.m_shxBigFont, m_shxBigFont, this->GetTextStyleInFile (), DgnTextStyleProperty::ShxBigFont); }
bool                            RunProperties::IsHasColorOverridden                     () const                { return m_overrides.m_hasColor; }
void                            RunProperties::ClearHasColorOverride                    ()                      { clearPropertyOverride (m_overrides.m_hasColor, m_hasColor, this->GetTextStyleInFile (), DgnTextStyleProperty::HasColor); }
bool                            RunProperties::IsColorOverridden                        () const                { return m_overrides.m_color; }
void                            RunProperties::ClearColorOverride                       ()                      { clearPropertyOverride (m_overrides.m_color, m_color, this->GetTextStyleInFile (), DgnTextStyleProperty::Color); }
bool                            RunProperties::IsBoldOverridden                         () const                { return m_overrides.m_isBold; }
void                            RunProperties::ClearIsBoldOverride                      ()                      { clearPropertyOverride (m_overrides.m_isBold, m_isBold, this->GetTextStyleInFile (), DgnTextStyleProperty::IsBold); }
bool                            RunProperties::IsItalicOverridden                       () const                { return m_overrides.m_isItalic; }
void                            RunProperties::ClearIsItalicOverride                    ()                      { clearPropertyOverride (m_overrides.m_isItalic, m_isItalic, this->GetTextStyleInFile (), DgnTextStyleProperty::IsItalics); }
bool                            RunProperties::IsCustomSlantAngleOverridden             () const                { return m_overrides.m_customSlantAngle; }
void                            RunProperties::ClearCustomSlantAngleOverride            ()                      { clearPropertyOverride (m_overrides.m_customSlantAngle, m_customSlantAngle, this->GetTextStyleInFile (), DgnTextStyleProperty::CustomSlantAngle); }
bool                            RunProperties::IsUnderlinedOverridden                   () const                { return m_overrides.m_isUnderlined; }
void                            RunProperties::ClearIsUnderlinedOverride                ()                      { clearPropertyOverride (m_overrides.m_isUnderlined, m_isUnderlined, this->GetTextStyleInFile (), DgnTextStyleProperty::IsUnderlined); }
bool                            RunProperties::IsShouldUseUnderlineStyleOverridden      () const                { return m_overrides.m_shouldUseUnderlineStyle; }
void                            RunProperties::ClearShouldUseUnderlineStyleOverride     ()                      { clearPropertyOverride (m_overrides.m_shouldUseUnderlineStyle, m_shouldUseUnderlineStyle, this->GetTextStyleInFile (), DgnTextStyleProperty::ShouldUseUnderlineStyle); }
bool                            RunProperties::IsUnderlineOffsetOverridden              () const                { return m_overrides.m_underlineOffset; }
void                            RunProperties::ClearUnderlineOffsetOverride             ()                      { clearFactorPropertyOverride (m_overrides.m_underlineOffset, m_underlineOffset, this->GetTextStyleInFile (), DgnTextStyleProperty::UnderlineOffsetFactor, *m_dgnModel); }
bool                            RunProperties::IsUnderlineColorOverridden               () const                { return m_overrides.m_underlineColor; }
void                            RunProperties::ClearUnderlineColorOverride              ()                      { clearPropertyOverride (m_overrides.m_underlineColor, m_underlineColor, this->GetTextStyleInFile (), DgnTextStyleProperty::UnderlineColor); }
bool                            RunProperties::IsUnderlineLineStyleOverridden           () const                { return m_overrides.m_underlineLineStyle; }
void                            RunProperties::ClearUnderlineLineStyleOverride          ()                      { clearPropertyOverride (m_overrides.m_underlineLineStyle, m_underlineLineStyle, this->GetTextStyleInFile (), DgnTextStyleProperty::UnderlineLineStyle); }
bool                            RunProperties::IsUnderlineWeightOverridden              () const                { return m_overrides.m_underlineWeight; }
void                            RunProperties::ClearUnderlineWeightOverride             ()                      { clearPropertyOverride (m_overrides.m_underlineWeight, m_underlineWeight, this->GetTextStyleInFile (), DgnTextStyleProperty::UnderlineWeight); }
bool                            RunProperties::IsOverlinedOverridden                    () const                { return m_overrides.m_isOverlined; }
void                            RunProperties::ClearIsOverlinedOverride                 ()                      { clearPropertyOverride (m_overrides.m_isOverlined, m_isOverlined, this->GetTextStyleInFile (), DgnTextStyleProperty::IsOverlined); }
bool                            RunProperties::IsShouldUseOverlineStyleOverridden       () const                { return m_overrides.m_shouldUseOverlineStyle; }
void                            RunProperties::ClearShouldUseOverlineStyleOverride      ()                      { clearPropertyOverride (m_overrides.m_shouldUseOverlineStyle, m_shouldUseOverlineStyle, this->GetTextStyleInFile (), DgnTextStyleProperty::ShouldUseOverlineStyle); }
bool                            RunProperties::IsOverlineOffsetOverridden               () const                { return m_overrides.m_overlineOffset; }
void                            RunProperties::ClearOverlineOffsetOverride              ()                      { clearFactorPropertyOverride (m_overrides.m_overlineOffset, m_overlineOffset, this->GetTextStyleInFile (), DgnTextStyleProperty::OverlineOffsetFactor, *m_dgnModel); }
bool                            RunProperties::IsOverlineColorOverridden                () const                { return m_overrides.m_overlineColor; }
void                            RunProperties::ClearOverlineColorOverride               ()                      { clearPropertyOverride (m_overrides.m_overlineColor, m_overlineColor, this->GetTextStyleInFile (), DgnTextStyleProperty::OverlineColor); }
bool                            RunProperties::IsOverlineLineStyleOverridden            () const                { return m_overrides.m_overlineLineStyle; }
void                            RunProperties::ClearOverlineLineStyleOverride           ()                      { clearPropertyOverride (m_overrides.m_overlineLineStyle, m_overlineLineStyle, this->GetTextStyleInFile (), DgnTextStyleProperty::OverlineLineStyle); }
bool                            RunProperties::IsOverlineWeightOverridden               () const                { return m_overrides.m_overlineWeight; }
void                            RunProperties::ClearOverlineWeightOverride              ()                      { clearPropertyOverride (m_overrides.m_overlineWeight, m_overlineWeight, this->GetTextStyleInFile (), DgnTextStyleProperty::OverlineWeight); }
bool                            RunProperties::IsCharacterSpacingTypeOverridden         () const                { return m_overrides.m_characterSpacingType; }
void                            RunProperties::ClearCharacterSpacingTypeOverride        ()                      { clearPropertyOverride (m_overrides.m_characterSpacingType, (UInt32&)m_characterSpacingType, this->GetTextStyleInFile (), DgnTextStyleProperty::CharacterSpacingType); }
bool                            RunProperties::IsCharacterSpacingValueOverridden        () const                { return m_overrides.m_characterSpacingValue; }
void                            RunProperties::ClearCharacterSpacingValueOverride       ()                      { clearFactorPropertyOverride (m_overrides.m_characterSpacingValue, m_characterSpacingValue, this->GetTextStyleInFile (), DgnTextStyleProperty::CharacterSpacingValueFactor, *m_dgnModel); }
bool                            RunProperties::IsShouldUseBackgroundOverridden          () const                { return m_overrides.m_shouldUseBackground; }
void                            RunProperties::ClearShouldUseBackgroundOverride         ()                      { clearPropertyOverride (m_overrides.m_shouldUseBackground, m_shouldUseBackground, this->GetTextStyleInFile (), DgnTextStyleProperty::ShouldUseBackground); }
bool                            RunProperties::IsBackgroundFillColorOverridden          () const                { return m_overrides.m_backgroundFillColor; }
void                            RunProperties::ClearBackgroundFillColorOverride         ()                      { clearPropertyOverride (m_overrides.m_backgroundFillColor, m_backgroundFillColor, this->GetTextStyleInFile (), DgnTextStyleProperty::BackgroundFillColor); }
bool                            RunProperties::IsBackgroundBorderColorOverridden        () const                { return m_overrides.m_backgroundBorderColor; }
void                            RunProperties::ClearBackgroundBorderColorOverride       ()                      { clearPropertyOverride (m_overrides.m_backgroundBorderColor, m_backgroundBorderColor, this->GetTextStyleInFile (), DgnTextStyleProperty::BackgroundBorderColor); }
bool                            RunProperties::IsBackgroundBorderLineStyleOverridden    () const                { return m_overrides.m_backgroundBorderLineStyle; }
void                            RunProperties::ClearBackgroundBorderLineStyleOverride   ()                      { clearPropertyOverride (m_overrides.m_backgroundBorderLineStyle, m_backgroundBorderLineStyle, this->GetTextStyleInFile (), DgnTextStyleProperty::BackgroundBorderLineStyle); }
bool                            RunProperties::IsBackgroundBorderWeightOverridden       () const                { return m_overrides.m_backgroundBorderWeight; }
void                            RunProperties::ClearBackgroundBorderWeightOverride      ()                      { clearPropertyOverride (m_overrides.m_backgroundBorderWeight, m_backgroundBorderWeight, this->GetTextStyleInFile (), DgnTextStyleProperty::BackgroundBorderWeight); }
bool                            RunProperties::IsBackgroundBorderPaddingOverridden      () const                { return m_overrides.m_backgroundBorderPadding; }
void                            RunProperties::ClearBackgroundBorderPaddingOverride     ()                      { clearFactorPropertyOverride (m_overrides.m_backgroundBorderPadding, m_backgroundBorderPadding, this->GetTextStyleInFile (), DgnTextStyleProperty::BackgroundBorderPaddingFactor, *m_dgnModel); }
bool                            RunProperties::IsRunOffsetOverridden                    () const                { return m_overrides.m_runOffset; }
void                            RunProperties::ClearRunOffsetOverride                   ()                      { clearFactorPropertyOverride (m_overrides.m_runOffset, m_runOffset, this->GetTextStyleInFile (), DgnTextStyleProperty::RunOffsetFactor, *m_dgnModel); }
bool                            RunProperties::IsSubScriptOverridden                    () const                { return m_overrides.m_isSubScript; }
void                            RunProperties::ClearIsSubScriptOverride                 ()                      { clearPropertyOverride (m_overrides.m_isSubScript, m_isSubScript, this->GetTextStyleInFile (), DgnTextStyleProperty::IsSubScript); }
bool                            RunProperties::IsSuperScriptOverridden                  () const                { return m_overrides.m_isSuperScript; }
void                            RunProperties::ClearIsSuperScriptOverride               ()                      { clearPropertyOverride (m_overrides.m_isSuperScript, m_isSuperScript, this->GetTextStyleInFile (), DgnTextStyleProperty::IsSuperScript); }
bool                            RunProperties::IsWidthOverridden                        () const                { return m_overrides.m_width; }
void                            RunProperties::ClearWidthOverride                       ()                      { clearFontSizePropertyOverride (m_overrides.m_width, m_fontSize.x, this->GetTextStyleInFile (), DgnTextStyleProperty::Width, *m_dgnModel); }
bool                            RunProperties::IsHeightOverridden                       () const                { return m_overrides.m_height; }
void                            RunProperties::ClearHeightOverride                      ()                      { clearFontSizePropertyOverride (m_overrides.m_height, m_fontSize.y, this->GetTextStyleInFile (), DgnTextStyleProperty::Height, *m_dgnModel); }
bool                            RunProperties::_HasTextStyle                            () const                { return (0 != m_textStyleId && m_dgnModel->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(m_textStyleId)).IsValid ()); }
UInt32                          RunProperties::_GetTextStyleId                          () const                { return m_textStyleId; }
DgnTextStylePtr                 RunProperties::_GetTextStyleInFile                      () const                { return m_dgnModel->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(m_textStyleId)); }
RunPropertiesPtr                RunProperties::Clone                                    () const                { return new RunProperties (*this); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
RunProperties::RunProperties (DgnModelR dgnCache) :
    RunPropertiesBase ()
    {
    InitDefaults (dgnCache);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
RunProperties::RunProperties (DgnFontCR font, DgnFontCP shxBigFont, DPoint2dCR fontSize, DgnModelR dgnCache) :
    RunPropertiesBase (font, shxBigFont, fontSize)
    {
    InitDefaults (dgnCache);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
RunProperties::RunProperties (TextParamWideCR params, DPoint2dCR fontSize, DgnModelR dgnCache) :
    RunPropertiesBase (params, fontSize, dgnCache.GetDgnProject ()),
    m_dgnModel (&dgnCache)
    {
    FromElementDataInternal (params);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
RunProperties::RunProperties (DgnTextStyleCR textStyle, DgnModelR dgnCache) :
    RunPropertiesBase ()
    {
    InitDefaults (dgnCache);
    ApplyTextStyle (textStyle, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
RunProperties::RunProperties (RunPropertiesCR rhs) :
    RunPropertiesBase (rhs),
    m_dgnModel      (rhs.m_dgnModel),
    m_overrides     (rhs.m_overrides),
    m_lineAlignment (rhs.m_lineAlignment),
    m_textStyleId   (rhs.m_textStyleId)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunProperties::InitDefaults (DgnModelR dgnCache)
    {
    m_dgnModel          = &dgnCache;
    m_lineAlignment     = LINE_ALIGNMENT_Bottom; // Seems like as good a default as any.
    m_textStyleId       = 0;
    
    // RunProperties::Overrides constructor zeros itself out; depend on that.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
RunPropertiesPtr    RunProperties::Create   (TextParamWideCR params, DPoint2dCR fontSize, DgnModelR model)  { return new RunProperties (params, fontSize, model); }
RunPropertiesPtr    RunProperties::Create   (DgnFontCR font, DPoint2dCR fontSize, DgnModelR dgnCache)       { return new RunProperties (font, NULL, fontSize, dgnCache); }
RunPropertiesPtr    RunProperties::Create   (DgnTextStyleCR textStyle, DgnModelR dgnCache)                  { return new RunProperties (textStyle, dgnCache); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunProperties::_FromElementData (TextParamWideCR params, DPoint2dCR fontSize, DgnProjectCR project)
    {
    T_Super::_FromElementData (params, fontSize, project);
    FromElementDataInternal (params);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunProperties::FromElementDataInternal (TextParamWideCR params)
    {
    m_lineAlignment = (LineAlignment)params.renderingFlags.lineAlignment;
    
    DgnTextStylePtr styleObj;
    
    if (params.flags.textStyle && (0 != params.textStyleId) && (styleObj = m_dgnModel->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(params.textStyleId))).IsValid ())
        {
        m_textStyleId = params.textStyleId;
        }
    else
        {
        m_textStyleId = 0;
        m_overrides.Clear ();
        return;
        }
    
    m_overrides.m_font      = params.overridesFromStyle.fontNo;
    m_overrides.m_shxBigFont   = params.overridesFromStyle.shxBigFont;
    
    // Superclass processing can correct bad font data (e.g. attempting to set big font to something that isn't a big font), so double-check.
    if (!m_overrides.m_font)
        {
        DgnFontCP styleFont;
        styleObj->GetPropertyValue (DgnTextStyleProperty::Font, styleFont);
        
        m_overrides.m_font = (m_font != styleFont);
        }
    
    if (!m_overrides.m_shxBigFont)
        {
        DgnFontCP styleShxBigFont;
        styleObj->GetPropertyValue (DgnTextStyleProperty::ShxBigFont, styleShxBigFont);
        
        m_overrides.m_shxBigFont = (m_shxBigFont != styleShxBigFont);
        }
    
    m_overrides.m_width                     = params.overridesFromStyle.width;
    m_overrides.m_height                    = params.overridesFromStyle.height;
    m_overrides.m_customSlantAngle          = params.overridesFromStyle.slant;
    m_overrides.m_characterSpacingValue     = params.overridesFromStyle.interCharSpacing;
    m_overrides.m_underlineOffset           = params.overridesFromStyle.underlineOffset;
    m_overrides.m_overlineOffset            = params.overridesFromStyle.overlineOffset;
    m_overrides.m_isUnderlined              = params.overridesFromStyle.underline;
    m_overrides.m_isOverlined               = params.overridesFromStyle.overline;
    m_overrides.m_isItalic                  = params.overridesFromStyle.italics;
    m_overrides.m_isBold                    = params.overridesFromStyle.bold;
    m_overrides.m_isSuperScript             = params.overridesFromStyle.superscript;
    m_overrides.m_isSubScript               = params.overridesFromStyle.subscript;
    m_overrides.m_characterSpacingType      = params.overridesFromStyle.fixedSpacing || params.overridesFromStyle.acadInterCharSpacing;
    m_overrides.m_shouldUseBackground       = params.overridesFromStyle.background;
    m_overrides.m_backgroundBorderLineStyle = params.overridesFromStyle.backgroundstyle;
    m_overrides.m_backgroundBorderWeight    = params.overridesFromStyle.backgroundweight;
    m_overrides.m_backgroundBorderColor     = params.overridesFromStyle.backgroundcolor;
    m_overrides.m_backgroundFillColor       = params.overridesFromStyle.backgroundfillcolor;
    m_overrides.m_backgroundBorderPadding   = params.overridesFromStyle.backgroundborder;
    m_overrides.m_underlineLineStyle        = params.overridesFromStyle.underlinestyle;
    m_overrides.m_underlineWeight           = params.overridesFromStyle.underlineweight;
    m_overrides.m_underlineColor            = params.overridesFromStyle.underlinecolor;
    m_overrides.m_overlineLineStyle         = params.overridesFromStyle.overlinestyle;
    m_overrides.m_overlineWeight            = params.overridesFromStyle.overlineweight;
    m_overrides.m_overlineColor             = params.overridesFromStyle.overlinecolor;
    m_overrides.m_runOffset                 = params.overridesFromStyle.lineOffset;
    m_overrides.m_shouldUseOverlineStyle    = params.overridesFromStyle.overlinestyleflag;
    m_overrides.m_shouldUseUnderlineStyle   = params.overridesFromStyle.underlinestyleflag;
    m_overrides.m_color                     = params.overridesFromStyle.color;
    m_overrides.m_hasColor                  = params.overridesFromStyle.colorFlag;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunProperties::_ToElementData (TextParamWideR params, DgnProjectR project) const
    {
    T_Super::_ToElementData (params, project);
    ToElementDataInternal (params);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunProperties::ToElementDataInternal (TextParamWideR params) const
    {
    params.renderingFlags.lineAlignment = m_lineAlignment;
    
    if (this->HasTextStyle ())
        {
        params.textStyleId      = m_textStyleId;
        params.flags.textStyle  = true;
        }
    else
        {
        params.textStyleId      = 0;
        params.flags.textStyle  = false;
        }
    
    params.overridesFromStyle.fontNo                = m_overrides.m_font;
    params.overridesFromStyle.shxBigFont            = m_overrides.m_shxBigFont;
    params.overridesFromStyle.width                 = m_overrides.m_width;
    params.overridesFromStyle.height                = m_overrides.m_height;
    params.overridesFromStyle.slant                 = m_overrides.m_customSlantAngle;
    params.overridesFromStyle.interCharSpacing      = m_overrides.m_characterSpacingValue;
    params.overridesFromStyle.underlineOffset       = m_overrides.m_underlineOffset;
    params.overridesFromStyle.overlineOffset        = m_overrides.m_overlineOffset;
    params.overridesFromStyle.underline             = m_overrides.m_isUnderlined;
    params.overridesFromStyle.overline              = m_overrides.m_isOverlined;
    params.overridesFromStyle.italics               = m_overrides.m_isItalic;
    params.overridesFromStyle.bold                  = m_overrides.m_isBold;
    params.overridesFromStyle.superscript           = m_overrides.m_isSuperScript;
    params.overridesFromStyle.subscript             = m_overrides.m_isSubScript;
    params.overridesFromStyle.fixedSpacing          = m_overrides.m_characterSpacingType && CharacterSpacingType::FixedWidth == m_characterSpacingType;
    params.overridesFromStyle.background            = m_overrides.m_shouldUseBackground;
    params.overridesFromStyle.backgroundstyle       = m_overrides.m_backgroundBorderLineStyle;
    params.overridesFromStyle.backgroundweight      = m_overrides.m_backgroundBorderWeight;
    params.overridesFromStyle.backgroundcolor       = m_overrides.m_backgroundBorderColor;
    params.overridesFromStyle.backgroundfillcolor   = m_overrides.m_backgroundFillColor;
    params.overridesFromStyle.backgroundborder      = m_overrides.m_backgroundBorderPadding;
    params.overridesFromStyle.underlinestyle        = m_overrides.m_underlineLineStyle;
    params.overridesFromStyle.underlineweight       = m_overrides.m_underlineWeight;
    params.overridesFromStyle.underlinecolor        = m_overrides.m_underlineColor;
    params.overridesFromStyle.overlinestyle         = m_overrides.m_overlineLineStyle;
    params.overridesFromStyle.overlineweight        = m_overrides.m_overlineWeight;
    params.overridesFromStyle.overlinecolor         = m_overrides.m_overlineColor;
    params.overridesFromStyle.lineOffset            = m_overrides.m_runOffset;
    params.overridesFromStyle.overlinestyleflag     = m_overrides.m_shouldUseOverlineStyle;
    params.overridesFromStyle.underlinestyleflag    = m_overrides.m_shouldUseUnderlineStyle;
    params.overridesFromStyle.color                 = m_overrides.m_color;
    params.overridesFromStyle.colorFlag             = m_overrides.m_hasColor;
    params.overridesFromStyle.acadInterCharSpacing  = m_overrides.m_characterSpacingType && CharacterSpacingType::Factor == m_characterSpacingType;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void RunProperties::_ToStyle (DgnTextStyleR style) const
    {
    double  heightFactor    = m_fontSize.y;
    double  normalFactor    = heightFactor;
    
    DPoint2d    backgroundBorderPadding;    backgroundBorderPadding.Scale (m_backgroundBorderPadding, (1.0 / normalFactor));
    DPoint2d    runOffset;                  runOffset.Scale (m_runOffset, (1.0 / normalFactor));
    double      icSpacingValue              = (CharacterSpacingType::Factor == m_characterSpacingType) ? m_characterSpacingValue : (m_characterSpacingValue / normalFactor);
    
    style.SetPropertyValue(DgnTextStyleProperty::Font, (DgnFontCP)m_font);
    style.SetPropertyValue(DgnTextStyleProperty::ShxBigFont, (DgnFontCP)m_shxBigFont);
    style.SetPropertyValue(DgnTextStyleProperty::HasColor, (bool)m_hasColor);
    style.SetPropertyValue(DgnTextStyleProperty::Color, (UInt32)m_color);
    style.SetPropertyValue(DgnTextStyleProperty::IsBold, (bool)m_isBold);
    style.SetPropertyValue(DgnTextStyleProperty::IsItalics, (bool)m_isItalic);
    style.SetPropertyValue(DgnTextStyleProperty::CustomSlantAngle, (double)m_customSlantAngle);
    style.SetPropertyValue(DgnTextStyleProperty::IsUnderlined, (bool)m_isUnderlined);
    style.SetPropertyValue(DgnTextStyleProperty::ShouldUseUnderlineStyle, (bool)m_shouldUseUnderlineStyle);
    style.SetPropertyValue(DgnTextStyleProperty::UnderlineOffsetFactor, (double)(m_underlineOffset / normalFactor));
    style.SetPropertyValue(DgnTextStyleProperty::UnderlineColor, (UInt32)m_underlineColor);
    style.SetPropertyValue(DgnTextStyleProperty::UnderlineLineStyle, m_underlineLineStyle);
    style.SetPropertyValue(DgnTextStyleProperty::UnderlineWeight, (UInt32)m_underlineWeight);
    style.SetPropertyValue(DgnTextStyleProperty::IsOverlined, (bool)m_isOverlined);
    style.SetPropertyValue(DgnTextStyleProperty::ShouldUseOverlineStyle, (bool)m_shouldUseOverlineStyle);
    style.SetPropertyValue(DgnTextStyleProperty::OverlineOffsetFactor, (double)(m_overlineOffset / normalFactor));
    style.SetPropertyValue(DgnTextStyleProperty::OverlineColor, (UInt32)m_overlineColor);
    style.SetPropertyValue(DgnTextStyleProperty::OverlineLineStyle, m_overlineLineStyle);
    style.SetPropertyValue(DgnTextStyleProperty::OverlineWeight, (UInt32)m_overlineWeight);
    style.SetPropertyValue(DgnTextStyleProperty::ShouldUseBackground, (bool)m_shouldUseBackground);
    style.SetPropertyValue(DgnTextStyleProperty::BackgroundFillColor, (UInt32)m_backgroundFillColor);
    style.SetPropertyValue(DgnTextStyleProperty::BackgroundBorderColor, (UInt32)m_backgroundBorderColor);
    style.SetPropertyValue(DgnTextStyleProperty::BackgroundBorderLineStyle, m_backgroundBorderLineStyle);
    style.SetPropertyValue(DgnTextStyleProperty::BackgroundBorderWeight, (UInt32)m_backgroundBorderWeight);
    style.SetPropertyValue(DgnTextStyleProperty::IsSubScript, (bool)m_isSubScript);
    style.SetPropertyValue(DgnTextStyleProperty::IsSuperScript, (bool)m_isSuperScript);
    style.SetPropertyValue(DgnTextStyleProperty::Width, (double)(m_fontSize.x));
    style.SetPropertyValue(DgnTextStyleProperty::Height, (double)(m_fontSize.y));
    style.SetPropertyValue(DgnTextStyleProperty::BackgroundBorderPaddingFactor, (DPoint2dCR)backgroundBorderPadding);
    style.SetPropertyValue(DgnTextStyleProperty::RunOffsetFactor, (DPoint2dCR)runOffset);
    style.SetPropertyValue(DgnTextStyleProperty::CharacterSpacingValueFactor, (double)icSpacingValue);
    style.SetPropertyValue(DgnTextStyleProperty::CharacterSpacingType, (UInt32)m_characterSpacingType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
bool RunProperties::Equals (RunPropertiesCR rhs) const { return this->Equals (rhs, 0.0, true); }
bool RunProperties::Equals (RunPropertiesCR rhs, double tolerance, bool shouldIgnoreElementOverhead) const
    {
    if (!RunPropertiesBase::Equals (rhs, tolerance, shouldIgnoreElementOverhead))
        return false;
    
    // Purposefully not comparing DgnModels... it would be a lot of extra work to remap everything,
    //  and I just don't think it will affect many cases. Even in the QATools case where we have
    //  difference DgnModels, they should still have the same units and color table etc.
    
    if (m_lineAlignment != rhs.m_lineAlignment) return false;
    if (m_textStyleId   != rhs.m_textStyleId)   return false;
    
    if (!m_overrides.Equals (rhs.m_overrides))
        return false;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunProperties::_SetIsBold                  (bool value)                    { T_Super::_SetIsBold (value);                  if (this->HasTextStyle ()) m_overrides.m_isBold                     = true; }
void RunProperties::_SetIsItalic                (bool value)                    { T_Super::_SetIsItalic (value);                if (this->HasTextStyle ()) m_overrides.m_isItalic                   = true; }
void RunProperties::_SetCustomSlantAngle        (double value)                  { T_Super::_SetCustomSlantAngle (value);        if (this->HasTextStyle ()) m_overrides.m_customSlantAngle           = true; }
void RunProperties::_SetIsUnderlined            (bool value)                    { T_Super::_SetIsUnderlined (value);            if (this->HasTextStyle ()) m_overrides.m_isUnderlined               = true; }
void RunProperties::_SetShouldUseUnderlineStyle (bool value)                    { T_Super::_SetShouldUseUnderlineStyle (value); if (this->HasTextStyle ()) m_overrides.m_shouldUseUnderlineStyle    = true; }
void RunProperties::_SetUnderlineOffset         (double value)                  { T_Super::_SetUnderlineOffset (value);         if (this->HasTextStyle ()) m_overrides.m_underlineOffset            = true; }
void RunProperties::_SetIsOverlined             (bool value)                    { T_Super::_SetIsOverlined (value);             if (this->HasTextStyle ()) m_overrides.m_isOverlined                = true; }
void RunProperties::_SetShouldUseOverlineStyle  (bool value)                    { T_Super::_SetShouldUseOverlineStyle (value);  if (this->HasTextStyle ()) m_overrides.m_shouldUseOverlineStyle     = true; }
void RunProperties::_SetOverlineOffset          (double value)                  { T_Super::_SetOverlineOffset (value);          if (this->HasTextStyle ()) m_overrides.m_overlineOffset             = true; }
void RunProperties::_SetCharacterSpacingType    (CharacterSpacingType value)    { T_Super::_SetCharacterSpacingType (value);    if (this->HasTextStyle ()) m_overrides.m_characterSpacingType       = true; }
void RunProperties::_SetCharacterSpacingValue   (double value)                  { T_Super::_SetCharacterSpacingValue (value);   if (this->HasTextStyle ()) m_overrides.m_characterSpacingValue      = true; }
void RunProperties::_SetShouldUseBackground     (bool value)                    { T_Super::_SetShouldUseBackground (value);     if (this->HasTextStyle ()) m_overrides.m_shouldUseBackground        = true; }
void RunProperties::_SetRunOffset               (DPoint2dCR value)              { T_Super::_SetRunOffset (value);               if (this->HasTextStyle ()) m_overrides.m_runOffset                  = true; }
void RunProperties::_SetIsSubScript             (bool value)                    { T_Super::_SetIsSubScript (value);             if (this->HasTextStyle ()) m_overrides.m_isSubScript                = true; }
void RunProperties::_SetIsSuperScript           (bool value)                    { T_Super::_SetIsSuperScript (value);           if (this->HasTextStyle ()) m_overrides.m_isSuperScript              = true; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunProperties::_SetColor (UInt32 value)
    {
    T_Super::_SetColor (value);
    
    // Base class sets both color and has-color flag, so mark overrides on both.

    if (this->HasTextStyle ())
        {
        m_overrides.m_color     = true;
        m_overrides.m_hasColor  = true;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void RunProperties::_SetFont (DgnFontCR value)
    {
    T_Super::_SetFont (value);
    
    // Base class setter refuse?
    if (&value != m_font)
        return;
    
    if (this->HasTextStyle ())
        m_overrides.m_font = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void RunProperties::_SetShxBigFont (DgnFontCP value)
    {
    T_Super::_SetShxBigFont (value);
    
    // Base class setter refuse?
    if (value != m_shxBigFont)
        return;
    
    if (this->HasTextStyle ())
        m_overrides.m_shxBigFont = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunProperties::_ClearColor ()
    {
    T_Super::_ClearColor ();
    
    if (!this->HasTextStyle ())
        return;
    
    m_overrides.m_hasColor  = false;
    m_overrides.m_color     = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunProperties::_SetFontSize (DPoint2dCR value)
    {
    T_Super::_SetFontSize (value);
    
    if (!this->HasTextStyle ())
        return;
    
    m_overrides.m_width     = true;
    m_overrides.m_height    = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunProperties::_SetUnderlineStyle (UInt32 const * color, Int32 const * lineStyle, UInt32 const * weight)
    {
    T_Super::_SetUnderlineStyle (color, lineStyle, weight);
    
    if (!this->HasTextStyle ())
        return;
    
    if (NULL != color)      m_overrides.m_underlineColor                    = true;
    if (NULL != lineStyle)  m_overrides.m_underlineLineStyle                = true;
    if (NULL != weight)     m_overrides.m_underlineWeight                   = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunProperties::_SetOverlineStyle (UInt32 const * color, Int32 const * lineStyle, UInt32 const * weight)
    {
    T_Super::_SetOverlineStyle (color, lineStyle, weight);
    
    if (!this->HasTextStyle ())
        return;
    
    if (NULL != color)      m_overrides.m_overlineColor                     = true;
    if (NULL != lineStyle)  m_overrides.m_overlineLineStyle                 = true;
    if (NULL != weight)     m_overrides.m_overlineWeight                    = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunProperties::_SetBackgroundStyle (UInt32 const * fillColor, UInt32 const * borderColor, Int32 const * borderLineStyle, UInt32 const * borderWeight, DPoint2d const * borderPadding)
    {
    T_Super::_SetBackgroundStyle (fillColor, borderColor, borderLineStyle, borderWeight, borderPadding);
    
    if (!this->HasTextStyle ())
        return;
    
    if (NULL != fillColor)          m_overrides.m_backgroundFillColor       = true;
    if (NULL != borderColor)        m_overrides.m_backgroundBorderColor     = true;
    if (NULL != borderLineStyle)    m_overrides.m_backgroundBorderLineStyle = true;
    if (NULL != borderWeight)       m_overrides.m_backgroundBorderWeight    = true;
    if (NULL != borderPadding)      m_overrides.m_backgroundBorderPadding   = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
void RunProperties::_RemoveTextStyle ()
    {
    m_overrides.Clear ();
    m_textStyleId = 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2009
//---------------------------------------------------------------------------------------
template<class T>
static void setPropertyFromStyle (T& value, bool& overrideFlag, DgnTextStyleProperty tsProp, DgnTextStylePropertyMaskCR applyMask, DgnTextStylePropertyMaskCR overridesMask, DgnTextStyleCR newStyle)
    {
    if (!applyMask.IsPropertySet (tsProp))
        return;
    
    newStyle.GetPropertyValue (tsProp, value);
    
    overrideFlag = overridesMask.IsPropertySet(tsProp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2009
//---------------------------------------------------------------------------------------
static void setPropertyFromStyle (double& value, bool& overrideFlag, double scaleFactor, DgnTextStyleProperty tsProp, DgnTextStylePropertyMaskCR applyMask, DgnTextStylePropertyMaskCR overridesMask, DgnTextStyleCR newStyle)
    {
    if (!applyMask.IsPropertySet(tsProp))
        return;
    
    newStyle.GetPropertyValue (tsProp, value);
    value *= scaleFactor;
    
    overrideFlag = overridesMask.IsPropertySet(tsProp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2009
//---------------------------------------------------------------------------------------
static void setPropertyFromStyle (DPoint2dR value, bool& overrideFlag, double scaleFactor, DgnTextStyleProperty tsProp, DgnTextStylePropertyMaskCR applyMask, DgnTextStylePropertyMaskCR overridesMask, DgnTextStyleCR newStyle)
    {
    if (!applyMask.IsPropertySet(tsProp))
        return;
    
    newStyle.GetPropertyValue (tsProp, value);
    value.Scale (value, scaleFactor);
    
    overrideFlag = overridesMask.IsPropertySet(tsProp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
void RunProperties::SetPropertiesFromStyle (DgnTextStylePropertyMaskCR applyMask, DgnTextStyleCR newStyle, DgnTextStylePropertyMaskCR overridesMask)
    {
    double  heightFactor;   newStyle.GetPropertyValue (DgnTextStyleProperty::Height, heightFactor);
    double  normalFactor    = heightFactor;
    
    setPropertyFromStyle (m_font,                               m_overrides.m_font,                                         DgnTextStyleProperty::Font,                         applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_shxBigFont,                         m_overrides.m_shxBigFont,                                   DgnTextStyleProperty::ShxBigFont,                   applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_hasColor,                           m_overrides.m_hasColor,                                     DgnTextStyleProperty::HasColor,                     applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_color,                              m_overrides.m_color,                                        DgnTextStyleProperty::Color,                        applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_isBold,                             m_overrides.m_isBold,                                       DgnTextStyleProperty::IsBold,                       applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_isItalic,                           m_overrides.m_isItalic,                                     DgnTextStyleProperty::IsItalics,                    applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_customSlantAngle,                   m_overrides.m_customSlantAngle,                             DgnTextStyleProperty::CustomSlantAngle,             applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_isUnderlined,                       m_overrides.m_isUnderlined,                                 DgnTextStyleProperty::IsUnderlined,                 applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_shouldUseUnderlineStyle,            m_overrides.m_shouldUseUnderlineStyle,                      DgnTextStyleProperty::ShouldUseUnderlineStyle,      applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_underlineOffset,                    m_overrides.m_underlineOffset,              normalFactor,   DgnTextStyleProperty::UnderlineOffsetFactor,              applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_underlineColor,                     m_overrides.m_underlineColor,                               DgnTextStyleProperty::UnderlineColor,               applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_underlineLineStyle,                 m_overrides.m_underlineLineStyle,                           DgnTextStyleProperty::UnderlineLineStyle,           applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_underlineWeight,                    m_overrides.m_underlineWeight,                              DgnTextStyleProperty::UnderlineWeight,              applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_isOverlined,                        m_overrides.m_isOverlined,                                  DgnTextStyleProperty::IsOverlined,                  applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_shouldUseOverlineStyle,             m_overrides.m_shouldUseOverlineStyle,                       DgnTextStyleProperty::ShouldUseOverlineStyle,       applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_overlineOffset,                     m_overrides.m_overlineOffset,               normalFactor,   DgnTextStyleProperty::OverlineOffsetFactor,               applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_overlineColor,                      m_overrides.m_overlineColor,                                DgnTextStyleProperty::OverlineColor,                applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_overlineLineStyle,                  m_overrides.m_overlineLineStyle,                            DgnTextStyleProperty::OverlineLineStyle,            applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_overlineWeight,                     m_overrides.m_overlineWeight,                               DgnTextStyleProperty::OverlineWeight,               applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_shouldUseBackground,                m_overrides.m_shouldUseBackground,                          DgnTextStyleProperty::ShouldUseBackground,          applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_backgroundFillColor,                m_overrides.m_backgroundFillColor,                          DgnTextStyleProperty::BackgroundFillColor,          applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_backgroundBorderColor,              m_overrides.m_backgroundBorderColor,                        DgnTextStyleProperty::BackgroundBorderColor,        applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_backgroundBorderLineStyle,          m_overrides.m_backgroundBorderLineStyle,                    DgnTextStyleProperty::BackgroundBorderLineStyle,    applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_backgroundBorderWeight,             m_overrides.m_backgroundBorderWeight,                       DgnTextStyleProperty::BackgroundBorderWeight,       applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_backgroundBorderPadding,            m_overrides.m_backgroundBorderPadding,      normalFactor,   DgnTextStyleProperty::BackgroundBorderPaddingFactor,      applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_runOffset,                          m_overrides.m_runOffset,                    normalFactor,   DgnTextStyleProperty::RunOffsetFactor,                    applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_isSubScript,                        m_overrides.m_isSubScript,                                  DgnTextStyleProperty::IsSubScript,                  applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_isSuperScript,                      m_overrides.m_isSuperScript,                                DgnTextStyleProperty::IsSuperScript,                applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_fontSize.x,                         m_overrides.m_width,                                        DgnTextStyleProperty::Width,                        applyMask, overridesMask, newStyle);
    setPropertyFromStyle (m_fontSize.y,                         m_overrides.m_height,                                       DgnTextStyleProperty::Height,                       applyMask, overridesMask, newStyle);
    
    if (applyMask.IsPropertySet(DgnTextStyleProperty::CharacterSpacingType))
        {
        UInt32 csType;
        newStyle.GetPropertyValue(DgnTextStyleProperty::CharacterSpacingType, csType);
        m_characterSpacingType = (CharacterSpacingType)csType;

        m_overrides.m_characterSpacingType = overridesMask.IsPropertySet(DgnTextStyleProperty::CharacterSpacingType);
        }

    // Bad data like this can come from DWG, or admittedly corrupt DGN data. Make sure to use correct values moving forward.
    if (applyMask.IsPropertySet (DgnTextStyleProperty::Font) && (DGNFONTVARIANT_ShxBig == m_font->GetVariant ()))
        {
        // BeDataAssertOnce (false && L"Invalid font provided; substitution will occur.");
        
        m_font              = &DgnFontManager::GetDefaultShxFont ();
        m_overrides.m_font  = true;
        }
    
    if (applyMask.IsPropertySet(DgnTextStyleProperty::ShxBigFont) && (NULL != m_shxBigFont) && (DGNFONTVARIANT_ShxBig != m_shxBigFont->GetVariant()))
        {
        // BeDataAssertOnce (false && L"Invalid big font provided; substitution will occur.");
        
        m_shxBigFont                = DgnFontManager::GetDefaultShxBigFont ();
        m_overrides.m_shxBigFont    = true;
        }
    
    // Other property handling that can't be done trivially above.
    if (applyMask.IsPropertySet(DgnTextStyleProperty::CharacterSpacingValueFactor))
        {
        newStyle.GetPropertyValue (DgnTextStyleProperty::CharacterSpacingValueFactor, m_characterSpacingValue);
        m_overrides.m_characterSpacingValue = overridesMask.IsPropertySet(DgnTextStyleProperty::CharacterSpacingValueFactor);
        
        if (CharacterSpacingType::Factor != m_characterSpacingType)
            m_characterSpacingValue *= normalFactor;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunProperties::_ApplyTextStyle (DgnTextStyleCR newStyle, bool respectOverrides)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    DgnTextStylePtr             fileStyle       = DgnTextStyle::GetByName (newStyle.GetName ().c_str (), newStyle.GetFile ());
    DgnTextStylePropertyMaskPtr    overridesMask   = fileStyle.IsValid () ? newStyle.Compare (*fileStyle) : DgnTextStylePropertyMask::CreatePropMask ();
    
    if (fileStyle.IsNull () || !respectOverrides)
        m_overrides.Clear ();
    
    DgnTextStylePropertyMaskPtr applyMask = DgnTextStylePropertyMask::CreatePropMask ();
    if (!m_overrides.m_font)                        applyMask->SetPropertyFlag (DgnTextStyleProperty::Font, true);
    if (!m_overrides.m_shxBigFont)                  applyMask->SetPropertyFlag (DgnTextStyleProperty::ShxBigFont, true);
    if (!m_overrides.m_hasColor)                    applyMask->SetPropertyFlag (DgnTextStyleProperty::HasColor, true);
    if (!m_overrides.m_color)                       applyMask->SetPropertyFlag (DgnTextStyleProperty::Color, true);
    if (!m_overrides.m_isBold)                      applyMask->SetPropertyFlag (DgnTextStyleProperty::IsBold, true);
    if (!m_overrides.m_isItalic)                    applyMask->SetPropertyFlag (DgnTextStyleProperty::IsItalics, true);
    if (!m_overrides.m_customSlantAngle)            applyMask->SetPropertyFlag (DgnTextStyleProperty::CustomSlantAngle, true);
    if (!m_overrides.m_isUnderlined)                applyMask->SetPropertyFlag (DgnTextStyleProperty::IsUnderlined, true);
    if (!m_overrides.m_shouldUseUnderlineStyle)     applyMask->SetPropertyFlag (DgnTextStyleProperty::ShouldUseUnderlineStyle, true);
    if (!m_overrides.m_underlineOffset)             applyMask->SetPropertyFlag (DgnTextStyleProperty::UnderlineOffsetFactor, true);
    if (!m_overrides.m_underlineColor)              applyMask->SetPropertyFlag (DgnTextStyleProperty::UnderlineColor, true);
    if (!m_overrides.m_underlineLineStyle)          applyMask->SetPropertyFlag (DgnTextStyleProperty::UnderlineLineStyle, true);
    if (!m_overrides.m_underlineWeight)             applyMask->SetPropertyFlag (DgnTextStyleProperty::UnderlineWeight, true);
    if (!m_overrides.m_isOverlined)                 applyMask->SetPropertyFlag (DgnTextStyleProperty::IsOverlined, true);
    if (!m_overrides.m_shouldUseOverlineStyle)      applyMask->SetPropertyFlag (DgnTextStyleProperty::ShouldUseOverlineStyle, true);
    if (!m_overrides.m_overlineOffset)              applyMask->SetPropertyFlag (DgnTextStyleProperty::OverlineOffsetFactor, true);
    if (!m_overrides.m_overlineColor)               applyMask->SetPropertyFlag (DgnTextStyleProperty::OverlineColor, true);
    if (!m_overrides.m_overlineLineStyle)           applyMask->SetPropertyFlag (DgnTextStyleProperty::OverlineLineStyle, true);
    if (!m_overrides.m_overlineWeight)              applyMask->SetPropertyFlag (DgnTextStyleProperty::OverlineWeight, true);
    if (!m_overrides.m_shouldUseBackground)         applyMask->SetPropertyFlag (DgnTextStyleProperty::ShouldUseBackground, true);
    if (!m_overrides.m_backgroundFillColor)         applyMask->SetPropertyFlag (DgnTextStyleProperty::BackgroundFillColor, true);
    if (!m_overrides.m_backgroundBorderColor)       applyMask->SetPropertyFlag (DgnTextStyleProperty::BackgroundBorderColor, true);
    if (!m_overrides.m_backgroundBorderLineStyle)   applyMask->SetPropertyFlag (DgnTextStyleProperty::BackgroundBorderLineStyle, true);
    if (!m_overrides.m_backgroundBorderWeight)      applyMask->SetPropertyFlag (DgnTextStyleProperty::BackgroundBorderWeight, true);
    if (!m_overrides.m_backgroundBorderPadding)     applyMask->SetPropertyFlag (DgnTextStyleProperty::BackgroundBorderPaddingFactor, true);
    if (!m_overrides.m_runOffset)                   applyMask->SetPropertyFlag (DgnTextStyleProperty::RunOffsetFactor, true);
    if (!m_overrides.m_isSubScript)                 applyMask->SetPropertyFlag (DgnTextStyleProperty::IsSubScript, true);
    if (!m_overrides.m_isSuperScript)               applyMask->SetPropertyFlag (DgnTextStyleProperty::IsSuperScript, true);
    if (!m_overrides.m_width)                       applyMask->SetPropertyFlag (DgnTextStyleProperty::Width, true);
    if (!m_overrides.m_height)                      applyMask->SetPropertyFlag (DgnTextStyleProperty::Height, true);
    if (!m_overrides.m_characterSpacingValue)       applyMask->SetPropertyFlag (DgnTextStyleProperty::CharacterSpacingValueFactor, true);
    
    if (!m_overrides.m_characterSpacingType)
        {
        if (CharacterSpacingType::Factor == m_characterSpacingType)
            applyMask->SetPropertyFlag (DgnTextStyleProperty::CharacterSpacingType, true);
        
        if (CharacterSpacingType::FixedWidth == m_characterSpacingType)
            applyMask->SetPropertyFlag (DgnTextStyleProperty::FixedSpacing, true);
        }
    
    this->SetPropertiesFromStyle (*applyMask, newStyle, *overridesMask);
    
    m_textStyleId = fileStyle.IsValid () ? (UInt32)fileStyle->GetID () : 0;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
void RunProperties::_SetProperties (DgnTextStyleCR newStyle, DgnTextStylePropertyMaskCR applyMask)
    {
    DgnTextStylePropertyMaskPtr overridesMask = DgnTextStylePropertyMask::Create();
    if (this->HasTextStyle ())
        overridesMask->SetAllProperties (true);
    
    this->SetPropertiesFromStyle (applyMask, newStyle, *overridesMask);
    }
