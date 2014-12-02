/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnFontManager/DgnFontManager.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2009
//---------------------------------------------------------------------------------------
DgnGlyphLayoutContext::DgnGlyphLayoutContext(DgnFontCR font, DgnFontCP shxBixFont)
    {
    m_font = &font;
    m_shxBigFont = shxBixFont;

    m_characterSpacingType = CharacterSpacingType::Absolute;
    m_characterSpacingValue = 0.0;
    m_displayOffset.Zero();
    m_displaySize.Zero();
    m_isBold = false;
    m_shouldUseItalicTypeface = false;
    m_runLayoutFlags = GLYPH_RUN_LAYOUT_FLAG_None;
    m_glyphLayoutListener = NULL;
    m_shouldIgnoreDisplayShifts = false;
    m_shouldIgnorePercentEscapes = false;
    m_shouldIgnoreLsb = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2009
//---------------------------------------------------------------------------------------
CharacterSpacingType DgnGlyphLayoutContext::GetCharacterSpacingType() const { return m_characterSpacingType; }
void DgnGlyphLayoutContext::SetCharacterSpacingType(CharacterSpacingType value) { m_characterSpacingType = value; }
double DgnGlyphLayoutContext::GetCharacterSpacingValue() const { return m_characterSpacingValue; }
void DgnGlyphLayoutContext::SetCharacterSpacingValue(double value) { m_characterSpacingValue = value; }
DPoint3d DgnGlyphLayoutContext::GetDisplayOffset() const { return m_displayOffset; }
void DgnGlyphLayoutContext::SetDisplayOffset(DPoint3d value) { m_displayOffset = value; }
DPoint2d DgnGlyphLayoutContext::GetDisplaySize() const { return m_displaySize; }
void DgnGlyphLayoutContext::SetDisplaySize(DPoint2d value) { m_displaySize = value; }
DgnFontCR DgnGlyphLayoutContext::GetFont() const { return *m_font; }
DgnFontCP DgnGlyphLayoutContext::GetShxBigFont() const { return m_shxBigFont; }
bool DgnGlyphLayoutContext::IsBold() const { return m_isBold; }
void DgnGlyphLayoutContext::SetIsBold(bool value) { m_isBold = value; }
bool DgnGlyphLayoutContext::ShouldUseItalicTypeface() const { return m_shouldUseItalicTypeface; }
void DgnGlyphLayoutContext::SetShouldUseItalicTypeface(bool value) { m_shouldUseItalicTypeface = value; }
DgnGlyphRunLayoutFlags DgnGlyphLayoutContext::GetRunLayoutFlags() const { return m_runLayoutFlags; }
bool DgnGlyphLayoutContext::IsVertical() const { return (GLYPH_RUN_LAYOUT_FLAG_Backwards == (GLYPH_RUN_LAYOUT_FLAG_Backwards & m_runLayoutFlags)); }
bool DgnGlyphLayoutContext::IsBackwards() const { return (GLYPH_RUN_LAYOUT_FLAG_UpsideDown == (GLYPH_RUN_LAYOUT_FLAG_UpsideDown & m_runLayoutFlags)); }
bool DgnGlyphLayoutContext::IsUpsideDown() const { return (GLYPH_RUN_LAYOUT_FLAG_Vertical == (GLYPH_RUN_LAYOUT_FLAG_Vertical & m_runLayoutFlags)); }
void DgnGlyphLayoutContext::SetRunLayoutFlags(DgnGlyphRunLayoutFlags value) { m_runLayoutFlags = value; }
DgnGlyphLayoutContext::T_FontCharsCR DgnGlyphLayoutContext::GetFontChars() const { return m_fontChars; }
DgnGlyphLayoutContext::T_EdfMaskCR DgnGlyphLayoutContext::GetEDFMask() const { return m_edfMask; }
IDgnGlyphLayoutListener* DgnGlyphLayoutContext::GetIDgnGlyphLayoutListenerP() const { return m_glyphLayoutListener; }
void DgnGlyphLayoutContext::SetIDgnGlyphLayoutListenerP(IDgnGlyphLayoutListener* value) { m_glyphLayoutListener = value; }
bool DgnGlyphLayoutContext::ShouldIgnoreDisplayShifts() const { return m_shouldIgnoreDisplayShifts; }
void DgnGlyphLayoutContext::SetShouldIgnoreDisplayShifts(bool value) { m_shouldIgnoreDisplayShifts = value; }
bool DgnGlyphLayoutContext::ShouldIgnorePercentEscapes() const { return m_shouldIgnorePercentEscapes; }
void DgnGlyphLayoutContext::SetShouldIgnorePercentEscapes(bool value) { m_shouldIgnorePercentEscapes = value; }
bool DgnGlyphLayoutContext::ShouldIgnoreLsb() const { return m_shouldIgnoreLsb; }
void DgnGlyphLayoutContext::SetShouldIgnoreLsb(bool value) { m_shouldIgnoreLsb = value; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2009
//---------------------------------------------------------------------------------------
void DgnGlyphLayoutContext::SetFontChars(T_FontChars::value_type const* chars, T_EdfMask::value_type const* edfs, size_t count)
    {
    m_fontChars.resize(count);
    m_edfMask.resize(count);
    
    if (EXPECTED_CONDITION(NULL != chars))
        memcpy(&m_fontChars[0], chars, (sizeof (T_FontChars::value_type) * count));
    else
        std::fill(m_fontChars.begin(), m_fontChars.end(), 0);

    if (NULL != edfs)
        memcpy(&m_edfMask[0], edfs, (sizeof (T_EdfMask::value_type) * count));
    else
        std::fill(m_edfMask.begin(), m_edfMask.end(), false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2009
//---------------------------------------------------------------------------------------
void DgnGlyphLayoutContext::SetFontChars(WCharCP chars, T_EdfMask::value_type const* edfs, size_t count)
    {
    m_fontChars.resize(count);
    m_edfMask.resize(count);

    DgnFontCR fontForCodePage = (m_shxBigFont ? *m_shxBigFont : *m_font);

    if (EXPECTED_CONDITION(NULL != chars))
        std::transform(chars, chars + count, m_fontChars.begin(), [&](WChar const& uniChar){return fontForCodePage.RemapUnicodeCharToFontChar(uniChar); });
    else
        std::fill(m_fontChars.begin(), m_fontChars.end(), 0);

    if (NULL != edfs)
        memcpy(&m_edfMask[0], edfs, (sizeof (T_EdfMask::value_type) * count));
    else
        std::fill(m_edfMask.begin(), m_edfMask.end(), false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2009
//---------------------------------------------------------------------------------------
void DgnGlyphLayoutContext::SetFontChars(Utf8CP chars, size_t count)
    {
    // Best way to go character-by-character in Utf8 is just to convert...
    WString wstr;
    EXPECTED_CONDITION(SUCCESS == BeStringUtilities::Utf8ToWChar(wstr, chars, count));
    SetFontChars(wstr.c_str(), NULL, wstr.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2009
//---------------------------------------------------------------------------------------
void DgnGlyphLayoutContext::SetPropertiesFromRunPropertiesBase(RunPropertiesBaseCR props)
    {
    m_characterSpacingType = props.GetCharacterSpacingType();
    m_characterSpacingValue = props.GetCharacterSpacingValue();
    m_displayOffset = props.GetDisplayOffset();
    m_displaySize = props.GetDisplaySize();
    m_font = &props.GetFont();
    m_shxBigFont = props.GetShxBigFontCP();
    m_isBold = props.IsBold();
    m_shouldIgnoreLsb = props.ShouldIgnoreLSB();
    m_shouldUseItalicTypeface = props.ShouldUseItalicTypeface();
    }

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2009
//---------------------------------------------------------------------------------------
DRange2dR                               DgnGlyphLayoutResult::GetBlackBoxRangeR                 () { return m_blackBoxRange; }
DRange2dR                               DgnGlyphLayoutResult::GetCellBoxRangeR                  () { return m_cellBoxRange; }
DgnGlyphLayoutResult::T_GlyphCodesR     DgnGlyphLayoutResult::GetGlyphCodesR                    () { return m_glyphCodes; }
DgnGlyphLayoutResult::T_GlyphOriginsR   DgnGlyphLayoutResult::GetGlyphOriginsR                  () { return m_glyphOrigins; }
DRange2dR                               DgnGlyphLayoutResult::GetJustificationCellBoxRangeR     () { return m_justificationCellBoxRange; }
DRange2dR                               DgnGlyphLayoutResult::GetJustificationBlackBoxRangeR    () { return m_justificationBlackBoxRange; }
T_DoubleVectorR                         DgnGlyphLayoutResult::GetLeadingCaretOffsetsR           () { return m_leadingCaretOffsets; }
double&                                 DgnGlyphLayoutResult::GetMaxHorizontalCellIncrementR    () { return m_maxHorizontalCellIncrement; }
T_DoubleVectorR                         DgnGlyphLayoutResult::GetTrailingCaretOffsetsR          () { return m_trailingCaretOffsets; }
double&                                 DgnGlyphLayoutResult::GetTrailingInterCharacterSpacingR () { return m_trailingInterCharacterSpacing; }
bool&                                   DgnGlyphLayoutResult::IsLastGlyphBlankR                 () { return m_isLastGlyphBlank; }
double&                                 DgnGlyphLayoutResult::LeftSideBearingToIgnoreR          () { return m_leftSideBearingToIgnore; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2009
//---------------------------------------------------------------------------------------
DgnGlyphLayoutResult::DgnGlyphLayoutResult () :
    m_trailingInterCharacterSpacing (0.0),
    m_maxHorizontalCellIncrement    (0.0),
    m_isLastGlyphBlank              (false),
    m_leftSideBearingToIgnore       (0.0)
    {
    m_cellBoxRange.Init ();
    m_blackBoxRange.Init ();
    m_justificationCellBoxRange.Init();
    m_justificationBlackBoxRange.Init ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2009
//---------------------------------------------------------------------------------------
DRange2d DgnGlyphLayoutResult::ComputeElementRange (DgnGlyphLayoutContextCR layoutContext) const
    {
    DRange2d elementRange = m_cellBoxRange;

    // Didn't use to have an IsVertical check, even though it kinda makes sense; this is for DWG at any rate, so not breaking what's already broke...
    if (layoutContext.ShouldIgnoreLsb ())
        elementRange.low.x += m_leftSideBearingToIgnore;

    // I'm generally speechless about this old legacy behavior...
    if (!layoutContext.IsVertical () && !m_isLastGlyphBlank && !layoutContext.GetEDFMask ().back ())
        elementRange.high.x = m_blackBoxRange.high.x;

    return elementRange;
    }

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
BentleyStatus   DgnGlyph::FillGpa           (GPArrayR gpa) const    { return _FillGpa (gpa); }
double          DgnGlyph::GetBlackBoxBottom () const                { return m_blackBoxStart.y; }
double          DgnGlyph::GetBlackBoxHeight () const                { return (m_blackBoxEnd.y - m_blackBoxStart.y); }
double          DgnGlyph::GetBlackBoxLeft   () const                { return m_blackBoxStart.x; }
double          DgnGlyph::GetBlackBoxRight  () const                { return m_blackBoxEnd.x; }
double          DgnGlyph::GetBlackBoxWidth  () const                { return (m_blackBoxEnd.x - m_blackBoxStart.x); }
double          DgnGlyph::GetCellBoxHeight  () const                { return (m_cellBoxEnd.y - m_cellBoxStart.y); }
double          DgnGlyph::GetCellBoxRight   () const                { return m_cellBoxEnd.x; }
double          DgnGlyph::GetCellBoxWidth   () const                { return (m_cellBoxEnd.x - m_cellBoxStart.x); }
FontChar        DgnGlyph::GetCharCode       () const                { return m_charCode; }
DgnFontType     DgnGlyph::GetType           () const                { return _GetType (); }
bool            DgnGlyph::IsBlank           () const                { return _IsBlank (); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnGlyph::DgnGlyph (FontChar charCode) :
    m_charCode (charCode)
    {
    m_blackBoxStart.Zero ();
    m_blackBoxEnd.Zero ();
    m_cellBoxStart.Zero ();
    m_cellBoxEnd.Zero ();
    }

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnFontConfigurationData::DgnFontConfigurationData () :
    m_codePage                  (LangCodePage::Unknown),
    m_degreeCharCode            (0),
    m_diameterCharCode          (0),
    m_plusMinusCharCode         (0),
    m_shouldCreateShxUnifont    (false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static bool isCharacterSpecial (FontChar fontChar, DgnFontCR font)
    {
    return ((font.GetDegreeCharCode () == fontChar) || (font.GetDiameterCharCode () == fontChar) || (font.GetPlusMinusCharCode () == fontChar));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
FontChar                    DgnFont::CharIter::CurrCharCode ()                                      { return m_charCode; }
DgnFont::CharIter::CharType DgnFont::CharIter::CurrCharType ()                                      { return m_charType; }
bool                        DgnFont::CharIter::IsValid      ()                                      { return (m_currChar <= m_lastCharacter); }
void                        DgnFont::CharIter::SetChar      (FontChar charCode, CharType charType)  { m_charCode = charCode; m_charType = charType; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFont::CharIter::CharIter (DgnFontCR font, FontCharCP chars, size_t nChars) :
    m_startOfString                 (chars),
    m_lastCharacter                 (chars + nChars - 1),
    m_currChar                      (chars - 1),
    m_font                          (font),
    m_shouldIgnorePercentEscapes    (false)
    {
    ToNext ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFont::CharIter::CharIter (DgnFontCR font, FontCharCP chars, size_t nChars, bool shouldIgnorePercentEscapes) :
    m_startOfString                 (chars),
    m_lastCharacter                 (chars + nChars - 1),
    m_currChar                      (chars - 1),
    m_font                          (font),
    m_shouldIgnorePercentEscapes    (shouldIgnorePercentEscapes)
    {
    ToNext ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void DgnFont::CharIter::ToNext ()
    {
    ++m_currChar;

    if (!IsValid ())
        return;
    
    if (m_shouldIgnorePercentEscapes || ('%' != *m_currChar) || (m_currChar == m_lastCharacter) || ('%' != *(m_currChar + 1)))
        {
        SetChar (*m_currChar, (isCharacterSpecial (*m_currChar, m_font) ? CHARTYPE_Special : CHARTYPE_Normal));
        return;
        }

    m_currChar += 2;
    
    // %% as last two characters? Just ignore them...
    if (!IsValid ())
        return;

    switch (*m_currChar)
        {
        case 'd':
        case 'D':
            SetChar (m_font.GetDegreeCharCode (), CHARTYPE_Special);
            return;

        case 'p':
        case 'P':
             SetChar (m_font.GetPlusMinusCharCode (), CHARTYPE_Special);
             return;

        case 'c':
        case 'C':
            SetChar (m_font.GetDiameterCharCode (), CHARTYPE_Special);
            return;
        }

    if ((*m_currChar < '0') || (*m_currChar > '9'))
        {
        SetChar (*m_currChar, CHARTYPE_Normal);
        return;
        }

    UInt16 val = (*m_currChar - '0');
    
    for (int count = 0; IsValid () && (count < 2); ++count)
        {
        if ((m_currChar == m_lastCharacter) || (*(m_currChar + 1) < '0') || (*(m_currChar + 1) > '9'))
            break;
        
        ++m_currChar;
        
        val = (val * 10) + (*m_currChar - '0');
        }

    SetChar (val, isCharacterSpecial (val, m_font) ? CHARTYPE_Special : CHARTYPE_Compose);
    }

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

const double DgnFont::VERTICAL_TEXT_WIDTH_FACTOR = 1.15;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2010
//---------------------------------------------------------------------------------------
static bool detectEscapeSequence (WCharCP str, DgnFontCR font, WChar& realChar, size_t& numCharsInEscapeSequence)
    {
    if (L'\\' != *str)
        return false;

    ++str;

    if (0 == *str)
        return false;

    if (L'\\' == *str)
        {
        realChar                    = L'\\';
        numCharsInEscapeSequence    = 2;
        
        return true;
        }
    
    // Need to differentiate literal zero vs. failure to parse a number... Also don't allow sign characters...
    if (!iswdigit (*str))
        return false;
    
    int charCodeI = BeStringUtilities::Wtoi (str);

    if ((charCodeI < 0) || (charCodeI > USHRT_MAX))
        return false;

    realChar                    = font.RemapFontCharToUnicodeChar ((UInt16)charCodeI);
    numCharsInEscapeSequence    = (1 + (size_t)(floor (log10 ((double)charCodeI)) + 1.0));
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
bool                            DgnFont::ContainsCharacter          (WChar uniChar) const                                                   {return _ContainsCharacter(uniChar);}
DgnFontPtr                      DgnFont::Embed                      (DgnProjectR project) const                                             {return _Embed(project);}
LangCodePage                    DgnFont::GetCodePage                () const                                                                {return _GetCodePage();}
FontChar                        DgnFont::GetDegreeCharCode          () const                                                                {return _GetDegreeCharCode();}
double                          DgnFont::GetDescenderRatio          () const                                                                {return _GetDescenderRatio();}
FontChar                        DgnFont::GetDiameterCharCode        () const                                                                {return _GetDiameterCharCode();}
Utf8StringCR                    DgnFont::GetName                    () const                                                                {return m_name;}
FontChar                        DgnFont::GetPlusMinusCharCode       () const                                                                {return _GetPlusMinusCharCode();}
DgnFontType                     DgnFont::GetType                    () const                                                                {return _GetType();}
DgnFontVariant                  DgnFont::GetVariant                 () const                                                                {return _GetVariant();}
bool                            DgnFont::IsLastResort               () const                                                                {return m_isLastResort;}
bool                            DgnFont::IsMissing                  () const                                                                {return m_isMissing;}
BentleyStatus                   DgnFont::LayoutGlyphs               (DgnGlyphLayoutContextCR context, DgnGlyphLayoutResultR result) const   {return _LayoutGlyphs(context, result);}
WChar                           DgnFont::RemapFontCharToUnicodeChar (FontChar charCode) const                                               {return _RemapFontCharToUnicodeChar(charCode);}
FontChar                        DgnFont::RemapUnicodeCharToFontChar (WChar uniChar) const                                                   {return _RemapUnicodeCharToFontChar(uniChar);}
bool                            DgnFont::ShouldDrawWithLineWeight   () const                                                                {return _ShouldDrawWithLineWeight();}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnFont::DgnFont (Utf8CP name, DgnFontConfigurationData const& config) :
    m_name          (name),
    m_isMissing     (false),
    m_isLastResort  (false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
void DgnFont::CompressEscapeSequences (WStringR outStr, WCharCP inStr) const
    {
    if (WString::IsNullOrEmpty (inStr))
        {
        outStr.clear ();
        return;
        }

    size_t                  lenOfcompressedStr          = wcslen (inStr);
    ScopedArray<WChar>      compressedStrBuff           (lenOfcompressedStr + 1);
    WCharP                  compressedStr               = compressedStrBuff.GetData ();
    WChar                   realChar;
    size_t                  numCharsInEscapeSequence;
    
    BeStringUtilities::Wcsncpy (compressedStr, (lenOfcompressedStr + 1), inStr);

    for (WCharP currCharP = compressedStr; ((size_t)(currCharP - compressedStr) < lenOfcompressedStr); ++currCharP)
        {
        if (!detectEscapeSequence (currCharP, *this, realChar, numCharsInEscapeSequence))
            continue;
        
        *currCharP = realChar;
        
        BeStringUtilities::Wmemmove (
            (currCharP + 1),
            (lenOfcompressedStr - (size_t)((currCharP + 1) - compressedStr) + 1),
            (currCharP + numCharsInEscapeSequence),
            (((compressedStr + lenOfcompressedStr) - (currCharP + numCharsInEscapeSequence)) + 1));

        lenOfcompressedStr -= (numCharsInEscapeSequence - 1);
        }
    
    outStr.assign (compressedStr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
bool DgnFont::DoGlyphsHaveBlankGeometry (FontCharCP charCodes, size_t numCharCodes) const
    {
    for (UInt32 iCharCode = 0; iCharCode < numCharCodes; ++iCharCode)
        {
        if (!_IsGlyphBlank (charCodes[iCharCode]))
            return false;
        }
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
bool DgnFont::Equals (DgnFontCR rhs) const
    {
    return ((_GetType () == rhs._GetType ()) && m_name.EqualsI (rhs.m_name.c_str ()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
void DgnFont::ExpandEscapeSequences (WStringR outStr, WCharCP inStr) const
    {
    if (WString::IsNullOrEmpty (inStr))
        {
        outStr.clear ();
        return;
        }

    outStr.clear ();

    for (WCharCP currCharP = inStr; (0 != *currCharP); ++currCharP)
        {
        if (L'\\' == *currCharP)
            outStr.append (L"\\\\");
        else
            outStr.push_back (*currCharP);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
WString DgnFont::FontCharsToUnicodeString (FontCharCP fontCharBuffer) const
    {
    WString uniString;

    if ((NULL == fontCharBuffer) || (0 == *fontCharBuffer))
        return uniString;
    
    for (FontCharCP currFontChar = fontCharBuffer; 0 != *currFontChar; ++currFontChar)
        uniString += RemapFontCharToUnicodeChar (*currFontChar);
    
    return uniString;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
bool DgnFont::IsAccessibleByProject (DgnProjectCR project) const
    {
    // Last resort fonts are always available; also always consider missing fonts available.
    if (IsLastResort() || IsMissing())
        return true;

    return (NULL != DgnFontManager::FindFont (m_name.c_str (), _GetType (), &project));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
WChar DgnFont::_RemapFontCharToUnicodeChar (FontChar charCode) const
    {
    // If the font is already Unicode, nothing to remap.
    if (LangCodePage::Unicode == _GetCodePage ())
        return charCode;

    // Some trivial remappings (do before 0x7f check because RSC fonts remap these all over the place).
    if (charCode == _GetDegreeCharCode ())    return SPECIALCHAR_UnicodeDegree;
    if (charCode == _GetPlusMinusCharCode ()) return SPECIALCHAR_UnicodePlusMinus;
    if (charCode == _GetDiameterCharCode ())  return SPECIALCHAR_UnicodeDiameter;

    // Character meanings under 128 are the same in every encoding.
    if (charCode <= 0x007f)
        return charCode;

    char localeBuffer[3];
    memset (&localeBuffer, 0, sizeof (localeBuffer));
    
    *reinterpret_cast<FontCharP>(localeBuffer) = charCode;
    
    WString tempUniString;
    BeStringUtilities::LocaleCharToWChar (tempUniString, localeBuffer, _GetCodePage (), _countof (localeBuffer));

    BeAssert ((1 == tempUniString.size ()) && L"Couldn't remap the locale FontChar to a single system-dependent Unicode character.");

    return tempUniString[0];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
FontChar DgnFont::_RemapUnicodeCharToFontChar (WChar uniChar) const
    {
    // If the font is already Unicode, nothing to remap.
    if (LangCodePage::Unicode == _GetCodePage ())
        return (FontChar)uniChar;

    // Character meanings under 128 are the same in every encoding.
    if (uniChar <= 0x007f)
        return (FontChar)uniChar;

    // Some trivial remappings.
    if (SPECIALCHAR_UnicodeDegree == uniChar)      return _GetDegreeCharCode ();
    if (SPECIALCHAR_UnicodePlusMinus == uniChar)   return _GetPlusMinusCharCode ();
    if (SPECIALCHAR_UnicodeDiameter == uniChar)    return _GetDiameterCharCode ();

    WChar   uniBuffer[]         = { uniChar, 0 };
    AString tempLocaleString;
    
    if (UNEXPECTED_CONDITION (SUCCESS != BeStringUtilities::WCharToLocaleChar (tempLocaleString, _GetCodePage (), uniBuffer, _countof (uniBuffer))))
        return (FontChar)uniChar;

    if (UNEXPECTED_CONDITION (tempLocaleString.size () > 2))
        return (FontChar)uniChar;
    
    if (1 == tempLocaleString.size ())
        return (0x00ff & (FontChar)tempLocaleString[0]);
    
    return *reinterpret_cast<FontCharCP>(&tempLocaleString[0]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontCP DgnFont::ResolveToRenderFont () const
    {
    if (!IsMissing ())
        return this;
    
    switch (GetVariant ())
        {
        case DGNFONTVARIANT_Rsc:        return &DgnFontManager::GetDefaultRscFont ();
        case DGNFONTVARIANT_ShxShape:   // Fall through; distinction for SHX is only whether it's a big font or not
        case DGNFONTVARIANT_ShxUni:     return &DgnFontManager::GetDefaultShxFont ();
        case DGNFONTVARIANT_ShxBig:     return DgnFontManager::GetDefaultShxBigFont ();
        case DGNFONTVARIANT_TrueType:   return &DgnFontManager::GetDefaultTrueTypeFont ();
        }
    
    BeAssert (false && L"Unknown/unexpected DgnFontVariant.");
    return &DgnFontManager::GetDefaultTrueTypeFont ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
T_FontChars DgnFont::UnicodeStringToFontChars (WCharCP uniString) const
    {
    T_FontChars fontCharBuffer;

    if (!WString::IsNullOrEmpty (uniString))
        {    
        for (WCharCP currUniChar = uniString; 0 != *currUniChar; ++currUniChar)
            fontCharBuffer.push_back (RemapUnicodeCharToFontChar (*currUniChar));
        }
    
    fontCharBuffer.push_back (0);

    return fontCharBuffer;
    }

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
static WString detectEffectiveLocaleName ()
    {
#ifdef WIP_CFGVAR // MS_RTCONFIG
    WString rtConfig;
    if (SUCCESS == ConfigurationManager::GetVariable (rtConfig, L"MS_RTCONFIG"))
        {
        if (0 == BeStringUtilities::Wcsicmp (L"english", rtConfig.c_str ()))        return L"english";
        if (0 == BeStringUtilities::Wcsicmp (L"japanese", rtConfig.c_str ()))       return L"japanese";
        if (0 == BeStringUtilities::Wcsicmp (L"simpchinese", rtConfig.c_str ()))    return L"simpchinese";
        if (0 == BeStringUtilities::Wcsicmp (L"korean", rtConfig.c_str ()))         return L"korean";
        if (0 == BeStringUtilities::Wcsicmp (L"tradchinese", rtConfig.c_str ()))    return L"tradchinese";
        if (0 == BeStringUtilities::Wcsicmp (L"vietnamese", rtConfig.c_str ()))     return L"vietnamese";
        if (0 == BeStringUtilities::Wcsicmp (L"arabic", rtConfig.c_str ()))         return L"arabic";
        if (0 == BeStringUtilities::Wcsicmp (L"hebrew", rtConfig.c_str ()))         return L"hebrew";
        if (0 == BeStringUtilities::Wcsicmp (L"thai", rtConfig.c_str ()))           return L"thai";

        BeAssert (false && L"Unknown/unexpected MS_RTCONFIG value.");
        }
#endif

    LangCodePage acp = LangCodePage::Unknown;
    BeStringUtilities::GetCurrentCodePage (acp);
    switch (acp)
        {
        case LangCodePage::LatinI:               return L"english";
        case LangCodePage::Japanese:             return L"japanese";
        case LangCodePage::Simplified_Chinese:   return L"simpchinese";
        case LangCodePage::Korean:               return L"korean";
        case LangCodePage::Traditional_Chinese:  return L"tradchinese";
        case LangCodePage::Vietnamese:           return L"vietnamese";
        case LangCodePage::Arabic:               return L"arabic";
        case LangCodePage::Hebrew:               return L"hebrew";
        case LangCodePage::OEM_Thai:             return L"thai";

// WIP_NONPORT
#if !defined (BENTLEY_WIN32)
        // Only allowed on Unix where there isn't really an ACP.
        case LangCodePage::None:                 return L"english";
#endif

        }
    
    BeAssert (false && L"Unknown/unexpected active code page.");
    return L"english";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static DgnFontCP findFirstAvailableFont (Utf8CP fontNamesString, DgnFontType type, DgnProjectCP project)
    {
    T_Utf8StringVector fontNames;
    BeStringUtilities::Split(fontNamesString, ",;", NULL, fontNames);

    for (T_Utf8StringVector::const_iterator fontNameIter = fontNames.begin (); fontNames.end () != fontNameIter; ++fontNameIter)
        {
        DgnFontCP foundFont = DgnFontManager::FindFont (fontNameIter->c_str (), type, project);
        if (NULL == foundFont)
            continue;
        
        return foundFont;
        }
    
    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
static void getFontsFromCatalog (bmap<DgnFontKey, DgnFontCP>& foundFontsMap, T_FontCatalogMap catalog, DgnFontVariant variant)
    {
    for (T_FontCatalogMap::const_iterator fontIter = catalog.begin (); catalog.end () != fontIter; ++fontIter)
        {
        DgnFontCR font = *fontIter->second;
        if ((DGNFONTVARIANT_DontCare != variant) && (0 == (font.GetVariant () & variant)))
            continue;

        DgnFontKey fontKey (font.GetType (), font.GetName());
        if (foundFontsMap.end () != foundFontsMap.find (fontKey))
            continue;
            
        foundFontsMap[fontKey] = &font;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnFontCR   DgnFontManager::GetDecoratorFont        ()  { return *T_HOST.GetDgnFontManager ().m_decoratorFont; }
DgnFontCR   DgnFontManager::GetDefaultRscFont       ()  { return *T_HOST.GetDgnFontManager ().m_defaultRscFont; }
DgnFontCR   DgnFontManager::GetDefaultShxFont       ()  { return *T_HOST.GetDgnFontManager ().m_defaultShxFont; }
DgnFontCP   DgnFontManager::GetDefaultShxBigFont    ()  { return T_HOST.GetDgnFontManager ().m_defaultShxBigFont; }
DgnFontCR   DgnFontManager::GetDefaultTrueTypeFont  ()  { return *T_HOST.GetDgnFontManager ().m_defaultTrueTypeFont; }
bool        DgnFontManager::IsUsingAnRtlLocale      ()  { return T_HOST.GetDgnFontManager ().m_isUsingAnRtlLocale; }
bool        DgnFontManager::IsFontLinkingEnabled    ()  { return T_HOST.GetDgnFontManager ().m_isFontLinkingEnabled; }
bool        DgnFontManager::IsGlyphShapingDisabled  ()  { return T_HOST.GetDgnFontManager ().m_isGlyphShapingDisabled; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnFontType DgnFontManager::ConvertFontVariantToType (DgnFontVariant fontVariant)
    {
    switch (fontVariant)
        {
        case DGNFONTVARIANT_Rsc:        return DgnFontType::Rsc;
        case DGNFONTVARIANT_TrueType:   return DgnFontType::TrueType;
        case DGNFONTVARIANT_ShxShape:   return DgnFontType::Shx;
        case DGNFONTVARIANT_ShxUni:     return DgnFontType::Shx;
        case DGNFONTVARIANT_ShxBig:     return DgnFontType::Shx;
        }
    
    BeAssert (false && L"Unknown/unexpected DgnFontVariant");
    return DgnFontType::None;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnFontCP DgnFontManager::FindFont (Utf8CP name, DgnFontType type, DgnProjectCP project)
    {
    // Embedded fonts get first priority.
    if (NULL != project)
        {
        auto foundFontIter = project->Fonts().EmbeddedFonts().find (DgnFontKey (type, name));
        if (project->Fonts().EmbeddedFonts().end () != foundFontIter)
            return foundFontIter->second.get ();
        }
    
    auto systemFonts  = T_HOST.GetDgnFontManager().m_systemFonts;
    auto foundFontIter = systemFonts.find (DgnFontKey (type, name));
    
    if (systemFonts.end () != foundFontIter)
        return foundFontIter->second.get ();
    
    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
DgnFontConfigurationData const* DgnFontManager::FindCustomFontConfiguration (DgnFontKey const& fontKey)
    {
    auto&   configMap   = T_HOST.GetDgnFontManager().m_perFontConfigurations;
    auto    foundConfig = configMap.find (fontKey);
    
    // No specific entry? Try the wildcard...
    if (configMap.end () == foundConfig)
        foundConfig = configMap.find (DgnFontKey(fontKey.m_type, "*"));
    
    if (configMap.end () == foundConfig)
        return NULL;
    
    return &foundConfig->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnFontCR DgnFontManager::GetFontForCodePage (UInt32 baseFontID, UInt32 shxBigFontID, DgnProjectCR project)
    {
    DgnFontCP shxBigFont = DgnFontManager::ResolveFont (shxBigFontID, project, DGNFONTVARIANT_ShxBig);
    if (NULL != shxBigFont)
        return *shxBigFont;
    
    return *DgnFontManager::ResolveFont (baseFontID, project, DGNFONTVARIANT_DontCare);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
T_DgnFontCPs DgnFontManager::GetFonts (DgnFontVariant variant, DgnProjectCP project)
    {
    bmap<DgnFontKey, DgnFontCP> foundFontsMap;
    
    // Embedded fonts get first priority.
    if (NULL != project)
        getFontsFromCatalog (foundFontsMap, project->Fonts().EmbeddedFonts(), variant);
    
    getFontsFromCatalog (foundFontsMap, T_HOST.GetDgnFontManager ().m_systemFonts, variant);
    
    T_DgnFontCPs fonts;

    for (auto fontIter : foundFontsMap)
        fonts.push_back (fontIter.second);

    return fonts;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnFontCR DgnFontManager::GetLastResortRscFont () const
    {
    if (!m_lastResortRscFont.IsValid ())
        {
        m_lastResortRscFont = DgnRscFont::CreateLastResortFont ();
        BeAssert (m_lastResortRscFont.IsValid ());
        }

    return *m_lastResortRscFont;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnFontCR DgnFontManager::GetLastResortShxFont () const
    {
    if (!m_lastResortShxFont.IsValid ())
        {
        m_lastResortShxFont = DgnShxFont::CreateLastResortFont ();
        BeAssert (m_lastResortShxFont.IsValid ());
        }
    
    return *m_lastResortShxFont;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnFontCR DgnFontManager::GetLastResortTrueTypeFont () const
    {
    if (!m_lastResortTrueTypeFont.IsValid ())
        {
        m_lastResortTrueTypeFont = DgnTrueTypeFont::CreateLastResortFont ();
        BeAssert (m_lastResortTrueTypeFont.IsValid ());
        }
    
    return *m_lastResortTrueTypeFont;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
void DgnFontManager::Initialize ()
    {
    BeFileName  fontConfigPath = T_HOST.GetFontAdmin ()._GetFontConfigurationFilePath ();
    BeXmlStatus xmlStatus;
    BeXmlDomPtr dom   = BeXmlDom::CreateAndReadFromFile (xmlStatus, fontConfigPath.GetName ());
    bool isConfigurationValid = ((BEXML_Success == xmlStatus) && dom.IsValid ());
    
    if (!isConfigurationValid)
        T_HOST.GetFontAdmin ()._OnNoFontConfiguration (fontConfigPath.GetName ());
    else
        ReadPerFontConfigurations (*dom);
    
    DgnRscFont::AcquireSystemFonts (m_systemFonts);
    DgnShxFont::AcquireSystemFonts (m_systemFonts);
    DgnTrueTypeFont::AcquireSystemFonts (m_systemFonts);

    if (isConfigurationValid)
        {
        ProcessLocaleConfiguration (*dom, detectEffectiveLocaleName ().c_str ());
        }
    else
        {
        m_defaultRscFont            = &GetLastResortRscFont ();
        m_defaultShxFont            = &GetLastResortShxFont ();
        m_defaultShxBigFont         = NULL;
        m_defaultTrueTypeFont       = &GetLastResortTrueTypeFont ();
        m_isUsingAnRtlLocale        = false;
        m_isFontLinkingEnabled      = false;
        m_isGlyphShapingDisabled    = false;
        }
    
    Utf8String decoratorFontName = T_HOST.GetGraphicsAdmin ()._GetDecoratorFontName ();

    // We only plan to support TrueType for the decorator font. GraphicsAdmin::_GetDecoratorFontName is documented as such.
    m_decoratorFont = const_cast<DgnFontP>(findFirstAvailableFont (decoratorFontName.c_str (), DgnFontType::TrueType, NULL));
    if (!m_decoratorFont.IsValid ())
        {
// WIP_NONPORT - TrueType
#if defined (BENTLEY_WIN32)
        m_decoratorFont = DgnTrueTypeFont::CreateDecoratorLogicalFont ();
#else
        m_decoratorFont = const_cast<DgnFontP>(&GetLastResortTrueTypeFont ());
#endif
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  05/2013
//---------------------------------------------------------------------------------------
void DgnFontManager::InitializeRscFontFinder (IDgnFontFinder& importer)
    {
    DgnRscFont::RegisterIDgnFontFinder (importer);
    DgnRscFont::AcquireSystemFonts (m_systemFonts);

    m_lastResortRscFont = NULL;
    m_defaultRscFont = &GetLastResortRscFont ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
void DgnFontManager::_OnHostTermination (bool)
    {
    DgnRscFont::OnHostTermination ();
    DgnTrueTypeFont::OnHostTermination ();
    
    delete this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
void DgnFontManager::ProcessLocaleConfiguration (BeXmlDomR dom, WCharCP effectiveLocaleName)
    {
    // Supports UniscribeServices::ItemizeString to indicate when strings are considered to start right-to-left.
    m_isUsingAnRtlLocale = ((0 == BeStringUtilities::Wcsicmp(L"arabic", effectiveLocaleName)) || (0 == BeStringUtilities::Wcsicmp(L"hebrew", effectiveLocaleName)));

    BeXmlDom::IterableNodeSet languageNodes;
    dom.SelectNodes (languageNodes, "/FontConfig/Languages/LangInfo", NULL);

    for (BeXmlDom::IterableNodeSet::const_iterator nodeIter = languageNodes.begin (); languageNodes.end () != nodeIter; ++nodeIter)
        {
        BeXmlNodeP&         localeElement           = *nodeIter;
        xmlXPathContextPtr  localeElementContext    = dom.AcquireXPathContext (localeElement);
        
        WString localeName;
        if ((BEXML_Success != dom.SelectNodeContent (localeName, "./Language", localeElementContext, BeXmlDom::NODE_BIAS_Last)) || localeName.empty ())
            { BeDataAssert (false && L"Invalid or missing Language attribute in language element."); continue; }
        
        if (0 != BeStringUtilities::Wcsicmp (localeName.c_str (), effectiveLocaleName))
            continue;
        
        WString defaultRscFontName;
        if ((BEXML_Success != dom.SelectNodeContent (defaultRscFontName, "./DefaultRscFont", localeElementContext, BeXmlDom::NODE_BIAS_Last)) || defaultRscFontName.empty ())
            { BeDataAssert (false && L"Invalid or missing DefaultRscFont attribute in language element."); continue; }
        
        m_defaultRscFont = findFirstAvailableFont (Utf8String (defaultRscFontName.c_str ()).c_str (), DgnFontType::Rsc, NULL);
        if ((NULL == m_defaultRscFont) || m_defaultRscFont->IsMissing ())
            m_defaultRscFont = &GetLastResortRscFont ();

        WString defaultShxFontName;
        if ((BEXML_Success != dom.SelectNodeContent (defaultShxFontName, "./DefaultShxFont", localeElementContext, BeXmlDom::NODE_BIAS_Last)) || defaultShxFontName.empty ())
            { BeDataAssert (false && L"Invalid or missing DefaultShxFont attribute in language element."); continue; }
        
        m_defaultShxFont = findFirstAvailableFont (Utf8String (defaultShxFontName.c_str ()).c_str (), DgnFontType::Shx, NULL);
        if ((NULL == m_defaultShxFont) || m_defaultShxFont->IsMissing ())
            m_defaultShxFont = &GetLastResortShxFont ();

        WString defaultShxBigFontName;
        if ((BEXML_Success != dom.SelectNodeContent (defaultShxBigFontName, "./DefaultShxBigFont", localeElementContext, BeXmlDom::NODE_BIAS_Last)) || defaultShxBigFontName.empty ())
            {
            m_defaultShxBigFont = NULL;
            }
        else
            {
            m_defaultShxBigFont = findFirstAvailableFont (Utf8String (defaultShxBigFontName.c_str ()).c_str (), DgnFontType::Shx, NULL);
            if ((NULL != m_defaultShxBigFont) && (m_defaultShxBigFont->IsMissing () || (DGNFONTVARIANT_ShxBig != m_defaultShxBigFont->GetVariant ())))
                m_defaultShxBigFont = NULL;
            }

        WString defaultTTFontName;
        if ((BEXML_Success != dom.SelectNodeContent (defaultTTFontName, "./DefaultTTFont", localeElementContext, BeXmlDom::NODE_BIAS_Last)) || defaultTTFontName.empty ())
            { BeDataAssert (false && L"Invalid or missing DefaultTTFont attribute in language element."); continue; }
        
        m_defaultTrueTypeFont = findFirstAvailableFont (Utf8String (defaultTTFontName.c_str ()).c_str (), DgnFontType::TrueType, NULL);
        if ((NULL == m_defaultTrueTypeFont) || m_defaultTrueTypeFont->IsMissing ())
            m_defaultTrueTypeFont = &GetLastResortTrueTypeFont ();
        
        if (BEXML_Success != dom.SelectNodeContentAsBool (m_isFontLinkingEnabled, "./EnableFontLinking", localeElementContext, BeXmlDom::NODE_BIAS_Last))
            m_isFontLinkingEnabled = false;

        bool isGlyphShapingDisabled;
        if (BEXML_Success != dom.SelectNodeContentAsBool(isGlyphShapingDisabled, "./DisableGlyphShaping", localeElementContext, BeXmlDom::NODE_BIAS_Last))
            isGlyphShapingDisabled = false;
        
        m_isGlyphShapingDisabled = isGlyphShapingDisabled;
        
        return;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
void DgnFontManager::ReadPerFontConfigurations (BeXmlDomR dom)
    {
    BeXmlDom::IterableNodeSet fontNodes;
    dom.SelectNodes (fontNodes, "/FontConfig/Fonts/*", NULL);

    for (BeXmlDom::IterableNodeSet::const_iterator nodeIter = fontNodes.begin (); fontNodes.end () != nodeIter; ++nodeIter)
        {
        BeXmlNodeP& fontElement         = *nodeIter;
        bool        isForRscFont        = (0 == BeStringUtilities::Stricmp ("RscFontInfo", reinterpret_cast<CharCP>(fontElement->name)));
        bool        isForShxFont        = (0 == BeStringUtilities::Stricmp ("ShxFontInfo", reinterpret_cast<CharCP>(fontElement->name)));
        bool        isForTrueTypeFont   = (0 == BeStringUtilities::Stricmp ("TTFontInfo", reinterpret_cast<CharCP>(fontElement->name)));
        
        if (!isForRscFont && !isForShxFont && !isForTrueTypeFont)
            { BeDataAssert (false && L"Unknown font element name."); continue; }

        xmlXPathContextPtr          fontElementContext = dom.AcquireXPathContext (fontElement);
        DgnFontConfigurationData    fontConfig;
        
        WString fontNamesValue;
        if ((BEXML_Success != dom.SelectNodeContent (fontNamesValue, "./Name", fontElementContext, BeXmlDom::NODE_BIAS_Last)) || fontNamesValue.empty ())
            { BeDataAssert (false && L"Invalid or missing Name attribute in font element."); continue; }
        
        UInt32 codePageValue;
        if (BEXML_Success != dom.SelectNodeContentAsUInt32 (codePageValue, "./CodePage", fontElementContext, BeXmlDom::NODE_BIAS_Last))
            codePageValue = 0;
        
        fontConfig.m_codePage = (LangCodePage)codePageValue;

        DgnFontType fontType = DgnFontType::None;        

        if (isForRscFont)
            {
            // Defaults are common for all RSC fonts, so we can set them here.
            if (BEXML_Success != dom.SelectNodeContentAsUInt16 (fontConfig.m_degreeCharCode, "./DegreeChar", fontElementContext, BeXmlDom::NODE_BIAS_Last))
                fontConfig.m_degreeCharCode = 94;
            
            if (BEXML_Success != dom.SelectNodeContentAsUInt16 (fontConfig.m_diameterCharCode, "./DiameterChar", fontElementContext, BeXmlDom::NODE_BIAS_Last))
                fontConfig.m_diameterCharCode = 216;
            
            if (BEXML_Success != dom.SelectNodeContentAsUInt16 (fontConfig.m_plusMinusCharCode, "./PlusMinusChar", fontElementContext, BeXmlDom::NODE_BIAS_Last))
                fontConfig.m_plusMinusCharCode = 200;

            if (BEXML_Success != dom.SelectNodeContentAsBool (fontConfig.m_shouldCreateShxUnifont, "/CreateShxUnifont", NULL, BeXmlDom::NODE_BIAS_Last))
                fontConfig.m_shouldCreateShxUnifont = true;
            
            fontType = DgnFontType::Rsc;
            }
        else if (isForShxFont)
            {
            // Because default values vary by SHX font type, which we don't know here, defer until getEffectiveFontConfig in DgnShxFont.cpp.
            if (BEXML_Success != dom.SelectNodeContentAsUInt16 (fontConfig.m_degreeCharCode, "./DegreeChar", fontElementContext, BeXmlDom::NODE_BIAS_Last))
                fontConfig.m_degreeCharCode = 0;
            
            if (BEXML_Success != dom.SelectNodeContentAsUInt16 (fontConfig.m_diameterCharCode, "./DiameterChar", fontElementContext, BeXmlDom::NODE_BIAS_Last))
                fontConfig.m_diameterCharCode = 0;
            
            if (BEXML_Success != dom.SelectNodeContentAsUInt16 (fontConfig.m_plusMinusCharCode, "./PlusMinusChar", fontElementContext, BeXmlDom::NODE_BIAS_Last))
                fontConfig.m_plusMinusCharCode = 0;
            
            fontType = DgnFontType::Shx;
            }
        else
            {
            fontType = DgnFontType::TrueType;
            }

        bvector<WString> fontNames;
        BeStringUtilities::Split(fontNamesValue.c_str(), L",;", NULL, fontNames);

        for (auto fontNameIter  : fontNames)
            m_perFontConfigurations.Insert (DgnFontKey (fontType, Utf8String(fontNameIter)), fontConfig);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2012
//---------------------------------------------------------------------------------------
DgnFontCP DgnFontManager::ResolveFont (UInt32 fontID, DgnProjectCR project, DgnFontVariant preferredFallbackType)
    {
    DgnFontCP foundFont = project.Fonts().FindFont(fontID);
    if ((NULL != foundFont) && !foundFont->IsMissing () && ((DGNFONTVARIANT_DontCare == preferredFallbackType) || (foundFont->GetVariant () == preferredFallbackType)))
        return foundFont;

    if ((NULL != foundFont) && (DGNFONTVARIANT_DontCare == preferredFallbackType))
        preferredFallbackType = foundFont->GetVariant ();
    
    switch (preferredFallbackType)
        {
        case DGNFONTVARIANT_DontCare:   { return &DgnFontManager::GetDefaultTrueTypeFont (); }
        case DGNFONTVARIANT_Rsc:        { return &DgnFontManager::GetDefaultRscFont (); }
        case DGNFONTVARIANT_TrueType:   { return &DgnFontManager::GetDefaultTrueTypeFont (); }
        case DGNFONTVARIANT_ShxShape:   { return &DgnFontManager::GetDefaultShxFont (); }
        case DGNFONTVARIANT_ShxUni:     { return &DgnFontManager::GetDefaultShxFont (); }
        case DGNFONTVARIANT_ShxBig:     { return DgnFontManager::GetDefaultShxBigFont (); }
        }

    BeAssert (false && L"Unknown/unexpected DgnFontVariant.");
    return NULL;
    }
