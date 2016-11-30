/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/WebTraverser/WebTraverser.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/SpatialEntityData.h>
#include "../ISpatialEntityTraversalObserver.h"

#include <curl/curl.h>
#include <string>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass
// Observer handling downloaded data
//=======================================================================================

struct WebTraversalObserver: public ISpatialEntityTraversalObserver
    {
    bool m_dualMode = false;
    bool m_updateMode = false;
    bool m_verbose = true;
public:

    WebTraversalObserver(bool dualMode, bool updateMode, const char* dbName, const char* pwszConnStr, bool verbose = true);

    virtual void OnFileListed(bvector<Utf8String>& fileList, Utf8CP file, Utf8CP dataset);
    virtual void OnFileDownloaded(Utf8CP file);
    virtual void OnDataExtracted(SpatialEntityDataCR data);
    };


END_BENTLEY_REALITYPLATFORM_NAMESPACE