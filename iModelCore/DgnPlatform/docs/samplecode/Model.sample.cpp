/*--------------------------------------------------------------------------------------+
|
|     $Source: docs/samplecode/Model.sample.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ Model_Includes.sampleCode
// Primary header file for the DgnPlatform API 
#include <DgnPlatform/DgnPlatformApi.h>

// helper macro for using the DgnPlatform namespace
USING_NAMESPACE_BENTLEY_DGN
//__PUBLISH_EXTRACT_END__

//__PUBLISH_EXTRACT_START__ Model_CreateAndInsert.sampleCode
//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
SpatialModelPtr createAndInsertSpatialModel(DgnElementCR modeledElement, Utf8CP modelName)
    {
    ModelHandlerR handler = dgn_ModelHandler::Spatial::GetHandler();
    DgnDbR db = modeledElement.GetDgnDb();
    DgnClassId modelClassId = db.Domains().GetClassId(handler);
    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, modelClassId, modeledElement.GetElementId(), DgnModel::CreateModelCode(modelName)));

    return (DgnDbStatus::Success == model->Insert()) ? model->ToSpatialModelP()  : nullptr;
    }

//__PUBLISH_EXTRACT_END__
//__PUBLISH_EXTRACT_START__ Model_QueryByName.sampleCode
//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
SpatialModelPtr querySpatialModelByName(DgnDbR db, Utf8CP modelName)
    {
    DgnModelId modelId = db.Models().QueryModelId(DgnModel::CreateModelCode(modelName));
    if (!modelId.IsValid())
        return nullptr;

    return db.Models().Get<SpatialModel>(modelId);
    }

//__PUBLISH_EXTRACT_END__
