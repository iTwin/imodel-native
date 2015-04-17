/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/IIModelPublishExtension.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnCore/ElementHandle.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (V10_WIP_ELEMENTHANDLER)
/*================================================================================**//**
* Extension to provide handle I-Model Publishing for an element handler.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IIModelPublishExtension : Handler::Extension
{
    ELEMENTHANDLER_EXTENSION_DECLARE_MEMBERS (IIModelPublishExtension, DGNPLATFORM_EXPORT)

    /*---------------------------------------------------------------------------------**//**
    *  Handle conversion of the input element to its representation within an IModel.
    *
    * @param    eh  IN the element to be tested.
    * @param    forceRePublish  IN force to publish even if up-to-date.
    * @return   SUCCESS to replace the element in model
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual StatusInt    _HandleIModelPublish (EditElementHandleR eh, bool forceRePublish) = 0;

    /*---------------------------------------------------------------------------------**//**
    *  Handle retarget of the input element to its representation within an IModel.
    *  Method should update the element so we look in the storage instead of
    *  file system, set the filename to embed, an optional alias and return the embedded file type
    * @param    fileToEmbed  IN file name to use when embedding. If left empty, the file will not be embedded
    * @param    alias  IN alias to use when embedding, may be empty
    * @param    eh  IN the element to be tested.
    * @return   EMBEDDEDFILE_Type for example EMBEDDEDFILE_Type_V8, if none 0 return will replace
    *           replace the element in the model
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual int    _HandleIModelRetarget (EditElementHandleR eh, WStringR fileToEmbed, WStringR alias) = 0;

    /*---------------------------------------------------------------------------------**//**
    *  Handles element provenance for publishing
    *
    * @param    eh  IN the element to get the provenance from.
    * @param    originalSourceFilename  OUT this would be the original source filename.
    * @param    newSourceFilename  OUT this would be the newly created source filename. If left empty, the
    *           publishing code will assume that no new file was created for the original source file.
    * @return   SUCCESS to indicate that we have provide the info and the the provenance should be written
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual StatusInt   _RetrieveProvenance  (ElementHandleCR eh, WStringR originalSourceFilename, WStringR newSourceFilename) = 0;

}; // IIModelPublishExtension
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE

