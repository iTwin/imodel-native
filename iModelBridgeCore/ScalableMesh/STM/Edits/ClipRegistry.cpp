/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Edits/ClipRegistry.cpp $
|    $RCSfile: ClipRegistry.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/09/14 15:28:03 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "ClipRegistry.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

ClipRegistry::ClipRegistry(ISMDataStoreTypePtr<Extent3dType>& smDataStore)
    {                  
    m_smDataStore = smDataStore;                
    m_maxID = 0;  
    m_lastClipID = 0;   
    }

ClipRegistry::~ClipRegistry()
    {    
    }

uint64_t ClipRegistry::AddClip(const DPoint3d* clip, size_t clipSize)
    {     
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition);
    dataStore->StoreBlock(const_cast<DPoint3d*>(clip), clipSize, m_maxID);
    return m_maxID++;
    }

void ClipRegistry::ModifyClip(uint64_t id, const DPoint3d* clip, size_t clipSize)
    {    
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition);
    dataStore->StoreBlock(const_cast<DPoint3d*>(clip), clipSize, id);
    }


void ClipRegistry::DeleteClip(uint64_t id)
    {       
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition);
    dataStore->DestroyBlock(/*m_clips[id].GetBlockID()*/id);    
    }

bool ClipRegistry::HasClip(uint64_t id)
    {    
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition);
    size_t nOfPts = dataStore->GetBlockDataCount(id);
    if (nOfPts == 0) return false;
    else return true;
    }

bool ClipRegistry::HasSkirt(uint64_t id)
    {    
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetNodeDataStore(dataStore, 0, SMStoreDataType::Skirt);
    size_t nOfPts = dataStore->GetBlockDataCount(id);
    if (nOfPts == 0) return false;
    else return true;
    }

void ClipRegistry::GetClip(uint64_t id, bvector<DPoint3d>& clip)
    {        
       if (m_lastClipSet && m_lastClipID == id)
            {
            clip = m_lastClipValue;
            return;
            }
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition);
    size_t nOfPts = dataStore->GetBlockDataCount(id);
    if (nOfPts == 0) return;
    else clip.resize(nOfPts);
    dataStore->LoadBlock(&clip[0], nOfPts, id);
    m_lastClipSet = true;
    m_lastClipID = id;
    m_lastClipValue = clip;
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
    m_smDataStore->GetNodeDataStore(dataStore, 0, SMStoreDataType::Skirt);
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
    m_smDataStore->GetNodeDataStore(dataStore, 0, SMStoreDataType::Skirt);    
    dataStore->StoreBlock(newSkirts.data(), newSkirts.size(), id);
    }


void ClipRegistry::DeleteSkirt(uint64_t id)
    {
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetNodeDataStore(dataStore, 0, SMStoreDataType::Skirt);    
    dataStore->DestroyBlock(id);    
    }

void ClipRegistry::GetSkirt(uint64_t id, bvector<bvector<DPoint3d>>& skirts)
    {
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetNodeDataStore(dataStore, 0, SMStoreDataType::Skirt);    
    
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
    m_smDataStore->GetNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition);

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);            
    clipDefinitionExOpsPtr->SetMetadata(id, importance, nDimensions);
    }

void ClipRegistry::GetClipMetadata(uint64_t id, double& importance, int& nDimensions)
    {    
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition);

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);            
    clipDefinitionExOpsPtr->GetMetadata(id, importance, nDimensions);
    }

void ClipRegistry::GetAllClipsIds(bvector<uint64_t>& allClipIds)
    {    
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition);    

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);            
    clipDefinitionExOpsPtr->GetAllIDs(allClipIds);
    }    

void ClipRegistry::SetAutoCommit(bool autoCommit)
    {
    ISM3DPtDataStorePtr dataStore;
    m_smDataStore->GetNodeDataStore(dataStore, 0, SMStoreDataType::ClipDefinition);

    IClipDefinitionExtOpsPtr clipDefinitionExOpsPtr;
    dataStore->GetClipDefinitionExtOps(clipDefinitionExOpsPtr);            
    clipDefinitionExOpsPtr->SetAutoCommit(autoCommit);    
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE