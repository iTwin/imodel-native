/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "ClipRegistry.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

ClipRegistry::ClipRegistry(ISMDataStoreTypePtr<Extent3dType>& smDataStore)
    {                  
    m_smDataStore = smDataStore;                
    m_maxID = 0;  
    m_lastClipID = 0;   
    m_lastClipSet = false;
    }

ClipRegistry::~ClipRegistry()
    {    
    }

uint64_t ClipRegistry::AddClip(const DPoint3d* clip, size_t clipSize)
    {     
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, true);       
    dataStore->StoreBlock(const_cast<DPoint3d*>(clip), clipSize, m_maxID);
    return m_maxID++;
    }

void ClipRegistry::ModifyClip(uint64_t id, const DPoint3d* clip, size_t clipSize)
    {    
		{
			std::lock_guard<std::mutex> lock(m_lastClipMutex);
			if (m_lastClipSet && m_lastClipID == id)
			{
				m_lastClipValue.resize(clipSize);
				memcpy(&m_lastClipValue[0], clip, clipSize * sizeof(DPoint3d));
			}
		}
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, true);
    dataStore->StoreBlock(const_cast<DPoint3d*>(clip), clipSize, id);
    }

void ClipRegistry::ModifyClip(uint64_t id, const ClipVectorPtr& clip,  SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
{
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, true);

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);
    clipDefinitionExOpsPtr->StoreClipWithParameters(clip, id, geom, type, isActive);
}

void ClipRegistry::AddClipWithParameters(uint64_t clipID, const DPoint3d* pts, size_t ptsSize, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
    {
		{
			std::lock_guard<std::mutex> lock(m_lastClipMutex);
			if (m_lastClipSet && m_lastClipID == clipID)
			{
				m_lastClipValue.resize(ptsSize);
				memcpy(&m_lastClipValue[0], pts, ptsSize * sizeof(DPoint3d));
				
			}
		}
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, true);

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);
    bvector<DPoint3d> clipData;
    clipData.insert(clipData.end(), pts, pts + ptsSize);
    clipDefinitionExOpsPtr->StoreClipWithParameters(clipData, clipID, geom, type, isActive);
    }

void ClipRegistry::AddClipWithParameters(uint64_t clipID, const ClipVectorPtr& clip, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
{
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, true);

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);
    clipDefinitionExOpsPtr->StoreClipWithParameters(clip, clipID, geom, type, isActive);
}

void ClipRegistry::DeleteClip(uint64_t id)
    {       
    std::lock_guard<std::mutex> lock(m_lastClipMutex);

    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, false))
        return;
    dataStore->DestroyBlock(/*m_clips[id].GetBlockID()*/id);
    if (m_lastClipSet && m_lastClipID == id)
        {
        m_lastClipID = 0;
        m_lastClipSet = false;
        m_lastClipValue.clear();
        }
    }

bool ClipRegistry::HasClip(uint64_t id)
    {    
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, false))
        return false;        
    size_t nOfPts = dataStore->GetBlockDataCount(id);
    if (nOfPts == 0) return false;
    else return true;
    }

bool ClipRegistry::IsClipDefinitionFileExist()
    {
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, false))
        return false;

    return true;
    }

bool ClipRegistry::HasSkirt(uint64_t id)
    {    
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::Skirt, false))
        return false;

    size_t nOfPts = dataStore->GetBlockDataCount(id);
    if (nOfPts == 0) return false;
    else return true;
    }

void ClipRegistry::GetClip(uint64_t id, bvector<DPoint3d>& clip)
    {  
		{
			std::lock_guard<std::mutex> lock(m_lastClipMutex);
			if (m_lastClipSet && m_lastClipID == id)
			{
				clip = m_lastClipValue;
				return;
			}
		}
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, false))
        return;

    size_t nOfPts = dataStore->GetBlockDataCount(id);
    if (nOfPts == 0) return;
    else clip.resize(nOfPts);
    dataStore->LoadBlock(&clip[0], nOfPts, id);
	{
		std::lock_guard<std::mutex> lock(m_lastClipMutex);
		m_lastClipSet = true;
		m_lastClipID = id;
		m_lastClipValue = clip;
	}
    }

void ClipRegistry::GetClipWithParameters(uint64_t id, bvector<DPoint3d>& clip, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive)
    {
    /*if (m_lastClipSet && m_lastClipID == id)
        {
        clip = m_lastClipValue;
        return;
        }*/
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, false))
        return;

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);
    clipDefinitionExOpsPtr->LoadClipWithParameters(clip, id, geom, type, isActive);
	{
		std::lock_guard<std::mutex> lock(m_lastClipMutex);
		m_lastClipSet = true;
		m_lastClipID = id;
		m_lastClipValue = clip;
	}
    }

void ClipRegistry::GetClipWithParameters(uint64_t id, ClipVectorPtr& clip, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive)
{
    /*if (m_lastClipSet && m_lastClipID == id)
    {
    clip = m_lastClipValue;
    return;
    }*/
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, false))
        return;

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);
    clipDefinitionExOpsPtr->LoadClipWithParameters(clip, id, geom, type, isActive);
}

uint64_t ClipRegistry::AddSkirts(const bvector<bvector<DPoint3d>>& skirts)
    {       
    bvector<DPoint3d> newSkirts;
    for (auto& skirt : skirts)
        {
        newSkirts.insert(newSkirts.end(), skirt.begin(), skirt.end());
        newSkirts.push_back(DPoint3d::From(DBL_MAX, DBL_MAX, DBL_MAX));
        }

    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::Skirt, true);
    dataStore->StoreBlock(newSkirts.data(), newSkirts.size(), m_maxID);
    return m_maxID++;
    }

