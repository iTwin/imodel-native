/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/Registry/iModelBridgeRegistry.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
struct iModelBridgeRegistry : RefCounted<IModelBridgeRegistry>
{
    struct AssignCmdLineArgs
        {
        BeFileName m_stagingDir;
        Utf8String m_repositoryName;

        int ParseCommandLine(int argc, WCharCP argv[]);
        };

private:
    BeSQLite::Db m_stateDb;
    BeFileName m_stateFileName;
    BeFileName m_stagingDir;

    BeSQLite::DbResult OpenOrCreateStateDb();

    void DiscoverInstalledBridges();
    bool QueryAnyInstalledBridges();
    BentleyStatus SearchForBridgeToAssignToDocument(BeFileNameCR);
    void SearchForBridgesToAssignToDocumentsInDir(BeFileNameCR);
    void SearchForBridgesToAssignToDocuments();
    BentleyStatus QueryBridgeAssignedToDocument(BeFileNameR libPath, WStringR name, BeFileNameCR docName);
    BeFileName QueryBridgeLibraryPathByName(uint64_t* rowid, WStringCR bridgeName);
    BentleyStatus ComputeBridgeAffinityToDocument(iModelBridgeWithAffinity& affinity, BeFileNameCR affinityPath, BeFileNameCR filePath);
    BentleyStatus WriteBridgesFile();
    //bool _IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) override;
    static void* GetBridgeFunction(BeFileNameCR bridgeDllName, Utf8CP funcName);

    bool _IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) override;
    BentleyStatus _FindBridgeInRegistry(BeFileNameR bridgeLibraryPath, BeFileNameR bridgeAssetsDir, WStringCR bridgeName) override;
    BentleyStatus _GetDocumentProperties(iModelBridgeDocumentProperties&, BeFileNameCR fn) override;
    BentleyStatus _GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BeFileNameR localFilePath, BeSQLite::BeGuid const& docGuid) override;

    static void InitCrt(bool quietAsserts);

    iModelBridgeRegistry(BeFileNameCR stagingDir, Utf8StringCR iModelName);
    ~iModelBridgeRegistry();

public:
    //! @private
    static int ComputeAffinityMain(int argc, WCharCP argv[]);
    //! @private
    static int AssignMain(int argc, WCharCP argv[]);
    //! @private
    static RefCountedPtr<iModelBridgeRegistry> OpenForFwk(BeFileNameCR stagingDir, Utf8StringCR iModelName);

};

END_BENTLEY_SQLITE_NAMESPACE
