//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMSQLiteStore.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>

#include "SMSQLiteStore.h"
#include "SMSQLiteStore.hpp"
#include "SMStreamedSourceStore.hpp"

template class SMSQLiteStore<DRange3d>;

template class SMSQLiteNodeDataStore<DPoint3d, DRange3d>;
    
template class SMStreamedSourceStore<byte, DRange3d>;

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

void SMSQLiteClipDefinitionExtOps::GetAllCoverageIDs(bvector<uint64_t>& allIds)
    {
    m_smSQLiteFile->GetAllCoverageIDs(allIds);
    }

void SMSQLiteClipDefinitionExtOps::GetIsClipActive(uint64_t id, bool& isActive)
    {
    m_smSQLiteFile->GetIsClipActive(id, isActive);
    }

void SMSQLiteClipDefinitionExtOps::GetClipType(uint64_t id, SMNonDestructiveClipType& type)
    {
    m_smSQLiteFile->GetClipType(id, type);
    }

void SMSQLiteClipDefinitionExtOps::SetClipOnOrOff(uint64_t id, bool isActive)
    {
    m_smSQLiteFile->SetClipOnOrOff(id, isActive);
    }

void SMSQLiteClipDefinitionExtOps::StoreClipWithParameters(const bvector<DPoint3d>& clipData, uint64_t id, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
    {

    HCDPacket pi_uncompressedPacket, pi_compressedPacket;

    pi_uncompressedPacket.SetBuffer(const_cast<DPoint3d*>(clipData.data()), clipData.size() * sizeof(DPoint3d));
    pi_uncompressedPacket.SetDataSize(clipData.size()* sizeof(DPoint3d));
    WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);

    size_t uncompressedSize = clipData.size()*sizeof(DPoint3d);
    bvector<uint8_t> data;
    data.resize(pi_compressedPacket.GetDataSize());
    memcpy(&data[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());

    int64_t clipId = id;
    m_smSQLiteFile->StoreClipPolygon(clipId, data, uncompressedSize, geom, type, isActive);
    }

void SMSQLiteClipDefinitionExtOps::LoadClipWithParameters(bvector<DPoint3d>& clipData, uint64_t id, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive)
    {
    bvector<uint8_t> data;
    size_t uncompressedSize;
    m_smSQLiteFile->GetClipPolygon(id, data, uncompressedSize, geom, type, isActive);

    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_compressedPacket.SetBuffer(&data[0], data.size());
    pi_compressedPacket.SetDataSize(data.size());

    clipData.resize(uncompressedSize / sizeof(DPoint3d));
    if(!clipData.empty())
        pi_uncompressedPacket.SetBuffer(clipData.data(), clipData.size() * sizeof(DPoint3d));
    pi_uncompressedPacket.SetBufferOwnership(false);


    LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket);
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
