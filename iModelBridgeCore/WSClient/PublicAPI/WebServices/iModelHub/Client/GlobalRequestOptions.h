/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeJsonCpp/BeJsonUtilities.h>
#include <WebServices/iModelHub/Common.h>
#include <WebServices/Client/WSChangeset.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

typedef RefCountedPtr<struct GlobalRequestOptions> GlobalRequestOptionsPtr;

//=======================================================================================
//@bsiclass                                      Karolis.Uzkuraitis             07/2018
//=======================================================================================
struct GlobalRequestOptions : RefCountedBase
    {
private:
    std::unique_ptr<bmap<Utf8String, Utf8String>> m_requestOptionsPtr = nullptr;

public:
    void SetRequestOptions(bmap<Utf8String, Utf8String> const& requestOptions)
        {
        m_requestOptionsPtr = std::make_unique<bmap<Utf8String, Utf8String>>(requestOptions);
        }

    void ResetRequestOptions()
        {
        m_requestOptionsPtr = nullptr;
        }

    void InsertRequestOptions(JsonValueR jsonValue) const;

    void InsertRequestOptions(std::shared_ptr<WebServices::WSChangeset> changeset) const;
    };

END_BENTLEY_IMODELHUB_NAMESPACE