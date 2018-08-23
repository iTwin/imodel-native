/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ScalableMesh/ScalableMeshDefs.h>
#include <Bentley/Bentley.h>
#include <GeoCoord/BaseGeoCoord.h>
#include <ScalableMesh/ITextureProvider.h>
#include <ScalableMesh/IScalableMeshQuery.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <Bentley/RefCounted.h>
#include <ScalableMesh/IScalableMeshEdit.h>
#include <ScalableMesh/IScalableMeshAnalysis.h>
#include <ScalableMesh/IScalableMeshInfo.h>
#include <ScalableMesh/IScalableMeshClippingOptions.h>
#include "ScalableMeshDefs.h"

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

#ifdef VANCOUVER_API
#define DGN_NAMESPACE BENTLEY_NAMESPACE_NAME::DgnPlatform
#else
#define DGN_NAMESPACE BENTLEY_NAMESPACE_NAME::Dgn
#endif

#ifdef VANCOUVER_API
#define CLIP_VECTOR_NAMESPACE BENTLEY_NAMESPACE_NAME
#else
#define CLIP_VECTOR_NAMESPACE BENTLEY_NAMESPACE_NAME::Dgn
#endif

//ADD_BENTLEY_TYPEDEFS (BENTLEY_NAMESPACE_NAME::ScalableMesh, IDTMVolume)

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

typedef std::function<bool(bool& shouldCreateGraph, bvector<bvector<DPoint3d>>& newMeshPts, bvector<bvector<int32_t>>& newMeshIndexes, bvector<Utf8String>& newMeshMetadata, bvector<bvector<DPoint2d>>& newMeshUvs, bvector<bvector<uint8_t>>& newMeshTex, const bvector<IScalableMeshMeshPtr>& submeshes, const bvector<Utf8String>& meshMetadata, DRange3d nodeExt)> MeshUserFilterCallback;

struct IScalableMesh;
typedef RefCountedPtr<IScalableMesh> IScalableMeshPtr;

struct IScalableMeshTileTriangulatorManager;
typedef RefCountedPtr<IScalableMeshTileTriangulatorManager> IScalableMeshTileTriangulatorManagerPtr;

struct IScalableMeshGroundPreviewer;
typedef RefCountedPtr<IScalableMeshGroundPreviewer> IScalableMeshGroundPreviewerPtr;

struct IScalableMeshProgress;
typedef RefCountedPtr<IScalableMeshProgress>            IScalableMeshProgressPtr;

namespace GeoCoords {
struct GCS;
} 

enum CountType { COUNTTYPE_POINTS, COUNTTYPE_LINEARS, COUNTTYPE_BOTH };

struct Count;

enum DTMAnalysisType
    {
    Precise =0,
    Fast,
    RawDataOnly,
    ViewOnly,
    Qty
    };

enum SMCloudServerType
    {
    LocalDisk = 0,
    LocalDiskCURL,
    Azure,
    WSG
    };

struct IScalableMeshMemoryCounts
{

public:
    BENTLEY_SM_EXPORT static void SetMaximumMemoryUsage(size_t maxNumberOfBytes);
    BENTLEY_SM_EXPORT static void SetMaximumVideoMemoryUsage(size_t maxNumberOfBytes);

    BENTLEY_SM_EXPORT static size_t GetAmountOfUsedMemory();
    BENTLEY_SM_EXPORT static size_t GetAmountOfUsedVideoMemory();

    BENTLEY_SM_EXPORT static size_t GetMaximumMemoryUsage();
    BENTLEY_SM_EXPORT static size_t GetMaximumVideoMemoryUsage();
};

struct IScalableMeshProgress;
/*=================================================================================**//**
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMeshGroundPreviewer  : virtual public RefCountedBase
{
private:

protected:
    
    virtual bool      _IsCurrentPreviewEnough() const = 0;

    virtual StatusInt _UpdatePreview(PolyfaceQueryCR currentGround) = 0;    

    virtual bool _UpdateProgress(IScalableMeshProgress* progress) = 0;
    
public:

    BENTLEY_SM_IMPORT_EXPORT bool      IsCurrentPreviewEnough() const;

    BENTLEY_SM_IMPORT_EXPORT StatusInt UpdatePreview(PolyfaceQueryCR currentGround);

    BENTLEY_SM_IMPORT_EXPORT bool UpdateProgress(IScalableMeshProgress* progress);
        
};


/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMesh :  IRefCounted 
    {
    private:
        

    /*__PUBLISH_SECTION_END__*/
    /*__PUBLISH_CLASS_VIRTUAL__*/
    protected:                         

        //Methods for the public interface.       
        virtual int64_t          _GetPointCount() = 0;

        virtual uint64_t          _GetNodeCount() = 0;

        virtual bool          _IsTerrain() = 0;

        virtual bool          _IsTextured() = 0;

        virtual StatusInt     _GetTextureInfo(IScalableMeshTextureInfoPtr& textureInfo) const = 0;
        
        virtual bool          _IsCesium3DTiles() = 0;

        virtual bool           _IsStubFile() = 0;

        virtual Utf8String    _GetProjectWiseContextShareLink() = 0;

        virtual DTMStatusInt     _GetRange(DRange3dR range) = 0;

        virtual StatusInt         _GetBoundary(bvector<DPoint3d>& boundary) = 0;

        virtual int                    _GenerateSubResolutions() = 0;      

        virtual int64_t                _GetBreaklineCount() const = 0;

        virtual ScalableMeshCompressionType   _GetCompressionType() const = 0;        
        
        virtual Count                  _GetCountInRange (const DRange2d& range, const CountType& type, const uint64_t& maxNumberCountedPoints) const = 0;

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

        virtual IScalableMeshEditPtr    _GetMeshEditInterface() const = 0;

        virtual IScalableMeshAnalysisPtr    _GetMeshAnalysisInterface() = 0;

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*   _GetDTMInterface(DTMAnalysisType type) = 0;

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*   _GetDTMInterface(DMatrix4d& storageToUors, DTMAnalysisType type) = 0;

        virtual const GeoCoords::GCS&               _GetGCS() const = 0;

        virtual StatusInt                           _SetGCS(const GeoCoords::GCS& sourceGCS) = 0;

        virtual ScalableMeshState                          _GetState() const = 0;
                               
        virtual bool                                _IsReadOnly() const = 0;

        virtual bool                                _IsShareable() const = 0;
        
        //Synchonization with data sources functions
        virtual bool                                _InSynchWithSources() const = 0; 

        virtual bool                                _LastSynchronizationCheck(time_t& last) const = 0;                

        virtual int                                 _GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const = 0;

        virtual int                                 _Generate3DTiles(const WString& outContainerName, const WString& outDatasetName, SMCloudServerType server, IScalableMeshProgressPtr progress = nullptr, CLIP_VECTOR_NAMESPACE::ClipVectorPtr clips = nullptr, uint64_t coverageId = (uint64_t)-1) const = 0;

        virtual int                                 _SaveAs(const WString& destination, CLIP_VECTOR_NAMESPACE::ClipVectorPtr clips = nullptr, IScalableMeshProgressPtr progress = nullptr) = 0;

#ifdef SCALABLE_MESH_ATP
        virtual int                                 _ChangeGeometricError(const WString& outContainerName, const WString& outDatasetName = L"", SMCloudServerType server = SMCloudServerType::LocalDisk, const double& newGeometricErrorValue = 0.0) const = 0;
        virtual int                                 _LoadAllNodeHeaders(size_t& nbLoadedNodes, int level) const = 0;
        virtual int                                 _LoadAllNodeData(size_t& nbLoadedNodes, int level) const = 0;
        virtual int                                 _SaveGroupedNodeHeaders(const WString& pi_pOutputDirPath, const short& pi_pGroupMode) const = 0;
