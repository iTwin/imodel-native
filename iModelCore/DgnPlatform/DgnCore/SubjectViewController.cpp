/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SubjectViewController.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/SubjectViewController.h>
#include <DgnPlatform/DgnDbTables.h>

USING_NAMESPACE_BENTLEY_DGN

//-------------------------------------------------------------------------------------------
// @bsimethod 												Diego.Pinate 	10/17
//-------------------------------------------------------------------------------------------
SubjectViewController::SubjectViewController(SpatialViewDefinition const& view, DgnDbP db, SubjectColorMap const& subjectColors)
    : SpatialViewController(view), m_db(db)
    {
    m_subjectColors = subjectColors;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod 												Diego.Pinate 	10/17
//-------------------------------------------------------------------------------------------
SubjectCPtr GetParentJobSubjectRecursive(DgnDbP db, ECClassCP subjectClass, DgnElementCPtr element)
    {
    if (element == nullptr)
        return nullptr;

    ECClassCP elementClass = element->GetElementClass();
    if (elementClass->Is(subjectClass))
        {
        SubjectCPtr candidate = db->Elements().Get<Subject>(element->GetElementId());
        BeAssert(candidate.IsValid());
        if (JobSubjectUtils::IsJobSubject(*candidate))
            return candidate;
        }
    
    return GetParentJobSubjectRecursive(db, subjectClass, db->Elements().GetElement(element->GetParentId()));
    }

//-------------------------------------------------------------------------------------------
// @bsimethod 												Diego.Pinate 	10/17
//-------------------------------------------------------------------------------------------
SubjectCPtr SubjectViewController::GetParentJobSubject(DgnElementCP element)
    {
    ECClassCP subjectClass = m_db->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Subject);
    return GetParentJobSubjectRecursive(m_db, subjectClass, element);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod 												Diego.Pinate 	10/17
//-------------------------------------------------------------------------------------------
void    SubjectViewController::_OverrideGraphicParams(Dgn::Render::OvrGraphicParamsR ovr, Dgn::GeometrySourceCP source)
    {
    // Find the job subject of the element and colorize based on it
    DgnElementCP element = source->ToElement();
    SubjectCPtr jobSubject = GetParentJobSubject(element);
    BeAssert(jobSubject.IsValid());
    BeAssert(m_subjectColors.find(jobSubject->GetElementId()) != m_subjectColors.end());
    ovr = m_subjectColors[jobSubject->GetElementId()];
    }

//-------------------------------------------------------------------------------------------
// @bsimethod 												Diego.Pinate 	10/17
//-------------------------------------------------------------------------------------------
void    SubjectViewController::ToggleVisibility(DgnElementId subjectId, bool isVisible)
    {
    m_subjectColors[subjectId].SetLineTransparency(isVisible ? 255 : 0);
    m_subjectColors[subjectId].SetFillTransparency(isVisible ? 255 : 0);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod 												Diego.Pinate 	10/17
//-------------------------------------------------------------------------------------------
void    SubjectViewController::SetOverrides(DgnElementId subjectId, Render::OvrGraphicParamsR overrides)
    {
    m_subjectColors[subjectId] = overrides;
    }