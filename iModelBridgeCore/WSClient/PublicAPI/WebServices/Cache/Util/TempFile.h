/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Util/TempFile.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/BeFileName.h>

#include <WebServices/Cache/WebServicesCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
class TempFile
    {
    private:
        BeFileName m_filePath;

    public:
        //! Create temporary file path that will be cleanuped after TempFile object goes out of scope
        WSCACHE_EXPORT TempFile(BeFileNameCR tempDir, Utf8StringCR fileName);
        //! Destructor will call Cleanup()
        WSCACHE_EXPORT ~TempFile();
        //! Get path to file
        WSCACHE_EXPORT BeFileNameCR GetPath() const;
        //! Optionally delete temporary file if it exists. This method is called in destructor
        WSCACHE_EXPORT void Cleanup();
    };

typedef std::shared_ptr<TempFile> TempFilePtr;

END_BENTLEY_WEBSERVICES_NAMESPACE
