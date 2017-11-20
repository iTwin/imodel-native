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
void    SubjectViewController::_AddFeatureOverrides(Render::FeatureSymbologyOverrides& ovrs) const
    {
    for (auto mapping : m_subjectColors)
        ovrs.OverrideModel(mapping.first, mapping.second);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod 												Diego.Pinate 	10/17
//-------------------------------------------------------------------------------------------
void    SubjectViewController::ToggleVisibility(DgnModelId modelId, bool isVisible)
    {
#ifdef TODO_SUBJECT_OVERRIDES
    if (m_subjectColors.find(subjectId) == m_subjectColors.end())
        return;
    
    m_subjectColors[subjectId].SetLineTransparency(isVisible ? 0 : 255);
    m_subjectColors[subjectId].SetFillTransparency(isVisible ? 0 : 255);
#endif
    }

//-------------------------------------------------------------------------------------------
// @bsimethod 												Diego.Pinate 	11/17
//-------------------------------------------------------------------------------------------
bool     SubjectViewController::IsVisible(DgnModelId subjectId) const
    {
#ifdef TODO_SUBJECT_OVERRIDES
    if (m_subjectColors.find(subjectId) == m_subjectColors.end())
        {
        BeAssert(false && "Subject not found in View Controller.");
        return false;
        }
    
    SubjectColorMap::const_iterator iter = m_subjectColors.find(subjectId);
    Byte alpha = iter->second.GetFillColor().GetAlpha();
    return alpha == 0;
#endif
    return false;
    }