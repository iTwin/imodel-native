/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RunPropertiesBase.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace std;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- TextParamWide ----------------------------------------------------------------------------------------------------------------- TextParamWide --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
TextParamWide::TextParamWide ()
    {
    Initialize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2009
//---------------------------------------------------------------------------------------
void TextParamWide::Initialize ()
    {
    memset (this, 0, sizeof (*this));
    this->annotationScale = 1.0;
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- RunPropertiesBase --------------------------------------------------------------------------------------------------------- RunPropertiesBase --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

const double RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SCALE = 0.3;
const double RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SHIFT = 0.25;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
RunPropertiesBase::RunPropertiesBase () :
    RefCountedBase ()
    {
    DPoint2d zeroSize = { 0.0, 0.0 };

    this->InitDefaults (DgnFontManager::GetDefaultTrueTypeFont (), NULL, zeroSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
RunPropertiesBase::RunPropertiesBase (DgnFontCR font, DgnFontCP shxBigFont, DPoint2dCR fontSize) :
    RefCountedBase ()
    {
    this->InitDefaults (font, shxBigFont, fontSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
RunPropertiesBase::RunPropertiesBase (TextParamWideCR params, DPoint2dCR fontSize, DgnProjectCR project) :
    RefCountedBase ()
    {
    this->FromElementDataInternal (params, fontSize, project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
RunPropertiesBase::RunPropertiesBase (RunPropertiesBaseCR rhs) :
    RefCountedBase (),
    m_font                      (rhs.m_font),
    m_shxBigFont                (rhs.m_shxBigFont),
    m_hasColor                  (rhs.m_hasColor),
    m_color                     (rhs.m_color),
    m_isBold                    (rhs.m_isBold),
    m_isItalic                  (rhs.m_isItalic),
    m_customSlantAngle          (rhs.m_customSlantAngle),
    m_isUnderlined              (rhs.m_isUnderlined),
    m_shouldUseUnderlineStyle   (rhs.m_shouldUseUnderlineStyle),
    m_underlineOffset           (rhs.m_underlineOffset),
    m_underlineColor            (rhs.m_underlineColor),
    m_underlineLineStyle        (rhs.m_underlineLineStyle),
    m_underlineWeight           (rhs.m_underlineWeight),
    m_isOverlined               (rhs.m_isOverlined),
    m_shouldUseOverlineStyle    (rhs.m_shouldUseOverlineStyle),
    m_overlineOffset            (rhs.m_overlineOffset),
    m_overlineColor             (rhs.m_overlineColor),
    m_overlineLineStyle         (rhs.m_overlineLineStyle),
    m_overlineWeight            (rhs.m_overlineWeight),
    m_characterSpacingType      (rhs.m_characterSpacingType),
    m_characterSpacingValue     (rhs.m_characterSpacingValue),
    m_shouldUseBackground       (rhs.m_shouldUseBackground),
    m_backgroundFillColor       (rhs.m_backgroundFillColor),
    m_backgroundBorderColor     (rhs.m_backgroundBorderColor),
    m_backgroundBorderLineStyle (rhs.m_backgroundBorderLineStyle),
    m_backgroundBorderWeight    (rhs.m_backgroundBorderWeight),
    m_backgroundBorderPadding   (rhs.m_backgroundBorderPadding),
    m_runOffset                 (rhs.m_runOffset),
    m_isSubScript               (rhs.m_isSubScript),
    m_isSuperScript             (rhs.m_isSuperScript),
    m_shouldIgnoreLSB           (rhs.m_shouldIgnoreLSB),
    m_fontSize                  (rhs.m_fontSize)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::InitDefaults (DgnFontCR font, DgnFontCP shxBigFont, DPoint2dCR fontSize)
    {
    m_font                      = &font;
    m_shxBigFont                = shxBigFont;
    m_hasColor                  = false;
    m_color                     = 0;
    m_isBold                    = false;
    m_isItalic                  = false;
    m_customSlantAngle          = 0.0;
    m_isUnderlined              = false;
    m_shouldUseUnderlineStyle   = false;
    m_underlineOffset           = 0.0;
    m_underlineColor            = 0;
    m_underlineLineStyle        = 0;
    m_underlineWeight           = 0;
    m_isOverlined               = false;
    m_shouldUseOverlineStyle    = false;
    m_overlineOffset            = 0.0;
    m_overlineColor             = 0;
    m_overlineLineStyle         = 0;
    m_overlineWeight            = 0;
    m_characterSpacingType      = CharacterSpacingType::Absolute;
    m_characterSpacingValue     = 0.0;
    m_shouldUseBackground       = false;
    m_backgroundFillColor       = 0;
    m_backgroundBorderColor     = 0;
    m_backgroundBorderLineStyle = 0;
    m_backgroundBorderWeight    = 0;
    m_isSubScript               = false;
    m_isSuperScript             = false;
    m_shouldIgnoreLSB           = false;
    m_fontSize                  = fontSize;
    
    m_backgroundBorderPadding.setComponents (0.0, 0.0);
    m_runOffset.setComponents (0.0, 0.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
DgnFontCR               RunPropertiesBase::GetFont () const                                         { return *m_font; }
void                    RunPropertiesBase::SetFont (DgnFontCR value)                                { _SetFont (value); }
DgnFontCR               RunPropertiesBase::ResolveFont () const                                     { return *m_font->ResolveToRenderFont(); }
DgnFontCP               RunPropertiesBase::GetShxBigFontCP () const                                 { return m_shxBigFont; }
void                    RunPropertiesBase::SetShxBigFont (DgnFontCP value)                          { _SetShxBigFont (value); }
DgnFontCP               RunPropertiesBase::ResolveShxBigFontCP () const                             { return m_shxBigFont ? m_shxBigFont->ResolveToRenderFont() : NULL; }
DgnFontCR               RunPropertiesBase::GetFontForCodePage () const                              { return (NULL != m_shxBigFont) ? *m_shxBigFont : *m_font; }
bool                    RunPropertiesBase::HasColor () const                                        { return m_hasColor; }
void                    RunPropertiesBase::ClearColor ()                                            { _ClearColor (); }
void                    RunPropertiesBase::_ClearColor ()                                           { m_hasColor = false; m_color = 0; }
UInt32                  RunPropertiesBase::GetColor () const                                        { return m_color; }
void                    RunPropertiesBase::SetColor (UInt32 value)                                  { _SetColor (value); }
void                    RunPropertiesBase::_SetColor (UInt32 value)                                 { m_color = value; m_hasColor = true; }
bool                    RunPropertiesBase::IsBold () const                                          { return m_isBold; }
void                    RunPropertiesBase::SetIsBold (bool value)                                   { _SetIsBold (value); }
void                    RunPropertiesBase::_SetIsBold (bool value)                                  { m_isBold = value; }
bool                    RunPropertiesBase::IsItalic () const                                        { return m_isItalic; }
void                    RunPropertiesBase::SetIsItalic (bool value)                                 { _SetIsItalic (value); }
void                    RunPropertiesBase::_SetIsItalic (bool value)                                { m_isItalic = value; }
double                  RunPropertiesBase::GetCustomSlantAngle () const                             { return m_customSlantAngle; }
void                    RunPropertiesBase::SetCustomSlantAngle (double value)                       { _SetCustomSlantAngle (value); }
void                    RunPropertiesBase::_SetCustomSlantAngle (double value)                      { m_customSlantAngle = value; }
bool                    RunPropertiesBase::IsUnderlined () const                                    { return m_isUnderlined; }
void                    RunPropertiesBase::SetIsUnderlined (bool value)                             { _SetIsUnderlined (value); }
void                    RunPropertiesBase::_SetIsUnderlined (bool value)                            { m_isUnderlined = value; }
bool                    RunPropertiesBase::ShouldUseUnderlineStyle () const                         { return m_shouldUseUnderlineStyle; }
void                    RunPropertiesBase::SetShouldUseUnderlineStyle (bool value)                  { _SetShouldUseUnderlineStyle (value); }
void                    RunPropertiesBase::_SetShouldUseUnderlineStyle (bool value)                 { m_shouldUseUnderlineStyle = value; }
double                  RunPropertiesBase::GetUnderlineOffset () const                              { return m_underlineOffset; }
void                    RunPropertiesBase::SetUnderlineOffset (double value)                        { _SetUnderlineOffset (value); }
void                    RunPropertiesBase::_SetUnderlineOffset (double value)                       { m_underlineOffset = value; }
bool                    RunPropertiesBase::IsOverlined () const                                     { return m_isOverlined; }
void                    RunPropertiesBase::SetIsOverlined (bool value)                              { _SetIsOverlined (value); }
void                    RunPropertiesBase::_SetIsOverlined (bool value)                             { m_isOverlined = value; }
bool                    RunPropertiesBase::ShouldUseOverlineStyle () const                          { return m_shouldUseOverlineStyle; }
void                    RunPropertiesBase::SetShouldUseOverlineStyle (bool value)                   { _SetShouldUseOverlineStyle (value); }
void                    RunPropertiesBase::_SetShouldUseOverlineStyle (bool value)                  { m_shouldUseOverlineStyle = value; }
double                  RunPropertiesBase::GetOverlineOffset () const                               { return m_overlineOffset; }
void                    RunPropertiesBase::SetOverlineOffset (double value)                         { _SetOverlineOffset (value); }
void                    RunPropertiesBase::_SetOverlineOffset (double value)                        { m_overlineOffset = value; }
CharacterSpacingType    RunPropertiesBase::GetCharacterSpacingType () const                         { return m_characterSpacingType; }
void                    RunPropertiesBase::SetCharacterSpacingType (CharacterSpacingType value)     { _SetCharacterSpacingType (value); }
void                    RunPropertiesBase::_SetCharacterSpacingType (CharacterSpacingType value)    { m_characterSpacingType = value; }
double                  RunPropertiesBase::GetCharacterSpacingValue () const                        { return m_characterSpacingValue; }
void                    RunPropertiesBase::SetCharacterSpacingValue (double value)                  { _SetCharacterSpacingValue (value); }
void                    RunPropertiesBase::_SetCharacterSpacingValue (double value)                 { m_characterSpacingValue = value; }
bool                    RunPropertiesBase::ShouldUseBackground () const                             { return m_shouldUseBackground; }
void                    RunPropertiesBase::SetShouldUseBackground (bool value)                      { _SetShouldUseBackground (value); }
void                    RunPropertiesBase::_SetShouldUseBackground (bool value)                     { m_shouldUseBackground = value; }
DPoint2dCR              RunPropertiesBase::GetRunOffset () const                                    { return m_runOffset; }
void                    RunPropertiesBase::SetRunOffset (DPoint2dCR value)                          { _SetRunOffset (value); }
void                    RunPropertiesBase::_SetRunOffset (DPoint2dCR value)                         { m_runOffset = value; }
bool                    RunPropertiesBase::IsSubScript () const                                     { return m_isSubScript; }
void                    RunPropertiesBase::SetIsSubScript (bool value)                              { _SetIsSubScript (value); }
void                    RunPropertiesBase::_SetIsSubScript (bool value)                             { m_isSubScript = value; }
bool                    RunPropertiesBase::IsSuperScript () const                                   { return m_isSuperScript; }
void                    RunPropertiesBase::SetIsSuperScript (bool value)                            { _SetIsSuperScript (value); }
void                    RunPropertiesBase::_SetIsSuperScript (bool value)                           { m_isSuperScript = value; }
bool                    RunPropertiesBase::ShouldIgnoreLSB () const                                 { return m_shouldIgnoreLSB; }
void                    RunPropertiesBase::SetShouldIgnoreLSB (bool value)                          { _SetShouldIgnoreLSB (value); }
void                    RunPropertiesBase::_SetShouldIgnoreLSB (bool value)                         { m_shouldIgnoreLSB = value; }
DPoint2dCR              RunPropertiesBase::GetFontSize () const                                     { return m_fontSize; }
void                    RunPropertiesBase::SetFontSize (DPoint2dCR value)                           { _SetFontSize (value); }
void                    RunPropertiesBase::_SetFontSize (DPoint2dCR value)                          { m_fontSize = value; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::_SetFont (DgnFontCR value)
    {
    if (DGNFONTVARIANT_ShxBig == value.GetVariant ())
        { BeAssert (false && L"Big fonts cannot be set as the normal font."); return; }
    
    m_font = &value;

    if ((NULL != m_shxBigFont) && (DgnFontType::Shx != m_font->GetType ()))
        this->SetShxBigFont (NULL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::_SetShxBigFont (DgnFontCP value)
    {
    if ((NULL != value) && (DGNFONTVARIANT_ShxBig != value->GetVariant ()))
        { BeAssert (false && L"Value must be an SHX big font."); return; }

    m_shxBigFont = value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::GetUnderlineStyle (UInt32* color, Int32* lineStyle, UInt32* weight) const
    {
    if (NULL != color)              *color                      = m_underlineColor;
    if (NULL != lineStyle)          *lineStyle                  = m_underlineLineStyle;
    if (NULL != weight)             *weight                     = m_underlineWeight;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::SetUnderlineStyle (UInt32 const * color, Int32 const * lineStyle, UInt32 const * weight) { _SetUnderlineStyle (color, lineStyle, weight); }
void RunPropertiesBase::_SetUnderlineStyle (UInt32 const * color, Int32 const * lineStyle, UInt32 const * weight)
    {
    if (NULL != color)              m_underlineColor            = *color;
    if (NULL != lineStyle)          m_underlineLineStyle        = *lineStyle;
    if (NULL != weight)             m_underlineWeight           = *weight;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::GetOverlineStyle (UInt32* color, Int32* lineStyle, UInt32* weight) const
    {
    if (NULL != color)              *color                      = m_overlineColor;
    if (NULL != lineStyle)          *lineStyle                  = m_overlineLineStyle;
    if (NULL != weight)             *weight                     = m_overlineWeight;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::SetOverlineStyle (UInt32 const * color, Int32 const * lineStyle, UInt32 const * weight) { _SetOverlineStyle (color, lineStyle, weight); }
void RunPropertiesBase::_SetOverlineStyle (UInt32 const * color, Int32 const * lineStyle, UInt32 const * weight)
    {
    if (NULL != color)              m_overlineColor             = *color;
    if (NULL != lineStyle)          m_overlineLineStyle         = *lineStyle;
    if (NULL != weight)             m_overlineWeight            = *weight;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::GetBackgroundStyle (UInt32* fillColor, UInt32* borderColor, Int32* borderLineStyle, UInt32* borderWeight, DPoint2d* borderPadding) const
    {
    if (NULL != fillColor)          *fillColor                  = m_backgroundFillColor;
    if (NULL != borderColor)        *borderColor                = m_backgroundBorderColor;
    if (NULL != borderLineStyle)    *borderLineStyle            = m_backgroundBorderLineStyle;
    if (NULL != borderWeight)       *borderWeight               = m_backgroundBorderWeight;
    if (NULL != borderPadding)      *borderPadding              = m_backgroundBorderPadding;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::SetBackgroundStyle (UInt32 const * fillColor, UInt32 const * borderColor, Int32 const * borderLineStyle, UInt32 const * borderWeight, DPoint2d const * borderPadding) { _SetBackgroundStyle (fillColor, borderColor, borderLineStyle, borderWeight, borderPadding); }
void RunPropertiesBase::_SetBackgroundStyle (UInt32 const * fillColor, UInt32 const * borderColor, Int32 const * borderLineStyle, UInt32 const * borderWeight, DPoint2d const * borderPadding)
    {
    if (NULL != fillColor)          m_backgroundFillColor       = *fillColor;
    if (NULL != borderColor)        m_backgroundBorderColor     = *borderColor;
    if (NULL != borderLineStyle)    m_backgroundBorderLineStyle = *borderLineStyle;
    if (NULL != borderWeight)       m_backgroundBorderWeight    = *borderWeight;
    if (NULL != borderPadding)      m_backgroundBorderPadding   = *borderPadding;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::FromElementData (TextParamWideCR params, DPoint2dCR fontSize, DgnProjectCR project)
    {
    this->_FromElementData (params, fontSize, project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::_FromElementData (TextParamWideCR params, DPoint2dCR fontSize, DgnProjectCR project)
    {
    this->FromElementDataInternal (params, fontSize, project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::FromElementDataInternal (TextParamWideCR params, DPoint2dCR fontSize, DgnProjectCR project)
    {
    // Try DgnProject::FindFont first to allow missing fonts.
    m_font = project.Fonts().FindFont(params.font);
    if (NULL == m_font)
        m_font = DgnFontManager::ResolveFont (params.font, project, DGNFONTVARIANT_DontCare);

    if (0 == params.shxBigFont)
        {
        m_shxBigFont = NULL;
        }
    else
        {
        m_shxBigFont = project.Fonts().FindFont (params.shxBigFont);
        if (NULL == m_shxBigFont)
            m_shxBigFont = DgnFontManager::ResolveFont (params.shxBigFont, project, DGNFONTVARIANT_ShxBig);
        }
    
    // Bad data like this can come from DWG, or admittedly corrupt DGN data. Make sure to use correct values moving forward.
    if (DGNFONTVARIANT_ShxBig == m_font->GetVariant ())
        {
        BeDataAssert (false); // Invalid font provided; substitution will occur.
        
        m_font = &DgnFontManager::GetDefaultShxFont ();
        }
    
    if ((NULL != m_shxBigFont) && (DGNFONTVARIANT_ShxBig != m_shxBigFont->GetVariant ()))
        {
        BeDataAssert (false); // Invalid font provided; substitution will occur.
        m_shxBigFont = DgnFontManager::GetDefaultShxBigFont ();
        }
    
    m_hasColor                  = params.exFlags.color;
    m_color                     = params.color;
    m_isBold                    = params.exFlags.bold;
    m_isItalic                  = params.flags.slant;
    m_customSlantAngle          = params.slant;
    
    m_isUnderlined              = params.flags.underline;
    m_shouldUseUnderlineStyle   = params.exFlags.underlineStyle;
    m_underlineColor            = params.underlineColor;
    m_underlineLineStyle        = params.underlineStyle;
    m_underlineWeight           = params.underlineWeight;
    m_underlineOffset           = params.underlineSpacing;

    m_isOverlined               = params.exFlags.overline;
    m_shouldUseOverlineStyle    = params.exFlags.overlineStyle;
    m_overlineColor             = params.overlineColor;
    m_overlineLineStyle         = params.overlineStyle;
    m_overlineWeight            = params.overlineWeight;
    m_overlineOffset            = params.overlineSpacing;

    m_characterSpacingValue     = params.characterSpacing;

    if (params.flags.fixedWidthSpacing)
        {
        m_characterSpacingType = CharacterSpacingType::FixedWidth;
        }
    else if (params.exFlags.acadInterCharSpacing)
        {
        m_characterSpacingType = CharacterSpacingType::Factor;
        }
    else
        {
        if (!params.flags.interCharSpacing)
            m_characterSpacingValue = 0.0;
        
        m_characterSpacingType = CharacterSpacingType::Absolute;
        }

    m_shouldUseBackground       = params.flags.bgColor;
    m_backgroundFillColor       = params.backgroundFillColor;
    m_backgroundBorderColor     = params.backgroundColor;
    m_backgroundBorderLineStyle = params.backgroundStyle;
    m_backgroundBorderWeight    = params.backgroundWeight;
    m_backgroundBorderPadding   = params.backgroundBorder;
    
    if (!params.flags.offset)
        m_runOffset.zero ();
    else
        m_runOffset = params.lineOffset;
    
    m_isSubScript               = params.flags.subscript;
    m_isSuperScript             = params.flags.superscript;
    m_shouldIgnoreLSB           = params.renderingFlags.alignEdge;
    
    m_fontSize                  = fontSize;
    
    // Storage duplicates backwards/upside down information by negating font size; this is not desirable at run time.
    m_fontSize.x = fabs (m_fontSize.x);
    m_fontSize.y = fabs (m_fontSize.y);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::ToElementData (TextParamWideR params, DgnProjectR project) const
    {
    this->_ToElementData (params, project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::_ToElementData (TextParamWideR params, DgnProjectR project) const
    {
    this->ToElementDataInternal (params, project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::ToElementDataInternal (TextParamWideR params, DgnProjectR project) const
    {
    project.Fonts().AcquireFontNumber (params.font, *m_font);
    
    if (NULL != m_shxBigFont)
        {
        project.Fonts().AcquireFontNumber (params.shxBigFont, *m_shxBigFont);
        params.flags.shxBigFont = true;
        }
    else
        {
        params.flags.shxBigFont = false;
        }
    
    params.flags.codePage_deprecated    = true;
    params.codePage_deprecated          = (int)(UInt32)this->GetFontForCodePage ().GetCodePage ();
    params.exFlags.color                = m_hasColor;
    params.color                        = m_color;
    params.exFlags.bold                 = m_isBold;
    params.flags.slant                  = m_isItalic;
    params.slant                        = m_customSlantAngle;
 
    params.flags.underline              = m_isUnderlined;
    params.exFlags.underlineStyle       = m_shouldUseUnderlineStyle;
    params.underlineColor               = m_underlineColor;
    params.underlineStyle               = m_underlineLineStyle;
    params.underlineWeight              = m_underlineWeight;
    params.underlineSpacing             = m_underlineOffset;

    params.exFlags.overline             = m_isOverlined;
    params.exFlags.overlineStyle        = m_shouldUseOverlineStyle;
    params.overlineColor                = m_overlineColor;
    params.overlineStyle                = m_overlineLineStyle;
    params.overlineWeight               = m_overlineWeight;
    params.overlineSpacing              = m_overlineOffset;

    switch (m_characterSpacingType)
        {
        case (CharacterSpacingType::FixedWidth):
            params.flags.interCharSpacing       = 1;
            params.flags.fixedWidthSpacing      = 1;
            params.exFlags.acadInterCharSpacing = 0;
            break;
        
        case (CharacterSpacingType::Factor):
            params.flags.interCharSpacing       = 1;
            params.flags.fixedWidthSpacing      = 0;
            params.exFlags.acadInterCharSpacing = 1;
            break;
        
        case (CharacterSpacingType::Absolute):
            params.flags.interCharSpacing       = (0.0 != m_characterSpacingValue);
            params.flags.fixedWidthSpacing      = 0;
            params.exFlags.acadInterCharSpacing = 0;
            break;
        }
    
    params.characterSpacing             = m_characterSpacingValue;
    
    params.flags.bgColor                = m_shouldUseBackground;
    params.backgroundFillColor          = m_backgroundFillColor;
    params.backgroundColor              = m_backgroundBorderColor;
    params.backgroundStyle              = m_backgroundBorderLineStyle;
    params.backgroundWeight             = m_backgroundBorderWeight;
    params.backgroundBorder             = m_backgroundBorderPadding;
    
    params.flags.offset                 = (0.0 != m_runOffset.x || 0.0 != m_runOffset.y);
    params.lineOffset                   = m_runOffset;
    
    params.flags.subscript              = m_isSubScript;
    params.flags.superscript            = m_isSuperScript;
    params.renderingFlags.alignEdge     = m_shouldIgnoreLSB;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
bool RunPropertiesBase::Equals (RunPropertiesBaseCR rhs) const  { return this->Equals (rhs, 0.0, true); }
bool RunPropertiesBase::Equals (RunPropertiesBaseCR rhs, double tolerance, bool shouldIgnoreElementOverhead) const
    {
    if (!m_font->Equals (*rhs.m_font))                                              return false;
    if (m_color                     != rhs.m_color)                                 return false;
    if (m_isBold                    != rhs.m_isBold)                                return false;
    if (m_isItalic                  != rhs.m_isItalic)                              return false;
    if (fabs (m_customSlantAngle - rhs.m_customSlantAngle) > tolerance)             return false;
    if (m_isUnderlined              != rhs.m_isUnderlined)                          return false;
    if (m_shouldUseUnderlineStyle   != rhs.m_shouldUseUnderlineStyle)               return false;
    if (fabs (m_underlineOffset - rhs.m_underlineOffset) > tolerance)               return false;
    if (m_underlineColor            != rhs.m_underlineColor)                        return false;
    if (m_underlineLineStyle        != rhs.m_underlineLineStyle)                    return false;
    if (m_underlineWeight           != rhs.m_underlineWeight)                       return false;
    if (m_isOverlined               != rhs.m_isOverlined)                           return false;
    if (m_shouldUseOverlineStyle    != rhs.m_shouldUseOverlineStyle)                return false;
    if (fabs (m_overlineOffset - rhs.m_overlineOffset) > tolerance)                 return false;
    if (m_overlineColor             != rhs.m_overlineColor)                         return false;
    if (m_overlineLineStyle         != rhs.m_overlineLineStyle)                     return false;
    if (m_overlineWeight            != rhs.m_overlineWeight)                        return false;
    if (m_characterSpacingType      != rhs.m_characterSpacingType)                  return false;
    if (fabs (m_characterSpacingValue - rhs.m_characterSpacingValue) > tolerance)   return false;
    if (m_shouldUseBackground       != rhs.m_shouldUseBackground)                   return false;
    if (m_backgroundFillColor       != rhs.m_backgroundFillColor)                   return false;
    if (m_backgroundBorderColor     != rhs.m_backgroundBorderColor)                 return false;
    if (m_backgroundBorderLineStyle != rhs.m_backgroundBorderLineStyle)             return false;
    if (m_backgroundBorderWeight    != rhs.m_backgroundBorderWeight)                return false;
    if (m_isSubScript               != rhs.m_isSubScript)                           return false;
    if (m_isSuperScript             != rhs.m_isSuperScript)                         return false;
    if (m_shouldIgnoreLSB           != rhs.m_shouldIgnoreLSB)                       return false;
    
    if (((NULL != m_shxBigFont) && (NULL == rhs.m_shxBigFont)) || ((NULL == m_shxBigFont) && (NULL != rhs.m_shxBigFont)) || ((NULL != m_shxBigFont) && !m_shxBigFont->Equals (*rhs.m_shxBigFont)))
        return false;

    if (!shouldIgnoreElementOverhead)
        {
        if (m_hasColor != rhs.m_hasColor)
            return false;
        }
    
    if (!m_backgroundBorderPadding.IsEqual (rhs.m_backgroundBorderPadding, tolerance))
        return false;
    
    if (!m_runOffset.IsEqual (rhs.m_runOffset, tolerance))
        return false;
    
    if (!m_fontSize.IsEqual (rhs.m_fontSize, tolerance))
        return false;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
DPoint2d RunPropertiesBase::GetDisplaySize () const
    {
    DPoint2d size = m_fontSize;
    
    if (m_isSuperScript)
        size.scale (&size, RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SCALE);
    else if (m_isSubScript)
        size.scale (&size, RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SCALE);
    
    return size;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
DPoint3d RunPropertiesBase::GetDisplayOffset () const
    {
    DPoint3d offset;
    offset.Init (m_runOffset.x, m_runOffset.y, 0.0);

    if (m_isSuperScript)
        offset.y += (m_fontSize.y * (1.0 - RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SHIFT));
    else if (m_isSubScript)
        offset.y -= (m_fontSize.y * RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SHIFT);
    
    return offset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
void RunPropertiesBase::ApplyScale (DPoint2dCR scaleFactor, bool isVertical)
    {
    DPoint2d scale = scaleFactor;

    if (isVertical)
        {
        double t = scale.x;
        scale.x = scale.y;
        scale.y = t;
        }

    m_fontSize.x *= scale.x;
    m_fontSize.y *= scale.y;

    m_characterSpacingValue *= scale.x;

    m_backgroundBorderPadding.x *= scale.x;
    m_backgroundBorderPadding.y *= scale.y;

    m_runOffset.x *= scale.x;
    m_runOffset.y *= scale.y;

    m_underlineOffset *= scale.y;
    
    m_overlineOffset *= scale.y;

    if (scale.x * scale.y < 0)
        m_customSlantAngle *= -1.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
bool RunPropertiesBase::ShouldUseItalicTypeface () const
    {
    if (NULL == m_font)
        {
        BeAssert (false);
        return false;
        }
    
    if (DgnFontType::TrueType != m_font->GetType ())
        return false;
    
    if (!IsItalic ())
        return false;
    
    return (0.0 == m_customSlantAngle);
    }
