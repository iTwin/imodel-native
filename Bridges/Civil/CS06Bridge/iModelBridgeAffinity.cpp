/*--------------------------------------------------------------------------------------+
|
|     $Source: CS06Bridge/iModelBridgeAffinity.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <CS06BridgeInternal.h>

/*=================================================================================**//**
* @bsiclass                                                     Abeesh.Basheer     03/09
+===============+===============+===============+===============+===============+======*/
/*struct AffinityHost : iModelBridgeBimHost
    {
    private:
        BentleyApi::Dgn::DgnPlatformLib::Host::RepositoryAdmin* m_repositoryAdmin;
        iModelBridgeKnownLocationsAdmin* m_knownLocationsAdmin;

    public:
        AffinityHost();
    };

AffinityHost::AffinityHost() : m_repositoryAdmin(new BentleyApi::Dgn::DgnPlatformLib::Host::RepositoryAdmin()), 
    m_knownLocationsAdmin(new iModelBridgeKnownLocationsAdmin()), 
    iModelBridgeBimHost(m_repositoryAdmin, "", "", "")
    {
    
    //iModelBridgeBimHost();
    }*/

BEGIN_CS06BRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" void iModelBridge_getAffinity(iModelBridge::BridgeAffinity& bridgeAffinity, BeFileName const& affinityLibraryPath, BeFileName const& sourceFileName)
    {
    bridgeAffinity.m_affinity = iModelBridge::Affinity::None;

    /*try
        {
        static AffinityHost host;
        Dgn::DgnViewLib::Host& threadHost = Dgn::DgnViewLib::GetHost();
        if (!threadHost.IsInitialized())
            Dgn::DgnViewLib::Initialize(host, false);

        DgnFileStatus status;
        DgnDocumentPtr doc = DgnDocument::CreateForLocalFile(sourceFileName.c_str());
        if (doc.IsNull())
            return;

        DgnFilePtr dgnFilePtr = DgnFile::Create(*doc, DgnFileOpenMode::ReadOnly);
        if (dgnFilePtr.IsNull())
            return;

        StatusInt nStatus;
        dgnFilePtr->LoadFile(&nStatus, nullptr, true);

        DgnV8Api::ModelId modelId = dgnFilePtr->GetDefaultModelId();

        DependencyManager::SetProcessingDisabled(false);

        auto rootModelP = dgnFilePtr->LoadRootModelById(&nStatus, modelId, true, true, true);

        DependencyManager::SetProcessingDisabled(true);

        if (!rootModelP)
            return;

        DgnV8Api::SchemaInfo schemaInfo(Bentley::ECN::SchemaKey(L"Bentley_Civil__Model_Geometry", 0, 0), *dgnFilePtr, L"", L"", 0);
        auto& dgnECManager = DgnV8Api::DgnECManager::GetManager();
        Bentley::ECN::ECSchemaPtr nativeSchema = dgnECManager.LocateSchemaInDgnFile(schemaInfo, Bentley::ECN::SCHEMAMATCHTYPE_Latest);
        if (!nativeSchema.IsNull())
            {
            bridgeAffinity.m_bridgeRegSubKey = L"OpenRoads Designer Bridge";
            bridgeAffinity.m_affinity = iModelBridgeAffinityLevel::ExactMatch;
            return;
            }

        bridgeAffinity.m_bridgeRegSubKey = L"MicroStation";
        bridgeAffinity.m_affinity = iModelBridge::Affinity::Low;
        }
    catch (...)
        {
        return; 
        }*/
    }

END_CS06BRIDGE_NAMESPACE
