/*--------------------------------------------------------------------------------------+
|     $Source: DgnCore/DgnTextStyle.cpp $
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <algorithm>

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
void LegacyTextStyle::Scale(double scale)
    {
    LegacyTextStyleOverrideFlags allPropertiesMask;
    memset (&allPropertiesMask, 0xff, sizeof(allPropertiesMask));
    
    Scale(scale, allPropertiesMask);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
void LegacyTextStyle::Scale(double scale, LegacyTextStyleOverrideFlagsCR mask)
    {
    if ((scale <= 0.0) || (1.0 == scale))
        return;

    if (mask.width)                                                                         width *= scale;
    if (mask.height)                                                                        height *= scale;
    if (mask.underlineOffset)                                                               underlineOffset *= scale;
    if (mask.overlineOffset)                                                                overlineOffset *= scale;
    if (mask.lineOffset)                                                                    lineOffset.Scale(scale);
    if (mask.backgroundborder)                                                              backgroundBorder.Scale(scale);
    if (mask.linespacing && (DgnLineSpacingType::AtLeast != (DgnLineSpacingType)flags.acadLineSpacingType))   lineSpacing *= scale; // At Least line spacing is a multiplier, not a distance.
    if (mask.interCharSpacing && !flags.acadInterCharSpacing)                               interCharSpacing *= scale;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
DgnTextStylePropertyMaskPtr DgnTextStylePropertyMask::Create() { return new DgnTextStylePropertyMask(); }
DgnTextStylePropertyMask::DgnTextStylePropertyMask()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
DgnTextStylePropertyMaskPtr DgnTextStylePropertyMask::Clone() const
    {
    DgnTextStylePropertyMaskP copy = new DgnTextStylePropertyMask();
    copy->m_propertyFlags = m_propertyFlags;

    return copy;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
bool DgnTextStylePropertyMask::IsPropertySet(DgnTextStyleProperty property) const
    {
    return m_propertyFlags.test((size_t)property - 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
bool DgnTextStylePropertyMask::AreAnyPropertiesSet() const
    {
    return m_propertyFlags.any();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
void DgnTextStylePropertyMask::SetProperty(DgnTextStyleProperty property, bool value)
    {
    m_propertyFlags.set((size_t)property - 1, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
void DgnTextStylePropertyMask::SetAllProperties(bool value)
    {
    if (value)
        m_propertyFlags.set();
    else
        m_propertyFlags.reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
void DgnTextStylePropertyMask::LogicalOr(DgnTextStylePropertyMaskCR rhs)
    {
    m_propertyFlags |= rhs.m_propertyFlags;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyle::GetPropertyValue(DgnTextStyleProperty property, bool& value) const { return _GetPropertyValue(property, value); }
BentleyStatus DgnTextStyle::SetPropertyValue(DgnTextStyleProperty property, bool value) { return _SetPropertyValue(property, value); }
BentleyStatus DgnTextStyle::GetPropertyValue(DgnTextStyleProperty property, UInt32& value) const { return _GetPropertyValue(property, value); }
BentleyStatus DgnTextStyle::SetPropertyValue(DgnTextStyleProperty property, UInt32 value) { return _SetPropertyValue(property, value); }
BentleyStatus DgnTextStyle::GetPropertyValue(DgnTextStyleProperty property, Int32& value) const { return _GetPropertyValue(property, value); }
BentleyStatus DgnTextStyle::SetPropertyValue(DgnTextStyleProperty property, Int32 value) { return _SetPropertyValue(property, value); }
BentleyStatus DgnTextStyle::GetPropertyValue(DgnTextStyleProperty property, DPoint2dR value) const { return _GetPropertyValue(property, value); }
BentleyStatus DgnTextStyle::SetPropertyValue(DgnTextStyleProperty property, DPoint2dCR value) { return _SetPropertyValue(property, value); }
BentleyStatus DgnTextStyle::GetPropertyValue(DgnTextStyleProperty property, double& value) const { return _GetPropertyValue(property, value); }
BentleyStatus DgnTextStyle::SetPropertyValue(DgnTextStyleProperty property, double value) { return _SetPropertyValue(property, value); }
BentleyStatus DgnTextStyle::GetPropertyValue(DgnTextStyleProperty property, DgnFontCP& value) const { return _GetPropertyValue(property, value); }
BentleyStatus DgnTextStyle::SetPropertyValue(DgnTextStyleProperty property, DgnFontCP value) { return _SetPropertyValue(property, value); }
DgnTextStylePtr DgnTextStyle::Clone() const { return _Clone(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
DgnTextStylePtr DgnTextStyle::Create(DgnProjectR project) { return new DgnTextStyle(project); }
DgnTextStyle::DgnTextStyle(DgnProjectR project) :
    m_project(&project)
    {
    // Since we store distances as a factor of text height, it is invalid to have a 0.0 text height.
    // Ultimately, all users of this API should set the height to their desired value anyway.
    // Instead of forcing callers to provide the height up-front, I am choosing to do this as a default.
    m_height = 1.0;

    m_backgroundBorderColor = 0;
    m_backgroundBorderLineStyle = 0;
    m_backgroundBorderPaddingFactor = { 0.0, 0.0 };
    m_backgroundBorderWeight = 0;
    m_backgroundFillColor = 0;
    m_shxBigFont = NULL;
    m_characterSpacingType = CharacterSpacingType::Absolute;
    m_characterSpacingValueFactor = 0.0;
    m_color = 0;
    m_customSlantAngle = 0.0;
    m_font = NULL;
    m_hasColor = false;
    m_isBackwards = false;
    m_isBold = false;
    m_isFullJustification = false;
    m_isItalics = false;
    m_isOverlined = false;
    m_isSubScript = false;
    m_isSuperScript = false;
    m_isUnderlined = false;
    m_isUpsideDown = false;
    m_isVertical = false;
    m_justification = TextElementJustification::LeftTop;
    m_lineSpacingType = DgnLineSpacingType::Exact;
    m_lineSpacingValueFactor = 0.0;
    m_maxCharactersPerLine = 0;
    m_overlineColor = 0;
    m_overlineLineStyle = 0;
    m_overlineOffsetFactor = 0.0;
    m_overlineWeight = 0;
    m_runOffsetFactor = { 0.0, 0.0 };
    m_shouldUseBackground = false;
    m_shouldUseOverlineStyle = false;
    m_shouldUseUnderlineStyle = false;
    m_underlineColor = 0;
    m_underlineLineStyle = 0;
    m_underlineOffsetFactor = 0.0;
    m_underlineWeight = 0;
    m_widthFactor = 0.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
DgnTextStylePtr DgnTextStyle::_Clone() const
    {
    DgnTextStyleP copy = new DgnTextStyle(*m_project);
    copy->m_id = m_id;
    copy->m_name = m_name;
    copy->m_description = m_description;

    copy->CopyPropertyValuesFrom(*this);

    return copy;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
DgnProjectR DgnTextStyle::GetProject() const { return *m_project; }
DgnStyleId DgnTextStyle::GetId() const { return m_id; }
void DgnTextStyle::SetId(DgnStyleId value) { m_id = value; }
Utf8StringCR DgnTextStyle::GetName() const { return m_name; }
void DgnTextStyle::SetName(Utf8CP value) { m_name.AssignOrClear(value); }
Utf8StringCR DgnTextStyle::GetDescription() const { return m_description; }
void DgnTextStyle::SetDescription(Utf8CP value) { m_description.AssignOrClear(value); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyle::_GetPropertyValue(DgnTextStyleProperty property, bool& value) const
    {
    switch (property)
        {
        case DgnTextStyleProperty::HasColor:                value = m_hasColor;                 return SUCCESS;
        case DgnTextStyleProperty::IsBackwards:             value = m_isBackwards;              return SUCCESS;
        case DgnTextStyleProperty::IsBold:                  value = m_isBold;                   return SUCCESS;
        case DgnTextStyleProperty::IsFullJustification:     value = m_isFullJustification;      return SUCCESS;
        case DgnTextStyleProperty::IsItalics:               value = m_isItalics;                return SUCCESS;
        case DgnTextStyleProperty::IsOverlined:             value = m_isOverlined;              return SUCCESS;
        case DgnTextStyleProperty::IsSubScript:             value = m_isSubScript;              return SUCCESS;
        case DgnTextStyleProperty::IsSuperScript:           value = m_isSuperScript;            return SUCCESS;
        case DgnTextStyleProperty::IsUnderlined:            value = m_isUnderlined;             return SUCCESS;
        case DgnTextStyleProperty::IsUpsideDown:            value = m_isUpsideDown;             return SUCCESS;
        case DgnTextStyleProperty::IsVertical:              value = m_isVertical;               return SUCCESS;
        case DgnTextStyleProperty::ShouldUseBackground:     value = m_shouldUseBackground;      return SUCCESS;
        case DgnTextStyleProperty::ShouldUseOverlineStyle:  value = m_shouldUseOverlineStyle;   return SUCCESS;
        case DgnTextStyleProperty::ShouldUseUnderlineStyle: value = m_shouldUseUnderlineStyle;  return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyle::_SetPropertyValue(DgnTextStyleProperty property, bool value)
    {
    switch (property)
        {
        case DgnTextStyleProperty::HasColor:                m_hasColor = value;                 return SUCCESS;
        case DgnTextStyleProperty::IsBackwards:             m_isBackwards = value;              return SUCCESS;
        case DgnTextStyleProperty::IsBold:                  m_isBold = value;                   return SUCCESS;
        case DgnTextStyleProperty::IsFullJustification:     m_isFullJustification = value;      return SUCCESS;
        case DgnTextStyleProperty::IsItalics:               m_isItalics = value;                return SUCCESS;
        case DgnTextStyleProperty::IsOverlined:             m_isOverlined = value;              return SUCCESS;
        case DgnTextStyleProperty::IsUnderlined:            m_isUnderlined = value;             return SUCCESS;
        case DgnTextStyleProperty::IsUpsideDown:            m_isUpsideDown = value;             return SUCCESS;
        case DgnTextStyleProperty::IsVertical:              m_isVertical = value;               return SUCCESS;
        case DgnTextStyleProperty::ShouldUseBackground:     m_shouldUseBackground = value;      return SUCCESS;
        case DgnTextStyleProperty::ShouldUseOverlineStyle:  m_shouldUseOverlineStyle = value;   return SUCCESS;
        case DgnTextStyleProperty::ShouldUseUnderlineStyle: m_shouldUseUnderlineStyle = value;  return SUCCESS;

        case DgnTextStyleProperty::IsSubScript:
            // Cannot be subscript and superscript at the same time.
            PRECONDITION(!value || !m_isSuperScript, ERROR)

            m_isSubScript = value;
            return SUCCESS;
        
        case DgnTextStyleProperty::IsSuperScript:
            // Cannot be subscript and superscript at the same time.
            PRECONDITION(!value || !m_isSubScript, ERROR)

            m_isSuperScript = value;
            return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyle::_GetPropertyValue(DgnTextStyleProperty property, UInt32& value) const
    {
    switch (property)
        {
        case DgnTextStyleProperty::BackgroundBorderColor:   value = m_backgroundBorderColor;        return SUCCESS;
        case DgnTextStyleProperty::BackgroundBorderWeight:  value = m_backgroundBorderWeight;       return SUCCESS;
        case DgnTextStyleProperty::BackgroundFillColor:     value = m_backgroundFillColor;          return SUCCESS;
        case DgnTextStyleProperty::CharacterSpacingType:    value = (UInt32)m_characterSpacingType; return SUCCESS;
        case DgnTextStyleProperty::Color:                   value = m_color;                        return SUCCESS;
        case DgnTextStyleProperty::Justification:           value = (UInt32)m_justification;        return SUCCESS;
        case DgnTextStyleProperty::LineSpacingType:         value = (UInt32)m_lineSpacingType;      return SUCCESS;
        case DgnTextStyleProperty::MaxCharactersPerLine:    value = m_maxCharactersPerLine;         return SUCCESS;
        case DgnTextStyleProperty::OverlineColor:           value = m_overlineColor;                return SUCCESS;
        case DgnTextStyleProperty::OverlineWeight:          value = m_overlineWeight;               return SUCCESS;
        case DgnTextStyleProperty::UnderlineColor:          value = m_underlineColor;               return SUCCESS;
        case DgnTextStyleProperty::UnderlineWeight:         value = m_underlineWeight;              return SUCCESS;
        
        case DgnTextStyleProperty::Font:
            if (NULL == m_font)
                {
                value = 0;
                return ERROR;
                }
            
            return m_project->Fonts().AcquireFontNumber(value, *m_font);

        case DgnTextStyleProperty::ShxBigFont:
            if (NULL == m_shxBigFont)
                {
                // (A) It is valid to not have an SHX big font.
                // (B) Persistence considers 0 to mean no big font, since 0 is never a valid SHX font number.
                value = 0;
                return SUCCESS;
                }
            
            return m_project->Fonts().AcquireFontNumber(value, *m_shxBigFont);
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyle::_SetPropertyValue(DgnTextStyleProperty property, UInt32 value)
    {
    switch (property)
        {
        case DgnTextStyleProperty::BackgroundBorderColor:   m_backgroundBorderColor = value;                        return SUCCESS;
        case DgnTextStyleProperty::BackgroundBorderWeight:  m_backgroundBorderWeight = value;                       return SUCCESS;
        case DgnTextStyleProperty::BackgroundFillColor:     m_backgroundFillColor = value;                          return SUCCESS;
        case DgnTextStyleProperty::Color:                   m_color = value;                                        return SUCCESS;
        case DgnTextStyleProperty::OverlineColor:           m_overlineColor = value;                                return SUCCESS;
        case DgnTextStyleProperty::OverlineWeight:          m_overlineWeight = value;                               return SUCCESS;
        case DgnTextStyleProperty::UnderlineColor:          m_underlineColor = value;                               return SUCCESS;
        case DgnTextStyleProperty::UnderlineWeight:         m_underlineWeight = value;                              return SUCCESS;

        case DgnTextStyleProperty::CharacterSpacingType:
            {
            // Ensure it is a valid enumeration value.
            static const CharacterSpacingType POSSIBLE_VALUES[] = { CharacterSpacingType::Absolute, CharacterSpacingType::FixedWidth, CharacterSpacingType::Factor };
            PRECONDITION(std::any_of(std::begin(POSSIBLE_VALUES), std::end(POSSIBLE_VALUES), [&](CharacterSpacingType possibleValue) { return (CharacterSpacingType)value == possibleValue; }), ERROR)
            
            m_characterSpacingType = (CharacterSpacingType)value;
            return SUCCESS;
            }
        
        case DgnTextStyleProperty::Justification:
            {
            // Ensure it is a valid enumeration value.
            static const TextElementJustification POSSIBLE_VALUES[] = { TextElementJustification::LeftTop, TextElementJustification::LeftMiddle, TextElementJustification::LeftBaseline, TextElementJustification::LeftDescender,
                    TextElementJustification::CenterTop, TextElementJustification::CenterMiddle, TextElementJustification::CenterBaseline, TextElementJustification::CenterDescender,
                    TextElementJustification::RightTop, TextElementJustification::RightMiddle, TextElementJustification::RightBaseline, TextElementJustification::RightDescender,
                    TextElementJustification::LeftMarginTop, TextElementJustification::LeftMarginMiddle, TextElementJustification::LeftMarginBaseline, TextElementJustification::LeftMarginDescender,
                    TextElementJustification::RightMarginTop, TextElementJustification::RightMarginMiddle, TextElementJustification::RightMarginBaseline, TextElementJustification::RightMarginDescender };
            PRECONDITION(std::any_of(std::begin(POSSIBLE_VALUES), std::end(POSSIBLE_VALUES), [&](TextElementJustification possibleValue) { return (TextElementJustification)value == possibleValue; }), ERROR)

            m_justification = (TextElementJustification)value;
            return SUCCESS;
            }
            
        case DgnTextStyleProperty::LineSpacingType:
            {
            // Ensure it is a valid enumeration value.
            static const DgnLineSpacingType POSSIBLE_VALUES[] = { DgnLineSpacingType::Exact, DgnLineSpacingType::Automatic, DgnLineSpacingType::ExactFromLineTop, DgnLineSpacingType::AtLeast };
            PRECONDITION(std::any_of(std::begin(POSSIBLE_VALUES), std::end(POSSIBLE_VALUES), [&](DgnLineSpacingType possibleValue) { return (DgnLineSpacingType)value == possibleValue; }), ERROR)
            
            m_lineSpacingType = (DgnLineSpacingType)value;
            return SUCCESS;
            }
        
        case DgnTextStyleProperty::MaxCharactersPerLine:
            // This is stored in V8 text node elements as a UInt16... ensure it is in that range instead of have a confusing UInt16 overload.
            PRECONDITION(value <= UINT16_MAX, ERROR)
            
            m_maxCharactersPerLine = value;
            return SUCCESS;
        
        case DgnTextStyleProperty::Font:
            {
            // Ensure the requested font exists and that it is NOT an SHX big font.
            DgnFontCP foundFont = m_project->Fonts().FindFont(value);
            
            // We get too many importer cases with missing fonts to just ignore this case...
            // I'd prefer to throw a data assert, but that can also be too noisy in some importer cases. Clone remapper should have reported an issue for missing fonts anyway.
            //if (!EXPECTED_DATA_CONDITION(NULL != foundFont) || !EXPECTED_DATA_CONDITION(DGNFONTVARIANT_ShxBig != foundFont->GetVariant()))
            if ((NULL == foundFont) || (DGNFONTVARIANT_ShxBig == foundFont->GetVariant()))
                foundFont = ((value < 256) ? &DgnFontManager::GetDefaultRscFont() : &DgnFontManager::GetDefaultTrueTypeFont());
            
            m_font = foundFont;
            return SUCCESS;
            }

        case DgnTextStyleProperty::ShxBigFont:
            {
            // While 0 is a valid font number, it is never a valid SHX font number. It is also commonly persisted to mean to big font, so optimize for that case.
            if (0 == value)
                {
                m_shxBigFont = NULL;
                return SUCCESS;
                }
            
            // Ensure the requested font exists and that it is an SHX big font.
            DgnFontCP foundFont = m_project->Fonts().FindFont(value);
            
            // We get too many importer cases with missing fonts to just ignore this case...
            // I'd prefer to throw a data assert, but that can also be too noisy in some importer cases. Clone remapper should have reported an issue for missing fonts anyway.
            // if (!EXPECTED_DATA_CONDITION(NULL != foundFont) || !EXPECTED_DATA_CONDITION(DGNFONTVARIANT_ShxBig == foundFont->GetVariant()))
            if ((NULL == foundFont) || (DGNFONTVARIANT_ShxBig != foundFont->GetVariant()))
                foundFont = DgnFontManager::GetDefaultShxBigFont();
            
            m_shxBigFont = foundFont;
            return SUCCESS;
            }
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyle::_GetPropertyValue(DgnTextStyleProperty property, Int32& value) const
    {
    switch (property)
        {
        case DgnTextStyleProperty::BackgroundBorderLineStyle:   value = m_backgroundBorderLineStyle;    return SUCCESS;
        case DgnTextStyleProperty::OverlineLineStyle:           value = m_overlineLineStyle;            return SUCCESS;
        case DgnTextStyleProperty::UnderlineLineStyle:          value = m_underlineLineStyle;           return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyle::_SetPropertyValue(DgnTextStyleProperty property, Int32 value)
    {
    switch (property)
        {
        case DgnTextStyleProperty::BackgroundBorderLineStyle:   m_backgroundBorderLineStyle = value;    return SUCCESS;
        case DgnTextStyleProperty::OverlineLineStyle:           m_overlineLineStyle = value;            return SUCCESS;
        case DgnTextStyleProperty::UnderlineLineStyle:          m_underlineLineStyle = value;           return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyle::_GetPropertyValue(DgnTextStyleProperty property, DPoint2dR value) const
    {
    switch (property)
        {
        case DgnTextStyleProperty::BackgroundBorderPaddingFactor:   value = m_backgroundBorderPaddingFactor;  return SUCCESS;
        case DgnTextStyleProperty::RunOffsetFactor:                 value = m_runOffsetFactor;                return SUCCESS;
        
        case DgnTextStyleProperty::BackgroundBorderPadding:
            if (UNEXPECTED_CONDITION(m_height <= 0.0))
                return ERROR;
            
            value = DPoint2d::FromScale(m_backgroundBorderPaddingFactor, m_height);
            return SUCCESS;
        
        case DgnTextStyleProperty::RunOffset:
            if (UNEXPECTED_CONDITION(m_height <= 0.0))
                return ERROR;
            
            value = DPoint2d::FromScale(m_runOffsetFactor, m_height);
            return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyle::_SetPropertyValue(DgnTextStyleProperty property, DPoint2dCR value)
    {
    switch (property)
        {
        case DgnTextStyleProperty::BackgroundBorderPaddingFactor:   m_backgroundBorderPaddingFactor = value;  return SUCCESS;
        case DgnTextStyleProperty::RunOffsetFactor:                 m_runOffsetFactor = value;                return SUCCESS;
        
        case DgnTextStyleProperty::BackgroundBorderPadding:
            if (UNEXPECTED_CONDITION(m_height <= 0.0))
                return ERROR;
            
            m_backgroundBorderPaddingFactor = DPoint2d::FromScale(value, (1.0 / m_height));
            return SUCCESS;
        
        case DgnTextStyleProperty::RunOffset:
            if (UNEXPECTED_CONDITION(m_height <= 0.0))
                return ERROR;
            
            m_runOffsetFactor = DPoint2d::FromScale(value, (1.0 / m_height));
            return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyle::_GetPropertyValue(DgnTextStyleProperty property, double& value) const
    {
    switch (property)
        {
        case DgnTextStyleProperty::CharacterSpacingValueFactor: value = m_characterSpacingValueFactor;  return SUCCESS;
        case DgnTextStyleProperty::CustomSlantAngle:            value = m_customSlantAngle;             return SUCCESS;
        case DgnTextStyleProperty::Height:                      value = m_height;                       return SUCCESS;
        case DgnTextStyleProperty::LineSpacingValueFactor:      value = m_lineSpacingValueFactor;       return SUCCESS;
        case DgnTextStyleProperty::OverlineOffsetFactor:        value = m_overlineOffsetFactor;         return SUCCESS;
        case DgnTextStyleProperty::UnderlineOffsetFactor:       value = m_underlineOffsetFactor;        return SUCCESS;
        case DgnTextStyleProperty::WidthFactor:                 value = m_widthFactor;                  return SUCCESS;

        case DgnTextStyleProperty::CharacterSpacingValue:
            if (UNEXPECTED_CONDITION(m_height <= 0.0))
                return ERROR;
            
            if (CharacterSpacingType::Factor != m_characterSpacingType)
                value = (m_characterSpacingValueFactor * m_height);
            else
                value = m_characterSpacingValueFactor;
            
            return SUCCESS;

        case DgnTextStyleProperty::LineSpacingValue:
            if (UNEXPECTED_CONDITION(m_height <= 0.0))
                return ERROR;
            
            if (DgnLineSpacingType::AtLeast != m_lineSpacingType)
                value = (m_lineSpacingValueFactor * m_height);
            else
                value = m_lineSpacingValueFactor;
            
            return SUCCESS;

        case DgnTextStyleProperty::OverlineOffset:
            if (UNEXPECTED_CONDITION(m_height <= 0.0))
                return ERROR;
            
            value = (m_overlineOffsetFactor * m_height);
            return SUCCESS;

        case DgnTextStyleProperty::UnderlineOffset:
            if (UNEXPECTED_CONDITION(m_height <= 0.0))
                return ERROR;
            
            value = (m_underlineOffsetFactor * m_height);
            return SUCCESS;

        case DgnTextStyleProperty::Width:
            if (UNEXPECTED_CONDITION(m_height <= 0.0))
                return ERROR;
            
            value = (m_widthFactor * m_height);
            return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyle::_SetPropertyValue(DgnTextStyleProperty property, double value)
    {
    switch (property)
        {
        case DgnTextStyleProperty::CharacterSpacingValueFactor: m_characterSpacingValueFactor = value;  return SUCCESS;
        case DgnTextStyleProperty::CustomSlantAngle:            m_customSlantAngle = value;             return SUCCESS;
        case DgnTextStyleProperty::Height:                      m_height = value;                       return SUCCESS;
        case DgnTextStyleProperty::LineSpacingValueFactor:      m_lineSpacingValueFactor = value;       return SUCCESS;
        case DgnTextStyleProperty::OverlineOffsetFactor:        m_overlineOffsetFactor = value;         return SUCCESS;
        case DgnTextStyleProperty::UnderlineOffsetFactor:       m_underlineOffsetFactor = value;        return SUCCESS;
        case DgnTextStyleProperty::WidthFactor:                 m_widthFactor = value;                  return SUCCESS;
        
        case DgnTextStyleProperty::CharacterSpacingValue:
            if (UNEXPECTED_CONDITION(m_height <= 0.0))
                return ERROR;
            
            if (CharacterSpacingType::Factor != m_characterSpacingType)
                value = (m_characterSpacingValueFactor / m_height);
            else
                value = m_characterSpacingValueFactor;
            
            return SUCCESS;

        case DgnTextStyleProperty::LineSpacingValue:
            if (UNEXPECTED_CONDITION(m_height <= 0.0))
                return ERROR;
            
            if (DgnLineSpacingType::AtLeast != m_lineSpacingType)
                value = (m_lineSpacingValueFactor / m_height);
            else
                value = m_lineSpacingValueFactor;
            
            return SUCCESS;

        case DgnTextStyleProperty::OverlineOffset:
            if (UNEXPECTED_CONDITION(m_height <= 0.0))
                return ERROR;
            
            value = (m_overlineOffsetFactor / m_height);
            return SUCCESS;

        case DgnTextStyleProperty::UnderlineOffset:
            if (UNEXPECTED_CONDITION(m_height <= 0.0))
                return ERROR;
            
            value = (m_underlineOffsetFactor / m_height);
            return SUCCESS;

        case DgnTextStyleProperty::Width:
            if (UNEXPECTED_CONDITION(m_height <= 0.0))
                return ERROR;
            
            value = (m_widthFactor / m_height);
            return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyle::_GetPropertyValue(DgnTextStyleProperty property, DgnFontCP& value) const
    {
    switch (property)
        {
        case DgnTextStyleProperty::Font:        value = m_font;         return SUCCESS;
        case DgnTextStyleProperty::ShxBigFont:  value = m_shxBigFont;   return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyle::_SetPropertyValue(DgnTextStyleProperty property, DgnFontCP value)
    {
    switch (property)
        {
        case DgnTextStyleProperty::Font:        m_font = value;         return SUCCESS;
        case DgnTextStyleProperty::ShxBigFont:  m_shxBigFont = value;   return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
DgnTextStylePropertyMaskPtr DgnTextStyle::Compare(DgnTextStyleCR rhs) const
    {
    DgnTextStylePropertyMaskPtr diff = DgnTextStylePropertyMask::Create();

    if (m_backgroundBorderColor != rhs.m_backgroundBorderColor)                         diff->SetProperty(DgnTextStyleProperty::BackgroundBorderColor, true);
    if (m_backgroundBorderLineStyle != rhs.m_backgroundBorderLineStyle)                 diff->SetProperty(DgnTextStyleProperty::BackgroundBorderLineStyle, true);
    if (!m_backgroundBorderPaddingFactor.IsEqual(rhs.m_backgroundBorderPaddingFactor))  diff->SetProperty(DgnTextStyleProperty::BackgroundBorderPaddingFactor, true);
    if (m_backgroundBorderWeight != rhs.m_backgroundBorderWeight)                       diff->SetProperty(DgnTextStyleProperty::BackgroundBorderWeight, true);
    if (m_backgroundFillColor != rhs.m_backgroundFillColor)                             diff->SetProperty(DgnTextStyleProperty::BackgroundFillColor, true);
    if (m_shxBigFont != rhs.m_shxBigFont)                                               diff->SetProperty(DgnTextStyleProperty::ShxBigFont, true);
    if (m_characterSpacingType != rhs.m_characterSpacingType)                           diff->SetProperty(DgnTextStyleProperty::CharacterSpacingType, true);
    if (m_characterSpacingValueFactor != rhs.m_characterSpacingValueFactor)             diff->SetProperty(DgnTextStyleProperty::CharacterSpacingValueFactor, true);
    if (m_color != rhs.m_color)                                                         diff->SetProperty(DgnTextStyleProperty::Color, true);
    if (m_customSlantAngle != rhs.m_customSlantAngle)                                   diff->SetProperty(DgnTextStyleProperty::CustomSlantAngle, true);
    if (m_font != rhs.m_font)                                                           diff->SetProperty(DgnTextStyleProperty::Font, true);
    if (m_hasColor != rhs.m_hasColor)                                                   diff->SetProperty(DgnTextStyleProperty::HasColor, true);
    if (m_height != rhs.m_height)                                                       diff->SetProperty(DgnTextStyleProperty::Height, true);
    if (m_isBackwards != rhs.m_isBackwards)                                             diff->SetProperty(DgnTextStyleProperty::IsBackwards, true);
    if (m_isBold != rhs.m_isBold)                                                       diff->SetProperty(DgnTextStyleProperty::IsBold, true);
    if (m_isFullJustification != rhs.m_isFullJustification)                             diff->SetProperty(DgnTextStyleProperty::IsFullJustification, true);
    if (m_isItalics != rhs.m_isItalics)                                                 diff->SetProperty(DgnTextStyleProperty::IsItalics, true);
    if (m_isOverlined != rhs.m_isOverlined)                                             diff->SetProperty(DgnTextStyleProperty::IsOverlined, true);
    if (m_isSubScript != rhs.m_isSubScript)                                             diff->SetProperty(DgnTextStyleProperty::IsSubScript, true);
    if (m_isSuperScript != rhs.m_isSuperScript)                                         diff->SetProperty(DgnTextStyleProperty::IsSuperScript, true);
    if (m_isUnderlined != rhs.m_isUnderlined)                                           diff->SetProperty(DgnTextStyleProperty::IsUnderlined, true);
    if (m_isUpsideDown != rhs.m_isUpsideDown)                                           diff->SetProperty(DgnTextStyleProperty::IsUpsideDown, true);
    if (m_isVertical != rhs.m_isVertical)                                               diff->SetProperty(DgnTextStyleProperty::IsVertical, true);
    if (m_justification != rhs.m_justification)                                         diff->SetProperty(DgnTextStyleProperty::Justification, true);
    if (m_lineSpacingType != rhs.m_lineSpacingType)                                     diff->SetProperty(DgnTextStyleProperty::LineSpacingType, true);
    if (m_lineSpacingValueFactor != rhs.m_lineSpacingValueFactor)                       diff->SetProperty(DgnTextStyleProperty::LineSpacingValueFactor, true);
    if (m_maxCharactersPerLine != rhs.m_maxCharactersPerLine)                           diff->SetProperty(DgnTextStyleProperty::MaxCharactersPerLine, true);
    if (m_overlineColor != rhs.m_overlineColor)                                         diff->SetProperty(DgnTextStyleProperty::OverlineColor, true);
    if (m_overlineLineStyle != rhs.m_overlineLineStyle)                                 diff->SetProperty(DgnTextStyleProperty::OverlineLineStyle, true);
    if (m_overlineOffsetFactor != rhs.m_overlineOffsetFactor)                           diff->SetProperty(DgnTextStyleProperty::OverlineOffsetFactor, true);
    if (m_overlineWeight != rhs.m_overlineWeight)                                       diff->SetProperty(DgnTextStyleProperty::OverlineWeight, true);
    if (!m_runOffsetFactor.IsEqual(rhs.m_runOffsetFactor))                              diff->SetProperty(DgnTextStyleProperty::RunOffsetFactor, true);
    if (m_shouldUseBackground != rhs.m_shouldUseBackground)                             diff->SetProperty(DgnTextStyleProperty::ShouldUseBackground, true);
    if (m_shouldUseOverlineStyle != rhs.m_shouldUseOverlineStyle)                       diff->SetProperty(DgnTextStyleProperty::ShouldUseOverlineStyle, true);
    if (m_shouldUseUnderlineStyle != rhs.m_shouldUseUnderlineStyle)                     diff->SetProperty(DgnTextStyleProperty::ShouldUseUnderlineStyle, true);
    if (m_underlineColor != rhs.m_underlineColor)                                       diff->SetProperty(DgnTextStyleProperty::UnderlineColor, true);
    if (m_underlineLineStyle != rhs.m_underlineLineStyle)                               diff->SetProperty(DgnTextStyleProperty::UnderlineLineStyle, true);
    if (m_underlineOffsetFactor != rhs.m_underlineOffsetFactor)                         diff->SetProperty(DgnTextStyleProperty::UnderlineOffsetFactor, true);
    if (m_underlineWeight != rhs.m_underlineWeight)                                     diff->SetProperty(DgnTextStyleProperty::UnderlineWeight, true);
    if (m_widthFactor != rhs.m_widthFactor)                                             diff->SetProperty(DgnTextStyleProperty::WidthFactor, true);

    return diff;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
void DgnTextStyle::CopyPropertyValuesFrom(DgnTextStyleCR rhs)
    {
    DgnTextStylePropertyMaskPtr allProperties = DgnTextStylePropertyMask::Create();
    allProperties->SetAllProperties(true);
    CopyPropertyValuesFrom(rhs, *allProperties);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
void DgnTextStyle::CopyPropertyValuesFrom(DgnTextStyleCR rhs, DgnTextStylePropertyMaskCR mask)
    {
    PRECONDITION(m_project == rhs.m_project, )

    if (mask.IsPropertySet(DgnTextStyleProperty::BackgroundBorderColor))            m_backgroundBorderColor = rhs.m_backgroundBorderColor;
    if (mask.IsPropertySet(DgnTextStyleProperty::BackgroundBorderLineStyle))        m_backgroundBorderLineStyle = rhs.m_backgroundBorderLineStyle;
    if (mask.IsPropertySet(DgnTextStyleProperty::BackgroundBorderPaddingFactor))    m_backgroundBorderPaddingFactor = rhs.m_backgroundBorderPaddingFactor;
    if (mask.IsPropertySet(DgnTextStyleProperty::BackgroundBorderWeight))           m_backgroundBorderWeight = rhs.m_backgroundBorderWeight;
    if (mask.IsPropertySet(DgnTextStyleProperty::BackgroundFillColor))              m_backgroundFillColor = rhs.m_backgroundFillColor;
    if (mask.IsPropertySet(DgnTextStyleProperty::ShxBigFont))                       m_shxBigFont = rhs.m_shxBigFont;
    if (mask.IsPropertySet(DgnTextStyleProperty::CharacterSpacingType))             m_characterSpacingType = rhs.m_characterSpacingType;
    if (mask.IsPropertySet(DgnTextStyleProperty::CharacterSpacingValueFactor))      m_characterSpacingValueFactor = rhs.m_characterSpacingValueFactor;
    if (mask.IsPropertySet(DgnTextStyleProperty::Color))                            m_color = rhs.m_color;
    if (mask.IsPropertySet(DgnTextStyleProperty::CustomSlantAngle))                 m_customSlantAngle = rhs.m_customSlantAngle;
    if (mask.IsPropertySet(DgnTextStyleProperty::Font))                             m_font = rhs.m_font;
    if (mask.IsPropertySet(DgnTextStyleProperty::HasColor))                         m_hasColor = rhs.m_hasColor;
    if (mask.IsPropertySet(DgnTextStyleProperty::Height))                           m_height = rhs.m_height;
    if (mask.IsPropertySet(DgnTextStyleProperty::IsBackwards))                      m_isBackwards = rhs.m_isBackwards;
    if (mask.IsPropertySet(DgnTextStyleProperty::IsBold))                           m_isBold = rhs.m_isBold;
    if (mask.IsPropertySet(DgnTextStyleProperty::IsFullJustification))              m_isFullJustification = rhs.m_isFullJustification;
    if (mask.IsPropertySet(DgnTextStyleProperty::IsItalics))                        m_isItalics = rhs.m_isItalics;
    if (mask.IsPropertySet(DgnTextStyleProperty::IsOverlined))                      m_isOverlined = rhs.m_isOverlined;
    if (mask.IsPropertySet(DgnTextStyleProperty::IsSubScript))                      m_isSubScript = rhs.m_isSubScript;
    if (mask.IsPropertySet(DgnTextStyleProperty::IsSuperScript))                    m_isSuperScript = rhs.m_isSuperScript;
    if (mask.IsPropertySet(DgnTextStyleProperty::IsUnderlined))                     m_isUnderlined = rhs.m_isUnderlined;
    if (mask.IsPropertySet(DgnTextStyleProperty::IsUpsideDown))                     m_isUpsideDown = rhs.m_isUpsideDown;
    if (mask.IsPropertySet(DgnTextStyleProperty::IsVertical))                       m_isVertical = rhs.m_isVertical;
    if (mask.IsPropertySet(DgnTextStyleProperty::Justification))                    m_justification = rhs.m_justification;
    if (mask.IsPropertySet(DgnTextStyleProperty::LineSpacingType))                  m_lineSpacingType = rhs.m_lineSpacingType;
    if (mask.IsPropertySet(DgnTextStyleProperty::LineSpacingValueFactor))           m_lineSpacingValueFactor = rhs.m_lineSpacingValueFactor;
    if (mask.IsPropertySet(DgnTextStyleProperty::MaxCharactersPerLine))             m_maxCharactersPerLine = rhs.m_maxCharactersPerLine;
    if (mask.IsPropertySet(DgnTextStyleProperty::OverlineColor))                    m_overlineColor = rhs.m_overlineColor;
    if (mask.IsPropertySet(DgnTextStyleProperty::OverlineLineStyle))                m_overlineLineStyle = rhs.m_overlineLineStyle;
    if (mask.IsPropertySet(DgnTextStyleProperty::OverlineOffsetFactor))             m_overlineOffsetFactor = rhs.m_overlineOffsetFactor;
    if (mask.IsPropertySet(DgnTextStyleProperty::OverlineWeight))                   m_overlineWeight = rhs.m_overlineWeight;
    if (mask.IsPropertySet(DgnTextStyleProperty::RunOffsetFactor))                  m_runOffsetFactor = rhs.m_runOffsetFactor;
    if (mask.IsPropertySet(DgnTextStyleProperty::ShouldUseBackground))              m_shouldUseBackground = rhs.m_shouldUseBackground;
    if (mask.IsPropertySet(DgnTextStyleProperty::ShouldUseOverlineStyle))           m_shouldUseOverlineStyle = rhs.m_shouldUseOverlineStyle;
    if (mask.IsPropertySet(DgnTextStyleProperty::ShouldUseUnderlineStyle))          m_shouldUseUnderlineStyle = rhs.m_shouldUseUnderlineStyle;
    if (mask.IsPropertySet(DgnTextStyleProperty::UnderlineColor))                   m_underlineColor = rhs.m_underlineColor;
    if (mask.IsPropertySet(DgnTextStyleProperty::UnderlineLineStyle))               m_underlineLineStyle = rhs.m_underlineLineStyle;
    if (mask.IsPropertySet(DgnTextStyleProperty::UnderlineOffsetFactor))            m_underlineOffsetFactor = rhs.m_underlineOffsetFactor;
    if (mask.IsPropertySet(DgnTextStyleProperty::UnderlineWeight))                  m_underlineWeight = rhs.m_underlineWeight;
    if (mask.IsPropertySet(DgnTextStyleProperty::WidthFactor))                      m_widthFactor = rhs.m_widthFactor;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
bool DgnTextStyle::ProcessProperties(PropertyContextR proc, bool canChange)
    {
    bool wasChanged = false;

    // Fonts
    UInt32 fontID;
    
    if (EXPECTED_CONDITION(SUCCESS == GetPropertyValue(DgnTextStyleProperty::Font, fontID))
        && proc.DoFontCallback(canChange ? &fontID : NULL, EachFontArg(fontID, PROPSCALLBACK_FLAGS_NoFlagsSet, proc)))
        {
        wasChanged = true;
        SetPropertyValue(DgnTextStyleProperty::Font, fontID);
        }

    if (SUCCESS == GetPropertyValue(DgnTextStyleProperty::ShxBigFont, fontID)
        && proc.DoFontCallback(canChange ? &fontID : NULL, EachFontArg(fontID, PROPSCALLBACK_FLAGS_NoFlagsSet, proc)))
        {
        wasChanged = true;
        SetPropertyValue(DgnTextStyleProperty::ShxBigFont, fontID);
        }

    // Colors
    wasChanged |= proc.DoColorCallback(canChange ? &m_color : NULL, EachColorArg(m_color, SETPROPCBEIFLAG(m_hasColor), proc));
    wasChanged |= proc.DoColorCallback(canChange ? &m_backgroundFillColor : NULL, EachColorArg(m_backgroundFillColor, (PropsCallbackFlags)(PROPSCALLBACK_FLAGS_IsBackgroundID | SETPROPCBEIFLAG(m_shouldUseBackground)), proc));
    wasChanged |= proc.DoColorCallback(canChange ? &m_underlineColor : NULL, EachColorArg(m_underlineColor, SETPROPCBEIFLAG(m_shouldUseUnderlineStyle), proc));
    wasChanged |= proc.DoColorCallback(canChange ? &m_overlineColor : NULL, EachColorArg(m_overlineColor, SETPROPCBEIFLAG(m_shouldUseOverlineStyle), proc));
    wasChanged |= proc.DoColorCallback(canChange ? &m_backgroundBorderColor : NULL, EachColorArg(m_backgroundBorderColor, SETPROPCBEIFLAG(m_shouldUseBackground), proc));

    // Weights
    wasChanged |= proc.DoWeightCallback(canChange ? &m_underlineWeight : NULL, EachWeightArg(m_underlineWeight, SETPROPCBEIFLAG(m_shouldUseUnderlineStyle), proc));
    wasChanged |= proc.DoWeightCallback(canChange ? &m_overlineWeight : NULL, EachWeightArg(m_overlineWeight, SETPROPCBEIFLAG(m_shouldUseOverlineStyle), proc));
    wasChanged |= proc.DoWeightCallback(canChange ? &m_backgroundBorderWeight : NULL, EachWeightArg(m_backgroundBorderWeight, SETPROPCBEIFLAG(m_shouldUseBackground), proc));

    // LineStyles
    wasChanged |= proc.DoLineStyleCallback(canChange ? &m_underlineLineStyle : NULL, EachLineStyleArg(m_underlineLineStyle, NULL, SETPROPCBEIFLAG(m_shouldUseUnderlineStyle), proc));
    wasChanged |= proc.DoLineStyleCallback(canChange ? &m_overlineLineStyle : NULL, EachLineStyleArg(m_overlineLineStyle, NULL, SETPROPCBEIFLAG(m_shouldUseOverlineStyle), proc));
    wasChanged |= proc.DoLineStyleCallback(canChange ? &m_backgroundBorderLineStyle : NULL, EachLineStyleArg(m_backgroundBorderLineStyle, NULL, SETPROPCBEIFLAG(m_shouldUseBackground), proc));

    return wasChanged;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2014
//---------------------------------------------------------------------------------------
bool DgnTextStyle::ProcessPropertiesForClone(PropertyContextR context, bool applyScale)
    {
    DgnModelP sourceDgnModel = context.GetSourceDgnModel();
    DgnModelP destinationDgnModel = context.GetDestinationDgnModel();
    
    PRECONDITION(NULL != sourceDgnModel, false);
    PRECONDITION(NULL != destinationDgnModel, false);
    
    // Re-map IDs.
    bool wasChanged = ProcessProperties(context, true);
    
    // Scale height (the only distance; others are factors of height).
    DgnProjectR destinationProject = destinationDgnModel->GetDgnProject();
    
    // Done; assume the new project.
    m_project = &destinationProject;

    return wasChanged;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
void DgnTextStyle::Scale(double factor)
    {
    m_height *= factor;
    m_runOffsetFactor.Scale(factor);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
Utf8String DgnTextStylePersistence::ToJson(DgnTextStyleCR textStyle)
    {
    Json::Value textStyleJson(Json::objectValue);

    if (textStyle.m_hasColor)                               textStyleJson["hasColor"] = Json::Value(textStyle.m_hasColor);
    if (textStyle.m_isBackwards)                            textStyleJson["isBackwards"] = Json::Value(textStyle.m_isBackwards);
    if (textStyle.m_isBold)                                 textStyleJson["isBold"] = Json::Value(textStyle.m_isBold);
    if (textStyle.m_isFullJustification)                    textStyleJson["isFullJustification"] = Json::Value(textStyle.m_isFullJustification);
    if (textStyle.m_isItalics)                              textStyleJson["isItalics"] = Json::Value(textStyle.m_isItalics);
    if (textStyle.m_isOverlined)                            textStyleJson["isOverlined"] = Json::Value(textStyle.m_isOverlined);
    if (textStyle.m_isSubScript)                            textStyleJson["isSubScript"] = Json::Value(textStyle.m_isSubScript);
    if (textStyle.m_isSuperScript)                          textStyleJson["isSuperScript"] = Json::Value(textStyle.m_isSuperScript);
    if (textStyle.m_isUnderlined)                           textStyleJson["isUnderlined"] = Json::Value(textStyle.m_isUnderlined);
    if (textStyle.m_isUpsideDown)                           textStyleJson["isUpsideDown"] = Json::Value(textStyle.m_isUpsideDown);
    if (textStyle.m_isVertical)                             textStyleJson["isVertical"] = Json::Value(textStyle.m_isVertical);
    if (textStyle.m_shouldUseBackground)                    textStyleJson["shouldUseBackground"] = Json::Value(textStyle.m_shouldUseBackground);
    if (textStyle.m_shouldUseOverlineStyle)                 textStyleJson["shouldUseOverlineStyle"] = Json::Value(textStyle.m_shouldUseOverlineStyle);
    if (textStyle.m_shouldUseUnderlineStyle)                textStyleJson["shouldUseUnderlineStyle"] = Json::Value(textStyle.m_shouldUseUnderlineStyle);
    if (0.0 != textStyle.m_characterSpacingValueFactor)     textStyleJson["characterSpacingValue"] = Json::Value(textStyle.m_characterSpacingValueFactor);
    if (0.0 != textStyle.m_customSlantAngle)                textStyleJson["customSlantAngle"] = Json::Value(textStyle.m_customSlantAngle);
    if (0.0 != textStyle.m_height)                          textStyleJson["height"] = Json::Value(textStyle.m_height);
    if (0.0 != textStyle.m_lineSpacingValueFactor)          textStyleJson["lineSpacingValue"] = Json::Value(textStyle.m_lineSpacingValueFactor);
    if (0.0 != textStyle.m_overlineOffsetFactor)            textStyleJson["overlineOffset"] = Json::Value(textStyle.m_overlineOffsetFactor);
    if (0.0 != textStyle.m_underlineOffsetFactor)           textStyleJson["underlineOffset"] = Json::Value(textStyle.m_underlineOffsetFactor);
    if (0.0 != textStyle.m_widthFactor)                     textStyleJson["widthFactor"] = Json::Value(textStyle.m_widthFactor);
    if (0.0 != textStyle.m_backgroundBorderPaddingFactor.x) textStyleJson["backgroundBorderPadding.x"] = Json::Value(textStyle.m_backgroundBorderPaddingFactor.x);
    if (0.0 != textStyle.m_backgroundBorderPaddingFactor.y) textStyleJson["backgroundBorderPadding.y"] = Json::Value(textStyle.m_backgroundBorderPaddingFactor.y);
    if (0.0 != textStyle.m_runOffsetFactor.x)               textStyleJson["runOffset.x"] = Json::Value(textStyle.m_runOffsetFactor.x);
    if (0.0 != textStyle.m_runOffsetFactor.y)               textStyleJson["runOffset.y"] = Json::Value(textStyle.m_runOffsetFactor.y);
    if (0 != textStyle.m_backgroundBorderLineStyle)         textStyleJson["backgroundBorderLineStyle"] = Json::Value((Json::Int)textStyle.m_backgroundBorderLineStyle);
    if (0 != textStyle.m_overlineLineStyle)                 textStyleJson["overlineLineStyle"] = Json::Value((Json::Int)textStyle.m_overlineLineStyle);
    if (0 != textStyle.m_underlineLineStyle)                textStyleJson["underlineLineStyle"] = Json::Value((Json::Int)textStyle.m_underlineLineStyle);
    if (0 != textStyle.m_backgroundBorderColor)             textStyleJson["backgroundBorderColor"] = Json::Value((Json::UInt)textStyle.m_backgroundBorderColor);
    if (0 != textStyle.m_backgroundBorderWeight)            textStyleJson["backgroundBorderWeight"] = Json::Value((Json::UInt)textStyle.m_backgroundBorderWeight);
    if (0 != textStyle.m_backgroundFillColor)               textStyleJson["backgroundFillColor"] = Json::Value((Json::UInt)textStyle.m_backgroundFillColor);
    if (0 != textStyle.m_color)                             textStyleJson["color"] = Json::Value((Json::UInt)textStyle.m_color);
    if (0 != textStyle.m_maxCharactersPerLine)              textStyleJson["maxCharactersPerLine"] = Json::Value((Json::UInt)textStyle.m_maxCharactersPerLine);
    if (0 != textStyle.m_overlineColor)                     textStyleJson["overlineColor"] = Json::Value((Json::UInt)textStyle.m_overlineColor);
    if (0 != textStyle.m_overlineWeight)                    textStyleJson["overlineWeight"] = Json::Value((Json::UInt)textStyle.m_overlineWeight);
    if (0 != textStyle.m_underlineColor)                    textStyleJson["underlineColor"] = Json::Value((Json::UInt)textStyle.m_underlineColor);
    if (0 != textStyle.m_underlineWeight)                   textStyleJson["underlineWeight"] = Json::Value((Json::UInt)textStyle.m_underlineWeight);
    if (0 != (UInt32)textStyle.m_characterSpacingType)      textStyleJson["characterSpacingType"] = Json::Value((Json::UInt)textStyle.m_characterSpacingType);
    if (0 != (UInt32)textStyle.m_lineSpacingType)           textStyleJson["lineSpacingType"] = Json::Value((Json::UInt)textStyle.m_lineSpacingType);
    if (0 != (UInt32)textStyle.m_justification)             textStyleJson["justification"] = Json::Value((Json::UInt)textStyle.m_justification);

    // It is important to call GetPropertyValue/SetPropertyValue for dealing with font numbers so that sub-classes can control the behavior.
    UInt32 fontID;
    if (SUCCESS == textStyle.GetPropertyValue(DgnTextStyleProperty::Font, fontID))
        textStyleJson["font"] = Json::Value((Json::UInt)fontID);
    
    if (SUCCESS == textStyle.GetPropertyValue(DgnTextStyleProperty::ShxBigFont, fontID))
        textStyleJson["shxBigFont"] = Json::Value((Json::UInt)fontID);

    return Json::FastWriter::ToString(textStyleJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStylePersistence::FromJson(DgnTextStyleR textStyle, Utf8CP jsonString)
    {
    Json::Value textStyleJson;
    if (UNEXPECTED_CONDITION(!Json::Reader::Parse(jsonString, textStyleJson)))
        return ERROR;
    
    Json::Value jsonValue;

    jsonValue = textStyleJson["hasColor"];                  if (jsonValue.isBool()) textStyle.m_hasColor = jsonValue.asBool();                                          else { textStyle.m_hasColor = false; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["isBackwards"];               if (jsonValue.isBool()) textStyle.m_isBackwards = jsonValue.asBool();                                       else { textStyle.m_isBackwards = false; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["isBold"];                    if (jsonValue.isBool()) textStyle.m_isBold = jsonValue.asBool();                                            else { textStyle.m_isBold = false; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["isFullJustification"];       if (jsonValue.isBool()) textStyle.m_isFullJustification = jsonValue.asBool();                               else { textStyle.m_isFullJustification = false; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["isItalics"];                 if (jsonValue.isBool()) textStyle.m_isItalics = jsonValue.asBool();                                         else { textStyle.m_isItalics = false; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["isOverlined"];               if (jsonValue.isBool()) textStyle.m_isOverlined = jsonValue.asBool();                                       else { textStyle.m_isOverlined = false; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["isSubScript"];               if (jsonValue.isBool()) textStyle.m_isSubScript = jsonValue.asBool();                                       else { textStyle.m_isSubScript = false; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["isSuperScript"];             if (jsonValue.isBool()) textStyle.m_isSuperScript = jsonValue.asBool();                                     else { textStyle.m_isSuperScript = false; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["isUnderlined"];              if (jsonValue.isBool()) textStyle.m_isUnderlined = jsonValue.asBool();                                      else { textStyle.m_isUnderlined = false; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["isUpsideDown"];              if (jsonValue.isBool()) textStyle.m_isUpsideDown = jsonValue.asBool();                                      else { textStyle.m_isUpsideDown = false; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["isVertical"];                if (jsonValue.isBool()) textStyle.m_isVertical = jsonValue.asBool();                                        else { textStyle.m_isVertical = false; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["shouldUseBackground"];       if (jsonValue.isBool()) textStyle.m_shouldUseBackground = jsonValue.asBool();                               else { textStyle.m_shouldUseBackground = false; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["shouldUseOverlineStyle"];    if (jsonValue.isBool()) textStyle.m_shouldUseOverlineStyle = jsonValue.asBool();                            else { textStyle.m_shouldUseOverlineStyle = false; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["shouldUseUnderlineStyle"];   if (jsonValue.isBool()) textStyle.m_shouldUseUnderlineStyle = jsonValue.asBool();                           else { textStyle.m_shouldUseUnderlineStyle = false; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["characterSpacingValue"];     if (jsonValue.isDouble()) textStyle.m_characterSpacingValueFactor = jsonValue.asDouble();                   else { textStyle.m_characterSpacingValueFactor = 0.0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["customSlantAngle"];          if (jsonValue.isDouble()) textStyle.m_customSlantAngle = jsonValue.asDouble();                              else { textStyle.m_customSlantAngle = 0.0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["height"];                    if (jsonValue.isDouble()) textStyle.m_height = jsonValue.asDouble();                                        else { textStyle.m_height = 0.0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["lineSpacingValue"];          if (jsonValue.isDouble()) textStyle.m_lineSpacingValueFactor = jsonValue.asDouble();                        else { textStyle.m_lineSpacingValueFactor = 0.0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["overlineOffset"];            if (jsonValue.isDouble()) textStyle.m_overlineOffsetFactor = jsonValue.asDouble();                          else { textStyle.m_overlineOffsetFactor = 0.0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["underlineOffset"];           if (jsonValue.isDouble()) textStyle.m_underlineOffsetFactor = jsonValue.asDouble();                         else { textStyle.m_underlineOffsetFactor = 0.0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["widthFactor"];               if (jsonValue.isDouble()) textStyle.m_widthFactor = jsonValue.asDouble();                                   else { textStyle.m_widthFactor = 0.0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["backgroundBorderPadding.x"]; if (jsonValue.isDouble()) textStyle.m_backgroundBorderPaddingFactor.x = jsonValue.asDouble();               else { textStyle.m_backgroundBorderPaddingFactor.x = 0.0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["backgroundBorderPadding.y"]; if (jsonValue.isDouble()) textStyle.m_backgroundBorderPaddingFactor.y = jsonValue.asDouble();               else { textStyle.m_backgroundBorderPaddingFactor.y = 0.0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["runOffset.x"];               if (jsonValue.isDouble()) textStyle.m_runOffsetFactor.x = jsonValue.asDouble();                             else { textStyle.m_runOffsetFactor.x = 0.0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["runOffset.y"];               if (jsonValue.isDouble()) textStyle.m_runOffsetFactor.y = jsonValue.asDouble();                             else { textStyle.m_runOffsetFactor.y = 0.0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["backgroundBorderLineStyle"]; if (jsonValue.isIntegral()) textStyle.m_backgroundBorderLineStyle = jsonValue.asInt();                      else { textStyle.m_backgroundBorderLineStyle = 0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["overlineLineStyle"];         if (jsonValue.isIntegral()) textStyle.m_overlineLineStyle = jsonValue.asInt();                              else { textStyle.m_overlineLineStyle = 0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["underlineLineStyle"];        if (jsonValue.isIntegral()) textStyle.m_underlineLineStyle = jsonValue.asInt();                             else { textStyle.m_underlineLineStyle = 0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["backgroundBorderColor"];     if (jsonValue.isIntegral()) textStyle.m_backgroundBorderColor = jsonValue.asUInt();                         else { textStyle.m_backgroundBorderColor = 0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["backgroundBorderWeight"];    if (jsonValue.isIntegral()) textStyle.m_backgroundBorderWeight = jsonValue.asUInt();                        else { textStyle.m_backgroundBorderWeight = 0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["backgroundFillColor"];       if (jsonValue.isIntegral()) textStyle.m_backgroundFillColor = jsonValue.asUInt();                           else { textStyle.m_backgroundFillColor = 0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["color"];                     if (jsonValue.isIntegral()) textStyle.m_color = jsonValue.asUInt();                                         else { textStyle.m_color = 0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["maxCharactersPerLine"];      if (jsonValue.isIntegral()) textStyle.m_maxCharactersPerLine = jsonValue.asUInt();                          else { textStyle.m_maxCharactersPerLine = 0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["overlineColor"];             if (jsonValue.isIntegral()) textStyle.m_overlineColor = jsonValue.asUInt();                                 else { textStyle.m_overlineColor = 0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["overlineWeight"];            if (jsonValue.isIntegral()) textStyle.m_overlineWeight = jsonValue.asUInt();                                else { textStyle.m_overlineWeight = 0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["underlineColor"];            if (jsonValue.isIntegral()) textStyle.m_underlineColor = jsonValue.asUInt();                                else { textStyle.m_underlineColor = 0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["underlineWeight"];           if (jsonValue.isIntegral()) textStyle.m_underlineWeight = jsonValue.asUInt();                               else { textStyle.m_underlineWeight = 0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["characterSpacingType"];      if (jsonValue.isIntegral()) textStyle.m_characterSpacingType = (CharacterSpacingType)jsonValue.asUInt();    else { textStyle.m_characterSpacingType = (CharacterSpacingType)0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["lineSpacingType"];           if (jsonValue.isIntegral()) textStyle.m_lineSpacingType = (DgnLineSpacingType)jsonValue.asUInt();           else { textStyle.m_lineSpacingType = (DgnLineSpacingType)0; BeDataAssert(jsonValue.isNull()); }
    jsonValue = textStyleJson["justification"];             if (jsonValue.isIntegral()) textStyle.m_justification = (TextElementJustification)jsonValue.asUInt();       else { textStyle.m_justification = (TextElementJustification)0; BeDataAssert(jsonValue.isNull()); }

    // It is important to call GetPropertyValue/SetPropertyValue for dealing with font numbers so that sub-classes can control the behavior.
    jsonValue = textStyleJson["font"];
    if (jsonValue.isIntegral())
        {
        UInt32 fontID = textStyleJson["font"].asUInt();
        EXPECTED_DATA_CONDITION(SUCCESS == textStyle.SetPropertyValue(DgnTextStyleProperty::Font, fontID));
        }
    else
        {
        textStyle.m_font = NULL;
        BeDataAssert(jsonValue.isNull());
        }
    
    jsonValue = textStyleJson["shxBigFont"];
    if (jsonValue.isIntegral())
        {
        UInt32 fontID = textStyleJson["shxBigFont"].asUInt();
        EXPECTED_DATA_CONDITION(SUCCESS == textStyle.SetPropertyValue(DgnTextStyleProperty::ShxBigFont, fontID));
        }
    else
        {
        textStyle.m_shxBigFont = NULL;
        BeDataAssert(jsonValue.isNull());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
LegacyTextStyle DgnTextStylePersistence::Legacy::ToLegacyStyle(DgnTextStyleCR textStyle)
    {
    LegacyTextStyle legacyStyle;
    memset(&legacyStyle, 0, sizeof(legacyStyle));

    PRECONDITION(textStyle.m_height > 0.0, legacyStyle)

    // Direct mappings
    legacyStyle.backgroundFillColor = textStyle.m_backgroundFillColor;
    legacyStyle.backgroundStyle.color = textStyle.m_backgroundBorderColor;
    legacyStyle.backgroundStyle.style = textStyle.m_backgroundBorderLineStyle;
    legacyStyle.backgroundStyle.weight = textStyle.m_backgroundBorderWeight;
    legacyStyle.color = textStyle.m_color;
    legacyStyle.flags.acadInterCharSpacing = (CharacterSpacingType::Factor == textStyle.m_characterSpacingType);
    legacyStyle.flags.acadLineSpacingType = (UInt32)textStyle.m_lineSpacingType;
    legacyStyle.flags.background = textStyle.m_shouldUseBackground;
    legacyStyle.flags.bold = textStyle.m_isBold;
    legacyStyle.flags.color = textStyle.m_hasColor;
    legacyStyle.flags.fixedSpacing = (CharacterSpacingType::FixedWidth == textStyle.m_characterSpacingType);
    legacyStyle.flags.fullJustification = textStyle.m_isFullJustification;
    legacyStyle.flags.italics = textStyle.m_isItalics;
    legacyStyle.flags.overline = textStyle.m_isOverlined;
    legacyStyle.flags.overlineStyle = textStyle.m_shouldUseOverlineStyle;
    legacyStyle.flags.subscript = textStyle.m_isSubScript;
    legacyStyle.flags.superscript = textStyle.m_isSuperScript;
    legacyStyle.flags.underline = textStyle.m_isUnderlined;
    legacyStyle.flags.underlineStyle = textStyle.m_shouldUseUnderlineStyle;
    legacyStyle.height = textStyle.m_height;
    legacyStyle.just = (UInt16)textStyle.m_justification;
    legacyStyle.lineLength = (UInt16)textStyle.m_maxCharactersPerLine;
    legacyStyle.lineOffset = textStyle.m_runOffsetFactor;
    legacyStyle.nodeJust = (UInt16)textStyle.m_justification;
    legacyStyle.overlineStyle.color = textStyle.m_overlineColor;
    legacyStyle.overlineStyle.style = textStyle.m_overlineLineStyle;
    legacyStyle.overlineStyle.weight = textStyle.m_overlineWeight;
    legacyStyle.parentId = 0;
    legacyStyle.slant = textStyle.m_customSlantAngle;
    legacyStyle.underlineStyle.color = textStyle.m_underlineColor;
    legacyStyle.underlineStyle.style = textStyle.m_underlineLineStyle;
    legacyStyle.underlineStyle.weight = textStyle.m_underlineWeight;
    legacyStyle.widthFactor = textStyle.m_widthFactor;

    // Convert height-based factors to distances
    legacyStyle.backgroundBorder = DPoint2d::FromScale(textStyle.m_backgroundBorderPaddingFactor, textStyle.m_height);
    legacyStyle.interCharSpacing = ((CharacterSpacingType::Factor != textStyle.m_characterSpacingType) ? (textStyle.m_characterSpacingValueFactor * textStyle.m_height) : textStyle.m_characterSpacingValueFactor);
    legacyStyle.lineSpacing = ((DgnLineSpacingType::AtLeast != textStyle.m_lineSpacingType) ? (textStyle.m_lineSpacingValueFactor * textStyle.m_height) : textStyle.m_lineSpacingValueFactor);
    legacyStyle.overlineOffset = (textStyle.m_overlineOffsetFactor * textStyle.m_height);
    legacyStyle.underlineOffset = (textStyle.m_underlineOffsetFactor * textStyle.m_height);
    legacyStyle.width = (textStyle.m_widthFactor * textStyle.m_height);

    // Font objects to IDs
    // Tricky: You must call GetPropertyValue because during import, you cannot create font objects (the default storage for fonts).
    //  During import, a specialized sub-class of DgnTextStyle is used to handle this.
    //  Plus, even in the normal case, the getter is a convenience to map a number to a font object.
    if (SUCCESS != textStyle.GetPropertyValue(DgnTextStyleProperty::Font, legacyStyle.fontNo))
        { BeDataAssert(false); }
    
    if ((NULL != textStyle.m_shxBigFont) && (SUCCESS != textStyle.GetPropertyValue(DgnTextStyleProperty::ShxBigFont, legacyStyle.shxBigFont)))
        { BeDataAssert(false); }
    
    // Break out direction
    if (textStyle.m_isBackwards)    legacyStyle.textDirection |= TXTDIR_BACKWARDS;
    if (textStyle.m_isUpsideDown)   legacyStyle.textDirection |= TXTDIR_UPSIDEDOWN;
    if (textStyle.m_isVertical)     legacyStyle.textDirection |= TXTDIR_VERTICAL;

    return legacyStyle;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
void DgnTextStylePersistence::Legacy::FromLegacyStyle(DgnTextStyleR textStyle, LegacyTextStyleCR legacyStyle)
    {
    // Legacy/V8 styles supported a 0.0 height, which means place text with 1 master unit.
    // This would prevent us from storing distances as factors (vastly preferred), so don't allow it.
    // Since projects can only have one master unit, it's not horrible to just re-map.
    double legacyStyleHeight = legacyStyle.height;
    if (0.0 == legacyStyleHeight)
        legacyStyleHeight = 1.0;

    // Direct mappings
    textStyle.m_backgroundBorderColor = legacyStyle.backgroundStyle.color;
    textStyle.m_backgroundBorderLineStyle = legacyStyle.backgroundStyle.style;
    textStyle.m_backgroundBorderWeight = legacyStyle.backgroundStyle.weight;
    textStyle.m_backgroundFillColor = legacyStyle.backgroundFillColor;
    textStyle.m_color = legacyStyle.color;
    textStyle.m_customSlantAngle = legacyStyle.slant;
    textStyle.m_hasColor = legacyStyle.flags.color;
    textStyle.m_height = legacyStyleHeight;
    textStyle.m_isBold = legacyStyle.flags.bold;
    textStyle.m_isFullJustification = legacyStyle.flags.fullJustification;
    textStyle.m_isItalics = legacyStyle.flags.italics;
    textStyle.m_isOverlined = legacyStyle.flags.overline;
    textStyle.m_isSubScript = legacyStyle.flags.subscript;
    textStyle.m_isSuperScript = legacyStyle.flags.superscript;
    textStyle.m_isUnderlined = legacyStyle.flags.underline;
    textStyle.m_justification = (TextElementJustification)legacyStyle.just;
    textStyle.m_lineSpacingType = (DgnLineSpacingType)legacyStyle.flags.acadLineSpacingType;
    textStyle.m_maxCharactersPerLine = legacyStyle.lineLength;
    textStyle.m_overlineColor = legacyStyle.overlineStyle.color;
    textStyle.m_overlineLineStyle = legacyStyle.overlineStyle.style;
    textStyle.m_overlineWeight = legacyStyle.overlineStyle.weight;
    textStyle.m_runOffsetFactor = legacyStyle.lineOffset;
    textStyle.m_shouldUseBackground = legacyStyle.flags.background;
    textStyle.m_shouldUseOverlineStyle = legacyStyle.flags.overlineStyle;
    textStyle.m_shouldUseUnderlineStyle = legacyStyle.flags.underlineStyle;
    textStyle.m_widthFactor = legacyStyle.widthFactor;
    textStyle.m_underlineColor = legacyStyle.underlineStyle.color;
    textStyle.m_underlineLineStyle = legacyStyle.underlineStyle.style;
    textStyle.m_underlineWeight = legacyStyle.underlineStyle.weight;
    
    // Convert distances to height-based factors
    textStyle.m_backgroundBorderPaddingFactor = DPoint2d::FromScale(legacyStyle.backgroundBorder, (1.0 / legacyStyleHeight));
    textStyle.m_characterSpacingValueFactor = (!legacyStyle.flags.acadInterCharSpacing ? (legacyStyle.interCharSpacing / legacyStyleHeight) : legacyStyle.interCharSpacing);
    textStyle.m_lineSpacingValueFactor = ((DgnLineSpacingType::AtLeast != (DgnLineSpacingType)legacyStyle.flags.acadLineSpacingType) ? (legacyStyle.lineSpacing / legacyStyleHeight) : legacyStyle.lineSpacing);
    textStyle.m_overlineOffsetFactor = (legacyStyle.overlineOffset / legacyStyleHeight);
    textStyle.m_underlineOffsetFactor = (legacyStyle.underlineOffset / legacyStyleHeight);
    
    // Font objects from IDs
    // Tricky: You must call SetPropertyValue because during import, you cannot create font objects (the default storage for fonts).
    //  During import, a specialized sub-class of DgnTextStyle is used to handle this.
    //  Plus, even in the normal case, the setter is a convenience to map a number to a font object.
    if (SUCCESS != textStyle.SetPropertyValue(DgnTextStyleProperty::Font, legacyStyle.fontNo))
        {
        // One of our regression test files, 54_Master_All_DaimlerVTC.i.dgn, has a lot of dimension elements with corrupt font IDs... I'd love to assert, but this would make it very inconvenient.
        // BeDataAssert(false);
        
        // I also believe it is better to assign a font instead of leaving blank in a corrupt case. Users of text styles typically require a font and a non-zero height to function.
        // While we cannot pick anything "good", all callers don't want to have to deal with a NULL font property; they could check the legacy font number ahead of time if they can actually do something more intelligent.
        textStyle.SetPropertyValue(DgnTextStyleProperty::Font, &DgnFontManager::GetDefaultTrueTypeFont());
        }

    if (SUCCESS != textStyle.SetPropertyValue(DgnTextStyleProperty::ShxBigFont, legacyStyle.shxBigFont))
        {
        // One of our regression test files, 54_Master_All_DaimlerVTC.i.dgn, has a lot of dimension elements with corrupt font IDs... I'd love to assert, but this would make it very inconvenient.
        // BeDataAssert(false);
        
        // Unlike the base font property, this is most commonly NULL, so I feel it's acceptable to default this to NULL.
        textStyle.SetPropertyValue(DgnTextStyleProperty::ShxBigFont, (DgnFontCP)NULL);
        }

    // Break out character spacing
    if (legacyStyle.flags.fixedSpacing)
        textStyle.m_characterSpacingType = CharacterSpacingType::FixedWidth;
    else if (legacyStyle.flags.acadInterCharSpacing)
        textStyle.m_characterSpacingType = CharacterSpacingType::Factor;
    else
        textStyle.m_characterSpacingType = CharacterSpacingType::Absolute;
    
    // Break out direction
    textStyle.m_isBackwards = (TXTDIR_BACKWARDS == (TXTDIR_BACKWARDS & legacyStyle.textDirection));
    textStyle.m_isUpsideDown = (TXTDIR_UPSIDEDOWN == (TXTDIR_UPSIDEDOWN & legacyStyle.textDirection));
    textStyle.m_isVertical = (TXTDIR_VERTICAL == (TXTDIR_VERTICAL & legacyStyle.textDirection));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
LegacyTextStyleOverrideFlags DgnTextStylePersistence::Legacy::ToLegacyMask(DgnTextStylePropertyMaskCR mask)
    {
    LegacyTextStyleOverrideFlags v8Mask;
    memset (&v8Mask, 0, sizeof (v8Mask));

    v8Mask.fontNo = mask.IsPropertySet(DgnTextStyleProperty::Font);
    v8Mask.shxBigFont = mask.IsPropertySet(DgnTextStyleProperty::ShxBigFont);
    v8Mask.width = mask.IsPropertySet(DgnTextStyleProperty::WidthFactor);
    v8Mask.height = mask.IsPropertySet(DgnTextStyleProperty::Height);
    v8Mask.slant = mask.IsPropertySet(DgnTextStyleProperty::CustomSlantAngle);
    v8Mask.linespacing = mask.IsPropertySet(DgnTextStyleProperty::LineSpacingValueFactor);
    v8Mask.interCharSpacing = mask.IsPropertySet(DgnTextStyleProperty::CharacterSpacingValueFactor);
    v8Mask.underlineOffset = mask.IsPropertySet(DgnTextStyleProperty::UnderlineOffsetFactor);
    v8Mask.overlineOffset = mask.IsPropertySet(DgnTextStyleProperty::OverlineOffsetFactor);
    v8Mask.just = mask.IsPropertySet(DgnTextStyleProperty::Justification);
    v8Mask.lineLength = mask.IsPropertySet(DgnTextStyleProperty::MaxCharactersPerLine);
    v8Mask.direction = mask.IsPropertySet(DgnTextStyleProperty::IsBackwards);
    v8Mask.direction = mask.IsPropertySet(DgnTextStyleProperty::IsUpsideDown);
    v8Mask.direction = mask.IsPropertySet(DgnTextStyleProperty::IsVertical);
    v8Mask.underline = mask.IsPropertySet(DgnTextStyleProperty::IsUnderlined);
    v8Mask.overline = mask.IsPropertySet(DgnTextStyleProperty::IsOverlined);
    v8Mask.italics = mask.IsPropertySet(DgnTextStyleProperty::IsItalics);
    v8Mask.bold = mask.IsPropertySet(DgnTextStyleProperty::IsBold);
    v8Mask.superscript = mask.IsPropertySet(DgnTextStyleProperty::IsSuperScript);
    v8Mask.subscript = mask.IsPropertySet(DgnTextStyleProperty::IsSubScript);
    v8Mask.fixedSpacing = mask.IsPropertySet(DgnTextStyleProperty::CharacterSpacingType);
    v8Mask.background = mask.IsPropertySet(DgnTextStyleProperty::ShouldUseBackground);
    v8Mask.backgroundstyle = mask.IsPropertySet(DgnTextStyleProperty::BackgroundBorderLineStyle);
    v8Mask.backgroundweight = mask.IsPropertySet(DgnTextStyleProperty::BackgroundBorderWeight);
    v8Mask.backgroundcolor = mask.IsPropertySet(DgnTextStyleProperty::BackgroundBorderColor);
    v8Mask.backgroundfillcolor = mask.IsPropertySet(DgnTextStyleProperty::BackgroundFillColor);
    v8Mask.backgroundborder = mask.IsPropertySet(DgnTextStyleProperty::BackgroundBorderPaddingFactor);
    v8Mask.underlinestyle = mask.IsPropertySet(DgnTextStyleProperty::UnderlineLineStyle);
    v8Mask.underlineweight = mask.IsPropertySet(DgnTextStyleProperty::UnderlineWeight);
    v8Mask.underlinecolor = mask.IsPropertySet(DgnTextStyleProperty::UnderlineColor);
    v8Mask.overlinestyle = mask.IsPropertySet(DgnTextStyleProperty::OverlineLineStyle);
    v8Mask.overlineweight = mask.IsPropertySet(DgnTextStyleProperty::OverlineWeight);
    v8Mask.overlinecolor = mask.IsPropertySet(DgnTextStyleProperty::OverlineColor);
    v8Mask.lineOffset = mask.IsPropertySet(DgnTextStyleProperty::RunOffsetFactor);
    v8Mask.overlinestyleflag = mask.IsPropertySet(DgnTextStyleProperty::ShouldUseOverlineStyle);
    v8Mask.underlinestyleflag = mask.IsPropertySet(DgnTextStyleProperty::ShouldUseUnderlineStyle);
    v8Mask.color = mask.IsPropertySet(DgnTextStyleProperty::Color);
    v8Mask.widthFactor = mask.IsPropertySet(DgnTextStyleProperty::WidthFactor);
    v8Mask.colorFlag = mask.IsPropertySet(DgnTextStyleProperty::HasColor);
    v8Mask.fullJustification = mask.IsPropertySet(DgnTextStyleProperty::IsFullJustification);
    v8Mask.acadLineSpacingType = mask.IsPropertySet(DgnTextStyleProperty::LineSpacingType);
    v8Mask.backwards = mask.IsPropertySet(DgnTextStyleProperty::IsBackwards);
    v8Mask.upsidedown = mask.IsPropertySet(DgnTextStyleProperty::IsUpsideDown);
    v8Mask.acadInterCharSpacing = mask.IsPropertySet(DgnTextStyleProperty::CharacterSpacingType);

    return v8Mask;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
DgnTextStylePropertyMaskPtr DgnTextStylePersistence::Legacy::FromLegacyMask(LegacyTextStyleOverrideFlags const& v8Mask)
    {
    DgnTextStylePropertyMaskPtr mask = DgnTextStylePropertyMask::Create();

    mask->SetProperty(DgnTextStyleProperty::Font, v8Mask.fontNo);
    mask->SetProperty(DgnTextStyleProperty::ShxBigFont, v8Mask.shxBigFont);
    mask->SetProperty(DgnTextStyleProperty::WidthFactor, v8Mask.width);
    mask->SetProperty(DgnTextStyleProperty::Height, v8Mask.height);
    mask->SetProperty(DgnTextStyleProperty::CustomSlantAngle, v8Mask.slant);
    mask->SetProperty(DgnTextStyleProperty::LineSpacingValueFactor, v8Mask.linespacing);
    mask->SetProperty(DgnTextStyleProperty::CharacterSpacingValueFactor, v8Mask.interCharSpacing);
    mask->SetProperty(DgnTextStyleProperty::UnderlineOffsetFactor, v8Mask.underlineOffset);
    mask->SetProperty(DgnTextStyleProperty::OverlineOffsetFactor, v8Mask.overlineOffset);
    mask->SetProperty(DgnTextStyleProperty::Justification, v8Mask.just);
    mask->SetProperty(DgnTextStyleProperty::MaxCharactersPerLine, v8Mask.lineLength);
    mask->SetProperty(DgnTextStyleProperty::IsBackwards, v8Mask.direction);
    mask->SetProperty(DgnTextStyleProperty::IsUpsideDown, v8Mask.direction);
    mask->SetProperty(DgnTextStyleProperty::IsVertical, v8Mask.direction);
    mask->SetProperty(DgnTextStyleProperty::IsUnderlined, v8Mask.underline);
    mask->SetProperty(DgnTextStyleProperty::IsOverlined, v8Mask.overline);
    mask->SetProperty(DgnTextStyleProperty::IsItalics, v8Mask.italics);
    mask->SetProperty(DgnTextStyleProperty::IsBold, v8Mask.bold);
    mask->SetProperty(DgnTextStyleProperty::IsSuperScript, v8Mask.superscript);
    mask->SetProperty(DgnTextStyleProperty::IsSubScript, v8Mask.subscript);
    mask->SetProperty(DgnTextStyleProperty::CharacterSpacingType, v8Mask.fixedSpacing);
    mask->SetProperty(DgnTextStyleProperty::ShouldUseBackground, v8Mask.background);
    mask->SetProperty(DgnTextStyleProperty::BackgroundBorderLineStyle, v8Mask.backgroundstyle);
    mask->SetProperty(DgnTextStyleProperty::BackgroundBorderWeight, v8Mask.backgroundweight);
    mask->SetProperty(DgnTextStyleProperty::BackgroundBorderColor, v8Mask.backgroundcolor);
    mask->SetProperty(DgnTextStyleProperty::BackgroundFillColor, v8Mask.backgroundfillcolor);
    mask->SetProperty(DgnTextStyleProperty::BackgroundBorderPaddingFactor, v8Mask.backgroundborder);
    mask->SetProperty(DgnTextStyleProperty::UnderlineLineStyle, v8Mask.underlinestyle);
    mask->SetProperty(DgnTextStyleProperty::UnderlineWeight, v8Mask.underlineweight);
    mask->SetProperty(DgnTextStyleProperty::UnderlineColor, v8Mask.underlinecolor);
    mask->SetProperty(DgnTextStyleProperty::OverlineLineStyle, v8Mask.overlinestyle);
    mask->SetProperty(DgnTextStyleProperty::OverlineWeight, v8Mask.overlineweight);
    mask->SetProperty(DgnTextStyleProperty::OverlineColor, v8Mask.overlinecolor);
    mask->SetProperty(DgnTextStyleProperty::RunOffsetFactor, v8Mask.lineOffset);
    mask->SetProperty(DgnTextStyleProperty::ShouldUseOverlineStyle, v8Mask.overlinestyleflag);
    mask->SetProperty(DgnTextStyleProperty::ShouldUseUnderlineStyle, v8Mask.underlinestyleflag);
    mask->SetProperty(DgnTextStyleProperty::Color, v8Mask.color);
    mask->SetProperty(DgnTextStyleProperty::WidthFactor, v8Mask.widthFactor);
    mask->SetProperty(DgnTextStyleProperty::HasColor, v8Mask.colorFlag);
    mask->SetProperty(DgnTextStyleProperty::IsFullJustification, v8Mask.fullJustification);
    mask->SetProperty(DgnTextStyleProperty::LineSpacingType, v8Mask.acadLineSpacingType);
    mask->SetProperty(DgnTextStyleProperty::IsBackwards, v8Mask.backwards);
    mask->SetProperty(DgnTextStyleProperty::IsUpsideDown, v8Mask.upsidedown);
    mask->SetProperty(DgnTextStyleProperty::CharacterSpacingType, v8Mask.acadInterCharSpacing);

    return mask;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
TextParamWide DgnTextStylePersistence::Legacy::ToTextParamWide(DgnTextStyleCR textStyle, DPoint2dP fontSize, UInt32* maxCharactersPerLine, UInt32 fallbackFontNumber, UInt32 fallbackShxBigFontNumber)
    {
    TextParamWide params;

    PRECONDITION(textStyle.m_height > 0.0, params)

    // Direct mappings
    params.backgroundColor = textStyle.m_backgroundBorderColor;
    params.backgroundStyle = textStyle.m_backgroundBorderLineStyle;
    params.backgroundWeight = textStyle.m_backgroundBorderWeight;
    params.backgroundFillColor = textStyle.m_backgroundFillColor;
    params.flags.fixedWidthSpacing = (CharacterSpacingType::FixedWidth == textStyle.m_characterSpacingType);
    params.flags.interCharSpacing = (CharacterSpacingType::Factor == textStyle.m_characterSpacingType);
    params.color = textStyle.m_color;
    params.slant = textStyle.m_customSlantAngle;
    params.exFlags.color = textStyle.m_hasColor;
    params.exFlags.backwards = textStyle.m_isBackwards;
    params.exFlags.bold = textStyle.m_isBold;
    params.exFlags.fullJustification = textStyle.m_isFullJustification;
    params.flags.slant = textStyle.m_isItalics;
    params.exFlags.overline = textStyle.m_isOverlined;
    params.flags.subscript = textStyle.m_isSubScript;
    params.flags.superscript = textStyle.m_isSuperScript;
    params.flags.underline = textStyle.m_isUnderlined;
    params.exFlags.upsidedown = textStyle.m_isUpsideDown;
    params.flags.vertical = textStyle.m_isVertical;
    params.just = (int)textStyle.m_justification;
    params.exFlags.acadLineSpacingType = (UInt32)textStyle.m_lineSpacingType;
    params.overlineColor = textStyle.m_overlineColor;
    params.overlineStyle = textStyle.m_overlineLineStyle;
    params.overlineWeight = textStyle.m_overlineWeight;
    params.lineOffset = textStyle.m_runOffsetFactor;
    params.flags.bgColor = textStyle.m_shouldUseBackground;
    params.exFlags.overlineStyle = textStyle.m_shouldUseOverlineStyle;
    params.exFlags.underlineStyle = textStyle.m_shouldUseUnderlineStyle;
    params.underlineColor = textStyle.m_underlineColor;
    params.underlineStyle = textStyle.m_underlineLineStyle;
    params.underlineWeight = textStyle.m_underlineWeight;
    params.widthFactor = textStyle.m_widthFactor;

    // Convert height-based factors to distances
    params.backgroundBorder = DPoint2d::FromScale(textStyle.m_backgroundBorderPaddingFactor, textStyle.m_height);
    params.characterSpacing = ((CharacterSpacingType::Factor != textStyle.m_characterSpacingType) ? (textStyle.m_characterSpacingValueFactor * textStyle.m_height) : textStyle.m_characterSpacingValueFactor);
    params.lineSpacing = ((DgnLineSpacingType::AtLeast != textStyle.m_lineSpacingType) ? (textStyle.m_lineSpacingValueFactor * textStyle.m_height) : textStyle.m_lineSpacingValueFactor);
    params.overlineSpacing = (textStyle.m_overlineOffsetFactor * textStyle.m_height);
    params.underlineSpacing = (textStyle.m_underlineOffsetFactor * textStyle.m_height);

    // Font objects to IDs
    // Try to survive missing font IDs with the fallbacks.
    if (SUCCESS != textStyle.GetPropertyValue(DgnTextStyleProperty::Font, params.font))
        {
        if (-1 != fallbackFontNumber)
            params.font = fallbackFontNumber;
        else
            BeDataAssert(false);
        }

    if (SUCCESS != textStyle.GetPropertyValue(DgnTextStyleProperty::ShxBigFont, params.shxBigFont))
        {
        if (-1 != fallbackShxBigFontNumber)
            params.shxBigFont = fallbackShxBigFontNumber;
        else
            BeDataAssert(false);
        }
        
    // Optional parameters
    if (NULL != fontSize)
        {
        fontSize->x = (textStyle.m_widthFactor * textStyle.m_height);
        fontSize->y = textStyle.m_height;
        }

    if (NULL != maxCharactersPerLine)
        *maxCharactersPerLine = textStyle.m_maxCharactersPerLine;

    return params;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
void DgnTextStylePersistence::Legacy::FromTextParamWide(DgnTextStyleR textStyle, TextParamWideCR params, DPoint2dCR fontSize, UInt32 maxCharactersPerLine)
    {
    PRECONDITION(fontSize.y > 0.0, )
    
    // Direct mappings
    textStyle.m_backgroundBorderColor = params.backgroundColor;
    textStyle.m_backgroundBorderLineStyle = params.backgroundStyle;
    textStyle.m_backgroundBorderWeight = params.backgroundWeight;
    textStyle.m_backgroundFillColor = params.backgroundFillColor;
    textStyle.m_color = params.color;
    textStyle.m_customSlantAngle = params.slant;
    textStyle.m_hasColor = params.exFlags.color;
    textStyle.m_height = fontSize.y;
    textStyle.m_isBackwards = params.exFlags.backwards;
    textStyle.m_isBold = params.exFlags.bold;
    textStyle.m_isFullJustification = params.exFlags.fullJustification;
    textStyle.m_isItalics = params.flags.slant;
    textStyle.m_isOverlined = params.exFlags.overline;
    textStyle.m_isSubScript = params.flags.subscript;
    textStyle.m_isSuperScript = params.flags.superscript;
    textStyle.m_isUnderlined = params.flags.underline;
    textStyle.m_isUpsideDown = params.exFlags.upsidedown;
    textStyle.m_isVertical = params.flags.vertical;
    textStyle.m_justification = (TextElementJustification)params.just;
    textStyle.m_lineSpacingType = (DgnLineSpacingType)params.exFlags.acadLineSpacingType;
    textStyle.m_maxCharactersPerLine = maxCharactersPerLine;
    textStyle.m_overlineColor = params.overlineColor;
    textStyle.m_overlineLineStyle = params.overlineStyle;
    textStyle.m_overlineWeight = params.overlineWeight;
    textStyle.m_runOffsetFactor = params.lineOffset;
    textStyle.m_shouldUseBackground = params.flags.bgColor;
    textStyle.m_shouldUseOverlineStyle = params.exFlags.overlineStyle;
    textStyle.m_shouldUseUnderlineStyle = params.exFlags.underlineStyle;
    textStyle.m_underlineColor = params.underlineColor;
    textStyle.m_underlineLineStyle = params.underlineStyle;
    textStyle.m_underlineWeight = params.underlineWeight;

    // Convert distances to height-based factors
    textStyle.m_backgroundBorderPaddingFactor = DPoint2d::FromScale(params.backgroundBorder, (1.0 / fontSize.y));
    textStyle.m_characterSpacingValueFactor = (!params.flags.interCharSpacing ? (params.characterSpacing / fontSize.y) : params.characterSpacing);
    textStyle.m_lineSpacingValueFactor = ((DgnLineSpacingType::AtLeast != (DgnLineSpacingType)params.exFlags.acadLineSpacingType) ? (params.lineSpacing / fontSize.y) : params.lineSpacing);
    textStyle.m_overlineOffsetFactor = (params.overlineSpacing / fontSize.y);
    textStyle.m_underlineOffsetFactor = (params.underlineSpacing / fontSize.y);
    textStyle.m_widthFactor = (fontSize.y / fontSize.x);
    
    // Font objects from IDs
    if (UNEXPECTED_CONDITION(SUCCESS != textStyle.SetPropertyValue(DgnTextStyleProperty::Font, params.font)))
        { BeDataAssert(false); }

    if (UNEXPECTED_CONDITION(SUCCESS != textStyle.SetPropertyValue(DgnTextStyleProperty::ShxBigFont, params.shxBigFont)))
        { BeDataAssert(false); }
    
    // Break out character spacing
    if (params.flags.fixedWidthSpacing)
        textStyle.m_characterSpacingType = CharacterSpacingType::FixedWidth;
    else if (params.flags.interCharSpacing)
        textStyle.m_characterSpacingType = CharacterSpacingType::Factor;
    else
        textStyle.m_characterSpacingType = CharacterSpacingType::Absolute;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
TextParamWide DgnTextStylePersistence::Legacy::ToTextParamWide(LegacyTextStyleCR textStyle, DgnProjectCR project, DPoint2dP fontSize, UInt32* maxCharactersPerLine)
    {
    /*
    Missing source mappings:
        flags.acadShapeFile
        flags.fractions
        nodeJust
        parentId
    
    Missing target mappings:
        param.flags.textStyle
        param.flags.unused3
        params.annotationScale
        params.assocId
        params.codePage_deprecated
        params.exFlags.acadFittedText
        params.exFlags.annotationScale
        params.exFlags.bitMaskContainsTabCRLF
        params.exFlags.crCount
        params.exFlags.isField
        params.exFlags.renderPercentsAsMText
        params.exFlags.stackedFractionAlign
        params.exFlags.stackedFractionSection
        params.exFlags.stackedFractionType
        params.exFlags.styleOverrides
        params.exFlags.wordWrapTextNode
        params.flags.codePage_deprecated
        params.flags.offset
        params.flags.shxBigFont
        params.flags.unused
        params.flags.unused2
        params.lineStyle_deprecated
        params.nodeNumber
        params.renderingFlags.alignEdge
        params.renderingFlags.documentType
        params.renderingFlags.lineAlignment
        params.renderingFlags.unused
        params.textnodeWordWrapLength
        params.textStyleId
        params.viewIndependent
    */

    TextParamWide params; // has constructor

    // Legacy/V8 styles supported a 0.0 height, which means place text with 1 master unit.
    // This would prevent us from storing distances as factors (vastly preferred), so don't allow it.
    double legacyStyleHeight = textStyle.height;
    if (0.0 == legacyStyleHeight)
        legacyStyleHeight = 1.0;

    // Direct mappings
    params.backgroundBorder = textStyle.backgroundBorder;
    params.backgroundColor = textStyle.backgroundStyle.color;
    params.backgroundFillColor = textStyle.backgroundFillColor;
    params.backgroundStyle = textStyle.backgroundStyle.style;
    params.backgroundWeight = textStyle.backgroundStyle.weight;
    params.characterSpacing = textStyle.interCharSpacing;
    params.color = textStyle.color;
    params.exFlags.acadInterCharSpacing = textStyle.flags.acadInterCharSpacing;
    params.exFlags.acadLineSpacingType = textStyle.flags.acadLineSpacingType;
    params.exFlags.backwards = (TXTDIR_BACKWARDS == (TXTDIR_BACKWARDS & textStyle.textDirection));
    params.exFlags.bold = textStyle.flags.bold;
    params.exFlags.color = textStyle.flags.color;
    params.exFlags.fullJustification = textStyle.flags.fullJustification;
    params.exFlags.overline = textStyle.flags.overline;
    params.exFlags.overlineStyle = textStyle.flags.overlineStyle;
    params.exFlags.underlineStyle = textStyle.flags.underlineStyle;
    params.exFlags.upsidedown = (TXTDIR_UPSIDEDOWN == (TXTDIR_UPSIDEDOWN & textStyle.textDirection));
    params.flags.bgColor = textStyle.flags.background;
    params.flags.fixedWidthSpacing = textStyle.flags.fixedSpacing;
    params.flags.interCharSpacing = textStyle.flags.acadInterCharSpacing;
    params.flags.rightToLeft_deprecated = (TXTDIR_RIGHTLEFT == (TXTDIR_RIGHTLEFT & textStyle.textDirection));
    params.flags.slant = textStyle.flags.italics;
    params.flags.subscript = textStyle.flags.subscript;
    params.flags.superscript = textStyle.flags.superscript;
    params.flags.underline = textStyle.flags.underline;
    params.flags.vertical = (TXTDIR_VERTICAL == (TXTDIR_VERTICAL & textStyle.textDirection));
    params.font = textStyle.fontNo;
    params.just = textStyle.just;
    params.lineOffset = textStyle.lineOffset;
    params.lineSpacing = textStyle.lineSpacing;
    params.overlineColor = textStyle.overlineStyle.color;
    params.overlineSpacing = textStyle.overlineOffset;
    params.overlineStyle = textStyle.overlineStyle.style;
    params.overlineWeight = textStyle.overlineStyle.weight;
    params.overridesFromStyle = textStyle.overrideFlags;
    params.shxBigFont = textStyle.shxBigFont;
    params.slant = textStyle.slant;
    params.underlineColor = textStyle.underlineStyle.color;
    params.underlineSpacing = textStyle.underlineOffset;
    params.underlineStyle = textStyle.underlineStyle.style;
    params.underlineWeight = textStyle.underlineStyle.weight;
    params.widthFactor = textStyle.widthFactor;

    // Optional parameters
    if (NULL != fontSize)
        {
        fontSize->x = textStyle.width; // Trusting width over widthFactor?
        fontSize->y = textStyle.height;
        }

    if (NULL != maxCharactersPerLine)
        *maxCharactersPerLine = textStyle.lineLength;

    return params;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
