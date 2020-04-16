/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshClipContainer.h>
#include "ScalableMeshDraping.h"

#include "ScalableMeshMemoryPools.h"

#include "Stores/SMSQLiteStore.h"

#include "ScalableMeshVolume.h"

#include "ScalableMesh.h"

#include "ScalableMeshClippingOptions.h"

#include <CloudDataSource/DataSourceManager.h>

using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

namespace GeoCoords
    {
    struct GCS;
    };

struct ScalableMeshGroup;

struct ScalableMeshGroupDTM : public RefCounted<BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM>, IDTMDraping, IDTMVolume
    {

    ScalableMeshGroup* m_group;
    Transform m_transformToUors;
    DTMAnalysisType m_type;

    protected:

        virtual IDTMDrapingP     _GetDTMDraping() override;
        virtual IDTMDrainageP    _GetDTMDrainage() override;
        virtual IDTMContouringP  _GetDTMContouring() override;
        virtual int64_t _GetPointCount() override;
        virtual DTMStatusInt _GetRange(DRange3dR range) override;
        virtual BcDTMP _GetBcDTM() override;
        virtual DTMStatusInt _GetBoundary(DTMPointArray& ret) override;
        virtual DTMStatusInt _CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints) override;
        virtual DTMStatusInt _CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints, DTMAreaValuesCallback progressiveCallback, DTMCancelProcessCallback isCancelledCallback) override;
        virtual DTMStatusInt _GetTransformDTM(DTMPtr& transformedDTM, TransformCR transformation) override;
        virtual bool _GetTransformation(TransformR transformation) override;
        virtual IDTMVolumeP _GetDTMVolume() override;

        virtual DTMStatusInt _ExportToGeopakTinFile(WCharCP fileNameP, TransformCP transformation) override;

        //draping calls
        virtual DTMStatusInt _DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int& drapedTypeP, DPoint3dCR point) override;

        virtual DTMStatusInt _DrapeLinear(DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints) override;

        virtual bool _DrapeAlongVector(DPoint3d* endPt, double *slope, double *aspect, DPoint3d triangle[3], int *drapedType, DPoint3dCR point, double directionOfVector, double slopeOfVector) override;

        virtual bool _ProjectPoint(DPoint3dR pointOnDTM, DMatrix4dCR w2vMap, DPoint3dCR testPoint) override;

        virtual bool _IntersectRay(DPoint3dR pointOnDTM, DVec3dCR direction, DPoint3dCR testPoint) override;
        virtual bool _IntersectRay(bvector<DTMRayIntersection>& pointOnDTM, DVec3dCR direction, DPoint3dCR testPoint) override;
        
        //volume calls

        virtual DTMStatusInt _ComputeCutFillVolume(double* cut, double* fill, double* volume, PolyfaceHeaderCP mesh) override;

        virtual DTMStatusInt _ComputeCutFillVolumeClosed(double* cut, double* fill, double* volume, PolyfaceHeaderCP mesh) override;

        virtual bool _RestrictVolumeToRegion(uint64_t regionId) override;

        virtual void _RemoveAllRestrictions() override;

    public:

        ScalableMeshGroupDTM(IScalableMesh* scMeshGroup);

        virtual ~ScalableMeshGroupDTM()
            {
            }

        static RefCountedPtr<ScalableMeshGroupDTM> Create(IScalableMeshPtr& scMesh)
            {
            return new ScalableMeshGroupDTM(scMesh.get());
            }


        static RefCountedPtr<ScalableMeshGroupDTM> Create(IScalableMesh* scMesh)
            {
            return new ScalableMeshGroupDTM(scMesh);
            }

        void SetAnalysisType(DTMAnalysisType type)
            {
            m_type = type;
            }

        void SetStorageToUors(DMatrix4d& storageToUors);
    };