#endif

        virtual void _ReFilter() = 0;

        virtual uint64_t                           _AddClip(const DPoint3d* pts, size_t ptsSize) = 0;

        virtual bool                               _ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID) = 0;

        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, bool alsoAddOnTerrain = true) = 0;

        virtual bool                               _RemoveClip(uint64_t clipID) = 0;

        virtual bool                               _GetClip(uint64_t clipID, bvector<DPoint3d>& clipData) = 0;

        virtual void                               _SynchronizeClipData(const bvector<bpair<uint64_t, bvector<DPoint3d>>>& listOfClips, const bvector<bpair<uint64_t, bvector<bvector<DPoint3d>>>>& listOfSkirts) = 0;


        virtual bool                               _GetSkirt(uint64_t skirtID, bvector<bvector<DPoint3d>>& skirt) = 0;

        virtual bool                               _ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID) = 0;

        virtual bool                               _AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID, bool alsoAddOnTerrain = true) = 0;

        virtual bool                               _RemoveSkirt(uint64_t skirtID) = 0;

        virtual bool                               _IsInsertingClips() = 0;

        virtual void                               _SetIsInsertingClips(bool toggleInsertMode) = 0;

        virtual bool                               _ShouldInvertClips() = 0;

        virtual void                               _SetInvertClip(bool invertClips) = 0;

        virtual void                               _ModifyClipMetadata(uint64_t clipId, double importance, int nDimensions) = 0;

        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) = 0;

        virtual bool                               _ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) = 0;


        virtual bool                               _AddClip(const CLIP_VECTOR_NAMESPACE::ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) = 0;

        virtual bool                               _ModifyClip(const CLIP_VECTOR_NAMESPACE::ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) = 0;


        virtual void                               _GetAllClipsIds(bvector<uint64_t>& ids) = 0;
        virtual void                               _SetClipOnOrOff(uint64_t id, bool isActive) = 0;
        virtual void                               _GetIsClipActive(uint64_t id, bool& isActive) = 0;

        virtual void                               _GetClipType(uint64_t id, SMNonDestructiveClipType& type) = 0;

		virtual void                              _CompactExtraFiles() = 0;

		virtual void                              _WriteExtraFiles() = 0;

        virtual void                               _GetCurrentlyViewedNodes(bvector<IScalableMeshNodePtr>& nodes) = 0;

        virtual void                               _SetCurrentlyViewedNodes(const bvector<IScalableMeshNodePtr>& nodes) = 0;

        virtual void                               _TextureFromRaster(ITextureProviderPtr provider) = 0;
        
        virtual void                               _SetEditFilesBasePath(const Utf8String& path) = 0;

        virtual Utf8String                         _GetEditFilesBasePath() = 0;

        virtual void                               _GetExtraFileNames(bvector<BeFileName>& extraFileNames) const = 0;

        virtual IScalableMeshNodePtr               _GetRootNode() = 0;

        virtual void                               _ImportTerrainSM(WString terrainPath) = 0;

        virtual SMStatus                      _DetectGroundForRegion(BeFileName& createdTerrain, const BeFileName& coverageTempDataFolder, const bvector<DPoint3d>& coverageData, uint64_t id, IScalableMeshGroundPreviewerPtr groundPreviewer, GeoCoordinates::BaseGCSCPtr& destinationGcs, bool limitResolution) = 0;
                                                                                                                                                                 
        virtual BentleyStatus                      _CreateCoverage(const bvector<DPoint3d>& coverageData, uint64_t id, const Utf8String& coverageName) = 0;

        virtual void                               _GetAllCoverages(bvector<bvector<DPoint3d>>& coverageData) = 0;

        virtual void                               _GetCoverageIds(bvector<uint64_t>& ids) const = 0;

        virtual void                               _GetCoverageName(Utf8String& name, uint64_t id) const = 0;
        
        virtual BentleyStatus                      _DeleteCoverage(uint64_t id) = 0;

		virtual IScalableMeshClippingOptions&      _EditClippingOptions() = 0;

        virtual IScalableMeshPtr                   _GetTerrainSM() =0 ;

        virtual BentleyStatus                      _SetReprojection(GeoCoordinates::BaseGCSCR targetCS, TransformCR approximateTransform) =0;

#ifdef VANCOUVER_API
        virtual BentleyStatus                      _Reproject(GeoCoordinates::BaseGCSCP targetCS, DgnModelRefP dgnModel) = 0;
#else
        virtual BentleyStatus                      _Reproject(DgnGCSCP targetCS, DgnDbR dgnProject) = 0;
#endif

        virtual Transform              _GetReprojectionTransform() const = 0;


        virtual  IScalableMeshPtr             _GetGroup() = 0;

        virtual void                          _AddToGroup(IScalableMeshPtr& sMesh, bool isRegionRestricted = false, const DPoint3d* region = nullptr, size_t nOfPtsInRegion = 0) = 0;


        virtual void                          _RemoveFromGroup(IScalableMeshPtr& sMesh) = 0;

        virtual void                          _SetGroupSelectionFromPoint(DPoint3d firstPoint) = 0;
        virtual void                          _ClearGroupSelection() = 0;

        virtual void                          _RemoveAllDisplayData() = 0;
        
         
#ifdef WIP_MESH_IMPORT
        virtual void  _GetAllTextures(bvector<IScalableMeshTexturePtr>& textures) = 0;
