/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/ScalableMeshConversion.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <VersionedDgnV8Api/DgnPlatform/ScalableMeshBaseElementHandler.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA


struct DgnV8ThreeMxClipFlags
{
    unsigned        m_isMask:1;
    unsigned        m_clipZLow:1;
    unsigned        m_clipZHigh:1;
    unsigned        m_reserved:29;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 07/17
+---------------+---------------+---------------+---------------+---------------+------*/
ConvertToDgnDbElementExtension::Result ConvertScalableMeshAttachment::_PreConvertElement(DgnV8EhCR v8el, Converter& converter, ResolvedModelMapping const& v8mm)
    {
    Transform               location;
    ClipVectorPtr           clipVector;
    uint32_t                activeClassifierId = 0xffff;
    ModelSpatialClassifiers classifiers;
    Utf8String              rootUrl;


    if (SUCCESS != RealityMeshAttachmentConversion::ExtractAttachment (rootUrl, location, clipVector, classifiers, activeClassifierId, v8el, converter, v8mm, ScalableMeshElementHandler::XATTRIBUTEID_ScalableMeshAttachment))
        return Result::SkipElement;

    Utf8String linkName(BeFileName(rootUrl).GetFileNameWithoutExtension());
    DgnDbR db = converter.GetDgnDb();
    RepositoryLinkPtr repositoryLink = RepositoryLink::Create(*db.GetRealityDataSourcesModel(), rootUrl.c_str(), linkName.c_str());
    if (!repositoryLink.IsValid() || !repositoryLink->Insert().IsValid())
        return Result::SkipElement;

    // In DgnV8, ThreeMx attachments are elements, and their visibility is determined by their level. In DgnDb they are DgnModels. 
    // For every spatial view in the DgnDb, determine whether the category of the element's original level is on, and if so
    // add the new ThreeMxModel to the list of viewed models.
    Utf8String modelName;
    BeFileName smFileName; 

    WString rootUrlW(rootUrl.c_str(), true);
  
  	//Avoid slash in URL being converted to backslash  
    smFileName.AppendString(rootUrlW.c_str());

    IMeshSpatialModelP spatialModel(ScalableMeshModelHandler::AttachTerrainModel(db, modelName, smFileName, *repositoryLink, true, clipVector.get(), &classifiers));
    DgnModelId modelId = spatialModel->GetModelId();

    DgnCategoryId category = converter.GetSyncInfo().GetCategory(v8el, v8mm);

    for (auto const& entry : ViewDefinition::MakeIterator(db))
        {
        auto viewController = ViewDefinition::LoadViewController(entry.GetId(), db);
        if (!viewController.IsValid() || !viewController->IsSpatialView() || !viewController->GetViewDefinitionR().GetCategorySelector().IsCategoryViewed(category))
            continue;

        auto& modelSelector = viewController->ToSpatialViewP()->GetSpatialViewDefinition().GetModelSelector();
        modelSelector.AddModel(modelId);
        modelSelector.Update();
        }

    return Result::SkipElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                  07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertScalableMeshAttachment::Register()
    {
    ConvertScalableMeshAttachment* instance = new ConvertScalableMeshAttachment();

    DgnV8Api::ElementHandlerId handlerId(ScalableMeshElementHandler::XATTRIBUTEID_ScalableMeshAttachment, 0);
    DgnV8Api::Handler* elHandler = DgnV8Api::ElementHandlerManager::FindHandler(handlerId);

    assert(elHandler != nullptr);
    
    RegisterExtension(*elHandler, *instance);

    RegisterExtension(DgnV8Api::ScalableMeshBaseElementHandler::GetInstance(), *instance);
    }


