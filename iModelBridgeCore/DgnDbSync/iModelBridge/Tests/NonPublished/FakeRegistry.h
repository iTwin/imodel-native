/*--------------------------------------------------------------------------------------+
|
|  $Source: iModelBridge/Tests/NonPublished/FakeRegistry.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <iModelBridge/iModelBridgeRegistry.h>
BEGIN_BENTLEY_DGN_NAMESPACE
struct FakeRegistry : public iModelBridgeRegistryBase
    {
    private:
    virtual void          _DiscoverInstalledBridges() override;
    virtual BentleyStatus _FindBridgeInRegistry(BeFileNameR bridgeLibraryPath, BeFileNameR bridgeAssetsDir, WStringCR bridgeName)override;
    bmap<WString, std::function<T_iModelBridge_getAffinity>> m_affinityCalc;
    public:
    
    FakeRegistry(BeFileNameCR stagingDir, BeFileNameCR dbName);
    void AddBridge(WStringCR bridgeName, std::function<T_iModelBridge_getAffinity> const& affinityCalc);
    virtual BentleyStatus ComputeBridgeAffinityToDocument(iModelBridgeWithAffinity& affinity, BeFileNameCR affinityPath, BeFileNameCR filePath) override;
    void WriteAssignments();
    void Save();
    };
END_BENTLEY_DGN_NAMESPACE