#endif

    /*__PUBLISH_SECTION_START__*/
    public:
        //! Gets the number of points of the DTM.
        //! @return The number of points of the DTM..

        //! Gets the draping interface.
        //! @return The draping interface.

        BENTLEY_SM_EXPORT void TextureFromRaster(ITextureProviderPtr provider);

        BENTLEY_SM_EXPORT int64_t          GetPointCount();

        BENTLEY_SM_EXPORT uint64_t          GetNodeCount();

        BENTLEY_SM_EXPORT bool          IsTerrain();

        BENTLEY_SM_EXPORT bool          IsTextured();

        BENTLEY_SM_EXPORT StatusInt     GetTextureInfo(IScalableMeshTextureInfoPtr& textureInfo) const;        

        BENTLEY_SM_EXPORT bool          IsCesium3DTiles();

        BENTLEY_SM_EXPORT bool          IsStubFile();

        BENTLEY_SM_EXPORT Utf8String    GetProjectWiseContextShareLink();

        BENTLEY_SM_EXPORT DTMStatusInt     GetRange(DRange3dR range);

        BENTLEY_SM_EXPORT StatusInt          GetBoundary(bvector<DPoint3d>& boundary);

                          int                    GenerateSubResolutions();

        BENTLEY_SM_EXPORT int64_t                GetBreaklineCount() const;
            
                   ScalableMeshCompressionType   GetCompressionType() const;

        BENTLEY_SM_EXPORT int                    GetNbResolutions() const;    

        BENTLEY_SM_EXPORT size_t                 GetTerrainDepth() const;

        BENTLEY_SM_EXPORT IScalableMeshPointQueryPtr         GetQueryInterface(ScalableMeshQueryType queryType) const;

        IScalableMeshPointQueryPtr         GetQueryInterface(ScalableMeshQueryType                queryType,                                                              
                                                                               BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS,
                                                                               const DRange3d&                      extentInTargetGCS) const;

        BENTLEY_SM_EXPORT IScalableMeshMeshQueryPtr    GetMeshQueryInterface(MeshQueryType queryType) const;

        BENTLEY_SM_EXPORT IScalableMeshMeshQueryPtr     GetMeshQueryInterface(MeshQueryType queryType,
                                                                        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,
                                                                        const DRange3d&                      extentInTargetGCS) const;

        BENTLEY_SM_EXPORT IScalableMeshNodeRayQueryPtr    GetNodeQueryInterface() const;
                
        BENTLEY_SM_EXPORT IScalableMeshEditPtr        GetMeshEditInterface() const;

        BENTLEY_SM_EXPORT IScalableMeshAnalysisPtr    GetMeshAnalysisInterface();

        BENTLEY_SM_EXPORT BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*   GetDTMInterface(DTMAnalysisType type = DTMAnalysisType::Precise);

        BENTLEY_SM_EXPORT BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*   GetDTMInterface(DMatrix4d& storageToUors, DTMAnalysisType type = DTMAnalysisType::Precise);

        BENTLEY_SM_EXPORT const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr&
                                           GetBaseGCS() const;
        BENTLEY_SM_EXPORT StatusInt              SetBaseGCS(const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCS);

        BENTLEY_SM_EXPORT const GeoCoords::GCS&  GetGCS() const;
        BENTLEY_SM_EXPORT StatusInt              SetGCS(const GeoCoords::GCS& gcs);

        BENTLEY_SM_EXPORT void                   SetEditFilesBasePath(const Utf8String& path);

        BENTLEY_SM_EXPORT Utf8String             GetEditFilesBasePath();

        BENTLEY_SM_EXPORT void                   GetExtraFileNames(bvector<BeFileName>& extraFileNames) const;

        BENTLEY_SM_EXPORT ScalableMeshState      GetState() const;
        
        BENTLEY_SM_EXPORT bool                   IsReadOnly() const;

        BENTLEY_SM_EXPORT bool                   IsShareable() const;                        
         
        //Synchonization with data sources functions
        BENTLEY_SM_EXPORT bool                   InSynchWithSources() const; 

        // Deprecated. Remove.
        bool                                     InSynchWithDataSources() const { return InSynchWithSources(); }

        BENTLEY_SM_EXPORT bool                   LastSynchronizationCheck(time_t& last) const;                

        BENTLEY_SM_EXPORT IScalableMeshNodePtr  GetRootNode();

        int                    GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const;

        Count                  GetCountInRange (const DRange2d& range, const CountType& type, const uint64_t& maxNumberCountedPoints) const;

        BENTLEY_SM_EXPORT uint64_t               AddClip(const DPoint3d* pts, size_t ptsSize);

        BENTLEY_SM_EXPORT bool                   AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID);

        BENTLEY_SM_EXPORT bool                   AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive=true);

        BENTLEY_SM_EXPORT bool                   ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID);

        BENTLEY_SM_EXPORT bool                   ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive = true);

        BENTLEY_SM_EXPORT bool                   AddClip(const CLIP_VECTOR_NAMESPACE::ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive = true);

        BENTLEY_SM_EXPORT bool                   ModifyClip(const CLIP_VECTOR_NAMESPACE::ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive = true);

        BENTLEY_SM_EXPORT bool                   RemoveClip(uint64_t clipID);

        BENTLEY_SM_EXPORT bool                   GetClip(uint64_t clipID, bvector<DPoint3d>& clipData);

        BENTLEY_SM_EXPORT void                   SynchronizeClipData(const bvector<bpair<uint64_t, bvector<DPoint3d>>>& listOfClips, const bvector<bpair<uint64_t, bvector<bvector<DPoint3d>>>>& listOfSkirts);


        BENTLEY_SM_EXPORT bool                   GetSkirt(uint64_t skirtID, bvector<bvector<DPoint3d>>& skirtData);

        BENTLEY_SM_EXPORT bool                   ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID);

        BENTLEY_SM_EXPORT bool                   AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID);

        BENTLEY_SM_EXPORT bool                   RemoveSkirt(uint64_t skirtID);

        BENTLEY_SM_EXPORT bool                   IsInsertingClips();

        BENTLEY_SM_EXPORT void                   SetIsInsertingClips(bool toggleInsertMode);

        BENTLEY_SM_EXPORT bool                   ShouldInvertClips();

        BENTLEY_SM_EXPORT void                   SetInvertClip(bool invertClips);

        void                   ModifyClipMetadata(uint64_t clipId,double importance, int nDimensions);

        BENTLEY_SM_EXPORT void                   GetAllClipIds(bvector<uint64_t>& ids);

        BENTLEY_SM_EXPORT void                   SetClipOnOrOff (uint64_t id, bool isActive);
        BENTLEY_SM_EXPORT void                   GetIsClipActive(uint64_t id, bool& isActive);

        BENTLEY_SM_EXPORT void                   GetClipType(uint64_t id, SMNonDestructiveClipType& type);

		BENTLEY_SM_EXPORT void                   CompactExtraFiles();

		BENTLEY_SM_EXPORT void                   WriteExtraFiles();

        BENTLEY_SM_EXPORT void                   GetCurrentlyViewedNodes(bvector<IScalableMeshNodePtr>& nodes);

        BENTLEY_SM_EXPORT void                   SetCurrentlyViewedNodes(const bvector<IScalableMeshNodePtr>& nodes);

        BENTLEY_SM_EXPORT int                    Generate3DTiles(const WString& outContainerName, WString outDatasetName = L"", SMCloudServerType server = SMCloudServerType::LocalDisk, IScalableMeshProgressPtr progress = nullptr, CLIP_VECTOR_NAMESPACE::ClipVectorPtr clips = nullptr) const;

        BENTLEY_SM_EXPORT int                    SaveAs(const WString& destination, CLIP_VECTOR_NAMESPACE::ClipVectorPtr clips = nullptr, IScalableMeshProgressPtr progress = nullptr);

                          void                   ImportTerrainSM(WString terrainPath);

                          IScalableMeshPtr       GetTerrainSM();

        BENTLEY_SM_EXPORT SMStatus          DetectGroundForRegion(BeFileName& createdTerrain, const BeFileName& coverageTempDataFolder, const bvector<DPoint3d>& coverageData, uint64_t id, IScalableMeshGroundPreviewerPtr groundPreviewer, GeoCoordinates::BaseGCSCPtr destinationGcs = nullptr, bool limitResolutions = false);

        BENTLEY_SM_EXPORT BentleyStatus          CreateCoverage(const bvector<DPoint3d>& coverageData, uint64_t id, const Utf8String& coverageName);

        BENTLEY_SM_EXPORT void                   GetAllCoverages(bvector<bvector<DPoint3d>>& coverageData);

        BENTLEY_SM_EXPORT void                   GetCoverageIds(bvector<uint64_t>& ids) const;

        BENTLEY_SM_EXPORT void                   GetCoverageName(Utf8String& name, uint64_t id) const;

        BENTLEY_SM_EXPORT BentleyStatus          DeleteCoverage(uint64_t id);

		BENTLEY_SM_EXPORT IScalableMeshClippingOptions&      EditClippingOptions();

        BENTLEY_SM_EXPORT BentleyStatus          SetReprojection(GeoCoordinates::BaseGCSCR targetCS, TransformCR approximateTransform);

