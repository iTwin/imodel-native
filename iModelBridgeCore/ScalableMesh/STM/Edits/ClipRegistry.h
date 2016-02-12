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
#include <Geom/GeomApi.h>
#include <ScalableMesh/IScalableMesh.h>
#include "..\SMPointTileStore.h"
#include "..\SMSQLiteClipDefinitionsTileStore.h"
//#include "..\PointTypeDPoint3d.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

typedef IDTMFile::Extent3d64f        YProtPtExtentType;
class ClipRegistry : public HFCShareableObject<ClipRegistry>
    {
    //HFCPtr<SMPointTaggedTileStore<DPoint3d, YProtPtExtentType>> m_clipStore;
    HFCPtr<SMSQLiteClipDefinitionsTileStore<YProtPtExtentType>> m_clipStore;
    HFCPtr<HPMCountLimitedPool<DPoint3d>> m_pool;
    vector<HPMStoredPooledVector<DPoint3d>> m_clips;
    SMPointIndexHeader<YProtPtExtentType> h;
    WString m_path;
    public:

    ClipRegistry(const WString& fileName)
        {
        m_pool = new HPMCountLimitedPool<DPoint3d>(new HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment(100, 2000 * sizeof(DPoint3d)), 200000);
       // IDTMFile::File::Ptr filePtr = IDTMFile::File::Open(fileName.c_str());
        StatusInt status;
        SMSQLiteFilePtr filePtr = SMSQLiteFile::Open(fileName.c_str(), false, status);
        m_path = fileName;
        if (status && nullptr != filePtr.get()) m_clipStore = new SMSQLiteClipDefinitionsTileStore<YProtPtExtentType>(filePtr);//new SMPointTaggedTileStore<DPoint3d, YProtPtExtentType>(filePtr, false);
        h.m_depth = 0;
        h.m_SplitTreshold = 1;
        if (filePtr == NULL /*|| !m_clipStore->LoadMasterHeader(&h, 1)*/)
            {
            m_clipStore = NULL;
            }
            //m_clipStore->StoreMasterHeader(&h, 0);
        m_clips.resize(h.m_depth);
        for (auto& clip : m_clips)
            {
            clip.SetBlockID(&clip - &m_clips[0]);
            if (clip.Discarded()) clip.Inflate();
            }

        }

    ~ClipRegistry()
        {
        for (auto& clip : m_clips)
            {
            clip.UnPin();
            if(clip.IsDirty() && !clip.Discarded()) clip.Discard();
            }
        if (m_clipStore != NULL)
            {
            m_clipStore->StoreMasterHeader(&h, 0);
            m_clipStore->Close();
            }
        }

    void OpenStore()
        {
        //IDTMFile::File::Ptr filePtr = IDTMFile::File::Create(m_path.c_str());
        StatusInt status;
        SMSQLiteFilePtr filePtr = SMSQLiteFile::Open(m_path.c_str(), false, status);
        filePtr->Create(m_path.c_str());
        if (filePtr.get() != nullptr && filePtr->IsOpen()) m_clipStore = new SMSQLiteClipDefinitionsTileStore<YProtPtExtentType>(filePtr);//new SMPointTaggedTileStore<DPoint3d, YProtPtExtentType>(filePtr, false);
        }

    uint64_t AddClip(const DPoint3d* clip, size_t clipSize)
        {
        if (m_clipStore == NULL) OpenStore();
        if (m_clips.size() + 1 > m_clips.capacity())
            {
            for (auto& element : m_clips)
                {
                    element.UnPin();
                    if (!element.Discarded()) element.Discard();
                }
            m_clips.resize(m_clips.size() + 1);
            for (size_t i = 0; i < m_clips.size() - 1; ++i) m_clips[i].Pin();
            }
        else m_clips.resize(m_clips.size() + 1);
        h.m_depth += 1;
        m_clipStore->StoreMasterHeader(&h, 0);
        auto& newClip = m_clips.back();
        newClip.SetStore(m_clipStore);
        newClip.SetPool(m_pool);
        newClip.push_back(clip, clipSize);
        newClip.Discard();
        newClip.Inflate();
        newClip.Pin();
        return newClip.GetBlockID().m_integerID;
        }

    void ModifyClip(uint64_t id, const DPoint3d* clip, size_t clipSize)
        {
        m_clips[id].clear();
        m_clips[id].push_back(clip, clipSize);
        }


    void DeleteClip(uint64_t id, const DPoint3d* clip, size_t clipSize)
        {
        m_clips[id].clear();
        m_clipStore->DestroyBlock(m_clips[id].GetBlockID());
        m_clips[id].SetDirty(false);
        }

    void GetClip(uint64_t id, bvector<DPoint3d>& clip)
        {
        if (id < 0 || id > m_clips.size()) return;
        clip.resize(m_clips[id].size());
        m_clips[id].get(&clip[0], clip.size());
        }

    size_t GetNbClips()
        {
        return m_clips.size();
        }
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE