/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshRDSProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/ScalableMeshDefs.h>

/*--------------------------------------------------------------------------------------+
|   Header File Dependencies
+--------------------------------------------------------------------------------------*/

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

class IScalableMeshRDSProvider;
typedef BENTLEY_NAMESPACE_NAME::RefCountedPtr<IScalableMeshRDSProvider> IScalableMeshRDSProviderPtr;

/*=================================================================================**//**
IScalableMeshRDSProvider defines an interface that enables communications with RDS
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class IScalableMeshRDSProvider: public IRefCounted
{
    /*__PUBLISH_SECTION_END__*/
    /*__PUBLISH_CLASS_VIRTUAL__*/

protected:

    virtual Utf8String _GetAzureURLAddress() = 0;

    virtual Utf8String _GetRDSURLAddress() = 0;

    virtual Utf8String _GetToken() = 0;

    virtual Utf8String _GetRootDocument() = 0;

    virtual Utf8String _GetProjectID() = 0;



    /*__PUBLISH_SECTION_START__*/

public:

    BENTLEY_SM_EXPORT Utf8String GetAzureURLAddress();

    BENTLEY_SM_EXPORT Utf8String GetRDSURLAddress();

    BENTLEY_SM_EXPORT Utf8String GetToken();

    BENTLEY_SM_EXPORT Utf8String GetRootDocument();

    BENTLEY_SM_EXPORT Utf8String GetProjectID();

    BENTLEY_SM_EXPORT static IScalableMeshRDSProviderPtr Create(const Utf8String& serverUrl, const Utf8String& projectGuid, const Utf8String& pwcsMeshGuid);
};


END_BENTLEY_SCALABLEMESH_NAMESPACE

