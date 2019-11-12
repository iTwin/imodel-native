//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>

#include "SMExternalProviderDataStore.h"
#include "SMExternalProviderDataStore.hpp"

template class SMExternalProviderDataStore<DPoint3d, DRange3d>;
template class SMExternalProviderDataStore<Utf8String, DRange3d>;

SMExternalClipDefinitionExtOps::SMExternalClipDefinitionExtOps(IClipDefinitionDataProvider* provider)
{
	m_clipProvider = provider;
}


void SMExternalClipDefinitionExtOps::StoreClipWithParameters(const bvector<DPoint3d>& clipData, uint64_t id, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
{
	int64_t clipId = id;
	m_clipProvider->SetClipPolygon(clipData, clipId, type);
}

void SMExternalClipDefinitionExtOps::LoadClipWithParameters(bvector<DPoint3d>& clipData, uint64_t id, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive)
{
	int64_t clipId = id;
	isActive = true;
    bvector<DPoint3d> poly;
	m_clipProvider->GetClipPolygon(poly, clipId, type);
    if (!poly.empty())
    {
        clipData = poly;
        geom = SMClipGeometryType::Polygon;
    }
    else
    {
        ClipVectorPtr cp = nullptr;
        m_clipProvider->GetClipVector(cp, clipId, type);
        if (cp != nullptr)
        {
            auto polyCP = cp->front()->GetPolygon();
            clipData.clear();
            if(polyCP != nullptr)
                {
                for(auto&pt : *polyCP)
                    clipData.push_back(DPoint3d::From(pt.x, pt.y, cp->front()->GetZHigh()));
                cp->front()->GetTransformFromClip()->Multiply(&clipData[0], clipData.data(), (int)clipData.size());
                }
            geom = SMClipGeometryType::BoundedVolume;
        }
    }
}

void SMExternalClipDefinitionExtOps::StoreClipWithParameters(const ClipVectorPtr& clipData, uint64_t id, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
{
    int64_t clipId = id;
    m_clipProvider->SetClipVector(clipData, clipId, type);
}

void SMExternalClipDefinitionExtOps::LoadClipWithParameters(ClipVectorPtr& clipData, uint64_t id, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive)
{
    int64_t clipId = id;
    isActive = true;
    ClipVectorPtr cp = nullptr;
    m_clipProvider->GetClipVector(cp, clipId, type);
    if (cp != nullptr)
    {
        clipData = cp;
        geom = SMClipGeometryType::BoundedVolume;
    }
}

void SMExternalClipDefinitionExtOps::GetAllIDs(bvector<uint64_t>& allIds)
{
	m_clipProvider->ListClipIDs(allIds);
}

void SMExternalClipDefinitionExtOps::GetAllCoverageIDs(bvector<uint64_t>& allIds)
{
	m_clipProvider->ListTerrainRegionIDs(allIds);
}


void SMExternalClipDefinitionExtOps::GetClipType(uint64_t id, SMNonDestructiveClipType& type)
{
    bvector<DPoint3d> clipData;
    m_clipProvider->GetClipPolygon(clipData, id, type);
    if(clipData.empty())
    {
        ClipVectorPtr cp = nullptr;
        m_clipProvider->GetClipVector(cp, id, type);
    }
}


