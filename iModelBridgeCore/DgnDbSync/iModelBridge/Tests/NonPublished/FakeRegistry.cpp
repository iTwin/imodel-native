/*--------------------------------------------------------------------------------------+
|
|  $Source: iModelBridge/Tests/NonPublished/FakeRegistry.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/FakeRegistry.h>
USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void            FakeRegistry::_DiscoverInstalledBridges()
    {
    for (auto iter : m_affinityCalc)
        {
        auto stmt = m_stateDb.GetCachedStatement("INSERT INTO fwk_InstalledBridges (Name,BridgeLibraryPath,AffinityLibraryPath,IsPowerPlatformBased) VALUES(?,?,?,?)");
        stmt->BindText(1, Utf8String(iter.first).c_str(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindText(2, Utf8String(iter.first).c_str(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindText(3, Utf8String(iter.first).c_str(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindBoolean(4, false);
        stmt->Step(); // may fail with constraint if bridge library is already registered.
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   FakeRegistry::_FindBridgeInRegistry(BeFileNameR bridgeLibraryPath, BeFileNameR bridgeAssetsDir, WStringCR bridgeName)
    {
    auto iter = m_affinityCalc.find(bridgeName);
    if (m_affinityCalc.end() == iter)
        return ERROR;

    //For the fake registry bridgename is used for affinityPath.
    bridgeLibraryPath = BeFileName(bridgeName);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
FakeRegistry::FakeRegistry(BeFileNameCR stagingDir, BeFileNameCR dbName)
    :iModelBridgeRegistryBase(stagingDir, dbName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus            FakeRegistry::WriteAssignments()
    {
    if (BentleyApi::BeSQLite::DbResult::BE_SQLITE_OK != OpenOrCreateStateDb())
        return ERROR;
    _DiscoverInstalledBridges();
    SearchForBridgesToAssignToDocuments();
    return (BentleyApi::BeSQLite::DbResult::BE_SQLITE_OK == m_stateDb.SaveChanges() ? SUCCESS : ERROR);
    //m_stateDb.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void FakeRegistry::AddBridge(WStringCR bridgeName, std::function<T_iModelBridge_getAffinity>const & affinityCalc)
    {
    m_affinityCalc[bridgeName] = affinityCalc;
    _DiscoverInstalledBridges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   FakeRegistry::ComputeBridgeAffinityToDocument(iModelBridgeWithAffinity& affinity, BeFileNameCR affinityPath, BeFileNameCR filePath)
    {
    //For the fake registry bridgename is used for affinityPath.
    auto iter = m_affinityCalc.find(affinityPath.c_str());
    if (m_affinityCalc.end() == iter)
        return ERROR;

    WChar registryName[MAX_PATH] = {0};
    iter->second(registryName, MAX_PATH, affinity.m_affinity, affinityPath.c_str(), filePath);

    affinity.m_bridgeRegSubKey = registryName;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void    FakeRegistry::Save()
    {
    m_stateDb.SaveChanges();
    }