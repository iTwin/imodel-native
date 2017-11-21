/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/SubjectViewController.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnPlatform/ViewController.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/IAuxCoordSys.h>
#include <DgnPlatform/ViewContext.h>
#include <DgnPlatform/UpdatePlan.h>
#include <DgnPlatform/ViewDefinition.h>
#include <Bentley/BeThread.h>
#include <BeSQLite/RTreeMatch.h>
#include <ECDb/ECSqlStatement.h>

DGNPLATFORM_TYPEDEFS(SubjectViewController)
DGNPLATFORM_REF_COUNTED_PTR(SubjectViewController)

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_RENDER

BEGIN_BENTLEY_DGN_NAMESPACE
//=======================================================================================
//! Used in Model Alignment to colorize elements based on their Job Subject
// @bsistruct                                                   Diego.Pinate    03/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SubjectViewController : SpatialViewController
{
    DEFINE_T_SUPER(SpatialViewController)
    friend struct SpatialViewDefinition;

    typedef bmap<DgnModelId, FeatureSymbologyOverrides::Appearance> SubjectColorMap;

private:
    SubjectColorMap     m_subjectColors;
    DgnDbP              m_db;
    ECSqlStatement*     m_elementToSubjectStmt;


protected:
    void _AddFeatureOverrides(Render::FeatureSymbologyOverrides&) const override;
    SubjectCPtr GetParentJobSubject(DgnElementCP element);

public:
    //! Turns on/off all models and categories
    //! @param[in] visible true if visible
    DGNPLATFORM_EXPORT void     SetModelsAndCategoriesVisibility(bool visible);

    //! Toggles visibility of a specific subject
    //! @param[in] modelId
    //! @param[in] isVisible
    DGNPLATFORM_EXPORT void     ToggleVisibility(DgnModelId modelId, bool isVisible);

    //! Returns true if a subject is visible
    //! @param[in] modelId ID of the job subject
    //! @return true if job subject is visible
    DGNPLATFORM_EXPORT bool     IsVisible(DgnModelId modelId) const;

    //! Constructor
    DGNPLATFORM_EXPORT SubjectViewController(SpatialViewDefinition const& view, DgnDbP db, SubjectColorMap const& subjectColors);
    DGNPLATFORM_EXPORT ~SubjectViewController();
};

END_BENTLEY_DGN_NAMESPACE
