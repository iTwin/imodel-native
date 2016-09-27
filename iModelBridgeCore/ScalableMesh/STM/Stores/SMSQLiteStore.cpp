//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMSQLiteStore.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>

#include "SMSQLiteStore.h"
#include "SMSQLiteStore.hpp"

template class SMSQLiteStore<DRange3d>;

template class SMSQLiteNodeDataStore<DPoint3d, DRange3d>;
    

SMSQLiteClipDefinitionExtOps::SMSQLiteClipDefinitionExtOps(SMSQLiteFilePtr& smSQLiteFile)
    {
    m_smSQLiteFile = smSQLiteFile;
    }

SMSQLiteClipDefinitionExtOps::~SMSQLiteClipDefinitionExtOps()
    {
    }

void SMSQLiteClipDefinitionExtOps::GetMetadata(uint64_t id, double& importance, int& nDimensions)
    {
    m_smSQLiteFile->GetClipPolygonMetadata(id, importance, nDimensions);
    }

void SMSQLiteClipDefinitionExtOps::SetMetadata(uint64_t id, double importance, int nDimensions)
    {
    m_smSQLiteFile->SetClipPolygonMetadata(id, importance, nDimensions);
    }

void SMSQLiteClipDefinitionExtOps::GetAllIDs(bvector<uint64_t>& allIds)
    {
    m_smSQLiteFile->GetAllClipIDs(allIds);
    }    

void SMSQLiteClipDefinitionExtOps::SetAutoCommit(bool autoCommit) 
    {
    m_smSQLiteFile->m_autocommit = autoCommit;
    }
