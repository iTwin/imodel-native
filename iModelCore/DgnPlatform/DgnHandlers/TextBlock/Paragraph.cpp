/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/Paragraph.cpp $
|
|   $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace std;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- ParagraphProperties::Overrides ------------------------------------------------------------------------------- ParagraphProperties::Overrides --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
ParagraphProperties::Overrides::Overrides ()
    {
    Clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void ParagraphProperties::Overrides::Clear ()
    {
    m_justification     = false;
    m_isFullJustified   = false;
    m_lineSpacingType   = false;
    m_lineSpacingValue  = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
bool ParagraphProperties::Overrides::Equals (Overrides const & rhs) const
    {
    if (m_justification     != rhs.m_justification)     return false;
    if (m_isFullJustified   != rhs.m_isFullJustified)   return false;
    if (m_lineSpacingType   != rhs.m_lineSpacingType)   return false;
    if (m_lineSpacingValue  != rhs.m_lineSpacingValue)  return false;
    
    return true;
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- ParagraphProperties ----------------------------------------------------------------------------------------------------- ParagraphProperties --
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
static void clearLineSpacingPropertyOverride (bool& overrideFlag, double& value, DgnTextStylePtr textStyle, DgnTextStyleProperty tsProp, DgnModelR dgnModel, DgnLineSpacingType lsType)
    {
    if (!clearPropertyOverride (overrideFlag, value, textStyle, tsProp) || (DgnLineSpacingType::AtLeast == lsType))
        return;
    
    double scale;
    textStyle->GetPropertyValue (DgnTextStyleProperty::Height, scale);
    
    value *= scale;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
IDgnTextStyleApplyable const &  ParagraphProperties::AsIDgnTextStyleApplyable       () const                            { return *this; }
IDgnTextStyleApplyable&         ParagraphProperties::AsIDgnTextStyleApplyableR      ()                                  { return *this; }
ParagraphPropertiesPtr          ParagraphProperties::Clone                          () const                            { return new ParagraphProperties (*this); }
DgnModelR                       ParagraphProperties::GetDgnModelR                   () const                            { return *m_dgnModel; }
TextElementJustification        ParagraphProperties::GetJustification               () const                            { return m_justification; }
void                            ParagraphProperties::SetJustification               (TextElementJustification value)    { m_justification = value; if (this->HasTextStyle ()) m_overrides.m_justification = true; }
bool                            ParagraphProperties::IsJustificationOverridden      () const                            { return m_overrides.m_justification; }
void                            ParagraphProperties::ClearJustificationOverride     ()                                  { clearPropertyOverride (m_overrides.m_justification, (UInt32&)m_justification, this->GetTextStyleInFile (), DgnTextStyleProperty::Justification); }
bool                            ParagraphProperties::IsFullJustified                () const                            { return m_isFullJustified; }
void                            ParagraphProperties::SetIsFullJustified             (bool value)                        { m_isFullJustified = value; if (this->HasTextStyle ()) m_overrides.m_isFullJustified = true; }
bool                            ParagraphProperties::IsFullJustifiedOverridden      () const                            { return m_overrides.m_isFullJustified; }
void                            ParagraphProperties::ClearIsFullJustifiedOverride   ()                                  { clearPropertyOverride (m_overrides.m_isFullJustified, m_isFullJustified, this->GetTextStyleInFile (), DgnTextStyleProperty::IsFullJustification); }
DgnLineSpacingType                 ParagraphProperties::GetLineSpacingType             () const                            { return m_lineSpacingType; }
void                            ParagraphProperties::SetLineSpacingType             (DgnLineSpacingType value)             { m_lineSpacingType = value; if (this->HasTextStyle ()) m_overrides.m_lineSpacingType = true; }
bool                            ParagraphProperties::IsLineSpacingTypeOverridden    () const                            { return m_overrides.m_lineSpacingType; }
void                            ParagraphProperties::ClearLineSpacingTypeOverride   ()                                  { clearPropertyOverride (m_overrides.m_lineSpacingType, (UInt32&)m_lineSpacingType, this->GetTextStyleInFile (), DgnTextStyleProperty::LineSpacingType); }
double                          ParagraphProperties::GetLineSpacingValue            () const                            { return m_lineSpacingValue; }
void                            ParagraphProperties::SetLineSpacingValue            (double value)                      { m_lineSpacingValue = value; if (this->HasTextStyle ()) m_overrides.m_lineSpacingValue = true; }
bool                            ParagraphProperties::IsLineSpacingValueOverridden   () const                            { return m_overrides.m_lineSpacingValue; }
void                            ParagraphProperties::ClearLineSpacingValueOverride  ()                                  { clearLineSpacingPropertyOverride (m_overrides.m_lineSpacingValue, m_lineSpacingValue, this->GetTextStyleInFile (), DgnTextStyleProperty::LineSpacingValueFactor, *m_dgnModel, m_lineSpacingType); }
IndentationDataCR               ParagraphProperties::GetIndentation                 () const                            { return m_indentation; }
void                            ParagraphProperties::SetIndentation                 (IndentationDataCR value)           { m_indentation = value;}
bool                            ParagraphProperties::_HasTextStyle                  () const                            { return (0 != m_textStyleId && m_dgnModel->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(m_textStyleId)).IsValid ()); }
UInt32                          ParagraphProperties::_GetTextStyleId                () const                            { return m_textStyleId; }
DgnTextStylePtr                 ParagraphProperties::_GetTextStyleInFile            () const                            { return m_dgnModel->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(m_textStyleId)); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
ParagraphPropertiesPtr ParagraphProperties::Create (DgnModelR dgnCache) { return new ParagraphProperties (dgnCache); }
ParagraphProperties::ParagraphProperties (DgnModelR dgnCache) :
    RefCountedBase ()
    {
    this->InitDefaults (dgnCache);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
ParagraphPropertiesPtr ParagraphProperties::Create (TextParamWideCR params, DgnModelR dgnCache) { return new ParagraphProperties (params, dgnCache); }
ParagraphProperties::ParagraphProperties (TextParamWideCR params, DgnModelR dgnCache) :
    RefCountedBase ()
    {
    m_dgnModel = &dgnCache;
    
    this->FromElementData (params);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
ParagraphPropertiesPtr ParagraphProperties::Create (DgnTextStyleCR textStyle, DgnModelR dgnCache) { return new ParagraphProperties (textStyle, dgnCache); }
ParagraphProperties::ParagraphProperties (DgnTextStyleCR textStyle, DgnModelR dgnCache) :
    RefCountedBase ()
    {
    this->InitDefaults (dgnCache);
    this->ApplyTextStyle (textStyle, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
ParagraphProperties::ParagraphProperties (ParagraphPropertiesCR rhs) :
    RefCountedBase (),
    m_dgnModel          (rhs.m_dgnModel),
    m_overrides         (rhs.m_overrides),
    m_justification     (rhs.m_justification),
    m_isFullJustified   (rhs.m_isFullJustified),
    m_lineSpacingType   (rhs.m_lineSpacingType),
    m_lineSpacingValue  (rhs.m_lineSpacingValue),
    m_textStyleId       (rhs.m_textStyleId),
    m_indentation       (rhs.m_indentation)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void ParagraphProperties::InitDefaults (DgnModelR dgnCache)
    {
    m_dgnModel          = &dgnCache;
    m_justification     = TextElementJustification::LeftTop;
    m_isFullJustified   = false;
    m_lineSpacingType   = DgnLineSpacingType::Exact;
    m_lineSpacingValue  = 0.0;
    m_textStyleId       = 0;
    
    // ParagraphProperties::Overrides constructor zeros itself out; depend on that.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void ParagraphProperties::ToElementData (TextParamWideR params) const
    {
    // Of the three parameter structures that have to co-exist in the TextParamWide (RunProperties, ParagraphProperties, TextBlockProperties),
    //  RunProperties is the only one that actually gets to define the text style. During export, it fills in the parameters first, and we
    //  cannot attempt to posthumously compute override flags.
    
    // WARNING: Dangerous because this means the calls have to be in order (annotation scale is TextBlock-based).
    double effectiveAnnotationScale = params.exFlags.annotationScale ? params.annotationScale : 1.0;
    
    params.just                         = (int)m_justification;
    params.exFlags.fullJustification    = m_isFullJustified;
    params.exFlags.acadLineSpacingType  = static_cast<UInt32>(m_lineSpacingType);
    params.lineSpacing                  = m_lineSpacingValue * (m_lineSpacingType == DgnLineSpacingType::AtLeast ? 1.0 : effectiveAnnotationScale);
    
    DgnTextStylePtr paramsTextStyle = m_dgnModel->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(params.textStyleId));
    
    // Params don't have a style? Don't set overrides or style.
    if (!params.flags.textStyle || !paramsTextStyle.IsValid ())
        return;
    
    // Same style? Set overrides as stored.
    if (this->HasTextStyle () && (m_textStyleId == params.textStyleId))
        {
        params.overridesFromStyle.just                  = m_overrides.m_justification;
        params.overridesFromStyle.fullJustification     = m_overrides.m_isFullJustified;
        params.overridesFromStyle.acadLineSpacingType   = m_overrides.m_lineSpacingType;
        params.overridesFromStyle.linespacing           = m_overrides.m_lineSpacingValue;
        
        return;
        }
    
    // Otherwise params have a different style, configure overrides as needed.
    UInt32  paramsTextStyle_justification;      paramsTextStyle->GetPropertyValue(DgnTextStyleProperty::Justification, paramsTextStyle_justification);
    bool    paramsTextStyle_isFullJustified;    paramsTextStyle->GetPropertyValue(DgnTextStyleProperty::IsFullJustification, paramsTextStyle_isFullJustified);
    UInt32  paramsTextStyle_lineSpacingType;    paramsTextStyle->GetPropertyValue(DgnTextStyleProperty::LineSpacingType, paramsTextStyle_lineSpacingType);
    double  paramsTextStyle_lineSpacingValue;   paramsTextStyle->GetPropertyValue(DgnTextStyleProperty::LineSpacingValueFactor, paramsTextStyle_lineSpacingValue);
    
    params.overridesFromStyle.just                  = m_overrides.m_justification       || ((UInt32)m_justification != paramsTextStyle_justification);
    params.overridesFromStyle.fullJustification     = m_overrides.m_isFullJustified     || (m_isFullJustified != paramsTextStyle_isFullJustified);
    params.overridesFromStyle.acadLineSpacingType   = m_overrides.m_lineSpacingType     || (static_cast<UInt32>(m_lineSpacingType) != paramsTextStyle_lineSpacingType);
    params.overridesFromStyle.linespacing           = m_overrides.m_lineSpacingValue    || (m_lineSpacingValue != paramsTextStyle_lineSpacingValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void ParagraphProperties::FromElementData (TextParamWideCR params)
    {
    // WARNING: Dangerous because this means the calls have to be in order (annotation scale is text-block based).
    double effectiveAnnotationScale = params.exFlags.annotationScale ? params.annotationScale : 1.0;
    
    m_overrides.m_justification     = params.overridesFromStyle.just;
    m_overrides.m_isFullJustified   = params.overridesFromStyle.fullJustification;
    m_overrides.m_lineSpacingType   = params.overridesFromStyle.acadLineSpacingType;
    m_overrides.m_lineSpacingValue  = params.overridesFromStyle.linespacing;
    
    m_justification                 = (TextElementJustification)params.just;
    m_isFullJustified               = params.exFlags.fullJustification;
    m_lineSpacingType               = (DgnLineSpacingType)params.exFlags.acadLineSpacingType;
    m_lineSpacingValue              = fabs (params.lineSpacing);
    
    if (DgnLineSpacingType::AtLeast != m_lineSpacingType)
        m_lineSpacingValue /= effectiveAnnotationScale;
    
    if (params.flags.textStyle && (0 != params.textStyleId) && m_dgnModel->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(params.textStyleId)).IsValid ())
        m_textStyleId = params.textStyleId;
    else
        m_textStyleId = 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void ParagraphProperties::_ToStyle (DgnTextStyleR style) const
    {
    double lineSpacingValue = m_lineSpacingValue;
    
    if (DgnLineSpacingType::AtLeast != m_lineSpacingType)
        {
        double factor;
        style.GetPropertyValue (DgnTextStyleProperty::Height, factor);
        
        lineSpacingValue /= factor;
        }
    
    style.SetPropertyValue(DgnTextStyleProperty::Justification,         (UInt32)m_justification);
    style.SetPropertyValue(DgnTextStyleProperty::IsFullJustification,     (bool)m_isFullJustified);
    style.SetPropertyValue(DgnTextStyleProperty::LineSpacingType,       (UInt32)m_lineSpacingType);
    style.SetPropertyValue(DgnTextStyleProperty::LineSpacingValueFactor,           (double)lineSpacingValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
bool ParagraphProperties::Equals (ParagraphPropertiesCR rhs) const { return this->Equals (rhs, 0.0); }
bool ParagraphProperties::Equals (ParagraphPropertiesCR rhs, double tolerance) const
    {
    // Purposefully not comparing DgnModels... it would be a lot of extra work to remap everything,
    //  and I just don't think it will affect many cases. Even in the QATools case where we have
    //  difference DgnModels, they should still have the same units and color table etc.
    
    if (m_justification     != rhs.m_justification)     return false;
    if (m_isFullJustified   != rhs.m_isFullJustified)   return false;
    if (m_lineSpacingType   != rhs.m_lineSpacingType)   return false;
    if (m_textStyleId       != rhs.m_textStyleId)       return false;
    
    if (fabs (m_lineSpacingValue - rhs.m_lineSpacingValue) > tolerance) return false;
    
    if (!m_overrides.Equals     (rhs.m_overrides))              return false;
    if (!m_indentation.Equals   (rhs.m_indentation, tolerance)) return false;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
void ParagraphProperties::_RemoveTextStyle ()
    {
    m_overrides.Clear ();
    m_textStyleId = 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
void ParagraphProperties::SetPropertiesFromStyle (DgnTextStylePropertyMaskCR applyMask, DgnTextStyleCR newStyle, DgnTextStylePropertyMaskCR overridesMask)
    {
    if (applyMask.IsPropertySet (DgnTextStyleProperty::Justification))        {   newStyle.GetPropertyValue (DgnTextStyleProperty::Justification,      (UInt32&)m_justification);      m_overrides.m_justification     = overridesMask.IsPropertySet (DgnTextStyleProperty::Justification);      }
    if (applyMask.IsPropertySet(DgnTextStyleProperty::IsFullJustification))    { newStyle.GetPropertyValue(DgnTextStyleProperty::IsFullJustification, m_isFullJustified);             m_overrides.m_isFullJustified = overridesMask.IsPropertySet(DgnTextStyleProperty::IsFullJustification); }
    if (applyMask.IsPropertySet(DgnTextStyleProperty::LineSpacingType))      { newStyle.GetPropertyValue(DgnTextStyleProperty::LineSpacingType, (UInt32&) m_lineSpacingType);    m_overrides.m_lineSpacingType = overridesMask.IsPropertySet(DgnTextStyleProperty::LineSpacingType); }
    
    if (applyMask.IsPropertySet(DgnTextStyleProperty::LineSpacingValueFactor))
        {
        newStyle.GetPropertyValue (DgnTextStyleProperty::LineSpacingValueFactor, m_lineSpacingValue);
        m_overrides.m_lineSpacingValue = overridesMask.IsPropertySet(DgnTextStyleProperty::LineSpacingValueFactor);
        
        if (DgnLineSpacingType::AtLeast != m_lineSpacingType)
            {
            double scale;
            newStyle.GetPropertyValue (DgnTextStyleProperty::Height, scale);
            
            m_lineSpacingValue *= scale;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void ParagraphProperties::_ApplyTextStyle (DgnTextStyleCR newStyle, bool respectOverrides)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    DgnTextStylePtr             fileStyle       = DgnTextStyle::GetByName (newStyle.GetName ().c_str (), newStyle.GetFile ());
    DgnTextStylePropertyMaskPtr    overridesMask   = fileStyle.IsValid () ? newStyle.Compare (*fileStyle) : DgnTextStylePropertyMask::CreatePropMask ();
    
    if (fileStyle.IsNull () || !respectOverrides)
        m_overrides.Clear ();
    
    DgnTextStylePropertyMaskPtr applyMask = DgnTextStylePropertyMask::CreatePropMask ();
    if (!m_overrides.m_justification)       applyMask->SetPropertyFlag (DgnTextStyleProperty::Justification,        true);
    if (!m_overrides.m_isFullJustified)     applyMask->SetPropertyFlag (DgnTextStyleProperty::FullJustification,    true);
    if (!m_overrides.m_lineSpacingType)     applyMask->SetPropertyFlag (DgnTextStyleProperty::LineSpacingType,      true);
    if (!m_overrides.m_lineSpacingValue)    applyMask->SetPropertyFlag (DgnTextStyleProperty::LineSpacing,          true);
    
    SetPropertiesFromStyle (*applyMask, newStyle, *overridesMask);
    
    m_textStyleId = fileStyle.IsValid () ? (UInt32)fileStyle->GetID () : 0;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
void ParagraphProperties::_SetProperties (DgnTextStyleCR newStyle, DgnTextStylePropertyMaskCR applyMask)
    {
    DgnTextStylePropertyMaskPtr overridesMask = DgnTextStylePropertyMask::Create ();
    if (this->HasTextStyle ())
        overridesMask->SetAllProperties(true);
    
    this->SetPropertiesFromStyle (applyMask, newStyle, *overridesMask);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void ParagraphProperties::ApplyScale (DPoint2dCR scaleFactor, bool isVertical)
    {
    double scale = (isVertical ? scaleFactor.x : scaleFactor.y);
    if (m_lineSpacingType != DgnLineSpacingType::AtLeast)
        m_lineSpacingValue *=scale ;
    
    m_indentation.Scale (scale);
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- Paragraph ------------------------------------------------------------------------------------------------------------------------- Paragraph --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
DgnLineSpacingType             Paragraph::GetLineSpacingType       () const                        { return m_properties.m_lineSpacingType; }
double                      Paragraph::GetLineSpacing           () const                        { return m_properties.m_lineSpacingValue; }
TextElementJustification    Paragraph::GetJustification         () const                        { return m_properties.m_justification; }
bool                        Paragraph::IsFullJustification      () const                        { return m_properties.m_isFullJustified; }
bool                        Paragraph::GetIsFullJustification   () const                        { return m_properties.m_isFullJustified; }
UInt32                      Paragraph::GetLineCount             () const                        { return static_cast<UInt32>(m_lineArray.size ()); }
ParagraphPropertiesCR       Paragraph::GetProperties            () const                        { return m_properties; }

TextBlockNodeLevel          Paragraph::_GetUnitLevel            () const                        { return TEXTBLOCK_NODE_LEVEL_Paragraph; }
void                        Paragraph::AddLine                  (LineR line)                    { m_lineArray.push_back (&line); }
void                        Paragraph::SetProperties            (ParagraphPropertiesCR value)   { m_properties = value; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/04
//---------------------------------------------------------------------------------------
DVec3d Paragraph::ComputeLineSpacing (bool isVertical, double logicalNodeNumberHeight, double annotationScale, LineCR existingLine, LineCR newLine) const
    {
    DVec3d          displacement;
    double          lineSpacing         = (DgnLineSpacingType::AtLeast == this->GetLineSpacingType ()) ? this->GetLineSpacing () : annotationScale * this->GetLineSpacing ();
    DgnLineSpacingType lineSpacingType     = this->GetLineSpacingType ();
    double          nodeNumberHeight    = logicalNodeNumberHeight;
    double          displacementValue   = lineSpacing;

    // Returns total spacing between the top of one line to the top of the next line.
    if (!isVertical)
        {
        if (0.0 != lineSpacing)
            {
            switch (lineSpacingType)
                {
                case DgnLineSpacingType::Exact:
                case DgnLineSpacingType::Automatic:
                    {
                    displacementValue = existingLine.GetMaxDistanceAboveBaseline () + lineSpacing;
                    break;
                    }
                case DgnLineSpacingType::ExactFromLineTop:
                    {
                    double  existingLineAscension   = existingLine.GetMaxDistanceAboveBaseline ();
                    double  newLineAscension        = newLine.GetMaxDistanceAboveBaseline ();

                    double existingLineMaxNominalDescender = 0.0;
                    for (UInt32 runIndex = 0; runIndex < existingLine.GetRunCount (); runIndex++)
                        {
                        double currRunLineOffset = existingLine.GetRun (runIndex)->ComputeTransformedNominalRange ().low.y;

                        if (currRunLineOffset >= 0.0)
                            continue;

                        if (currRunLineOffset > existingLineMaxNominalDescender)
                            existingLineMaxNominalDescender = currRunLineOffset;
                        }
                    
                    double newLineMaxNominalDescender = 0.0;
                    for (UInt32 runIndex = 0; runIndex < newLine.GetRunCount (); runIndex++)
                        {
                        double currRunLineOffset = newLine.GetRun (runIndex)->ComputeTransformedNominalRange ().low.y;

                        if (currRunLineOffset >= 0.0)
                            continue;

                        if (currRunLineOffset > newLineMaxNominalDescender)
                            newLineMaxNominalDescender = currRunLineOffset;
                        }
                    
                    displacementValue = (existingLineAscension - existingLineMaxNominalDescender) + (lineSpacing - newLineAscension + newLineMaxNominalDescender);

                    break;
                    }
                case DgnLineSpacingType::AtLeast:
                    {
                    double  maxAsc  = (existingLine.GetMaxAscender () / 3.0);
                    double  lowY    = existingLine.GetLowestUnitY ();
                    double  nnH     = (nodeNumberHeight / 3.0);

                    displacementValue = ((maxAsc + lowY + nnH) * lineSpacing);

                    break;
                    }
                }
            }
        else
            {
            switch (lineSpacingType)
                {
                case DgnLineSpacingType::Exact:
                case DgnLineSpacingType::Automatic:
                    displacementValue = existingLine.GetMaxUnitHeight () + existingLine.GetMaxDescender ();
                    break;
                case DgnLineSpacingType::ExactFromLineTop:
                case DgnLineSpacingType::AtLeast:
                    displacementValue = existingLine.GetMaxUnitHeight () + existingLine.GetMaxDescender ();
                    break;
                }
            }
        }
    else
        {
        // The calculation of line spacing results in a slight difference between V8.5
        //  and XM.  In V8.5, the algorithm keeps appending chars until it exceeds a line
        //  and once it exceeds at a line break, it starts popping of chars until the previous
        //  word is reached.  In XM, we basically calculate it at the end. Also, the algorithm
        //  calculates width of characters and stores it as maxwidth in line.  Due to the
        //  extra addition and popping off, sometimes if there is character in the next line
        //  that is wider than the chars in the previous line, then the max width is more than
        //  it should be.  This is fixed in XM.  So, in XM in some cases, editing mline vertical
        //  text results in shifting (compression).
        
        double baseWidth = existingLine.GetMaxHorizontalCellIncrement ();

        if (0.0 != this->GetLineSpacing ())
            {
            switch (lineSpacingType)
                {
                case DgnLineSpacingType::Exact:
                    {
                    displacementValue = baseWidth + this->GetLineSpacing ();
                    break;
                    }
                case DgnLineSpacingType::Automatic:
                    {
                    displacementValue = this->GetLineSpacing () > baseWidth ? this->GetLineSpacing () : baseWidth;
                    break;
                    }
                case DgnLineSpacingType::ExactFromLineTop:
                    {
                    displacementValue = this->GetLineSpacing ();
                    break;
                    }
                case DgnLineSpacingType::AtLeast:
                    {
                    // This code was largely derived from trial-and-error by making test cases in AutoCAD 2008.
                    //  I believe there are only two free variables: text height and line spacing factor.
                    //  Testing needs to vary text width and line spacing factors independently, including using
                    //  mixed heights in a single line.
                    //  Previous (i.e. XM) code was insultingly wrong (didn't even look at the line spacing value!).

                    double  minUnitXSize    = DBL_MAX;
                    double  maxUnitXSize    = 0.0;

                    for (size_t runIndex = 0; runIndex < existingLine.GetRunCount (); runIndex++)
                        {
                        double unitWidth = existingLine.GetRun (runIndex)->GetProperties ().GetFontSize ().x ;

                        if (unitWidth < minUnitXSize)
                            minUnitXSize = unitWidth;

                        if (unitWidth > maxUnitXSize)
                            maxUnitXSize = unitWidth;
                        }
                    
                    if (minUnitXSize == maxUnitXSize)
                        displacementValue = maxUnitXSize * (4.0 / 3.0) * this->GetLineSpacing ();
                    else
                        displacementValue = ((3.0 / 4.0) * maxUnitXSize) * (4.0 / 3.0) * this->GetLineSpacing ();

                    break;
                    }
                }
            }
        else
            {
            displacementValue = baseWidth;
            }
        }

    if (isVertical)
        {
        displacement.y = displacement.z = 0.0;
        displacement.x = displacementValue;
        }
    else
        {
        displacement.x = displacement.z = 0.0;
        displacement.y = -displacementValue;
        }

    return displacement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
bool Paragraph::IsComplete () const
    {
    LineP lastLine = m_lineArray.back ();
    if (0 == lastLine->GetRunCount ())
        return false;

    return (NULL != dynamic_cast <ParagraphBreakP> (lastLine->GetRun (lastLine->GetRunCount () - 1)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/05
//---------------------------------------------------------------------------------------
bool Paragraph::IsEmpty () const
    {
    if (GetLineCount () > 1)
        return false;

    return GetLine (0)->IsEmpty ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/04
//---------------------------------------------------------------------------------------
Paragraph::Paragraph (DgnModelR dgnCache) :
    TextBlockNode (),
    m_properties (dgnCache)
    {
    m_lineArray.push_back (new Line ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/04
//---------------------------------------------------------------------------------------
Paragraph::Paragraph (ParagraphCR rhs) :
    TextBlockNode (rhs),
    m_properties (rhs.m_properties)
    {
    for (size_t i = 0; i < rhs.m_lineArray.size (); ++i)
        m_lineArray.push_back (new Line (*rhs.m_lineArray[i]));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/06
//---------------------------------------------------------------------------------------
Paragraph::~Paragraph ()
    {
    for (size_t i = 0; i < m_lineArray.size (); ++i)
        delete m_lineArray[i];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void Paragraph::InitFrom (ParagraphCR paragraph)
    {
    m_properties    = paragraph.m_properties;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/07
//---------------------------------------------------------------------------------------
double Paragraph::GetIndentForLine (LineCP line) const
    {
    return ((line == m_lineArray.front ()) ? m_properties.GetIndentation().GetFirstLineIndent () : m_properties.GetIndentation().GetHangingIndent ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void Paragraph::GetParagraphStyle (TextParamWideR textParams) const
    {
    textParams.exFlags.fullJustification    = IsFullJustification () ? true : false;
    textParams.just                         = static_cast<int>(GetJustification ());
    textParams.lineSpacing                  = GetLineSpacing ();
    textParams.exFlags.acadLineSpacingType  = static_cast<UInt32>(GetLineSpacingType ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/05
//---------------------------------------------------------------------------------------
void Paragraph::ExtractNodes (CaretCR caret, TextBlockNodeArrayR unitArray)
    {
    size_t  lineIndex   = caret.GetLineIndex ();
    LineP   line        = GetLine (lineIndex);
    
    if (NULL == line)
        { BeAssert (false && L"Invalid caret."); return; }
    
    size_t extractLineStart;
    
    if (0 == caret.GetRunIndex () && 0 == caret.GetCharacterIndex ())
        {
        extractLineStart = lineIndex;
        }
    else
        {
        line->ExtractNodes (caret, unitArray);
        extractLineStart = lineIndex + 1;
        }

    for (size_t i = extractLineStart; i < GetLineCount (); i++)
        unitArray.push_back (GetLine (i));

    m_lineArray.erase (m_lineArray.begin () + extractLineStart, m_lineArray.begin () + GetLineCount ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/08
//---------------------------------------------------------------------------------------
static void adjustTrailingSpacesForWordWrappedLine (LineR existingLine, LineR newLine, RunR newRun, ProcessContextR processContext)
    {
    // Only allow strict CharStreams (not even child classes).
    if (typeid (newRun) != typeid (CharStream))
        return;
    
    CharStreamP newCharStream = dynamic_cast<CharStreamP>(&newRun);

    // If previous line ended in line feed, then we don't want to move space characters around.
    size_t existingLineRunCount = existingLine.GetRunCount ();
    if (existingLineRunCount > 0 && NULL != dynamic_cast<LineBreakP>(existingLine.GetRun (existingLineRunCount - 1)))
        return;

    HorizontalJustification hJust;
    VerticalJustification   vJust;
    processContext.GetTextBlock ()->GetHorizontalVerticalJustifications (&hJust, &vJust, processContext.GetParagraph ()->GetJustification ());

    if ((!processContext.GetTextBlock ()->GetProperties ().IsVertical () && (HORIZONTAL_JUSTIFICATION_Right == hJust)) ||
        (processContext.GetTextBlock ()->GetProperties ().IsVertical () && ((VERTICAL_JUSTIFICATION_Baseline == vJust) || (VERTICAL_JUSTIFICATION_Descender == vJust))))
        {
        Caret caret = processContext.GetTextBlock ()->End ();
        caret.MoveToPreviousCharacter ();

        size_t  existingLineIndex           = caret.GetLineIndex ();
        size_t  existingLineParagraphIndex  = caret.GetParagraphIndex ();

        bool hasCaretMoved = false;
        while (!caret.IsAtBeginning ()
                && (NULL != dynamic_cast<CharStreamCP>(caret.GetCurrentRunCP ())) // Things like Fraction return 0x20 for GetChar... can't fight that now.
                && (0x20 == caret.GetNextCharacter ())
                && (caret.GetLineIndex () == existingLineIndex)
                && (caret.GetParagraphIndex () == existingLineParagraphIndex))
            {
            caret.MoveToPreviousCharacter ();
            hasCaretMoved = true;
            }

        caret.MoveToNextCharacter ();

        if (!hasCaretMoved || caret.GetLineIndex () != existingLineIndex || caret.GetParagraphIndex () != existingLineParagraphIndex)
            return;

        TextBlockNodeArray carryOverUnits;
        existingLine.ExtractNodes (caret, carryOverUnits);

        // Line::ExtractNodes will only look at run index (which will only look at character index).
        //  This will make a 0, 0, 0, 0 caret, and only the run and character indices are needed.
        Caret newLineBeginExtractCaret = processContext.GetTextBlock ()->Begin ();

        TextBlockNodeArray existingUnits;
        newLine.ExtractNodes (newLineBeginExtractCaret, existingUnits);

        newLine.AppendNodes (carryOverUnits, processContext);
        newLine.AppendNodes (existingUnits, processContext);

        existingLine.ComputeRange (false, processContext.GetParagraph ()->GetLineSpacingType (), processContext.GetTextBlock ()->GetNodeOrFirstRunHeight ());
        newLine.ComputeRange (false, processContext.GetParagraph ()->GetLineSpacingType (), processContext.GetTextBlock ()->GetNodeOrFirstRunHeight ());
        }
    else
        {
        size_t newRunFirstNonSpaceIndex = 0;
        for (; newRunFirstNonSpaceIndex < newRun.GetCharacterCount (); newRunFirstNonSpaceIndex++)
            {
            if (0x20 != newRun.GetCharacter (newRunFirstNonSpaceIndex))
                break;
            }

        if (newRun.GetCharacterCount () == newRunFirstNonSpaceIndex || 0 == newRunFirstNonSpaceIndex)
            return;

        RunP carryOverRun;
        RunP remainingNewRun;
        newRun.Splice (carryOverRun, remainingNewRun, newRunFirstNonSpaceIndex);

        CharStreamP remainingNewCharStream = dynamic_cast<CharStreamP>(remainingNewRun);
        BeAssert (NULL != remainingNewCharStream);
        if (NULL != remainingNewCharStream)
            {
            // Note that carryOverRun was assigned to &newRun by splice, and cannot be deleted.
            // Also note that carryOverRunCopy is freed by AppendNodes.
            RunP carryOverRunCopy = carryOverRun->Clone ();

            TextBlockNodeArray carryOverUnits;
            carryOverUnits.push_back (carryOverRunCopy);

            AppendStatus appendStatus = existingLine.AppendNodes (carryOverUnits, processContext);
            if (APPEND_STATUS_Appended != appendStatus)
                newLine.AppendNodes (carryOverUnits, processContext);

            WString remainingNewRunChars = remainingNewCharStream->GetString ();
            newCharStream->SetString (remainingNewRunChars);

            existingLine.ComputeRange (false, processContext.GetParagraph ()->GetLineSpacingType (), processContext.GetTextBlock ()->GetNodeOrFirstRunHeight ());
            }

        //carryOverRun is &newRun and should not be deleted.
        delete remainingNewRun;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/04
//---------------------------------------------------------------------------------------
AppendStatus Paragraph::AppendNodes (TextBlockNodeArrayR unitArray, ProcessContextR processContext)
    {
    processContext.SetParagraph (this);

    TextBlockCP     textBlock       = processContext.GetTextBlock ();
    AppendStatus    appendStatus    = APPEND_STATUS_Appended;

    // Loop until we have no units left
    while (0 != unitArray.size ())
        {
        LineP   currentLine = m_lineArray.back ();
        LineP   nextLine = NULL;

        if (unitArray[0]->GetUnitLevel () < TEXTBLOCK_NODE_LEVEL_Line)
            break;

        // see if the first unit in the input array is a line
        if (NULL != (nextLine = (dynamic_cast <LineP> (unitArray[0]))))
            {
            // if the current line is complete and we dont have to process words, use the line as such
            if ((NULL == currentLine || currentLine->IsComplete (*nextLine->GetRun (0), processContext)) && processContext.GetProcessLevel () <= PROCESS_LEVEL_Line)
                {
                // calculate a new origin based on the curren line's position and height
                DPoint3d origin;
                if (NULL == currentLine)
                    {
                    memset (&origin, 0, sizeof (origin));
                    }
                else
                    {
                    DVec3d displacement;
                    currentLine->GetBaselineAdjustedOrigin (origin, *textBlock, GetLineSpacingType ());
                    displacement = this->ComputeLineSpacing (textBlock->GetProperties ().IsVertical (), textBlock->GetNodeOrFirstRunHeight (), textBlock->GetProperties ().GetAnnotationScale (), *currentLine, *nextLine);
                    origin.add (&displacement);
                    }

                nextLine->SetBaselineAdjustedOrigin (origin, *textBlock, GetLineSpacingType ());
                adjustTrailingSpacesForWordWrappedLine (*currentLine, *nextLine, *nextLine->GetRun (0), processContext);
                m_lineArray.push_back (nextLine);
                unitArray.erase (unitArray.begin ());
                }
            else
                {
                // The previous line is not complete. Remove the first line in unitArray and then break up the next line and put the words of
                //  of the next line at the beginning of the unitarray and continue processing
                unitArray.erase (unitArray.begin ());
                for (size_t i = 0; i < nextLine->GetRunCount (); i++)
                    unitArray.insert (unitArray.begin () + i, nextLine->GetRun (i));

                nextLine->m_runArray.clear ();
                delete nextLine;

                // need not do anything to the indices
                continue;
                }
            
            // if this line ends in a carriage return, then break out because we should add any more lines to this paragraph
            if (nextLine->EndsInParagraphBreak ())
                {
                appendStatus = APPEND_STATUS_ParagraphBreak;
                break;
                }
            }
        else
            {
            DPoint3d origin;
            currentLine->GetBaselineAdjustedOrigin (origin, *textBlock, GetLineSpacingType ());
            if (currentLine->IsComplete (*((RunP) unitArray[0]), processContext))
                {
                DVec3d displacement;

                // we need to break the line create a new line
                nextLine = new Line ();

                adjustTrailingSpacesForWordWrappedLine (*currentLine, *nextLine, *((RunP) unitArray[0]), processContext);
                m_lineArray.push_back (nextLine);
                displacement = this->ComputeLineSpacing (textBlock->GetProperties ().IsVertical (), textBlock->GetNodeOrFirstRunHeight (), textBlock->GetProperties ().GetAnnotationScale (), *currentLine, *nextLine);
                origin.add (&displacement);
                }
            else
                {
                nextLine = currentLine;
                }
            
            // get the line at the current line index insert units into that line. set its origin to what we have
            appendStatus = nextLine->AppendNodes (unitArray, processContext);

            if (this->GetLineCount () > 1)
                {
                LineCP existingLine = this->GetLine (this->GetLineCount () - 2);
                existingLine->GetBaselineAdjustedOrigin (origin, *textBlock, GetLineSpacingType ());

                DVec3d displacement = this->ComputeLineSpacing (textBlock->GetProperties ().IsVertical (), textBlock->GetNodeOrFirstRunHeight (), textBlock->GetProperties ().GetAnnotationScale (), *existingLine, *nextLine);
                origin.add (&displacement);
                }

            // set the line origin after appending units so that we can compute the baseline displacement correctly
            nextLine->SetBaselineAdjustedOrigin (origin, *textBlock, GetLineSpacingType ());

            if (APPEND_STATUS_ParagraphBreak & appendStatus)
                {
                // no more lines are possible
                break;
                }
            }
        }

    this->ComputeRange (false, textBlock->GetNodeOrFirstRunHeight ());
    processContext.SetParagraph (NULL);
    return appendStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/04
//---------------------------------------------------------------------------------------
LineP Paragraph::GetLine (size_t index) const
    {
    if (index >= m_lineArray.size ())
        return NULL;
    
    return m_lineArray[index];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/05
//---------------------------------------------------------------------------------------
void Paragraph::_Drop (TextBlockNodeArrayR unitArray)
    {
    for (size_t i = 0; i < GetLineCount (); i++)
        {
        TextBlockNodeArray droppedArray;
        GetLine (i)->Drop (droppedArray);
        
        unitArray.insert (unitArray.end (), droppedArray.begin (), droppedArray.end ());
        }
    
    m_lineArray.clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void Paragraph::ComputeRange (bool recomputeComponentRanges, double nodeNumberHeight)
    {
    if (recomputeComponentRanges)
        {
        for (size_t i = 0; i < GetLineCount (); i++)
            {
            LineP line = GetLine (i);
            line->ComputeRange (recomputeComponentRanges, GetLineSpacingType (), nodeNumberHeight);
            }
        }
    
    // If we are in exactly line spacing mode, then the range of each line is computed differently based on acad algorithm

    m_nominalRange.init ();
    for (size_t i = 0; i < GetLineCount (); i++)
        {
        DRange3d lineRange = GetLine (i)->ComputeTransformedNominalRange ();
        ExtendNominalRange (lineRange);
        }

    m_exactRange.init ();
    for (size_t i = 0; i < GetLineCount (); i++)
        {
        DRange3d lineRange = GetLine (i)->ComputeTransformedExactRange ();
        ExtendExactRange (lineRange);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/06
//---------------------------------------------------------------------------------------
void Paragraph::Scale (DPoint2dCR scale, bool vertical)
    {
    m_properties.ApplyScale (scale, vertical);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/07
//---------------------------------------------------------------------------------------
BentleyStatus Paragraph::ComputeCaretAtLocation (CaretR caret, DPoint3dCR locationIn, bool isVertical, bool isStrict) const
    {
    Transform transform = this->GetTransform ();
    transform.inverseOf (&transform);

    DPoint3d location = locationIn;
    transform.multiply (&location);

    for (size_t i = 0; i < this->GetLineCount (); ++i)
        {
        LineP       line    = this->GetLine (i);
        DRange3d    range;
        
        range.Init ();
        line->ComputeTransformedHitTestRange (range);
        
        // Vertical glyphs are centered based on font size. Thus, their range does not necessarily include the origin (in X).
        //  We need to be able to round-trip ComputeCaretParameters with this method, and that method will utilize origins.
        if (isVertical)
            range.Extend (line->GetOrigin ());
        
        bool    containsPoint           = isVertical ? location.x < range.high.x : location.y >= range.low.y;
        bool    dontConsiderNextLine    = false;
        
        if (containsPoint && isStrict)
            {
            containsPoint &= isVertical ? location.x > range.low.x : location.y < range.high.y;
            dontConsiderNextLine = true;
            }
        
        if (containsPoint)
            {
            caret.SetLineIndex (i);
            return line->ComputeCaretAtLocation (caret, location, isVertical, isStrict);
            }
        
        if (dontConsiderNextLine)
            return ERROR;
        }
    
    if (isStrict)
        return ERROR;

    caret.SetLineIndex (m_lineArray.size () - 1);
    return m_lineArray.back ()->ComputeCaretAtLocation (caret, location, isVertical, isStrict);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void Paragraph::ComputeCaretParameters (DPoint3dR location, DVec3dR direction, CaretCR caret, TextBlockCR textBlock) const
    {
    LineCP line = this->GetLine (caret.GetLineIndex ());
    if (NULL != line)
        {
        line->ComputeCaretParameters (location, direction, caret);
        }
    else
        {
        LineCP lastLine = m_lineArray.back ();
        if (NULL == lastLine)
            return;
        
        RunCP lastRunInLastLine = lastLine->GetRun (lastLine->GetRunCount () - 1);
        if (NULL == dynamic_cast<LineBreakCP>(lastRunInLastLine))
            return;
        
        // This means we're off the end of the Paragraph. If we end in a line break, the correct caret location
        //  is actually where the next line should go, thus allowing visualization of appended text.
        Line                trailingLine;
        ProcessContext      context (&textBlock, this, lastLine);
        TextBlockNodeArray  nodes;
        
        // We basically need this for determing caret height, thus I don't think it's terribly important what character we use.
        nodes.push_back (new CharStream (L" ", lastRunInLastLine->GetProperties (), textBlock.ComputeRunLayoutFlags ()));
        
        trailingLine.AppendNodes (nodes, context);
        
        DPoint3d origin;
        lastLine->GetBaselineAdjustedOrigin (origin, textBlock, this->GetLineSpacingType ());
        
        origin.Add (this->ComputeLineSpacing (textBlock.GetProperties ().IsVertical (), textBlock.GetNodeOrFirstRunHeight (), textBlock.GetProperties ().GetAnnotationScale (), *lastLine, trailingLine));
        
        trailingLine.SetBaselineAdjustedOrigin (origin, textBlock, this->GetLineSpacingType ());
        
        trailingLine.ComputeCaretParameters (location, direction, caret);
        }

    DPoint3d origin = this->_GetOrigin ();
    location.sumOf (&location, &origin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/07
//---------------------------------------------------------------------------------------
void Paragraph::Draw (ViewContextR context, bool isViewIndependent, TextBlockDrawOptionsCR options) const
    {
    Transform transform = this->GetTransform ();
    context.PushTransform (transform);
        {
        for (size_t i = 0; i < m_lineArray.size (); ++i)
            m_lineArray[i]->Draw (context, isViewIndependent, options);
        }
    context.PopTransformClip ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
void Paragraph::SetJustification (TextElementJustification just)
    {
    if (m_properties.m_justification == just && m_properties.m_overrides.m_justification)
        return;

    m_properties.m_justification = just;
    m_properties.m_overrides.m_justification = m_properties.HasTextStyle ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
void Paragraph::SetLineSpacingType (DgnLineSpacingType lineSpacingType)
    {
    if (m_properties.m_lineSpacingType == lineSpacingType && m_properties.m_overrides.m_lineSpacingType)
        return;

    m_properties.m_lineSpacingType = lineSpacingType;
    m_properties.m_overrides.m_lineSpacingType = m_properties.HasTextStyle ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
void Paragraph::SetLineSpacing (double lineSpacing)
    {
    if (m_properties.m_lineSpacingValue == lineSpacing && m_properties.m_overrides.m_lineSpacingValue)
        return;

    m_properties.m_lineSpacingValue = lineSpacing;
    m_properties.m_overrides.m_lineSpacingValue = m_properties.HasTextStyle ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
void Paragraph::SetJustificationOverrideFlag (bool overrideFromStyle)
    {
    if (!GetProperties ().HasTextStyle ())
        return;

    DgnTextStylePtr myTextStyle = GetProperties().GetDgnModelR().GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(GetProperties ().m_textStyleId));
    if (!myTextStyle.IsValid ())
        {
        BeAssert (false);
        return;
        }

    m_properties.m_overrides.m_justification = overrideFromStyle;

    if (!overrideFromStyle)
        myTextStyle->GetPropertyValue (DgnTextStyleProperty::Justification, (UInt32&)m_properties.m_justification);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
void Paragraph::SetLineSpacingOverrideFlag (bool overrideFromStyle)
    {
    if (!GetProperties ().HasTextStyle ())
        return;

    DgnTextStylePtr myTextStyle = GetProperties().GetDgnModelR().GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(GetProperties().m_textStyleId));
    if (!myTextStyle.IsValid ())
        {
        BeAssert (false);
        return;
        }

    m_properties.m_overrides.m_lineSpacingValue = overrideFromStyle;

    if (!overrideFromStyle)
        myTextStyle->GetPropertyValue (DgnTextStyleProperty::LineSpacingValueFactor, m_properties.m_lineSpacingValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
void Paragraph::SetLineSpacingTypeOverrideFlag (bool overrideFromStyle)
    {
    if (!GetProperties ().HasTextStyle ())
        return;

    DgnTextStylePtr myTextStyle = GetProperties().GetDgnModelR().GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(GetProperties().m_textStyleId));
    if (!myTextStyle.IsValid ())
        {
        BeAssert (false);
        return;
        }

    m_properties.m_overrides.m_lineSpacingType = overrideFromStyle;

    if (!overrideFromStyle)
        myTextStyle->GetPropertyValue (DgnTextStyleProperty::LineSpacingType, (UInt32&)m_properties.m_lineSpacingType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
bool Paragraph::Equals (ParagraphCR rhs, TextBlockCompareOptionsCR compareOptions) const
    {
    if (!T_Super::Equals (rhs, compareOptions))
        return false;
    
    if (!m_properties.Equals (rhs.m_properties, compareOptions.GetTolerance ()))
        return false;
    
    if (m_lineArray.size () != rhs.m_lineArray.size ())
        return false;
    
    for (LineArray::const_iterator lhsIter = m_lineArray.begin (), rhsIter = rhs.m_lineArray.begin (); m_lineArray.end () != lhsIter; ++lhsIter, ++rhsIter)
        if (!(*lhsIter)->Equals (**rhsIter, compareOptions))
            return false;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
void Paragraph::ComputeTransformedHitTestRange (DRange3dR hitTestRange) const
    {
    FOR_EACH (LineCP line, m_lineArray)
        {
        DRange3d lineRange;
        lineRange.Init ();

        line->ComputeTransformedHitTestRange (lineRange);

        hitTestRange.Extend (&lineRange.low, 2);
        }
    
    this->GetTransform ().Multiply (hitTestRange, hitTestRange);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
void Paragraph::ComputeElementRange (DRange3dR value) const
    {
    value.Init ();

    FOR_EACH (LineCP line, m_lineArray)
        {
        DRange3d lineElementRange;
        line->ComputeElementRange (lineElementRange);

        line->GetTransform ().Multiply (lineElementRange, lineElementRange);

        value.Extend (&lineElementRange.low, 2);
        }
    }
