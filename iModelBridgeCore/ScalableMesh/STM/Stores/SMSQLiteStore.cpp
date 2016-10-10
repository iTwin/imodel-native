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

void SMSQLiteClipDefinitionExtOps::GetAllPolys(bvector<bvector<DPoint3d>>& polys)
    {
    bvector<bvector<uint8_t>> data;
    bvector<size_t> sizes;
    m_smSQLiteFile->GetAllPolys(data, sizes);

    for (size_t i = 0; i < data.size(); ++i)
        {
        HCDPacket pi_uncompressedPacket, pi_compressedPacket;
        pi_compressedPacket.SetBuffer(&data[i][0], data[i].size());
        pi_compressedPacket.SetDataSize(data[i].size());
        
        bvector<DPoint3d> poly(sizes[i]/sizeof(DPoint3d));
        pi_uncompressedPacket.SetBuffer(poly.data(), poly.size() * sizeof(DPoint3d));
        pi_uncompressedPacket.SetBufferOwnership(false);


        LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket);
        polys.push_back(poly);
        }
    }

void SMSQLiteClipDefinitionExtOps::SetAutoCommit(bool autoCommit) 
    {
    m_smSQLiteFile->m_autocommit = autoCommit;
    }
