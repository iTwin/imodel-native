/*--------------------------------------------------------------------------------------+
|    $RCSfile: ClipRegistry.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/09/14 15:28:03 $
|     $Author: Elenie.Godzaridis $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <Bentley/Bentley.h>
#include <Bentley/bvector.h>
#include <ImagePP/all/h/HFCPtr.h>
#include <ScalableMesh/IScalableMesh.h>
#include "../SMPointTileStore.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

typedef DRange3d       Extent3dType;
class ClipRegistry : public HFCShareableObject<ClipRegistry>
    {            
    WString m_path;
    bmap<uint64_t, bvector<DPoint3d>> m_clipDefs;
    uint64_t m_maxID;
    bool m_lastClipSet;
    uint64_t m_lastClipID;
    bvector<DPoint3d> m_lastClipValue;
	std::mutex m_lastClipMutex;

    ISMDataStoreTypePtr<Extent3dType> m_smDataStore;

    public:

    ClipRegistry(ISMDataStoreTypePtr<Extent3dType>& smDataStore);            
        
    ~ClipRegistry();
                    
    void StoreAllClips();        

    uint64_t AddClip(const DPoint3d* clip, size_t clipSize);

    void AddClipWithParameters(uint64_t clipID, const DPoint3d* pts, size_t ptsSize, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive);

    void AddClipWithParameters(uint64_t clipID, const ClipVectorPtr& clip, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive);
        
    void ModifyClip(uint64_t id, const DPoint3d* clip, size_t clipSize);

    void ModifyClip(uint64_t id, const ClipVectorPtr& clip, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive);

    void DeleteClip(uint64_t id);
        
    bool HasClip(uint64_t id);
        
    bool HasSkirt(uint64_t id);
        
    void GetClip(uint64_t id, bvector<DPoint3d>& clip);

    BENTLEY_SM_EXPORT void GetClipWithParameters(uint64_t id, bvector<DPoint3d>& clip, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive);

    BENTLEY_SM_EXPORT void GetClipWithParameters(uint64_t id, ClipVectorPtr& clip, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive);
        
    uint64_t AddSkirts(const bvector<bvector<DPoint3d>>& skirts);
        
    void ModifySkirt(uint64_t id, const bvector<bvector<DPoint3d>>& skirts);
        
    void DeleteSkirt(uint64_t id);
        
    void GetSkirt(uint64_t id, bvector<bvector<DPoint3d>>& skirts);

    void ModifyCoverage(uint64_t id, const DPoint3d* clip, size_t clipSize, const Utf8String& coverageName);

    void GetCoverage(uint64_t id, bvector<DPoint3d>& clip);    

    BENTLEY_SM_EXPORT  void GetCoverageName(uint64_t id, Utf8String& coverageName);

    bool HasCoverage(uint64_t id);

    void GetAllCoveragePolygons(bvector<bvector<DPoint3d>>& allPolys);

    BENTLEY_SM_EXPORT void GetAllCoverageIds(bvector<uint64_t>& ids);

    void DeleteCoverage(uint64_t id);
        
    size_t GetNbClips();
        
    void SetClipMetadata(uint64_t id, double importance, int nDimensions);        

    void GetClipMetadata(uint64_t id, double& importance, int& nDimensions);
        
    void GetAllClipsIds(bvector<uint64_t>& allClipIds);  

    void SetClipOnOrOff(uint64_t id, bool isActive);
    void GetIsClipActive(uint64_t id, bool& isActive);
	void GetClipType(uint64_t id, SMNonDestructiveClipType& type);

    bool IsClipDefinitionFileExist();
    
    void SetAutoCommit(bool autoCommit);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE