/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbSync/DgnV8/ConverterApp.h>
#include "abdfilechecker.h"
#include <VersionedDgnV8Api\DgnPlatform\DgnFile.h>
#include <VersionedDgnV8Api\DgnPlatform\DgnModel.h>
#include <VersionedDgnV8Api\DgnPlatform\DgnFileIO\DgnModelRefCollections.h>
#include <VersionedDgnV8Api\DgnPlatform\ElementHandle.h>
#include <VersionedDgnV8Api\DgnPlatform\ElementUtil.h>


#define TFLABEL_LINK_ID 20343 /* 0x4F77 Triforma element ID TFLabel.  Contains unique_id, part, family, etc. */

#ifdef USEABDFILECHECKER
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Oleg.Nikitin                    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool elementHandle_hasTriformaLabel(ElementHandleR eh)
    {
    if (SHARED_CELL_ELM == eh.GetElementType())
        {
        if (DgnFileP dgnFile = eh.GetDgnFileP())
            {
            if (ElementRefP scdElRef = CellUtil::FindSharedCellDefinition(*eh.GetElementCP(), *dgnFile))
                {
                ElementHandle scdeh(scdElRef, NULL);
                if (elementHandle_hasTriformaLabel(scdeh))
                    return true;
                }
            }
        }

    for (ChildElemIter child(eh, ExposeChildrenReason::Count); child.IsValid(); child=child.ToNext())
        {
        if (elementHandle_hasTriformaLabel(child))
            return true;
        }

    return eh.BeginElementLinkages(TFLABEL_LINK_ID).IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Oleg.Nikitin                    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool dgnModel_containsTriformaElements(DgnModelR model)
    {
    for each (ElementHandle eh in model.GetReachableElements())
        {
        if (elementHandle_hasTriformaLabel(eh))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Oleg.Nikitin                    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool dgnFile_isABDFile(DgnFilePtr file)
    {
    if (!file.IsValid())
        return false;

    ModelItemVector modelItems;
    file->GetModelIndex().CopyItems(modelItems);
    DgnAttachmentLoadOptions loadOptions(true, false, false);
    for (ModelItemVector::const_iterator it = modelItems.begin(); it != modelItems.end(); it++)
        {
        ModelId modelID = (*it).GetModelId();
        DgnModelPtr model = file->LoadModelById(modelID);
        if (model.IsValid())
            {
            model->ReadAndLoadDgnAttachments(loadOptions);
            if (dgnModel_containsTriformaElements(*model))
                return true;
            }
        }

    return false;
    }

#endif