#ifdef VANCOUVER_API
        BENTLEY_SM_EXPORT BentleyStatus          Reproject(GeoCoordinates::BaseGCSCP targetCS, DgnModelRefP dgnModel);
#else
        BENTLEY_SM_EXPORT BentleyStatus          Reproject(DgnGCSCP targetCS, DgnDbR dgnProject);
#endif

        BENTLEY_SM_EXPORT Transform              GetReprojectionTransform() const;

        BENTLEY_SM_EXPORT static IScalableMeshPtr        GetFor                 (const WChar*          filePath,
                                                                                 bool                    openReadOnly,
                                                                                 bool                    openShareable,
                                                                                 StatusInt&              status);

        BENTLEY_SM_EXPORT static IScalableMeshPtr        GetFor(const WChar*          filePath,
                                                                const Utf8String&      baseEditsFilePath,
                                                                bool                   useTempFolderForEditFiles,
                                                                bool                   openReadOnly,
                                                                bool                   openShareable,
                                                                StatusInt&             status);

        BENTLEY_SM_EXPORT static IScalableMeshPtr        GetFor(const WChar*          filePath,
                                                                const Utf8String&     baseEditsFilePath,
                                                                bool                  useTempFolderForEditFiles,
                                                                bool                  needsNeighbors,
                                                                bool                  openReadOnly,
                                                                bool                  openShareable,
                                                                StatusInt&            status);

        BENTLEY_SM_EXPORT static IScalableMeshPtr        GetFor                 (const WChar*          filePath,
                                                                                 bool                    openReadOnly,
                                                                                 bool                    openShareable);

        BENTLEY_SM_EXPORT static IScalableMeshPtr        GetFor(const WChar*          filePath,
                                                                const Utf8String&     baseEditsFilePath,
                                                                bool                  openReadOnly,
                                                                bool                  openShareable);
        
        BENTLEY_SM_EXPORT static IScalableMeshPtr        GetFor(const WChar*          filePath,
                                                                bool                  needsNeighbors,
                                                                bool                    openReadOnly,
                                                                bool                    openShareable,
                                                                StatusInt&              status);
                                                                
        BENTLEY_SM_EXPORT  IScalableMeshPtr             GetGroup ();

        BENTLEY_SM_EXPORT void                          AddToGroup(IScalableMeshPtr& sMesh, bool isRegionRestricted = false, const DPoint3d* region = nullptr, size_t nOfPtsInRegion = 0);


        BENTLEY_SM_EXPORT void                          RemoveFromGroup(IScalableMeshPtr& sMesh);
        BENTLEY_SM_EXPORT void                          SetGroupSelectionFromPoint(DPoint3d firstPoint);
        BENTLEY_SM_EXPORT void                          ClearGroupSelection();

        BENTLEY_SM_EXPORT void                          RemoveAllDisplayData();

