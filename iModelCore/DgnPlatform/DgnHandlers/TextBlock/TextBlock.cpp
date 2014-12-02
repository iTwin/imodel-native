/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/TextBlock.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <algorithm>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace std;

#define SETMAX(x,y)                 (x > y ? x : y)
#define IS_FRACTION_SEPARATOR(ch)   ((ch == '/') || (ch == '#') || (ch == '^'))
#define IS_FRACTION_END(ch)         ((ch == 0) || (ch == ';'))

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- IDgnTextStyleApplyable ----------------------------------------------------------------------------------------------- IDgnTextStyleApplyable --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool            IDgnTextStyleApplyable::HasTextStyle        () const                                                { return this->_HasTextStyle (); }
UInt32          IDgnTextStyleApplyable::GetTextStyleId      () const                                                { return this->_GetTextStyleId (); }
DgnTextStylePtr IDgnTextStyleApplyable::GetTextStyleInFile  () const                                                { return this->_GetTextStyleInFile (); }
void            IDgnTextStyleApplyable::ToStyle             (DgnTextStyleR style) const                             { this->_ToStyle (style); }
void            IDgnTextStyleApplyable::ApplyTextStyle      (DgnTextStyleCR style, bool respectOverrides)           { this->_ApplyTextStyle (style, respectOverrides); }
void            IDgnTextStyleApplyable::RemoveTextStyle     ()                                                      { this->_RemoveTextStyle (); }
void            IDgnTextStyleApplyable::SetProperties       (DgnTextStyleCR style, DgnTextStylePropertyMaskCR mask)    { this->_SetProperties (style, mask); }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- TextBlockCompareOptions --------------------------------------------------------------------------------------------- TextBlockCompareOptions --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
bool    TextBlockCompareOptions::ShouldIgnoreInternalState          ()  const       { return m_shouldIgnoreInternalState; }
void    TextBlockCompareOptions::SetShouldIgnoreInternalState       (bool val)      { m_shouldIgnoreInternalState = val; }
bool    TextBlockCompareOptions::ShouldIgnoreCachedValues           () const        { return m_shouldIgnoreCachedValues; }
void    TextBlockCompareOptions::SetShouldIgnoreCachedValues        (bool val)      { m_shouldIgnoreCachedValues = val; }
bool    TextBlockCompareOptions::ShouldIgnoreElementOverhead        () const        { return m_shouldIgnoreElementOverhead; }
void    TextBlockCompareOptions::SetShouldIgnoreElementOverhead     (bool val)      { m_shouldIgnoreElementOverhead = val; }
double  TextBlockCompareOptions::GetTolerance                       () const        { return m_tolerance; }
void    TextBlockCompareOptions::SetTolerance                       (double val)    { m_tolerance = val; }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
TextBlockCompareOptions::TextBlockCompareOptions (bool shouldIgnoreInternalState, bool shouldIgnoreCachedValues, bool shouldIgnoreElementOverhead, double tolerance) :
    m_shouldIgnoreInternalState     (shouldIgnoreInternalState),
    m_shouldIgnoreCachedValues      (shouldIgnoreCachedValues),
    m_shouldIgnoreElementOverhead   (shouldIgnoreElementOverhead),
    m_tolerance                     (tolerance)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