struct ScalableMeshGroup : public RefCounted<IScalableMesh>
    {
    struct RegionInfo
        {
        bool hasRestrictedRegion;
        bvector<DPoint3d> region;
        };

    private:

        bvector<IScalableMesh*> m_members;
        bvector<RegionInfo> m_regions;

        IScalableMesh* m_selection;

        RefCountedPtr<ScalableMeshGroupDTM>  m_smGroupsDTM[DTMAnalysisType::Qty];

		ScalableMeshClippingOptions m_options;

    protected:

        virtual void                               _TextureFromRaster(ITextureProviderPtr provider) override {}

        virtual int64_t          _GetPointCount() override;

        virtual uint64_t          _GetNodeCount() override;

        virtual bool          _IsTerrain() override;

        virtual bool          _IsTextured() override;

        virtual StatusInt     _GetTextureInfo(IScalableMeshTextureInfoPtr& textureInfo) const override;
        
        virtual bool          _IsCesium3DTiles() override{ return false; }

        virtual bool           _IsStubFile() override { return false; }

        virtual Utf8String    _GetProjectWiseContextShareLink() override { return Utf8String(); }

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*  _GetDTMInterface(DTMAnalysisType type) override;

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*  _GetDTMInterface(DMatrix4d& storageToUors, DTMAnalysisType type) override;

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*  _GetDTMInterface(DMatrix4d& storageToUors, bvector<DPoint3d>& regionPts, DTMAnalysisType type) override;

        virtual DTMStatusInt     _GetRange(DRange3dR range) override;

        virtual StatusInt         _GetBoundary(bvector<DPoint3d>& boundary) override;


        // Inherited from IMRDTM                   
        virtual Count                  _GetCountInRange(const DRange2d& range, const CountType& type, const uint64_t& maxNumberCountedPoints) const;
        virtual int                    _GenerateSubResolutions() override { return ERROR; }
        virtual int64_t                _GetBreaklineCount() const override;
        virtual ScalableMeshCompressionType   _GetCompressionType() const override;
        virtual int                    _GetNbResolutions() const override;
        virtual size_t                    _GetTerrainDepth() const override;
        virtual IScalableMeshPointQueryPtr         _GetQueryInterface(ScalableMeshQueryType     queryType) const override { return nullptr; }
        virtual IScalableMeshPointQueryPtr         _GetQueryInterface(ScalableMeshQueryType                queryType,
                                                                      BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS,
                                                                      const DRange3d&                      extentInTargetGCS) const override
            {
            return nullptr;
            }
        virtual IScalableMeshMeshQueryPtr     _GetMeshQueryInterface(MeshQueryType queryType) const override { return nullptr; }
        virtual IScalableMeshMeshQueryPtr     _GetMeshQueryInterface(MeshQueryType queryType,
                                                                     BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS,
                                                                     const DRange3d&                      extentInTargetGCS) const override
            {
            return nullptr;
            }
        virtual IScalableMeshNodeRayQueryPtr     _GetNodeQueryInterface() const override { return nullptr; }

        virtual IScalableMeshEditPtr    _GetMeshEditInterface() const override { return nullptr; }
        virtual IScalableMeshAnalysisPtr    _GetMeshAnalysisInterface() override { return nullptr; }

        virtual const GeoCoords::GCS&  _GetGCS() const override;
        virtual StatusInt              _SetGCS(const GeoCoords::GCS& sourceGCS) override { return ERROR; }
        virtual ScalableMeshState             _GetState() const override;        
        virtual bool                   _IsReadOnly() const override;
        virtual bool                   _IsShareable() const override;
        virtual int                    _GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const;



        virtual void                               _RegenerateClips(bool forceRegenerate = false) override;
        virtual uint64_t                           _AddClip(const DPoint3d* pts, size_t ptsSize) override;
        virtual bool                               _ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID) override;
        virtual bool                               _GetClip(uint64_t clipID, bvector<DPoint3d>& clipData) override;
        virtual bool                               _GetClip(uint64_t clipID, ClipVectorPtr& clipData) override;
        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, bool alsoAddOnTerrain = true) override;
        virtual bool                               _RemoveClip(uint64_t clipID) override;
        virtual bool                               _IsInsertingClips() override;
        virtual void                               _SetIsInsertingClips(bool toggleInsertMode) override;

        virtual bool                               _ShouldInvertClips() override { return false; }

        virtual void                               _SetInvertClip(bool invertClips) override {}

        virtual void                               _ModifyClipMetadata(uint64_t clipId, double importance, int nDimensions) override;
        virtual void                               _GetAllClipsIds(bvector<uint64_t>& allClipIds) override;
        virtual void                               _SynchronizeClipData(const bvector<bpair<uint64_t, bvector<DPoint3d>>>& listOfClips, const bvector<bpair<uint64_t, bvector<bvector<DPoint3d>>>>& listOfSkirts) override;

		virtual void                               _CompactExtraFiles() override {}

		virtual void                               _WriteExtraFiles() override {}

        virtual bool                               _GetSkirt(uint64_t skirtID, bvector<bvector<DPoint3d>>& skirt) override;
        virtual bool                               _ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID) override;
        virtual bool                               _AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID, bool alsoAddOnTerrain = true) override;
        virtual bool                               _RemoveSkirt(uint64_t skirtID) override;
        virtual int                                _Generate3DTiles(const WString& outContainerName, const WString& outDatasetName, SMCloudServerType server, IScalableMeshProgressPtr progress, ClipVectorPtr clips, uint64_t coverageId) const override { return ERROR; }
        virtual int                                 _SaveAs(const WString& destination, ClipVectorPtr clips = nullptr, IScalableMeshProgressPtr progress = nullptr) override { return ERROR; }
        virtual void                               _ImportTerrainSM(WString terrainPath) override {  }
        virtual IScalableMeshPtr                    _GetTerrainSM() override { return nullptr; }

        virtual BentleyStatus                      _SetReprojection(GeoCoordinates::BaseGCSCR targetCS, TransformCR approximateTransform) override;
#ifdef VANCOUVER_API
        virtual BentleyStatus                      _Reproject(GeoCoordinates::BaseGCSCP targetCS, DgnModelRefP dgnModel) override;
