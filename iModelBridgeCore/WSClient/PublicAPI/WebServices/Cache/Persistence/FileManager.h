/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Persistence/FileManager.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <Bentley/BeFileName.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef std::shared_ptr<struct IFileManager> IFileManagerPtr;

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct IFileManager
    {
    virtual ~IFileManager() {}

    virtual BeFileNameStatus DeleteFile(BeFileNameCR path) = 0;
    };

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileManager : IFileManager
    {
    virtual BeFileNameStatus DeleteFile(BeFileNameCR path) override { return BeFileName::BeDeleteFile(path); }
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
