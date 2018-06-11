/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateAPI/iModelBridge/iModelBridgeRegistry.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// NB: Do not include anything from DgnPlatform! That is not on the include path, and we can't put it there when building standalone programs.

#include <iModelBridge/iModelBridgeFwkTypes.h>
#include <BeSQLite/BeSQLite.h>
#include <Logging/bentleylogging.h>

#ifndef UNIX_MAIN_CALLS_WMAIN
#define UNIX_MAIN_CALLS_WMAIN(ARGV_TYPE)                                            \
extern "C" int main(int argc, char** argv)                                          \
    {                                                                               \
    BentleyApi::bvector<wchar_t*> argv_w_ptrs;                                      \
    for(int i=0; i<argc; i++)                                                       \
        {                                                                           \
        BentleyApi::WString argw(argv[i], BentleyApi::BentleyCharEncoding::Utf8);   \
        auto argp = new wchar_t[argw.size() + 1];                                   \
        wcscpy(argp, argw.data());                                                  \
        argv_w_ptrs.push_back(argp);                                                \
        }                                                                           \
                                                                                    \
    return wmain(argc, (ARGV_TYPE)argv_w_ptrs.data());                              \
    }
#endif

BEGIN_BENTLEY_DGN_NAMESPACE


//=======================================================================================
// @bsiclass                                                    Sam.Wilson   02/15
//=======================================================================================
struct iModelBridgeRegistryBase : RefCounted<IModelBridgeRegistry>
{
    struct AssignCmdLineArgs
        {
        BeFileName m_stagingDir;
        BeFileName m_loggingConfigFileName;
        Utf8String m_repositoryName;

        int ParseCommandLine(int argc, WCharCP argv[]);
        };

protected:
    BeSQLite::Db m_stateDb;
    IMODEL_BRIDGE_FWK_EXPORT BeSQLite::DbResult OpenOrCreateStateDb();
private:
    BeFileName m_stateFileName;
    BeFileName m_stagingDir;

    
    bool QueryAnyInstalledBridges();
  
    
    void SearchForBridgesToAssignToDocumentsInDir(BeFileNameCR);
    
    BentleyStatus QueryBridgeAssignedToDocument(BeFileNameR libPath, WStringR name, BeFileNameCR docName);
    BeFileName QueryBridgeLibraryPathByName(uint64_t* rowid, WStringCR bridgeName);
    
    BentleyStatus WriteBridgesFile();
    //bool _IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) override;
    static void* GetBridgeFunction(BeFileNameCR bridgeDllName, Utf8CP funcName);
    void EnsureDocumentPropertiesFor(BeFileNameCR);
    
    static void InitCrt(bool quietAsserts);

protected:
    //Exported for testing
    
    IMODEL_BRIDGE_FWK_EXPORT virtual BentleyStatus ComputeBridgeAffinityToDocument(iModelBridgeWithAffinity& affinity, BeFileNameCR affinityPath, BeFileNameCR filePath);
    IMODEL_BRIDGE_FWK_EXPORT void SearchForBridgesToAssignToDocuments();
    
    IMODEL_BRIDGE_FWK_EXPORT bool _IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) override;
    IMODEL_BRIDGE_FWK_EXPORT void _QueryAllFilesAssignedToBridge(bvector<BeFileName>& fns, wchar_t const* bridgeRegSubKey) override;
    
    
    IMODEL_BRIDGE_FWK_EXPORT BentleyStatus _GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BeFileNameR localFilePath, BeSQLite::BeGuid const& docGuid) override;
    IMODEL_BRIDGE_FWK_EXPORT BentleyStatus _AssignFileToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) override;
    

    IMODEL_BRIDGE_FWK_EXPORT iModelBridgeRegistryBase(BeFileNameCR stagingDir, BeFileNameCR dbName);
    IMODEL_BRIDGE_FWK_EXPORT ~iModelBridgeRegistryBase();

public:
    IMODEL_BRIDGE_FWK_EXPORT BentleyStatus SearchForBridgeToAssignToDocument(BeFileNameCR);

    IMODEL_BRIDGE_FWK_EXPORT BentleyStatus RemoveFileAssignment(BeFileNameCR fn);
    IMODEL_BRIDGE_FWK_EXPORT void SetDocumentProperties(iModelBridgeDocumentProperties&, BeFileNameCR fn);
    IMODEL_BRIDGE_FWK_EXPORT BentleyStatus _GetDocumentProperties(iModelBridgeDocumentProperties&, BeFileNameCR fn) override;
    //! @private
    static BeFileName MakeDbName(BeFileNameCR stagingDir, Utf8StringCR iModelName);
    //! @private
    static int ComputeAffinityMain(int argc, WCharCP argv[]);
    //! @private
    static int AssignMain(int argc, WCharCP argv[]);
};

/*---------------------------------------------------------------------------------**//**
// @bsiclass                                      Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct iModelBridgeRegistry : iModelBridgeRegistryBase
    {
    private:
    virtual BentleyStatus _FindBridgeInRegistry(BeFileNameR bridgeLibraryPath, BeFileNameR bridgeAssetsDir, WStringCR bridgeName) override;
    
    public:

    virtual void _DiscoverInstalledBridges() override;
    //! @private
    static RefCountedPtr<iModelBridgeRegistry> OpenForFwk(BeSQLite::DbResult&, BeFileNameCR stagingDir, Utf8StringCR iModelName);
    iModelBridgeRegistry(BeFileNameCR stagingDir, BeFileNameCR dbName);
    };

END_BENTLEY_DGN_NAMESPACE
