/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ScalableMesh/ScalableMeshDefs.h>
#include <Bentley\Bentley.h>
#include <GeoCoord/BaseGeoCoord.h>
#include <ScalableMesh/IScalableMeshQuery.h>

#include <Bentley/RefCounted.h>
#undef static_assert

#ifndef VANCOUVER_API // HIMMosaic apparently moved into the imagepp namespace in dgndb
namespace BENTLEY_NAMESPACE_NAME
    {
    namespace ImagePP
        {
#endif
        class HIMMosaic;

#ifndef VANCOUVER_API
        }
    }
#define MOSAIC_TYPE BENTLEY_NAMESPACE_NAME::ImagePP::HIMMosaic
#else 
#define MOSAIC_TYPE HIMMosaic
#endif
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

enum DTMAnalysisType
    {
    Precise =0,
    Fast,
    Qty
    };

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

        virtual bool          _IsTerrain() = 0;

        virtual DTMStatusInt     _GetRange(DRange3dR range) = 0;

        virtual StatusInt         _GetBoundary(bvector<DPoint3d>& boundary) = 0;

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

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*   _GetDTMInterface(DTMAnalysisType type) = 0;

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*   _GetDTMInterface(DMatrix4d& storageToUors, DTMAnalysisType type) = 0;

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

        virtual int                                 _ConvertToCloud(const WString& pi_pOutputDirPath) const = 0;

