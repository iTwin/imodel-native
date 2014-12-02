/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/Fraction.cpp $
|
|   $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace std;

static const double STACKEDFRACTION_DEFAULT_SCALE                       =   0.7;
static const double STACKEDFRACTION_DIAGONAL_DENOMINATOR_OFFSET_FACTOR  =   0.30;
static const double STACKEDFRACTION_DIAGONAL_NUMERATOR_OFFSET_FACTOR    =   0.70;
static const double STACKEDFRACTION_NORMAL_NUMERATOR_OFFSET_FACTOR      =   4.0 / 3.0;
static const double STACKEDFRACTION_OVERLINE_OFFSET_SCALE_FACTOR        =   1.0 / 6.0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- Fraction --------------------------------------------------------------------------------------------------------------------------- Fraction --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
Fraction::Fraction (TextParamWideCR textParams, DPoint2dCR fontSize, DgnModelR dgnCache) :
    Run (textParams, fontSize, dgnCache),
    m_numeratorRun      (NULL),
    m_denominatorRun    (NULL),
    m_isDirty           (true)
    {
    m_textScale.x = STACKEDFRACTION_DEFAULT_SCALE;
    m_textScale.y = STACKEDFRACTION_DEFAULT_SCALE;

    m_properties.SetLineAlignment ((LineAlignment)textParams.exFlags.stackedFractionAlign);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
Fraction::Fraction (RunPropertiesCR runProperties, DgnGlyphRunLayoutFlags layoutFlags) :
    Run (runProperties, layoutFlags),
    m_numeratorRun      (NULL),
    m_denominatorRun    (NULL),
    m_isDirty           (true)
    {
    m_textScale.x = STACKEDFRACTION_DEFAULT_SCALE;
    m_textScale.y = STACKEDFRACTION_DEFAULT_SCALE;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
Fraction::Fraction (FractionCR fraction) :
    Run (fraction),
    m_numeratorRun      (NULL),
    m_denominatorRun    (NULL),
    m_textScale         (fraction.m_textScale),
    m_isDirty           (true)
    {
    if (NULL != fraction.m_numeratorRun)
        m_numeratorRun = new CharStream (*fraction.m_numeratorRun);

    if (NULL != fraction.m_denominatorRun)
        m_denominatorRun = new CharStream (*fraction.m_denominatorRun);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/06
//---------------------------------------------------------------------------------------
Fraction::~Fraction ()
    {
    DELETE_AND_CLEAR (m_numeratorRun);
    DELETE_AND_CLEAR (m_denominatorRun);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
size_t                      Fraction::_GetCharacterCount            () const                                { return 1; }
WChar                       Fraction::_GetCharacter                 (size_t index) const                    { return 0 == index ? L' ' : 0; }
size_t                      Fraction::_GetNextWordBreakIndex        (size_t offset) const                   { return (0 == offset) ? 0 : 1; }
AppendStatus                Fraction::_AppendNextRunToLine          (LineR, RunP&, ProcessContextCR)        { return APPEND_STATUS_RunsNotMergeable; }
bool                        Fraction::_CanFit                       (LineR line, ProcessContextCR context)  { return FitsInLine (line, context); }
void                        Fraction::_ComputeRange                 ()                                      { }
bool                        Fraction::_ContainsOnlyWhitespace       () const                                { return false; }

CharStreamP                 Fraction::GetCharStream                 (size_t index) const                    { return (index == 0) ? m_numeratorRun : m_denominatorRun; }
DPoint2dCR                  Fraction::GetTextScale                  () const                                { return m_textScale; }
void                        Fraction::SetTextScale                  (DPoint2dCR textScale)                  { m_textScale = textScale; m_isDirty = true; }
WChar                       Fraction::GetSeparatorChar              () const                                { return _GetSeparatorChar (); }
StackedFractionType         Fraction::GetFractionType               () const                                { return _GetFractionType (); }
StackedFractionAlignment    Fraction::GetAlignment                  () const                                { return (StackedFractionAlignment)m_properties.GetLineAlignment (); }
void                        Fraction::SetAlignment                  (StackedFractionAlignment value)        { m_properties.SetLineAlignment ((LineAlignment)value); m_isDirty = true; }
CharStreamCP                Fraction::GetNumerator                  () const                                { return m_numeratorRun; }
CharStreamCP                Fraction::GetDenominator                () const                                { return m_denominatorRun; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/07
//---------------------------------------------------------------------------------------
void Fraction::Init (WCharCP numText, WCharCP denText, StackedFractionAlignment fractionAlignment, DPoint2dCP textScaleFactor)
    {
    this->SetAlignment (fractionAlignment);

    if (NULL != textScaleFactor)
        {
        this->SetTextScale (*textScaleFactor);
        }
    else
        {
        m_textScale.x = STACKEDFRACTION_DEFAULT_SCALE;
        m_textScale.y = STACKEDFRACTION_DEFAULT_SCALE;
        }

    if (!WString::IsNullOrEmpty (numText))
        m_numeratorRun = new CharStream (numText, m_properties, m_layoutFlags);

    if (!WString::IsNullOrEmpty (denText))
        m_denominatorRun = new CharStream (denText, m_properties, m_layoutFlags);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/05
//---------------------------------------------------------------------------------------
void Fraction::_GenerateNumeratorElementParams (TextParamWideR textParams, DPoint2dR fontSize, DgnProjectR project) const
    {
    m_properties.ToElementData (textParams, project);

    textParams.flags.underline                  = m_numeratorRun->GetProperties ().IsUnderlined ();
    textParams.underlineSpacing                 = m_numeratorRun->GetProperties ().GetUnderlineOffset ();
    textParams.exFlags.overline                 = false;
    textParams.overlineSpacing                  = 0.0;
    textParams.renderingFlags.lineAlignment     = false;
    textParams.exFlags.stackedFractionSection   = (UInt32)StackedFractionSection::Numerator;
    textParams.exFlags.stackedFractionType      = (UInt32)this->GetFractionType ();
    textParams.exFlags.stackedFractionAlign     = static_cast<UInt32>(this->GetAlignment ());

    fontSize.x = m_properties.GetFontSize().x * m_textScale.x;
    fontSize.y = m_properties.GetFontSize().y * m_textScale.y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/05
//---------------------------------------------------------------------------------------
void Fraction::_GenerateDenominatorElementParams (TextParamWideR textParams, DPoint2dR fontSize, DgnProjectR project) const
    {
    m_properties.ToElementData (textParams, project);

    textParams.flags.underline                  = false;
    textParams.underlineSpacing                 = 0.0;
    textParams.exFlags.overline                 = m_denominatorRun->GetProperties ().IsOverlined ();
    textParams.overlineSpacing                  = m_denominatorRun->GetProperties ().GetOverlineOffset ();
    textParams.renderingFlags.lineAlignment     = false;
    textParams.exFlags.stackedFractionSection   = (UInt32)StackedFractionSection::Denominator;
    textParams.exFlags.stackedFractionType      = (UInt32)this->GetFractionType ();
    textParams.exFlags.stackedFractionAlign     = static_cast<UInt32>(this->GetAlignment ());

    fontSize.x = m_properties.GetFontSize().x * m_textScale.x;
    fontSize.y = m_properties.GetFontSize().y * m_textScale.y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
bool Fraction::FitsInLine (LineCR line, ProcessContextCR processContext)
    {
    TextBlockCP textBlock   = processContext.GetTextBlock ();
    ParagraphCP paragraph   = processContext.GetParagraph ();
    
    if ((textBlock->GetProperties ().GetMaxCharactersPerLine () > 0) && ((line.GetNumberOfChars () + 1) > textBlock->GetProperties ().GetMaxCharactersPerLine ()))
        return false;

    if (0.0 == textBlock->GetProperties ().GetDocumentWidth ())
        return true;
    
    this->Preprocess ();
    
    DRange3d    lineRange       = line.GetNominalRange ();
    DRange3d    nextWordRange   = this->ComputeJustificationRange ();
    double      indentation     = ((paragraph->GetLine (0) == &line) ? paragraph->GetProperties ().GetIndentation().GetFirstLineIndent () : paragraph->GetProperties ().GetIndentation().GetHangingIndent ());

    if (lineRange.IsNull ())
        memset (&lineRange, 0, sizeof (lineRange));
    
    if (textBlock->GetProperties ().IsVertical ())
        {
        // Remember, vertical text starts at y=0.0 and goes negative, so low.y is negative (but still indicates effective line length).
        double  lineHeight          = -lineRange.low.y;
        double  nextWordFitHeight   = -nextWordRange.low.y;

        return ((indentation + lineHeight + nextWordFitHeight) <= textBlock->GetProperties ().GetDocumentWidth ());
        }
    
    // Horizontal case.
    double  lineWidth           = lineRange.high.x;
    double  nextWordFitWidth    = nextWordRange.high.x;

    return ((indentation + lineWidth + nextWordFitWidth) <= textBlock->GetProperties ().GetDocumentWidth ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void Fraction::PreprocessCharStream (CharStreamR charStream) const
    {
    RunProperties runProperties = charStream.GetProperties ();

    DPoint2d fontSize = { GetProperties ().GetFontSize ().x * m_textScale.x, GetProperties ().GetFontSize ().y * m_textScale.y };
    runProperties.SetFontSize (fontSize);

    charStream.SetProperties (runProperties);
    charStream.Preprocess ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
double Fraction::_GetMaxDisplacementAboveOrigin () const
    {
    this->Preprocess ();

    if (m_nominalRange.isNull ())
        return 0.0;

    return m_nominalRange.high.y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
double Fraction::_GetMaxDisplacementBelowOrigin () const
    {
    this->Preprocess ();

    if (m_nominalRange.isNull ())
        return 0.0;

    return m_nominalRange.low.y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
double Fraction::_GetMaxExactHeightAboveOrigin () const
    {
    this->Preprocess ();

    if (m_exactRange.isNull ())
        return 0.0;

    return m_exactRange.high.y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
double Fraction::_GetMaxExactDepthBelowOrigin () const
    {
    this->Preprocess ();

    if (m_exactRange.isNull ())
        return 0.0;

    return m_exactRange.low.y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2009
//---------------------------------------------------------------------------------------
WString Fraction::_ToString (size_t offset, size_t length, TextBlockToStringOptionsCR options) const
    {
#if defined (_WIN32) || !defined (ANDROID) // WIP_NONPORT - stringstream not supported on Android

    // Fractions act as a single character for most intents and purposes...
    if ((0 != offset) || (0 == length))
        return WString ();
    
    if (options.ShouldSubstitueAtomicRunContent ())
        return WString (1, options.GetAtomicRunContentSubstituteChar ());
    
    wostringstream oss;
    
    if (NULL != m_numeratorRun)
        oss << m_numeratorRun->ToString ();
    
    if (NULL != m_numeratorRun && NULL != m_denominatorRun)
        oss << this->GetSeparatorChar ();
    
    if (NULL != m_denominatorRun)
        oss << m_denominatorRun->ToString ();
    
    return WString (oss.str ().c_str ());

#else
    /*WIP_NONPORT*/
    return L"";
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void Fraction::_ComputeOffsetToChar (DVec3dR offset, double& scale, CaretCR caret) const
    {
    offset.Zero ();
    
    if (caret.GetCharacterIndex () > 0)
        offset.x = this->GetNominalWidth ();
    
    scale = this->GetProperties ().GetFontSize ().y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/07
//---------------------------------------------------------------------------------------
void Fraction::_Splice (RunP& firstRun, RunP& secondRun, size_t index)
    {
    BeAssert (index <= 1);

    if (0 == index)
        {
        firstRun    = NULL;
        secondRun   = this;
        return;
        }

    firstRun    = this;
    secondRun   = NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
AppendStatus Fraction::_AppendToLine (LineR line, RunP& nextRun, ProcessContextCR processContext)
    {
    if (!line.IsEmpty () && !FitsInLine (line, processContext))
        return APPEND_STATUS_Overflow;

    line.AddRun (this);

    return APPEND_STATUS_Appended;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
void Fraction::_Preprocess () const
    {
    if (!m_isDirty)
        return;
    
    if (NULL != m_numeratorRun)
        PreprocessCharStream (*m_numeratorRun);

    if (NULL != m_denominatorRun)
        PreprocessCharStream (*m_denominatorRun);

    _SetupNumeratorLocation ();
    _SetupDenominatorLocation ();

    _SetupNumeratorProperties ();
    _SetupDenominatorProperties ();
    
    m_isDirty = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/07
//---------------------------------------------------------------------------------------
void Fraction::_Draw (ViewContextR context, bool isViewIndependent, TextBlockDrawOptionsCR options) const
    {
    Transform transform = this->GetTransform ();
    context.PushTransform (transform);
        {
        if (NULL != m_numeratorRun)
            m_numeratorRun->Draw (context, isViewIndependent, this->GetFractionType (), StackedFractionSection::Numerator, options);

        if (NULL != m_denominatorRun)
            m_denominatorRun->Draw (context, isViewIndependent, this->GetFractionType (), StackedFractionSection::Denominator, options);
        }
    context.PopTransformClip ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/08
//---------------------------------------------------------------------------------------
void Fraction::_SetProperties (RunPropertiesCR updatedRunProperties)
    {
    T_Super::_SetProperties (updatedRunProperties);

    m_isDirty = true;

    if (NULL != m_numeratorRun)
        m_numeratorRun->SetProperties (updatedRunProperties);

    if (NULL != m_denominatorRun)
        m_denominatorRun->SetProperties (updatedRunProperties);

    Preprocess ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/08
//---------------------------------------------------------------------------------------
void Fraction::SetNumeratorText (WCharCP val)
    {
    if (NULL == val)
        { BeAssert (false); return; }
    
    // !!! Only safe to call before added to a TextBlock !!!
    if (NULL != m_numeratorRun)
        delete m_numeratorRun;

    m_numeratorRun  = new CharStream (val, m_properties, m_layoutFlags);
    m_isDirty       = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/08
//---------------------------------------------------------------------------------------
void Fraction::SetDenominatorText (WCharCP val)
    {
    if (NULL == val)
        { BeAssert (false); return; }
    
    // !!! Only safe to call before added to a TextBlock !!!
    if (NULL != m_denominatorRun)
        delete m_denominatorRun;

    m_denominatorRun    = new CharStream (val, m_properties, m_layoutFlags);
    m_isDirty           = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void Fraction::GenerateElementParameters (TextParamWideR textParams, DPoint2dR fontSize, size_t index, DgnProjectR project) const
    {
    return (0 == index) ? _GenerateNumeratorElementParams (textParams, fontSize, project) : _GenerateDenominatorElementParams (textParams, fontSize, project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          03/08
//---------------------------------------------------------------------------------------
bool Fraction::IsUnderlined () const
    {
    CharStreamCP numerator = GetNumerator ();
    if (NULL == numerator)
        return  false;

    CharStreamCP denominator = GetDenominator ();
    if (NULL == denominator)
        return  false;

    /*-----------------------------------------------------------------------------------
    Underline for fraction is exclusively determined by properties of its numerator and
    denominator components.  It does not seem to be controlled by run properties at all.
    That is, combinations of underline and overline determine whether the whole piece of
    fraction is underlined.
    -----------------------------------------------------------------------------------*/
    return (numerator->GetProperties ().IsUnderlined () || denominator->GetProperties ().IsOverlined ()) && denominator->GetProperties ().IsUnderlined ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/08
//---------------------------------------------------------------------------------------
DPoint2d Fraction::GetDefaultFractionScaleFactor ()
    {
    DPoint2d defaultScaleFactor = { STACKEDFRACTION_DEFAULT_SCALE, STACKEDFRACTION_DEFAULT_SCALE };
    return defaultScaleFactor;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
bool Fraction::_Equals (RunCR rhsRun, TextBlockCompareOptionsCR compareOptions) const
    {
    FractionCP rhs = dynamic_cast<FractionCP>(&rhsRun);
    if (NULL == rhs)
        return false;
    
    if (!T_Super::_Equals (rhsRun, compareOptions))
        return false;
    
    if (!m_textScale.IsEqual (rhs->m_textScale, compareOptions.GetTolerance ())) return false;
    
    if ((NULL == m_numeratorRun) != (NULL == rhs->m_numeratorRun))                                      return false;
    if (((NULL != m_numeratorRun) && !m_numeratorRun->Equals (*rhs->m_numeratorRun, compareOptions)))   return false;
    
    if ((NULL == m_denominatorRun) != (NULL == rhs->m_denominatorRun))                                      return false;
    if (((NULL != m_denominatorRun) && !m_denominatorRun->Equals (*rhs->m_denominatorRun, compareOptions))) return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
bool Fraction::_IsAtomic () const
    {
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
StackedFractionType Fraction::ComputeTypeFromSeparatorChar (WChar separatorChar)
    {
    switch (separatorChar)
        {
        case L'^':  return StackedFractionType::NoBar;
        case L'#':  return StackedFractionType::DiagonalBar;
        case L'/':  return StackedFractionType::HorizontalBar;
        }
    
    BeAssert (false && L"Unknown separator character.");
    return StackedFractionType::None;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
WChar Fraction::ComputeSeparatorCharFromType (StackedFractionType fractionType)
    {
    switch (fractionType)
        {
        case StackedFractionType::NoBar:           return L'^';
        case StackedFractionType::DiagonalBar:     return L'#';
        case StackedFractionType::HorizontalBar:   return L'/';
        }
    
    BeAssert (false && L"Unknown StackedFractionType.");
    return L' ';
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2011
//---------------------------------------------------------------------------------------
void Fraction::_GetElementRange (DRange3dR value) const
    {
    if (m_numeratorRun && m_denominatorRun)
        {
        DRange3d    numeratorRange;     m_numeratorRun->GetElementRange (numeratorRange);
        DRange3d    denominatorRange;   m_denominatorRun->GetElementRange (denominatorRange);

        m_numeratorRun->GetTransform ().Multiply (numeratorRange, numeratorRange);
        m_denominatorRun->GetTransform ().Multiply (denominatorRange, denominatorRange);

        value.Init ();
        value.Extend (&numeratorRange.low, 2);
        value.Extend (&denominatorRange.low, 2);

        return;
        }
    
    if (m_numeratorRun)
        {
        DRange3d numeratorRange;
        m_numeratorRun->GetElementRange (numeratorRange);
        m_numeratorRun->GetTransform ().Multiply (numeratorRange, numeratorRange);

        value.Init ();
        value.Extend (&numeratorRange.low, 2);

        return;
        }
    
    
    DRange3d denominatorRange;
    m_denominatorRun->GetElementRange (denominatorRange);
    m_denominatorRun->GetTransform ().Multiply (denominatorRange, denominatorRange);

    value.Init ();
    value.Extend (&denominatorRange.low, 2);
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- NoBarFraction ----------------------------------------------------------------------------------------------------------------- NoBarFraction --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/07
//---------------------------------------------------------------------------------------
NoBarFraction::NoBarFraction (RunPropertiesCR runProperties, DgnGlyphRunLayoutFlags layoutFlags) :
    Fraction (runProperties, layoutFlags)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/06
//---------------------------------------------------------------------------------------
NoBarFraction::NoBarFraction (WCharCP numText, WCharCP denText, TextParamWideCR textParams, DPoint2dCR fontSize, StackedFractionAlignment fractionAlignment, DPoint2dCP textScaleFactor, DgnModelR dgnCache) :
    Fraction (textParams, fontSize, dgnCache)
    {
    if (NULL == numText && NULL == denText)
        BeAssert (false);
    
    Init (numText, denText, fractionAlignment, textScaleFactor);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
NoBarFraction::NoBarFraction (WCharCP numText, WCharCP denText, RunPropertiesCR runProperties, DgnGlyphRunLayoutFlags layoutFlags, StackedFractionAlignment fractionAlignment, DPoint2dCP textScaleFactor) :
    Fraction (runProperties, layoutFlags)
    {
    if (NULL == numText && NULL == denText)
        BeAssert (false);
    
    Init (numText, denText, fractionAlignment, textScaleFactor);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
RunP                NoBarFraction::_Clone                      () const    { return new NoBarFraction (*this); }
void                NoBarFraction::_SetupNumeratorProperties   () const    { }
void                NoBarFraction::_SetupDenominatorProperties () const    { }
StackedFractionType NoBarFraction::_GetFractionType            () const    { return StackedFractionType::NoBar; }
WChar             NoBarFraction::_GetSeparatorChar           () const    { return GetProperties ().IsUnderlined () ? '/' : '^'; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
void NoBarFraction::_Preprocess () const
    {
    if (!m_isDirty)
        return;
    
    // T_Super sets m_isDirty.
    T_Super::_Preprocess ();

    NoBarFraction& nonConstThis = const_cast<NoBarFraction&>(*this);

    m_nominalRange.init ();
    m_exactRange.init ();

    if (NULL != m_numeratorRun)
        {
        nonConstThis.ExtendNominalRange (m_numeratorRun->ComputeTransformedNominalRange ());
        nonConstThis.ExtendExactRange (m_numeratorRun->ComputeTransformedExactRange ());
        }

    if (NULL != m_denominatorRun)
        {
        nonConstThis.ExtendNominalRange (m_denominatorRun->ComputeTransformedNominalRange ());
        nonConstThis.ExtendExactRange (m_denominatorRun->ComputeTransformedExactRange ());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void NoBarFraction::_SetupNumeratorLocation () const
    {
    if (NULL == m_numeratorRun)
        return;

    DVec3d originOffset;
    originOffset.y = m_numeratorRun->GetProperties ().GetFontSize ().y * STACKEDFRACTION_NORMAL_NUMERATOR_OFFSET_FACTOR;
    originOffset.x = originOffset.z = 0.0;
    originOffset.z = 0.0;

    DPoint3d origin;
    origin.init (&originOffset);
    m_numeratorRun->SetOrigin (origin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void NoBarFraction::_SetupDenominatorLocation () const
    {
    if (NULL == m_denominatorRun)
        return;

    DRange3d    denominatorRange    = m_denominatorRun->ComputeTransformedNominalRange ();
    double      numeratorWidth      = (NULL == m_numeratorRun) ? 0.0 : m_numeratorRun->GetNominalWidth ();
    double      denominatorWidth    = denominatorRange.isNull () ? 0.0 : (denominatorRange.high.x - denominatorRange.low.x);

    DVec3d originOffset;
    originOffset.x = (numeratorWidth > denominatorWidth) ? (numeratorWidth - denominatorWidth) * 0.5 : 0.0;
    originOffset.y = originOffset.z = 0.0;

    DPoint3d origin;
    origin.init (&originOffset);
    m_denominatorRun->SetOrigin (origin);
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- HorizontalBarFraction ------------------------------------------------------------------------------------------------- HorizontalBarFraction --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/07
//---------------------------------------------------------------------------------------
HorizontalBarFraction::HorizontalBarFraction (RunPropertiesCR runProperties, DgnGlyphRunLayoutFlags layoutFlags) :
    Fraction (runProperties, layoutFlags)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/06
//---------------------------------------------------------------------------------------
HorizontalBarFraction::HorizontalBarFraction (WCharCP numText, WCharCP denText, TextParamWideCR textParams, DPoint2dCR fontSize, StackedFractionAlignment fractionAlignment, DPoint2dCP textScaleFactor, DgnModelR dgnCache) :
    Fraction (textParams, fontSize, dgnCache)
    {
    if (NULL == numText && NULL == denText)
        BeAssert (false);
    
    Init (numText, denText, fractionAlignment, textScaleFactor);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
HorizontalBarFraction::HorizontalBarFraction (WCharCP numText, WCharCP denText, RunPropertiesCR runProperties, DgnGlyphRunLayoutFlags layoutFlags, StackedFractionAlignment fractionAlignment, DPoint2dCP textScaleFactor) :
    Fraction (runProperties, layoutFlags)
    {
    if (NULL == numText && NULL == denText)
        BeAssert (false);
    
    Init (numText, denText, fractionAlignment, textScaleFactor);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
RunP                HorizontalBarFraction::_Clone            () const    { return new HorizontalBarFraction (*this); }
StackedFractionType HorizontalBarFraction::_GetFractionType  () const    { return StackedFractionType::HorizontalBar; }
WChar             HorizontalBarFraction::_GetSeparatorChar () const    { return '/'; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
void HorizontalBarFraction::_Preprocess () const
    {
    if (!m_isDirty)
        return;
    
    // T_Super sets m_isDirty.
    T_Super::_Preprocess ();

    HorizontalBarFraction& nonConstThis = const_cast<HorizontalBarFraction&>(*this);

    m_nominalRange.init ();
    m_exactRange.init ();

    if (NULL != m_numeratorRun)
        {
        nonConstThis.ExtendNominalRange (m_numeratorRun->ComputeTransformedNominalRange ());
        nonConstThis.ExtendExactRange (m_numeratorRun->ComputeTransformedExactRange ());
        }

    if (NULL != m_denominatorRun)
        {
        nonConstThis.ExtendNominalRange (m_denominatorRun->ComputeTransformedNominalRange ());
        nonConstThis.ExtendExactRange (m_denominatorRun->ComputeTransformedExactRange ());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void HorizontalBarFraction::_SetupNumeratorLocation () const
    {
    if (NULL == m_numeratorRun)
        return;

    double      numeratorWidth      = m_numeratorRun->GetNominalWidth ();
    double      denominatorWidth    = (NULL == m_denominatorRun) ? 0.0 : m_denominatorRun->GetNominalWidth ();

    DVec3d originOffset;
    originOffset.x = numeratorWidth > denominatorWidth ? 0.0 : (denominatorWidth - numeratorWidth) * 0.5;
    originOffset.y = m_numeratorRun->GetProperties ().GetFontSize ().y * STACKEDFRACTION_NORMAL_NUMERATOR_OFFSET_FACTOR;
    originOffset.z = 0.0;

    DPoint3d origin;
    origin.init (&originOffset);
    m_numeratorRun->SetOrigin (origin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void HorizontalBarFraction::_SetupDenominatorLocation () const
    {
    if (NULL == m_denominatorRun)
        return;

    DRange3d    denominatorRange    = m_denominatorRun->ComputeTransformedNominalRange ();
    double      numeratorWidth      = (NULL == m_numeratorRun) ? 0.0 : m_numeratorRun->GetNominalWidth ();
    double      denominatorWidth    = denominatorRange.isNull () ? 0.0 : (denominatorRange.high.x - denominatorRange.low.x);

    DVec3d originOffset;
    originOffset.x = (numeratorWidth > denominatorWidth) ? (numeratorWidth - denominatorWidth) * 0.5 : 0.0;
    originOffset.y = originOffset.z = 0.0;

    DPoint3d origin;
    origin.init (&originOffset);
    m_denominatorRun->SetOrigin (origin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void HorizontalBarFraction::_SetupNumeratorProperties () const
    {
    if (NULL == m_numeratorRun)
        return;

    RunProperties   numeratorProperties = m_numeratorRun->GetProperties ();
    double          numeratorWidth      = m_numeratorRun->GetNominalWidth ();
    double          denominatorWidth    = (NULL == m_denominatorRun) ? 0.0 : m_denominatorRun->GetNominalWidth ();

    numeratorProperties.SetIsOverlined (false);

    if (numeratorWidth > denominatorWidth && NULL != m_denominatorRun)
        {
        numeratorProperties.SetIsUnderlined (true);
        numeratorProperties.SetUnderlineOffset (STACKEDFRACTION_OVERLINE_OFFSET_SCALE_FACTOR * numeratorProperties.GetFontSize ().y);
        }
    else
        {
        numeratorProperties.SetIsUnderlined (false);
        numeratorProperties.SetUnderlineOffset (0.0);
        }

    m_numeratorRun->SetProperties (numeratorProperties);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void HorizontalBarFraction::_SetupDenominatorProperties () const
    {
    if (NULL == m_denominatorRun)
        return;

    RunProperties   denominatorProperties   = m_denominatorRun->GetProperties ();
    double          numeratorWidth          = (NULL == m_numeratorRun) ? 0.0 : m_numeratorRun->GetNominalWidth ();
    double          denominatorWidth        = m_denominatorRun->GetNominalWidth ();

    denominatorProperties.SetIsUnderlined (false);

    if (numeratorWidth > denominatorWidth)
        {
        denominatorProperties.SetIsOverlined (false);
        denominatorProperties.SetOverlineOffset (0.0);
        }
    else
        {
        denominatorProperties.SetIsOverlined (true);
        denominatorProperties.SetOverlineOffset (STACKEDFRACTION_OVERLINE_OFFSET_SCALE_FACTOR * denominatorProperties.GetFontSize ().y);
        }

    m_denominatorRun->SetProperties (denominatorProperties);
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- DiagonalBarFraction ----------------------------------------------------------------------------------------------------- DiagonalBarFraction --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/07
//---------------------------------------------------------------------------------------
DiagonalBarFraction::DiagonalBarFraction (RunPropertiesCR runProperties, DgnGlyphRunLayoutFlags layoutFlags) :
    Fraction (runProperties, layoutFlags)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/06
//---------------------------------------------------------------------------------------
DiagonalBarFraction::DiagonalBarFraction (WCharCP numText, WCharCP denText, TextParamWideCR textParams, DPoint2dCR fontSize, StackedFractionAlignment fractionAlignment, DPoint2dCP textScaleFactor, DgnModelR dgnCache) :
    Fraction (textParams, fontSize, dgnCache)
    {
    if (NULL == numText && NULL == denText)
        BeAssert (false);
    
    Init (numText, denText, fractionAlignment, textScaleFactor);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
DiagonalBarFraction::DiagonalBarFraction (WCharCP numText, WCharCP denText, RunPropertiesCR runProperties, DgnGlyphRunLayoutFlags layoutFlags, StackedFractionAlignment fractionAlignment, DPoint2dCP textScaleFactor) :
    Fraction (runProperties, layoutFlags)
    {
    if (NULL == numText && NULL == denText)
        BeAssert (false);
    
    Init (numText, denText, fractionAlignment, textScaleFactor);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
RunP                DiagonalBarFraction::_Clone                        () const    { return new DiagonalBarFraction (*this); }
void                DiagonalBarFraction::_SetupNumeratorProperties     () const    { }
void                DiagonalBarFraction::_SetupDenominatorProperties   () const    { }
StackedFractionType DiagonalBarFraction::_GetFractionType              () const    { return StackedFractionType::DiagonalBar; }
WChar             DiagonalBarFraction::_GetSeparatorChar             () const    { return '#'; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
double DiagonalBarFraction::_GetNominalHeight () const
    {
    return m_nominalRange.isNull () ? 0.0 : m_nominalRange.high.y - m_nominalRange.low.y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
void DiagonalBarFraction::_Preprocess () const
    {
    if (!m_isDirty)
        return;
    
    // T_Super sets m_isDirty.
    T_Super::_Preprocess ();

    DiagonalBarFraction& nonConstThis = const_cast<DiagonalBarFraction&>(*this);

    m_nominalRange.init ();
    m_exactRange.init ();

    if (NULL != m_numeratorRun)
        {
        // Run offset does not normally affect ranges, because it does not normally affect layout; however, it affects our reported ranges.
        DRange3d    effectiveRange;
        DPoint2d    runOffset       = m_numeratorRun->GetProperties ().GetRunOffset ();
        
        effectiveRange          = m_numeratorRun->ComputeTransformedNominalRange ();
        effectiveRange.low.x    += runOffset.x;
        effectiveRange.low.y    += runOffset.y;
        effectiveRange.high.x   += runOffset.x;
        effectiveRange.high.y   += runOffset.y;
        
        nonConstThis.ExtendNominalRange (effectiveRange);
        
        effectiveRange          = m_numeratorRun->ComputeTransformedExactRange ();
        effectiveRange.low.x    += runOffset.x;
        effectiveRange.low.y    += runOffset.y;
        effectiveRange.high.x   += runOffset.x;
        effectiveRange.high.y   += runOffset.y;
        
        nonConstThis.ExtendExactRange (effectiveRange);
        }

    if (NULL != m_denominatorRun)
        {
        // Run offset does not normally affect ranges, because it does not affect layout; however, it affects our reported ranges.
        DRange3d    effectiveRange;
        DPoint2d    runOffset       = m_denominatorRun->GetProperties ().GetRunOffset ();
        
        effectiveRange          = m_denominatorRun->ComputeTransformedNominalRange ();
        effectiveRange.low.x    += runOffset.x;
        effectiveRange.low.y    += runOffset.y;
        effectiveRange.high.x   += runOffset.x;
        effectiveRange.high.y   += runOffset.y;
        
        nonConstThis.ExtendNominalRange (effectiveRange);
        
        effectiveRange          = m_denominatorRun->ComputeTransformedExactRange ();
        effectiveRange.low.x    += runOffset.x;
        effectiveRange.low.y    += runOffset.y;
        effectiveRange.high.x   += runOffset.x;
        effectiveRange.high.y   += runOffset.y;
        
        nonConstThis.ExtendExactRange (effectiveRange);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void DiagonalBarFraction::_SetupNumeratorLocation () const
    {
    if (NULL == m_numeratorRun)
        return;

    DRange3d numeratorNominalRange = m_numeratorRun->GetNominalRange ();

    DVec2d runOffset;
    runOffset.x = 0.0;
    runOffset.y = numeratorNominalRange.isNull () ? 0.0 : (numeratorNominalRange.high.y - numeratorNominalRange.low.y) * STACKEDFRACTION_DIAGONAL_NUMERATOR_OFFSET_FACTOR;

    RunProperties runProperties = m_numeratorRun->GetProperties ();
    runProperties.SetRunOffset (runOffset);
    
    m_numeratorRun->SetProperties (runProperties);

    DPoint3d origin;
    origin.Zero ();
    m_numeratorRun->SetOrigin (origin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void DiagonalBarFraction::_SetupDenominatorLocation () const
    {
    if (NULL == m_denominatorRun)
        return;

    DRange3d denominatorNominalRange = m_denominatorRun->GetNominalRange ();

    DVec2d runOffset;
    runOffset.x = 0.0;
    runOffset.y = denominatorNominalRange.isNull () ? 0.0 : (denominatorNominalRange.high.y - denominatorNominalRange.low.y) * -STACKEDFRACTION_DIAGONAL_DENOMINATOR_OFFSET_FACTOR;

    RunProperties runProperties = m_denominatorRun->GetProperties ();
    runProperties.SetRunOffset (runOffset);

    m_denominatorRun->SetProperties (runProperties);

    DPoint3d origin;
    origin.x = (NULL == m_numeratorRun) ? m_denominatorRun->GetNominalWidth () : m_numeratorRun->GetNominalWidth ();
    origin.y = origin.z = 0.0;

    m_denominatorRun->SetOrigin (origin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/08
//---------------------------------------------------------------------------------------
double DiagonalBarFraction::GetNumeratorOffsetFactor () const
    {
    return STACKEDFRACTION_DIAGONAL_NUMERATOR_OFFSET_FACTOR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/08
//---------------------------------------------------------------------------------------
double DiagonalBarFraction::GetDenominatorOffsetFactor () const
    {
    return STACKEDFRACTION_DIAGONAL_DENOMINATOR_OFFSET_FACTOR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/08
//---------------------------------------------------------------------------------------
double DiagonalBarFraction::ComputeAdditionalRunYOffset () const
    {
    return GetDenominatorOffsetFactor () * m_properties.GetFontSize ().y * GetTextScale ().y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
void DiagonalBarFraction::_GenerateNumeratorElementParams (TextParamWideR textParams, DPoint2dR fontSize, DgnProjectR project) const
    {
    T_Super::_GenerateNumeratorElementParams (textParams, fontSize, project);
    
    textParams.lineOffset   = m_numeratorRun->GetProperties ().GetRunOffset ();
    textParams.flags.offset = 1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
void DiagonalBarFraction::_GenerateDenominatorElementParams (TextParamWideR textParams, DPoint2dR fontSize, DgnProjectR project) const
    {
    T_Super::_GenerateDenominatorElementParams (textParams, fontSize, project);
    
    textParams.lineOffset   = m_denominatorRun->GetProperties ().GetRunOffset ();
    textParams.flags.offset = 1;
    }