#else
        virtual BentleyStatus                      _Reproject(DgnGCSCP targetCS, DgnDbR dgnProject) override;
#endif
        virtual Transform                          _GetReprojectionTransform() const override;

        virtual SMStatus                           _DetectGroundForRegion(BeFileName& createdTerrain, const BeFileName& coverageTempDataFolder, const bvector<DPoint3d>& coverageData, uint64_t id, IScalableMeshGroundPreviewerPtr groundPreviewer, BaseGCSCPtr& destinationGcs, bool limitResolution, bool reprojectElevation, const BeFileName& dataSourceDir) override;
        virtual BentleyStatus                      _CreateCoverage(const bvector<DPoint3d>& coverageData, uint64_t id, const Utf8String& coverageName) override;
        virtual void                               _GetAllCoverages(bvector<bvector<DPoint3d>>& coverageData) override;
        virtual void                               _GetCoverageIds(bvector<uint64_t>& ids) const override;
        virtual BentleyStatus                      _DeleteCoverage(uint64_t id) override;
        virtual void                               _GetCoverageName(Utf8String& name, uint64_t id) const override;

		virtual IScalableMeshClippingOptions&                               _EditClippingOptions() override { return m_options; };

        virtual void                               _SetClipOnOrOff(uint64_t id, bool isActive) override;
        virtual void                               _GetIsClipActive(uint64_t id, bool& isActive) override;

        virtual void                               _GetClipType(uint64_t id, SMNonDestructiveClipType& type) override;
        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) override;
        virtual bool                               _ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) override;
        virtual bool                               _AddClip(const ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) override;
        virtual bool                               _ModifyClip(const ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) override;

        virtual void                               _GetCurrentlyViewedNodes(bvector<IScalableMeshNodePtr>& nodes) override {}
        virtual void                               _SetCurrentlyViewedNodes(const bvector<IScalableMeshNodePtr>& nodes) override {}

#ifdef SCALABLE_MESH_ATP
        virtual int                    _ChangeGeometricError(const WString& outContainerName, const WString& outDatasetName = L"", SMCloudServerType server = SMCloudServerType::LocalDisk, const double& newGeometricErrorValue = 0.0) const { return ERROR; }
        virtual int                    _LoadAllNodeHeaders(size_t& nbLoadedNodes, int level) const override {return ERROR;}
        virtual int                    _LoadAllNodeData(size_t& nbLoadedNodes, int level) const override{return ERROR;}
        virtual int                    _SaveGroupedNodeHeaders(const WString& pi_pOutputDirPath, const short& pi_pGroupMode) const override {return ERROR;}
#endif
        
        virtual void                               _SetEditFilesBasePath(const Utf8String& path) override {}

        virtual Utf8String                         _GetEditFilesBasePath() override { return Utf8String(); }

        virtual void                               _GetExtraFileNames(bvector<BeFileName>& extraFileNames) const override {assert(!"Should not be called");}

        virtual IScalableMeshNodePtr               _GetRootNode() override
            {
            return nullptr;
            }

        virtual bool                                _LoadSources(IDTMSourceCollection& sources) const {return false; }


        //Data source synchronization functions.
        virtual bool                   _InSynchWithSources() const override { return false; }
        virtual bool                   _LastSynchronizationCheck(time_t& last) const override { return false; }        

        virtual  IScalableMeshPtr             _GetGroup() override { return nullptr; }

#ifdef WIP_MESH_IMPORT
        virtual void                          _GetAllTextures(bvector<IScalableMeshTexturePtr>& textures) override;
#endif

        virtual void                          _AddToGroup(IScalableMeshPtr& sMesh, bool isRegionRestricted = false, const DPoint3d* region = nullptr, size_t nOfPtsInRegion = 0) override
            {}


        virtual void                          _RemoveFromGroup(IScalableMeshPtr& sMesh) override {}

        virtual void                          _SetGroupSelectionFromPoint(DPoint3d firstPoint) override;
        virtual void                          _ClearGroupSelection() override;

        virtual void                          _RemoveAllDisplayData() override {}
        

     public:

         ScalableMeshGroup();

         const bvector<IScalableMesh*>&     GetMembers() { return m_members; }

         bool IsRegionRestricted(IScalableMesh* sMesh);

         bool IsWithinRegion(IScalableMesh* sMesh, const DPoint3d* pts, size_t nOfPts);

         void AddMember(IScalableMeshPtr& sMesh, bool isRegionRestricted = false, const DPoint3d* region = nullptr, size_t nOfPtsInRegion = 0);

         void RemoveMember(IScalableMeshPtr& sMesh);

         void SelectMember(IScalableMesh* sMesh);

         void ClearSelection();

         IScalableMesh* GetSelection();

         static IScalableMeshPtr Create()
             {
             return new ScalableMeshGroup();
             }
    };

    END_BENTLEY_SCALABLEMESH_NAMESPACE