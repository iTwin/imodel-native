/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Edits/ClipRegistry.h $
|    $RCSfile: ClipRegistry.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/09/14 15:28:03 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <Bentley/Bentley.h>
#include <Bentley/bvector.h>
#include <ScalableMesh/IScalableMesh.h>
#include "..\SMPointTileStore.h"
#include "..\SMSQLiteClipDefinitionsTileStore.h"
#include "..\SMSQLiteSkirtDefinitionsTileStore.h"
//#include "..\PointTypeDPoint3d.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

//typedef IDTMFile::Extent3d64f        YProtPtExtentType;
typedef DRange3d       YProtPtExtentType;
class ClipRegistry : public HFCShareableObject<ClipRegistry>
    {
    //HFCPtr<SMPointTaggedTileStore<DPoint3d, YProtPtExtentType>> m_clipStore;
    HFCPtr<SMSQLiteClipDefinitionsTileStore<YProtPtExtentType>> m_clipStore;
    HFCPtr<SMSQLiteSkirtDefinitionsTileStore<YProtPtExtentType>> m_skirtStore;    
    SMPointIndexHeader<YProtPtExtentType> h;
    WString m_path;
    bmap<uint64_t, bvector<DPoint3d>> m_clipDefs;
    uint64_t m_maxID;

    public:

    ClipRegistry(const WString& fileName)
        {       
       // IDTMFile::File::Ptr filePtr = IDTMFile::File::Open(fileName.c_str());
        StatusInt status;
        SMSQLiteFilePtr filePtr = SMSQLiteFile::Open(fileName.c_str(), false, status);
        m_path = fileName;
        if (status && nullptr != filePtr.get()) m_clipStore = new SMSQLiteClipDefinitionsTileStore<YProtPtExtentType>(filePtr);//new SMPointTaggedTileStore<DPoint3d, YProtPtExtentType>(filePtr, false);
        h.m_depth = 0;
        h.m_SplitTreshold = 1;
        m_maxID = 0;
        if (filePtr == NULL /*|| !m_clipStore->LoadMasterHeader(&h, 1)*/)
            {
            m_clipStore = NULL;
            }
            //m_clipStore->StoreMasterHeader(&h, 0);
        //m_clips.resize(h.m_depth);
        /*for (auto& clip : m_clips)
            {
            clip.SetBlockID(&clip - &m_clips[0]);
            if (clip.Discarded()) clip.Inflate();
            }*/
        if (filePtr != NULL && m_clipStore != NULL)
            {
            m_skirtStore = new SMSQLiteSkirtDefinitionsTileStore<YProtPtExtentType>(filePtr);
            LoadAllClips();
            }

        }

    SMSQLiteFilePtr GetFile()
        {
        if (m_clipStore == NULL) OpenStore();
        return m_clipStore->GetFile();
        }

    ~ClipRegistry()
        {
        StoreAllClips();
        /*for (auto& clip : m_clips)
            {
            clip.UnPin();
            if(clip.IsDirty() && !clip.Discarded()) clip.Discard();
            }*/
        if (m_clipStore != NULL)
            {
            m_clipStore->StoreMasterHeader(&h, 0);
            m_clipStore->Close();
            }
        }

    void OpenStore()
        {
        //IDTMFile::File::Ptr filePtr = IDTMFile::File::Create(m_path.c_str());
        StatusInt status = 0;
        SMSQLiteFilePtr filePtr = SMSQLiteFile::Open(m_path.c_str(), false, status);
        Utf8String utf8Path(m_path);
        if (status==0) filePtr->Create(utf8Path.c_str());
        if (filePtr.get() != nullptr && filePtr->IsOpen())
            {
            m_clipStore = new SMSQLiteClipDefinitionsTileStore<YProtPtExtentType>(filePtr);
            m_skirtStore = new SMSQLiteSkirtDefinitionsTileStore<YProtPtExtentType>(filePtr);
            }//new SMPointTaggedTileStore<DPoint3d, YProtPtExtentType>(filePtr, false);
        }

    void LoadAllClips()
        {
        //m_maxID = m_clipStore->GetNextID();
        }

    void StoreAllClips()
        {

        }

    uint64_t AddClip(const DPoint3d* clip, size_t clipSize)
        {
        if (m_clipStore == NULL) OpenStore();
     
        m_clipStore->StoreBlock(const_cast<DPoint3d*>(clip), clipSize, m_maxID);
        return m_maxID++;
        }

    void ModifyClip(uint64_t id, const DPoint3d* clip, size_t clipSize)
        {
        //m_clips[id].clear();
        //m_clips[id].push_back(clip, clipSize);
        if (m_clipStore == NULL) OpenStore();
        m_clipStore->StoreBlock(const_cast<DPoint3d*>(clip), clipSize, id);
        }


    void DeleteClip(uint64_t id)
        {
        //m_clips[id].clear();
        if (m_clipStore == NULL) OpenStore();
        m_clipStore->DestroyBlock(/*m_clips[id].GetBlockID()*/id);
        //m_clips[id].SetDirty(false);
        }

    void GetClip(uint64_t id, bvector<DPoint3d>& clip)
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

    uint64_t AddSkirts(const bvector<bvector<DPoint3d>>& skirts)
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

    void ModifySkirt(uint64_t id, const bvector<bvector<DPoint3d>>& skirts)
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


    void DeleteSkirt(uint64_t id)
        {
        //m_clips[id].clear();
        m_skirtStore->DestroyBlock(/*m_clips[id].GetBlockID()*/id);
        //m_clips[id].SetDirty(false);
        }

    void GetSkirt(uint64_t id, bvector<bvector<DPoint3d>>& skirts)
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

    size_t GetNbClips()
        {
        return 0;//m_clipStore->CountClips();// m_clips.size();
        }

    void SetClipMetadata(uint64_t id, double importance, int nDimensions)
        {
        if (m_clipStore == NULL) OpenStore();
        m_clipStore->SetMetadata(id, importance, nDimensions);
        }

    void GetClipMetadata(uint64_t id, double& importance, int& nDimensions)
        {
        if (m_clipStore == NULL) OpenStore();
        m_clipStore->GetMetadata(id, importance, nDimensions);
        }

    void GetAllClipsIds(bvector<uint64_t>& allClipIds)
        {
        if (m_clipStore == NULL) OpenStore();
        m_clipStore->GetAllIDs(allClipIds);
        }
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE