//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationParagraph.cpp $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationParagraphPtr AnnotationParagraph::Create(DgnProjectR project) { return new AnnotationParagraph(project); }
AnnotationParagraph::AnnotationParagraph(DgnProjectR project) :
    T_Super()
    {
    m_project = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationParagraphPtr AnnotationParagraph::Create(DgnProjectR project, DgnStyleId styleID)
    {
    auto par = AnnotationParagraph::Create(project);
    par->SetStyleId(styleID, SetAnnotationTextStyleOptions::Direct);

    return par;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationParagraphPtr AnnotationParagraph::Create(DgnProjectR project, DgnStyleId styleID, AnnotationRunBaseR run)
    {
    auto par = AnnotationParagraph::Create(project, styleID);
    par->GetRunsR().push_back(&run);

    return par;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationParagraphPtr AnnotationParagraph::Clone() const { return new AnnotationParagraph(*this); }
AnnotationParagraph::AnnotationParagraph(AnnotationParagraphCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
AnnotationParagraphR AnnotationParagraph::operator=(AnnotationParagraphCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
void AnnotationParagraph::CopyFrom(AnnotationParagraphCR rhs)
    {
    m_project = rhs.m_project;
    m_styleID = rhs.m_styleID;
    m_styleOverrides = rhs.m_styleOverrides;

    m_runs.clear();
    m_runs.reserve(rhs.m_runs.size());
    for (auto const& rhsRun : rhs.m_runs)
        m_runs.push_back(rhsRun->Clone());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
DgnProjectR AnnotationParagraph::GetDgnProjectR() const { return *m_project; }
DgnStyleId AnnotationParagraph::GetStyleId() const { return m_styleID; }
AnnotationTextStylePtr AnnotationParagraph::CreateEffectiveStyle() const { return m_project->Styles().AnnotationTextStyles().QueryById(m_styleID)->CreateEffectiveStyle(m_styleOverrides); }
AnnotationTextStylePropertyBagCR AnnotationParagraph::GetStyleOverrides() const { return m_styleOverrides; }
AnnotationTextStylePropertyBagR AnnotationParagraph::GetStyleOverridesR() { return m_styleOverrides; }
AnnotationRunCollectionCR AnnotationParagraph::GetRuns() const { return m_runs; }
AnnotationRunCollectionR AnnotationParagraph::GetRunsR() { return m_runs; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationParagraph::SetStyleId(DgnStyleId value, SetAnnotationTextStyleOptions options)
    {
    m_styleID = value;

    if (!isEnumFlagSet(SetAnnotationTextStyleOptions::PreserveOverrides, options))
        m_styleOverrides.ClearAllProperties();
    
    if (!isEnumFlagSet(SetAnnotationTextStyleOptions::DontPropogate, options))
        {
        for (auto& run : m_runs)
            run->SetStyleId(value, options);
        }
    }
