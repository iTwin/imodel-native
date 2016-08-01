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
#include <ImagePP/all/h/HFCPtr.h>
#include <ScalableMesh/IScalableMesh.h>
#include "..\SMPointTileStore.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

typedef DRange3d       Extent3dType;
class ClipRegistry : public HFCShareableObject<ClipRegistry>
    {            
    WString m_path;
    bmap<uint64_t, bvector<DPoint3d>> m_clipDefs;
    uint64_t m_maxID;
    
    ISMDataStoreTypePtr<Extent3dType> m_smDataStore;

    public:

    ClipRegistry(ISMDataStoreTypePtr<Extent3dType>& smDataStore);            
        
    ~ClipRegistry();
                    
    void StoreAllClips();        

    uint64_t AddClip(const DPoint3d* clip, size_t clipSize);
        
    void ModifyClip(uint64_t id, const DPoint3d* clip, size_t clipSize);        

    void DeleteClip(uint64_t id);
        
    bool HasClip(uint64_t id);
        
    bool HasSkirt(uint64_t id);
        
    void GetClip(uint64_t id, bvector<DPoint3d>& clip);
        
    uint64_t AddSkirts(const bvector<bvector<DPoint3d>>& skirts);
        
    void ModifySkirt(uint64_t id, const bvector<bvector<DPoint3d>>& skirts);
        
    void DeleteSkirt(uint64_t id);
        
    void GetSkirt(uint64_t id, bvector<bvector<DPoint3d>>& skirts);
        
    size_t GetNbClips();
        
    void SetClipMetadata(uint64_t id, double importance, int nDimensions);        

    void GetClipMetadata(uint64_t id, double& importance, int& nDimensions);
        
    void GetAllClipsIds(bvector<uint64_t>& allClipIds);  

    void SetAutoCommit(bool autoCommit);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE