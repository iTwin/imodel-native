/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <iModelBridge/iModelBridgeRegistry.h>
BEGIN_BENTLEY_DGN_NAMESPACE
struct FakeRegistry : public iModelBridgeRegistryBase
    {
    struct FakeBridgeDef
        {
        WString m_regSubKey;
        BeFileName m_libraryFilename;
        BeFileName m_bridgeAssetsDir;
        };

    private:
    virtual void          _DiscoverInstalledBridges() override;
    virtual BentleyStatus _FindBridgeInRegistry(BeFileNameR bridgeLibraryPath, BeFileNameR bridgeAssetsDir, WStringCR bridgeName)override;
    bmap<WString, std::function<T_iModelBridge_getAffinity>> m_affinityCalc;
    public:
    
    FakeRegistry(BeFileNameCR stagingDir, BeFileNameCR dbName);
    void AddBridge(WStringCR bridgeName, std::function<T_iModelBridge_getAffinity> const& affinityCalc);
    virtual BentleyStatus ComputeBridgeAffinityToDocument(iModelBridgeWithAffinity& affinity, BeFileNameCR affinityPath, BeFileNameCR filePath) override;
    BentleyStatus WriteAssignments();
    BentleyStatus WriteInstalledBridgesTable(bvector<FakeBridgeDef> const&);
    void Save();
    BeSQLite::DbResult Open() { return OpenOrCreateStateDb(); }

    int RunAssign(int argc, WCharCP argv[]);
    };
END_BENTLEY_DGN_NAMESPACE