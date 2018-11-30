/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelBridge/iModelBridgeFwkTypes.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <iModelBridge/iModelBridgeFwkRegistry.h>
#include <BeSQLite/BeSQLite.h>

#ifndef BEGIN_BENTLEY_DGN_NAMESPACE
  #define BEGIN_BENTLEY_DGN_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace Dgn {
  #define END_BENTLEY_DGN_NAMESPACE   } END_BENTLEY_NAMESPACE
    #define USING_NAMESPACE_BENTLEY_DGN         using namespace BentleyApi::Dgn;
#endif

BEGIN_BENTLEY_DGN_NAMESPACE


struct IModelBridgeRegistry : IRefCounted
    {
    virtual bool _IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) = 0;
    virtual void _QueryAllFilesAssignedToBridge(bvector<BeFileName>& fns, wchar_t const* bridgeRegSubKey) = 0;
    virtual BentleyStatus _GetDocumentProperties(iModelBridgeDocumentProperties&, BeFileNameCR fn) = 0;
    virtual BentleyStatus _GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BeFileNameR localFilePath, BeSQLite::BeGuid const& docGuid) = 0;
    virtual BentleyStatus _AssignFileToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) = 0;
    virtual void          _DiscoverInstalledBridges() = 0;
    virtual BentleyStatus _FindBridgeInRegistry(BeFileNameR bridgeLibraryPath, BeFileNameR bridgeAssetsDir, WStringCR bridgeName) = 0;
    virtual ~IModelBridgeRegistry() {}
    };

END_BENTLEY_DGN_NAMESPACE
