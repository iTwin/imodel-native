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
        BeFileName m_registryDir;
        BeFileName m_loggingConfigFileName;
        Utf8String m_repositoryName;
        BeFileName m_inputFileName;
        int ParseCommandLine(int argc, WCharCP argv[]);
        };

protected:
    BeSQLite::Db m_stateDb;
    IMODEL_BRIDGE_FWK_EXPORT BeSQLite::DbResult OpenOrCreateStateDb();
private:
    BeFileName m_stateFileName;
    BeFileName m_stagingDir;
    BeFileName m_registryDir;
    BeFileName m_masterFilePath;
    
    bool QueryAnyInstalledBridges();
  
    BentleyStatus SearchForBridgesToAssignToFile(BeFileNameCR fileName, WStringCR parentBridgeName, bset<BeFileName>& currentContext);
    void SearchForBridgesToAssignToDocumentsInDir(BeFileNameCR);
    
    BentleyStatus QueryBridgeAssignedToDocument(BeFileNameR libPath, WStringR name, BeFileNameCR docName);
    BeFileName QueryBridgeLibraryPathByName(uint64_t* rowid, WStringCR bridgeName);
    
    BentleyStatus WriteBridgesFile();
    //bool _IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) override;
    
    void EnsureDocumentPropertiesFor(BeFileNameCR);
    
    

    BentleyStatus    ComputeBridgeAffinityInParentContext(iModelBridgeWithAffinity& affinity, bool thisBridgeIsPP, WStringCR parent);
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
    IMODEL_BRIDGE_FWK_EXPORT BentleyStatus SearchForBridgeToAssignToDocument(WStringR bridgeName, BeFileNameCR, WStringCR parentBridgeName);

    IMODEL_BRIDGE_FWK_EXPORT BentleyStatus RemoveFileAssignment(BeFileNameCR fn);
    IMODEL_BRIDGE_FWK_EXPORT void SetDocumentProperties(iModelBridgeDocumentProperties&, BeFileNameCR fn);
    IMODEL_BRIDGE_FWK_EXPORT BentleyStatus _GetDocumentProperties(iModelBridgeDocumentProperties&, BeFileNameCR fn) override;
    //! @private
    static BeFileName MakeDbName(BeFileNameCR stagingDir, Utf8StringCR iModelName);
    //! @private
    
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDmsFileLocator
    {
    typedef std::pair<int, int> T_DocumentId;//VaultId, DocumentId

    struct ReferenceInfo
        {
        T_DocumentId m_id;
        //Unused variables below
        int m_referenceModelID;
        int m_nestDepth;
        int m_referenceType;
        int m_dmsFlags;
        int m_elementId;

        ReferenceInfo();
        void Clear();
        };

    struct DmsInfo
        {
        T_DocumentId            m_id;
        BeFileName              m_fileName;
        bvector<ReferenceInfo> m_refs;
        DmsInfo();
        void Clear();
        };
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct BASFileLocator : public IDmsFileLocator
    {
    private:
    bmap<T_DocumentId, DmsInfo> m_dmsInfoCache;
    BentleyStatus ReadMetaDataFromDmsDir(DmsInfo& info, BeFileNameCR dirName);
    static BentleyStatus GetReferenceInfoFromPrp(DmsInfo& info, BeFileNameCR fileName);
    static BentleyStatus GetDocumentInfoFromPrp(DmsInfo& info, BeFileNameCR fileName);
    public:
    IMODEL_BRIDGE_FWK_EXPORT BASFileLocator();
    IMODEL_BRIDGE_FWK_EXPORT BentleyStatus GetDocumentInfo(DmsInfo& info, BeFileNameCR fileName);
    IMODEL_BRIDGE_FWK_EXPORT BentleyStatus GetDocumentInfo(DmsInfo& info, T_DocumentId const& docId, BeFileNameCR dmsFolder);
    };

END_BENTLEY_DGN_NAMESPACE
