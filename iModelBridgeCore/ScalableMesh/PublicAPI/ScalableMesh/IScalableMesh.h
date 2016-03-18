/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <GeoCoord/BaseGeoCoord.h>
#include <ScalableMesh/IScalableMeshQuery.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <Bentley/RefCounted.h>

//ADD_BENTLEY_TYPEDEFS (BENTLEY_NAMESPACE_NAME::ScalableMesh, IDTMVolume)

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMesh;
typedef RefCountedPtr<IScalableMesh> IScalableMeshPtr;

struct IScalableMeshTileTriangulatorManager;
typedef RefCountedPtr<IScalableMeshTileTriangulatorManager> IScalableMeshTileTriangulatorManagerPtr;

namespace GeoCoords {
struct GCS;
} 

enum CountType { COUNTTYPE_POINTS, COUNTTYPE_LINEARS, COUNTTYPE_BOTH };

struct Count;

/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMesh abstract:  IRefCounted //BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM
    {
    private:
        

    /*__PUBLISH_SECTION_END__*/
    /*__PUBLISH_CLASS_VIRTUAL__*/
    protected:                         

        //Methods for the public interface.        
        virtual __int64          _GetPointCount() = 0;

        virtual DTMStatusInt     _GetRange(DRange3dR range) = 0;

        virtual int                    _GenerateSubResolutions() = 0;      

        virtual __int64                _GetBreaklineCount() const = 0;

        virtual ScalableMeshCompressionType   _GetCompressionType() const = 0;        
        
        virtual Count                  _GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const = 0;

        virtual int                    _GetNbResolutions() const = 0;

        virtual size_t                 _GetTerrainDepth() const = 0;

        virtual IScalableMeshPointQueryPtr         _GetQueryInterface(ScalableMeshQueryType queryType) const = 0;

        virtual IScalableMeshPointQueryPtr         _GetQueryInterface(ScalableMeshQueryType                queryType,                                                           
                                                                      BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,
                                                                      const DRange3d&                      extentInTargetGCS) const = 0;

        virtual IScalableMeshMeshQueryPtr     _GetMeshQueryInterface(MeshQueryType queryType) const = 0;

        virtual IScalableMeshMeshQueryPtr     _GetMeshQueryInterface(MeshQueryType                        queryType,
                                                                     BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,
                                                                     const DRange3d&                      extentInTargetGCS) const = 0;

        virtual IScalableMeshNodeRayQueryPtr     _GetNodeQueryInterface() const = 0;

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*   _GetDTMInterface() = 0;

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*   _GetDTMInterface(DMatrix4d& storageToUors) = 0;

        virtual const GeoCoords::GCS&               _GetGCS() const = 0;

        virtual StatusInt                           _SetGCS(const GeoCoords::GCS& sourceGCS) = 0;

        virtual ScalableMeshState                          _GetState() const = 0;
                       
        virtual bool                                _IsProgressive() const = 0;

        virtual bool                                _IsReadOnly() const = 0;

        virtual bool                                _IsShareable() const = 0;
        
        //Synchonization with data sources functions
        virtual bool                                _InSynchWithSources() const = 0; 

        virtual bool                                _LastSynchronizationCheck(time_t& last) const = 0;        

        virtual int                                 _SynchWithSources() = 0;  

        virtual int                                 _GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const = 0;

#ifdef SCALABLE_MESH_ATP
        virtual int                                 _LoadAllNodeHeaders(size_t& nbLoadedNodes) const = 0; 
#endif
        virtual uint64_t                           _AddClip(const DPoint3d* pts, size_t ptsSize) = 0;

        virtual bool                               _ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID) = 0;

        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID) = 0;;

        virtual bool                               _RemoveClip(uint64_t clipID) = 0;


        virtual bool                               _ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID) = 0;

        virtual bool                               _AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID) = 0;

        virtual bool                               _RemoveSkirt(uint64_t skirtID) = 0;

        virtual void                               _SetIsInsertingClips(bool toggleInsertMode) = 0;

        virtual void                               _ModifyClipMetadata(uint64_t clipId, double importance, int nDimensions) = 0;
    /*__PUBLISH_SECTION_START__*/
    public:
        //! Gets the number of points of the DTM.
        //! @return The number of points of the DTM..

        //! Gets the draping interface.
        //! @return The draping interface.

        BENTLEYSTM_EXPORT __int64          GetPointCount();

        BENTLEYSTM_EXPORT DTMStatusInt     GetRange(DRange3dR range);

        BENTLEYSTM_EXPORT int                    GenerateSubResolutions();

        BENTLEYSTM_EXPORT __int64                GetBreaklineCount() const;
            
        BENTLEYSTM_EXPORT ScalableMeshCompressionType   GetCompressionType() const;

        BENTLEYSTM_EXPORT int                    GetNbResolutions() const;    

        BENTLEYSTM_EXPORT size_t                 GetTerrainDepth() const;

        BENTLEYSTM_EXPORT IScalableMeshPointQueryPtr         GetQueryInterface(ScalableMeshQueryType queryType) const;

        BENTLEYSTM_EXPORT IScalableMeshPointQueryPtr         GetQueryInterface(ScalableMeshQueryType                queryType,                                                              
                                                                               BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS,
                                                                               const DRange3d&                      extentInTargetGCS) const;

        BENTLEYSTM_EXPORT IScalableMeshMeshQueryPtr    GetMeshQueryInterface(MeshQueryType queryType) const;

        BENTLEYSTM_EXPORT IScalableMeshMeshQueryPtr     GetMeshQueryInterface(MeshQueryType queryType,
                                                                        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,
                                                                        const DRange3d&                      extentInTargetGCS) const;

        BENTLEYSTM_EXPORT IScalableMeshNodeRayQueryPtr    GetNodeQueryInterface() const;

        BENTLEYSTM_EXPORT BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*   GetDTMInterface();

        BENTLEYSTM_EXPORT BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*   GetDTMInterface(DMatrix4d& storageToUors);

        BENTLEYSTM_EXPORT const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr&
                                           GetBaseGCS() const;
        BENTLEYSTM_EXPORT StatusInt              SetBaseGCS(const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCS);

        BENTLEYSTM_EXPORT const GeoCoords::GCS&  GetGCS() const;
        BENTLEYSTM_EXPORT StatusInt              SetGCS(const GeoCoords::GCS& gcs);

        BENTLEYSTM_EXPORT ScalableMeshState             GetState() const;

        BENTLEYSTM_EXPORT bool                   IsProgressive() const;

        BENTLEYSTM_EXPORT bool                   IsReadOnly() const;

        BENTLEYSTM_EXPORT bool                   IsShareable() const;                        
         
        //Synchonization with data sources functions
        BENTLEYSTM_EXPORT bool                   InSynchWithSources() const; 

        // Deprecated. Remove.
        bool                                     InSynchWithDataSources() const { return InSynchWithSources(); }

        BENTLEYSTM_EXPORT bool                   LastSynchronizationCheck(time_t& last) const;        

        BENTLEYSTM_EXPORT int                    SynchWithSources(); 

        BENTLEYSTM_EXPORT int                    GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const;

        BENTLEYSTM_EXPORT Count                  GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const;

        BENTLEYSTM_EXPORT uint64_t               AddClip(const DPoint3d* pts, size_t ptsSize);

        BENTLEYSTM_EXPORT bool                   AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID);

        BENTLEYSTM_EXPORT bool                   ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID);

        BENTLEYSTM_EXPORT bool                   RemoveClip(uint64_t clipID);

        BENTLEYSTM_EXPORT bool                   ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID);

        BENTLEYSTM_EXPORT bool                   AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID);

        BENTLEYSTM_EXPORT bool                   RemoveSkirt(uint64_t skirtID);

        BENTLEYSTM_EXPORT void                   SetIsInsertingClips(bool toggleInsertMode);

        BENTLEYSTM_EXPORT void                   ModifyClipMetadata(uint64_t clipId,double importance, int nDimensions);

    
        BENTLEYSTM_EXPORT static IScalableMeshPtr        GetFor                 (const WChar*          filePath,
                                                                    bool                    openReadOnly,
                                                                    bool                    openShareable,
                                                                    StatusInt&              status);

        BENTLEYSTM_EXPORT static IScalableMeshPtr        GetFor                 (const WChar*          filePath,
                                                                    bool                    openReadOnly,
                                                                    bool                    openShareable);

#ifdef SCALABLE_MESH_ATP
        BENTLEYSTM_EXPORT int                     LoadAllNodeHeaders(size_t& nbLoadedNodes) const; 
#endif

    };


struct Count
    {
    unsigned __int64 m_nbPoints;
    unsigned __int64 m_nbLinears;
    Count(unsigned __int64 nbPoints, unsigned __int64 nbLinears) { m_nbPoints = nbPoints; m_nbLinears = nbLinears; }
    };


void BENTLEYSTM_EXPORT edgeCollapseTest(WCharCP param);
void BENTLEYSTM_EXPORT edgeCollapsePrintGraph(WCharCP param);
void BENTLEYSTM_EXPORT edgeCollapseShowMesh(WCharCP param, PolyfaceQueryP& outMesh);


END_BENTLEY_SCALABLEMESH_NAMESPACE
