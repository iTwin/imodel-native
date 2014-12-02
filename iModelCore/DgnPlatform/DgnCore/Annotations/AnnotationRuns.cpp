//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationRuns.cpp $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationRunBase::AnnotationRunBase(DgnProjectR project) :
    T_Super()
    {
    m_project = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationRunBase::AnnotationRunBase(AnnotationRunBaseCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
AnnotationRunBaseR AnnotationRunBase::operator=(AnnotationRunBaseCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
void AnnotationRunBase::CopyFrom(AnnotationRunBaseCR rhs)
    {
    m_project = rhs.m_project;
    m_styleID = rhs.m_styleID;
    m_styleOverrides = rhs.m_styleOverrides;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationRunBasePtr AnnotationRunBase::Clone() const { return _Clone(); }
AnnotationRunType AnnotationRunBase::GetType() const { return _GetType(); }

DgnProjectR AnnotationRunBase::GetDgnProjectR() const { return *m_project; }
DgnStyleId AnnotationRunBase::GetStyleId() const { return m_styleID; }
AnnotationTextStylePtr AnnotationRunBase::CreateEffectiveStyle() const { return m_project->Styles().AnnotationTextStyles().QueryById(m_styleID)->CreateEffectiveStyle(m_styleOverrides); }
AnnotationTextStylePropertyBagCR AnnotationRunBase::GetStyleOverrides() const { return m_styleOverrides; }
AnnotationTextStylePropertyBagR AnnotationRunBase::GetStyleOverridesR() { return m_styleOverrides; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationRunBase::SetStyleId(DgnStyleId value, SetAnnotationTextStyleOptions options)
    {
    m_styleID = value;

    if (!isEnumFlagSet(SetAnnotationTextStyleOptions::PreserveOverrides, options))
        m_styleOverrides.ClearAllProperties();
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextRunPtr AnnotationTextRun::Create(DgnProjectR project) { return new AnnotationTextRun(project); }
AnnotationTextRun::AnnotationTextRun(DgnProjectR project) :
    T_Super(project)
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextRunPtr AnnotationTextRun::Create(DgnProjectR project, DgnStyleId styleID)
    {
    auto run = AnnotationTextRun::Create(project);
    run->SetStyleId(styleID, SetAnnotationTextStyleOptions::Direct);

    return run;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextRunPtr AnnotationTextRun::Create(DgnProjectR project, DgnStyleId styleID, Utf8CP content)
    {
    auto run = AnnotationTextRun::Create(project, styleID);
    run->SetContent(content);

    return run;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationTextRunPtr AnnotationTextRun::CloneAsTextRun() const { return new AnnotationTextRun(*this); }
AnnotationRunBasePtr AnnotationTextRun::_Clone() const { return CloneAsTextRun(); }
AnnotationTextRun::AnnotationTextRun(AnnotationTextRunCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
AnnotationTextRunR AnnotationTextRun::operator=(AnnotationTextRunCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
void AnnotationTextRun::CopyFrom(AnnotationTextRunCR rhs)
    {
    m_content = rhs.m_content;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationRunType AnnotationTextRun::_GetType() const { return AnnotationRunType::Text; }
Utf8StringCR AnnotationTextRun::GetContent() const { return m_content; }
void AnnotationTextRun::SetContent(Utf8CP value) { m_content = value; }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationFractionRunPtr AnnotationFractionRun::Create(DgnProjectR project) { return new AnnotationFractionRun(project); }
AnnotationFractionRun::AnnotationFractionRun(DgnProjectR project) :
    T_Super(project)
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationFractionRunPtr AnnotationFractionRun::Create(DgnProjectR project, DgnStyleId styleID)
    {
    auto run = AnnotationFractionRun::Create(project);
    run->SetStyleId(styleID, SetAnnotationTextStyleOptions::Direct);

    return run;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationFractionRunPtr AnnotationFractionRun::Create(DgnProjectR project, DgnStyleId styleID, Utf8CP numerator, Utf8CP denominator)
    {
    auto run = AnnotationFractionRun::Create(project, styleID);
    run->SetDenominatorContent(denominator);
    run->SetNumeratorContent(numerator);

    return run;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationFractionRunPtr AnnotationFractionRun::CloneAsFractionRun() const { return new AnnotationFractionRun(*this); }
AnnotationRunBasePtr AnnotationFractionRun::_Clone() const { return CloneAsFractionRun(); }
AnnotationFractionRun::AnnotationFractionRun(AnnotationFractionRunCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
AnnotationFractionRunR AnnotationFractionRun::operator=(AnnotationFractionRunCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
void AnnotationFractionRun::CopyFrom(AnnotationFractionRunCR rhs)
    {
    m_numeratorContent = rhs.m_numeratorContent;
    m_denominatorContent = rhs.m_denominatorContent;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationRunType AnnotationFractionRun::_GetType() const { return AnnotationRunType::Fraction; }
Utf8StringCR AnnotationFractionRun::GetDenominatorContent() const { return m_denominatorContent; }
void AnnotationFractionRun::SetDenominatorContent(Utf8CP value) { m_denominatorContent = value; }
Utf8StringCR AnnotationFractionRun::GetNumeratorContent() const { return m_numeratorContent; }
void AnnotationFractionRun::SetNumeratorContent(Utf8CP value) { m_numeratorContent = value; }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationLineBreakRunPtr AnnotationLineBreakRun::Create(DgnProjectR project) { return new AnnotationLineBreakRun(project); }
AnnotationLineBreakRun::AnnotationLineBreakRun(DgnProjectR project) :
    T_Super(project)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationLineBreakRunPtr AnnotationLineBreakRun::Create(DgnProjectR project, DgnStyleId styleID)
    {
    auto run = AnnotationLineBreakRun::Create(project);
    run->SetStyleId(styleID, SetAnnotationTextStyleOptions::Direct);

    return run;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationLineBreakRunPtr AnnotationLineBreakRun::CloneAsLineBreakRun() const { return new AnnotationLineBreakRun(*this); }
AnnotationRunBasePtr AnnotationLineBreakRun::_Clone() const { return CloneAsLineBreakRun(); }
AnnotationLineBreakRun::AnnotationLineBreakRun(AnnotationLineBreakRunCR rhs) : T_Super(rhs) { }
AnnotationLineBreakRunR AnnotationLineBreakRun::operator=(AnnotationLineBreakRunCR rhs) { T_Super::operator=(rhs); return *this;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationRunType AnnotationLineBreakRun::_GetType() const { return AnnotationRunType::LineBreak; }
