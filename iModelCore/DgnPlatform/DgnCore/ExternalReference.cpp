/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ExternalReference.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
BentleyStatus createFileUri(Utf8StringR fileUri, Utf8StringCR fileName)
    {
    BeFileName beFileName(fileName);

    WString fileNameAndExt (beFileName.GetFileNameAndExtension());
    if (WString::IsNullOrEmpty(fileNameAndExt.c_str()))
        {
        // Leave input file name unchanged
        fileUri = fileName;
        }
    else
        {
        Utf8String fileNameAndExtUtf8(fileNameAndExt);
        fileUri = fileNameAndExtUtf8;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
BentleyStatus resolveFileUri(BeFileNameR fileName, Utf8StringCR fileUri, DgnDbCR db)
    {
    BeFileName dbFileName(db.GetDbFileName());
    BeFileName dbPath(dbFileName.GetDirectoryName());

    // Here, we expect that fileUri is a relative file name (relative to the DgnDb)
    BeFileName relativeName(fileUri.c_str()); 
    dbPath.AppendToPath(relativeName.c_str());

    fileName = dbPath;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
BentleyStatus DgnPlatformLib::Host::RasterAttachmentAdmin::_CreateFileUri(Utf8StringR fileUri, Utf8StringCR fileName) const
    {
    return createFileUri(fileUri, fileName);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
BentleyStatus DgnPlatformLib::Host::RasterAttachmentAdmin::_ResolveFileUri(BeFileNameR fileName, Utf8StringCR fileUri, DgnDbCR db) const
    {
    return resolveFileUri(fileName, fileUri, db);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
BentleyStatus DgnPlatformLib::Host::PointCloudAdmin::_CreateFileUri(Utf8StringR fileUri, Utf8StringCR fileName) const
    {
    return createFileUri(fileUri, fileName);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
BentleyStatus DgnPlatformLib::Host::PointCloudAdmin::_ResolveFileUri(BeFileNameR fileName, Utf8StringCR fileUri, DgnDbCR db) const
    {
    return resolveFileUri(fileName, fileUri, db);
    }
