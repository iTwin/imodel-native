/*--------------------------------------------------------------------------------------+
|
|     $Source: CS06Bridge/iModelBridgeAffinity.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <CS06BridgeInternal.h>

BEGIN_CS06BRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Jonathan.DeCarlo                  11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" void iModelBridge_getAffinity(iModelBridge::BridgeAffinity& bridgeAffinity, BeFileName const& affinityLibraryPath, BeFileName const& sourceFileName)
    {
    bridgeAffinity.m_affinity = iModelBridgeAffinityLevel::None;

    Dgn06::DgnDb::OpenParams openParams(BeSQLite06::Db::OpenMode::Readonly);
    BentleyG06::BeFileName sourceFileName06 = MarshalHelper::MarshalBimBeFileNameTo06BeFileName(sourceFileName);
    BeSQLite06::DbResult status;
    Dgn06::DgnDbPtr dgnDbPtr = Dgn06::DgnDb::OpenDgnDb(&status, sourceFileName06, openParams);
    if (dgnDbPtr.IsNull())
        return;

    // TODO: Is there anything else we need to check here to make sure it is a ConceptStation model?

    bridgeAffinity.m_affinity = iModelBridgeAffinityLevel::ExactMatch;
    bridgeAffinity.m_bridgeRegSubKey = CS06Bridge::GetRegistrySubKey();
    }

END_CS06BRIDGE_NAMESPACE
