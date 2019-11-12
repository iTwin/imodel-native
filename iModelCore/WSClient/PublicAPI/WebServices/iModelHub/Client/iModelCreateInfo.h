/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/iModelBaseInfo.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

USING_NAMESPACE_BENTLEY_WEBSERVICES

typedef RefCountedPtr<struct iModelCreateInfo> iModelCreateInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(iModelCreateInfo);

//=======================================================================================
//@bsiclass                                      Vilius.Kazlauskas             08/2019
//=======================================================================================
struct iModelCreateInfo : iModelBaseInfo
{
private:
    iModelCreateInfo(Utf8StringCR name, Utf8StringCR description, bvector<double> extent, Utf8StringCR imodelTemplate)
        : iModelBaseInfo(name, description, extent, imodelTemplate) {}
public:
    static iModelCreateInfoPtr Create(Utf8StringCR name, Utf8StringCR description, bvector<double> extent = bvector<double>())
        {return iModelCreateInfoPtr(new iModelCreateInfo(name, description, extent, ""));}
};
END_BENTLEY_IMODELHUB_NAMESPACE