#ifdef SCALABLE_MESH_ATP
        BENTLEY_SM_EXPORT int                     ChangeGeometricError(const WString& outContainerName, WString outDatasetName, SMCloudServerType server, const double& newGeometricErrorValue) const;
        BENTLEY_SM_EXPORT int                     LoadAllNodeHeaders(size_t& nbLoadedNodes, int level) const;
        BENTLEY_SM_EXPORT int                     LoadAllNodeData(size_t& nbLoadedNodes, int level) const;
        BENTLEY_SM_EXPORT int                     SaveGroupedNodeHeaders(const WString& pi_pOutputDirPath, const short& pi_pGroupMode) const;
#endif

#ifdef WIP_MESH_IMPORT
        BENTLEY_SM_EXPORT void  GetAllTextures(bvector<IScalableMeshTexturePtr>& textures);
#endif
        
        void ReFilter();

    };


struct Count
    {
    uint64_t m_nbPoints;
    uint64_t m_nbLinears;
    Count(uint64_t nbPoints, uint64_t nbLinears) { m_nbPoints = nbPoints; m_nbLinears = nbLinears; }
    };


void BENTLEY_SM_EXPORT edgeCollapseTest(WCharCP param);
void BENTLEY_SM_EXPORT edgeCollapsePrintGraph(WCharCP param);
void BENTLEY_SM_EXPORT edgeCollapseShowMesh(WCharCP param, PolyfaceQueryP& outMesh);


END_BENTLEY_SCALABLEMESH_NAMESPACE
