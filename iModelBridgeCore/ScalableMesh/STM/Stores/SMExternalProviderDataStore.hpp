#pragma once
#include "SMExternalProviderDataStore.h"
#include "SMSQLiteStore.h"

#include <type_traits>

template <class DATATYPE, class EXTENT> SMExternalProviderDataStore<DATATYPE, EXTENT>::SMExternalProviderDataStore(SMStoreDataType dataType, SMIndexNodeHeader<EXTENT>* nodeHeader, IClipDefinitionDataProvider* provider)
{
	m_clipProvider = provider;
	m_nodeHeader = nodeHeader;
	m_dataType = dataType;
}

template <class DATATYPE, class EXTENT> SMExternalProviderDataStore<DATATYPE, EXTENT>::~SMExternalProviderDataStore()
{
}


template <class DATATYPE, class EXTENT> HPMBlockID SMExternalProviderDataStore<DATATYPE, EXTENT>::StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID)
{
	if (m_dataType != SMStoreDataType::ClipDefinition && m_dataType != SMStoreDataType::CoveragePolygon)
	{
		assert(false);
		return 0;
	}
	bvector<DATATYPE> dataForClip;
	dataForClip.assign(DataTypeArray, DataTypeArray + countData);
	switch (m_dataType)
	{
	case SMStoreDataType::ClipDefinition:
		m_clipProvider->SetClipPolygon(dataForClip, blockID.m_integerID);
		break;
	case SMStoreDataType::CoveragePolygon:
		m_clipProvider->SetTerrainRegion(dataForClip, blockID.m_integerID);
		break;
	case SMStoreDataType::CoverageName:
		m_clipProvider->SetTerrainRegionName(*(Utf8String*)DataTypeArray, blockID.m_integerID);
		return true;
	default:
		assert(!"Unsupported type");
		break;
	}

	return HPMBlockID(blockID.m_integerID);
}

template <> HPMBlockID SMExternalProviderDataStore<Utf8String, DRange3d>::StoreBlock(Utf8String* DataTypeArray, size_t countData, HPMBlockID blockID)
{
	if (m_dataType != SMStoreDataType::CoverageName)
	{
		assert(false);
		return 0;
	}
	switch (m_dataType)
	{
	case SMStoreDataType::CoverageName:
		m_clipProvider->SetTerrainRegionName(*DataTypeArray, blockID.m_integerID);
		break;
	default:
		assert(!"Unsupported type");
		break;
	}

	return HPMBlockID(blockID.m_integerID);
}

template <class DATATYPE, class EXTENT> size_t SMExternalProviderDataStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID) const
{
	assert(m_dataType != SMStoreDataType::PointAndTriPtIndices && m_dataType != SMStoreDataType::Cesium3DTiles);

	return GetBlockDataCount(blockID, m_dataType);
}


template <class DATATYPE, class EXTENT> size_t SMExternalProviderDataStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const
{
	size_t blockDataCount = 0;
	bvector<DATATYPE> dataForClip;
	switch (dataType)
	{
	case SMStoreDataType::ClipDefinition:
		m_clipProvider->GetClipPolygon(dataForClip, blockID.m_integerID);
		blockDataCount = dataForClip.size();
		break;
	case SMStoreDataType::CoveragePolygon:
		m_clipProvider->GetTerrainRegion(dataForClip, blockID.m_integerID);
		blockDataCount = dataForClip.size(); 
		break;
	default:
		assert(!"Unsupported type");
		break;
	}

	return blockDataCount;
}

template <> size_t SMExternalProviderDataStore<Utf8String, DRange3d>::GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const
{
	size_t blockDataCount = 0;
	Utf8String str;
	switch (dataType)
	{
	case SMStoreDataType::CoverageName:
		m_clipProvider->GetTerrainRegionName(str, blockID.m_integerID);
		blockDataCount = str.empty() ? 0 : 1;
	default:
		assert(!"Unsupported type");
		break;
	}

	return blockDataCount;
}

template <class DATATYPE, class EXTENT> void SMExternalProviderDataStore<DATATYPE, EXTENT>::ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta)
{
	assert(!"Unsupported type");
}

template <class DATATYPE, class EXTENT> void SMExternalProviderDataStore<DATATYPE, EXTENT>::ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta, SMStoreDataType dataType)
{

    assert(!"Unsupported type");
}



template <class DATATYPE, class EXTENT> size_t SMExternalProviderDataStore<DATATYPE, EXTENT>::LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
{
	if (!blockID.IsValid() || maxCountData == 0) return 0;

	bvector<DATATYPE> dataForClip;
	switch (m_dataType)
	{
	case SMStoreDataType::ClipDefinition:
		m_clipProvider->GetClipPolygon(dataForClip, blockID.m_integerID);
		break;
	case SMStoreDataType::CoveragePolygon:
		m_clipProvider->GetTerrainRegion(dataForClip, blockID.m_integerID);
		break;
	default:
		assert(!"Unsupported type");
		break;
	} 

	if(!dataForClip.empty())
		memcpy(DataTypeArray, dataForClip.data(), std::min(dataForClip.size() * sizeof(DATATYPE), maxCountData * sizeof(DATATYPE)));
	return std::min(dataForClip.size() * sizeof(DATATYPE), maxCountData * sizeof(DATATYPE));
}


template <> size_t SMExternalProviderDataStore<Utf8String, DRange3d>::LoadBlock(Utf8String* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
{
	if (!blockID.IsValid() || maxCountData == 0) return 0;
	Utf8String str;
	switch (m_dataType)
	{
	case SMStoreDataType::CoverageName:
			m_clipProvider->GetTerrainRegionName(str, blockID.m_integerID);
			DataTypeArray[0] = str;
			return 1;
	default:
		assert(!"Unsupported type");
		break;
	}

	return 0;
}


template <class DATATYPE, class EXTENT> size_t SMExternalProviderDataStore<DATATYPE, EXTENT>::LoadCompressedBlock(bvector<uint8_t>& DataTypeArray, size_t maxCountData, HPMBlockID blockID)
{
	assert(!"Unsupported type");
	return 0;
}

template <class DATATYPE, class EXTENT> bool SMExternalProviderDataStore<DATATYPE, EXTENT>::DestroyBlock(HPMBlockID blockID)
{
	switch (m_dataType)
	{
	case SMStoreDataType::CoveragePolygon:
		m_clipProvider->RemoveTerrainRegion(blockID.m_integerID);
		return true;
	case SMStoreDataType::ClipDefinition:
		m_clipProvider->RemoveClipPolygon(blockID.m_integerID);
		return true;
	case SMStoreDataType::CoverageName:
		m_clipProvider->RemoveTerrainRegionName(blockID.m_integerID);
		return true;
	}
	return false;
}


template <class DATATYPE, class EXTENT> bool SMExternalProviderDataStore<DATATYPE, EXTENT>::GetClipDefinitionExtOps(IClipDefinitionExtOpsPtr& clipDefinitionExOpsPtr)
{
	if (m_dataType != SMStoreDataType::ClipDefinition && m_dataType != SMStoreDataType::CoveragePolygon && m_dataType != SMStoreDataType::CoverageName )
	{
		assert(!"Unexpected call");
		return false;
	}

	clipDefinitionExOpsPtr = new SMExternalClipDefinitionExtOps(m_clipProvider);

	return true;
}