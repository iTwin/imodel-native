//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationParagraph.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationParagraph::AnnotationParagraph(DgnDbR project) :
    T_Super()
    {
    m_dgndb = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationParagraphPtr AnnotationParagraph::Create(DgnDbR project, AnnotationTextStyleId styleID)
    {
    auto par = AnnotationParagraph::Create(project);
    par->SetStyleId(styleID, SetAnnotationTextStyleOptions::Direct);

    return par;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationParagraphPtr AnnotationParagraph::Create(DgnDbR project, AnnotationTextStyleId styleID, AnnotationRunBaseR run)
    {
    auto par = AnnotationParagraph::Create(project, styleID);
    par->GetRunsR().push_back(&run);

    return par;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationParagraph::CopyFrom(AnnotationParagraphCR rhs)
    {
    m_dgndb = rhs.m_dgndb;
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
void AnnotationParagraph::SetStyleId(AnnotationTextStyleId value, SetAnnotationTextStyleOptions options)
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
