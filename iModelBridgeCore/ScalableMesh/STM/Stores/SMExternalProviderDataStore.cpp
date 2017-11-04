//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMExternalProviderDataStore.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
	m_clipProvider->GetClipPolygon(clipData, clipId, type);
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
}