LegacyTextStyle DgnTextStylePersistence::Legacy::FromTextParamWide(TextParamWideCR params, DPoint2dCR fontSize, UInt32 maxCharactersPerLine)
    {
    // See ToTextParamWide(LegacyTextStyleCR... for missing mappings.
    
    LegacyTextStyle textStyle;
    memset(&textStyle, 0, sizeof(textStyle));

    // Direct mappings
    textStyle.backgroundBorder = params.backgroundBorder;
    textStyle.backgroundFillColor = params.backgroundFillColor;
    textStyle.backgroundStyle.color = params.backgroundColor;
    textStyle.backgroundStyle.style = params.backgroundStyle;
    textStyle.backgroundStyle.weight = params.backgroundWeight;
    textStyle.color = params.color;
    textStyle.flags.acadInterCharSpacing = params.exFlags.acadInterCharSpacing;
    textStyle.flags.acadInterCharSpacing = params.flags.interCharSpacing;
    textStyle.flags.acadLineSpacingType = params.exFlags.acadLineSpacingType;
    textStyle.flags.background = params.flags.bgColor;
    textStyle.flags.bold = params.exFlags.bold;
    textStyle.flags.color = params.exFlags.color;
    textStyle.flags.fixedSpacing = params.flags.fixedWidthSpacing;
    textStyle.flags.fullJustification = params.exFlags.fullJustification;
    textStyle.flags.italics = params.flags.slant;
    textStyle.flags.overline = params.exFlags.overline;
    textStyle.flags.overlineStyle = params.exFlags.overlineStyle;
    textStyle.flags.subscript = params.flags.subscript;
    textStyle.flags.superscript = params.flags.superscript;
    textStyle.flags.underline = params.flags.underline;
    textStyle.flags.underlineStyle = params.exFlags.underlineStyle;
    textStyle.fontNo = params.font;
    textStyle.height = fontSize.y;
    textStyle.interCharSpacing = params.characterSpacing;
    textStyle.just = (UInt16)params.just;
    textStyle.lineLength = (UInt16)maxCharactersPerLine;
    textStyle.lineOffset = params.lineOffset;
    textStyle.lineSpacing = params.lineSpacing;
    textStyle.overlineOffset = params.overlineSpacing;
    textStyle.overlineStyle.color = params.overlineColor;
    textStyle.overlineStyle.style = params.overlineStyle;
    textStyle.overlineStyle.weight = params.overlineWeight;
    textStyle.overrideFlags = params.overridesFromStyle;
    textStyle.shxBigFont = params.shxBigFont;
    textStyle.slant = params.slant;
    textStyle.textDirection = textStyle.textDirection | (params.exFlags.backwards ? TXTDIR_BACKWARDS : 0);
    textStyle.textDirection = textStyle.textDirection | (params.exFlags.upsidedown ? TXTDIR_UPSIDEDOWN : 0);
    textStyle.textDirection = textStyle.textDirection | (params.flags.rightToLeft_deprecated ? TXTDIR_RIGHTLEFT : 0);
    textStyle.textDirection = textStyle.textDirection | (params.flags.vertical ? TXTDIR_VERTICAL : 0);
    textStyle.underlineOffset = params.underlineSpacing;
    textStyle.underlineStyle.color = params.underlineColor;
    textStyle.underlineStyle.style = params.underlineStyle;
    textStyle.underlineStyle.weight = params.underlineWeight;
    textStyle.width = fontSize.x;
    textStyle.widthFactor = params.widthFactor;

    return textStyle;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
NoFontObjectDgnTextStyle::NoFontObjectDgnTextStyle(DgnProjectR project) :
    T_Super(project),
    m_fontNumber(0),
    m_shxBigFontNumber(0)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus NoFontObjectDgnTextStyle::_GetPropertyValue(DgnTextStyleProperty property, UInt32& value) const
    {
    switch (property)
        {
        case DgnTextStyleProperty::Font:        value = m_fontNumber;       return SUCCESS;
        case DgnTextStyleProperty::ShxBigFont:  value = m_shxBigFontNumber; return SUCCESS;
        }

    return T_Super::_GetPropertyValue(property, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus NoFontObjectDgnTextStyle::_SetPropertyValue(DgnTextStyleProperty property, UInt32 value)
    {
    switch (property)
        {
        case DgnTextStyleProperty::Font:        m_fontNumber = value;       return SUCCESS;
        case DgnTextStyleProperty::ShxBigFont:  m_shxBigFontNumber = value; return SUCCESS;
        }

    return T_Super::_SetPropertyValue(property, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus NoFontObjectDgnTextStyle::_GetPropertyValue(DgnTextStyleProperty property, DgnFontCP& value) const
    {
    switch (property)
        {
        case DgnTextStyleProperty::Font:
        case DgnTextStyleProperty::ShxBigFont:
            BeAssert(false);
            return ERROR;
        }

    return T_Super::_GetPropertyValue(property, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus NoFontObjectDgnTextStyle::_SetPropertyValue(DgnTextStyleProperty property, DgnFontCP value)
    {
    switch (property)
        {
        case DgnTextStyleProperty::Font:
        case DgnTextStyleProperty::ShxBigFont:
            BeAssert(false);
            return ERROR;
        }

    return T_Super::_SetPropertyValue(property, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
DgnTextStylePtr NoFontObjectDgnTextStyle::_Clone() const
    {
    DgnTextStylePtr copy = T_Super::_Clone();

    copy->SetPropertyValue(DgnTextStyleProperty::Font, m_fontNumber);
    copy->SetPropertyValue(DgnTextStyleProperty::ShxBigFont, m_shxBigFontNumber);

    return copy;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
DgnTextStylePtr DgnTextStyles::QueryById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), NULL);

    DgnStyles::Style foundStyle = m_project.Styles().QueryStyleById(DgnStyleType::Text, id);
    if (!foundStyle.GetId().IsValid())
        return NULL;

    DgnTextStylePtr textStyle = DgnTextStyle::Create(m_project);
    textStyle->SetDescription(foundStyle.GetDescription());
    textStyle->SetId(id);
    textStyle->SetName(foundStyle.GetName());

    if (SUCCESS != DgnTextStylePersistence::FromJson(*textStyle, (Utf8CP)foundStyle.GetData()))
        return NULL;

    return textStyle;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
bool DgnTextStyles::ExistsById(DgnStyleId id) const
    {
    PRECONDITION(id.IsValid(), false);

    DgnStyles::Style foundStyle = m_project.Styles().QueryStyleById(DgnStyleType::Text, id);

    return foundStyle.GetId().IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
DgnTextStylePtr DgnTextStyles::QueryByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), NULL);

    DgnStyleId foundStyleId = m_project.Styles().QueryStyleId(DgnStyleType::Text, name);
    if (!foundStyleId.IsValid())
        return NULL;

    return QueryById(foundStyleId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
bool DgnTextStyles::ExistsByName(Utf8CP name) const
    {
    PRECONDITION(!Utf8String::IsNullOrEmpty(name), false);

    DgnStyleId foundStyleId = m_project.Styles().QueryStyleId(DgnStyleType::Text, name);

    return foundStyleId.IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyles::InsertWithId(DgnTextStyleCR textStyle)
    {
    PRECONDITION(!textStyle.GetName().empty(), ERROR);
    PRECONDITION(textStyle.GetId().IsValid(), ERROR);
    PRECONDITION(&textStyle.GetProject() == &m_project, ERROR);

    Utf8String styleData = DgnTextStylePersistence::ToJson(textStyle);
    if (styleData.empty())
        return ERROR;

    DgnStyles::Style styleRow(textStyle.GetId(), DgnStyleType::Text, textStyle.GetName().c_str(), textStyle.GetDescription().c_str(), &styleData[0], styleData.SizeInBytes());
    if (UNEXPECTED_CONDITION(BE_SQLITE_DONE != m_project.Styles().InsertStyleWithId(styleRow)))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
DgnTextStylePtr DgnTextStyles::Insert(DgnTextStyleCR textStyle)
    {
    PRECONDITION(!textStyle.GetName().empty(), NULL);
    PRECONDITION(&textStyle.GetProject() == &m_project, NULL);

    Utf8String styleData = DgnTextStylePersistence::ToJson(textStyle);
    if (styleData.empty())
        return NULL;

    DgnStyles::Style styleRow(DgnStyleId(), DgnStyleType::Text, textStyle.GetName().c_str(), textStyle.GetDescription().c_str(), &styleData[0], styleData.SizeInBytes());
    if (UNEXPECTED_CONDITION(BE_SQLITE_DONE != m_project.Styles().InsertStyle(styleRow)))
        return NULL;

    return QueryById(styleRow.GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyles::Update(DgnTextStyleCR textStyle)
    {
    PRECONDITION(textStyle.GetId().IsValid(), ERROR);
    PRECONDITION(!textStyle.GetName().empty(), ERROR);
    PRECONDITION(&textStyle.GetProject() == &m_project, ERROR);

    Utf8String styleData = DgnTextStylePersistence::ToJson(textStyle);
    if (styleData.empty())
        return ERROR;

    DgnStyles::Style styleRow(textStyle.GetId(), DgnStyleType::Text, textStyle.GetName().c_str(), textStyle.GetDescription().c_str(), &styleData[0], styleData.SizeInBytes());
    if (UNEXPECTED_CONDITION(BE_SQLITE_DONE != m_project.Styles().UpdateStyle(styleRow)))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
BentleyStatus DgnTextStyles::Delete(DgnStyleId id)
    {
    PRECONDITION(id.IsValid(), ERROR);

    if (UNEXPECTED_CONDITION(BE_SQLITE_DONE != m_project.Styles().DeleteStyle(DgnStyleType::Text, id)))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
DgnStyles::Iterator DgnTextStyles::MakeIterator(DgnStyleSort sortOrder) const
    {
    Utf8String queryModifierClauses;
    queryModifierClauses.Sprintf("WHERE Type=%d", DgnStyleType::Text);

    switch (sortOrder)
        {
        case DgnStyleSort::None:       break;
        case DgnStyleSort::NameAsc:    queryModifierClauses += " ORDER BY Name ASC";   break;
        case DgnStyleSort::NameDsc:    queryModifierClauses += " ORDER BY Name DESC";  break;

        default:
            BeAssert(false); // Unknown/unexpected DgnStyleSort
            break;
        }

    DgnStyles::Iterator it(m_project);
    it.Params().SetWhere(queryModifierClauses.c_str());
    return it;
    }