TextBlockCompareOptionsPtr TextBlockCompareOptions::CreateForCompareContentAndLocation ()
    {
    return new TextBlockCompareOptions (true, true, true, 0.0);
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
bool TextBlockCompareOptions::AreDoublesEqual (double lhs, double rhs) const
    {
    return (fabs (lhs - rhs) <= m_tolerance);
    }
    
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- TextBlockDrawOptions --------------------------------------------------------------------------------------------------- TextBlockDrawOptions --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
RunCP   TextBlockDrawOptions::GetHighlightRunCP         () const        { return m_highlightRun; }
void    TextBlockDrawOptions::SetHighlightRun           (RunCP value)   { m_highlightRun = value; }
bool    TextBlockDrawOptions::ShouldDrawWhiteSpace      () const        { return m_shouldDrawWhiteSpace; }
void    TextBlockDrawOptions::SetShouldDrawWhiteSpace   (bool value)    { m_shouldDrawWhiteSpace = value; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
TextBlockDrawOptions::TextBlockDrawOptions () :
    m_highlightRun          (NULL),
    m_shouldDrawWhiteSpace  (false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
TextBlockDrawOptionsPtr TextBlockDrawOptions::CreateDefault ()
    {
    return new TextBlockDrawOptions ();
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
TextBlockDrawOptionsPtr TextBlockDrawOptions::Clone () const
    {
    TextBlockDrawOptionsP rhs   = new TextBlockDrawOptions ();
    rhs->m_highlightRun         = m_highlightRun;
    rhs->m_shouldDrawWhiteSpace = m_shouldDrawWhiteSpace;

    return rhs;
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- TextBlockToStringOptions ------------------------------------------------------------------------------------------- TextBlockToStringOptions --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
bool    TextBlockToStringOptions::ShouldSubstitueAtomicRunContent       () const        { return m_shouldSubstitueAtomicRunContent; }
void    TextBlockToStringOptions::ClearShouldSubstitueAtomicRunContent  ()              { m_shouldSubstitueAtomicRunContent = false; }
void    TextBlockToStringOptions::SetAtomicRunContentSubstituteChar     (WChar value)   { m_shouldSubstitueAtomicRunContent = true; m_atomicRunContentSubstituteChar = value; }
WChar   TextBlockToStringOptions::GetAtomicRunContentSubstituteChar     () const        { return m_atomicRunContentSubstituteChar; }
bool    TextBlockToStringOptions::ShouldExpandRscFractions              () const        { return m_shouldExpandRscFractions; }
void    TextBlockToStringOptions::SetShouldExpandRscFractions           (bool value)    { m_shouldExpandRscFractions = value; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
TextBlockToStringOptions::TextBlockToStringOptions () :
    m_shouldSubstitueAtomicRunContent   (false),
    m_atomicRunContentSubstituteChar    (0x0000),
    m_shouldExpandRscFractions          (true)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
TextBlockToStringOptionsPtr TextBlockToStringOptions::CreateDefault ()
    {
    return new TextBlockToStringOptions ();
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
TextBlockToStringOptionsPtr TextBlockToStringOptions::Clone () const
    {
    TextBlockToStringOptionsP rhs           = new TextBlockToStringOptions ();
    rhs->m_shouldSubstitueAtomicRunContent  = m_shouldSubstitueAtomicRunContent;
    rhs->m_atomicRunContentSubstituteChar   = m_atomicRunContentSubstituteChar;

    return rhs;
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- TextBlockProperties::Overrides ------------------------------------------------------------------------------- TextBlockProperties::Overrides --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
TextBlockProperties::Overrides::Overrides ()
    {
    Clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void TextBlockProperties::Overrides::Clear ()
    {
    m_isBackwards           = false;
    m_isUpsideDown          = false;
    m_isVertical            = false;
    m_maxCharactersPerLine  = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
bool TextBlockProperties::Overrides::Equals (Overrides const & rhs) const
    {
    if (m_isBackwards           != rhs.m_isBackwards)           return false;
    if (m_isUpsideDown          != rhs.m_isUpsideDown)          return false;
    if (m_isVertical            != rhs.m_isVertical)            return false;
    if (m_maxCharactersPerLine  != rhs.m_maxCharactersPerLine)  return false;
    
    return true;
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- TextBlockProperties ----------------------------------------------------------------------------------------------------- TextBlockProperties --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2009
//---------------------------------------------------------------------------------------
template<class T>
static void clearPropertyOverride (bool& overrideFlag, T& value, DgnTextStylePtr textStyle, DgnTextStyleProperty tsProp)
    {
    overrideFlag = false;
    
    if (!textStyle.IsValid ())
        return;
    
    textStyle->GetPropertyValue (tsProp, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
IDgnTextStyleApplyable const &  TextBlockProperties::AsIDgnTextStyleApplyable           () const                            { return *this; }
IDgnTextStyleApplyable&         TextBlockProperties::AsIDgnTextStyleApplyableR          ()                                  { return *this; }
TextBlockPropertiesPtr          TextBlockProperties::Clone                              () const                            { return new TextBlockProperties (*this); }
DgnModelR                       TextBlockProperties::GetDgnModelR                       () const                            { return *m_dgnModel; }
bool                            TextBlockProperties::HasAnnotationScale                 () const                            { return m_hasAnnotationScale; }
void                            TextBlockProperties::ClearAnnotationScale               ()                                  { m_hasAnnotationScale = false; m_annotationScale = 1.0; }
double                          TextBlockProperties::GetAnnotationScale                 () const                            { return m_annotationScale; }
void                            TextBlockProperties::SetAnnotationScale                 (double value)                      { m_annotationScale = value; m_hasAnnotationScale = true; }
bool                            TextBlockProperties::IsBackwards                        () const                            { return m_isBackwards; }
void                            TextBlockProperties::SetIsBackwards                     (bool value)                        { m_isBackwards = value; if (this->HasTextStyle ()) m_overrides.m_isBackwards = true; }
bool                            TextBlockProperties::IsBackwardsOverridden              () const                            { return m_overrides.m_isBackwards; }
void                            TextBlockProperties::ClearIsBackwardsOverride           ()                                  { clearPropertyOverride (m_overrides.m_isBackwards, m_isBackwards, this->GetTextStyleInFile (), DgnTextStyleProperty::IsBackwards); }
bool                            TextBlockProperties::IsUpsideDown                       () const                            { return m_isUpsideDown; }
void                            TextBlockProperties::SetIsUpsideDown                    (bool value)                        { m_isUpsideDown = value; if (this->HasTextStyle ()) m_overrides.m_isUpsideDown = true; }
bool                            TextBlockProperties::IsUpsideDownOverridden             () const                            { return m_overrides.m_isUpsideDown; }
void                            TextBlockProperties::ClearIsUpsideDownOverride          ()                                  { clearPropertyOverride (m_overrides.m_isUpsideDown, m_isUpsideDown, this->GetTextStyleInFile (), DgnTextStyleProperty::IsUpsideDown); }
bool                            TextBlockProperties::IsViewIndependent                  () const                            { return m_isViewIndependent; }
void                            TextBlockProperties::SetIsViewIndependent               (bool value)                        { m_isViewIndependent = value; }
bool                            TextBlockProperties::IsVertical                         () const                            { return m_isVertical; }
void                            TextBlockProperties::SetIsVertical                      (bool value)                        { m_isVertical = value; if (this->HasTextStyle ()) m_overrides.m_isVertical = true; }
bool                            TextBlockProperties::IsVerticalOverridden               () const                            { return m_overrides.m_isVertical; }
void                            TextBlockProperties::ClearIsVerticalOverride            ()                                  { clearPropertyOverride (m_overrides.m_isVertical, m_isVertical, this->GetTextStyleInFile (), DgnTextStyleProperty::IsVertical); }
bool                            TextBlockProperties::IsFitted                           () const                            { return m_isFitted; }
void                            TextBlockProperties::SetIsFitted                        (bool value)                        { m_isFitted = value; }
UInt32                          TextBlockProperties::GetMaxCharactersPerLine            () const                            { return m_maxCharactersPerLine; }
void                            TextBlockProperties::SetMaxCharactersPerLine            (UInt32 value)                      { m_maxCharactersPerLine = value; if (this->HasTextStyle ()) m_overrides.m_maxCharactersPerLine = true; }
bool                            TextBlockProperties::IsMaxCharactersPerLineOverridden   () const                            { return m_overrides.m_maxCharactersPerLine; }
void                            TextBlockProperties::ClearMaxCharactersPerLineOverride  ()                                  { clearPropertyOverride (m_overrides.m_maxCharactersPerLine, m_maxCharactersPerLine, this->GetTextStyleInFile (), DgnTextStyleProperty::MaxCharactersPerLine); }
double                          TextBlockProperties::GetDocumentWidth                   () const                            { return m_documentWidth; }
void                            TextBlockProperties::SetDocumentWidth                   (double value)                      { m_documentWidth = value; }
bool                            TextBlockProperties::_HasTextStyle                      () const                            { return (0 != m_textStyleId && m_dgnModel->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(m_textStyleId)).IsValid ()); }
UInt32                          TextBlockProperties::_GetTextStyleId                    () const                            { return m_textStyleId; }
DgnTextStylePtr                 TextBlockProperties::_GetTextStyleInFile                () const                            { return m_dgnModel->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(m_textStyleId)); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
TextBlockPropertiesPtr TextBlockProperties::Create (DgnModelR dgnCache) { return new TextBlockProperties (dgnCache); }
TextBlockProperties::TextBlockProperties (DgnModelR dgnCache) :
    RefCountedBase ()
    {
    this->InitDefaults (dgnCache);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
TextBlockPropertiesPtr TextBlockProperties::Create (TextParamWideCR params, DPoint2dCR fontSize, UInt32 maxCharactersPerLine, DgnModelR dgnCache) { return new TextBlockProperties (params, fontSize, maxCharactersPerLine, dgnCache); }
TextBlockProperties::TextBlockProperties (TextParamWideCR params, DPoint2dCR fontSize, UInt32 maxCharactersPerLine, DgnModelR dgnCache) :
    RefCountedBase (),
    m_dgnModel (&dgnCache)
    {
    this->FromElementData (params, fontSize, maxCharactersPerLine);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
TextBlockPropertiesPtr TextBlockProperties::Create (DgnTextStyleCR textStyle, DgnModelR dgnCache) { return new TextBlockProperties (textStyle, dgnCache); }
TextBlockProperties::TextBlockProperties (DgnTextStyleCR textStyle, DgnModelR dgnCache) :
    RefCountedBase ()
    {
    this->InitDefaults (dgnCache);
    this->ApplyTextStyle (textStyle, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2009
//---------------------------------------------------------------------------------------
TextBlockProperties::TextBlockProperties (TextBlockPropertiesCR rhs) :
    RefCountedBase (),
    m_dgnModel              (rhs.m_dgnModel),
    m_overrides             (rhs.m_overrides),
    m_hasAnnotationScale    (rhs.m_hasAnnotationScale),
    m_annotationScale       (rhs.m_annotationScale),
    m_isBackwards           (rhs.m_isBackwards),
    m_isUpsideDown          (rhs.m_isUpsideDown),
    m_isViewIndependent     (rhs.m_isViewIndependent),
    m_isVertical            (rhs.m_isVertical),
    m_isFitted              (rhs.m_isFitted),
    m_maxCharactersPerLine  (rhs.m_maxCharactersPerLine),
    m_documentWidth         (rhs.m_documentWidth),
    m_textStyleId           (rhs.m_textStyleId)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void TextBlockProperties::InitDefaults (DgnModelR dgnCache)
    {
    m_dgnModel              = &dgnCache;
#ifdef WIP_V10_MODELINFO
    m_hasAnnotationScale    = TO_BOOL (dgnModel_getModelFlag (m_dgnModel, MODELFLAG_USE_ANNOTATION_SCALE));
    m_annotationScale       = dgnModel_getEffectiveAnnotationScale (m_dgnModel);
#else
    m_hasAnnotationScale = false;
    m_annotationScale = 1.0;
#endif
    m_isBackwards           = false;
    m_isUpsideDown          = false;
    m_isViewIndependent     = false;
    m_isVertical            = false;
    m_isFitted              = false;
    m_maxCharactersPerLine  = 0;
    m_documentWidth         = 0.0;
    m_textStyleId           = 0;
    
    // TextBlockProperties::Overrides constructor zeros itself out; depend on that.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void TextBlockProperties::ToElementData (TextParamWideR params, UInt32* maxCharactersPerLine) const
    {
    // Of the three parameter structures that have to co-exist in the TextParamWide (RunProperties, ParagraphProperties, TextBlockProperties),
    //  RunProperties is the only one that actually gets to define the text style. During export, it fills in the parameters first, and we
    //  cannot attempt to posthumously compute override flags.
    
    params.exFlags.annotationScale  =   m_hasAnnotationScale;
    params.annotationScale          =   m_annotationScale;
    params.exFlags.backwards        =   m_isBackwards;
    params.exFlags.upsidedown       =   m_isUpsideDown;
    params.viewIndependent          =   m_isViewIndependent;
    params.flags.vertical           =   m_isVertical;
    params.exFlags.acadFittedText   =   m_isFitted;
    params.textnodeWordWrapLength   =   m_documentWidth;
    
    if (NULL != maxCharactersPerLine)
        *maxCharactersPerLine = m_maxCharactersPerLine;
    
    DgnTextStylePtr paramsTextStyle = m_dgnModel->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(params.textStyleId));
    
    // Params don't have a style? Don't set overrides or style.
    if (!params.flags.textStyle || !paramsTextStyle.IsValid ())
        return;
    
    // Same style? Set overrides as stored.
    if (this->HasTextStyle () && (m_textStyleId == params.textStyleId))
        {
        params.overridesFromStyle.backwards     =   m_overrides.m_isBackwards;
        params.overridesFromStyle.upsidedown    =   m_overrides.m_isUpsideDown;
        params.overridesFromStyle.direction     |=  m_overrides.m_isVertical;
        params.overridesFromStyle.lineLength    =   m_overrides.m_maxCharactersPerLine;
        
        return;
        }
    
    // Otherwise params have a different style, configure overrides as needed.
    bool    paramsTextStyle_isBackwards;            paramsTextStyle->GetPropertyValue (DgnTextStyleProperty::IsBackwards, paramsTextStyle_isBackwards);
    bool    paramsTextStyle_isUpsideDown;           paramsTextStyle->GetPropertyValue (DgnTextStyleProperty::IsUpsideDown, paramsTextStyle_isUpsideDown);
    bool    paramsTextStyle_isVertical;             paramsTextStyle->GetPropertyValue (DgnTextStyleProperty::IsVertical, paramsTextStyle_isVertical);
    UInt32  paramsTextStyle_maxCharactersPerLine;   paramsTextStyle->GetPropertyValue (DgnTextStyleProperty::MaxCharactersPerLine, paramsTextStyle_maxCharactersPerLine);
    
    params.overridesFromStyle.backwards     =   m_overrides.m_isBackwards           || (m_isBackwards != paramsTextStyle_isBackwards);
    params.overridesFromStyle.upsidedown    =   m_overrides.m_isUpsideDown          || (m_isUpsideDown != paramsTextStyle_isUpsideDown);
    params.overridesFromStyle.direction     |=  m_overrides.m_isVertical            || (m_isVertical != paramsTextStyle_isVertical);
    params.overridesFromStyle.lineLength    =   m_overrides.m_maxCharactersPerLine  || (m_maxCharactersPerLine != paramsTextStyle_maxCharactersPerLine);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void TextBlockProperties::FromElementData (TextParamWideCR params, DPoint2dCR fontSize, UInt32 maxCharactersPerLine)
    {
    m_overrides.m_isBackwards           = params.overridesFromStyle.backwards;
    m_overrides.m_isUpsideDown          = params.overridesFromStyle.upsidedown;
    m_overrides.m_maxCharactersPerLine  = params.overridesFromStyle.lineLength;

    m_hasAnnotationScale                = params.exFlags.annotationScale;
    m_annotationScale                   = params.annotationScale;
    m_isBackwards                       = (params.exFlags.backwards || fontSize.x < 0.0);
    m_isUpsideDown                      = (params.exFlags.upsidedown || fontSize.y < 0.0);
    m_isViewIndependent                 = TO_BOOL (params.viewIndependent);
    m_isVertical                        = params.flags.vertical;
    m_isFitted                          = params.exFlags.acadFittedText;
    m_maxCharactersPerLine              = maxCharactersPerLine;
    m_documentWidth                     = params.textnodeWordWrapLength;
    
    DgnTextStylePtr textStyle = (params.flags.textStyle && (0 != params.textStyleId)) ? m_dgnModel->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(params.textStyleId)) : NULL;
    if (textStyle.IsValid ())
        {
        bool isTextStyleVertical;
        textStyle->GetPropertyValue (DgnTextStyleProperty::IsVertical, isTextStyleVertical);
        
        m_overrides.m_isVertical = (params.flags.vertical != isTextStyleVertical);
        m_textStyleId = params.textStyleId;
        }
    else
        {
        m_overrides.m_isVertical = false; // Persisted overrides do not have an override bit solely for vertical; derive from a style if present.
        m_textStyleId = 0;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void TextBlockProperties::_ToStyle (DgnTextStyleR style) const
    {
    style.SetPropertyValue (DgnTextStyleProperty::IsBackwards,         (bool)m_isBackwards);
    style.SetPropertyValue(DgnTextStyleProperty::IsUpsideDown, (bool) m_isUpsideDown);
    style.SetPropertyValue(DgnTextStyleProperty::IsVertical, (bool) m_isVertical);
    style.SetPropertyValue(DgnTextStyleProperty::MaxCharactersPerLine, (UInt32) m_maxCharactersPerLine);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
bool TextBlockProperties::Equals (TextBlockPropertiesCR rhs) const { return this->Equals (rhs, 0.0); }
bool TextBlockProperties::Equals (TextBlockPropertiesCR rhs, double tolerance) const
    {
    // Purposefully not comparing DgnModels... it would be a lot of extra work to remap everything, and I just don't think it will affect many cases.
    //  Even in the QATools case where we have different DgnModels, they should still have the same units and color table etc...
    
    if (m_hasAnnotationScale    != rhs.m_hasAnnotationScale)    return false;
    if (m_isBackwards           != rhs.m_isBackwards)           return false;
    if (m_isUpsideDown          != rhs.m_isUpsideDown)          return false;
    if (m_isViewIndependent     != rhs.m_isViewIndependent)     return false;
    if (m_isVertical            != rhs.m_isVertical)            return false;
    if (m_isFitted              != rhs.m_isFitted)              return false;
    if (m_maxCharactersPerLine  != rhs.m_maxCharactersPerLine)  return false;
    if (m_textStyleId           != rhs.m_textStyleId)           return false;
    
    if (fabs (m_annotationScale - rhs.m_annotationScale) > tolerance)   return false;
    if (fabs (m_documentWidth - rhs.m_documentWidth) > tolerance)       return false;
    
    if (!m_overrides.Equals (rhs.m_overrides)) return false;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
void TextBlockProperties::_RemoveTextStyle ()
    {
    m_overrides.Clear ();
    m_textStyleId = 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
void TextBlockProperties::SetPropertiesFromStyle (DgnTextStylePropertyMaskCR applyMask, DgnTextStyleCR newStyle, DgnTextStylePropertyMaskCR overridesMask)
    {
    if (applyMask.IsPropertySet (DgnTextStyleProperty::IsBackwards))            {   newStyle.GetPropertyValue (DgnTextStyleProperty::IsBackwards,          m_isBackwards);                 m_overrides.m_isBackwards           = overridesMask.IsPropertySet (DgnTextStyleProperty::IsBackwards);          }
    if (applyMask.IsPropertySet(DgnTextStyleProperty::IsUpsideDown))           { newStyle.GetPropertyValue(DgnTextStyleProperty::IsUpsideDown, m_isUpsideDown);                m_overrides.m_isUpsideDown = overridesMask.IsPropertySet(DgnTextStyleProperty::IsUpsideDown); }
    if (applyMask.IsPropertySet(DgnTextStyleProperty::IsVertical))             { newStyle.GetPropertyValue(DgnTextStyleProperty::IsVertical, m_isVertical);                  m_overrides.m_isVertical = overridesMask.IsPropertySet(DgnTextStyleProperty::IsVertical); }
    if (applyMask.IsPropertySet(DgnTextStyleProperty::MaxCharactersPerLine))           { newStyle.GetPropertyValue(DgnTextStyleProperty::MaxCharactersPerLine, m_maxCharactersPerLine);        m_overrides.m_maxCharactersPerLine = overridesMask.IsPropertySet(DgnTextStyleProperty::MaxCharactersPerLine); }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void TextBlockProperties::_ApplyTextStyle (DgnTextStyleCR newStyle, bool respectOverrides)
    {
    DgnTextStylePtr             fileStyle       = newStyle.GetProject().Styles().TextStyles().QueryByName(newStyle.GetName ().c_str ());
    DgnTextStylePropertyMaskPtr    overridesMask   = fileStyle.IsValid () ? newStyle.Compare (*fileStyle) : DgnTextStylePropertyMask::Create();
    
    if (fileStyle.IsNull () || !respectOverrides)
        m_overrides.Clear ();
    
    DgnTextStylePropertyMaskPtr applyMask = DgnTextStylePropertyMask::Create();
    if (!m_overrides.m_isBackwards)             applyMask->SetProperty(DgnTextStyleProperty::IsBackwards,            true);
    if (!m_overrides.m_isUpsideDown)            applyMask->SetProperty(DgnTextStyleProperty::IsUpsideDown,           true);
    if (!m_overrides.m_isVertical)              applyMask->SetProperty(DgnTextStyleProperty::IsVertical,             true);
    if (!m_overrides.m_maxCharactersPerLine)    applyMask->SetProperty(DgnTextStyleProperty::MaxCharactersPerLine,           true);
    
    this->SetPropertiesFromStyle (*applyMask, newStyle, *overridesMask);
    
    m_textStyleId = fileStyle.IsValid () ? (UInt32)fileStyle->GetId ().GetValue() : 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
void TextBlockProperties::_SetProperties (DgnTextStyleCR newStyle, DgnTextStylePropertyMaskCR applyMask)
    {
    DgnTextStylePropertyMaskPtr overridesMask = DgnTextStylePropertyMask::Create();
    if (this->HasTextStyle ())
        overridesMask->SetAllProperties (true);
    
    this->SetPropertiesFromStyle (applyMask, newStyle, *overridesMask);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
void TextBlockProperties::ApplyScale (DPoint2dCR scaleFactor)
    {
    if (m_isVertical)
        m_documentWidth *= scaleFactor.y;
    else
        m_documentWidth *= scaleFactor.x;
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- TextBlock ------------------------------------------------------------------------------------------------------------------------- TextBlock --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void            TextBlock::SetLineSpacingType                               (DgnLineSpacingType lineSpacingType, ParagraphRangeCR range)        { SetParagraphProperty (&Paragraph::SetLineSpacingType, lineSpacingType, range); }
void            TextBlock::SetLineSpacingTypeOverrideFlag                   (bool overrideFromStyle, ParagraphRangeCR range)                 { SetParagraphProperty (&Paragraph::SetLineSpacingTypeOverrideFlag, overrideFromStyle, range); }
void            TextBlock::SetLineSpacingValue                              (double exactLineSpacing, ParagraphRangeCR range)                { SetParagraphProperty (&Paragraph::SetLineSpacing, exactLineSpacing, range); }
void            TextBlock::SetLineSpacingValueOverrideFlag                  (bool overrideFromStyle, ParagraphRangeCR range)                 { SetParagraphProperty (&Paragraph::SetLineSpacingOverrideFlag, overrideFromStyle, range); }
void            TextBlock::SetJustification                                 (TextElementJustification justification, ParagraphRangeCR range) { SetParagraphProperty (&Paragraph::SetJustification, justification, range); }
void            TextBlock::SetJustificationOverrideFlag                     (bool overrideFromStyle, ParagraphRangeCR range)                 { SetParagraphProperty (&Paragraph::SetJustificationOverrideFlag, overrideFromStyle, range); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void                        TextBlock::SetV7Compatible                      (bool V7Compatible)             { m_V7Compatible = V7Compatible; }
void                        TextBlock::SetForceTextNodeFlag                 (bool forceTextNode)            { m_forceTextNode = forceTextNode; }
void                        TextBlock::SetType                              (TextBlockType type)            { m_type = type; }
RotMatrix                   TextBlock::GetOrientation                       () const                        { return m_orientation; }
bool                        TextBlock::GetForceTextNodeFlag                 () const                        { return m_forceTextNode; }
bool                        TextBlock::IsV7Compatible                       () const                        { return m_V7Compatible; }
TextBlockType               TextBlock::GetType                              () const                        { return m_type; }
RunPropertiesCP             TextBlock::GetTextNodeRunProperties             () const                        { return m_nodeProperties; }
bool                        TextBlock::IsDGNType                            () const                        { return TEXTBLOCK_TYPE_Dgn == m_type; }
bool                        TextBlock::IsMTextType                          () const                        { return TEXTBLOCK_TYPE_DwgMText == m_type; }
bool                        TextBlock::IsDTextType                          () const                        { return TEXTBLOCK_TYPE_DwgDText == m_type; }
void                        TextBlock::SetDescrOffset                       (double offset)                 { m_descrOffset = offset; }
void                        TextBlock::SetDescrSide                         (DescriptorSide iAbove)         { m_descrSide = iAbove; }
void                        TextBlock::SetDescrStartPoint                   (DPoint3dCR startPoint)         { m_descrStartPoint = startPoint; }
void                        TextBlock::SetDescrDistance                     (double distance)               { m_alongDist = distance; m_useAlongDist = true; }
void                        TextBlock::SetViewIndependent                   (bool viewIndependent)          { m_properties.SetIsViewIndependent (viewIndependent); }
void                        TextBlock::SetNodeNumber                        (UInt32 nodeNumber)             { m_nodeNumber = nodeNumber; }
void                        TextBlock::SetTextOrigin                        (DPoint3dCR origin)             { m_origin = origin; m_primaryOriginType = ORIGIN_TYPE_TextBlock; }
void                        TextBlock::SetTextElementOrigin                 (DPoint3dCR origin)             { m_primaryOrigin = origin; m_primaryOriginType = ORIGIN_TYPE_Element; }
void                        TextBlock::SetUserOrigin                        (DPoint3dCR origin)             { m_primaryOrigin = origin; m_primaryOriginType = ORIGIN_TYPE_User; }
void                        TextBlock::SetTextAutoCADOrigin                 (DPoint3dCR origin)             { m_primaryOrigin = origin; m_primaryOriginType = ORIGIN_TYPE_AutoCAD; }
void                        TextBlock::SetOrientation                       (RotMatrixCR rMatrix)           { m_orientation = rMatrix; }
TextBlockPropertiesCR       TextBlock::GetProperties                        () const                        { return m_properties; }
UInt32                      TextBlock::GetNodeNumber                        () const                        { return m_nodeNumber; }
DgnModelR                   TextBlock::GetDgnModelR                         () const                        { return *m_dgnModel; }
UInt32                      TextBlock::GetParagraphCount                    () const                        { return static_cast<UInt32>(m_paragraphArray.size ()); }
DgnTextStylePtr             TextBlock::GetTextStyle                         () const                        { return m_properties.GetDgnModelR().GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(m_properties.GetTextStyleId ())); }
void                        TextBlock::SetProcessLevel                      (ProcessLevel processLevel)     { m_processLevel = processLevel; }
DPoint3d                    TextBlock::GetTextOrigin                        () const                        { return GetOrigin (); }
TextBlockPtr                TextBlock::Clone                                () const                        { return new TextBlock (*this); }
DRange3d                    TextBlock::ComputeJustificationRange            () const                        { return TextBlockUtilities::ComputeJustificationRange (m_nominalRange, m_exactRange); }
ParagraphPropertiesCP       TextBlock::GetParagraphProperties               (size_t index) const            { return (index < m_paragraphArray.size ()) ? &m_paragraphArray[index]->GetProperties () : NULL; }
double                      TextBlock::GetNominalWidth                      () const                        { return (m_nominalRange.high.x < m_nominalRange.low.x) ? 0.0 : m_nominalRange.high.x - m_nominalRange.low.x; }
double                      TextBlock::GetExactWidth                        () const                        { return (m_exactRange.high.x < m_exactRange.low.x) ? 0.0 : m_exactRange.high.x - m_exactRange.low.x; }
double                      TextBlock::GetNominalHeight                     () const                        { return (m_nominalRange.high.y < m_nominalRange.low.y) ? 0.0 : m_nominalRange.high.y - m_nominalRange.low.y; }
double                      TextBlock::GetExactHeight                       () const                        { return (m_exactRange.high.y < m_exactRange.low.y) ? 0.0 : m_exactRange.high.y - m_exactRange.low.y; }
DRange3d                    TextBlock::GetNominalRange                      () const                        { return m_nominalRange; }
DRange3d                    TextBlock::GetExactRange                        () const                        { return m_exactRange; }
OriginType                  TextBlock::GetOriginType                        () const                        { return m_primaryOriginType; }
RunPropertiesP              TextBlock::GetNodeProperties                    () const                        { return m_nodeProperties; }
ParagraphPropertiesCR       TextBlock::GetParagraphPropertiesForAdd         () const                        { return m_paragraphPropertiesForAdd; }
ParagraphPropertiesR        TextBlock::GetParagraphPropertiesForAddR        ()                              { return m_paragraphPropertiesForAdd; }
void                        TextBlock::SetParagraphPropertiesForAdd         (ParagraphPropertiesCR value)   { m_paragraphPropertiesForAdd = value; }
RunPropertiesCR             TextBlock::GetRunPropertiesForAdd               () const                        { return m_runPropertiesForAdd; }
RunPropertiesR              TextBlock::GetRunPropertiesForAddR              ()                              { return m_runPropertiesForAdd; }
void                        TextBlock::SetRunPropertiesForAdd               (RunPropertiesCR value)         { m_runPropertiesForAdd = value; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/06
//---------------------------------------------------------------------------------------
void TextBlock::ComputeLocalCaretParameters (DPoint3dR location, DVec3dR direction, CaretCR caret) const
    {
    location.Zero ();
    direction.Zero ();

    RunCP   caretRun                    = caret.GetCurrentRunCP ();
    bool    isCaretRunLineBreak         = (NULL != dynamic_cast<LineBreakCP>(caretRun));
    bool    isCaretRunParagraphBreak    = (NULL != dynamic_cast<ParagraphBreakCP>(caretRun));
    
    if (!this->IsEmpty () && (!caret.IsAtEnd () || (!isCaretRunLineBreak && !isCaretRunParagraphBreak)))
        {
        caret.GetCurrentParagraphCP ()->ComputeCaretParameters (location, direction, caret, *this);
        return;
        }

    // This means we're off the end of the TextBlock.
    //  If we end in a line or paragraph break, the correct caret location is actually where the next line should go, thus allowing visualization of appended text.
    Paragraph trailingParagraph (*m_dgnModel);
    trailingParagraph.SetProperties (m_paragraphPropertiesForAdd);
    this->SetOriginForNextParagraph (&trailingParagraph, caret.GetCurrentParagraphCP ());
    
    ProcessContext      context (this, &trailingParagraph, NULL);
    TextBlockNodeArray  nodes;
    
    // We basically need this for determing caret height, thus I don't think it's terribly important what character we use.
    nodes.push_back (new CharStream (L"A", m_runPropertiesForAdd, this->ComputeRunLayoutFlags ()));
    
    trailingParagraph.AppendNodes (nodes, context);
    
    Caret danglingCaret (*this);
    danglingCaret.SetParagraphIndex (m_paragraphArray.size ());
    trailingParagraph.ComputeCaretParameters (location, direction, danglingCaret, *this);

    if (m_properties.IsVertical ())
        return;
        
    HorizontalJustification hJust;
    TextBlock::GetHorizontalVerticalJustifications (&hJust, NULL, trailingParagraph.GetProperties ().GetJustification ());
    if (HORIZONTAL_JUSTIFICATION_Left == hJust)
        return;
    
    DRange3d jRange = this->ComputeJustificationRange ();
    if (jRange.IsNull ())
        return;

    switch (hJust)
        {
        case HORIZONTAL_JUSTIFICATION_Center:   location.x += ((jRange.high.x - jRange.low.x) / 2.0);   break;
        case HORIZONTAL_JUSTIFICATION_Right:    location.x += (jRange.high.x - jRange.low.x);           break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/08
//---------------------------------------------------------------------------------------
void TextBlock::ComputeCaretParameters (DPoint3dR location, DVec3dR direction, CaretCR caret) const
    {
    this->ComputeLocalCaretParameters (location, direction, caret);

    Transform textBlockToWorld = this->GetTransform ();
    textBlockToWorld.multiply (&location);

    RotMatrix textBlockToWorldRotation;
    textBlockToWorld.getMatrix (&textBlockToWorldRotation);
    textBlockToWorldRotation.multiply (&direction);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void TextBlock::SetProperties (TextBlockPropertiesCR value)
    {
    if ((value.HasAnnotationScale () != m_properties.HasAnnotationScale ()) || (m_properties.HasAnnotationScale () && (value.GetAnnotationScale () != m_properties.GetAnnotationScale ())))
        this->ApplyAnnotationScale (value.GetAnnotationScale (), value.HasAnnotationScale ());
    
    m_properties    = value;
    m_dirty         = this->Begin ();
    m_processLevel  = PROCESS_LEVEL_Character;
    
    DgnGlyphRunLayoutFlags layoutFlags = this->ComputeRunLayoutFlags ();
    
    RunRange runRange (*this);
    FOR_EACH (RunCR constRun , runRange)
        const_cast<RunR>(constRun).SetRunLayoutFlags (layoutFlags);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void TextBlock::SetIsVertical (bool vertical)
    {
    if (vertical == m_properties.IsVertical () && m_properties.m_overrides.m_isVertical)
        return;

    m_dirty                                 = this->Begin ();
    m_processLevel                          = PROCESS_LEVEL_Character;
    m_properties.SetIsVertical (vertical);
    m_properties.m_overrides.m_isVertical   = m_properties.HasTextStyle ();

    RunRange runRange (*this);
    FOR_EACH (RunCR constRun, runRange)
        const_cast<RunR>(constRun).SetIsVertical (vertical);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
void TextBlock::SetVerticalOverrideFlag (bool overrideFromStyle)
    {
    if (!this->GetProperties ().HasTextStyle ())
        return;

    DgnTextStylePtr myTextStyle = GetTextStyle ();
    if (!myTextStyle.IsValid ())
        {
        BeAssert (false);
        return;
        }

    m_properties.m_overrides.m_isVertical = overrideFromStyle;

    if (!overrideFromStyle)
        {
        bool isVertical;
        myTextStyle->GetPropertyValue (DgnTextStyleProperty::IsVertical, isVertical);
        
        if (m_properties.IsVertical () != isVertical)
            {
            TextBlockPropertiesPtr newProps = m_properties.Clone ();
            newProps->SetIsVertical (isVertical);
            
            this->SetProperties (*newProps);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void TextBlock::SetMaxCharactersPerLine (UInt16 maxUnitsPerLine)
    {
    if (m_properties.GetMaxCharactersPerLine () == maxUnitsPerLine && m_properties.m_overrides.m_maxCharactersPerLine)
        return;

    m_dirty                                         = this->Begin ();
    m_processLevel                                  = PROCESS_LEVEL_Character;
    m_properties.SetMaxCharactersPerLine (maxUnitsPerLine);
    m_properties.m_overrides.m_maxCharactersPerLine = m_properties.HasTextStyle ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
void TextBlock::SetMaxCharactersPerLineOverrideFlag (bool overrideFromStyle)
    {
    if (!this->GetProperties ().HasTextStyle ())
        return;

    DgnTextStylePtr myTextStyle = GetTextStyle ();
    if (!myTextStyle.IsValid ())
        {
        BeAssert (false);
        return;
        }

    m_properties.m_overrides.m_maxCharactersPerLine = overrideFromStyle;

    if (!overrideFromStyle)
        {
        UInt32 maxCharactersPerLine;
        myTextStyle->GetPropertyValue (DgnTextStyleProperty::MaxCharactersPerLine, maxCharactersPerLine);
        m_properties.SetMaxCharactersPerLine (maxCharactersPerLine);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void TextBlock::SetLineBreakLength (double lineBreakLength)
    {
    if (m_properties.GetDocumentWidth () == lineBreakLength)
        return;

    m_dirty = this->Begin ();
    SetProcessLevel (PROCESS_LEVEL_Character);
    m_properties.SetDocumentWidth (lineBreakLength);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void TextBlock::SetIsUpsideDown (bool upsideDown)
    {
    if (m_properties.IsUpsideDown () == upsideDown && m_properties.m_overrides.m_isUpsideDown)
        return;

    m_dirty                                 = this->Begin ();
    m_processLevel                          = PROCESS_LEVEL_Character;
    m_properties.SetIsUpsideDown (upsideDown);
    m_properties.m_overrides.m_isUpsideDown = m_properties.HasTextStyle ();
    
    RunRange runRange (*this);
    FOR_EACH (RunCR constRun, runRange)
        const_cast<RunR>(constRun).SetIsUpsideDown (upsideDown);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
void TextBlock::SetUpsideDownOverrideFlag (bool overrideFromStyle)
    {
    if (!this->GetProperties ().HasTextStyle ())
        return;

    DgnTextStylePtr myTextStyle = GetTextStyle ();
    if (!myTextStyle.IsValid ())
        {
        BeAssert (false);
        return;
        }

    m_properties.m_overrides.m_isUpsideDown = overrideFromStyle;

    if (!overrideFromStyle)
        {
        bool isUpsideDown;
        myTextStyle->GetPropertyValue (DgnTextStyleProperty::IsUpsideDown, isUpsideDown);
        
        if (m_properties.IsUpsideDown () != isUpsideDown)
            {
            TextBlockPropertiesPtr newProps = m_properties.Clone ();
            newProps->SetIsUpsideDown (isUpsideDown);
            
            this->SetProperties (*newProps);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void TextBlock::SetIsBackwards (bool backwards)
    {
    if (m_properties.IsBackwards () == backwards && m_properties.m_overrides.m_isBackwards)
        return;

    m_dirty                                 = this->Begin ();
    m_processLevel                          = PROCESS_LEVEL_Character;
    m_properties.SetIsBackwards (backwards);
    m_properties.m_overrides.m_isBackwards  = m_properties.HasTextStyle ();

    RunRange runRange (*this);
    FOR_EACH (RunCR constRun, runRange)
        const_cast<RunR>(constRun).SetIsBackwards (backwards);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
void TextBlock::SetBackwardsOverrideFlag (bool overrideFromStyle)
    {
    if (!this->GetProperties ().HasTextStyle ())
        return;

    DgnTextStylePtr myTextStyle = GetTextStyle ();
    if (!myTextStyle.IsValid ())
        {
        BeAssert (false);
        return;
        }

    m_properties.m_overrides.m_isBackwards = overrideFromStyle;

    if (!overrideFromStyle)
        {
        bool isBackwards;
        myTextStyle->GetPropertyValue (DgnTextStyleProperty::IsBackwards, isBackwards);
        
        if (m_properties.IsBackwards () != isBackwards)
            {
            TextBlockPropertiesPtr newProps = m_properties.Clone ();
            newProps->SetIsBackwards (isBackwards);
            
            this->SetProperties (*newProps);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/05
//---------------------------------------------------------------------------------------
void TextBlock::InitDefaults (DgnModelR dgnCache)
    {
    m_dgnModel                  = &dgnCache;
    m_primaryOriginType         = ORIGIN_TYPE_TextBlock;
    m_type                      = TEXTBLOCK_TYPE_Dgn;
    m_isForEditing              = false;
    m_nodeNumber                = 0;
    m_fittedLength              = 0;
    m_V7Compatible              = false;
    m_forceTextNode             = false;
    m_nodeProperties            = NULL;
    m_processLevel              = PROCESS_LEVEL_TextBlock;
    m_alongRootId               = 0;
    m_descrOffset               = 0.0;
    m_alongDist                 = 0.0;
    m_descrSide                 = DESCRIPTOR_SIDE_Above;
    m_useAlongDist              = false;
    m_alongDgnModel             = NULL;
    
    m_orientation.initIdentity ();
    m_origin.zero ();
    m_primaryOrigin.zero ();
    m_nominalRange.init ();
    m_exactRange.init ();
    m_descrStartPoint.zero ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/04
//---------------------------------------------------------------------------------------
void TextBlock::AppendParagraph (ParagraphR paragraph)
    {
    m_paragraphArray.push_back (&paragraph);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/05
//---------------------------------------------------------------------------------------
void TextBlock::GetHorizontalVerticalJustifications (HorizontalJustification* horizontalJustification, VerticalJustification* verticalJustification, TextElementJustification justification)
    {
    if (NULL != horizontalJustification)
        {
        switch (justification)
            {
            case TextElementJustification::LeftBaseline:
            case TextElementJustification::LeftCap:
            case TextElementJustification::LeftMiddle:
            case TextElementJustification::LeftDescender:
            case TextElementJustification::LeftTop:
                *horizontalJustification = HORIZONTAL_JUSTIFICATION_Left;
                break;

            case TextElementJustification::LeftMarginBaseline:
            case TextElementJustification::LeftMarginCap:
            case TextElementJustification::LeftMarginMiddle:
            case TextElementJustification::LeftMarginDescender:
            case TextElementJustification::LeftMarginTop:
                *horizontalJustification = HORIZONTAL_JUSTIFICATION_LeftMargin;
                break;

            case TextElementJustification::CenterBaseline:
            case TextElementJustification::CenterCap:
            case TextElementJustification::CenterMiddle:
            case TextElementJustification::CenterDescender:
            case TextElementJustification::CenterTop:
                *horizontalJustification = HORIZONTAL_JUSTIFICATION_Center;
                break;

            case TextElementJustification::RightBaseline:
            case TextElementJustification::RightCap:
            case TextElementJustification::RightMiddle:
            case TextElementJustification::RightDescender:
            case TextElementJustification::RightTop:
                *horizontalJustification = HORIZONTAL_JUSTIFICATION_Right;
                break;

            case TextElementJustification::RightMarginBaseline:
            case TextElementJustification::RightMarginCap:
            case TextElementJustification::RightMarginMiddle:
            case TextElementJustification::RightMarginDescender:
            case TextElementJustification::RightMarginTop:
                *horizontalJustification = HORIZONTAL_JUSTIFICATION_RightMargin;
                break;

            default:
                BeAssert (false);
                *horizontalJustification = HORIZONTAL_JUSTIFICATION_Left;
                break;
            }
        }

    if (NULL != verticalJustification)
        {
        switch (justification)
            {
            case TextElementJustification::LeftBaseline:
            case TextElementJustification::LeftMarginBaseline:
            case TextElementJustification::CenterBaseline:
            case TextElementJustification::RightBaseline:
            case TextElementJustification::RightMarginBaseline:
                *verticalJustification = VERTICAL_JUSTIFICATION_Baseline;
                break;

            case TextElementJustification::LeftCap:
            case TextElementJustification::LeftMarginCap:
            case TextElementJustification::CenterCap:
            case TextElementJustification::RightCap:
            case TextElementJustification::RightMarginCap:
                *verticalJustification = VERTICAL_JUSTIFICATION_Cap;
                break;

            case TextElementJustification::LeftMiddle:
            case TextElementJustification::LeftMarginMiddle:
            case TextElementJustification::CenterMiddle:
            case TextElementJustification::RightMiddle:
            case TextElementJustification::RightMarginMiddle:
                *verticalJustification = VERTICAL_JUSTIFICATION_Middle;
                break;

            case TextElementJustification::LeftDescender:
            case TextElementJustification::LeftMarginDescender:
            case TextElementJustification::CenterDescender:
            case TextElementJustification::RightDescender:
            case TextElementJustification::RightMarginDescender:
                *verticalJustification = VERTICAL_JUSTIFICATION_Descender;
                break;

            case TextElementJustification::LeftTop:
            case TextElementJustification::LeftMarginTop:
            case TextElementJustification::CenterTop:
            case TextElementJustification::RightTop:
            case TextElementJustification::RightMarginTop:
                *verticalJustification = VERTICAL_JUSTIFICATION_Top;
                break;

            default:
                BeAssert (false);
                *verticalJustification = VERTICAL_JUSTIFICATION_Top;
                break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/06
//---------------------------------------------------------------------------------------
TextElementJustification TextBlock::GetJustificationFromAlignments (HorizontalJustification horizontalJustification, VerticalJustification verticalJustification)
    {
    switch (horizontalJustification)
        {
        case HORIZONTAL_JUSTIFICATION_Left:
            {
            switch (verticalJustification)
                {
                case VERTICAL_JUSTIFICATION_Baseline:   return TextElementJustification::LeftBaseline;
                case VERTICAL_JUSTIFICATION_Middle:     return TextElementJustification::LeftMiddle;
                case VERTICAL_JUSTIFICATION_Descender:  return TextElementJustification::LeftDescender;
                case VERTICAL_JUSTIFICATION_Top:        return TextElementJustification::LeftTop;
                }
            break;
            }

        case HORIZONTAL_JUSTIFICATION_LeftMargin:
            {
            switch (verticalJustification)
                {
                case VERTICAL_JUSTIFICATION_Baseline:   return TextElementJustification::LeftMarginBaseline;
                case VERTICAL_JUSTIFICATION_Middle:     return TextElementJustification::LeftMarginMiddle;
                case VERTICAL_JUSTIFICATION_Descender:  return TextElementJustification::LeftMarginDescender;
                case VERTICAL_JUSTIFICATION_Top:        return TextElementJustification::LeftMarginTop;
                }
            break;
            }

        case HORIZONTAL_JUSTIFICATION_Center:
            {
            switch (verticalJustification)
                {
                case VERTICAL_JUSTIFICATION_Baseline:   return TextElementJustification::CenterBaseline;
                case VERTICAL_JUSTIFICATION_Middle:     return TextElementJustification::CenterMiddle;
                case VERTICAL_JUSTIFICATION_Descender:  return TextElementJustification::CenterDescender;
                case VERTICAL_JUSTIFICATION_Top:        return TextElementJustification::CenterTop;
                }
            break;
            }

        case HORIZONTAL_JUSTIFICATION_Right:
            {
            switch (verticalJustification)
                {
                case VERTICAL_JUSTIFICATION_Baseline:   return TextElementJustification::RightBaseline;
                case VERTICAL_JUSTIFICATION_Middle:     return TextElementJustification::RightMiddle;
                case VERTICAL_JUSTIFICATION_Descender:  return TextElementJustification::RightDescender;
                case VERTICAL_JUSTIFICATION_Top:        return TextElementJustification::RightTop;
                }
            break;
            }

        case HORIZONTAL_JUSTIFICATION_RightMargin:
            {
            switch (verticalJustification)
                {
                case VERTICAL_JUSTIFICATION_Baseline:   return TextElementJustification::RightMarginBaseline;
                case VERTICAL_JUSTIFICATION_Middle:     return TextElementJustification::RightMarginMiddle;
                case VERTICAL_JUSTIFICATION_Descender:  return TextElementJustification::RightMarginDescender;
                case VERTICAL_JUSTIFICATION_Top:        return TextElementJustification::RightMarginTop;
                }
            break;
            }
        }
    
    BeAssert (false);
    return (TextElementJustification)0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/05
//---------------------------------------------------------------------------------------
bool TextBlock::IsSingleLineFontTrueType () const
    {
    RunP run = GetParagraph (0)->GetLine (0)->GetRun (0);
    if (NULL == run)
        return false;

    DgnFontCR font = run->GetProperties ().GetFont ();

    return (DgnFontType::TrueType == font.GetType ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/08
//---------------------------------------------------------------------------------------
double TextBlock::ComputeHorizontalDescenderJustificationOffset () const
    {
    // Descender distance is the descender ratio of the font times the nominal height (ascender).
    //  This ensures that no matter what characters are actually in the run, the offset is the same,
    //  and determined by the font designer. For lines with runs that have mixed fonts, find the
    //  maximum descender distance and use it.
    // AutoCAD MText uses exact range for descender; match that for MText text blocks.

    ParagraphR  lastParagraph           = *GetParagraph (GetParagraphCount () - 1);
    LineR       lastLine                = *lastParagraph.GetLine (lastParagraph.GetLineCount () - 1);
    size_t      lastLineRunCount        = lastLine.GetRunCount ();
    double      maxDescenderDistance    = 0.0;

    for (size_t lastLineRunIndex = 0; lastLineRunIndex < lastLineRunCount; lastLineRunIndex++)
        {
        RunR        run                 = *lastLine.GetRun (lastLineRunIndex);
        DgnFontCR   runFont             = run.GetProperties ().ResolveFont ();
        bool        isRunFontRSC        = (DgnFontType::Rsc == runFont.GetType ());
        double      descenderDistance;

        if (run.GetProperties ().IsSuperScript () || run.GetProperties ().IsSubScript ())
            continue;

        if ((IsMTextType () || IsComplexText ()) && !isRunFontRSC)
            {
            descenderDistance = run.ComputeTransformedExactRange ().low.y;

            if (descenderDistance > 0.0)
                descenderDistance = 0.0;
            else
                descenderDistance *= -1.0;
            }
        else
            {

            double  descenderRatio  = runFont.GetDescenderRatio ();
            DVec3d  runLineOffset   = run.GetLineOffset ();

            descenderDistance = descenderRatio * run.GetProperties ().GetFontSize ().y - runLineOffset.y;
            }

        if (descenderDistance > maxDescenderDistance)
            maxDescenderDistance = descenderDistance;
        }

    return maxDescenderDistance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/08
//---------------------------------------------------------------------------------------
double TextBlock::ComputeVerticalDescenderJustificationOffset () const
    {
    // Similar to ComputeHorizontalDescenderJustificationOffset, but we must consider the last run
    //  of each line (the effective "bottom" characters) instead of simply the last line.

    double  maxDescenderDistance    = 0.0;

    LineRange lineRange (*this);
    FOR_EACH (LineCR line, lineRange)
        {
        if (0 == line.GetRunCount ())
            continue;

        RunR        lastRun             = *line.GetRun (line.GetRunCount () - 1);
        DgnFontCR   runFont             = lastRun.GetProperties ().ResolveFont ();
        bool        isRunFontRSC        = (DgnFontType::Rsc == runFont.GetType ());
        double      descenderDistance;

        if ((IsMTextType () || IsComplexText ()) && !isRunFontRSC)
            {
            descenderDistance = lastRun.ComputeTransformedExactRange ().low.y - lastRun.ComputeTransformedNominalRange ().low.y;

            if (descenderDistance > 0.0)
                descenderDistance = 0.0;
            else
                descenderDistance *= -1.0;
            }
        else
            {
            DgnFontCR   runFont         = lastRun.GetProperties ().ResolveFont ();
            double      descenderRatio  = runFont.GetDescenderRatio ();

            descenderDistance = descenderRatio * lastRun.GetProperties ().GetFontSize ().y;
            }

        if (descenderDistance > maxDescenderDistance)
            maxDescenderDistance = descenderDistance;
        }

    return maxDescenderDistance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/08
//---------------------------------------------------------------------------------------
static size_t computeNumberOfLinesForMiddleExactlyLineSpacing (TextBlockCR textBlock)
    {
    // TR#253871: We must ignore a single trailing blank line.
    size_t lineCount = textBlock.GetLineCount (textBlock.Begin (), textBlock.End ());
    if (lineCount <= 1)
        return 1;

    Caret lastLineCaret = textBlock.End ();

    lastLineCaret.MoveToPreviousLine ();

    LineCP lastLine = lastLineCaret.GetCurrentLineCP ();
    if (NULL == lastLine)
        {
        BeAssert (false);
        return lineCount;
        }

    if (0 == lastLine->GetRunCount ())
        lineCount--;

    return lineCount;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
DVec3d TextBlock::GetJustificationOffset () const
    {
    DVec3d offset;
    offset.init (0.0, 0.0, 0.0);
    
    if (this->IsEmpty ())
        return offset;
    
    DRange3d                    nominalRange    = m_nominalRange;
    DRange3d                    exactRange      = m_exactRange;
    DRange3d                    jRange          = ComputeJustificationRange ();
    TextElementJustification    justification   = GetParagraph (0)->GetJustification ();

    // Get the last line's displacement below origin.
    Caret caret = End ();
    caret.MoveToPreviousLine ();

    HorizontalJustification horizontalJustification;
    VerticalJustification   verticalJustification;
    GetHorizontalVerticalJustifications (&horizontalJustification, &verticalJustification, justification);

    // Note that in general we want to unify on the concept of justification range. However, vertical
    //  text has been a consistent abomination in the past, so I am purposefully more-or-less keeping
    //  old logic so as to not shift too much text on round-trip. As "odd" (putting it nicely) as it is,
    //  really cleaning it up will cause many shifts in existing text which we can't accept.

    if (m_properties.IsVertical ())
        {
        double  referenceWidth  = 0.0;
        double  referenceHeight = 0.0;
        double  alignShift      = 0.0;

        if (0.0 != m_properties.GetDocumentWidth ())
            referenceHeight = m_properties.GetDocumentWidth ();
        else
            referenceHeight = nominalRange.isNull () ? 0.0 : (nominalRange.high.y - nominalRange.low.y);

        switch (m_type)
            {
            case TEXTBLOCK_TYPE_DwgMText:
                referenceWidth  = exactRange.isNull () ? 0.0 : (exactRange.high.x - exactRange.low.x);
                alignShift      = exactRange.isNull () ? 0.0 : m_exactRange.low.x;
                break;
            
            case TEXTBLOCK_TYPE_DwgDText:
                referenceWidth  = exactRange.isNull () ? 0.0 : (exactRange.high.x - exactRange.low.x);
                break;
            
            case TEXTBLOCK_TYPE_Dgn:
                referenceWidth  = nominalRange.isNull () ? 0.0 : (nominalRange.high.x - nominalRange.low.x);
                break;
            
            default:
                BeAssert (false);
                break;
            }

        switch (horizontalJustification)
            {
            case HORIZONTAL_JUSTIFICATION_LeftMargin:   offset.x = alignShift;                          break;
            case HORIZONTAL_JUSTIFICATION_Left:         offset.x = alignShift;                          break;
            case HORIZONTAL_JUSTIFICATION_Center:       offset.x = 0.5 * referenceWidth + alignShift;   break;
            case HORIZONTAL_JUSTIFICATION_Right:        offset.x = referenceWidth + alignShift;         break;
            case HORIZONTAL_JUSTIFICATION_RightMargin:  offset.x = referenceWidth + alignShift;         break;
            
            default:
                BeAssert (false);
                break;
            }

        offset.x *= -1.0;

        switch (verticalJustification)
            {
            case VERTICAL_JUSTIFICATION_Top:                                                                                        break;
            case VERTICAL_JUSTIFICATION_Middle:     offset.y = 0.5 * referenceHeight;                                               break;
            case VERTICAL_JUSTIFICATION_Baseline:   offset.y = referenceHeight;                                                     break;
            case VERTICAL_JUSTIFICATION_Descender:  offset.y = referenceHeight + ComputeVerticalDescenderJustificationOffset ();    break;
            case VERTICAL_JUSTIFICATION_Cap:                                                                                        break;
            
            default:
                BeAssert (false);
                break;
            }

        return offset;
        }

    double  referenceWidth  = 0.0;
    double  referenceHeight = 0.0;
    double  alignShift      = 0.0;

    if (0.0 != m_properties.GetDocumentWidth())
        {
        referenceWidth = m_properties.GetDocumentWidth ();
        }
    else
        {
        if (IsDTextType () && IsSingleLineFontTrueType ())
            {
            referenceWidth  = nominalRange.isNull () ? 0.0 : (nominalRange.high.x - nominalRange.low.x);
            }
        else if (IsMTextType () || (IsDTextType () && HORIZONTAL_JUSTIFICATION_Center == horizontalJustification))
            {
            referenceWidth  = exactRange.isNull () ? 0.0 : (exactRange.high.x - exactRange.low.x);
            alignShift      = exactRange.isNull () ? 0.0 : exactRange.low.x;
            }
        else
            {
            referenceWidth  = jRange.isNull () ? 0.0 : (jRange.high.x - jRange.low.x);
            }
        }

    if (IsMTextType ())
        referenceHeight = exactRange.isNull () ? 0.0 : (exactRange.high.y - exactRange.low.y);
    else
        referenceHeight = nominalRange.isNull () ? 0.0 : (nominalRange.high.y - nominalRange.low.y);

    switch (horizontalJustification)
        {
        case HORIZONTAL_JUSTIFICATION_Left:         offset.x = IsDTextType () ? 0.0 : alignShift;   break;
        case HORIZONTAL_JUSTIFICATION_LeftMargin:   offset.x = alignShift;                          break;
        case HORIZONTAL_JUSTIFICATION_Center:       offset.x = 0.5 * referenceWidth + alignShift;   break;
        case HORIZONTAL_JUSTIFICATION_Right:        offset.x = referenceWidth + alignShift;         break;
        case HORIZONTAL_JUSTIFICATION_RightMargin:  offset.x = alignShift + GetPaperWidth ();       break;
        
        default:
            BeAssert (false);
            break;
        }

    offset.x *= -1.0;

    DgnLineSpacingType lineSpacingType = GetParagraph (0)->GetLineSpacingType ();
    if (DgnLineSpacingType::ExactFromLineTop == lineSpacingType)
        {
        double  nodeNumberHeight    = this->GetNodeOrFirstRunHeight ();

        referenceHeight = nominalRange.isNull () ? 0.0 : (nominalRange.high.y - nominalRange.low.y);

        // Just to re-iterate, the goal of this function is to compute the offset from the justification
        //  point (user origin) to the nominal top-left of the text block. That is, when the justification
        //  point is below (in axis-aligned Y) the text block origin, the offset should be positive.

        // How AutoCAD places user origin in relation to the baseline:
        //  Top:        User origin is at a height of NodeNumberHeight above the first line's baseline.
        //  Bottom:     User origin is at a height of 1/3 * NodeNumberHeight below the last line's baseline.
        //  Middle:     While top and bottom make some amount of sense, what I do for middle is completely reverse
        //              engineered. I empirically found that in AutoCAD, the justification point was at a distance
        //              ((numLines - 1) * lineSpacing + (2/3) * nodeNumberHeight) / 2.0 from the last line's baseline.
        //              While this formula uses reasonable numbers/factors, is still pretty obtuse.
        //
        //  Cap:        I don't expect this; copy from non-exactly line spacing case.
        //  Bottom:     This is not a valid DWG case; copy from non-exactly line spacing case.

        // AutoCAD moves fractions around normal text based on alignment. We move text around the
        //  fraction via line offset. We must counter-act this.
        
        LineCP  firstLine           = this->GetLine (0);
        double  maxFirstLineOffset  = 0.0;
        
        for (UInt32 runIndex = 0; runIndex < firstLine->GetRunCount (); runIndex++)
            {
            CharStreamCP currCharStream = dynamic_cast<CharStreamCP>(firstLine->GetRun (runIndex));
            if (NULL == currCharStream)
                continue;

            double currRunLineOffset = currCharStream->GetLineOffset ().y;

            if (currRunLineOffset > maxFirstLineOffset)
                maxFirstLineOffset = currRunLineOffset;
            }

        LineCP  lastLine            = this->GetLine (this->GetLineCount (this->Begin (), this->End ()) - 1);
        double  maxLastLineOffset   = 0.0;
        
        for (UInt32 runIndex = 0; runIndex < lastLine->GetRunCount (); runIndex++)
            {
            CharStreamCP currCharStream = dynamic_cast<CharStreamCP>(lastLine->GetRun (runIndex));
            if (NULL == currCharStream)
                continue;

            double currRunLineOffset = currCharStream->GetLineOffset ().y;

            if (currRunLineOffset > maxLastLineOffset)
                maxLastLineOffset = currRunLineOffset;
            }

        switch (verticalJustification)
            {
            case VERTICAL_JUSTIFICATION_Top:
                {
                offset.y = firstLine->GetMaxDistanceAboveBaseline () - nodeNumberHeight;
                offset.y -= maxFirstLineOffset;

                break;
                }

            case VERTICAL_JUSTIFICATION_Middle:
                {
                size_t  lineCount                       = computeNumberOfLinesForMiddleExactlyLineSpacing (*this);
                double  lineSpacingValue                = GetParagraph (0)->GetLineSpacing ();
                double  offsetFromBottomMostBaseline    = ((lineCount - 1) * lineSpacingValue + ((2.0 / 3.0) * nodeNumberHeight)) / 2.0;

                offset.y = referenceHeight - offsetFromBottomMostBaseline;
                offset.y -= maxLastLineOffset;

                break;
                }

            case VERTICAL_JUSTIFICATION_Baseline:
                {
                offset.y = referenceHeight;
                break;
                }
            
            case VERTICAL_JUSTIFICATION_Descender:
                {
                offset.y = referenceHeight + ((1.0 / 3.0) * nodeNumberHeight);
                offset.y -= maxLastLineOffset;

                break;
                }

            case VERTICAL_JUSTIFICATION_Cap:
                {
                offset.y = referenceHeight;
                break;
                }
            
            default:
                {
                BeAssert (false);
                break;
                }
            }
        }
    else
        {
        switch (verticalJustification)
            {
            case VERTICAL_JUSTIFICATION_Top:
                {
                offset.y = 0.0;
                break;
                }
            
            case VERTICAL_JUSTIFICATION_Middle:
                {
                offset.y = 0.5 * referenceHeight;
                break;
                }
            
            case VERTICAL_JUSTIFICATION_Baseline:
                {
                offset.y = referenceHeight;
                break;
                }

            case VERTICAL_JUSTIFICATION_Descender:
                // MText must normally use exact range, which includes descenders to begin with.
                if (IsMTextType ())
                    offset.y = referenceHeight;
                else
                    offset.y = referenceHeight + ComputeHorizontalDescenderJustificationOffset ();
                break;
            
            case VERTICAL_JUSTIFICATION_Cap:
                {
                offset.y = referenceHeight;
                break;
                }
            
            default:
                {
                BeAssert (false);
                break;
                }
            }
        }

    return offset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
TextBlockPtr TextBlock::Create (DgnModelR dgnCache) { return new TextBlock (dgnCache); }

#if defined (_MSC_VER)
    #pragma warning(push)
    #pragma warning (disable:4355)
#endif // defined (_MSC_VER)
TextBlock::TextBlock (DgnModelR dgnCache) :
    RefCountedBase (),
    m_properties                (dgnCache),
    m_paragraphPropertiesForAdd (dgnCache),
    m_runPropertiesForAdd       (dgnCache),
    m_dirty                     (*this)
    {
    this->InitDefaults (dgnCache);
    }
#if defined (_MSC_VER)
    #pragma warning(pop)
#endif // defined (_MSC_VER)

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
TextBlockPtr TextBlock::Create (TextBlockPropertiesCR tbProps, ParagraphPropertiesCR paraProps, RunPropertiesCR runProps, DgnModelR dgnCache) { return new TextBlock (tbProps, paraProps, runProps, dgnCache); }

#if defined (_MSC_VER)
    #pragma warning(push)
    #pragma warning (disable:4355)
#endif // defined (_MSC_VER)
TextBlock::TextBlock (TextBlockPropertiesCR tbProps, ParagraphPropertiesCR paraProps, RunPropertiesCR runProps, DgnModelR dgnCache) :
    RefCountedBase (),
    m_properties                (tbProps),
    m_paragraphPropertiesForAdd (paraProps),
    m_runPropertiesForAdd       (runProps),
    m_dirty                     (*this)
    {
    this->InitDefaults (dgnCache);
    }
#if defined (_MSC_VER)
    #pragma warning(pop)
#endif // defined (_MSC_VER)

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
TextBlockPtr TextBlock::Create (DgnTextStyleCR textStyle, DgnModelR dgnCache) { return new TextBlock (textStyle, dgnCache); }

#if defined (_MSC_VER)
    #pragma warning(push)
    #pragma warning (disable:4355)
#endif // defined (_MSC_VER)
TextBlock::TextBlock (DgnTextStyleCR textStyle, DgnModelR dgnCache) :
    RefCountedBase (),
    m_properties                (textStyle, dgnCache),
    m_paragraphPropertiesForAdd (textStyle, dgnCache),
    m_runPropertiesForAdd       (textStyle, dgnCache),
    m_dirty                     (*this)
    {
    this->InitDefaults (dgnCache);
    }
#if defined (_MSC_VER)
    #pragma warning(pop)
#endif // defined (_MSC_VER)

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/07
//---------------------------------------------------------------------------------------
TextBlockPtr TextBlock::Create (TextBlockCR rhs) { return new TextBlock (rhs); }
TextBlock::TextBlock (TextBlockCR rhs) :
    RefCountedBase (),
    m_dgnModel                  (rhs.m_dgnModel),
    m_properties                (rhs.m_properties),
    m_orientation               (rhs.m_orientation),
    m_origin                    (rhs.m_origin),
    m_primaryOriginType         (rhs.m_primaryOriginType),
    m_primaryOrigin             (rhs.m_primaryOrigin),
    m_type                      (rhs.m_type),
    m_isForEditing              (rhs.m_isForEditing),
    m_nodeNumber                (rhs.m_nodeNumber),
    m_fittedLength              (rhs.m_fittedLength),
    m_V7Compatible              (rhs.m_V7Compatible),
    m_forceTextNode             (rhs.m_forceTextNode),
    m_nodeProperties            ((NULL != rhs.m_nodeProperties) ? new RunProperties (*rhs.m_nodeProperties) : NULL),
    m_dirty                     (rhs.m_dirty),
    m_processLevel              (rhs.m_processLevel),
    m_nominalRange              (rhs.m_nominalRange),
    m_exactRange                (rhs.m_exactRange),
    m_alongRootId               (rhs.m_alongRootId),
    m_descrStartPoint           (rhs.m_descrStartPoint),
    m_descrOffset               (rhs.m_descrOffset),
    m_alongDist                 (rhs.m_alongDist),
    m_descrSide                 (rhs.m_descrSide),
    m_useAlongDist              (rhs.m_useAlongDist),
    m_alongDgnModel             (rhs.m_alongDgnModel),
    m_paragraphPropertiesForAdd (rhs.m_paragraphPropertiesForAdd),
    m_runPropertiesForAdd       (rhs.m_runPropertiesForAdd)
    {
    for (size_t i = 0; i < rhs.m_paragraphArray.size (); ++i)
        m_paragraphArray.push_back (new Paragraph (*rhs.m_paragraphArray[i]));

    // Same position, but different TextBlock.
    m_dirty.SetTextBlock (this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/06
//---------------------------------------------------------------------------------------
TextBlock::~TextBlock  ()
    {
    for (size_t i = 0; i < m_paragraphArray.size (); ++i)
        delete m_paragraphArray[i];

    DELETE_AND_CLEAR (m_nodeProperties)
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/04
//---------------------------------------------------------------------------------------
void TextBlock::Clear ()
    {
    size_t nParagraphs = m_paragraphArray.size ();
    for (size_t i = 0; i < nParagraphs; i++)
        delete m_paragraphArray[i];
    
    m_paragraphArray.clear ();
    m_dirty = this->Begin ();
    
    ComputeRange (true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
bool TextBlock::IsComplexText () const
    {
    if (this->IsMTextType () || this->IsAlongElement ())
        return true;

    int nParagraphs = GetParagraphCount ();
    if (nParagraphs > 1 && !(nParagraphs == 2 && GetParagraph (1)->IsEmpty ()))
        return true;

    ParagraphP paragraph = GetParagraph (0);
    if (paragraph->GetLineCount () > 1)
        return true;

    LineP line = paragraph->GetLine (0);
    if (1 == line->GetRunCount () && NULL == dynamic_cast <CharStream*> (line->GetRun (0)))
        return true;

    RunPropertiesCP firstRunProperties  = this->GetFirstRunProperties ();
    RunPropertiesCP nodeProperties      = this->GetTextNodeRunProperties ();
    if (NULL != nodeProperties && NULL != firstRunProperties && !nodeProperties->Equals (*firstRunProperties))
        return true;

    if (line->GetRunCount () > 0 && NULL != (dynamic_cast <Tab*> (line->GetRun (line->GetRunCount () - 1))))
        return true;

    bool isFirstRun = true;

    RunRange runRange (*this);
    FOR_EACH (RunCR run, runRange)
        {
        CharStreamCP runAsCharStream = dynamic_cast<CharStreamCP>(&run);
        
        if (NULL != dynamic_cast <TabCP> (&run)                                         ||
            NULL != dynamic_cast <LineBreakCP> (&run)                                   ||
            NULL != dynamic_cast <ParagraphBreakCP> (&run)                              ||
            NULL != dynamic_cast <FractionCP> (&run)                                    ||
            ((NULL != runAsCharStream) && (NULL != runAsCharStream->GetFieldData ())) )
            {
            return true;
            }

        if (isFirstRun)
            {
            isFirstRun = false;
            }
        else
            {
            BeAssert (NULL != firstRunProperties);
            if (!firstRunProperties->Equals (run.GetProperties ()))
                return true;
            }
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/05
//---------------------------------------------------------------------------------------
bool TextBlock::IsEmpty () const
    {
    size_t numParagraphs = m_paragraphArray.size ();
    
    if (numParagraphs > 1)
        return false;
    
    if (0 == numParagraphs)
        return true;
    
    ParagraphCP paragraph   = m_paragraphArray.front ();
    size_t      lineCount   = paragraph->GetLineCount ();
    
    if (lineCount > 1)
        return false;
    
    if (0 == lineCount)
        return true;
    
    return (0 == paragraph->GetLine (0)->GetRunCount ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/07
//---------------------------------------------------------------------------------------
RunPropertiesCP TextBlock::GetRunProperties (size_t index) const
    {
    if (this->IsEmpty ())
        return NULL;

    RunRange    range   (*this);
    RunIterator runIter = range.begin ();

    while ((index > 0) && (range.end () != runIter))
        {
        ++runIter;
        --index;
        }

    if ((index > 0) || (range.end () == runIter))
        return NULL;

    return &(*runIter).GetProperties ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/05
//---------------------------------------------------------------------------------------
UInt32 TextBlock::ReplaceStaticFractionsWithStackedFractions (CaretCR start, CaretCR end)
    {
    // Overall operation: move backwards over indicated runs, processing each run with a forward iterator.

    UInt32 numResultingRuns = 1;

    // The outer-most backwards iterating run caret.
    Caret backwardsRunIterCaret = end;
    backwardsRunIterCaret.MoveToPreviousRun ();

    do
        {
        // Only thing to process are CharStream's.
        CharStreamP charStream = dynamic_cast <CharStreamP> (backwardsRunIterCaret.GetCurrentRunP ());
        if (NULL != charStream)
            {
            // This caret tracks the beginning of the run that the inner loop is processing. Note that the inner
            //  loop run can adjust this caret as it adds runs due to splitting/insertion of Fraction's. Even though
            //  the inner loop is inserting runs, the outer backwards-iterating loop can stay intact (because it
            //  it is iterating backwards), thus the need for a separate caret.
            Caret effectiveRunStartCaret = backwardsRunIterCaret;

            // For reach character in charStream...
            //  Note that charStream is not always backwardsRunIterCaret.GetRun becauase when it encounters the
            //  first fraction glyph, it will split charStream, and re-assign charStream to the right-hand split.
            for (UInt32 index = 0; index < charStream->GetCharacterCount (); index++)
                {
                // See if the current glyph is a fraction...
                UInt8   numerator;
                UInt8   denominator;

                if (!charStream->GetFraction (numerator, denominator, index))
                    continue;

                // Make a diagonal fraction run to represent it...
                WChar   numStr[8];
                WChar   denomStr[8];

                BeStringUtilities::Snwprintf (numStr, L"%d", numerator);
                BeStringUtilities::Snwprintf (denomStr, L"%d", denominator);

                FractionP fraction = new DiagonalBarFraction (numStr, denomStr, charStream->GetProperties (), this->ComputeRunLayoutFlags (), StackedFractionAlignment::Middle, NULL);

                // Extract all units from TextBlock that occur after the run we're processing...
                TextBlockNodeArray  nodeArray;
                Caret               extractStart    = effectiveRunStartCaret; // ExtractNodes moves the caret you give it, make a local copy.
                
                this->ExtractNodes (extractStart, nodeArray);

                // Isolate the fraction glyph (secondRun), and get any left-hand- (firstRun) and right-hand-side (thirdRun) runs.
                RunP    firstRun    = NULL;
                RunP    secondRun   = NULL;
                RunP    thirdRun    = NULL;

                charStream->Splice (firstRun, secondRun, index);

                if (NULL != secondRun)
                    secondRun->Splice (secondRun, thirdRun, 1);
                else
                    BeAssert (false); // secondRun should always contain the fraction character we're replacing

                // In order to insert back into the TextBlock, we must look at what's in our extracted unit array.
                //  If the first unit is a run, we can append directly on to the TextBlock (and have the new runs
                //  appended to the end of the last line). If the first unit is a line (or paragraph, which always
                //  has at least one line), then we should inject directly into it. This ensures we keep formatting
                //  and properties of the appropriate line/paragraph.
                ParagraphP  firstUnitAsParagraph    = dynamic_cast<ParagraphP>(nodeArray[0]);
                LineP       firstUnitAsLine         = dynamic_cast<LineP>(nodeArray[0]);
                RunP        firstUnitAsRun          = dynamic_cast<RunP>(nodeArray[0]);
                LineP       workingLine             = NULL;

                if (NULL != firstUnitAsLine)
                    workingLine = firstUnitAsLine;
                else if (NULL != firstUnitAsParagraph)
                    workingLine = firstUnitAsParagraph->GetLine (0);

                UInt32 numRunsAdded = 0;

                // If first unit is run, simply append to nodeArray (which will be directly appended to the TextBlock).
                if (NULL != firstUnitAsRun)
                    {
                    nodeArray.pop_front ();

                    if (NULL != thirdRun)
                        {
                        nodeArray.push_front (thirdRun);
                        numRunsAdded++;
                        }

                    nodeArray.push_front (fraction);

                    if (NULL != firstRun)
                        {
                        nodeArray.push_front (firstRun);
                        numRunsAdded++;
                        }
                    }
                // Else if the first unit resolves to a line, take care to inject the runs at the appropriate location in
                //  the line, and then let the line be appended to the TextBlock later).
                else if (NULL != workingLine)
                    {
                    TextBlockNodeArray lineUnits;
                    workingLine->ExtractNodes (effectiveRunStartCaret, lineUnits);

                    lineUnits.pop_front ();

                    if (NULL != thirdRun)
                        {
                        lineUnits.push_front (thirdRun);
                        numRunsAdded++;
                        }

                    lineUnits.push_front (fraction);

                    if (NULL != firstRun)
                        {
                        lineUnits.push_front (firstRun);
                        numRunsAdded++;
                        }

                    ProcessContext procContext (this, (NULL != firstUnitAsParagraph) ? firstUnitAsParagraph : this->GetParagraph (this->GetParagraphCount () - 1), workingLine);
                    workingLine->AppendNodes (lineUnits, procContext);
                    }
                else
                    {
                    BeAssert (false); // A type of Unit we did not account for.
                    }

                // No matter what happened above, append nodeArray to the TextBlock.
                this->AppendNodes (nodeArray);

                // Should never be NULL, but already BeAssert'ed above.
                //  Note that both conditionals above effectively popped secondRun, and it should never still be
                //  part of the TextBlock (and hence safe/our responsibility to delete since we split it out).
                if (NULL != secondRun)
                    delete secondRun;

                numResultingRuns += numRunsAdded;

                // If thirdRun is NULL, then there're no more characters to process, so let it advance the outer iterator.
                if (NULL == thirdRun)
                    break;

                // If we added runs, we must advance where this inner loop will pick up next.
                for (UInt32 i = 0; i < numRunsAdded; i++)
                    effectiveRunStartCaret.MoveToNextRun ();

                charStream  = static_cast<CharStreamP>(thirdRun);
                index       = -1;
                }
            }

        backwardsRunIterCaret.MoveToPreviousRun ();
        } while (backwardsRunIterCaret.IsAfter (start));

    return numResultingRuns;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
BentleyStatus TextBlock::ReplaceRunProperties (RunPropertiesCR updatedRunProperties, size_t& index)
    {
    Caret   start (*this);
    RunCP   run;
    size_t  localIndex = index;

    while (NULL != (run = start.GetCurrentRunCP ()) && 0 != localIndex)
        {
        localIndex--;
        
        if (SUCCESS != start.MoveToNextRun ())
            break;
        }

    if (localIndex > 0)
        {
        BeAssert (false);
        return ERROR;
        }

    Caret end (start);
    end.MoveToNextRun ();

    DgnFontCR  oldFont             = run->GetProperties ().GetFont ();
    DgnFontCR  newFont             = updatedRunProperties.GetFont ();
    UInt32  numResultingRuns    = 1;

    if (DgnFontType::Rsc == oldFont.GetType () && DgnFontType::Rsc != newFont.GetType ())
        numResultingRuns = this->ReplaceStaticFractionsWithStackedFractions (start, end);

    Caret runIterCaret = start;
    
    for (UInt32 i = 0; i < numResultingRuns; i++)
        {
        if (NULL == runIterCaret.GetCurrentRunCP ())
            {
            BeAssert (false);
            break;
            }

        runIterCaret.GetCurrentRunP ()->SetProperties (updatedRunProperties);
        runIterCaret.MoveToNextRun ();
        }

    index += numResultingRuns - 1;

    m_processLevel = PROCESS_LEVEL_Character;
    m_dirty = this->Begin ();
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/07
//---------------------------------------------------------------------------------------
RunPropertiesCP TextBlock::GetFirstRunProperties () const
    {
    Caret   caret   (*this);
    RunCP   run     = caret.GetCurrentRunCP ();

    return ((NULL == run) ? NULL : &run->GetProperties ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/07
//---------------------------------------------------------------------------------------
RunPropertiesCP TextBlock::GetLastRunProperties () const
    {
    Caret textBlockBeginning = this->Begin ();

    for (Caret caret = this->End (); caret.IsAfterOrAt (textBlockBeginning); caret.MoveToPreviousRun ())
        {
        RunCP run = caret.GetCurrentRunCP ();

        if (NULL != run)
            return &run->GetProperties ();
        }

    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
size_t TextBlock::GetRunPropertiesCount () const
    {
    size_t count = 0;
    
    RunIterator endIter = this->End ().CreateRunEnd ();

    for (RunIterator runIter = this->Begin ().CreateRunBegin (); runIter != endIter; ++runIter)
        ++count;
    
    return count;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Venkat.Kalyan                   12/2005
//---------------------------------------------------------------------------------------
static void extractRootFromDisplayPath (UInt64& rootId, DisplayPathCP displayPath)
    {
    if (NULL == displayPath)
        return;

    EditElementHandle rootEEH;

#if defined (WIP_V10_DISPLAYPATH) // Should not need to support path transforms (i.e. references/shared cells) in Graphite...
    displayPath->GetTransformedElement (rootEEH, displayPath->GetCursorIndex (), NULL);
#else
    rootEEH.SetElementRef (displayPath->GetCursorElem ());
#endif
    
    if (!rootEEH.IsValid ())
        {
        BeAssert (false);
        
        rootId      = INVALID_ELEMENTID;
        
        return;
        }

    rootId = rootEEH.GetElementCP()->GetElementId().GetValue();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
void TextBlock::SetAlongDisplayPath (DisplayPathCP displayPath)
    {
#if defined (NEEDS_WORK_DGNITEM)
    if (!AlongTextDependency::IsRootValid (*displayPath))
        return;
#endif

    UInt64   rootId;
    extractRootFromDisplayPath (rootId, displayPath);

    if (rootId == m_alongRootId)
        return;

    m_dirty         = this->Begin ();
    m_processLevel  = PROCESS_LEVEL_Character;
    m_alongRootId   = rootId;
    m_alongDgnModel = displayPath->GetRoot ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/06
//---------------------------------------------------------------------------------------
void TextBlock::SetTextPrimaryOriginType (OriginType primaryOriginType)
    {
    switch (primaryOriginType)
        {
        case ORIGIN_TYPE_AutoCAD:   SetTextAutoCADOrigin (GetTextAutoCADOrigin ()); break;
        case ORIGIN_TYPE_TextBlock: SetTextOrigin (GetTextOrigin ());               break;
        case ORIGIN_TYPE_Element:   SetTextElementOrigin (GetTextElementOrigin ()); break;
        case ORIGIN_TYPE_User:      SetUserOrigin (GetUserOrigin ());           break;
        
        default:
            BeAssert (false);
            break;
        }
    }

#if defined (NEEDS_WORK_DGNITEM)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/05
//---------------------------------------------------------------------------------------
BentleyStatus TextBlock::GetAlongTextDependency (AlongTextDependencyR atDep) const
    {
    if (0 == m_alongRootId)
        return ERROR;

    // NEEDS_WORK: Need to make this an assoc point linkage
    bool    useDescrStartPoint  = (0.0 == m_alongDist);
    double  descriptorOffset    = 0.0;
    
    switch (m_descrSide)
        {
        case DESCRIPTOR_SIDE_Above: descriptorOffset = m_descrOffset;   break;
        case DESCRIPTOR_SIDE_Below: descriptorOffset = -m_descrOffset;  break;
        case DESCRIPTOR_SIDE_On:    descriptorOffset = 0.0;             break;
        
        default:
            BeAssert (false);
            break;
        }

    atDep.m_rootId                                                  = m_alongRootId;
    atDep.m_customDependencyData.m_startOffsetAlongElement          = m_alongDist;
    atDep.m_customDependencyData.m_distanceFromElement              = descriptorOffset;
    atDep.m_customDependencyData.m_startPoint                       = m_descrStartPoint;
    atDep.m_customDependencyData.m_parameters.m_areParametersUsed   = true;
    atDep.m_customDependencyData.m_parameters.m_isBelowText         = (DESCRIPTOR_SIDE_Above != m_descrSide);
    atDep.m_customDependencyData.m_parameters.m_useStartPoint       = useDescrStartPoint;
    
    return SUCCESS;
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/04
//---------------------------------------------------------------------------------------
DVec3d TextBlock::GetParagraphSpacing (bool isVertical, double logicalNodeNumberHeight, double annotationScale, ParagraphCR existingParagraph, ParagraphCR newParagraph) const
    {
    DVec3d  displacementIn;
    LineCP  existingLine    = existingParagraph.GetLine (existingParagraph.GetLineCount () - 1);
    LineCP  newLine         = newParagraph.GetLine (0);
    DVec3d  displacement    = existingParagraph.ComputeLineSpacing (isVertical, logicalNodeNumberHeight, annotationScale, *existingLine, (NULL == newLine) ? *existingLine : *newLine);

    if (isVertical)
        {
        DPoint3d origin = existingLine->GetOrigin ();

        displacementIn.x = origin.x + displacement.x;
        displacementIn.y = displacementIn.z = 0.0;
        }
    else
        {
        DPoint3d origin = existingLine->GetOrigin ();

        DVec3d baselineDisplacement;
        existingLine->ComputeBaselineDisplacement (baselineDisplacement, existingParagraph.GetLineSpacingType (), *this);

        displacementIn.y = origin.y + displacement.y - baselineDisplacement.y;
        displacementIn.x = displacementIn.z = 0.0;
        }

    return displacementIn;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void TextBlock::ComputeRange (bool recomputeComponentRanges)
    {
    if (recomputeComponentRanges)
        {
        for (size_t i = 0; i < GetParagraphCount (); i++)
            {
            ParagraphP  paragraph = GetParagraph (i);
            paragraph->ComputeRange (recomputeComponentRanges, GetNodeOrFirstRunHeight ());
            }
        }

    m_nominalRange.init ();
    for (size_t i = 0; i < GetParagraphCount (); i++)
        {
        DRange3d paragraphRange = GetParagraph (i)->ComputeTransformedNominalRange ();
        m_nominalRange.Extend (&paragraphRange.low, 2);
        }

    m_exactRange.init ();
    for (size_t i = 0; i < GetParagraphCount (); i++)
        {
        DRange3d paragraphRange = GetParagraph (i)->ComputeTransformedExactRange ();
        m_exactRange.Extend (&paragraphRange.low, 2);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/05
//---------------------------------------------------------------------------------------
void TextBlock::ExtractNodes (CaretR caret, TextBlockNodeArrayR nodeArray)
    {
    if (caret.IsAtEnd ())
        return;

    size_t      paragraphIndex  = caret.GetParagraphIndex ();
    ParagraphP  paragraph       = GetParagraph (paragraphIndex);
    
    if (NULL == paragraph)
        { BeAssert (false && L"Invalid caret."); return; }
    
    size_t extractStartParagraph;
    
    if (0 == caret.GetLineIndex () && 0 == caret.GetRunIndex () && 0 == caret.GetCharacterIndex ())
        {
        extractStartParagraph = paragraphIndex;
        }
    else
        {
        paragraph->ExtractNodes (caret, nodeArray);
        extractStartParagraph = paragraphIndex + 1;
        }

    for (size_t i = extractStartParagraph; i < GetParagraphCount (); i++)
        nodeArray.push_back (GetParagraph (i));

    m_paragraphArray.erase (m_paragraphArray.begin () + extractStartParagraph, m_paragraphArray.begin () + GetParagraphCount ());
    
    ComputeRange (true);
    
    caret = End ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/07
//---------------------------------------------------------------------------------------
void TextBlock::RemoveNodes (CaretCR start)
    {
    TextBlockNodeArray  nodes;
    Caret               startClone  (start);
    
    this->ExtractNodes (startClone, nodes);

    for (TextBlockNodeArray::size_type i = 0; i < nodes.size (); ++i)
        delete nodes[i];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/05
//---------------------------------------------------------------------------------------
void TextBlock::AppendNodes (TextBlockNodeArrayR nodeArray)
    {
#if defined (NEEDS_WORK_DGNITEM)
    if (0 == m_alongRootId)
        {
        AppendNodesNormal (nodeArray);
        return;
        }

    TextBlockNodeArray  newTextBlockNodeArray;
    size_t              nUnits                  = nodeArray.size ();
    
    for (size_t i = 0; i < nUnits; i++)
        nodeArray[i]->Drop (newTextBlockNodeArray);

    if (SUCCESS != AppendNodesAlongElement (newTextBlockNodeArray))
        AppendNodesNormal (newTextBlockNodeArray);
#endif
    }

#if defined (NEEDS_WORK_DGNITEM)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
static double alongElement_computeDistanceAtPoint (GPArrayP srcGPA, DPoint3dCR pt)
    {
    GPArrayParam ptParam;
    srcGPA->GetClosestPoint (NULL, &ptParam, pt, false);
    
    GPArraySmartP partialGPA;
    partialGPA->CopyPortionOf (srcGPA, GPArrayParam (0, 0.0), ptParam);
    
    return partialGPA->Length ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
// Do NOT return computedPoint from this function (take it as an argument like it already is). There is an RVO optimizer bug (x86) when this function is called from alongElement_layoutCharacters, which results in an unmatched push/pop of the stack, and when alongElement_layoutCharacters goes to restore registers that it saved on the stack, it restores garbage, and bad things happen later.
// Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 15.00.30729.01 for 80x86
// Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 16.00.30319.01 for 80x86

static void alongElement_computePointAtDistance (DPoint3dR computedPoint, GPArrayP srcGPA, double distance)
    {
    computedPoint.Zero ();
    
    size_t  componentIndex;
    double  componentParameter;

    if (!srcGPA->GetPointAtDistanceFromStart (distance, componentIndex, componentParameter, computedPoint))
        { BeAssert (false); }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
static BentleyStatus alongElement_extractCharactersAsCharStreams (TextBlockR textBlock, TextBlockNodeArrayCR newUnits, CharStreamArrayR resultingCharStreams)
    {
    LineP existingLine= NULL;
    ParagraphP firstPara = NULL;
    if (NULL != (firstPara = textBlock.GetParagraph (0)))
        existingLine = firstPara->GetLine (0);

    CharStreamArray charStreamsToProcess;
    
    // If we are adding to the end of the line, we must actually re-add the entire line to ensure we get center- and right-justification correct with the updated line length.
    if (NULL!= existingLine && 0 != existingLine->GetRunCount ())
        {
        TextBlockNodeArray  existingUnits;
        Caret               zeroCaret       = textBlock.Begin ();
        
        existingLine->ExtractNodes (zeroCaret, existingUnits);

        FOR_EACH (TextBlockNodeP existingUnit , existingUnits)
            {
            CharStreamP charStream = dynamic_cast<CharStreamP>(existingUnit);
            if (NULL == charStream)
                { BeAssert (false); delete existingUnit; continue; }
            
            charStreamsToProcess.push_back (charStream);
            }
        }

    // Transfer all new CharStreams
    FOR_EACH (TextBlockNodeP newUnit , newUnits)
        {
        CharStreamP charStream = dynamic_cast<CharStreamP>(newUnit);
        if (NULL != charStream)
            charStreamsToProcess.push_back (charStream);
        }
    
    // Split the CharStreams into individual characters
    FOR_EACH (RunP run , charStreamsToProcess)
        {
        while (NULL != run)
            {
            RunP splicedCharacterRun;
            run->Splice (splicedCharacterRun, run, 1);
            
            CharStreamP splicedCharacterCharStream = dynamic_cast<CharStreamP>(splicedCharacterRun);
            if (NULL == splicedCharacterCharStream)
                { BeAssert (false); delete splicedCharacterRun; continue; }
            
            resultingCharStreams.push_back (splicedCharacterCharStream);
            }
        }

    // Configure the new CharStreams we've just created
    for (TextBlockNodeArray::size_type i = 0; i < resultingCharStreams.size (); ++i)
        {
        resultingCharStreams[i]->SetIsLastRunInLine ((resultingCharStreams.size () - 1) == i);
        resultingCharStreams[i]->Preprocess ();
        }
    
    return resultingCharStreams.empty () ? ERROR : SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/05
//---------------------------------------------------------------------------------------
static bool alongElement_shouldReverseFlowDirection (GPArrayP pathGPA, DVec3dCP curveTangent, RotMatrixCR referenceOrientation, int componentIndex, double componentParameter)
    {
    DVec3d tmpTangent;
    if (NULL == curveTangent)
        {
        DPoint3d point;
        if (!pathGPA->GetPrimitiveFractionPointAndTangent (point, tmpTangent, componentIndex, componentParameter))
            { BeAssert (false); return false; }
        
        tmpTangent.Normalize ();
        
        curveTangent = &tmpTangent;
        }
    
    DVec3d referenceXVector;
    referenceOrientation.GetColumn (referenceXVector, 0);
    
    // Vancouver: Bias towards not reversing.
    //  In the V8i to Vancouver rewrite from element descriptor APIs to direct GPA access, round-off differences are popping up. Making the decision now to bias one way or another.
    return (referenceXVector.DotProduct (*curveTangent) < -mgds_fc_epsilon);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
static bool alongElement_shouldReverseFlowDirection (GPArrayP pathGPA, DVec3dCP curveTangent, RotMatrixCR referenceOrientation, double distanceAlong)
    {
    size_t      componentIndex;
    double      componentParameter;
    DPoint3d    xyz;
    
    if (!pathGPA->GetPointAtDistanceFromStart (distanceAlong, componentIndex, componentParameter, xyz))
        { BeAssert (false); return false; }
    
    return alongElement_shouldReverseFlowDirection (pathGPA, curveTangent, referenceOrientation, (int)componentIndex, componentParameter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
static void alongElement_computeOrientationData
(
GPArrayP    pathGPA,
int         componentIndex,
double      componentParameter,
RotMatrixCR referenceOrientation,
DVec3dP     curveTangent,
DVec3dP     curveNormal,
DVec3dP     curvePlaneNormal,
bool        autoNegateCurveTangent,
bool        is3d
)
    {
    DVec3d  tmpTangent;     if (NULL == curveTangent)       curveTangent        = &tmpTangent;
    DVec3d  tmpCurveNormal; if (NULL == curveNormal)        curveNormal         = &tmpCurveNormal;
    DVec3d  tmpPlaneNormal; if (NULL == curvePlaneNormal)   curvePlaneNormal    = &tmpPlaneNormal;
    
    curveTangent->Zero ();
    curveNormal->Zero ();
    curvePlaneNormal->Zero ();
    
    DPoint3d point;
    
    BeAssert (NULL != curveTangent);
    if (!pathGPA->GetPrimitiveFractionPointAndTangent (point, *curveTangent, componentIndex, componentParameter))
        { BeAssert (false); return; }
    
    curveTangent->Normalize ();
    
    if (autoNegateCurveTangent && alongElement_shouldReverseFlowDirection (pathGPA, curveTangent, referenceOrientation, componentIndex, componentParameter))
        curveTangent->Negate ();
    
    bool isPlanar = false;
    
    if (pathGPA->IsConnectedAndClosed (mgds_fc_epsilon, mgds_fc_epsilon))
        {
        DPlane3d plane;
        if (!pathGPA->GetPlane (plane))
            { BeAssert (false); return; }
    
        isPlanar = true;
        *curvePlaneNormal = plane.normal;
        }
    
    DVec3d referenceZVector;
    referenceOrientation.GetColumn (referenceZVector, 2);
    
    if (isPlanar)
        {
        if (curvePlaneNormal->DotProduct (referenceZVector) < 0.0) 
            curveNormal->CrossProduct (*curveTangent, *curvePlaneNormal);
        else
            curveNormal->CrossProduct (*curvePlaneNormal, *curveTangent);
        }
    else
        {
        if (!is3d)
            {
            curveNormal->CrossProduct (referenceZVector, *curveTangent);
            }
        else
            {
            curveNormal->CrossProduct (referenceZVector, *curveTangent);        
            if (0.0 == curveNormal->Magnitude ())
                {
                DVec3d xVec;
                xVec.Init (1.0, 0.0, 0.0);
                
                DVec3d refN;
                refN.CrossProduct (*curveTangent, xVec);
                
                curveNormal->CrossProduct (*curveTangent, refN);
                }
            }    
    
        curveNormal->Normalize ();
        curvePlaneNormal->CrossProduct (*curveTangent, *curveNormal);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
static void alongElement_computeOrientationData
(
GPArrayP    pathGPA,
double      distanceAlong,
RotMatrixCR referenceOrientation,
DVec3dP     curvetangent,
DVec3dP     curveNormal,
DVec3dP     curvePlaneNormal,
bool        autoNegateCurveNormal,
bool        is3d
)
    {
    DVec3d  tmpTangent;     if (NULL == curvetangent)       curvetangent        = &tmpTangent;
    DVec3d  tmpCurveNormal; if (NULL == curveNormal)        curveNormal         = &tmpCurveNormal;
    DVec3d  tmpPlaneNormal; if (NULL == curvePlaneNormal)   curvePlaneNormal    = &tmpPlaneNormal;
    
    curvetangent->Zero ();
    curveNormal->Zero ();
    curvePlaneNormal->Zero ();
    
    size_t  componentIndex;
    double  componentParameter;
    DPoint3d xyz;
    
    if (!pathGPA->GetPointAtDistanceFromStart (distanceAlong, componentIndex, componentParameter, xyz))
        { BeAssert (false); return; }
    
    alongElement_computeOrientationData (pathGPA, (int)componentIndex, componentParameter, referenceOrientation, curvetangent, curveNormal, curvePlaneNormal, autoNegateCurveNormal, is3d);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
static double alongElement_computeBaseOffsetDistance (GPArrayP pathGPA, AlongTextDependencyCR atDep)
    {
    if (!atDep.m_customDependencyData.m_parameters.m_useStartPoint)
        return atDep.m_customDependencyData.m_startOffsetAlongElement;
    
    return alongElement_computeDistanceAtPoint (pathGPA, atDep.m_customDependencyData.m_startPoint);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
static BentleyStatus alongElement_createPathGPA (GPArraySmartP& pathGPA, AlongTextDependencyCR atDep, TextBlockCR textBlock, double& baseOffsetDistance, bool& shouldReverseDirection)
    {
    //...............................................................................................................................................Resolve the target along path element
    DisplayPath path;
    if (SUCCESS != atDep.CreateDisplayPath (path, textBlock.GetDgnModelR ()))
        { BeAssert (false); return ERROR; }
    
    EditElementHandle pathElement;

#if defined (WIP_V10_DISPLAYPATH) // Should not need to support path transforms (i.e. references/shared cells) in Graphite...
    path.GetTransformedElement (pathElement, path.GetCursorIndex (), NULL);
#else
    pathElement.SetElementRef (path.GetCursorElem ());
#endif
    
    if (!pathElement.IsValid ())
        { BeAssert (false); return ERROR; }
    
    //...............................................................................................................................................Create a GPA for the original element
    if (SUCCESS != pathGPA->Add (pathElement))
        { BeAssert (false); return ERROR; }

    //...............................................................................................................................................Early-resolve the non-offset case
    if (0.0 == atDep.m_customDependencyData.m_distanceFromElement)
        {
        baseOffsetDistance      = alongElement_computeBaseOffsetDistance (pathGPA, atDep);
        shouldReverseDirection  = alongElement_shouldReverseFlowDirection (pathGPA, NULL, textBlock.GetOrientation (), baseOffsetDistance);

        return SUCCESS;
        }
    
    //...............................................................................................................................................Stroke the original path
    // Several GPA functions only support line segments, so we have to stroke. This is still preferable to creating temporary elements.
    //  Note that this will obviously tweak values compared to old versions, but at the end of the day, along-text is effectively facetted anyway, so it's not so bad.
    //  0.1 was picked as the angle tolerance through some empirical testing. Smaller values can be used, but I would not recommend making it larger.
    
    static const double STROKE_ANGLE_TOLERANCE = 0.1;
    
    GPArraySmartP strokedOriginalShape;
    strokedOriginalShape->AddStrokes (*pathGPA, 0.0, STROKE_ANGLE_TOLERANCE, 0.0);

    if (0 == strokedOriginalShape->GetCount ())
        { BeAssert (false); return ERROR; }
    
    //...............................................................................................................................................Compute some values based on original path
    // We do the computations we can on the original GPA (or its stroke if required) to get more accurate than the offset path will give.
    
    // Cache the data we'll use later to resolve the new base offset distance.
    double      baseOffsetDistanceOnOriginalPath    = alongElement_computeBaseOffsetDistance (strokedOriginalShape, atDep);
    DPoint3d    startPointOnOriginalPath;           alongElement_computePointAtDistance (startPointOnOriginalPath, pathGPA, baseOffsetDistanceOnOriginalPath);
    RotMatrix   referenceOrientation                = textBlock.GetOrientation ();
    DVec3d      curveNormalAtBasePoint;
    DVec3d      curveTangentAtBasePoint;
    
    alongElement_computeOrientationData (pathGPA, baseOffsetDistanceOnOriginalPath, referenceOrientation, &curveTangentAtBasePoint, &curveNormalAtBasePoint, NULL, true, textBlock.GetDgnModelR ().Is3d ());
    
    shouldReverseDirection = alongElement_shouldReverseFlowDirection (pathGPA, NULL, referenceOrientation, baseOffsetDistanceOnOriginalPath);
    
    // Compute the data we'll need for the offset.
    DVec3d  curveNormalAtPathStart;
    DVec3d  curvePlaneNormalAtPathStart;
    alongElement_computeOrientationData (pathGPA, 0, 0.0, referenceOrientation, NULL, &curveNormalAtPathStart, &curvePlaneNormalAtPathStart, false, textBlock.GetDgnModelR ().Is3d ());
    
    //...............................................................................................................................................Compute the offset GPA
    // Note that this reverse factor is not directly related to what alongElement_shouldReverseFlowDirection can compute.
    //  That reverse factor relates to direction along curve; our's relates to which side of the curve to offset.
    
    DVec3d  referenceYVector;   referenceOrientation.GetColumn (referenceYVector, 1);
    double  reverseFactor       = (referenceYVector.DotProduct (curveNormalAtPathStart) >= 0.0) ? 1.0 : -1.0;
    
    double effectiveDistanceFromElement = reverseFactor * atDep.m_customDependencyData.m_distanceFromElement;
    
    GPArraySmartP offsetGPA;
    offsetGPA->AddOffsetCurves (*strokedOriginalShape, effectiveDistanceFromElement, &curvePlaneNormalAtPathStart, msGeomConst_piOver2, -1.0);

    if (0 == offsetGPA->GetCount ())
        { BeAssert (false); return ERROR; }
    
    //...............................................................................................................................................Replace the original GPA
    pathGPA.ExtractFrom (offsetGPA);
    
    //...............................................................................................................................................Compute the updated base offset
    reverseFactor = shouldReverseDirection ? -1.0 : 1.0;
    
    if (atDep.m_customDependencyData.m_parameters.m_isBelowText)
        reverseFactor *= -1.0;
    
    effectiveDistanceFromElement = reverseFactor * atDep.m_customDependencyData.m_distanceFromElement;
    
    DPoint3d translatedStartPoint = {
                                        startPointOnOriginalPath.x + curveNormalAtBasePoint.x * effectiveDistanceFromElement,
                                        startPointOnOriginalPath.y + curveNormalAtBasePoint.y * effectiveDistanceFromElement,
                                        startPointOnOriginalPath.z + curveNormalAtBasePoint.z * effectiveDistanceFromElement
                                    };
    
    baseOffsetDistance = alongElement_computeDistanceAtPoint (pathGPA, translatedStartPoint);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/05
//---------------------------------------------------------------------------------------
static void alongElement_computeStartDistances (bvector<double>& startDistances, CharStreamArray const & runs)
    {
    startDistances.push_back (0.0);
    
    for (RunArray::size_type i = 0; i < runs.size () - 1; ++i)
        {
        DVec3d displacement = runs[i]->GetRunSpacing ();
        
        startDistances.push_back (startDistances.back () + displacement.x);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
static DescriptorSide alongElement_determineElementSide (AlongTextDependencyCR atDep)
    {
    if (atDep.m_customDependencyData.m_parameters.m_areParametersUsed)
        return atDep.m_customDependencyData.m_parameters.m_isBelowText ? DESCRIPTOR_SIDE_Below : DESCRIPTOR_SIDE_Above;
    
    return (atDep.m_customDependencyData.m_distanceFromElement < 0.0) ? DESCRIPTOR_SIDE_Below : DESCRIPTOR_SIDE_Above;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/05
//---------------------------------------------------------------------------------------
static void alongElement_computeJustificationOffset
(
DVec3dR                     justificationOffset,
double                      totalWidth,
double                      firstUnitWidth,
double                      firstUnitHeight,
TextElementJustification    justification,
DescriptorSide              side
)
    {
    justificationOffset.Zero ();
    
    switch (justification)
        {
        case TextElementJustification::LeftTop:
        case TextElementJustification::LeftMiddle:
        case TextElementJustification::LeftBaseline:
        case TextElementJustification::LeftMarginTop:
        case TextElementJustification::LeftMarginMiddle:
        case TextElementJustification::LeftMarginBaseline:
        case TextElementJustification::LeftCap:
        case TextElementJustification::LeftDescender:
        case TextElementJustification::LeftMarginCap:
        case TextElementJustification::LeftMarginDescender:
            
            justificationOffset.x = 0.0;
            break;
        
        case TextElementJustification::CenterTop:
        case TextElementJustification::CenterMiddle:
        case TextElementJustification::CenterBaseline:
        case TextElementJustification::CenterCap:
        case TextElementJustification::CenterDescender:
            
            justificationOffset.x = (totalWidth - firstUnitWidth) / 2.0;
            break;
        
        case TextElementJustification::RightMarginTop:
        case TextElementJustification::RightMarginMiddle:
        case TextElementJustification::RightMarginBaseline:
        case TextElementJustification::RightTop:
        case TextElementJustification::RightMiddle:
        case TextElementJustification::RightBaseline:
        case TextElementJustification::RightMarginCap:
        case TextElementJustification::RightMarginDescender:
        case TextElementJustification::RightCap:
        case TextElementJustification::RightDescender:
            
            justificationOffset.x = totalWidth - (firstUnitWidth / 2.0); // WTF mate?
            break;
        
        default:
            BeAssert (false);
            break;
        }

    switch (side)
        {
        case DESCRIPTOR_SIDE_Above: justificationOffset.y = 0.0;                        break;
        case DESCRIPTOR_SIDE_Below: justificationOffset.y = firstUnitHeight;            break;
        case DESCRIPTOR_SIDE_On:    justificationOffset.y = -(firstUnitHeight / 2.0);   break; // I don't think we'll actually encounter this...
        
        default:
            BeAssert (false);
            break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
static void alongElement_layoutCharacters
(
GPArrayP                pathGPA,
TextBlockR              textBlock,
AlongTextDependencyCR   atDep,
CharStreamArrayCR       characterCharStreams,
bvector<double> const & characterStartDistances,
DVec3d                  justificationOffset,
double                  baseOffsetDistance,
bool                    shouldReverseDirection
)
    {
    bool        isTextPathClosed            = TO_BOOL (pathGPA->IsConnectedAndClosed (mgds_fc_epsilon, mgds_fc_epsilon));
    double      textPathPerimeter           = pathGPA->Length ();
    RotMatrix   textBlockOrientation        = textBlock.GetOrientation ();
    double      firstUnitWidth              = characterCharStreams.front ()->GetNominalWidth ();
    double      reverseFactor               = shouldReverseDirection ? -1.0 : 1.0;
    
    RotMatrix invertedTextBlockRotation = textBlockOrientation;
    invertedTextBlockRotation.Invert ();
    
    for (CharStreamArray::size_type i = 0; i < characterCharStreams.size (); ++i)
        {
        CharStreamR currCharStream      = *characterCharStreams[i];
        double      currStartDistance   = baseOffsetDistance + (characterStartDistances[i] * reverseFactor) - (justificationOffset.x * reverseFactor);
        bool        isOffEnd            = false;
        
        if (isTextPathClosed)
            {
            if (currStartDistance < 0.0)
                currStartDistance += textPathPerimeter;
            else if (currStartDistance > textPathPerimeter)
                currStartDistance -= textPathPerimeter;
            }
        else
            {
            if (currStartDistance < 0.0)
                {
                isOffEnd = true;
                currStartDistance = 0.0;
                }
            else if (currStartDistance > textPathPerimeter)
                {
                isOffEnd = true;
                currStartDistance = textPathPerimeter;
                }
            }
        
        DVec3d curveTangent;
        DVec3d curveNormal;
        DVec3d curvePlaneNormal;
        alongElement_computeOrientationData (pathGPA, currStartDistance, textBlockOrientation, &curveTangent, &curveNormal, &curvePlaneNormal, false, textBlock.GetDgnModelR ().Is3d ());
        
        if (shouldReverseDirection)
            {
            curveTangent.Negate ();
            curveNormal.Negate ();
            }

        RotMatrix characterOrientation;
        characterOrientation.InitFromColumnVectors (curveTangent, curveNormal, curvePlaneNormal);

        DPoint3d characterOrigin;
        alongElement_computePointAtDistance (characterOrigin, pathGPA, currStartDistance);
        
        curveTangent.Scale (-firstUnitWidth / 2.0);
        characterOrigin.SumOf (characterOrigin, curveTangent);
        
        if (isOffEnd)
            {
            DVec3d xColumn;
            characterOrientation.GetColumn (xColumn, 0);
            xColumn.Scale ((characterStartDistances[i] - baseOffsetDistance) + currStartDistance - justificationOffset.x);
            
            characterOrigin.Add (xColumn);
            }
        
        DVec3d yColumn;
        characterOrientation.GetColumn (yColumn, 1);
        yColumn.Scale (-justificationOffset.y);
        
        characterOrigin.Add (yColumn);
        
        invertedTextBlockRotation.Multiply (characterOrigin);
        characterOrientation.InitProduct (invertedTextBlockRotation, characterOrientation);

        currCharStream.SetOrigin (characterOrigin);
        currCharStream.SetOrientation (characterOrientation);
        
        if (NULL == textBlock.GetParagraph (0))
            {
            ParagraphP para = new Paragraph (textBlock.GetDgnModelR ());
            para->SetProperties (textBlock.GetParagraphPropertiesForAdd ());
            textBlock.AppendParagraph (*para);
            }

        if (NULL == textBlock.GetParagraph (0)->GetLine (0))
            textBlock.GetParagraph (0)->AddLine (*new Line ());
        
        textBlock.GetParagraph (0)->GetLine (0)->AddRun (&currCharStream);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
static void alongElement_zeroNonRunOrigins (TextBlockR textBlock)
    {
    DPoint3d zeroOrigin;
    zeroOrigin.Zero ();
    
    textBlock.SetTextOrigin (zeroOrigin);
    textBlock.GetParagraph (0)->SetOrigin (zeroOrigin);
    textBlock.GetParagraph (0)->GetLine (0)->SetOrigin (zeroOrigin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/04
//---------------------------------------------------------------------------------------
BentleyStatus TextBlock::AppendNodesAlongElement (TextBlockNodeArrayCR newUnitsToAppend)
    {
    //...............................................................................................................................................Input and state validation
    if (0 == newUnitsToAppend.size () || NULL == m_dgnModel)
        return ERROR;

    AlongTextDependency atDep;
    if (SUCCESS != this->GetAlongTextDependency (atDep) || !atDep.IsValid (*m_dgnModel))
        return ERROR;

    //...............................................................................................................................................Get all valid characters as CharStreams
    CharStreamArray characterCharStreams;
    if (SUCCESS != alongElement_extractCharactersAsCharStreams (*this, newUnitsToAppend, characterCharStreams))
        return ERROR;
    
    //...............................................................................................................................................Get the geometry for the path
    GPArraySmartP   pathGPA;
    double          baseOffsetDistance;
    bool            shouldReverseDirection;
    
    if (SUCCESS != alongElement_createPathGPA (pathGPA, atDep, *this, baseOffsetDistance, shouldReverseDirection))
        return ERROR;
    
    //...............................................................................................................................................Compute start distances along curve
    bvector<double> characterStartDistances;
    alongElement_computeStartDistances (characterStartDistances, characterCharStreams);
    
    //...............................................................................................................................................Compute justification offset
    DescriptorSide  side                = alongElement_determineElementSide (atDep);
    double          firstUnitWidth      = characterCharStreams.front ()->GetNominalWidth ();
    double          firstUnitHeight     = characterCharStreams.front ()->GetNominalHeight ();
    double          totalWidth          = characterStartDistances.back () + characterCharStreams.back ()->GetNominalWidth () - characterStartDistances.front ();
    DVec3d          justificationOffset;
    
    alongElement_computeJustificationOffset (justificationOffset, totalWidth, firstUnitWidth, firstUnitHeight, m_paragraphPropertiesForAdd.GetJustification (), side);
    
    //...............................................................................................................................................Assign origin & orientation to characters
    alongElement_layoutCharacters (pathGPA, *this, atDep, characterCharStreams, characterStartDistances, justificationOffset, baseOffsetDistance, shouldReverseDirection);
    
    //...............................................................................................................................................Clean up
    alongElement_zeroNonRunOrigins (*this);
    ComputeRange (true);
    m_dirty = End ();
    
    return SUCCESS;
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/05
//---------------------------------------------------------------------------------------
double TextBlock::GetTextBlockReferenceDistance () const
    {
    double      referenceDistance   = m_properties.GetDocumentWidth ();
    DRange3d    jRange              = ComputeJustificationRange ();

    if (m_properties.IsVertical ())
        {
        if (0.0 == referenceDistance)
            referenceDistance = (jRange.high.y < jRange.low.y) ? 0.0 : jRange.high.y - jRange.low.y;
        }
    else
        {
        if (0.0 == referenceDistance)
            {
            if (TEXTBLOCK_TYPE_Dgn == GetType ())
                referenceDistance = jRange.isNull () ? 0.0 : jRange.high.x;
            else
                referenceDistance = GetNominalWidth ();
            }
        }

    return referenceDistance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/05
//---------------------------------------------------------------------------------------
void TextBlock::GetLineReferenceDistances (double& referenceDistance, double& alignShift, LineCR line, HorizontalJustification hJust) const
    {
    DRange3d lineNominalRange   = line.GetNominalRange ();
    DRange3d lineExactRange     = line.GetExactRange ();
    DRange3d lineJRange         = line.ComputeJustificationRange ();

    if (m_properties.IsVertical ())
        {
        referenceDistance = lineJRange.isNull () ? 0.0 : lineJRange.high.y - lineJRange.low.y;
        }
    else
        {
        alignShift = (TEXTBLOCK_TYPE_DwgMText == m_type) ? line.ComputeLeftEdgeAlignDistance () : 0.0;

        if (IsDTextType () && IsSingleLineFontTrueType ())
            referenceDistance = lineNominalRange.isNull () ? 0.0 : (lineNominalRange.high.x - lineNominalRange.low.x);
        else if (IsMTextType () || (IsDTextType () && HORIZONTAL_JUSTIFICATION_Center == hJust))
            referenceDistance = lineExactRange.isNull () ? 0.0 : (lineExactRange.high.x - lineExactRange.low.x);
        else
            referenceDistance = lineJRange.isNull () ? 0.0 : (lineJRange.high.x - lineJRange.low.x);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/04
//---------------------------------------------------------------------------------------
bool TextBlock::JustifyVerticalLine (LineR line, ParagraphCR paragraph, double maxHeight, double indent, bool wordWrappedText)
    {
    VerticalJustification   verticalJustification;
    HorizontalJustification horizontalJustification;
    GetHorizontalVerticalJustifications (&horizontalJustification, &verticalJustification, paragraph.GetJustification ());

    double  alignShift          = 0.0;
    double  referenceDistance   = 0.0;
    
    GetLineReferenceDistances (referenceDistance, alignShift, line, horizontalJustification);
    
    double  unusedSpace = maxHeight - referenceDistance;
    double  spaceToAdd  = 0.0;

    switch (verticalJustification)
        {
        case VERTICAL_JUSTIFICATION_Middle:     spaceToAdd = - (unusedSpace * 0.5); break;
        case VERTICAL_JUSTIFICATION_Baseline:     spaceToAdd = - unusedSpace;         break;
        case VERTICAL_JUSTIFICATION_Descender:  spaceToAdd = - unusedSpace;         break;
        case VERTICAL_JUSTIFICATION_Top:        spaceToAdd = 0.0;                   break;
        case VERTICAL_JUSTIFICATION_Cap:        spaceToAdd = 0.0;                   break;
        
        default:
            BeAssert (false);
            break;
        }

    double      totalDisplacement   = spaceToAdd + indent;
    DPoint3d    lineOrigin          = line.GetOrigin ();

    lineOrigin.y = totalDisplacement;
    line.SetBaselineAdjustedOrigin (lineOrigin, *this, paragraph.GetLineSpacingType ());
    line.ComputeRange (false, paragraph.GetLineSpacingType (), GetNodeOrFirstRunHeight ());

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/04
//---------------------------------------------------------------------------------------
bool TextBlock::JustifyHorizontalLine (LineR line, ParagraphCR paragraph, double maxWidth, double indent, bool wordWrappedText)
    {
    size_t      nRuns           = line.GetRunCount ();
    DPoint3d    lineOrigin;
    DVec3d      displacement;

    lineOrigin.zero ();
    displacement.zero ();

    VerticalJustification   verticalJustification;
    HorizontalJustification horizontalJustification;
    GetHorizontalVerticalJustifications (&horizontalJustification, &verticalJustification, paragraph.GetJustification ());

    bool    isMarginJustified   = (HORIZONTAL_JUSTIFICATION_LeftMargin == horizontalJustification) || (HORIZONTAL_JUSTIFICATION_RightMargin == horizontalJustification);
    double  alignShift          = 0.0;
    double  referenceDistance   = 0.0;
    
    GetLineReferenceDistances (referenceDistance, alignShift, line, horizontalJustification);
    
    double  unusedSpace                 = maxWidth - referenceDistance;
    bool    performedFullJustification  = false;

    if (nRuns > 0)
        {
        bool isWrapped = (0.0 != m_properties.GetDocumentWidth ());
        
        if (!isMarginJustified
                && paragraph.IsFullJustification ()
                && ((!isWrapped && (&line != this->End ().GetCurrentLineCP ())) || (isWrapped && (&line != paragraph.GetLine (paragraph.GetLineCount () - 1))))
                && (unusedSpace > mgds_fc_epsilon))
            {
            if (paragraph.GetIsFullJustification ())
                {
                line.FullJustify (unusedSpace, false);
                performedFullJustification = true;
                }
            }

        if (!performedFullJustification)
            {
            switch (horizontalJustification)
                {
                case HORIZONTAL_JUSTIFICATION_Left:         displacement.x = -alignShift;                                                                                       break;
                case HORIZONTAL_JUSTIFICATION_LeftMargin:   if (0.0 != m_properties.GetMaxCharactersPerLine ()) { displacement.x = (GetPaperWidth () - referenceDistance); }    break;
                case HORIZONTAL_JUSTIFICATION_Center:       displacement.x = unusedSpace * 0.5 - alignShift;                                                                    break;
                case HORIZONTAL_JUSTIFICATION_Right:        displacement.x = unusedSpace - alignShift;                                                                          break;
                case HORIZONTAL_JUSTIFICATION_RightMargin:  displacement.x = 0.0;                                                                                               break;
                
                default:
                    BeAssert (false);
                    break;
                }
            }
        }

    if (!performedFullJustification)
        {
        lineOrigin      = line.GetOrigin ();
        lineOrigin.x    = displacement.x + indent;
        
        line.SetOrigin (lineOrigin);
        }

    line.ComputeRange (false, paragraph.GetLineSpacingType (), GetNodeOrFirstRunHeight ());

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void TextBlock::JustifyLines (double minimumReferenceDistance)
    {
    if (0.0 == m_properties.GetDocumentWidth ())
        {
        double  measuredReferenceDistance   = 0.0;
        double  textblockReferenceDistance  = minimumReferenceDistance;
        
        for (LineIterator lineIter = this->Begin ().CreateLineBegin (); this->End ().CreateLineEnd () != lineIter; ++lineIter)
            {
            HorizontalJustification hJust;
            GetHorizontalVerticalJustifications (&hJust, NULL, lineIter.GetCurrentParagraphCP ()->GetJustification ());

            double  lineWidthOrHeight   = 0.0;
            double  alignShift          = 0.0;
            
            GetLineReferenceDistances (lineWidthOrHeight, alignShift, *lineIter, hJust);
            
            measuredReferenceDistance = SETMAX (measuredReferenceDistance, lineWidthOrHeight);
            }

        if (measuredReferenceDistance > textblockReferenceDistance)
            {
            m_dirty = this->Begin ();
            textblockReferenceDistance = measuredReferenceDistance;
            }

        for (LineIterator lineIter = this->Begin ().CreateLineBegin (); this->End ().CreateLineEnd () != lineIter; ++lineIter)
            {
            LineR               line                = const_cast<LineR>(*lineIter);
            ParagraphR          paragraph           = const_cast<ParagraphR>(*lineIter.GetCurrentParagraphCP ());
            IndentationDataCR   indentation         = paragraph.GetProperties().GetIndentation ();
            bool                isFirstLine         = (paragraph.GetLine (0) == &line);
            double              indent              = isFirstLine ? indentation.GetFirstLineIndent () : indentation.GetHangingIndent ();
            bool                lineChanged         = false;

            if (m_properties.IsVertical ())
                lineChanged = JustifyVerticalLine (line, paragraph, textblockReferenceDistance, indent, false);
            else
                lineChanged = JustifyHorizontalLine (line, paragraph, textblockReferenceDistance, indent, false);

            if (lineChanged)
                paragraph.ComputeRange (false, GetNodeOrFirstRunHeight ());
            }
        }
    else
        {
        for (size_t i = 0; i < GetParagraphCount (); i++)
            {
            ParagraphP          paragraph           = GetParagraph (i);
            IndentationDataCR   indentation         = paragraph->GetProperties().GetIndentation ();
            bool                paragraphChanged    = false;

            for (size_t j = 0; j < paragraph->GetLineCount (); j++)
                {
                bool    lineChanged     = false;
                LineP   line            = paragraph->GetLine (j);
                double  indent          = (j == 0) ? indentation.GetFirstLineIndent () : indentation.GetHangingIndent ();

                if (m_properties.IsVertical ())
                    lineChanged = JustifyVerticalLine (*line, *paragraph, m_properties.GetDocumentWidth (), indent, true);
                else
                    lineChanged = JustifyHorizontalLine (*line, *paragraph, m_properties.GetDocumentWidth(), indent, true);

                if (!paragraphChanged)
                    paragraphChanged = lineChanged;
                }

            if (paragraphChanged)
                paragraph->ComputeRange (false, GetNodeOrFirstRunHeight ());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
void TextBlock::SetOriginForNextParagraph (ParagraphP nextParagraph, ParagraphCP currentParagraph) const
    {
    DPoint3d origin;

    if (NULL == currentParagraph)
        {
        memset (&origin, 0, sizeof (origin));
        }
    else
        {
        DVec3d displacement = this->GetParagraphSpacing (m_properties.IsVertical (), GetNodeOrFirstRunHeight (), m_properties.GetAnnotationScale (), *currentParagraph, *nextParagraph);

        origin = currentParagraph->GetOrigin ();
        origin.add (&displacement);
        }

    nextParagraph->SetOrigin (origin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/05
//---------------------------------------------------------------------------------------
void TextBlock::AppendNodesNormal (TextBlockNodeArrayR nodeArray)
    {
    ProcessContext  processContext  (this, NULL, NULL);

    processContext.SetProcessLevel (m_processLevel);

    while (!nodeArray.empty ())
        {
        ParagraphP  currentParagraph    = m_paragraphArray.empty () ? NULL : m_paragraphArray.back ();
        ParagraphP  nextParagraph       = dynamic_cast<ParagraphP>(nodeArray[0]);

        if (NULL != nextParagraph)
            {
            if ((NULL == currentParagraph || currentParagraph->IsComplete ()) && m_processLevel <= PROCESS_LEVEL_Paragraph)
                {
                this->SetOriginForNextParagraph (nextParagraph, currentParagraph);
                m_paragraphArray.push_back (nextParagraph);
                nodeArray.erase (nodeArray.begin ());
                }
            else
                {
                if (NULL == currentParagraph || currentParagraph->IsComplete ())
                    {
                    ParagraphP paragraph = new Paragraph (GetDgnModelR ());
                    paragraph->InitFrom (*nextParagraph);
                    this->SetOriginForNextParagraph (paragraph, currentParagraph);
                    m_paragraphArray.push_back (paragraph);
                    }

                nodeArray.erase (nodeArray.begin ());

                for (size_t i = 0; i < nextParagraph->GetLineCount (); i++)
                    nodeArray.insert (nodeArray.begin () + i, nextParagraph->GetLine (i));

                nextParagraph->m_lineArray.clear ();
                delete nextParagraph;

                continue;
                }
            }
        else
            {
            if (NULL == currentParagraph || currentParagraph->IsComplete ())
                {
                nextParagraph = new Paragraph (GetDgnModelR ());
                nextParagraph->SetProperties (m_paragraphPropertiesForAdd);
                this->SetOriginForNextParagraph (nextParagraph, currentParagraph);
                m_paragraphArray.push_back (nextParagraph);
                }
            else
                {
                nextParagraph = currentParagraph;
                }

            nextParagraph->AppendNodes (nodeArray, processContext);

            if (this->GetParagraphCount () > 1)
                this->SetOriginForNextParagraph (nextParagraph, this->GetParagraph (this->GetParagraphCount () - 2));
            }
        }

    this->JustifyLines (m_properties.GetDocumentWidth ());

    m_dirty = this->End ();

    this->ComputeRange (false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
void TextBlock::InsertNodes (CaretR insertLocation, TextBlockNodeArrayR nodeArray)
    {
    TextBlockNodeArray extractedUnits;
    ExtractNodes (insertLocation, extractedUnits);
    
    m_dirty = insertLocation;
    m_processLevel = PROCESS_LEVEL_Run;
    
    AppendNodes (nodeArray);

    insertLocation = this->End ();

    AppendNodes (extractedUnits);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void TextBlock::RecomputeTextBlockOrigin () const
    {
    DVec3d offset;
    offset.zero ();
    
    switch (m_primaryOriginType)
        {
        case ORIGIN_TYPE_AutoCAD:
            {
            // AutoCAD origin is the top middle point for Vertical text and left bottom point for horizontal text
            if (!m_properties.IsVertical ())
                offset.y = m_nominalRange.high.y - m_nominalRange.low.y;
            
            break;
            }

        case ORIGIN_TYPE_Element:
            {
            if (!m_properties.IsVertical ())
                {
                ParagraphCP             paragraph                   = this->GetParagraph (0);
                ParagraphPropertiesCR   paragraphProperties         = (NULL != paragraph) ? paragraph->GetProperties () : this->GetParagraphPropertiesForAdd ();
                LineCP                  line                        = (NULL != paragraph) ? paragraph->GetLine (0) : NULL;
                bool                    lineContainsOnlyWhitespace  = (NULL != line) ? line->ContainsOnlyWhitespace () : true;

                if (paragraphProperties.GetLineSpacingType () == DgnLineSpacingType::AtLeast && !lineContainsOnlyWhitespace)
                    offset.y = line->GetMaxExactHeightAboveOrigin ();
                else if (!m_nominalRange.IsNull ())
                    offset.y = m_nominalRange.high.y - m_nominalRange.low.y;
                }
            
            break;
            }

        case ORIGIN_TYPE_User:
            {
            offset = this->GetJustificationOffset ();
            
            break;
            }

        default:
            return;
        }

    // Final Transforms
    // == [RR' Rt'+t] == [R" t"]

    Transform flipTransform = GetFlipTransform ();

    // R" = rotation
    RotMatrix rotation;
    RotMatrix rMatrix;
    
    flipTransform.getMatrix (&rMatrix);
    rotation.productOf (&m_orientation, &rMatrix);

    // calculate global left top point
    // origin = t"
    DPoint3d origin;
    
    rotation.multiply (&offset);
    origin.sumOf (&m_primaryOrigin, &offset);

    // t = t" - Rt'
    Transform transform;
    transform.initFrom (&m_orientation);

    DPoint3d    flipTranslation;
    DVec3d      flipOffset;
    
    flipTransform.getTranslation (&flipTranslation);
    transform.multiply (&flipOffset, &flipTranslation);

    m_origin.sumOf (&origin, &flipOffset, -1.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/05
//---------------------------------------------------------------------------------------
void TextBlock::Drop (TextBlockNodeArrayR nodeArray)
    {
    for (size_t i = 0; i < GetParagraphCount (); i++)
        {
        TextBlockNodeArray droppedArray;
        GetParagraph (i)->Drop (droppedArray);
        
        nodeArray.insert (nodeArray.end (), droppedArray.begin (), droppedArray.end ());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/05
//---------------------------------------------------------------------------------------
size_t TextBlock::GetLineCount (CaretCR startIn, CaretCR end) const
    {
    int numLines = 0;

    LineIterator endIter = this->End ().CreateLineEnd ();

    for (LineIterator runIter = this->Begin ().CreateLineBegin (); runIter != endIter; ++runIter)
        ++numLines;

    return numLines;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/05
//---------------------------------------------------------------------------------------
size_t TextBlock::GetCharacterCount (CaretCR startIn, CaretCR end) const
    {
    size_t count = 0;

    RunRange runRange (startIn, end);
    FOR_EACH (RunCR run, runRange)
        count += run.GetCharacterCount ();
    
    return count;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/05
//---------------------------------------------------------------------------------------
double TextBlock::GetPaperWidth () const
    {
    RunP run = GetParagraph (0)->GetLine (0)->GetRun (0);
    if (NULL == run)
        return 0.0;

    return (run->GetProperties ().GetFontSize().x * m_properties.GetMaxCharactersPerLine ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
DPoint3d TextBlock::GetOrigin () const
    {
    RecomputeTextBlockOrigin ();
    return m_origin;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/05
//---------------------------------------------------------------------------------------
LineP TextBlock::GetLine (size_t lineNo) const
    {
    if (lineNo >= GetLineCount (this->Begin (), this->End ()))
        return NULL;

    Caret caret (*this);
    
    while (lineNo-- != 0)
        caret.MoveToNextLine ();

    return caret.GetCurrentLineP ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/04
//---------------------------------------------------------------------------------------
ParagraphP TextBlock::GetParagraph (size_t index) const
    {
    if (index >= m_paragraphArray.size ())
        return NULL;
    
    return m_paragraphArray[index];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/05
//---------------------------------------------------------------------------------------
Transform TextBlock::GetFlipTransform () const
    {
    Transform   flipTransform;
    DPoint3d    userOrigin;
    DVec3d      justificationOffset = GetJustificationOffset ();

    userOrigin.zero ();
    userOrigin.sumOf (&userOrigin, &justificationOffset, -1.0);

    flipTransform.initIdentity ();
    
    if (m_properties.IsBackwards ())
        {
        DVec3d      vec;
        RotMatrix   flipMatrix;
        
        vec.x = vec.z = 0.0;
        vec.y = 1.0;
        
        flipMatrix.initFromVectorAndRotationAngle (&vec, msGeomConst_radiansPerDegree * 180.0);
        flipTransform.initFrom (&flipMatrix);
        }

    if (m_properties.IsUpsideDown ())
        {
        DVec3d      vec;
        RotMatrix   flipMatrix;
        
        vec.y = vec.z = 0.0;
        vec.x = 1.0;
        
        flipMatrix.initFromVectorAndRotationAngle (&vec, msGeomConst_radiansPerDegree * 180.0);
        flipTransform.productOf (&flipTransform, &flipMatrix);
        }

    flipTransform.setFixedPoint (&userOrigin);

    return flipTransform;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
Transform TextBlock::GetTransform () const
    {
    DPoint3d    origin      = this->GetOrigin ();
    RotMatrix   rotation    = this->GetOrientation ();
    Transform   transform;
    
    transform.InitFrom (rotation, origin);

    if (m_properties.IsBackwards () || m_properties.IsUpsideDown ())
        {
        DPoint3d    userOrigin;
        DVec3d      justificationOffset = this->GetJustificationOffset ();
        
        userOrigin.Zero ();
        userOrigin.SumOf (userOrigin, justificationOffset, -1.0);
        
        if (m_properties.IsBackwards ())
            {
            DVec3d yVec;
            
            yVec.x = yVec.z = 0.0;
            yVec.y = 1.0;

            RotMatrix flipMatrix;
            flipMatrix.InitFromVectorAndRotationAngle (yVec, msGeomConst_radiansPerDegree * 180.0);

            Transform flipTransform;
            flipTransform.InitFrom (flipMatrix);
            flipTransform.SetFixedPoint (userOrigin);
            
            transform.InitProduct (transform, flipTransform);
            }

        if (m_properties.IsUpsideDown ())
            {
            DVec3d xVec;
            
            xVec.y = xVec.z = 0.0;
            xVec.x = 1.0;

            RotMatrix flipMatrix;
            flipMatrix.InitFromVectorAndRotationAngle (xVec, msGeomConst_radiansPerDegree * 180.0);

            Transform flipTransform;
            flipTransform.InitFrom (flipMatrix);
            flipTransform.SetFixedPoint (userOrigin);
            
            transform.InitProduct (transform, flipTransform);
            }
        }

    return transform;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/06
//---------------------------------------------------------------------------------------
void TextBlock::SetTextNodeProperties (RunPropertiesCP runProperties)
    {
    if (NULL == runProperties)
        {
        delete m_nodeProperties;
        m_nodeProperties = NULL;
        return;
        }

    m_nodeProperties = new RunProperties (*runProperties);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/05
//---------------------------------------------------------------------------------------
void TextBlock::ApplyAnnotationScale (double annotationScale, bool applyAnnotationScaleFlag)
    {
    if (annotationScale < 0)
        return;

    double  currentAnnotationScale      = (m_properties.HasAnnotationScale () ? m_properties.GetAnnotationScale () : 1.0);
    double  annotationScaleChangeFactor = (applyAnnotationScaleFlag ? annotationScale : 1.0) / currentAnnotationScale;
    
    if (fabs (annotationScaleChangeFactor - 1.0) <= mgds_fc_epsilon)
        return;

    if (0.0 != m_properties.GetDocumentWidth ())
        SetLineBreakLength (m_properties.GetDocumentWidth () * annotationScaleChangeFactor);

    RunRange runRange (*this);
    FOR_EACH (RunCR constRun, runRange)
        {
        RunR            run             = const_cast<RunR>(constRun);
        RunProperties   runProperties   = run.GetProperties ();
        
        DPoint2d scaleFactor = { annotationScaleChangeFactor, annotationScaleChangeFactor };
        
        runProperties.ApplyScale (scaleFactor, m_properties.IsVertical ());
        run.SetProperties (runProperties);
        }

    if (NULL != m_nodeProperties)
        {
        DPoint2d scaleFactor = { annotationScaleChangeFactor, annotationScaleChangeFactor };
        m_nodeProperties->ApplyScale (scaleFactor, m_properties.IsVertical ());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
bool TextBlock::SetAnnotationScale (double annotationScale)
    {
    if (annotationScale <= 0.0)
        return false;

    if (fabs (m_properties.m_annotationScale - annotationScale) < mgds_fc_epsilon)
        return false;

    if (m_properties.HasAnnotationScale ())
        {
        ApplyAnnotationScale (annotationScale, m_properties.HasAnnotationScale ());
        
        m_properties.m_annotationScale  = annotationScale;
        m_dirty                         = this->Begin ();
        m_processLevel                  = PROCESS_LEVEL_Character;
        }
    else
        {
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/05
//---------------------------------------------------------------------------------------
bool TextBlock::SetApplyAnnotationScaleFlag (bool applyAnnotationScaleFlag)
    {
    if (m_properties.HasAnnotationScale () == applyAnnotationScaleFlag)
        return false;

    ApplyAnnotationScale (m_properties.m_annotationScale, applyAnnotationScaleFlag);
    
    m_dirty         = this->Begin ();
    m_processLevel  = PROCESS_LEVEL_Character;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void TextBlock::SetParagraphPropertiesForRange (ParagraphPropertiesCR paragraphProperties, ParagraphRangeCR range)
    {
    FOR_EACH (ParagraphCR constParagraph , const_cast<ParagraphRangeR>(range))
        {
        ParagraphR paragraph = const_cast<ParagraphR>(constParagraph);
        
        paragraph.SetProperties (paragraphProperties);
        }
    
    m_dirty = range.GetStartCaret ();
    while (SUCCESS == m_dirty.MoveToPreviousRunInParagraph ());
    
    m_processLevel = PROCESS_LEVEL_Run;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void TextBlock::IsolateRange (CaretR from, CaretR to)
    {
    if (from.IsAt (to))
        return;
    
    if (from.GetCharacterIndex () >= from.GetCurrentRunCP ()->GetCharacterCount ())
        from.MoveToNextRun ();
    
    if (0 == to.GetCharacterIndex ())
        {
        to.MoveToPreviousCharacter ();
        to.SetCharacterIndex (to.GetCharacterIndex () + 1);
        }
    
    // Head and tail already at run boundaries => nothing to do
    if (0 == from.GetCharacterIndex () && to.GetCharacterIndex () >= to.GetCurrentRunCP ()->GetCharacterCount ())
        return;
    
    // Head needs split?
    if (from.GetCharacterIndex () > 0)
        {
        bool    sameLine    = (from.GetCurrentLineCP () == to.GetCurrentLineCP ());
        bool    sameRun     = (from.GetCurrentRunCP () == to.GetCurrentRunCP ());
        
        from.GetCurrentLineP ()->SplitRunInPlace (from);
        
        if (sameLine)
            to.SetRunIndex (to.GetRunIndex () + 1);
        
        if (sameRun)
            to.SetCharacterIndex (to.GetCharacterIndex () - from.GetCharacterIndex ());
        
        from.SetRunIndex (from.GetRunIndex () + 1);
        from.SetCharacterIndex (0);
        }
    
    // Tail needs split?
    if (to.GetCharacterIndex () < to.GetCurrentRunCP ()->GetCharacterCount ())
        to.GetCurrentLineP ()->SplitRunInPlace (to);
    
    m_dirty = from;
    while (SUCCESS == m_dirty.MoveToPreviousRunInParagraph ());
    
    m_processLevel = PROCESS_LEVEL_Run;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void TextBlock::ApplyTextStyleToRunsInRange (DgnTextStyleCR newStyle, bool respectOverrides, CaretCR from_, CaretCR to_)
    {
    if (from_.IsAt (to_))
        return;
    
    Caret   from    = from_;
    Caret   to      = to_;
    
    Caret::EnforceCaretOrder (from, to);
    
    this->IsolateRange (from, to);
    
    RunRange runRange (from, to);
    FOR_EACH (RunCR constRun, runRange)
        {
        RunR                run         = const_cast<RunR>(constRun);
        RunPropertiesPtr    runProps    = run.GetProperties ().Clone ();
        
        runProps->ApplyTextStyle (newStyle, respectOverrides);
        
        run.SetProperties (*runProps);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2010
//---------------------------------------------------------------------------------------
void TextBlock::RemoveTextStyleFromRunsInRange (CaretCR from_, CaretCR to_)
    {
    if (from_.IsAt (to_))
        return;
    
    Caret   from    = from_;
    Caret   to      = to_;
    
    Caret::EnforceCaretOrder (from, to);
    
    this->IsolateRange (from, to);
    
    RunRange runRange (from, to);
    FOR_EACH (RunCR constRun, runRange)
        {
        RunR                run         = const_cast<RunR>(constRun);
        RunPropertiesPtr    runProps    = run.GetProperties ().Clone ();
        
        runProps->RemoveTextStyle ();
        
        run.SetProperties (*runProps);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2010
//---------------------------------------------------------------------------------------
void TextBlock::RevertTextStylesOnRunsInRange (CaretCR from_, CaretCR to_)
    {
    if (from_.IsAt (to_))
        return;
    
    Caret   from    = from_;
    Caret   to      = to_;
    
    Caret::EnforceCaretOrder (from, to);
    
    this->IsolateRange (from, to);
    
    RunRange runRange (from, to);
    FOR_EACH (RunCR run, runRange)
        {
        DgnTextStylePtr fileStyle = run.GetProperties ().GetTextStyleInFile ();
        if (!fileStyle.IsValid ())
            continue;
        
        RunPropertiesPtr newRunProps = run.GetProperties ().Clone ();
        newRunProps->ApplyTextStyle (*fileStyle, false);
        
        const_cast<RunR>(run).SetProperties (*newRunProps);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void TextBlock::SetPropertiesOnRunsInRange (DgnTextStyleCR newStyle, DgnTextStylePropertyMaskCR applyMask, CaretCR from_, CaretCR to_)
    {
    if (from_.IsAt (to_))
        return;
    
    Caret   from    = from_;
    Caret   to      = to_;
    
    Caret::EnforceCaretOrder (from, to);
    
    this->IsolateRange (from, to);
    
    RunRange runRange (from, to);
    FOR_EACH (RunCR constRun, runRange)
        {
        RunR                run         = const_cast<RunR>(constRun);
        RunPropertiesPtr    runProps    = run.GetProperties ().Clone ();
        
        runProps->SetProperties (newStyle, applyMask);
        
        run.SetProperties (*runProps);
        }
    
    // Allow the next PerformLayout to properly re-combine runs based on like-formatting (and otherwise account for formatting size changes).
    m_dirty = from;
    m_dirty.MoveToPreviousRun ();
    
    if (m_processLevel < PROCESS_LEVEL_Run)
        m_processLevel = PROCESS_LEVEL_Run;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void TextBlock::SetPropertiesOnParagraphsInRange (DgnTextStyleCR newStyle, DgnTextStylePropertyMaskCR applyMask, CaretCR from_, CaretCR to_)
    {
    Caret   from    = from_;
    Caret   to      = to_;
    
    Caret::EnforceCaretOrder (from, to);
    
    ParagraphRange paragraphRange (from, to);
    FOR_EACH (ParagraphCR constParagraph, paragraphRange)
        {
        ParagraphR              paragraph       = const_cast<ParagraphR>(constParagraph);
        ParagraphPropertiesPtr  paragraphProps  = paragraph.GetProperties ().Clone ();
        
        paragraphProps->SetProperties (newStyle, applyMask);
        
        paragraph.SetProperties (*paragraphProps);
        }
    
    // Allow the next PerformLayout to properly re-combine runs based on like-formatting (and otherwise account for formatting size changes).
    m_dirty = from;
    m_dirty.MoveToFrontOfParagraph ();
    
    if (m_processLevel < PROCESS_LEVEL_Run)
        m_processLevel = PROCESS_LEVEL_Run;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void TextBlock::AppendParagraphBreak ()
    {
    TextBlockNodeArray nodeArray;
    nodeArray.push_back (new ParagraphBreak (this->GetRunPropertiesForAdd (), this->ComputeRunLayoutFlags ()));
    
    AppendNodes (nodeArray);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void TextBlock::AppendLineBreak ()
    {
    TextBlockNodeArray nodeArray;
    nodeArray.push_back (new LineBreak (this->GetRunPropertiesForAdd (), this->ComputeRunLayoutFlags ()));
    
    AppendNodes (nodeArray);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void TextBlock::AppendTab ()
    {
    TextBlockNodeArray nodeArray;
    nodeArray.push_back (new Tab (this->GetRunPropertiesForAdd (), this->ComputeRunLayoutFlags ()));
    
    AppendNodes (nodeArray);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void TextBlock::AppendFixedSpace (double width)
    {
    RunProperties runProperties = this->GetRunPropertiesForAdd ();
    
    DPoint2d fontSize = runProperties.GetFontSize ();
    fontSize.x = width;
    
    runProperties.SetFontSize (fontSize);

    CharStreamP charStream = new CharStream (L" ", runProperties, this->ComputeRunLayoutFlags ());

    TextBlockNodeArray nodeArray;
    nodeArray.push_back (charStream);
    
    AppendNodes (nodeArray);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/08
//---------------------------------------------------------------------------------------
static void tokenizeStringIntoUnits (TextBlockNodeArrayR newUnits, WCharCP rawChars, RunPropertiesCR runProperties, DgnGlyphRunLayoutFlags layoutFlags)
    {
    if (WString::IsNullOrEmpty (rawChars))
        return;
    
    enum ControlCharacterType
        {
        ControlCharacterType_None,
        ControlCharacterType_CrLf,
        ControlCharacterType_Cr,
        ControlCharacterType_Lf,
        ControlCharacterType_Tab
        };

    const WChar CHAR_CARRIAGERETURN = 0x000D;
    const WChar CHAR_LINEFEED       = 0x000A;
    const WChar CHAR_TAB            = 0x0009;

    size_t                rawCharsCount   = wcslen (rawChars) + 1;
    ScopedArray<WChar>    charsBuffer     (rawCharsCount);
    WCharP                chars           = charsBuffer.GetData ();
    
    BeStringUtilities::Wcsncpy (chars, rawCharsCount, rawChars);

    size_t    numChars    = wcslen (chars);
    WCharCP   unitStart   = chars;
    WCharCP   stop        = chars + numChars;

    WCharP unitIter = chars;
    for (; unitIter < stop; ++unitIter)
        {
        ControlCharacterType controlChar = ControlCharacterType_None;

        if (CHAR_CARRIAGERETURN == *unitIter && (unitIter + 1) < stop && CHAR_LINEFEED == *(unitIter + 1))
            controlChar = ControlCharacterType_CrLf;
        else if (CHAR_CARRIAGERETURN == *unitIter)
            controlChar = ControlCharacterType_Cr;
        else if (CHAR_LINEFEED == *unitIter)
            controlChar = ControlCharacterType_Lf;
        else if (CHAR_TAB == *unitIter)
            controlChar = ControlCharacterType_Tab;

        if (ControlCharacterType_None == controlChar)
            continue;

        *unitIter = 0;
        if (unitStart != unitIter)
            newUnits.push_back (new CharStream (unitStart, runProperties, layoutFlags));

        switch (controlChar)
            {
            case (ControlCharacterType_CrLf):
                newUnits.push_back (new ParagraphBreak (runProperties, layoutFlags));
                unitStart = unitIter + 2;
                unitIter++;
                break;

            case (ControlCharacterType_Cr):
                newUnits.push_back (new ParagraphBreak (runProperties, layoutFlags));
                unitStart = unitIter + 1;
                break;

            case (ControlCharacterType_Lf):
                newUnits.push_back (new LineBreak (runProperties, layoutFlags));
                unitStart = unitIter + 1;
                break;

            case (ControlCharacterType_Tab):
                newUnits.push_back (new Tab (runProperties, layoutFlags));
                unitStart = unitIter + 1;
                break;

            default:
                BeAssert (false);
                break;
            }
        }

    if (unitStart != unitIter)
        newUnits.push_back (new CharStream (unitStart, runProperties, layoutFlags));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/07
//---------------------------------------------------------------------------------------
void TextBlock::AppendText (WCharCP chars)
    {
    if (WString::IsNullOrEmpty (chars))
        { BeAssert (false); return; }
    
    TextBlockNodeArray nodeArray;
    tokenizeStringIntoUnits (nodeArray, chars, m_runPropertiesForAdd, this->ComputeRunLayoutFlags ());

    if (nodeArray.size () > 0)
        this->AppendNodes (nodeArray);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void TextBlock::AppendEnterDataField (WCharCP value, size_t totalLength, EdfJustification edfJustification)
    {
    // Allow an empty string (for an empty EDF; we got a length, so this is perfectly valid).
    if ((NULL == value) || (0 == totalLength))
        { BeAssert (false); return; }

    // EDFs cannot have whitespace control characters, so don't run through tokenizeStringIntoUnits.
    TextBlockNodeArray nodes;
    nodes.push_back (new EdfCharStream (value, totalLength, edfJustification, m_runPropertiesForAdd, this->ComputeRunLayoutFlags ()));

    this->AppendNodes (nodes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
CaretPtr TextBlock::InsertEnterDataField (CaretCR at, WCharCP value, size_t totalLength, EdfJustification edfJustification)
    {
    if (0 == totalLength)
        { BeAssert (false); return at.Clone (); }

    // EDFs cannot have whitespace control characters, so don't run through tokenizeStringIntoUnits.
    TextBlockNodeArray nodes;
    nodes.push_back (new EdfCharStream (value, totalLength, edfJustification, m_runPropertiesForAdd, this->ComputeRunLayoutFlags ()));

    CaretPtr atClone = at.Clone ();
    
    this->InsertNodes (*atClone, nodes);

    return atClone;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2009
//---------------------------------------------------------------------------------------
static FractionP createFractionFromArgs
(
StackedFractionType         fractionType,
WCharCP                     numerator,
WCharCP                     denominator,
RunPropertiesCR             runProperties,
DgnGlyphRunLayoutFlags      layoutFlags,
StackedFractionAlignment    fractionAlignment,
DPoint2dCP                  fractionScale
)
    {
    if (NULL == numerator && NULL == denominator)
        { BeAssert (false); return NULL; }
    
    switch (fractionType)
        {
        case StackedFractionType::NoBar:           return new NoBarFraction            (numerator, denominator, runProperties, layoutFlags, fractionAlignment, fractionScale);
        case StackedFractionType::HorizontalBar:   return new HorizontalBarFraction    (numerator, denominator, runProperties, layoutFlags, fractionAlignment, fractionScale);
        case StackedFractionType::DiagonalBar:     return new DiagonalBarFraction      (numerator, denominator, runProperties, layoutFlags, fractionAlignment, fractionScale);
        }
    
    BeAssert (false);
    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void TextBlock::AppendStackedFraction (WCharCP numerator, WCharCP denominator, StackedFractionType fractionType, StackedFractionAlignment fractionAlignment, DPoint2dCP fractionScale)
    {
    FractionP fraction = createFractionFromArgs (fractionType, numerator, denominator, m_runPropertiesForAdd, this->ComputeRunLayoutFlags (), fractionAlignment, fractionScale);
    if (NULL == fraction)
        return;

    TextBlockNodeArray runArray;
    runArray.push_back (fraction);
    
    AppendNodes (runArray);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void TextBlock::AppendStackedFraction (WCharCP numerator, WCharCP denominator, StackedFractionType fractionType, StackedFractionAlignment fractionAlignment)
    {
    if (NULL == numerator && NULL == denominator)
        { BeAssert (false); return; }
    
    this->AppendStackedFraction (numerator, denominator, fractionType, fractionAlignment, NULL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/07
//---------------------------------------------------------------------------------------
template <typename T>
void TextBlock::SetParagraphProperty (void (Paragraph::*method) (T), T val, ParagraphRangeCR range)
    {
    FOR_EACH (ParagraphCR constParagraph , const_cast<ParagraphRangeR>(range))
        {
        ParagraphP paragraph = const_cast<ParagraphP>(&constParagraph);
        (paragraph->*method) (val);
        }

    m_dirty = range.GetStartCaret ();
    SetProcessLevel (PROCESS_LEVEL_Line);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/08
//---------------------------------------------------------------------------------------
bool TextBlock::ContainsLeadingIndents () const
    {
    for (UInt32 paragraphIndex = 0; paragraphIndex < this->GetParagraphCount (); paragraphIndex++)
        {
        ParagraphCP currParagraph = this->GetParagraph (paragraphIndex);
        if (NULL == currParagraph)
            {
            BeAssert (false);
            continue;
            }

        if (currParagraph->GetProperties().GetIndentation ().GetFirstLineIndent () > 0.0 || currParagraph->GetProperties().GetIndentation ().GetHangingIndent () > 0.0)
            return true;

        for (UInt32 lineIndex = 0; lineIndex < currParagraph->GetLineCount (); lineIndex++)
            {
            LineCP currLine = currParagraph->GetLine (lineIndex);
            if (NULL == currLine)
                {
                BeAssert (false);
                continue;
                }

            if (0 == currLine->GetRunCount ())
                continue;

            if (NULL != dynamic_cast<TabCP>(currLine->GetRun (0)))
                return true;
            }
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void TextBlock::SetCharacterSpacing (double characterSpacing, CharacterSpacingType characterSpacingType, CaretCR start, CaretCR end)
    {
    double annotationScale = m_properties.GetAnnotationScale ();
    
    RunRange runRange (start, end);
    FOR_EACH (RunCR constRun, runRange)
        {
        RunR            run             = const_cast<RunR>(constRun);
        RunProperties   runProperties   = run.GetProperties ();
        
        runProperties.SetCharacterSpacingType (characterSpacingType);
        runProperties.SetCharacterSpacingValue (characterSpacing * annotationScale);
        
        run.SetProperties (runProperties);
        }

    m_processLevel  = PROCESS_LEVEL_Character;
    m_dirty         = this->Begin ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
DPoint3d TextBlock::GetUserOrigin () const
    {
    DPoint3d textUserOrigin;

    if (ORIGIN_TYPE_User != m_primaryOriginType)
        {
        RecomputeTextBlockOrigin ();

        DVec3d justificationOffset = GetJustificationOffset ();

        m_orientation.multiply (&justificationOffset);
        textUserOrigin.sumOf (&m_origin, &justificationOffset, -1.0);
        }
    else
        {
        textUserOrigin = m_primaryOrigin;
        }

    return textUserOrigin;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/05
//---------------------------------------------------------------------------------------
DPoint3d TextBlock::GetTextElementOrigin () const
    {
    DPoint3d textElementOrigin;

    if (ORIGIN_TYPE_Element != m_primaryOriginType)
        {
        RecomputeTextBlockOrigin ();

        DVec3d offset;
        offset.zero ();
        
        if (!m_properties.IsVertical ())
            offset.y = - (m_nominalRange.high.y - m_nominalRange.low.y);

        m_orientation.multiply (&offset);
        textElementOrigin.sumOf (&m_origin, &offset);
        }
    else
        {
        textElementOrigin = m_primaryOrigin;
        }

    return textElementOrigin;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/05
//---------------------------------------------------------------------------------------
DPoint3d TextBlock::GetTextAutoCADOrigin () const
    {
    // This only seems to be called for DTEXT, and DTEXT origin is lower-left, just like our element origin.
    //  Therefore, we can short-circuit if origin type is already AutoCAD or Element. One caveat; DTEXT
    //  vertical's origin is actually effectively lower-center (due to SHX vertical text rendering).

    DPoint3d textAutoCADOrigin;

    if (ORIGIN_TYPE_AutoCAD != m_primaryOriginType && ORIGIN_TYPE_Element != m_primaryOriginType)
        {
        RecomputeTextBlockOrigin ();

        DVec3d offset;
        offset.zero ();

        if (m_properties.IsVertical ())
            offset.x = (m_nominalRange.high.x - m_nominalRange.low.x) / 2.0;
        else
            offset.y = -(m_nominalRange.high.y - m_nominalRange.low.y);

        m_orientation.multiply (&offset);
        textAutoCADOrigin.sumOf (&m_origin, &offset);
        }
    else
        {
        textAutoCADOrigin = m_primaryOrigin;

        RunCP   firstRun        = this->Begin ().GetCurrentRunCP ();
        bool    isFirstRunShx   = false;
        
        if (NULL != firstRun)
            isFirstRunShx = (DgnFontType::Shx == firstRun->GetProperties ().GetFont ().GetType ());
        
        // This needs to mirror DgnImporter::FromTextElement in DgnProcessor (due to special DText/SHX/Vertical positioning).
        if ((ORIGIN_TYPE_AutoCAD != m_primaryOriginType) && this->IsDGNType () && this->m_properties.IsVertical () && isFirstRunShx)
            textAutoCADOrigin.x += ((m_exactRange.high.x - m_exactRange.low.x) / 2.0);
        }

    return textAutoCADOrigin;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/04
//---------------------------------------------------------------------------------------
void TextBlock::PerformLayout ()
    {
    if (PROCESS_LEVEL_TextBlock == m_processLevel)
        {
        // nothing to process
        m_dirty = End ();
        }
    else
        {
        TextBlockNodeArray nodeArray;
        ExtractNodes (m_dirty, nodeArray);

        if (0 != nodeArray.size ())
            AppendNodes (nodeArray);

        m_processLevel = PROCESS_LEVEL_TextBlock;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void TextBlock::Reprocess ()
    {
    m_dirty = this->Begin ();
    PerformLayout ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
size_t TextBlock::ReevaluateFields (EvaluationReason reason)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    size_t nFieldsUpdated = 0;
    CaretPtr cur = CreateStartCaret();
    RunP run;
    TextFieldDataP fieldData;
    while (NULL != (run = cur->GetCurrentRunP()))
        {
        CharStreamP charStream = dynamic_cast<CharStreamP> (run);
        if (NULL != charStream && NULL != (fieldData = charStream->GetFieldData()))
            {
            TextFieldPtr field = TextField::Create (*fieldData, *m_dgnModel);
            BeAssert (field.IsValid() && "TextBlock::ReevaluateFields() failed to create field");
            if (field.IsValid())
                {
                WString updatedDisplayValue = field->GetDisplayValue();
                TextFieldDataPtr updatedFieldData = field->GetTextFieldData();
                BeAssert (updatedFieldData.IsValid() && "TextBlock::ReevaluateFields() could not obtain TextFieldData from field");
                if (updatedFieldData.IsValid())
                    {
                    charStream->SetString (updatedDisplayValue);
                    charStream->SetFieldData (updatedFieldData.get());
                    nFieldsUpdated++;
                    }
                }
            }

        if (SUCCESS != cur->MoveToNextRun())
            break;
        }

    return nFieldsUpdated;
#endif
    return  0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
void TextBlock::ReplaceNodes (CaretR start, CaretR end, TextBlockNodeArrayR unitsToInsert)
    {
    TextBlockNodeArray existingUnitsToKeep;
    this->ExtractNodes (end, existingUnitsToKeep);

    TextBlockNodeArray unitsToBeRemoved;
    this->ExtractNodes (start, unitsToBeRemoved);

    for (TextBlockNodeArray::size_type i = 0; i < unitsToBeRemoved.size (); i++)
        delete unitsToBeRemoved[i];

    m_dirty = start;

    this->AppendNodes (unitsToInsert);

    end = this->End ();
    end.SetCharacterIndex (0);

    this->AppendNodes (existingUnitsToKeep);

    end.MoveToNextRun (); // then advance it forward again after re-adding to be exclusive (vs. inclusive)
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/05
//---------------------------------------------------------------------------------------
void TextBlock::Refit (double fitLength, bool insertSpacesBetweenCharacters)
    {
    Caret caret (*this);

    if (insertSpacesBetweenCharacters)
        {
#if (NEEDS_WORK_RUN)
        SetCharacterSpacing (0.0, CharacterSpacingType::Absolute);
        Reprocess ();

        LineP   pWidestLine = GetWidestLine ();

        Caret       caret (*this);
        int         nUnits = 0;
        while (NULL != (caret.GetNextRun (pWidestLine, caret)))
            nUnits++;

        double      charSpacing = (fitLength - GetWidth ()) / (nUnits-1);
        SetCharacterSpacing (charSpacing, CharacterSpacingType::Absolute);
#endif
        }
    else
        {
        double scaleFactor = (fitLength / (m_properties.IsVertical () ? GetNominalHeight () : GetNominalWidth ()));
        if ((scaleFactor < mgds_fc_epsilon) || (fabs (scaleFactor - 1.0) < mgds_fc_epsilon))
            return;

        RunRange runRange (*this);
        FOR_EACH (RunCR constRun, runRange)
            {
            RunR            run             = const_cast<RunR>(constRun);
            RunProperties   runProperties   = run.GetProperties ();
            DPoint2d        fontSize        = runProperties.GetFontSize ();
            
            fontSize.x *= scaleFactor;
            fontSize.y *= scaleFactor;
            
            runProperties.SetFontSize (fontSize);
            
            run.SetProperties (runProperties);
            }
        }

    SetProcessLevel (PROCESS_LEVEL_Character);
    Reprocess ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/05
//---------------------------------------------------------------------------------------
void TextBlock::GetLineExactLocalRange (DRange3dR range, size_t lineNo) const
    {
    range.init ();
    
    LineP line = GetLine (lineNo);
    
    if (NULL == line)
        return;

    range = line->GetExactRange ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/05
//---------------------------------------------------------------------------------------
void TextBlock::ReplaceFieldsWithNormalString ()
    {
#ifdef BEIJING_DGNPLATFORM_WIP_Fields
    RunRange range (start, end);
    for (RunIterator runIter = range.begin (); range.end () != runIter; ++runIter)
        {
        FieldP field = static_cast <Field*> ((*runIter).FindSpanIf (isField));
        if (NULL != field)
            {
            while ((*runIter).IsPartOfSpan (field) && range.end () != runIter)
                {
                RunR run = const_cast<RunR>(*runIter);
                run.DetachSpan (field);
                ++runIter;
                }

            continue;
            }
        }
#endif // BEIJING_DGNPLATFORM_WIP_Fields
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
DRange3d TextBlock::ComputeTransformedNominalRange () const
    {
    RecomputeTextBlockOrigin ();
    DRange3d range = GetNominalRange ();

    Transform transform = GetTransform ();
    transform.multiply (&range, &range);

    return range;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/06
//---------------------------------------------------------------------------------------
static bool isFlipped (DPoint3dP oldBoxPoints, DPoint3dP newBoxPoints)
    {
    RotMatrix oldRotation;
    RotMatrix newRotation;
    
    oldRotation.initRotationFromOriginXY (oldBoxPoints, oldBoxPoints+1, oldBoxPoints+3);
    newRotation.initRotationFromOriginXY (newBoxPoints, newBoxPoints+1, newBoxPoints+3);

    DVec3d oldZVec;
    DVec3d newZVec;
    
    oldRotation.getColumns (NULL, NULL, &oldZVec);
    newRotation.getColumns (NULL, NULL, &newZVec);

    return (oldZVec.dotProduct (&newZVec) < 0.0 ? true : false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/06
//---------------------------------------------------------------------------------------
TextElementJustification TextBlock::MapJustificationsForMirroring (TextElementJustification justification, bool xFlip, bool yFlip) const
    {
    HorizontalJustification     horizontalJustification;
    VerticalJustification       verticalJustification;
    GetHorizontalVerticalJustifications (&horizontalJustification, &verticalJustification, justification);

    if (xFlip)
        {
        switch (horizontalJustification)
            {
            case HORIZONTAL_JUSTIFICATION_LeftMargin:  horizontalJustification = HORIZONTAL_JUSTIFICATION_RightMargin;    break;
            case HORIZONTAL_JUSTIFICATION_Left:         horizontalJustification = HORIZONTAL_JUSTIFICATION_Right;           break;
            case HORIZONTAL_JUSTIFICATION_Center:                                                                           break;
            case HORIZONTAL_JUSTIFICATION_Right:        horizontalJustification = HORIZONTAL_JUSTIFICATION_Left;            break;
            case HORIZONTAL_JUSTIFICATION_RightMargin: horizontalJustification = HORIZONTAL_JUSTIFICATION_LeftMargin;     break;
            
            default:
                BeAssert (false);
                break;
            }
        }

    if (yFlip)
        {
        switch (verticalJustification)
            {
            case VERTICAL_JUSTIFICATION_Top:        verticalJustification = VERTICAL_JUSTIFICATION_Baseline;  break;
            case VERTICAL_JUSTIFICATION_Middle:                                                             break;
            case VERTICAL_JUSTIFICATION_Baseline:     verticalJustification = VERTICAL_JUSTIFICATION_Top;     break;
            case VERTICAL_JUSTIFICATION_Descender:  verticalJustification = VERTICAL_JUSTIFICATION_Top;     break;
            case VERTICAL_JUSTIFICATION_Cap:                                                                break;
            
            default:
                BeAssert (false);
                break;
            }
        }

    return TextBlock::GetJustificationFromAlignments (horizontalJustification, verticalJustification);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void TextBlock::ComputePlanarRangeVertices (DPoint3d vertices[], DRange3dCR range)
    {
    if (range.isNull ())
        {
        memset (vertices, 0, sizeof (vertices[0]) * 5);
        return;
        }

    vertices[4].x = vertices[0].x = range.low.x;
    vertices[4].y = vertices[0].y = range.low.y;
    vertices[4].z = vertices[0].z = range.low.z;

    vertices[1].x = range.high.x;
    vertices[1].y = range.low.y;
    vertices[1].z = 0.0;

    vertices[2].x = range.high.x;
    vertices[2].y = range.high.y;
    vertices[2].z = range.high.z;

    vertices[3].x = range.low.x;
    vertices[3].y = range.high.y;
    vertices[3].z = 0.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/06
//---------------------------------------------------------------------------------------
void TextBlock::MirrorGetNewParameters (DPoint3dR origin, RotMatrixR rMatrix, TextElementJustification& just, TransformCP pTransform)
    {
    // We want to be able to transform our box, not strictly our text, so nullify the backwards/upside down flags.
    bool backwards  = m_properties.IsBackwards ();
    bool upsideDown = m_properties.IsUpsideDown ();

    // Purposefully not calling setters because we don't want any override flags to change.
    m_properties.m_isBackwards = false;
    m_properties.m_isUpsideDown = false;

    // Cache the current transform.
    Transform currentTransform = this->GetTransform ();

    // Get transformed box points.
    DPoint3d oldBoxPoints[5];
    DRange3d workingRange = this->ComputeJustificationRange ();

    TextBlock::ComputePlanarRangeVertices (oldBoxPoints, workingRange);

    // For horizontally centered text, the low of the working range is not in fact the correct origin point.
    //  Instead of detecting centered text and doing computations to adjust, it should be safe to simply
    //  make the assumed 'origin' built-in to the working range the actual text block origin (upper-left).
    //  Remember that text block origin is in world space, so bring it into nominal AA space.
    Transform invertedCurrTransform;
    invertedCurrTransform.inverseOf (&currentTransform);

    DPoint3d normalizedTextBlockOrigin = this->GetTextOrigin ();
    invertedCurrTransform.multiply (&normalizedTextBlockOrigin);

    oldBoxPoints[0].x = normalizedTextBlockOrigin.x;
    oldBoxPoints[3].x = normalizedTextBlockOrigin.x;

    currentTransform.multiply (oldBoxPoints, 5);

    // Compute the mirrored box based on the transformed existing box.
    DPoint3d newBoxPoints[5];
    memcpy (newBoxPoints, oldBoxPoints, sizeof (DPoint3d) * 5);
    pTransform->Multiply (newBoxPoints, 5);

    // Determine if we flipped.
    bool flipped = isFlipped (oldBoxPoints, newBoxPoints);

    // Compute what way we flipped.
    DVec3d originalXVec;
    originalXVec.differenceOf (oldBoxPoints+1, oldBoxPoints);
    originalXVec.normalize ();

    DVec3d finalXVec;
    finalXVec.differenceOf (newBoxPoints+1, newBoxPoints);
    finalXVec.normalize ();

    double dotP = originalXVec.dotProduct (&finalXVec);

    bool xFlip = false;
    bool yFlip = false;

    if (!flipped)
        {
        origin = newBoxPoints[3];
        rMatrix.initRotationFromOriginXY (newBoxPoints, newBoxPoints+1, newBoxPoints+3);
        }
    else
        {
        if (dotP > mgds_fc_epsilon)  // y-flip (horizontal)
            {
            origin = newBoxPoints[0];
            rMatrix.initRotationFromOriginXY (newBoxPoints+3, newBoxPoints+2, newBoxPoints);
            yFlip = true;
            }
        else                    // x-flip (vertical)
            {
            origin = newBoxPoints[2];
            rMatrix.initRotationFromOriginXY (newBoxPoints+1, newBoxPoints, newBoxPoints+2);
            xFlip = true;
            }

        // Re-map justifications.
        UInt32 nParagraphs = this->GetParagraphCount ();
        for (UInt32 i = 0; i < nParagraphs; i++)
            {
            ParagraphP pParagraph = this->GetParagraph (i);
            pParagraph->SetJustification (MapJustificationsForMirroring (pParagraph->GetJustification (), xFlip, yFlip));
            }

        m_dirty = this->Begin ();
        this->SetProcessLevel (PROCESS_LEVEL_Line);
        }

    // Restore the flags we nullified earlier.
    m_properties.m_isBackwards  = backwards;
    m_properties.m_isUpsideDown = upsideDown;

    // When going from left to right, must compute left-side bearing shift prior
    //  to Reprocess because that will potentially move space characters around
    //  for word-wrapping due to a horizontal justification change, which will
    //  throw off the exact range computation (omits trailing but not leading spaces).

    DRange3d    nominalRange    = GetNominalRange ();
    DRange3d    exactRange      = GetExactRange ();
    double      lsbShift        = (exactRange.low.x - nominalRange.low.x);

    Reprocess ();

    // If we didn't do a vertical flip (x-coordinate flip), nothing further to do as
    //  top and bottom "bearings" do not differ based on justification.

    if (!xFlip)
        return;

    // Otherwise we need to take into account left- and center-justified text use the left-side
    //  bearing when performing layout, whereas right-justified text does not use the right-side bearing.
    //  Therefore, in order to get an exact visual match, we must remove the left-side bearing in
    //  our calculations.

    just = MapJustificationsForMirroring (just, xFlip, yFlip);

    HorizontalJustification hJust;
    GetHorizontalVerticalJustifications (&hJust, NULL, just);

    // Word-wrapping is fun because flipping isn't strictly based on our box; when changing
    //  justifications, it's important to use the full box since text will bump to the opposite side.
    if (m_properties.GetDocumentWidth () > 0.0)
        {
        switch (hJust)
            {
            case (HORIZONTAL_JUSTIFICATION_Right):
                {
                lsbShift += m_properties.GetDocumentWidth () - (workingRange.high.x - workingRange.low.x);

                break;
                }
            case (HORIZONTAL_JUSTIFICATION_Center):
                {
                double leftOver     = m_properties.GetDocumentWidth () - (workingRange.high.x - workingRange.low.x);
                double leftOverHalf = leftOver / 2.0;

                lsbShift += leftOverHalf;

                break;
                }
            case (HORIZONTAL_JUSTIFICATION_Left):
                {
                nominalRange    = GetNominalRange ();
                exactRange      = GetExactRange ();
                lsbShift        = (exactRange.low.x - nominalRange.low.x);

                break;
                }
            }
        }

    // Our text could be rotated; can't just adjust the X-value.
    DVec3d xVec;
    xVec.init (-lsbShift, 0.0, 0.0);
    rMatrix.multiply (&xVec);
    origin.add (&xVec);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/06
//---------------------------------------------------------------------------------------
static bool extractTransformParameters (double& xScale, double& yScale, TransformR transform)
    {
    RotMatrix rMatrix;
    transform.getMatrix (&rMatrix);

    DVec3d xCol;
    DVec3d yCol;
    
    rMatrix.getColumn (&xCol, 0);
    rMatrix.getColumn (&yCol, 1);

    xScale = xCol.magnitude ();
    yScale = yCol.magnitude ();

    return !((fabs (xScale - 1.0) < mgds_fc_epsilon) && (fabs (xScale - 1.0) < mgds_fc_epsilon));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/06
//---------------------------------------------------------------------------------------
void TextBlock::Scale (DPoint2dCR scale)
    {
    RunRange runRange (*this);
    FOR_EACH (RunCR constRun, runRange)
        {
        RunR            run             = const_cast<RunR>(constRun);
        RunProperties   runProperties   = run.GetProperties ();
        
        runProperties.ApplyScale (scale, m_properties.IsVertical ());
        
        run.SetProperties (runProperties);
        }

    ParagraphRange paragraphRange (*this);
    FOR_EACH (ParagraphCR constParagraph, paragraphRange)
        {
        ParagraphR paragraph = const_cast<ParagraphR>(constParagraph);
        paragraph.Scale (scale, m_properties.IsVertical ());
        }

    m_properties.SetDocumentWidth (m_properties.GetDocumentWidth () * (m_properties.IsVertical () ? scale.y : scale.x));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/06
//---------------------------------------------------------------------------------------
void TextBlock::ApplyTransform (TransformCR transform)
    {
    DPoint2d    scale;
    Transform   unscaledTransform   = transform;
    bool        scalingPresent      = extractTransformParameters (scale.x, scale.y, unscaledTransform);
    RotMatrix   rMatrix;            unscaledTransform.GetMatrix (rMatrix);

    if (scalingPresent)
        {
        this->Scale (scale);
        this->SetProcessLevel (PROCESS_LEVEL_Character);
        }

    if (rMatrix.determinant () < 0.0)
        {
        DPoint3d                    origin;
        TextElementJustification    justification = this->GetParagraph (0)->GetJustification ();
        
        this->MirrorGetNewParameters (origin, m_orientation, justification, &unscaledTransform);
        this->SetTextOrigin (origin);
        }
    else
        {
        Transform newTransform = this->GetTransform ();
        newTransform.InitProduct (unscaledTransform, newTransform);

        DPoint3d origin;
        newTransform.GetTranslation (origin);
        
        this->SetTextOrigin (origin);

        newTransform.GetMatrix (m_orientation);
        m_orientation.ScaleRows (m_orientation, 1.0 / scale.x, 1.0 / scale.y, 1.0);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/06
//---------------------------------------------------------------------------------------
void TextBlock::MoveChars (CaretR caretIn, int nUnits) const
    {
    Caret caret (caretIn);
    
    for (int i = 0; i < nUnits; i++)
        {
        caret.MoveToNextCharacter ();
        
        if (caret.IsAtEnd ())
            break;
        }

    caretIn = caret;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/07
//---------------------------------------------------------------------------------------
BentleyStatus TextBlock::ComputeCaretAtLocation (CaretR caret, DPoint3dCR locationIn, bool isStrict) const
    {
    if (this->IsEmpty ())
        {
        caret = this->Begin ();
        return (isStrict ? ERROR : SUCCESS);
        }
    
    Transform transform = this->GetTransform ();
    transform.inverseOf (&transform);

    DPoint3d location = locationIn;
    transform.multiply (&location);
    
    bool isVertical = m_properties.IsVertical ();

    for (size_t i = 0; i < this->GetParagraphCount (); ++i)
        {
        ParagraphP  paragraph   = this->GetParagraph (i);
        DRange3d    range;
        
        range.Init ();
        paragraph->ComputeTransformedHitTestRange (range);
        
        // Vertical glyphs are centered based on font size. Thus, their range does not necessarily include the origin (in X).
        //  We need to be able to round-trip ComputeCaretParameters with this method, and that method will utilize origins.
        if (isVertical)
            range.Extend (paragraph->GetOrigin ());
        
        bool    containsPoint               = isVertical ? location.x < range.high.x : location.y >= range.low.y;
        bool    dontConsiderNextParagraph   = false;

        if (containsPoint && isStrict)
            {
            containsPoint &= isVertical ? location.x > range.low.x : location.y < range.high.y;
            dontConsiderNextParagraph = true;
            }
        
        if (containsPoint)
            {
            caret.SetParagraphIndex (i);
            return paragraph->ComputeCaretAtLocation (caret, location, isVertical, isStrict);
            }
        
        if (dontConsiderNextParagraph)
            return ERROR;
        }
    
    if (isStrict)
        return ERROR;

    // We're off the last paragraph... do we have a virtual paragraph at the bottom to go to?
    Caret endCaret = this->End ();
    endCaret.MoveToNextCharacter ();
    if (endCaret.IsOnVirtualLine ())
        {
        caret = endCaret;
        return SUCCESS;
        }
    
    caret.SetParagraphIndex (m_paragraphArray.size () - 1);
    return m_paragraphArray.back ()->ComputeCaretAtLocation (caret, location, isVertical, isStrict);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/07
//---------------------------------------------------------------------------------------
WString TextBlock::ToString () const
    {
    return this->ToString (this->Begin (), this->End (), *TextBlockToStringOptions::CreateDefault ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/07
//---------------------------------------------------------------------------------------
WString TextBlock::ToString (CaretCR from, CaretCR to) const
    {
    return this->ToString (from, to, *TextBlockToStringOptions::CreateDefault ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/07
//---------------------------------------------------------------------------------------
WString TextBlock::ToString (CaretCR from_, CaretCR to_, TextBlockToStringOptionsCR options) const
    {
    if (to_.IsAt (from_))
        return L"";
    
    Caret   from    = from_;
    Caret   to      = to_;

    Caret::EnforceCaretOrder (from, to);

    WString         accumulatedString;
    WString         runString;
    RunCP           firstRunToProcess   = from.GetCurrentRunCP ();
    RunCP           lastRunToProcess    = to.GetCurrentRunCP ();
    
    RunRange runRange (from, to);
    FOR_EACH (RunCR run, runRange)
        {
        // Short-term, only substr CharStream's; they are the only DOM node than can contain more than one character for these purposes
        //  (e.g. a fraction node can return a string greater than one character, but as an atomic unit, reports a length of 1 character and should not be split).
        //  Since this is a query operation, you are allowed to go into other atomic runs (e.g. EDFs and fields).

        if ((NULL != dynamic_cast<CharStreamCP>(&run)) && ((&run == firstRunToProcess) || (&run == lastRunToProcess)))
            {
            size_t  startOffset = ((&run == firstRunToProcess) ? from.GetCharacterIndex () : 0);
            size_t  count       = ((&run == lastRunToProcess) ? (to.GetCharacterIndex () - startOffset) : WString::npos);

            runString = run.ToString (startOffset, count, options);
            }
        else
            {
            runString = run.ToString (0, WString::npos, options);
            }
        
        accumulatedString.append (runString);
        }
    
    return accumulatedString.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/07
//---------------------------------------------------------------------------------------
Caret TextBlock::Begin () const
    {
    Caret caret (*this);
    return caret;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/07
//---------------------------------------------------------------------------------------
CaretPtr TextBlock::CreateStartCaret () const
    {
    return new Caret (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/07
//---------------------------------------------------------------------------------------
Caret TextBlock::End () const
    {
    if (this->IsEmpty ())
        return Caret (*this);
    
    size_t      paraIndex   = m_paragraphArray.size () - 1;
    ParagraphCP paragraph   = m_paragraphArray[paraIndex];
    size_t      lineIndex   = (NULL != paragraph) ? paragraph->GetLineCount () - 1 : 0;
    LineCP      line        = (NULL != paragraph) ? paragraph->GetLine (lineIndex) : NULL;
    size_t      runIndex    = (NULL != line) ? line->GetRunCount () - 1 : 0;
    RunCP       run         = (NULL != line) ? line->GetRun (runIndex) : NULL;
    size_t      charIndex   = (NULL != run) ? run->GetCharacterCount () : 0;
    
    Caret caret (*this);
    caret.SetParagraphIndex (paraIndex);
    caret.SetLineIndex (lineIndex);
    caret.SetRunIndex (runIndex);
    caret.SetCharacterIndex (charIndex);
    
    return caret;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/07
//---------------------------------------------------------------------------------------
CaretPtr TextBlock::CreateEndCaret () const
    {
    CaretP caret = new Caret (this->End ());
    return caret;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
void TextBlock::Draw (ViewContextR context) const
    {
    this->Draw (context, *TextBlockDrawOptions::CreateDefault ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/07
//---------------------------------------------------------------------------------------
void TextBlock::Draw (ViewContextR context, TextBlockDrawOptionsCR options) const
    {
    if (this->IsEmpty ())
        return;
    
    Transform transform = this->GetTransform ();
    context.PushTransform (transform);
        {
        for (size_t i = 0; i < m_paragraphArray.size (); ++i)
            m_paragraphArray[i]->Draw (context, m_properties.IsViewIndependent (), options);
        }
    context.PopTransformClip ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/08
//---------------------------------------------------------------------------------------
double TextBlock::GetNodeHeight () const
    {
    if (NULL != m_nodeProperties)
        return m_nodeProperties->GetFontSize ().y;

    return 0.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/08
//---------------------------------------------------------------------------------------
double TextBlock::GetNodeOrFirstRunHeight () const
    {
    // If node properties exist, return their height.
    if (NULL != m_nodeProperties)
        return m_nodeProperties->GetFontSize ().y;

    // There are several cases during a blank TextBlock's life that we may end up here
    //  (e.g. to compute initial spacing characteristics while adding the first run).
    //  If we are empty, with no node properties, 0.0 is the only reasonable thing.
    if (IsEmpty ())
        return 0.0;

    // Otherwise we must get the height of the first run.
    RunCP firstRun = NULL;
    if (GetParagraphCount () > 0)
        {
        ParagraphCR firstParagraph = *GetParagraph (0);
        if (firstParagraph.GetLineCount () > 0)
            {
            LineCR firstLine = *firstParagraph.GetLine (0);
            if (firstLine.GetRunCount () > 0)
                firstRun = firstLine.GetRun (0);
            }
        }

    if (NULL == firstRun)
        {
        BeAssert (false);
        return 0.0;
        }

    return firstRun->GetProperties ().GetFontSize ().y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/08
//---------------------------------------------------------------------------------------
bool TextBlock::IsAlongElement () const
    {
#if defined (NEEDS_WORK_DGNITEM)
    if (0 == m_alongRootId)
        return false;

    if (NULL == m_dgnModel)
        { BeAssert (false); return false; }
    
    AlongTextDependency atDep;
    
    if (SUCCESS != this->GetAlongTextDependency (atDep) || !atDep.IsValid (*m_dgnModel))
        return false;

    return true;
#endif
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/08
//---------------------------------------------------------------------------------------
void TextBlock::RemoveAlongElementDependency ()
    {
    m_alongRootId       = 0;
    m_descrOffset       = 0.0;
    m_alongDist         = 0.0;
    m_descrSide         = DESCRIPTOR_SIDE_Above;
    m_useAlongDist      = false;
    m_alongDgnModel     = NULL;

    m_descrStartPoint.zero ();

    // Even though the dirty settings below will force a full layout, when we re-add the character streams, we will not re-enforce their orientation (by design, somewhat arbitrarily).
    RunRange runRange (*this);
    FOR_EACH (RunCR run, runRange)
        const_cast<RunR>(run).SetOrientation (m_orientation);

    m_dirty         = this->Begin ();
    m_processLevel  = PROCESS_LEVEL_Character;
    }

#ifdef BEIJING_DGNPLATFORM_WIP_Fields
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/08
//---------------------------------------------------------------------------------------
FieldCollection TextBlock::GetFields () const
    {
    }
#endif // BEIJING_DGNPLATFORM_WIP_Fields

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
bool TextBlock::Equals (TextBlockCR rhs, TextBlockCompareOptionsCR compareOptions) const
    {
    // Purposefully not comparing DgnModels... it would be a lot of extra work to remap everything,
    //  and I just don't think it will affect many cases. Even in the QATools case where we have
    //  difference DgnModels, they should still have the same units and color table etc.
    
    if (!compareOptions.ShouldIgnoreInternalState ())
        {
        if (m_V7Compatible  != rhs.m_V7Compatible)  return false;
        if (m_forceTextNode != rhs.m_forceTextNode) return false;
        if (m_processLevel  != rhs.m_processLevel)  return false;
        if (m_alongDgnModel != rhs.m_alongDgnModel) return false;
        
        if (!m_dirty.IsAt (rhs.m_dirty)) return false;
        
        if (!m_paragraphPropertiesForAdd.Equals (rhs.m_paragraphPropertiesForAdd, compareOptions.GetTolerance ()))                                          return false;
        if (!m_runPropertiesForAdd.Equals       (rhs.m_runPropertiesForAdd, compareOptions.GetTolerance (), compareOptions.ShouldIgnoreElementOverhead ())) return false;
        }
    
    if (!compareOptions.ShouldIgnoreCachedValues ())
        {
        if (m_primaryOriginType != rhs.m_primaryOriginType) return false;
        
        if (!m_primaryOrigin.IsEqual    (rhs.m_primaryOrigin,   compareOptions.GetTolerance ()))    return false;
        if (!m_nominalRange.IsEqual     (rhs.m_nominalRange,    compareOptions.GetTolerance ()))    return false;
        if (!m_exactRange.IsEqual       (rhs.m_exactRange,      compareOptions.GetTolerance ()))    return false;
        }
    
    if (m_type              != rhs.m_type)              return false;
    if (m_nodeNumber        != rhs.m_nodeNumber)        return false;
    if (m_alongRootId       != rhs.m_alongRootId)       return false;
    if (m_descrSide         != rhs.m_descrSide)         return false;
    if (m_useAlongDist      != rhs.m_useAlongDist)      return false;
    
    if ((NULL == m_nodeProperties) != (NULL == rhs.m_nodeProperties))                                                                                                       return false;
    if (((NULL != m_nodeProperties) && !m_nodeProperties->Equals (*rhs.m_nodeProperties, compareOptions.GetTolerance (), compareOptions.ShouldIgnoreElementOverhead ())))   return false;
    
    if (!compareOptions.AreDoublesEqual (m_fittedLength,    rhs.m_fittedLength))    return false;
    if (!compareOptions.AreDoublesEqual (m_descrOffset,     rhs.m_descrOffset))     return false;
    if (!compareOptions.AreDoublesEqual (m_alongDist,       rhs.m_alongDist))       return false;
    
    if (!m_properties.Equals        (rhs.m_properties,      compareOptions.GetTolerance ()))    return false;
    if (!m_orientation.IsEqual      (rhs.m_orientation,     compareOptions.GetTolerance ()))    return false;
    if (!m_origin.IsEqual           (rhs.m_origin,          compareOptions.GetTolerance ()))    return false;
    if (!m_descrStartPoint.IsEqual  (rhs.m_descrStartPoint, compareOptions.GetTolerance ()))    return false;
    
    if (m_paragraphArray.size () != rhs.m_paragraphArray.size ())
        return false;
    
    for (ParagraphArray::const_iterator lhsIter = m_paragraphArray.begin (), rhsIter = rhs.m_paragraphArray.begin (); m_paragraphArray.end () != lhsIter; ++lhsIter, ++rhsIter)
        if (!(*lhsIter)->Equals (**rhsIter, compareOptions))
            return false;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
CaretPtr TextBlock::InsertText (CaretCR caret, WCharCP text)
    {
    if (WString::IsNullOrEmpty (text))
        { BeAssert (false); return caret.Clone (); }
    
    TextBlockNodeArray nodes;
    tokenizeStringIntoUnits (nodes, text, m_runPropertiesForAdd, this->ComputeRunLayoutFlags ());
    
    if (nodes.empty ())
        return caret.Clone ();
    
    Caret caretCopy = caret;
    
    // WIP_WordWrapping
    // This is about the most brute-force way to do this, but will have to suffice without a more fundamental shift in word-wrapping support
    //  (e.g. the ability to work backwards from the insertion point and re-break existing text to fit new letters -- which is essentially what this workaround does, but in an over-aggressive way).
    double docWidth = m_properties.GetDocumentWidth ();
    if (docWidth > 0.0)
        m_properties.SetDocumentWidth (0.0);
    
    this->InsertNodes (caretCopy, nodes);

    if (docWidth > 0.0)
        {
        size_t charOffset = caretCopy.ComputeGlobalCharacterOffset ();

        m_dirty = caret;
        m_processLevel = PROCESS_LEVEL_Character;
        m_properties.SetDocumentWidth (docWidth);
        
        if (SUCCESS != m_dirty.MoveToPreviousLine ())
            m_dirty.MoveToFrontOfLine ();
        
        this->PerformLayout ();

        caretCopy.GoToGlobalCharacterIndex (charOffset);
        }

    return caretCopy.Clone ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2009
//---------------------------------------------------------------------------------------
void TextBlock::InsertParagraphBreak (CaretCR caret)
    {
    TextBlockNodeArray nodes;
    nodes.push_back (new ParagraphBreak (m_runPropertiesForAdd, this->ComputeRunLayoutFlags ()));
    
    Caret caretCopy = caret;
    
    this->InsertNodes (caretCopy, nodes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2009
//---------------------------------------------------------------------------------------
void TextBlock::InsertLineBreak (CaretCR caret)
    {
    TextBlockNodeArray nodes;
    nodes.push_back (new LineBreak (m_runPropertiesForAdd, this->ComputeRunLayoutFlags ()));
    
    Caret caretCopy = caret;
    
    this->InsertNodes (caretCopy, nodes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2009
//---------------------------------------------------------------------------------------
void TextBlock::InsertTab (CaretCR caret)
    {
    TextBlockNodeArray nodes;
    nodes.push_back (new Tab (m_runPropertiesForAdd, this->ComputeRunLayoutFlags ()));
    
    Caret caretCopy = caret;
    
    this->InsertNodes (caretCopy, nodes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2009
//---------------------------------------------------------------------------------------
CaretPtr TextBlock::InsertStackedFraction (CaretCR caret, WCharCP numeratorText, WCharCP denominatorText, StackedFractionType fractionType, StackedFractionAlignment fractionAlignment)
    {
    FractionP fraction = createFractionFromArgs (fractionType, numeratorText, denominatorText, m_runPropertiesForAdd, this->ComputeRunLayoutFlags (), fractionAlignment, NULL);
    if (NULL == fraction)
        return caret.Clone ();

    TextBlockNodeArray nodes;
    nodes.push_back (fraction);
    
    Caret caretCopy = caret;
    
    this->InsertNodes (caretCopy, nodes);

    return caretCopy.Clone ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
CaretPtr TextBlock::Remove (CaretCR from_, CaretCR to_)
    {
    Caret   from    = from_;
    Caret   to      = to_;

    Caret::EnforceCaretOrder (from, to);
    
    TextBlockNodeArray leftOverNodes;
    
    this->ExtractNodes (to, leftOverNodes);
    this->RemoveNodes (from);
    
    // 'from' can easily become invalid when removing across runs; setting it here will be sufficient in many cases, to avoid the reset below if it otherwise becomes invalid.
    from = this->End ();

    this->AppendNodes (leftOverNodes);
    
    if (!from.IsValid ())
        from = this->End ();
    
    m_dirty = from;

    // WIP_WordWrapping
    // This is about the most brute-force way to do this, but will have to suffice without a more fundamental shift in word-wrapping support
    //  (e.g. the ability to work backwards from the insertion point and re-break existing text to fit new letters -- which is essentially what this workaround does, but in an over-aggressive way).
    if (m_properties.GetDocumentWidth () > 0.0)
        {
        size_t charOffset   = from.ComputeGlobalCharacterOffset ();
        m_dirty             = from;
        m_processLevel      = PROCESS_LEVEL_Character;
        
        if (SUCCESS != m_dirty.MoveToPreviousLine ())
            m_dirty.MoveToFrontOfLine ();
        
        this->PerformLayout ();

        from.GoToGlobalCharacterIndex (charOffset);
        }

    return new Caret (from);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void TextBlock::ProcessRangeForSelectionBox (bvector<DRange3d>& rects, CaretCR runCaret, DRange3dR range, size_t lineStartIdx) const
    {
    static const double ASCENDER_RANGE_PADDING = 0.1;
    
    if (range.IsNull ())
        return;
    
    if (m_properties.IsVertical ())
        {
        range.high.x = range.low.x + runCaret.GetCurrentLineCP ()->GetNominalWidth ();
        range.low.y -= runCaret.GetCurrentLineCP ()->GetMaxDescender ();
        }
    else
        {
        RunCP   run             = runCaret.GetCurrentRunCP ();
        double  fontHeight      = run->GetNominalHeight ();
        double  descenderRatio  = ((NULL != dynamic_cast<FractionCP>(run)) ? 0.0 : 0.3);
        
        range.high.y    += fontHeight * ASCENDER_RANGE_PADDING;
        range.low.y     -= fontHeight * descenderRatio;
        }
    
    // Given the padding above, look out for very narrow line spacing; drawing one selection range on top of another results in blending multiplication (e.g. brighter areas), so don't allow ranges between lines to cross.
    Caret lineCaret = runCaret;
    if (SUCCESS == lineCaret.MoveToNextLine ())
        {
        DPoint3d lineOrigin;
        lineOrigin.Zero ();
        this->GetLineTransformAtCaret (lineCaret).Multiply (&lineOrigin, &lineOrigin, 1);
            
        double nextLineTop = (lineOrigin.y + (lineCaret.GetCurrentLineCP ()->GetNominalHeight () * (1.0 + ASCENDER_RANGE_PADDING)));
        if (range.low.y < nextLineTop)
            range.low.y = nextLineTop;
        }
    
    this->GetRunTransformAtCaret (runCaret).Multiply (range, range);
    
    rects.push_back (range);
    
    // Ensure all rects on a line have same height.
    if ((rects.size () - 1) == lineStartIdx)
        return;
    
    DRange3d unionedRange;
    unionedRange.Init ();
    
    for (size_t i = lineStartIdx; i < rects.size (); ++i)
        unionedRange.UnionOf (unionedRange, rects[i]);
    
    for (size_t i = lineStartIdx; i < rects.size (); ++i)
        {
        rects[i].low.y  = unionedRange.low.y;
        rects[i].high.y = unionedRange.high.y;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void TextBlock::ComputeSelectionBoxes (bvector<DRange3d>& rects, CaretCR fromIn, CaretCR toIn) const
    {
    rects.clear ();
    
    if (fromIn.IsAt (toIn))
        return;
    
    Caret   from    (fromIn);
    Caret   to      (toIn);
    
    Caret::EnforceCaretOrder (from, to);
    
    // Easy if same run.
    if (from.GetCurrentRunCP () == to.GetCurrentRunCP ())
        {
        bvector<DRange3d> ranges;
        from.GetCurrentRunCP ()->GetRangesForSelection (ranges, from.GetCharacterIndex (), to.GetCharacterIndex ());
        
        for (bvector<DRange3d>::iterator rangesIter = ranges.begin (); ranges.end () != rangesIter; ++rangesIter)
            this->ProcessRangeForSelectionBox (rects, from, *rangesIter, 0);
        
        return;
        }
        
    // We have multiple runs... get sub-range of first and last runs, as well as any complete runs in between.
    
    bvector<DRange3d>   ranges;
    LineCP              lineMark            = from.GetCurrentLineCP ();
    size_t              lineMarkStartIdx    = 0;
    
    from.GetCurrentRunCP ()->GetRangesForSelection (ranges, from.GetCharacterIndex (), from.GetCurrentRunCP ()->GetCharacterCount ());
    for (bvector<DRange3d>::iterator rangesIter = ranges.begin (); ranges.end () != rangesIter; ++rangesIter)
        this->ProcessRangeForSelectionBox (rects, from, *rangesIter, lineMarkStartIdx);
    
    from.MoveToNextRun ();
    
    for (Caret runIter = from; runIter.GetCurrentRunCP () != to.GetCurrentRunCP (); runIter.MoveToNextRun ())
        {
        // Union height (width if vertical) of all ranges on a given line.
        if (runIter.GetCurrentLineCP () != lineMark)
            {
            lineMark            = runIter.GetCurrentLineCP ();
            lineMarkStartIdx    = rects.size ();
            }
        
        ranges.clear ();
        runIter.GetCurrentRunCP ()->GetRangesForSelection (ranges, runIter.GetCharacterIndex (), runIter.GetCurrentRunCP ()->GetCharacterCount ());
        for (bvector<DRange3d>::iterator rangesIter = ranges.begin (); ranges.end () != rangesIter; ++rangesIter)
            this->ProcessRangeForSelectionBox (rects, runIter, *rangesIter, lineMarkStartIdx);
        }
    
    if (to.GetCurrentLineCP () != lineMark)
        lineMarkStartIdx = rects.size ();
    
    ranges.clear ();
    to.GetCurrentRunCP ()->GetRangesForSelection (ranges, 0, to.GetCharacterIndex ());
    for (bvector<DRange3d>::iterator rangesIter = ranges.begin (); ranges.end () != rangesIter; ++rangesIter)
        this->ProcessRangeForSelectionBox (rects, to, *rangesIter, lineMarkStartIdx);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void TextBlock::InsertFromTextBlock (CaretCR from, CaretCR to, CaretCR at)
    {
    if (&from.GetTextBlock () != &to.GetTextBlock ())
        { BeAssert (false && L"TextBlocks of 'from' and 'to' carets must match."); return; }
    
    if (&at.GetTextBlock () != this)
        { BeAssert (false && L"TextBlock of 'at' caret must match this."); return; }
    
    BeAssert ((from.GetTextBlock ().m_dgnModel == m_dgnModel) && L"TextBlocks of source and destination should match DgnModels.");
    
    if (from.IsAt (to))
        return;
    
    Caret fromClone = from;
    Caret toClone   = to;
    Caret::EnforceCaretOrder (fromClone, toClone);
    
    // Yes, this is somewhat expensive, but gets to re-use a lot of complicated code to extract the highest level of nodes as possible, and splits nodes appropriately.
    TextBlockPtr copyOfSource = from.GetTextBlock ().Clone ();
    copyOfSource->RemoveNodes (toClone);
    
    TextBlockNodeArray nodes;
    copyOfSource->ExtractNodes (fromClone, nodes);
    
    Caret atClone = at;
    this->InsertNodes (atClone, nodes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2011
//---------------------------------------------------------------------------------------
static void applyPaddingFromBackground (RunCP run, bool isForHorizontal, double& maxPadding, ViewportCR viewport)
    {
    if (NULL == run)
        return;
        
    RunPropertiesCR runProps = run->GetProperties ();
    
    if (!runProps.ShouldUseBackground ())
        return;
        
    UInt32      bgWeight;
    DPoint2d    bgPadding;
    runProps.GetBackgroundStyle (NULL, NULL, NULL, &bgWeight, &bgPadding);

    double padding = (isForHorizontal ? bgPadding.x : bgPadding.y);
    
    double weightInUors = (viewport.GetIndexedLineWidth (bgWeight) * viewport.GetPixelSizeAtPoint (NULL));
    
    padding += (weightInUors / 2.0);
    
    maxPadding = std::max (maxPadding, padding);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
static void applyPaddingFromOverline (RunCP run, double& maxPadding, ViewportCR viewport)
    {
    if (NULL == run)
        return;
        
    RunPropertiesCR runProps = run->GetProperties ();
    
    if (!runProps.IsOverlined ())
        return;
        
    UInt32 weight;
    runProps.GetOverlineStyle (NULL, NULL, &weight);

    double padding = runProps.GetOverlineOffset ();
    
    double weightInUors = (viewport.GetIndexedLineWidth (weight) * viewport.GetPixelSizeAtPoint (NULL));
    
    padding += (weightInUors / 2.0);
    
    maxPadding = std::max (maxPadding, padding);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
static void applyPaddingFromUnderline (RunCP run, double& maxPadding, ViewportCR viewport)
    {
    if (NULL == run)
        return;
        
    RunPropertiesCR runProps = run->GetProperties ();
    
    if (!runProps.IsUnderlined ())
        return;
        
    UInt32 weight;
    runProps.GetUnderlineStyle (NULL, NULL, &weight);

    double padding = runProps.GetUnderlineOffset ();
    
    double weightInUors = (viewport.GetIndexedLineWidth (weight) * viewport.GetPixelSizeAtPoint (NULL));
    
    padding += (weightInUors / 2.0);
    
    maxPadding = std::max (maxPadding, padding);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
static void applyPaddingFromSubScript (RunCP run, double& maxPadding)
    {
    if (!run->GetProperties ().IsSubScript ())
        return;
    
    maxPadding = std::max (maxPadding, (run->GetProperties ().GetFontSize ().y * RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SHIFT));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
static void applyPaddingFromSuperScript (RunCP run, double& maxPadding)
    {
    if (!run->GetProperties ().IsSuperScript ())
        return;
    
    // (FontHeight * (1 - Shift)) + (FontHeight * Scale) - FontHeight
    double superScriptPadding = (run->GetProperties ().GetFontSize ().y * (RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SCALE - RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SHIFT));
    
    maxPadding = std::max (maxPadding, superScriptPadding);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
static void applyPaddingFromRunOffset (TextBlockCR textBlock, double& maxPadding, bool isTop)
    {
    if (textBlock.IsEmpty ())
        return;
        
    Caret runIter = textBlock.Begin ();
    do
        {
        RunCR       run             = *runIter.GetCurrentRunCP ();
        DPoint2d    runTopRight;
        
        runTopRight.x = (run.GetOrigin ().x + run.GetNominalWidth () + run.GetProperties ().GetRunOffset ().x);
        runTopRight.y = (run.GetOrigin ().y + run.GetNominalHeight () + run.GetProperties ().GetRunOffset ().y);
        
        runIter.GetCurrentLineCP ()->GetTransform ().Multiply (&runTopRight, &runTopRight, 1);
        runIter.GetCurrentParagraphCP ()->GetTransform ().Multiply (&runTopRight, &runTopRight, 1);

        if (isTop)
            {
            if (runTopRight.y > maxPadding)
                maxPadding = runTopRight.y;
            
            continue;
            }
        
        double runRightPad = (runTopRight.x - textBlock.GetNominalWidth ());
        
        if (runRightPad > maxPadding)
            maxPadding = runRightPad;
        }
        while (SUCCESS == runIter.MoveToNextRun ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2011
//---------------------------------------------------------------------------------------
static double computeMaxLeftDecorationPadding (TextBlockCR textBlock, ViewportCR viewport)
    {
    double maxPadding = 0.0;
    
    LineRange lineRange (textBlock);
    FOR_EACH (LineCR line, lineRange)
        applyPaddingFromBackground (line.GetRun (0), true, maxPadding, viewport);
    
    return maxPadding;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2011
//---------------------------------------------------------------------------------------
static double computeMaxTopDecorationPadding (TextBlockCR textBlock, ViewportCR viewport)
    {
    double  maxPadding  = 0.0;
    Caret   from        = textBlock.Begin ();
    Caret   to          = from;

    to.MoveToBackOfLine ();
    
    RunRange runRange (from, to);
    FOR_EACH (RunCR run, runRange)
        {
        applyPaddingFromBackground (&run, false, maxPadding, viewport);
        applyPaddingFromOverline (&run, maxPadding, viewport);
        applyPaddingFromSuperScript (&run, maxPadding);
        }
    
    applyPaddingFromRunOffset (textBlock, maxPadding, true);
    
    return maxPadding;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2011
//---------------------------------------------------------------------------------------
static double computeMaxRightDecorationPadding (TextBlockCR textBlock, ViewportCR viewport)
    {
    double maxPadding = 0.0;
    
    LineRange lineRange (textBlock);
    FOR_EACH (LineCR line, lineRange)
        applyPaddingFromBackground (line.GetRun (line.GetRunCount () - 1), true, maxPadding, viewport);
    
    applyPaddingFromRunOffset (textBlock, maxPadding, false);

    return maxPadding;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2011
//---------------------------------------------------------------------------------------
static double computeMaxBottomDecorationPadding (TextBlockCR textBlock, ViewportCR viewport)
    {
    double  maxPadding  = 0.0;
    Caret   to          = textBlock.End ();
    Caret   from        = to;

    from.MoveToFrontOfLine ();
    
    RunRange runRange (from, to);
    FOR_EACH (RunCR run, runRange)
        {
        applyPaddingFromBackground (&run, false, maxPadding, viewport);
        applyPaddingFromUnderline (&run, maxPadding, viewport);
        applyPaddingFromSubScript (&run, maxPadding);
        }
    
    return maxPadding;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
DRange3d TextBlock::ComputeEditorRange (DPadding* decorationPadding, ViewportCR viewport) const
    {
    // Nominal range would be wonderful... but we can't go cutting off descenders at the bottom now can we...
    //  I also don't want to use exact range, because I don't want things shifting when you add/remove descenders.
    //  Using font descender ratio should encompass the descenders, and provide a stable "exact" range for the same purposes.
    //  I think we will also end up having to accurately display the entire background border (and its padding).
    
    DRange3d        editorRange             = this->GetNominalRange ();
    RunCP           lastRun                 = this->End ().GetCurrentRunCP ();
    RunPropertiesCR effectiveLastRunProps   = ((NULL != lastRun) ? lastRun->GetProperties () : m_runPropertiesForAdd);
    
    if (m_properties.GetDocumentWidth () > 0.0)
        {
        editorRange.low.x   = 0.0;
        editorRange.high.x  = std::max (editorRange.high.x, m_properties.GetDocumentWidth ());
        }

    editorRange.low.y -= (effectiveLastRunProps.GetFontSize ().y * effectiveLastRunProps.ResolveFont ().GetDescenderRatio ());

    DPadding localPadding;
    if (NULL == decorationPadding)
        decorationPadding = &localPadding;

    decorationPadding->m_left   = computeMaxLeftDecorationPadding (*this, viewport);
    editorRange.low.x           -= decorationPadding->m_left;
    
    decorationPadding->m_top    = computeMaxTopDecorationPadding (*this, viewport);
    editorRange.high.y          += decorationPadding->m_top;
    
    decorationPadding->m_right  = computeMaxRightDecorationPadding (*this, viewport);
    editorRange.high.x          += decorationPadding->m_right;
    
    decorationPadding->m_bottom = computeMaxBottomDecorationPadding (*this, viewport);
    editorRange.low.y           -= decorationPadding->m_bottom;
    
    if (!this->IsEmpty () && !this->End ().IsOnVirtualLine ())
        return editorRange;
    
    Paragraph trailingParagraph (*m_dgnModel);
    trailingParagraph.SetProperties (m_paragraphPropertiesForAdd);
    this->SetOriginForNextParagraph (&trailingParagraph, (m_paragraphArray.empty () ? NULL : m_paragraphArray.back ()));
    
    // We basically need this for determining caret height, thus I don't think it's terribly important what character we use.
    RunP                trailingRun = new CharStream (L"A", m_runPropertiesForAdd, this->ComputeRunLayoutFlags ());
    ProcessContext      context     (this, &trailingParagraph, NULL);
    TextBlockNodeArray  nodes;
    
    nodes.push_back (trailingRun);
    
    trailingParagraph.AppendNodes (nodes, context);
    
    Caret danglingCaret (*this);
    danglingCaret.SetParagraphIndex (m_paragraphArray.size ());
    
    editorRange.UnionOf (editorRange, trailingParagraph.ComputeTransformedNominalRange ());
    editorRange.low.y -= (trailingRun->GetProperties ().GetFontSize ().y * trailingRun->GetProperties ().ResolveFont ().GetDescenderRatio ());

    return editorRange;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2010
//---------------------------------------------------------------------------------------
static WChar toUpperCallback (WChar lhsChar, WChar currChar)
    {
    return towupper (currChar);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2010
//---------------------------------------------------------------------------------------
static WChar toLowerCallback (WChar lhsChar, WChar currChar)
    {
    return towlower (currChar);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2010
//---------------------------------------------------------------------------------------
static WChar toTitleCallback (WChar lhsChar, WChar currChar)
    {
    if ((0 == lhsChar) || iswspace (lhsChar))
        return towupper (currChar);
    
    return towlower (currChar);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2010
//---------------------------------------------------------------------------------------
static WChar toFirstCapitalCallback (WChar lhsChar, WChar currChar)
    {
    if (0 == lhsChar)
        return towupper (currChar);
    
    return towlower (currChar);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker   12/10
//---------------------------------------------------------------------------------------
bool TextBlock::IsInField (CaretCR caret) const
    {
    CharStreamCP currRunAsCharStream = dynamic_cast<CharStreamCP>(caret.GetCurrentRunCP ());
    if (NULL == currRunAsCharStream)
        return false;
    
    return (NULL != currRunAsCharStream->GetFieldData ());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2010
//---------------------------------------------------------------------------------------
void TextBlock::ChangeCaseInternal (CaretCR _from, CaretCR _to, WChar (*changeCaseCallback)(WChar, WChar))
    {
    Caret   from    = _from;
    Caret   to      = _to;
    
    // This is acceptable case where carets are bad.
    if (this->IsEmpty ())
        return;

    if ((NULL == from.GetCurrentRunCP ()) || (NULL == to.GetCurrentRunCP ()))
        { BeAssert (false && L"Bad carets provided."); return; }

    if (from.GetCurrentRunCP ()->GetCharacterCount () == from.GetCharacterIndex ())
        from.MoveToNextCharacter ();
    
    if (to.GetCurrentRunCP ()->GetCharacterCount () == to.GetCharacterIndex ())
        to.MoveToNextCharacter ();
    
    Caret::EnforceCaretOrder (from, to);
    
    CharStreamP currCharStream  = NULL;
    WString     currString;
    
    for (CaretPtr charIter = from.Clone (); charIter->IsBefore (to); charIter->MoveToNextCharacter ())
        {
        RunP        run             = const_cast<RunP>(charIter->GetCurrentRunCP ());
        CharStreamP runAsCharStream = dynamic_cast<CharStreamP>(run);
        
        if ((NULL == runAsCharStream) || this->IsInField (*charIter))
            continue;
        
        if (runAsCharStream != currCharStream)
            {
            if (NULL != currCharStream)
                {
                currCharStream->SetString (currString);
                currString.clear ();
                }
            
            currCharStream  = runAsCharStream;
            currString      = runAsCharStream->GetString ();
            }
        
        currString[charIter->GetCharacterIndex ()] = changeCaseCallback (charIter->GetPreviousCharacter (), charIter->GetNextCharacter ());
        }
    
    currCharStream->SetString (currString);
    
    m_processLevel  = PROCESS_LEVEL_Character;
    m_dirty         = from;
    
    this->PerformLayout ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2010
//---------------------------------------------------------------------------------------
void TextBlock::ChangeCase (CaretCR from, CaretCR to, ChangeCaseOperation op)
    {
    switch (op)
        {
        case CHANGE_CASE_OPERATION_AllUpper:        this->ChangeCaseInternal (from, to, toUpperCallback);           return;
        case CHANGE_CASE_OPERATION_AllLower:        this->ChangeCaseInternal (from, to, toLowerCallback);           return;
        case CHANGE_CASE_OPERATION_Title:           this->ChangeCaseInternal (from, to, toTitleCallback);           return;
        case CHANGE_CASE_OPERATION_FirstCapital:    this->ChangeCaseInternal (from, to, toFirstCapitalCallback);    return;
        
        default:
            BeAssert (false && L"Unknown ChangeCaseOperation.");
            return;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
void TextBlock::ComputeElementRange (DRange3dR value) const
    {
    value.Init ();

    FOR_EACH (ParagraphCP paragraph, m_paragraphArray)
        {
        DRange3d paragraphElementRange;
        paragraph->ComputeElementRange (paragraphElementRange);

        paragraph->GetTransform ().Multiply (paragraphElementRange, paragraphElementRange);

        value.Extend (&paragraphElementRange.low, 2);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2010
//---------------------------------------------------------------------------------------
DgnGlyphRunLayoutFlags TextBlock::ComputeRunLayoutFlags () const
    {
    DgnGlyphRunLayoutFlags flags = GLYPH_RUN_LAYOUT_FLAG_None;
    
    if (m_properties.IsVertical ())
        flags = (DgnGlyphRunLayoutFlags)(flags | GLYPH_RUN_LAYOUT_FLAG_Vertical);
    
    if (m_properties.IsBackwards ())
        flags = (DgnGlyphRunLayoutFlags)(flags | GLYPH_RUN_LAYOUT_FLAG_Backwards);
    
    if (m_properties.IsUpsideDown ())
        flags = (DgnGlyphRunLayoutFlags)(flags | GLYPH_RUN_LAYOUT_FLAG_UpsideDown);
    
    if (m_isForEditing)
        flags = (DgnGlyphRunLayoutFlags)(flags | GLYPH_RUN_LAYOUT_FLAG_IsForEditing);

    return flags;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2011
//---------------------------------------------------------------------------------------
void TextBlock::SetIsForEditing (bool value)
    {
    if (m_isForEditing == value)
        return;

    m_isForEditing = value;

    DgnGlyphRunLayoutFlags layoutFlags = this->ComputeRunLayoutFlags ();

    RunRange runRange (*this);
    FOR_EACH (RunCR run, runRange)
        const_cast<RunR>(run).SetRunLayoutFlags (layoutFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawTextBlock (TextBlockCR text)
    {
    text.Draw (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/05
//---------------------------------------------------------------------------------------
static void getAdjustedLineTransform (TransformR lineTransform, DgnLineSpacingType lineSpacingType, LineCR line, TextBlockCR textBlock)
    {
    lineTransform = line.GetTransform ();
    
    if (!textBlock.GetProperties ().IsVertical ())
        return;

    DVec3d baselineDisplacement;
    line.ComputeBaselineDisplacement (baselineDisplacement, lineSpacingType, textBlock);
    baselineDisplacement.scale (-1.0);

    DPoint3d translation;
    translation.zero ();
    translation.add (&baselineDisplacement);

    Transform baselineDisplacementTransform;
    baselineDisplacementTransform.initFrom (&translation);
    lineTransform.productOf (&lineTransform, &baselineDisplacementTransform);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/05
//---------------------------------------------------------------------------------------
Transform TextBlock::GetLineTransformAtCaret (CaretCR caret) const
    {
    ParagraphCR paragraph           = *caret.GetCurrentParagraphCP ();
    Transform   paragraphTransform  = paragraph.GetTransform ();
    LineCR      line                = *caret.GetCurrentLineCP ();
    Transform   lineTransform;      getAdjustedLineTransform (lineTransform, paragraph.GetLineSpacingType (), line, *this);
    Transform   localTransform;     localTransform.InitProduct (paragraphTransform, lineTransform);
    Transform   transform;          transform.InitProduct (this->GetTransform (), localTransform);
    
    return transform;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
Transform TextBlock::GetRunTransformAtCaret (CaretCR caret) const
    {
    RunCR       run             = *caret.GetCurrentRunCP ();
    Transform   runTransform    = run.GetTransform ();
    Transform   transform;      transform.InitProduct (this->GetLineTransformAtCaret (caret), runTransform);
    
    return transform;
    }

