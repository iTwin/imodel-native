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

ClipRegistry::ClipRegistry(const WString& fileName)
    {              
    ISMDataStoreTypePtr<YProtPtExtentType> m_dataStore;

    StatusInt status;
    SMSQLiteFilePtr filePtr = SMSQLiteFile::Open(fileName.c_str(), false, status);
    m_path = fileName;
    if (status && nullptr != filePtr.get()) m_clipStore = new SMSQLiteClipDefinitionsTileStore<YProtPtExtentType>(filePtr);
    
    m_maxID = 0;

    if (filePtr == NULL)
        {
        m_clipStore = NULL;
        }
    
    if (filePtr != NULL && m_clipStore != NULL)
        {
        m_skirtStore = new SMSQLiteSkirtDefinitionsTileStore<YProtPtExtentType>(filePtr);
        LoadAllClips();
        }
    }

SMSQLiteFilePtr ClipRegistry::GetFile()
    {
    if (m_clipStore == NULL) OpenStore();
    return m_clipStore->GetFile();
    }

ClipRegistry::~ClipRegistry()
    {
    StoreAllClips();                
    }

void ClipRegistry::OpenStore()
    {
    //ISMStore::File::Ptr filePtr = ISMStore::File::Create(m_path.c_str());
    StatusInt status = 0;
    SMSQLiteFilePtr filePtr = SMSQLiteFile::Open(m_path.c_str(), false, status);
    Utf8String utf8Path(m_path);
    if (status==0) filePtr->Create(utf8Path.c_str());
    if (filePtr.get() != nullptr && filePtr->IsOpen())
        {
        m_clipStore = new SMSQLiteClipDefinitionsTileStore<YProtPtExtentType>(filePtr);
        m_skirtStore = new SMSQLiteSkirtDefinitionsTileStore<YProtPtExtentType>(filePtr);
        }
    }

void ClipRegistry::LoadAllClips()
    {
    //m_maxID = m_clipStore->GetNextID();
    }

void ClipRegistry::StoreAllClips()
    {

    }

uint64_t ClipRegistry::AddClip(const DPoint3d* clip, size_t clipSize)
    {
    if (m_clipStore == NULL) OpenStore();
 
    m_clipStore->StoreBlock(const_cast<DPoint3d*>(clip), clipSize, m_maxID);
    return m_maxID++;
    }

void ClipRegistry::ModifyClip(uint64_t id, const DPoint3d* clip, size_t clipSize)
    {
    //m_clips[id].clear();
    //m_clips[id].push_back(clip, clipSize);
    if (m_clipStore == NULL) OpenStore();
    m_clipStore->StoreBlock(const_cast<DPoint3d*>(clip), clipSize, id);
    }


void ClipRegistry::DeleteClip(uint64_t id)
    {
    //m_clips[id].clear();
    if (m_clipStore == NULL) OpenStore();
    m_clipStore->DestroyBlock(/*m_clips[id].GetBlockID()*/id);
    //m_clips[id].SetDirty(false);
    }

bool ClipRegistry::HasClip(uint64_t id)
    {
    if (m_clipStore == NULL) OpenStore();
    size_t nOfPts = m_clipStore->GetBlockDataCount(id);
    if (nOfPts == 0) return false;
    else return true;
    }

bool ClipRegistry::HasSkirt(uint64_t id)
    {
    if (m_skirtStore == NULL) OpenStore();
    size_t nOfPts = m_skirtStore->GetBlockDataCount(id);
    if (nOfPts == 0) return false;
    else return true;
    }

void ClipRegistry::GetClip(uint64_t id, bvector<DPoint3d>& clip)
    {
    /*if (id < 0 || id > m_clips.size()) return;
    clip.resize(m_clips[id].size());
    m_clips[id].get(&clip[0], clip.size());*/
    if (m_clipStore == NULL) OpenStore();
    size_t nOfPts = m_clipStore->GetBlockDataCount(id);
    if (nOfPts == 0) return;
    else clip.resize(nOfPts);
    m_clipStore->LoadBlock(&clip[0], nOfPts, id);
    }

uint64_t ClipRegistry::AddSkirts(const bvector<bvector<DPoint3d>>& skirts)
    {
    if (m_skirtStore == NULL) OpenStore();

    bvector<DPoint3d> newSkirts;
    for (auto& skirt : skirts)
        {
        newSkirts.insert(newSkirts.end(), skirt.begin(), skirt.end());
        newSkirts.push_back(DPoint3d::From(DBL_MAX, DBL_MAX, DBL_MAX));
        }
    m_skirtStore->StoreBlock(newSkirts.data(), newSkirts.size(), m_maxID);
    return m_maxID++;
    }

void ClipRegistry::ModifySkirt(uint64_t id, const bvector<bvector<DPoint3d>>& skirts)
    {
    if (m_skirtStore == NULL) OpenStore();

    bvector<DPoint3d> newSkirts;
    for (auto& skirt : skirts)
        {
        newSkirts.insert(newSkirts.end(), skirt.begin(), skirt.end());
        newSkirts.push_back(DPoint3d::From(DBL_MAX, DBL_MAX, DBL_MAX));
        }
    m_skirtStore->StoreBlock(newSkirts.data(), newSkirts.size(), id);
    }


void ClipRegistry::DeleteSkirt(uint64_t id)
    {
    //m_clips[id].clear();
    m_skirtStore->DestroyBlock(/*m_clips[id].GetBlockID()*/id);
    //m_clips[id].SetDirty(false);
    }

void ClipRegistry::GetSkirt(uint64_t id, bvector<bvector<DPoint3d>>& skirts)
    {
    /*if (id < 0 || id > m_clips.size()) return;
    clip.resize(m_clips[id].size());
    m_clips[id].get(&clip[0], clip.size());*/
    bvector<DPoint3d> outSkirt;
    size_t nOfPts = m_skirtStore->GetBlockDataCount(id);
    if (nOfPts == 0) return;
    else outSkirt.resize(nOfPts);
    m_skirtStore->LoadBlock(&outSkirt[0], nOfPts, id);
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
    return 0;//m_clipStore->CountClips();// m_clips.size();
    }

void ClipRegistry::SetClipMetadata(uint64_t id, double importance, int nDimensions)
    {
    if (m_clipStore == NULL) OpenStore();
    m_clipStore->SetMetadata(id, importance, nDimensions);
    }

void ClipRegistry::GetClipMetadata(uint64_t id, double& importance, int& nDimensions)
    {
    if (m_clipStore == NULL) OpenStore();
    m_clipStore->GetMetadata(id, importance, nDimensions);
    }

void ClipRegistry::GetAllClipsIds(bvector<uint64_t>& allClipIds)
    {
    if (m_clipStore == NULL) OpenStore();
    m_clipStore->GetAllIDs(allClipIds);
    }    

END_BENTLEY_SCALABLEMESH_NAMESPACE