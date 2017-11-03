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
    m_elementToSubjectStmt = new ECSqlStatement();
    m_elementToSubjectStmt->Prepare(*db, "SELECT s.ECInstanceId FROM Bis.Subject s \
                    JOIN Bis.SubjectOwnsPartitionElements sope ON sope.SourceECInstanceId=s.ECInstanceId \
                    JOIN Bis.ModelModelsElement mme ON sope.TargetECInstanceId = mme.TargetECInstanceId \
                    JOIN Bis.ModelContainsElements mce ON mme.SourceECInstanceId=mce.SourceECInstanceId \
                    WHERE mce.TargetECInstanceId=?");
    }

//-------------------------------------------------------------------------------------------
// @bsimethod 												Diego.Pinate 	11/17
//-------------------------------------------------------------------------------------------
SubjectViewController::~SubjectViewController()
    {
    delete m_elementToSubjectStmt;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod 												Diego.Pinate 	11/17
//-------------------------------------------------------------------------------------------
void    SubjectViewController::SetModelsAndCategoriesVisibility(bool visible)
    {
    CachedECSqlStatementPtr modelStmt = m_db->GetPreparedECSqlStatement("SELECT m.ECInstanceId FROM Bis.SpatialModel m");
    while (modelStmt->Step() == BE_SQLITE_ROW)
        ChangeModelDisplay(modelStmt->GetValueId<DgnModelId>(0), visible);

    CachedECSqlStatementPtr categoryStmt = m_db->GetPreparedECSqlStatement("SELECT c.ECInstanceId FROM Bis.Category c");
    while (categoryStmt->Step() == BE_SQLITE_ROW)
        ChangeCategoryDisplay(categoryStmt->GetValueId<DgnCategoryId>(0), visible);
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
    if (source == nullptr)
        return;
    
    // Find the job subject of the element and colorize based on it
    DgnElementCP element = source->ToElement();
    if (element == nullptr)
        return;
    
    m_elementToSubjectStmt->Reset();
    m_elementToSubjectStmt->BindId(1, element->GetElementId());
    m_elementToSubjectStmt->Step();
    DgnElementId subjectId = m_elementToSubjectStmt->GetValueId<DgnElementId>(0);
    if (!subjectId.IsValid())
        return;
    
    // TODO: Cache these job subjects
    // Get to the Job Subject if there's any
    SubjectCPtr jobSubject = m_db->Elements().Get<Subject>(subjectId);
    while (jobSubject.IsValid() && !JobSubjectUtils::IsJobSubject(*jobSubject))
        {
        subjectId = jobSubject->GetParentId();
        jobSubject = m_db->Elements().Get<Subject>(subjectId);
        }
    
    if (!jobSubject.IsValid())
        return;
    
    if (m_subjectColors.find(jobSubject->GetElementId()) == m_subjectColors.end())
        {
        BeAssert(false && "All Job Subjects should have an assigned color.");
        return;
        }

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