void ClipRegistry::ModifySkirt(uint64_t id, const bvector<bvector<DPoint3d>>& skirts)
    {    
    bvector<DPoint3d> newSkirts;
    for (auto& skirt : skirts)
        {
        newSkirts.insert(newSkirts.end(), skirt.begin(), skirt.end());
        newSkirts.push_back(DPoint3d::From(DBL_MAX, DBL_MAX, DBL_MAX));
        }

    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::Skirt, true);
    dataStore->StoreBlock(newSkirts.data(), newSkirts.size(), id);
    }


void ClipRegistry::DeleteSkirt(uint64_t id)
    {
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::Skirt, false))
        return;
    dataStore->DestroyBlock(id);    
    }

void ClipRegistry::GetSkirt(uint64_t id, bvector<bvector<DPoint3d>>& skirts)
    {
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::Skirt, false))
        return;
    

    bvector<DPoint3d> outSkirt;
    size_t nOfPts = dataStore->GetBlockDataCount(id);
    if (nOfPts == 0) return;
    else outSkirt.resize(nOfPts);
    dataStore->LoadBlock(&outSkirt[0], nOfPts, id);
    bvector<DPoint3d> curSkirt;
    for (auto&pt : outSkirt)
        {
        if (pt.x == DBL_MAX)
            {
            skirts.push_back(curSkirt);
            curSkirt.clear();
            }
        else curSkirt.push_back(pt);
        }

    }


size_t ClipRegistry::GetNbClips()
    {
    return 0;
    }

void ClipRegistry::SetClipMetadata(uint64_t id, double importance, int nDimensions)
    {    
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, false))
        { 
        assert(!"Should exist");
        return;
        }

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);            
    clipDefinitionExOpsPtr->SetMetadata(id, importance, nDimensions);
    }

void ClipRegistry::GetClipMetadata(uint64_t id, double& importance, int& nDimensions)
    {    
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, false))
        return;

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);            
    clipDefinitionExOpsPtr->GetMetadata(id, importance, nDimensions);
    }

void ClipRegistry::GetAllClipsIds(bvector<uint64_t>& allClipIds)
    {    
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, false))
        return;

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);            
    clipDefinitionExOpsPtr->GetAllIDs(allClipIds);
    } 

void ClipRegistry::GetIsClipActive(uint64_t id, bool& isActive)
    {
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, false))
        return;

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);
    clipDefinitionExOpsPtr->GetIsClipActive(id, isActive);
    }

void ClipRegistry::GetClipType(uint64_t id, SMNonDestructiveClipType& type)
    {
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, false))
        return;

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);
    clipDefinitionExOpsPtr->GetClipType(id, type);
    }

void ClipRegistry::SetClipOnOrOff(uint64_t id, bool isActive)
    {
    ISM3DPtDataStorePtr dataStore;    
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, false))
        return;

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);
    clipDefinitionExOpsPtr->SetClipOnOrOff(id, isActive);
    }

void ClipRegistry::SetAutoCommit(bool autoCommit)
    {
    ISM3DPtDataStorePtr dataStore;    
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition, false))
        return;

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);            
    clipDefinitionExOpsPtr->SetAutoCommit(autoCommit);    
    }

void ClipRegistry::ModifyCoverage(uint64_t id, const DPoint3d* clip, size_t clipSize, const Utf8String& coverageName)
    {
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::CoveragePolygon, true);
    dataStore->StoreBlock(const_cast<DPoint3d*>(clip), clipSize, id);
    
    ISMCoverageNameDataStorePtr nameDataStore;
    m_smDataStore->GetSisterNodeDataStore(nameDataStore, 0, true);
    Utf8String coverageNameStr(coverageName);
    nameDataStore->StoreBlock(&coverageNameStr, 1, id);
    }

void ClipRegistry::GetCoverage(uint64_t id, bvector<DPoint3d>& clip)
    {
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::CoveragePolygon, false))
        return;

    size_t nOfPts = dataStore->GetBlockDataCount(id);
    if (nOfPts == 0) return;
    else clip.resize(nOfPts);
    dataStore->LoadBlock(&clip[0], nOfPts, id);
    }


void ClipRegistry::GetCoverageName(uint64_t id, Utf8String& coverageName)
    {    
    ISMCoverageNameDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, false))
        return;

    size_t nbName = dataStore->GetBlockDataCount(id);
    if (nbName == 0) return;
    
    dataStore->LoadBlock(&coverageName, 1, id);
    }

bool ClipRegistry::HasCoverage(uint64_t id)
    {
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::CoveragePolygon, false))
        return false;
    size_t nOfPts = dataStore->GetBlockDataCount(id);
    return nOfPts > 0;
    }

void ClipRegistry::GetAllCoveragePolygons(bvector<bvector<DPoint3d>>& allPolys)
    {
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::CoveragePolygon, false))
        return;

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);
    clipDefinitionExOpsPtr->GetAllPolys(allPolys);
    }

void ClipRegistry::GetAllCoverageIds(bvector<uint64_t>& allIds)
    {
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::CoveragePolygon, false))
        return;

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);
    clipDefinitionExOpsPtr->GetAllCoverageIDs(allIds);
    }


void ClipRegistry::DeleteCoverage(uint64_t id)
    {
    ISM3DPtDataStorePtr dataStore;
    if (!m_smDataStore->GetSisterNodeDataStore(dataStore, 0, SMStoreDataType::CoveragePolygon, false))
        return;
    dataStore->DestroyBlock(id);
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE