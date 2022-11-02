//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/Annotations/Annotations.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

USING_NAMESPACE_BENTLEY_DGN

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationRunBase::AnnotationRunBase(DgnDbR project) :
    T_Super()
    {
    m_dgndb = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AnnotationRunBase::CopyFrom(AnnotationRunBaseCR rhs)
    {
    m_dgndb = rhs.m_dgndb;
    m_styleID = rhs.m_styleID;
    m_styleOverrides = rhs.m_styleOverrides;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AnnotationRunBase::SetStyleId(DgnElementId value, SetAnnotationTextStyleOptions options)
    {
    m_styleID = value;

    if (!isEnumFlagSet(SetAnnotationTextStyleOptions::PreserveOverrides, options))
        m_styleOverrides.ClearAllProperties();
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationTextRun::AnnotationTextRun(DgnDbR project) :
    T_Super(project),
    m_subsuperscript(AnnotationTextRunSubSuperScript::Neither)
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationTextRunPtr AnnotationTextRun::Create(DgnDbR project, DgnElementId styleID)
    {
    auto run = AnnotationTextRun::Create(project);
    run->SetStyleId(styleID, SetAnnotationTextStyleOptions::Direct);

    return run;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationTextRunPtr AnnotationTextRun::Create(DgnDbR project, DgnElementId styleID, Utf8CP content)
    {
    auto run = AnnotationTextRun::Create(project, styleID);
    run->SetContent(content);

    return run;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AnnotationTextRun::CopyFrom(AnnotationTextRunCR rhs)
    {
    m_content = rhs.m_content;
    m_subsuperscript = rhs.m_subsuperscript;
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationFractionRun::AnnotationFractionRun(DgnDbR project) :
    T_Super(project)
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationFractionRunPtr AnnotationFractionRun::Create(DgnDbR project, DgnElementId styleID)
    {
    auto run = AnnotationFractionRun::Create(project);
    run->SetStyleId(styleID, SetAnnotationTextStyleOptions::Direct);

    return run;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationFractionRunPtr AnnotationFractionRun::Create(DgnDbR project, DgnElementId styleID, Utf8CP numerator, Utf8CP denominator)
    {
    auto run = AnnotationFractionRun::Create(project, styleID);
    run->SetDenominatorContent(denominator);
    run->SetNumeratorContent(numerator);

    return run;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AnnotationFractionRun::CopyFrom(AnnotationFractionRunCR rhs)
    {
    m_numeratorContent = rhs.m_numeratorContent;
    m_denominatorContent = rhs.m_denominatorContent;
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationLineBreakRun::AnnotationLineBreakRun(DgnDbR project) :
    T_Super(project)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationLineBreakRunPtr AnnotationLineBreakRun::Create(DgnDbR project, DgnElementId styleID)
    {
    auto run = AnnotationLineBreakRun::Create(project);
    run->SetStyleId(styleID, SetAnnotationTextStyleOptions::Direct);

    return run;
    }
