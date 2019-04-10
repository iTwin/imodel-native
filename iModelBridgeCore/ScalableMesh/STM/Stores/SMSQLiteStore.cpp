//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMSQLiteStore.cpp $
//:>
//:>  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>

#include "SMSQLiteStore.h"
#include "SMExternalProviderDataStore.h"
#include "SMSQLiteStore.hpp"
#include "SMStreamedSourceStore.hpp"

template class SMSQLiteStore<DRange3d>;
   
template class SMSQLiteNodeDataStore<DPoint3d, DRange3d>;
    
template class SMStreamedSourceStore<byte, DRange3d>;

template class SMSQLiteNodeDataStore<Utf8String, DRange3d>;

template class SMSQLiteNodeDataStore<MTGGraph, DRange3d>;

template class SMSQLiteNodeDataStore<DifferenceSet, DRange3d>;

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

void SMSQLiteClipDefinitionExtOps::StoreClipWithParameters(const ClipVectorPtr& clipData, uint64_t id, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
{
    bvector<DPoint3d> clipDataPoly;
    if (clipData == nullptr) return;
    for (auto& primitive : *clipData)
    {
        primitive->ParseClipPlanes();
        auto cPlaneSet = primitive->GetMaskOrClipPlanes();
        if (nullptr == cPlaneSet)
            continue;
        for (auto& convexPlane : *cPlaneSet)
        {
            for (auto&plane : convexPlane)
            {
                auto p = plane.GetDPlane3d();
                clipDataPoly.push_back(p.origin);
                clipDataPoly.push_back(p.normal);
            }
        }
        DPoint3d limitPt = DPoint3d::From(DBL_MAX, primitive->ClipZHigh() ? primitive->GetZHigh() : 0, primitive->ClipZLow() ? primitive->GetZLow() : 0);
        clipDataPoly.push_back(limitPt);
    }

    HCDPacket pi_uncompressedPacket, pi_compressedPacket;

    pi_uncompressedPacket.SetBuffer(const_cast<DPoint3d*>(clipDataPoly.data()), clipDataPoly.size() * sizeof(DPoint3d));
    pi_uncompressedPacket.SetDataSize(clipDataPoly.size() * sizeof(DPoint3d));
    WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);

    size_t uncompressedSize = clipDataPoly.size() * sizeof(DPoint3d);
    bvector<uint8_t> data;
    data.resize(pi_compressedPacket.GetDataSize());
    memcpy(&data[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());

    int64_t clipId = id;
    m_smSQLiteFile->StoreClipPolygon(clipId, data, uncompressedSize, geom, type, isActive);
}

void SMSQLiteClipDefinitionExtOps::LoadClipWithParameters(ClipVectorPtr& clipData, uint64_t id, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive)
{
    bvector<uint8_t> data;
    size_t uncompressedSize;
    m_smSQLiteFile->GetClipPolygon(id, data, uncompressedSize, geom, type, isActive);

    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_compressedPacket.SetBuffer(&data[0], data.size());
    pi_compressedPacket.SetDataSize(data.size());

    bvector<DPoint3d> clipDataPoly;
    clipDataPoly.resize(uncompressedSize / sizeof(DPoint3d));
    if (!clipDataPoly.empty())
        pi_uncompressedPacket.SetBuffer(clipDataPoly.data(), clipDataPoly.size() * sizeof(DPoint3d));
    pi_uncompressedPacket.SetBufferOwnership(false);


    LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket);

    clipData = ClipVector::Create();
    bvector<ClipPlane> planesVec;
    for (size_t i = 0; i < clipDataPoly.size(); i+=2)
    {
        if (clipDataPoly[i].x == DBL_MAX)
        {
            ClipPlaneSet planeSet(planesVec.data(), planesVec.size());
            ClipPrimitivePtr primP = ClipPrimitive::CreateFromClipPlanes(planeSet);
            if (type != SMNonDestructiveClipType::Boundary)
                primP->SetIsMask(true);
            if(clipDataPoly[i].z != 0)
                primP->SetZLow(clipDataPoly[i].z);
            if (clipDataPoly[i].y != 0)
                primP->SetZHigh(clipDataPoly[i].y);
            clipData->push_back(primP);
            planesVec.clear();
        }
        else
        {
            DPoint3d pt = clipDataPoly[i];
            DVec3d normal = DVec3d::From(clipDataPoly[i + 1]);
            planesVec.push_back(ClipPlane(normal, pt));
        }
    }
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

SharedTransaction::SharedTransaction(SMSQLiteFilePtr fileP, bool readonly, bool startTransaction)
{
#ifndef VANCOUVER_API
    m_transactionNeedsClosing = false;
    m_dbNeedsClosing = false;
    m_smSQLiteFile = fileP;
    if (m_smSQLiteFile.IsValid() && !m_smSQLiteFile->IsOpen() && m_smSQLiteFile->IsShared())
    {
        bool openResult = m_smSQLiteFile->GetDb()->ReOpenShared(readonly, true);
        assert(openResult == true);
        m_dbNeedsClosing = true;
        if (startTransaction)
        {
            m_smSQLiteFile->GetDb()->StartTransaction();
            m_transactionNeedsClosing = true;
        }

    }
#endif
}

SharedTransaction::~SharedTransaction()
{
#ifndef VANCOUVER_API
    bool wasAbandoned;
    if (m_smSQLiteFile.IsValid() && m_smSQLiteFile->IsShared())
    {
        if (m_transactionNeedsClosing)
            m_smSQLiteFile->GetDb()->CommitTransaction();

        if (m_dbNeedsClosing)
            m_smSQLiteFile->GetDb()->CloseShared(wasAbandoned);      
    }
#endif
}