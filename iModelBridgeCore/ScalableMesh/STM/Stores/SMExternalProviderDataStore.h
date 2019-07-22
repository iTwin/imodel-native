#pragma once

#include "ISMDataStore.h"
#include "SMSQLiteSisterFile.h"
#include "SMStoreUtils.h"

template <class DATATYPE, class EXTENT> class SMExternalProviderDataStore : public ISMNodeDataStore<DATATYPE>
{
private:


	SMIndexNodeHeader<EXTENT>* m_nodeHeader;
	SMStoreDataType            m_dataType;
	IClipDefinitionDataProvider* m_clipProvider;

public:

	SMExternalProviderDataStore(SMStoreDataType dataType, SMIndexNodeHeader<EXTENT>* nodeHeader, IClipDefinitionDataProvider* provider);

	virtual ~SMExternalProviderDataStore();

	virtual HPMBlockID StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID) override;

	virtual size_t GetBlockDataCount(HPMBlockID blockID) const override;

	virtual size_t GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const override;

	virtual size_t LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;

	virtual size_t LoadCompressedBlock(bvector<uint8_t>& DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;

	virtual bool DestroyBlock(HPMBlockID blockID) override;

	virtual void ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) override;

	virtual void ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta, SMStoreDataType dataType) override;

	virtual bool GetClipDefinitionExtOps(IClipDefinitionExtOpsPtr& clipDefinitionExOpsPtr) override;
};



class SMExternalClipDefinitionExtOps : public IClipDefinitionExtOps
{
private:

	IClipDefinitionDataProvider* m_clipProvider;


public:

	SMExternalClipDefinitionExtOps(IClipDefinitionDataProvider* provider);

	virtual ~SMExternalClipDefinitionExtOps() {}

	virtual void GetMetadata(uint64_t id, double& importance, int& nDimensions) override {}

	virtual void SetMetadata(uint64_t id, double importance, int nDimensions) override {}

	virtual void GetAllIDs(bvector<uint64_t>& allIds) override;

	virtual void GetIsClipActive(uint64_t id, bool& isActive) override { }

	virtual void GetClipType(uint64_t id, SMNonDestructiveClipType& type) override;

	virtual void SetClipOnOrOff(uint64_t id, bool isActive) override {}

	virtual void GetAllPolys(bvector<bvector<DPoint3d>>& polys) override {}

	virtual void SetAutoCommit(bool autoCommit) override {}

	virtual void GetAllCoverageIDs(bvector<uint64_t>& allIds) override;

	virtual void StoreClipWithParameters(const bvector<DPoint3d>& clipData, uint64_t id, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) override;

	virtual void LoadClipWithParameters(bvector<DPoint3d>& clipData, uint64_t id, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive) override;

    virtual void StoreClipWithParameters(const ClipVectorPtr& clip, uint64_t id, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) override;

    virtual void LoadClipWithParameters(ClipVectorPtr& clip, uint64_t id, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive) override;

};