#ifdef SCALABLE_MESH_ATP
        virtual int                                 _LoadAllNodeHeaders(size_t& nbLoadedNodes, int level) const = 0;
        virtual int                                 _LoadAllNodeData(size_t& nbLoadedNodes, int level) const = 0;
        virtual int                                 _SaveGroupedNodeHeaders(const WString& pi_pOutputDirPath) const = 0;
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

        virtual void                               _GetAllClipsIds(bvector<uint64_t>& ids) = 0;

        virtual void                               _GetCurrentlyViewedNodes(bvector<IScalableMeshNodePtr>& nodes) = 0;

        virtual void                               _SetCurrentlyViewedNodes(const bvector<IScalableMeshNodePtr>& nodes) = 0;

        virtual void                               _TextureFromRaster(MOSAIC_TYPE* mosaicP, Transform unitTransform=Transform::FromIdentity()) = 0;

        virtual void                               _SetEditFilesBasePath(const Utf8String& path) = 0;

        virtual IScalableMeshNodePtr               _GetRootNode() = 0;
    /*__PUBLISH_SECTION_START__*/
    public:
        //! Gets the number of points of the DTM.
        //! @return The number of points of the DTM..

        //! Gets the draping interface.
        //! @return The draping interface.

        void TextureFromRaster(MOSAIC_TYPE* mosaicP, Transform unitTransform = Transform::FromIdentity());

        BENTLEY_SM_EXPORT __int64          GetPointCount();

        BENTLEY_SM_EXPORT bool          IsTerrain();

        BENTLEY_SM_EXPORT DTMStatusInt     GetRange(DRange3dR range);

        BENTLEY_SM_EXPORT StatusInt          GetBoundary(bvector<DPoint3d>& boundary);

        BENTLEY_SM_EXPORT int                    GenerateSubResolutions();

        BENTLEY_SM_EXPORT __int64                GetBreaklineCount() const;
            
        BENTLEY_SM_EXPORT ScalableMeshCompressionType   GetCompressionType() const;

        BENTLEY_SM_EXPORT int                    GetNbResolutions() const;    

        BENTLEY_SM_EXPORT size_t                 GetTerrainDepth() const;

        BENTLEY_SM_EXPORT IScalableMeshPointQueryPtr         GetQueryInterface(ScalableMeshQueryType queryType) const;

        BENTLEY_SM_EXPORT IScalableMeshPointQueryPtr         GetQueryInterface(ScalableMeshQueryType                queryType,                                                              
                                                                               BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS,
                                                                               const DRange3d&                      extentInTargetGCS) const;

        BENTLEY_SM_EXPORT IScalableMeshMeshQueryPtr    GetMeshQueryInterface(MeshQueryType queryType) const;

        BENTLEY_SM_EXPORT IScalableMeshMeshQueryPtr     GetMeshQueryInterface(MeshQueryType queryType,
                                                                        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,
                                                                        const DRange3d&                      extentInTargetGCS) const;

        BENTLEY_SM_EXPORT IScalableMeshNodeRayQueryPtr    GetNodeQueryInterface() const;

        BENTLEY_SM_EXPORT BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*   GetDTMInterface(DTMAnalysisType type = DTMAnalysisType::Precise);

        BENTLEY_SM_EXPORT BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*   GetDTMInterface(DMatrix4d& storageToUors, DTMAnalysisType type = DTMAnalysisType::Precise);

        BENTLEY_SM_EXPORT const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr&
                                           GetBaseGCS() const;
        BENTLEY_SM_EXPORT StatusInt              SetBaseGCS(const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCS);

        BENTLEY_SM_EXPORT const GeoCoords::GCS&  GetGCS() const;
        BENTLEY_SM_EXPORT StatusInt              SetGCS(const GeoCoords::GCS& gcs);

        BENTLEY_SM_EXPORT void                   SetEditFilesBasePath(const Utf8String& path);

        BENTLEY_SM_EXPORT ScalableMeshState             GetState() const;

        BENTLEY_SM_EXPORT bool                   IsProgressive() const;

        BENTLEY_SM_EXPORT bool                   IsReadOnly() const;

        BENTLEY_SM_EXPORT bool                   IsShareable() const;                        
         
        //Synchonization with data sources functions
        BENTLEY_SM_EXPORT bool                   InSynchWithSources() const; 

        // Deprecated. Remove.
        bool                                     InSynchWithDataSources() const { return InSynchWithSources(); }

        BENTLEY_SM_EXPORT bool                   LastSynchronizationCheck(time_t& last) const;        

        BENTLEY_SM_EXPORT int                    SynchWithSources(); 

        BENTLEY_SM_EXPORT IScalableMeshNodePtr  GetRootNode();

        BENTLEY_SM_EXPORT int                    GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const;

        BENTLEY_SM_EXPORT Count                  GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const;

        BENTLEY_SM_EXPORT uint64_t               AddClip(const DPoint3d* pts, size_t ptsSize);

        BENTLEY_SM_EXPORT bool                   AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID);

        BENTLEY_SM_EXPORT bool                   ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID);

        BENTLEY_SM_EXPORT bool                   RemoveClip(uint64_t clipID);

        BENTLEY_SM_EXPORT bool                   ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID);

        BENTLEY_SM_EXPORT bool                   AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID);

        BENTLEY_SM_EXPORT bool                   RemoveSkirt(uint64_t skirtID);

        BENTLEY_SM_EXPORT void                   SetIsInsertingClips(bool toggleInsertMode);

        BENTLEY_SM_EXPORT void                   ModifyClipMetadata(uint64_t clipId,double importance, int nDimensions);

        BENTLEY_SM_EXPORT void                   GetAllClipIds(bvector<uint64_t>& ids);

        BENTLEY_SM_EXPORT void                   GetCurrentlyViewedNodes(bvector<IScalableMeshNodePtr>& nodes);

        BENTLEY_SM_EXPORT void                   SetCurrentlyViewedNodes(const bvector<IScalableMeshNodePtr>& nodes);

        BENTLEY_SM_EXPORT int                    ConvertToCloud(const WString& pi_pOutputDirPath) const;

        BENTLEY_SM_EXPORT static IScalableMeshPtr        GetFor                 (const WChar*          filePath,
                                                                                 bool                    openReadOnly,
                                                                                 bool                    openShareable,
                                                                                 StatusInt&              status);

        BENTLEY_SM_EXPORT static IScalableMeshPtr        GetFor(const WChar*          filePath,
                                                                const Utf8String&      baseEditsFilePath,
                                                                bool                    openReadOnly,
                                                                bool                    openShareable,
                                                                StatusInt&              status);

        BENTLEY_SM_EXPORT static IScalableMeshPtr        GetFor                 (const WChar*          filePath,
                                                                                 bool                    openReadOnly,
                                                                                 bool                    openShareable);

        BENTLEY_SM_EXPORT static IScalableMeshPtr        GetFor(const WChar*          filePath,
                                                                const Utf8String&      baseEditsFilePath,
                                                                bool                    openReadOnly,
                                                                bool                    openShareable);

        BENTLEY_SM_EXPORT int                     LoadAllNodeHeaders(size_t& nbLoadedNodes, int level) const;
        BENTLEY_SM_EXPORT int                     LoadAllNodeData(size_t& nbLoadedNodes, int level) const;
        BENTLEY_SM_EXPORT int                     SaveGroupedNodeHeaders(const WString& pi_pOutputDirPath) const;

    };


struct Count
    {
    unsigned __int64 m_nbPoints;
    unsigned __int64 m_nbLinears;
    Count(unsigned __int64 nbPoints, unsigned __int64 nbLinears) { m_nbPoints = nbPoints; m_nbLinears = nbLinears; }
    };


void BENTLEY_SM_EXPORT edgeCollapseTest(WCharCP param);
void BENTLEY_SM_EXPORT edgeCollapsePrintGraph(WCharCP param);
void BENTLEY_SM_EXPORT edgeCollapseShowMesh(WCharCP param, PolyfaceQueryP& outMesh);


END_BENTLEY_SCALABLEMESH_NAMESPACE
