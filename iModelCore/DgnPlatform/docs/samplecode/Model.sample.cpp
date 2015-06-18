/*--------------------------------------------------------------------------------------+
|
|     $Source: docs/samplecode/Model.sample.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ Model_Includes.sampleCode
// Primary header file for the DgnPlatform API 
#include <DgnPlatform/DgnPlatformApi.h>

// helper macro for using the DgnPlatform namespace
USING_NAMESPACE_BENTLEY_DGNPLATFORM
//__PUBLISH_EXTRACT_END__

//__PUBLISH_EXTRACT_START__ Model_CreateAndInsert.sampleCode
//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
PhysicalModelP createAndInsertPhysicalModel(DgnDbR db, Utf8CP name)
    {
    ModelHandler& handler = PhysicalModelHandler::GetHandler();
    DgnClassId modelClassId = db.Domains().GetClassId(handler);
    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, modelClassId, name));

    if (DgnDbStatus::Success != model->Insert())
        return nullptr;

    return db.Models().Get<PhysicalModel>(model->GetModelId());
    }

//__PUBLISH_EXTRACT_END__
