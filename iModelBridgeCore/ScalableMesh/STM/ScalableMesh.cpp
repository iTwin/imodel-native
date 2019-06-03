/*--------------------------------------------------------------------------------------+
|    $RCSfile: ScalableMesh.cpp,v $
|   $Revision: 1.106 $
|       $Date: 2012/01/06 16:30:15 $
|     $Author: Raymond.Gauthier $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
  
#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h" 
extern bool   GET_HIGHEST_RES;

#include <STMInternal/Foundations/FoundationsPrivateTools.h>
/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include "InternalUtilityFunctions.h"
#include "ScalableMesh.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include "ScalableMeshQuadTreeQueries.h"

#include <ScalableMesh/GeoCoords/GCS.h>
#include <STMInternal/GeoCoords/WKTUtils.h>
#include <ScalableMesh/GeoCoords/Reprojection.h>

#include "ScalableMeshQuery.h"
#include "ScalableMeshSourcesPersistance.h"

#include <ScalableMesh/IScalableMeshPolicy.h>
#include <ScalableMesh/IScalableMeshSourceCollection.h>
#include <ScalableMesh/IScalableMeshDocumentEnv.h>
#include <ScalableMesh/IScalableMeshGroundExtractor.h>
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>
#include <ScalableMesh/IScalableMeshSources.h>

#include "ScalableMesh/ScalableMeshLib.h"
#include <CloudDataSource/DataSourceManager.h>

#include "ScalableMeshDraping.h"
#include "ScalableMeshInfo.h"
#include "ScalableMeshVolume.h"

#include "Edits/ClipRegistry.h"

#include <Vu/VuApi.h>
#include <Vu/vupoly.fdf>
#include "vuPolygonClassifier.h"
#include <ImagePP/all/h/HIMMosaic.h>
#include <ImagePP/all/h/HRFVirtualEarthFile.h>
#include "LogUtils.h"
#include "ScalableMeshEdit.h"
#include "ScalableMeshAnalysis.h"
#include <ScalableMesh/ScalableMeshLib.h>
#include <ScalableMesh/IScalableMeshNodeCreator.h>
#include "MosaicTextureProvider.h"
#include "ScalableMeshGroup.h"
#include "ScalableMeshMesher.h"
#include "ScalableMeshDb.h"
#include <ScalableMesh/IScalableMeshPublisher.h>

#ifndef VANCOUVER_API
#include <DgnPlatform/DgnGeoCoord.h>
#endif



//#include "CGALEdgeCollapse.h"

//DataSourceManager s_dataSourceManager;

extern bool s_stream_from_wsg;
extern bool s_stream_from_grouped_store;
extern bool s_is_virtual_grouping;

ScalableMeshScheduler* s_clipScheduler = nullptr;
std::mutex s_schedulerLock;

/*----------------------------------------------+
| Constant definitions                          |
+----------------------------------------------*/

/*----------------------------------------------+
| Private type definitions                      |
+----------------------------------------------*/

// typedef HGF2DTemplateExtent<double, HGF3DCoord<double>> FProtFeatureExtentType;
typedef HGF3DExtent<double> FProtFeatureExtentType;
//typedef HGF2DTemplateExtent<double, HGF3DCoord<double>> FProtPtExtentType;
typedef HGF3DExtent<double> FProtPtExtentType;
 
// typedef HGF2DTemplateExtent<double, HGF3DCoord<double>> XProtFeatureExtentType;
typedef HGF3DExtent<double> XProtFeatureExtentType;
//typedef HGF2DTemplateExtent<double, HGF3DCoord<double>> XProtPtExtentType;
typedef HGF3DExtent<double> XProtPtExtentType;


typedef HGF3DExtent<double> YProtFeatureExtentType;
//typedef HGF2DTemplateExtent<double, HGF3DCoord<double>> YProtFeatureExtentType;

/*==================================================================*/
/*                                                                  */
/*          INTERNAL FUNCTIONS                                      */
/*                                                                  */
/*==================================================================*/

using namespace ISMStore;

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

bool s_useSQLFormat = true;

#define USE_CODE_FOR_ERROR_DETECTION

namespace {
#if 0
/*----------------------------------------------------------------------------+
| Pool Singleton
+----------------------------------------------------------------------------*/
template <typename POINT> static HFCPtr<HPMCountLimitedPool<POINT> > PoolSingleton()
    {
    static HFCPtr<HPMCountLimitedPool<POINT> > pGlobalPointPool(new HPMCountLimitedPool<POINT>(new HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment(100, 2000 * sizeof(POINT)), 30000000));

    return pGlobalPointPool;
    }
#endif

inline const GCS& GetDefaultGCS ()
    {
    static const GCS DEFAULT_GCS(GetGCSFactory().Create(GeoCoords::Unit::GetMeter()));
    return DEFAULT_GCS;
    }

const size_t DEFAULT_WORKING_LAYER = 0;

}   

/*----------------------------------------------------------------------------+
|IScalableMesh Method Definition Section - Begin
+----------------------------------------------------------------------------*/

void IScalableMesh::TextureFromRaster(ITextureProviderPtr provider)
    {
    return _TextureFromRaster(provider);
    }

int64_t IScalableMesh::GetPointCount()
    {
    return _GetPointCount();
    }

uint64_t IScalableMesh::GetNodeCount()
    {
    return _GetNodeCount();
    }

bool IScalableMesh::IsTerrain()
    {
    return _IsTerrain();
    }

bool IScalableMesh::IsTextured()
    {
    return _IsTextured();
    }

StatusInt IScalableMesh::GetTextureInfo(IScalableMeshTextureInfoPtr& textureInfo) const
    {
    return _GetTextureInfo(textureInfo);
    }

bool IScalableMesh::IsCesium3DTiles()
    {
    return _IsCesium3DTiles();
    }

bool IScalableMesh::IsStubFile()
    {
    return _IsStubFile();
    }

Utf8String IScalableMesh::GetProjectWiseContextShareLink()
    {
    return _GetProjectWiseContextShareLink();
    }

DTMStatusInt IScalableMesh::GetRange(DRange3dR range)
    {
    return _GetRange(range);
    }

StatusInt IScalableMesh::GetBoundary(bvector<DPoint3d>& boundary)
    {
    return _GetBoundary(boundary);
    }

int IScalableMesh::GenerateSubResolutions()
    {
    return _GenerateSubResolutions();
    }
            
int64_t IScalableMesh::GetBreaklineCount() const
    {
    return _GetBreaklineCount();
    }
 
ScalableMeshCompressionType IScalableMesh::GetCompressionType() const
    {
    return _GetCompressionType();
    }

int IScalableMesh::GetNbResolutions() const
    {
    return _GetNbResolutions();
    }

size_t IScalableMesh::GetTerrainDepth() const
    {
    return _GetTerrainDepth();
    }


IScalableMeshPointQueryPtr IScalableMesh::GetQueryInterface(ScalableMeshQueryType queryType) const
    {
    return _GetQueryInterface(queryType);
    }

IScalableMeshPointQueryPtr IScalableMesh::GetQueryInterface(ScalableMeshQueryType                         queryType,                                          
                                                            BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS,
                                                            const DRange3d& extentInTargetGCS) const
    {
    return _GetQueryInterface(queryType, targetGCS, extentInTargetGCS);
    }

IScalableMeshMeshQueryPtr IScalableMesh::GetMeshQueryInterface(MeshQueryType queryType) const
    {
    return _GetMeshQueryInterface(queryType);
    }

IScalableMeshMeshQueryPtr IScalableMesh::GetMeshQueryInterface(MeshQueryType queryType,
                                                 BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS,
                                                 const DRange3d& extentInTargetGCS) const
    {
    return _GetMeshQueryInterface(queryType, targetGCS, extentInTargetGCS);
    }

IScalableMeshNodeRayQueryPtr IScalableMesh::GetNodeQueryInterface() const
    {
    return _GetNodeQueryInterface();
    } 

IScalableMeshEditPtr IScalableMesh::GetMeshEditInterface() const
    {
    return _GetMeshEditInterface();
    }

IScalableMeshAnalysisPtr IScalableMesh::GetMeshAnalysisInterface()
    {
    return _GetMeshAnalysisInterface();
    }

BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* IScalableMesh::GetDTMInterface(DTMAnalysisType type)
    {
    return _GetDTMInterface(type);
    }

BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* IScalableMesh::GetDTMInterface(DMatrix4d& storageToUors, DTMAnalysisType type)
    {
    return _GetDTMInterface(storageToUors, type);
    }

BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* IScalableMesh::GetDTMInterface(DMatrix4d& storageToUors, bvector<DPoint3d>& regionPts, DTMAnalysisType type)
{
    return _GetDTMInterface(storageToUors, regionPts, type);
}

const BaseGCSCPtr& IScalableMesh::GetBaseGCS() const 
    {
    return _GetGCS().GetGeoRef().GetBasePtr();
    }

StatusInt IScalableMesh::SetBaseGCS(const BaseGCSCPtr& baseGCSPtr)
    {
    SMStatus createStatus = SMStatus::S_SUCCESS;
    GCS gcs(GetGCSFactory().Create(baseGCSPtr, createStatus));
    if (SMStatus::S_SUCCESS != createStatus)
        return BSIERROR;

    return _SetGCS(gcs);
    }


const GeoCoords::GCS& IScalableMesh::GetGCS() const
    {
    return _GetGCS();
    }

StatusInt IScalableMesh::SetGCS(const GeoCoords::GCS& gcs)
    {
    return _SetGCS(gcs);
    }

ScalableMeshState IScalableMesh::GetState() const
    {
    return _GetState();
    }

bool IScalableMesh::IsReadOnly() const
    {
    return _IsReadOnly();
    }

bool IScalableMesh::IsShareable() const
    {
    return _IsShareable();
    }

bool IScalableMesh::LoadSources(IDTMSourceCollection& sources) const
    {
    return _LoadSources(sources);
    }

bool IScalableMesh::InSynchWithSources() const
    {
    return _InSynchWithSources();
    }

bool IScalableMesh::LastSynchronizationCheck(time_t& lastCheckTime) const
    {
    return _LastSynchronizationCheck(lastCheckTime);
    }

int IScalableMesh::GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const
    {
    return _GetRangeInSpecificGCS(lowPt, highPt, targetGCS);
    }


uint64_t IScalableMesh::AddClip(const DPoint3d* pts, size_t ptsSize)
    {
    return _AddClip(pts, ptsSize);
    }

bool IScalableMesh::AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID)
    {
    return _AddClip(pts, ptsSize, clipID);
    }

bool IScalableMesh::AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
    {
    return _AddClip(pts, ptsSize, clipID, geom, type, isActive);
    }

bool IScalableMesh::AddClip(const ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
{
    return _AddClip(clip, clipID, geom, type, isActive);
}

bool IScalableMesh::ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID)
    {
    return _ModifyClip(pts, ptsSize, clipID);
    }


bool IScalableMesh::ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
    {
    return _ModifyClip(pts, ptsSize, clipID, geom, type, isActive);
    }

bool IScalableMesh::ModifyClip(const ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
{
    return _ModifyClip(clip, clipID, geom, type, isActive);
}

void IScalableMesh::SynchronizeClipData(const bvector<bpair<uint64_t, bvector<DPoint3d>>>& listOfClips, const bvector<bpair<uint64_t, bvector<bvector<DPoint3d>>>>& listOfSkirts)
    {
    return _SynchronizeClipData(listOfClips, listOfSkirts);
    }

void IScalableMesh::ModifyClipMetadata(uint64_t clipId, double importance, int nDimensions)
    {
    return _ModifyClipMetadata(clipId, importance, nDimensions);
    }

bool IScalableMesh::RemoveClip(uint64_t clipID)
    {
    return _RemoveClip(clipID);
    }

bool IScalableMesh::GetClip(uint64_t clipID, bvector<DPoint3d>& clipData)
{
    return _GetClip(clipID, clipData);
}

bool IScalableMesh::GetClip(uint64_t clipID, ClipVectorPtr& clipData)
{
    return _GetClip(clipID, clipData);
}

bool IScalableMesh::IsInsertingClips()
    {
    return _IsInsertingClips();
    }

void IScalableMesh::SetIsInsertingClips(bool toggleInsertClips)
    {
    return _SetIsInsertingClips(toggleInsertClips);
    }


bool   IScalableMesh::ShouldInvertClips()
    {
    return _ShouldInvertClips();
    }

void   IScalableMesh::SetInvertClip(bool invertClips)
    {
    return _SetInvertClip(invertClips);
    }

bool IScalableMesh::GetSkirt(uint64_t skirtID, bvector<bvector<DPoint3d>>& skirt)
    {
    return _GetSkirt(skirtID, skirt);
    }

bool IScalableMesh::AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t clipID)
    {
    return _AddSkirt(skirt, clipID);
    }

bool IScalableMesh::ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t clipID)
    {
    return _ModifySkirt(skirt, clipID);
    }

void IScalableMesh::GetAllClipIds(bvector<uint64_t>& ids)
    {
    return _GetAllClipsIds(ids);
    }

void                              IScalableMesh::SetClipOnOrOff(uint64_t id, bool isActive)
    {
    return _SetClipOnOrOff(id, isActive);
    }

void                               IScalableMesh::GetIsClipActive(uint64_t id, bool& isActive)
    {
    return _GetIsClipActive(id, isActive);
    }

void                        IScalableMesh::GetClipType(uint64_t id, SMNonDestructiveClipType& type)
    {
    return _GetClipType(id, type);
    }

void IScalableMesh::CompactExtraFiles()
{
	return _CompactExtraFiles();
}

void IScalableMesh::WriteExtraFiles()
{
	return _WriteExtraFiles();
}

void IScalableMesh::GetCurrentlyViewedNodes(bvector<IScalableMeshNodePtr>& nodes)
    {
    return _GetCurrentlyViewedNodes(nodes);
    }

void IScalableMesh::SetCurrentlyViewedNodes(const bvector<IScalableMeshNodePtr>& nodes)
    {
    return _SetCurrentlyViewedNodes(nodes);
    }

void IScalableMesh::SetEditFilesBasePath(const Utf8String& path)
    {
    return _SetEditFilesBasePath(path);
    }

Utf8String IScalableMesh::GetEditFilesBasePath()
    {
    return _GetEditFilesBasePath();
    }

void IScalableMesh::GetExtraFileNames(bvector<BeFileName>& extraFileNames) const
    {
    return _GetExtraFileNames(extraFileNames);
    }

IScalableMeshNodePtr IScalableMesh::GetRootNode()
    {
    return _GetRootNode();
    }

#ifdef WIP_MESH_IMPORT
void  IScalableMesh::GetAllTextures(bvector<IScalableMeshTexturePtr>& textures)
    {
    return _GetAllTextures(textures);
    }
#endif

bool IScalableMesh::RemoveSkirt(uint64_t clipID)
    {
    return _RemoveSkirt(clipID);
    }


int IScalableMesh::SaveAs(const WString& destination, ClipVectorPtr clips, IScalableMeshProgressPtr progress)
    {
    return _SaveAs(destination, clips, progress);
    }

int IScalableMesh::Generate3DTiles(const WString& outContainerName, WString outDatasetName, SMCloudServerType server, IScalableMeshProgressPtr progress, ClipVectorPtr clips) const
    {
    return _Generate3DTiles(outContainerName, outDatasetName, server, progress, clips, (clips.IsValid() ? (uint64_t)-1 : (uint64_t)-2));
    }

BentleyStatus IScalableMesh::CreateCoverage(const bvector<DPoint3d>& coverageData, uint64_t id, const Utf8String& coverageName)
    {    
    return _CreateCoverage(coverageData, id, coverageName);
    }

SMStatus IScalableMesh::DetectGroundForRegion(BeFileName& createdTerrain, const BeFileName& coverageTempDataFolder, const bvector<DPoint3d>& coverageData, uint64_t id, IScalableMeshGroundPreviewerPtr groundPreviewer, BaseGCSCPtr destinationGcs, bool limitResolution, bool reprojectElevation, const BeFileName& dataSourceDir)
    {
    return _DetectGroundForRegion(createdTerrain, coverageTempDataFolder, coverageData, id, groundPreviewer, destinationGcs, limitResolution, reprojectElevation, dataSourceDir);
    }

void IScalableMesh::GetAllCoverages(bvector<bvector<DPoint3d>>& coverageData)
    {
    return _GetAllCoverages(coverageData);
    }

void IScalableMesh::GetCoverageIds(bvector<uint64_t>& ids) const
    {
    return _GetCoverageIds(ids);
    }

void IScalableMesh::GetCoverageName(Utf8String& name, uint64_t id) const
    {
    return _GetCoverageName(name, id);
    }

BentleyStatus IScalableMesh::DeleteCoverage(uint64_t id)
    {
    return _DeleteCoverage(id);
    }

IScalableMeshClippingOptions&  IScalableMesh::EditClippingOptions()
    {
	return _EditClippingOptions();
    }

void IScalableMesh::ImportTerrainSM(WString terrainPath)
    {
    return _ImportTerrainSM(terrainPath);
    }

IScalableMeshPtr IScalableMesh::GetTerrainSM()
    {
    return _GetTerrainSM();
    }

Transform  IScalableMesh::GetReprojectionTransform() const
    {
    return _GetReprojectionTransform();
    }

BentleyStatus   IScalableMesh::SetReprojection(GeoCoordinates::BaseGCSCR targetCS, TransformCR approximateTransform)
    {
    return _SetReprojection(targetCS, approximateTransform);
    }

#ifdef VANCOUVER_API
BentleyStatus   IScalableMesh::Reproject(GeoCoordinates::BaseGCSCP targetCS, DgnModelRefP dgnModel)
    {
    return _Reproject(targetCS, dgnModel);
    }
#else
BentleyStatus   IScalableMesh::Reproject(DgnGCSCP targetCS, DgnDbR dgnProject)
    {
    return _Reproject(targetCS, dgnProject);
    }
#endif

IScalableMeshPtr IScalableMesh::GetGroup()
    {
    return _GetGroup();
    }

void IScalableMesh::AddToGroup(IScalableMeshPtr& sMesh, bool isRegionRestricted, const DPoint3d* region, size_t nOfPtsInRegion)
    {
    return _AddToGroup(sMesh, isRegionRestricted, region, nOfPtsInRegion);
    }


void IScalableMesh::RemoveFromGroup(IScalableMeshPtr& sMesh)
    {
    return _RemoveFromGroup(sMesh);
    }

void IScalableMesh::SetGroupSelectionFromPoint(DPoint3d firstPoint)
    {
    return _SetGroupSelectionFromPoint(firstPoint);
    }

void  IScalableMesh::ClearGroupSelection()
    {
    return _ClearGroupSelection();
    }

void  IScalableMesh::RemoveAllDisplayData()
    {
    return _RemoveAllDisplayData();
    }


#ifdef SCALABLE_MESH_ATP
    
int IScalableMesh::ChangeGeometricError(const WString& outContainerName, WString outDatasetName, SMCloudServerType server, const double& newGeometricErrorValue) const
    {
    return _ChangeGeometricError(outContainerName, outDatasetName, server, newGeometricErrorValue);
    }

int IScalableMesh::LoadAllNodeHeaders(size_t& nbLoadedNodes, int level) const
    {
    return _LoadAllNodeHeaders(nbLoadedNodes, level);
    }

int IScalableMesh::LoadAllNodeData(size_t& nbLoadedNodes, int level) const
{
    return _LoadAllNodeData(nbLoadedNodes, level);
}

int IScalableMesh::SaveGroupedNodeHeaders(const WString& pi_pOutputDirPath, const short& pi_pGroupMode) const
    {
    return _SaveGroupedNodeHeaders(pi_pOutputDirPath, pi_pGroupMode);
    }
#endif

void IScalableMesh::ReFilter()
    {
    return _ReFilter();
    }

/*----------------------------------------------------------------------------+
|IScalableMesh Method Definition Section - End
+----------------------------------------------------------------------------*/


namespace {

AccessMode GetAccessModeFor(bool                    openReadOnly,
                            bool                    openShareable)
    {
    AccessMode accessMode;

    if (openReadOnly == true)
        {
        accessMode = HFC_READ_ONLY;

        if (openShareable == true)
            {
            accessMode |= HFC_SHARE_READ_WRITE;
            }           
        }
    else
        {            
        accessMode = HFC_READ_WRITE;

        if (openShareable == true)
            {                    
            accessMode |= HFC_SHARE_READ_ONLY;
            }            
        } 
    return accessMode;
    }

}

/*----------------------------------------------------------------------------+
|IScalableMesh::GetFor
+----------------------------------------------------------------------------*/

IScalableMeshPtr IScalableMesh::GetFor(const WChar*          filePath,
                                       const Utf8String&     baseEditsFilePath,
                                       bool                  useTempFolderForEditFiles,
                                       bool                  openReadOnly,
                                       bool                  openShareable,
                                       StatusInt&            status)
{
    return GetFor(filePath, baseEditsFilePath, useTempFolderForEditFiles, true, openReadOnly, openShareable, status);

}

IScalableMeshPtr IScalableMesh::GetFor(const WChar*          filePath,
                                       const Utf8String&     baseEditsFilePath,
                                       bool                  useTempFolderForEditFiles,
                                       bool                  needsNeighbors,
                                       bool                  openReadOnly,
                                       bool                  openShareable,
                                       StatusInt&            status)
{
    status = BSISUCCESS;
    bool isLocal = true;
    Utf8String newBaseEditsFilePath(baseEditsFilePath);
    SMSQLiteFilePtr smSQLiteFile;
    if ((BeFileName::GetExtension(filePath).CompareToI(L"3sm") == 0) ||
        (BeFileName::GetExtension(filePath).CompareToI(L"stm2") == 0))
        { // Open 3sm file
        StatusInt openStatus;
        
        if (openShareable && ScalableMeshDb::GetEnableSharedDatabase())
            {
            smSQLiteFile = SMSQLiteFile::Open(filePath, openReadOnly, openStatus, true);
            }
        else
            {
            smSQLiteFile = SMSQLiteFile::Open(filePath, openReadOnly, openStatus);
            }
			
        if (smSQLiteFile == nullptr)
            {
            status = BSIERROR;

            return 0; // Error opening file
            }
        }
    else if (BeFileName::GetExtension(filePath).CompareToI(L"s3sm") == 0)
        {
        auto directory = BeFileName::GetDirectoryName(filePath);
        isLocal = BeFileName::DoesPathExist(directory.c_str());
        }
    else if (BeFileName::GetExtension(filePath).CompareToI(L"json") == 0)    
        { // Open json streaming format
        auto directory = BeFileName::GetDirectoryName(filePath);
        isLocal = BeFileName::DoesPathExist(directory.c_str());
        if (!isLocal && newBaseEditsFilePath.empty())
            {
            const wchar_t* temp = L"C:\\Temp\\Bentley\\3SM";
            if (!BeFileName::DoesPathExist(temp)) BeFileName::CreateNewDirectory(temp);
            newBaseEditsFilePath.Assign(temp);
            }
        }
    else if (WString(filePath).ContainsI(L"realitydataservices") && WString(filePath).ContainsI(L"S3MXECPlugin"))
        { // Open from ProjectWise Context share
        isLocal = false;
        if (newBaseEditsFilePath.empty())
            {
            const wchar_t* temp = L"C:\\Temp\\Bentley\\3SM";
            if (!BeFileName::DoesPathExist(temp)) BeFileName::CreateNewDirectory(temp);
            newBaseEditsFilePath.Assign(temp);
            }
        }
    else
        {
        status = BSIERROR;
        return 0;
        }

    if (ScalableMeshLib::GetHost().GetRegisteredScalableMesh(filePath) != nullptr) return ScalableMeshLib::GetHost().GetRegisteredScalableMesh(filePath);
#ifndef LINUX_SCALABLEMESH_BUILD
    if(isLocal && 0 != _waccess(filePath, 04))
    {
        status = BSISUCCESS;
        return 0; // File not found
    }
#endif

    return ScalableMesh<DPoint3d>::Open(smSQLiteFile, filePath, newBaseEditsFilePath, useTempFolderForEditFiles, needsNeighbors, status);

}

IScalableMeshPtr IScalableMesh::GetFor(const WChar*          filePath,
                                       bool                    openReadOnly,
                                       bool                    openShareable,
                                       StatusInt&              status)
    {
    return GetFor(filePath, Utf8String(filePath), true, openReadOnly, openShareable, status);
    }

IScalableMeshPtr IScalableMesh::GetFor(const WChar*          filePath,
                                       bool                  needsNeighbors,
                                       bool                    openReadOnly,
                                       bool                    openShareable,
                                       StatusInt&              status)
    {
    return GetFor(filePath, Utf8String(filePath), true, needsNeighbors, openReadOnly, openShareable, status);
    }

/*----------------------------------------------------------------------------+
|IScalableMesh::GetFor
+----------------------------------------------------------------------------*/
IScalableMeshPtr IScalableMesh::GetFor   (const WChar*          filePath,
                            bool                    openReadOnly,
                            bool                    openShareable)
    {
    StatusInt status;
    return GetFor(filePath, Utf8String(filePath), true, openReadOnly, openShareable, status);
    }

/*----------------------------------------------------------------------------+
|IScalableMesh::GetFor
+----------------------------------------------------------------------------*/
IScalableMeshPtr IScalableMesh::GetFor(const WChar*          filePath,
                                       const Utf8String&     baseEditsFilePath,
                                       bool                  openReadOnly,
                                       bool                  openShareable)
    {
    StatusInt status;
    return IScalableMesh::GetFor(filePath, baseEditsFilePath, true, openReadOnly, openShareable, status);
    }

/*----------------------------------------------------------------------------+
|IScalableMesh::GetCountInRange
+----------------------------------------------------------------------------*/
Count IScalableMesh::GetCountInRange (const DRange2d& range, const CountType& type, const uint64_t& maxNumberCountedPoints) const
    {
    return _GetCountInRange(range, type, maxNumberCountedPoints);
    }


void      IScalableMesh::RegenerateClips(bool forceRegenerate)
{
    return _RegenerateClips(forceRegenerate);
}

/*----------------------------------------------------------------------------+
|ScalableMeshBase::ScalableMeshBase
+----------------------------------------------------------------------------*/

ScalableMeshBase::ScalableMeshBase(SMSQLiteFilePtr& smSQliteFile,
    const WString&             filePath)
    : m_workingLayer(DEFAULT_WORKING_LAYER),
    m_sourceGCS(GetDefaultGCS()),
    m_path(filePath),
    m_baseExtraFilesPath(filePath),    
    m_useTempPath(true), //Default is true, only legacy code should want not to use temporary folder.
    m_smSQLitePtr(smSQliteFile)
{
    memset(&m_contentExtent, 0, sizeof(m_contentExtent));

    // NEEDS_WORK_SM_STREAMING : if sqlite file is null, check for streaming parameters
    //HPRECONDITION(smSQliteFile != 0);
    
        m_useTempPath = true;

    SetDataSourceAccount(nullptr);
}


/*----------------------------------------------------------------------------+
|ScalableMeshBase::~ScalableMeshBase
+----------------------------------------------------------------------------*/
ScalableMeshBase::~ScalableMeshBase ()
    {
    }

/*----------------------------------------------------------------------------+
|ScalableMeshBase::GetIDTMFile
+----------------------------------------------------------------------------*/
const SMSQLiteFilePtr& ScalableMeshBase::GetDbFile() const
{
    return m_smSQLitePtr;
}


/*----------------------------------------------------------------------------+
|ScalableMeshBase::GetPath
+----------------------------------------------------------------------------*/
const WChar* ScalableMeshBase::GetPath () const
    {
    return m_path.c_str();
    }

/*----------------------------------------------------------------------------+
|ScalableMeshBase::LoadGCSFrom
+----------------------------------------------------------------------------*/

bool ScalableMeshBase::LoadGCSFrom(WString wktStr)
    {
    assert(!wktStr.empty()); // invalid parameter

    auto wktTemp = wktStr;
    ISMStore::WktFlavor fileWktFlavor = GetWKTFlavor(&wktTemp, wktStr);
    BaseGCS::WktFlavor  wktFlavor = BaseGCS::WktFlavor::wktFlavorUnknown;

    bool result = MapWktFlavorEnum(wktFlavor, fileWktFlavor);

    SMStatus gcsCreateStatus;
    if (result)
        {
        GCS gcs(GetGCSFactory().Create(wktTemp.c_str(), wktFlavor, gcsCreateStatus));

        if (SMStatus::S_SUCCESS != gcsCreateStatus)
            return false;

        using std::swap;
        swap(m_sourceGCS, gcs);
        return true;
        }

    // NEEDS_WORK_SM_STREAMING : assume default gcs for streaming only?
    auto baseGCS = BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS(wktStr.c_str());

    GCS gcs(GetGCSFactory().Create(baseGCS.get(), gcsCreateStatus));

    using std::swap;
    swap(m_sourceGCS, gcs);
    return true;
    }

bool ScalableMeshBase::LoadGCSFrom()
{
    assert(m_smSQLitePtr != nullptr); // Must only be called with valid sqlite file

    WString wktStr;
    if (m_smSQLitePtr == nullptr || m_smSQLitePtr->GetWkt(wktStr))
        return true;
    return LoadGCSFrom(wktStr);
}

template <class POINT> void ScalableMesh<POINT>::_RegenerateClips(bool forceRegenerate)
{
    if (nullptr == m_scmIndexPtr) return;
    auto store = m_scmIndexPtr->GetDataStore();

    if (store->DoesClipFileExist() && forceRegenerate)
    {
        SMMemoryPool::GetInstance()->RemoveAllItemsOfType(SMStoreDataType::DiffSet, (uint64_t)m_scmIndexPtr.GetPtr());
        store->EraseClipFile();
    }

    SetIsInsertingClips(true);

    bvector<uint64_t> existingClipIds;
    GetAllClipIds(existingClipIds);

    for (auto& id : existingClipIds)
    {
        bvector<DPoint3d> clipData;
        m_scmIndexPtr->GetClipRegistry()->GetClip(id, clipData);
        DRange3d extent = DRange3d::NullRange();
        if (!clipData.empty())
            extent.Extend(DRange3d::From(&clipData[0], (int)clipData.size()));
        else
        {
            SMClipGeometryType geom;
            SMNonDestructiveClipType type;
            bool isActive;
            m_scmIndexPtr->GetClipRegistry()->GetClipWithParameters(id, clipData, geom, type, isActive);
            if (geom == SMClipGeometryType::BoundedVolume)
            {
                ClipVectorPtr cp;
                m_scmIndexPtr->GetClipRegistry()->GetClipWithParameters(id, cp, geom, type, isActive);
                if (cp.IsValid())
                {
                    for (ClipPrimitivePtr& primitive : *cp)
                        primitive->SetIsMask(false);
                    cp->GetRange(extent, nullptr);
                }
                if (extent.Volume() == 0)
                {
                    if (extent.XLength() == 0)
                    {
                        extent.low.x -= 1.e-5;
                        extent.high.x += 1.e-5;
                    }
                    if (extent.YLength() == 0)
                    {
                        extent.low.y -= 1.e-5;
                        extent.high.y += 1.e-5;
                    }
                    if (extent.ZLength() == 0)
                    {
                        extent.low.z -= 1.e-5;
                        extent.high.z += 1.e-5;
                    }
                }
            }

        }

        Transform t = Transform::FromIdentity();
        if (IsCesium3DTiles()) t = GetReprojectionTransform();

        m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_ADD, id, extent, true, t);

    }

    SetIsInsertingClips(false);
    SaveEditFiles();
}

/*----------------------------------------------------------------------------+
|ScalableMesh::ScalableMesh
+----------------------------------------------------------------------------*/

template <class POINT> ScalableMesh<POINT>::ScalableMesh(SMSQLiteFilePtr& smSQLiteFile, const WString& path)
    : ScalableMeshBase(smSQLiteFile, path),
    m_areDataCompressed(false),
    m_computeTileBoundary(false),
    m_isCesium3DTiles(false),
    m_isFromStubFile(false),
    m_minScreenPixelsPerPoint(MEAN_SCREEN_PIXELS_PER_POINT),
    m_reprojectionTransform(Transform::FromIdentity()),
    m_isInvertingClips(false)
    {

	m_clippingOptions = new ScalableMeshClippingOptions([this](const ScalableMeshClippingOptions* changed)
	{
		if (nullptr == m_scmIndexPtr) return;
		auto store = m_scmIndexPtr->GetDataStore();
		store->SetClipDefinitionsProvider(changed->GetClipDefinitionsProvider());
		m_scmIndexPtr->m_canUseBcLibClips = !changed->HasOverlappingClips();

		if (changed->ShouldRegenerateStaleClipFiles() && !store->DoesClipFileExist())
		{
			/*SetIsInsertingClips(true);

			bvector<uint64_t> existingClipIds;
			GetAllClipIds(existingClipIds);

			for (auto& id : existingClipIds)
			{
				bvector<DPoint3d> clipData;
				m_scmIndexPtr->GetClipRegistry()->GetClip(id, clipData);
				DRange3d extent = DRange3d::NullRange();
				if (!clipData.empty())
					extent.Extend(DRange3d::From(&clipData[0], (int)clipData.size()));
                else
                {
                    SMClipGeometryType geom;
                    SMNonDestructiveClipType type;
                    bool isActive;
                    m_scmIndexPtr->GetClipRegistry()->GetClipWithParameters(id, clipData, geom, type, isActive);
                    if (geom == SMClipGeometryType::BoundedVolume)
                    {
                        ClipVectorPtr cp;
                        m_scmIndexPtr->GetClipRegistry()->GetClipWithParameters(id, cp, geom, type, isActive);
                        if (cp.IsValid())
                        {
                            for (ClipPrimitivePtr& primitive : *cp)
                                primitive->SetIsMask(false);
                            cp->GetRange(extent, nullptr);
                        }
                        if (extent.Volume() == 0)
                        {
                            if (extent.XLength() == 0)
                            {
                                extent.low.x -= 1.e-5;
                                extent.high.x += 1.e-5;
                            }
                            if (extent.YLength() == 0)
                            {
                                extent.low.y -= 1.e-5;
                                extent.high.y += 1.e-5;
                            }
                            if (extent.ZLength() == 0)
                            {
                                extent.low.z -= 1.e-5;
                                extent.high.z += 1.e-5;
                            }
                        }
                    }
                    
                }

				Transform t = Transform::FromIdentity();
				if (IsCesium3DTiles()) t = GetReprojectionTransform();

				m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_ADD, id, extent, true, t);

			}

			SetIsInsertingClips(false);
			SaveEditFiles();*/
            _RegenerateClips();
		}
	});
    }


/*----------------------------------------------------------------------------+
|ScalableMesh::~ScalableMesh
+----------------------------------------------------------------------------*/
template <class POINT> ScalableMesh<POINT>::~ScalableMesh()
    {    
    Close();
    }

template <class POINT> uint64_t ScalableMesh<POINT>::CountPointsInExtent(Extent3dType& extent, 
                                                                          HFCPtr<SMPointIndexNode<POINT, Extent3dType>> nodePtr,
                                                                          const uint64_t& maxNumberCountedPoints) const
    {
    const uint64_t maxCount = maxNumberCountedPoints <= 0 ? std::numeric_limits<uint64_t>::max() : maxNumberCountedPoints;

    if(ExtentOp<Extent3dType>::Overlap(nodePtr->GetContentExtent(),extent))
        {
        if(nodePtr->IsLeaf())
            {
            if (ExtentOp<Extent3dType>::Contains2d(nodePtr->GetContentExtent(), extent))
                {
                // Approximate number of points. The points are assumed to be evenly distributed in the tile extent.
                // This could potentially lead to poor performance if the points are concentrated in specific areas of
                // the tile extent.
                double nodeExtentDistanceX = ExtentOp<Extent3dType>::GetWidth(nodePtr->GetContentExtent());
                double nodeExtentDistanceY = ExtentOp<Extent3dType>::GetHeight(nodePtr->GetContentExtent());
                double nodeExtentArea = nodeExtentDistanceX * nodeExtentDistanceY;
                double extentDistanceX = ExtentOp<Extent3dType>::GetWidth(extent);
                double extentDistanceY = ExtentOp<Extent3dType>::GetHeight(extent);
                double extentArea = extentDistanceX * extentDistanceY;
                return uint64_t(nodePtr->GetCount()*(extentArea/nodeExtentArea));
                }
            else {
                double nodeExtentDistanceX = ExtentOp<Extent3dType>::GetWidth(nodePtr->GetContentExtent());
                double nodeExtentDistanceY = ExtentOp<Extent3dType>::GetHeight(nodePtr->GetContentExtent());
                double nodeExtentArea = nodeExtentDistanceX * nodeExtentDistanceY;
                // Create intersection extent
                double xAxisValues[] = { ExtentOp<Extent3dType>::GetXMin(nodePtr->GetContentExtent()), ExtentOp<Extent3dType>::GetXMax(nodePtr->GetContentExtent()),
                    ExtentOp<Extent3dType>::GetXMin(extent), ExtentOp<Extent3dType>::GetXMax(extent) };
                vector<double> xAxisValuesV (xAxisValues,xAxisValues+sizeof(xAxisValues)/ sizeof(double)) ;
                sort(xAxisValuesV.begin(),xAxisValuesV.end());
                double yAxisValues[] = { ExtentOp<Extent3dType>::GetYMin(nodePtr->GetContentExtent()), ExtentOp<Extent3dType>::GetYMax(nodePtr->GetContentExtent()), 
                    ExtentOp<Extent3dType>::GetYMin(extent), ExtentOp<Extent3dType>::GetYMax(extent) };
                vector<double> yAxisValuesV (yAxisValues,yAxisValues+sizeof(yAxisValues)/ sizeof(double)) ;
                sort(yAxisValuesV.begin(),yAxisValuesV.end());
                Extent3dType intersectionExtent = ExtentOp<Extent3dType>::Create(xAxisValuesV[1],yAxisValuesV[1],xAxisValuesV[2],yAxisValuesV[2]);
                double intersectionArea = ExtentOp<Extent3dType>::GetWidth(intersectionExtent)*ExtentOp<Extent3dType>::GetHeight(intersectionExtent);
                //return nodePtr->GetCount();
                return uint64_t( (intersectionArea != 0 && nodeExtentArea != 0) ? nodePtr->GetCount()*(intersectionArea/nodeExtentArea) : 0);
                }
            }
        else if(nodePtr->GetSubNodeNoSplit() != NULL)
            {
            return CountPointsInExtent(extent,nodePtr->GetSubNodeNoSplit(),maxCount);
            }
        else
            {
            uint64_t count = 0;
            for(size_t nodeIndex = 0; nodeIndex < nodePtr->GetNumberOfSubNodesOnSplit() && count < maxCount; nodeIndex++)
                {
                count += CountPointsInExtent(extent,nodePtr->m_apSubNodes[nodeIndex],maxCount);
                }
            return count;
            }
        }
    else return 0;
    }

template <class POINT> uint64_t ScalableMesh<POINT>::CountLinearsInExtent(YProtFeatureExtentType& extent, uint64_t maxFeatures) const
{
    assert(false && "Not supported!");
    return -1;
};

template <class POINT> Count ScalableMesh<POINT>::_GetCountInRange (const DRange2d& range, const CountType& type, const uint64_t& maxNumberCountedPoints) const
    {
    Count qty (0,0);
    switch (type)
        {
        case COUNTTYPE_POINTS:
        case COUNTTYPE_BOTH:
            {
            HFCPtr<SMPointIndexNode<POINT, Extent3dType>> nodePtr = m_scmIndexPtr->GetRootNode();
            Extent3dType pointExtent;
            ExtentOp<Extent3dType>::SetXMax(pointExtent, range.high.x);
            ExtentOp<Extent3dType>::SetXMin(pointExtent, range.low.x);
            ExtentOp<Extent3dType>::SetYMax(pointExtent, range.high.y);
            ExtentOp<Extent3dType>::SetYMin(pointExtent, range.low.y);
            ExtentOp<Extent3dType>::SetZMax(pointExtent, 0.0);
            ExtentOp<Extent3dType>::SetZMin(pointExtent, 0.0);
            qty.m_nbPoints = CountPointsInExtent (pointExtent, nodePtr, maxNumberCountedPoints);
            }
        case COUNTTYPE_LINEARS:
            {
            YProtFeatureExtentType queryExtent;
            queryExtent.Set(range.low.x, range.low.y, 0.0, range.high.x, range.high.y, 0.0);
            qty.m_nbLinears = CountLinearsInExtent (queryExtent, maxNumberCountedPoints);
            break;
            }
        default:
            {
            assert(false);
            }
        }
    return qty;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::Open
+----------------------------------------------------------------------------*/

template <class POINT>
IScalableMeshPtr ScalableMesh<POINT>::Open(SMSQLiteFilePtr&  smSQLiteFile,
                                           const WString&    filePath,
                                           const Utf8String& baseEditsFilePath,                                           
                                           bool              useTempFolderForEditFiles,
                                           bool              needsNeighbors,
                                           StatusInt&        status)
{
    ScalableMesh<POINT>* scmPtr = new ScalableMesh<POINT>(smSQLiteFile, filePath);
    IScalableMeshPtr scmP(scmPtr);
    scmP->SetEditFilesBasePath(baseEditsFilePath);    
    scmPtr->SetUseTempPath(useTempFolderForEditFiles);
    scmPtr->SetNeedsNeighbors(needsNeighbors);
    status = scmPtr->Open();
    if (status == BSISUCCESS)
        {
        bool isFromPWCS = false;
        WString newFilePath = filePath;
        if (scmPtr->IsCesium3DTiles() && !scmPtr->IsStubFile())
            {
            auto pwcsLink = scmPtr->GetProjectWiseContextShareLink();
            if (isFromPWCS = !pwcsLink.empty())
                newFilePath = WString(pwcsLink.c_str(), true);
            }
#ifndef VANCOUVER_API
        ScalableMeshLib::GetHost().RegisterScalableMesh(newFilePath, scmP);
#endif
        }
    return (BSISUCCESS == status ? scmP : 0);
}

/*----------------------------------------------------------------------------+
|ScalableMesh::Open
+----------------------------------------------------------------------------*/
static bool s_dropNodes = false;
static bool s_checkHybridNodeState = false;
template <class POINT> int ScalableMesh<POINT>::Open()
    {

#if TRACE_ON
    CachedDataEventTracer::GetInstance()->setLogDirectory("e:\\Elenie\\traceLogs\\");
    CachedDataEventTracer::GetInstance()->start();
#endif

    try 
        {
        bool isSingleFile = m_smSQLitePtr != nullptr ? m_smSQLitePtr->IsSingleFile() : false;

        // If there are no masterHeader => file empty ?
        if (isSingleFile && m_smSQLitePtr != nullptr && !m_smSQLitePtr->HasMasterHeader())
            return BSISUCCESS; // File is empty.

        {    
        ISMDataStoreTypePtr<Extent3dType> dataStore;
        if (!isSingleFile)
            {
            m_streamingSettings = new SMStreamingStore<Extent3dType>::SMStreamingSettings(m_path);

            if (!m_streamingSettings->IsValid())
                return ERROR;
            if (m_streamingSettings->IsDataFromRDS())
                m_smRDSProvider = IScalableMeshRDSProvider::Create("https://" + m_streamingSettings->GetUtf8ServerID() + ".bentley.com/", m_streamingSettings->GetUtf8ProjectID(), m_streamingSettings->GetUtf8GUID());
            m_isCesium3DTiles = m_streamingSettings->IsCesium3DTiles();
            m_isFromStubFile = m_streamingSettings->IsStubFile();

#ifndef VANCOUVER_API                                       
            dataStore = new SMStreamingStore<Extent3dType>(m_streamingSettings, m_smRDSProvider);
#else
            dataStore = SMStreamingStore<Extent3dType>::Create(m_streamingSettings, m_smRDSProvider);
#endif

            m_scmIndexPtr = new MeshIndexType(dataStore,
                ScalableMeshMemoryPools<POINT>::Get()->GetGenericPool(),
                10000,
                nullptr/*filterP*/,
                this->m_needsNeighbors,
                false,
                false,
                false,
                0,
                0);

            {
                if ((m_streamingSettings->GetGCSString().empty() && !LoadGCSFrom(WString(L"ll84"))) ||
                    (!m_streamingSettings->GetGCSString().empty() && !LoadGCSFrom(m_streamingSettings->GetGCSString())))
                return BSIERROR; // Error loading layer gcs
            }
            }
        else
            {

            if (!LoadGCSFrom())
                return BSIERROR; // Error loading layer gcs

#ifndef VANCOUVER_API                       
            dataStore = new SMSQLiteStore<Extent3dType>(m_smSQLitePtr);
#else
            dataStore = SMSQLiteStore<Extent3dType>::Create(m_smSQLitePtr);
#endif

            m_scmIndexPtr = new MeshIndexType(dataStore,
                ScalableMeshMemoryPools<POINT>::Get()->GetGenericPool(),
                10000,
                nullptr/*filterP*/,
                this->m_needsNeighbors,
                false,
                false,
                false,
                0,
                0);
            }


        BeFileName projectFilesPath(m_baseExtraFilesPath.c_str());

        bool result = dataStore->SetProjectFilesPath(projectFilesPath);
        assert(result == true);

        result = dataStore->SetUseTempPath(m_useTempPath);
        assert(result == true);

        ClipRegistry* registry = new ClipRegistry(dataStore);
        m_scmIndexPtr->SetClipRegistry(registry);


        //filterP.release();

#ifdef INDEX_DUMPING_ACTIVATED
        if (s_dropNodes)
            {
            //  m_scmIndexPtr->DumpOctTree("D:\\MyDoc\\Scalable Mesh Iteration 8\\PartialUpdate\\Neighbor\\Log\\nodeAfterOpen.xml", false); 
            m_scmIndexPtr->DumpOctTree((char *)"D:\\MyDoc\\RM - SM - Sprint 13\\New Store\\Dump\\nodeDump.xml", false);
            //m_scmIndexPtr->DumpOctTree("C:\\Users\\Richard.Bois\\Documents\\ScalableMeshWorkDir\\QuebecCityMini\\nodeAfterOpen.xml", false);      
       //     m_scmMPointIndexPtr->ValidateNeighbors();
            }
#endif
        }

#if 0

        if (s_checkHybridNodeState)
        {
            if (!m_smSQLitePtr->HasSources())
                return BSISUCCESS; // No sources were added to the STM

            IDTMSourceCollection sources;
            this->LoadSources(sources);

            auto sourceIter(sources.Begin());
            auto sourceIterEnd(sources.End());

            vector<DRange3d> source2_5dRanges;
            vector<DRange3d> source3dRanges;       

            // Remove and Add sources                                            
            while (sourceIter != sourceIterEnd)
                {
                SourceImportConfig sourceConfig(sourceIter->GetConfig());

                Import::ScalableMeshData scalableMesh(sourceIter->GetConfig().GetReplacementSMData());                                
                if (scalableMesh.IsRepresenting3dData() == SMis3D::is3D)
                    {               
                    source3dRanges.insert(source3dRanges.begin(), scalableMesh.GetExtent().begin(), scalableMesh.GetExtent().end());
                    }        
                else
                    {
                    source2_5dRanges.insert(source2_5dRanges.begin(), scalableMesh.GetExtent().begin(), scalableMesh.GetExtent().end());                
                    }

                m_scmIndexPtr->ValidateIs3dDataStates(source2_5dRanges, source3dRanges);    

                sourceIter++;
                }                       
            }            
#endif
                       

        m_contentExtent = ComputeTotalExtentFor(&*m_scmIndexPtr);
        //if (m_contentExtent.isNull() || m_contentExtent.isEmpty() || m_contentExtent.DiagonalDistance() == 0) return BSIERROR;
        for (int i = 0; i < (int)DTMAnalysisType::Qty; ++i)
            {
            m_scalableMeshDTM[i] = ScalableMeshDTM::Create(this);
            m_scalableMeshDTM[i]->SetAnalysisType((DTMAnalysisType)i);

            //TFS# 775936 - Ensure that the reprojection matrix is applied to the scalableMeshDTM object when reopening the 3SM internally 
            //              (e.g. : when adding texture to existing terrain).
            auto mat4d = DMatrix4d::From(m_reprojectionTransform);
            m_scalableMeshDTM[i]->SetStorageToUors(mat4d);
            }

        return BSISUCCESS;  
        }
    catch(...)
        {
        return BSIERROR;             
        }
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::Close
+----------------------------------------------------------------------------*/    
template <class POINT> int ScalableMesh<POINT>::Close
(
)
    {

#ifdef TRACE_ON
    CachedDataEventTracer::GetInstance()->analyze(-1);
#endif
    WString path = m_path;
    if (this->IsCesium3DTiles() && !this->IsStubFile())
        {
        auto pwcsLink = this->GetProjectWiseContextShareLink();
        if (!pwcsLink.empty())
            path = WString(pwcsLink.c_str(), true);
        }
#ifndef VANCOUVER_API
    BeAssert(ScalableMeshLib::IsInitialized()); // Must initialize lib first!
    ScalableMeshLib::GetHost().RemoveRegisteredScalableMesh(path);
#endif
    m_viewedNodes.clear();
    ClearProgressiveQueriesInfo();
    if (m_scalableMeshDTM[DTMAnalysisType::Fast] != nullptr)
        ((ScalableMeshDraping*)m_scalableMeshDTM[DTMAnalysisType::Fast]->GetDTMDraping())->ClearNodes();
    if (m_scalableMeshDTM[DTMAnalysisType::Precise] != nullptr)
        ((ScalableMeshDraping*)m_scalableMeshDTM[DTMAnalysisType::Precise]->GetDTMDraping())->ClearNodes();

    SMMemoryPool::CleanVideoMemoryPool();

    _SetIsInsertingClips(false);
    SaveEditFiles();

    m_scmIndexPtr = 0;

    if(m_smSQLitePtr != nullptr)
        m_smSQLitePtr->Close();
    m_smSQLitePtr = nullptr;

    
    return SUCCESS;        
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::ComputeTileBoundaryDuringQuery
+----------------------------------------------------------------------------*/
template <class POINT> void ScalableMesh<POINT>::ComputeTileBoundaryDuringQuery(bool doCompute)
    {
    m_computeTileBoundary = doCompute;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::SetMinScreenPixelsPerPoint
+----------------------------------------------------------------------------*/
template <class POINT> void ScalableMesh<POINT>::SetMinScreenPixelsPerPoint(double pi_minScreenPixelsPerPoint)
    {    
    m_minScreenPixelsPerPoint = pi_minScreenPixelsPerPoint;   
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::GetMinScreenPixelsPerPoint
+----------------------------------------------------------------------------*/
template <class POINT> double ScalableMesh<POINT>::GetMinScreenPixelsPerPoint()
    {
    return m_minScreenPixelsPerPoint;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::AreDataCompressed
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::AreDataCompressed()
    {
    return m_areDataCompressed;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::CreateSpatialIndexFromExtents
+----------------------------------------------------------------------------*/
template <class POINT> void ScalableMesh<POINT>::CreateSpatialIndexFromExtents(list<HVE2DSegment>& pi_rpBreaklineList, 
                                                                 BC_DTM_OBJ**        po_ppBcDtmObj)
    {   
    int status;

    if (pi_rpBreaklineList.size() > 0)
        {        
        if (*po_ppBcDtmObj != 0)
            {
            status = bcdtmObject_destroyDtmObject(po_ppBcDtmObj);
            assert(status == 0);            
            }

        status = bcdtmObject_createDtmObject(po_ppBcDtmObj);
        assert(status == 0);                   

        list<HVE2DSegment>::iterator BreaklineIter    = pi_rpBreaklineList.begin();
        list<HVE2DSegment>::iterator BreaklineIterEnd = pi_rpBreaklineList.end();

        DPoint3d     TileHullPts[2];        

        uint32_t TileNumber = 0;
          
        while (BreaklineIter != BreaklineIterEnd) 
            {                            
            TileHullPts[0].x = BreaklineIter->GetExtent().GetXMin(); 
            TileHullPts[0].y = BreaklineIter->GetExtent().GetYMin(); 
            TileHullPts[0].z = 0;

            TileHullPts[1].x = BreaklineIter->GetExtent().GetXMax();                             
            TileHullPts[1].y = BreaklineIter->GetExtent().GetYMax(); 
            TileHullPts[1].z = 0;

            status = bcdtmObject_storeDtmFeatureInDtmObject(*po_ppBcDtmObj, DTMFeatureType::Breakline, TileNumber, 1, &(*po_ppBcDtmObj)->nullFeatureId, TileHullPts, 2);

            assert(status == 0);      
            BreaklineIter++;
            TileNumber++;
            }         
        }
    }


ScalableMeshDTM::ScalableMeshDTM(IScalableMeshPtr& scMesh)
    {
    m_draping = new ScalableMeshDraping(scMesh);
    m_dtmVolume = new ScalableMeshVolume(scMesh);
    m_scMesh = scMesh.get();   
    m_tryCreateDtm = false;    
    }

void ScalableMeshDTM::SetStorageToUors(DMatrix4d& storageToUors)
    {
    m_transformToUors.InitFrom(storageToUors);
    m_draping->SetTransform(m_transformToUors);
    ((ScalableMeshVolume*)m_dtmVolume)->SetTransform(m_transformToUors);
    }


/*-------------------Methods inherited from IDTM-----------------------------*/
int64_t ScalableMeshDTM::_GetPointCount()
    {
    return m_scMesh->GetPointCount();
    }

BcDTMP ScalableMeshDTM::_GetBcDTM()
    {    
    if (!m_tryCreateDtm)
        {
        m_tryCreateDtm = true;

        //find the highest resolution that has less than 5M points
        IScalableMeshMeshQueryParamsPtr params = IScalableMeshMeshQueryParams::CreateParams();
        IScalableMeshMeshQueryPtr meshQueryInterface = m_scMesh->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION);
        bvector<IScalableMeshNodePtr> returnedNodes;
        params->SetLevel(m_scMesh->GetTerrainDepth());

        size_t totalPts = 0;
        if (meshQueryInterface->Query(returnedNodes, !m_bounds.empty() ? m_bounds.data() : 0, !m_bounds.empty() ? (int)m_bounds.size() : 0, params) != SUCCESS)
            return nullptr;
        for (auto& node : returnedNodes)
        {
            totalPts += node->GetPointCount();
        }
        while (totalPts > 5000000 && params->GetLevel() > 1)
        {
            returnedNodes.clear();
            params->SetLevel(params->GetLevel() - 1);
            meshQueryInterface->Query(returnedNodes, !m_bounds.empty()? m_bounds.data() : 0, !m_bounds.empty() ? (int)m_bounds.size() : 0, params);
            totalPts = 0;
            for (auto& node : returnedNodes)
            {
                totalPts += node->GetPointCount();
            }
        }
        if (returnedNodes.size() == 0) return nullptr;

        IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
        auto meshPtr = returnedNodes.front()->GetMesh(flags);
        ScalableMeshMesh* meshP = dynamic_cast<ScalableMeshMesh*>(meshPtr.get());
        //add all triangles from returned nodes to DTM
        for (auto nodeIter = returnedNodes.begin() + 1; nodeIter != returnedNodes.end(); ++nodeIter)
        {
            if ((*nodeIter)->GetPointCount() <= 4) continue;
            auto currentMeshPtr = (*nodeIter)->GetMesh(flags);
            if (!currentMeshPtr.IsValid()) continue;
            bvector<int32_t> indices(currentMeshPtr->GetPolyfaceQuery()->GetPointIndexCount());
            memcpy(&indices[0], currentMeshPtr->GetPolyfaceQuery()->GetPointIndexCP(), indices.size() * sizeof(int32_t));
            for (auto&idx : indices) idx += (int)meshP->GetNbPoints();
            meshP->AppendMesh(currentMeshPtr->GetPolyfaceQuery()->GetPointCount(), const_cast<DPoint3d*>(currentMeshPtr->GetPolyfaceQuery()->GetPointCP()), (int)indices.size(), &indices[0], 0, 0, 0, 0, 0, 0);
        }

        bool hasFeatureSource = false;

        IDTMSourceCollection sources;
        m_scMesh->LoadSources(sources);
        for (IDTMSourceCollection::const_iterator it = sources.Begin(); it != sources.End(); it++)
        {
            BeFileName path = BeFileName(it->GetPath().c_str());
            if (0 == BeFileName::GetExtension(path).CompareToI(L"dtm") ||
                0 == BeFileName::GetExtension(path).CompareToI(L"tin") ||
                0 == BeFileName::GetExtension(path).CompareToI(L"dgn") ||
                0 == BeFileName::GetExtension(path).CompareToI(L"bcdtm") ||
                0 == BeFileName::GetExtension(path).CompareToI(L"dat"))
            {
                hasFeatureSource = true;
            }
            
        }

        DTMStatusInt val = DTM_SUCCESS;
        if (hasFeatureSource)
        {
            meshP->RemoveDuplicates();
            val = meshP->GetAsBcDTM(m_dtm);
        }
        else
        {
            val = meshP->GetAsBcDTM(m_dtm, true);
        }
        if (val == DTM_ERROR) 
            {
            m_dtm = nullptr;
            return nullptr;
            }
        
        if (!m_transformToUors.IsIdentity())
            {
            val = m_dtm->Transform(m_transformToUors);

            if (val == DTM_ERROR)
                {
                m_dtm = nullptr;
                return nullptr;
                }            
            }        
        }

    return m_dtm.get();    
    }

DTMStatusInt ScalableMeshDTM::_GetBoundary(DTMPointArray& result)
    {
    DTMPointArray boundary;
    if (m_scMesh->GetBoundary(boundary) != SUCCESS)
        return DTM_ERROR;

    if (m_scMesh->IsCesium3DTiles())
        {
        result = boundary;
        }
    else
        {
        m_transformToUors.Multiply(result, boundary);
        }

    return DTM_SUCCESS;
    }

RefCountedPtr<ScalableMeshDTM> ScalableMeshDTM::ExtractRegion(bvector<DPoint3d>& region)
{
    RefCountedPtr<ScalableMeshDTM> myPtr = ScalableMeshDTM::Create(m_scMesh);
    myPtr->SetAnalysisType(m_draping->GetAnalysisType());
    auto mat4d = DMatrix4d::From(m_transformToUors);
    myPtr->SetStorageToUors(mat4d);
    myPtr->m_bounds = region;
    return myPtr;
}

IDTMDrapingP ScalableMeshDTM::_GetDTMDraping()
    {
    return m_draping;
    }

IDTMDrainageP    ScalableMeshDTM::_GetDTMDrainage()
    {
    assert(0);
    return 0;
    }

IDTMContouringP  ScalableMeshDTM::_GetDTMContouring()
    {
    assert(0);
    return 0;
    }

DTMStatusInt ScalableMeshDTM::_CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints)
    {
    IScalableMeshMeshQueryParamsPtr params = IScalableMeshMeshQueryParams::CreateParams();
    IScalableMeshMeshQueryPtr meshQueryInterface = m_scMesh->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION);
    bvector<IScalableMeshNodePtr> returnedNodes;
    params->SetLevel(m_scMesh->GetTerrainDepth());

#ifndef VANCOUVER_API
    Transform uorsToStorage = m_transformToUors.ValidatedInverse();
#else
    Transform uorsToStorage;
    uorsToStorage.InverseOf(m_transformToUors);
#endif
    bvector<DPoint3d> transPts(numPoints);
    memcpy(&transPts[0], pts, numPoints*sizeof(DPoint3d));
    uorsToStorage.Multiply(&transPts[0], numPoints);
    if (meshQueryInterface->Query(returnedNodes, &transPts[0], numPoints, params) != SUCCESS)
        return DTM_ERROR;
    bvector<bool> clips;
    flatArea = 0;
    slopeArea = 0;
    for (auto& node : returnedNodes)
        {
        double flatAreaTile = 0;
        double slopeAreaTile = 0;
        BcDTMPtr dtmP = node->GetBcDTM();
        if (dtmP != nullptr) 
//#ifdef VANCOUVER_API
            dtmP->CalculateSlopeArea(flatAreaTile, slopeAreaTile, &transPts[0], numPoints);
//#else
//            dtmP->CalculateSlopeArea(&flatAreaTile, &slopeAreaTile, &transPts[0], numPoints);
//#endif
        flatArea += flatAreaTile;
        slopeArea += slopeAreaTile;
        }
    DPoint3d pt = DPoint3d::From(flatArea, slopeArea, 0);
    m_transformToUors.Multiply(pt);
    flatArea = pt.x;
    slopeArea = pt.y;
    return DTM_SUCCESS;
    }

DTMStatusInt ScalableMeshDTM::_CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints, DTMAreaValuesCallback progressiveCallback, DTMCancelProcessCallback isCancelledCallback)
    {
    IScalableMeshMeshQueryParamsPtr params = IScalableMeshMeshQueryParams::CreateParams();
    IScalableMeshMeshQueryPtr meshQueryInterface = m_scMesh->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION);
    bvector<IScalableMeshNodePtr> returnedNodes;
    params->SetLevel(m_scMesh->GetTerrainDepth());

#ifndef VANCOUVER_API
    Transform uorsToStorage = m_transformToUors.ValidatedInverse();
#else
    Transform uorsToStorage;
    uorsToStorage.InverseOf(m_transformToUors);
#endif
    bvector<DPoint3d> transPts(numPoints);
    memcpy(&transPts[0], pts, numPoints*sizeof(DPoint3d));
    uorsToStorage.Multiply(&transPts[0], numPoints);
    for (auto& pt : transPts) pt.z = 0.0;
    if (meshQueryInterface->Query(returnedNodes, &transPts[0], numPoints, params) != SUCCESS)
        return DTM_ERROR;
    bvector<IScalableMeshNodePtr>* fullResolutionReturnedNodes = new bvector<IScalableMeshNodePtr> (returnedNodes);
    while (returnedNodes.size() > 20 && params->GetLevel() > 1)
        {
        returnedNodes.clear();
        params->SetLevel(params->GetLevel()-1);
        meshQueryInterface->Query(returnedNodes, &transPts[0], numPoints, params);
        }
    bvector<bool> clips;
    flatArea = 0;
    slopeArea = 0;
    for (auto& node : returnedNodes)
        {
        double flatAreaTile = 0;
        double slopeAreaTile = 0;
        DRange3d nodeExt = node->GetContentExtent();
        DPoint3d rangePts[5] = { DPoint3d::From(nodeExt.low.x, nodeExt.low.y, 0), DPoint3d::From(nodeExt.high.x, nodeExt.low.y, 0), DPoint3d::From(nodeExt.high.x, nodeExt.high.y, 0),
            DPoint3d::From(nodeExt.low.x, nodeExt.high.y, 0), DPoint3d::From(nodeExt.low.x, nodeExt.low.y, 0) };
       // DTM_POLYGON_OBJ* polyP = NULL;
       // long intersectFlag;
        bool restrictToPoly = true;
        /*if (DTM_SUCCESS != bcdtmPolygon_intersectPointArrayPolygons(&rangePts[0], 5, &transPts[0], numPoints, &intersectFlag, &polyP, 1e-8, 1e-8) || intersectFlag == 0) restrictToPoly = false;
        if (polyP != NULL) free(polyP);*/
        const CurveVectorPtr dtmBoundary = CurveVector::CreateLinear(rangePts, 5,CurveVector::BOUNDARY_TYPE_Outer, false);
        const CurveVectorPtr activeBoundary = CurveVector::CreateLinear(transPts, CurveVector::BOUNDARY_TYPE_Outer, false);
        const CurveVectorPtr intersection = CurveVector::AreaIntersection(*activeBoundary, *dtmBoundary);
        if (!intersection.IsValid() || intersection.IsNull()) restrictToPoly = false;
        BcDTMPtr dtmP = node->GetBcDTM();
        if (restrictToPoly)
            {
            double flat, slope;
            bvector<bvector<bvector<DPoint3d>>> allGeom;
            intersection->CollectLinearGeometry(allGeom);
            for (auto& geom : allGeom)
                for (auto& lineString : geom)
                    {
                    if (dtmP != nullptr)
                        {
//#ifdef VANCOUVER_API
                        if (DTM_SUCCESS == dtmP->CalculateSlopeArea(flat, slope, &lineString[0], (int)lineString.size()))
//#else
//                        if (DTM_SUCCESS == dtmP->CalculateSlopeArea(&flat, &slope, &lineString[0], (int)lineString.size()))
//#endif
                            {
                            flatAreaTile += flat;
                            slopeAreaTile += slope;
                            }
                        }
                    }
            }
        else
//#ifdef VANCOUVER_API
          if (dtmP != nullptr) dtmP->CalculateSlopeArea(flatAreaTile, slopeAreaTile,  0, 0);
//#else
//            if (dtmP != nullptr) dtmP->CalculateSlopeArea(&flatAreaTile, &slopeAreaTile, 0, 0);
//#endif
        flatArea += flatAreaTile;
        slopeArea += slopeAreaTile;
        }
    DPoint3d pt = DPoint3d::From(flatArea, slopeArea, 0);
    m_transformToUors.Multiply(pt);
	m_transformToUors.Multiply(pt);
    flatArea = pt.x;
    slopeArea = pt.y;
    if (fullResolutionReturnedNodes->size() == returnedNodes.size())
        {
        progressiveCallback(DTM_SUCCESS, flatArea, slopeArea);
        delete fullResolutionReturnedNodes;
        }
    else
        {
        DPoint3d* transPtsAsync = new DPoint3d[numPoints];
        memcpy(transPtsAsync,&transPts[0], numPoints*sizeof(DPoint3d));

        std::thread t(std::bind([] (bvector<IScalableMeshNodePtr>* nodes, DPoint3d* pts, int numPoints, DTMAreaValuesCallback progressiveCallback, DTMCancelProcessCallback isCancelledCallback, Transform& transformToUors)
            {
            double flatAreaFull = 0;
            double slopeAreaFull = 0;
            bool finished = true;
            DTMStatusInt retval = DTM_ERROR;
            for (auto& node : *nodes)
                {
                if (isCancelledCallback())
                    {
                    finished = false;
                    break;
                    }
                double flatAreaTile = 0;
                double slopeAreaTile = 0;
                DRange3d nodeExt = node->GetContentExtent();
                DPoint3d rangePts[5] = { DPoint3d::From(nodeExt.low.x, nodeExt.low.y, 0), DPoint3d::From(nodeExt.low.x, nodeExt.high.y, 0), DPoint3d::From(nodeExt.high.x, nodeExt.high.y, 0),
                    DPoint3d::From(nodeExt.high.x, nodeExt.low.y, 0), DPoint3d::From(nodeExt.low.x, nodeExt.low.y, 0) };
               // DTM_POLYGON_OBJ* polyP = NULL;
               // long intersectFlag;
                bool restrictToPoly = true;
                //if (DTM_SUCCESS != bcdtmPolygon_intersectPointArrayPolygons(&rangePts[0], 5, &pts[0], numPoints, &intersectFlag, &polyP, 1e-8, 1e-8) || intersectFlag == 0) restrictToPoly = false;
                //if (polyP != NULL) free(polyP);
                const CurveVectorPtr dtmBoundary = CurveVector::CreateLinear(rangePts, 5, CurveVector::BOUNDARY_TYPE_Outer, false);
                const CurveVectorPtr activeBoundary = CurveVector::CreateLinear(pts, numPoints, CurveVector::BOUNDARY_TYPE_Outer, false);
                const CurveVectorPtr intersection = CurveVector::AreaIntersection(*activeBoundary, *dtmBoundary);
                if (!intersection.IsValid() || intersection.IsNull()) restrictToPoly = false;
                BcDTMPtr dtmP = node->GetBcDTM();
                if (restrictToPoly)
                    {
                    double flat, slope;
                    bvector<bvector<bvector<DPoint3d>>> allGeom;
                    intersection->CollectLinearGeometry(allGeom);
                    for (auto& geom : allGeom)
                        for (auto& lineString : geom)
                            {
                            if (dtmP != nullptr)
                                {
                             //   #ifdef VANCOUVER_API
                                if (DTM_SUCCESS == dtmP->CalculateSlopeArea(flat, slope, &lineString[0], (int)lineString.size()))
                            //    #else
                            //    if (DTM_SUCCESS == dtmP->CalculateSlopeArea(&flat, &slope, &lineString[0], (int)lineString.size()))
                             //   #endif
                                    {
                                    retval = DTM_SUCCESS;
                                    flatAreaTile += flat;
                                    slopeAreaTile += slope;
                                    }
                                }
                            }
                    
                    }
                else
               // #ifdef VANCOUVER_API
                if (dtmP->CalculateSlopeArea(flatAreaTile, slopeAreaTile, 0,0) == DTM_SUCCESS) retval = DTM_SUCCESS;
              //  #else
              //  if (dtmP->CalculateSlopeArea(&flatAreaTile, &slopeAreaTile, 0,0) == DTM_SUCCESS) retval = DTM_SUCCESS;
              //  #endif
                
                flatAreaFull += flatAreaTile;
                slopeAreaFull += slopeAreaTile;
                }
			if (finished)
			{
				DPoint3d pt = DPoint3d::From(flatAreaFull, slopeAreaFull, 0);
			    transformToUors.Multiply(pt);
				transformToUors.Multiply(pt);
				flatAreaFull = pt.x;
				slopeAreaFull = pt.y;
				progressiveCallback(retval, flatAreaFull, slopeAreaFull);
			}
            delete nodes;
            delete[] pts;
            }, fullResolutionReturnedNodes, transPtsAsync, numPoints, progressiveCallback, isCancelledCallback, m_transformToUors));
        t.detach();
        }
    return DTM_SUCCESS;
    }

    DTMStatusInt ScalableMeshDTM::_ExportToGeopakTinFile(WCharCP fileNameP, TransformCP transformation)
    {   
    BcDTMP dtm(_GetBcDTM());

    if (dtm == nullptr)
        {
        return DTM_ERROR;
        }
    
    return dtm->ExportToGeopakTinFile(fileNameP, transformation);
    }

bool ScalableMeshDTM::_GetTransformation(TransformR transformation)
    {
    transformation = m_transformToUors;
    return true;
    }

DTMStatusInt ScalableMeshDTM::_GetTransformDTM(DTMPtr& transformedDTM, TransformCR)
    {
    return DTM_ERROR;
    }

DTMStatusInt ScalableMeshDTM::_GetRange(DRange3dR range)
    {
    m_scMesh->GetRange(range);
    return DTM_SUCCESS;
    }

IDTMVolumeP ScalableMeshDTM::_GetDTMVolume()
    {
    return m_dtmVolume;
    }

template <class POINT> BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* ScalableMesh<POINT>::_GetDTMInterface(DTMAnalysisType type)
 {
    return m_scalableMeshDTM[type].get();
 }

template <class POINT> BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* ScalableMesh<POINT>::_GetDTMInterface(DMatrix4d& storageToUors, DTMAnalysisType type)
    {
    //m_scalableMeshDTM[type]->SetStorageToUors(storageToUors);
    return m_scalableMeshDTM[type].get();
    }

template <class POINT> BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* ScalableMesh<POINT>::_GetDTMInterface(DMatrix4d& storageToUors, bvector<DPoint3d>& regionPts, DTMAnalysisType type)
{
    m_scalableMeshDTMRegions.push_back(m_scalableMeshDTM[type]->ExtractRegion(regionPts));
    return m_scalableMeshDTMRegions.back().get();
}



/*----------------------------------------------------------------------------+
|ScalableMesh::GetRootNode
+----------------------------------------------------------------------------*/
template <class POINT> HFCPtr<SMPointIndexNode<POINT, Extent3dType>> ScalableMesh<POINT>::GetRootNode()
    {
    HFCPtr<SMPointIndexNode<POINT, Extent3dType>> pRootNode;

    if (m_scmIndexPtr != 0)
        {
        pRootNode = m_scmIndexPtr->GetRootNode();
        }

    return pRootNode;    
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetPointCount
+----------------------------------------------------------------------------*/
template <class POINT> int64_t ScalableMesh<POINT>::_GetPointCount() 
    {                
    int64_t nbPoints = 0;

    if (m_scmIndexPtr != 0)
        {
        nbPoints += m_scmIndexPtr->GetCount();
        }
    
    //MST : Need to add the number of points in the linear index.    
    return nbPoints;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetNodeCount
+----------------------------------------------------------------------------*/
template <class POINT> uint64_t ScalableMesh<POINT>::_GetNodeCount()
    {
    if (m_scmIndexPtr != 0)
        {
        return m_scmIndexPtr->GetNodeCount();
        }
    return 0;
    }

template <class POINT> bool ScalableMesh<POINT>::_IsTerrain()
    {

    if (m_scmIndexPtr != 0)
        {
        return m_scmIndexPtr->IsTerrain();
        }
    return false;

    }

template <class POINT> bool ScalableMesh<POINT>::_IsTextured()
    {

    if (m_scmIndexPtr != 0)
        {
        return m_scmIndexPtr->IsTextured() != SMTextureType::None;
        }
    return false;
    }

template <class POINT> StatusInt ScalableMesh<POINT>::_GetTextureInfo(IScalableMeshTextureInfoPtr& textureInfo) const
    {    
    if (m_scmIndexPtr == 0)
        {
        return ERROR;        
        }   

    SMTextureType textureType = m_scmIndexPtr->IsTextured();
    bool isUsingBingMap = false;    
    WString bingMapType; 

    // NEEDS_WORK_SM_STREAMING : Save bing map server url in Cesium 3dtiles?
    if (m_smSQLitePtr != nullptr && textureType == SMTextureType::Streaming)
        {
        SourcesDataSQLite sourcesData;
        m_smSQLitePtr->LoadSources(sourcesData);
        
        IDTMSourceCollection sources;
        this->LoadSources(sources);
                
        for (IDTMSourceCollection::const_iterator sourceIt = sources.Begin(), sourcesEnd = sources.End(); sourceIt != sourcesEnd; ++sourceIt)
            {
            const IDTMSource& source = *sourceIt;
            if (source.GetSourceType() == DTM_SOURCE_DATA_IMAGE)
                {  
#if defined(VANCOUVER_API) || defined(DGNDB06_API)
				HFCPtr<HFCURL> pImageURL(HFCURL::Instanciate(source.GetPath()));
#else
				HFCPtr<HFCURL> pImageURL(HFCURL::Instanciate(Utf8String(source.GetPath())));
#endif

                if (HRFVirtualEarthCreator::GetInstance()->IsKindOfFile(pImageURL))
                    {                    
                    isUsingBingMap = true;
                    bingMapType = source.GetPath();                        
                    size_t nbReplaces = bingMapType.ReplaceAll(L"http://www.bing.com/maps/", L"");
                    assert(nbReplaces > 0);
                    break;
                    }                                
                }
            } 
        }

	textureInfo = IScalableMeshTextureInfoPtr(new ScalableMeshTextureInfo(m_scmIndexPtr->IsTextured(), isUsingBingMap, m_scmIndexPtr->GetDataStore()->IsTextureAvailable(), bingMapType));

    return SUCCESS;
    }

template <class POINT> bool ScalableMesh<POINT>::_IsCesium3DTiles()
    {
    return m_isCesium3DTiles;
    }

template <class POINT> bool ScalableMesh<POINT>::_IsStubFile()
    {
    return m_isFromStubFile;
    }

template <class POINT> Utf8String ScalableMesh<POINT>::_GetProjectWiseContextShareLink()
    {
#ifndef LINUX_SCALABLEMESH_BUILD
    if (m_smRDSProvider.IsValid()) return m_smRDSProvider->GetRDSURLAddress();
#endif    
    return Utf8String();
    }

template <class POINT> void ScalableMesh<POINT>::_TextureFromRaster(ITextureProviderPtr provider)
    {
    auto nextID = m_scmIndexPtr->GetDataStore()->GetNextID();
    nextID = nextID != uint64_t(-1) ? nextID : m_scmIndexPtr->GetNextID();
    m_scmIndexPtr->SetNextID(nextID);

    double ratioToMeter(GetGCS().GetUnit().GetRatioToBase());
#ifndef VANCOUVER_API
    Transform smUnitToMeterTransform(Transform::FromScaleFactors(ratioToMeter, ratioToMeter, ratioToMeter));
#else
    Transform smUnitToMeterTransform(Transform::FromRowValues(ratioToMeter, 0, 0, 0, 
                                                              0, ratioToMeter, 0, 0, 
                                                              0, 0, ratioToMeter, 0));
#endif

    m_scmIndexPtr->TextureFromRaster(provider, smUnitToMeterTransform);
    m_scmIndexPtr->Store();
    m_smSQLitePtr->Save();
    m_scmIndexPtr = 0;
    Open();
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetBreaklineCount
+----------------------------------------------------------------------------*/

template <class POINT> int64_t ScalableMesh<POINT>::_GetBreaklineCount() const
    {
   // assert(false && "Not supported!");
    return 0;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetRange
+----------------------------------------------------------------------------*/
template <class POINT> DTMStatusInt ScalableMesh<POINT>::_GetRange(DRange3dR range) 
    {


    range = m_contentExtent;
    return DTM_SUCCESS;
    }


/*----------------------------------------------------------------------------+
|ScalableMesh::_GetBoundary
+----------------------------------------------------------------------------*/
template <class POINT> StatusInt ScalableMesh<POINT>::_GetBoundary(bvector<DPoint3d>& boundary)
    {
    IScalableMeshMeshQueryPtr meshQueryInterface = GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION);
    bvector<IScalableMeshNodePtr> returnedNodes;
    IScalableMeshMeshQueryParamsPtr params = IScalableMeshMeshQueryParams::CreateParams();
    params->SetLevel(std::min((size_t)3, _GetTerrainDepth()));
    meshQueryInterface->Query(returnedNodes, 0,0, params); 
    if (returnedNodes.size() == 0) return ERROR;
    bvector<DPoint3d> current;
    DRange3d rangeCurrent = DRange3d::From(current);
	//PolyfaceHeaderPtr polyface = PolyfaceHeader::CreateVariableSizeIndexed();
	//bmap<DPoint3d, DPoint3d, DPoint3dZYXTolerancedSortComparison> ptsWithTolerance(DPoint3dZYXTolerancedSortComparison(1e-8, 0));
	bvector<bvector<DPoint3d>> bounds;
    for (auto& node : returnedNodes)
        {
        IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
        flags->SetLoadGraph(true);
        auto meshP = node->GetMesh(flags);

        if (IsCesium3DTiles()) 
            meshP->SetTransform(m_reprojectionTransform);
 
        bvector<DPoint3d> bound;
        if (meshP.get() != nullptr && meshP->GetBoundary(bound) == DTM_SUCCESS)
            {
           /* if (current.empty()) current = bound;
            else
                {
                VuPolygonClassifier vu(1e-8, 0);
                vu.ClassifyAUnionB(bound, current);
                bvector<DPoint3d> xyz;
                for (; vu.GetFace(xyz);)
                    {
                    DRange3d rangeXYZ = DRange3d::From(xyz);
                    if (rangeXYZ.XLength() * rangeXYZ.YLength() >= rangeCurrent.XLength() * rangeCurrent.YLength())
                        {
                        current = xyz;
                        rangeCurrent = rangeXYZ;
                        }
                    }
                }*/
			/*for (auto& pt : bound)
			    {
				if (ptsWithTolerance.count(pt) == 0) ptsWithTolerance[pt] = pt;
				pt = ptsWithTolerance[pt];
			    }*/
			bound.push_back(bound[0]);
			if (bsiGeom_getXYPolygonArea(&bound[0], (int)bound.size()) > 0)
				std::reverse(bound.begin(), bound.end());
			bounds.push_back(bound);
			//polyface->AddPolygon(bound);
            }
        }
	MergePolygonSets(bounds);
	
    if (bounds.size() > 0)
        {
        //TFS# 1023914 - If multiple bounds try to obtain a boundary from a DTM created withh all points of all bounds.
        if (bounds.size() > 1)
            {
            BcDTMPtr bcDtm(BcDTM::Create());

            for (auto& bound : bounds)
                {
                bcDtm->AddPoints(bound);
                }    

            DTMStatusInt status = bcDtm->Triangulate ();

            if (status == SUCCESS)
                {        
                BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPointArray pointArray;
                status = bcDtm->GetBoundary(pointArray);

                if (status == SUCCESS)
                    {        
                    current.clear();
                    current = pointArray;
                    }                
                }        
            }

        if (current.size() == 0)
            {
            current = bounds[0];
            }
        }
        
	//polyface->Compress();
	//size_t numOpen = 0, numClosed = 0;
/*	PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface);
	int n = 0;
	for (visitor->Reset(); visitor->AdvanceToNextFace();)
	{
		{
			WString namePoly = WString(L"C:\\work\\2017q2\\cs\\") + L"newpoly_";
			namePoly.append(to_wstring(n).c_str());
			namePoly.append(L".p");
			FILE* polyCliPFile = _wfopen(namePoly.c_str(), L"wb");
			size_t polySize = visitor->Point().size();
			fwrite(&polySize, sizeof(size_t), 1, polyCliPFile);
			fwrite(&visitor->Point()[0], sizeof(DPoint3d), polySize, polyCliPFile);
			fclose(polyCliPFile);
		}
		++n;
	}*/

	/*CurveVectorPtr boundCurve = polyface->ExtractBoundaryStrings(numOpen, numClosed);
	for (auto& curve : *boundCurve)
	    {
		if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == curve->GetCurvePrimitiveType() && curve->GetChildCurveVectorCP()->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Outer)
 		    {
			bvector<bvector<DPoint3d>> loops;
			curve->GetChildCurveVectorCP()->CollectLinearGeometry(loops);
			if (loops.empty())
			    {
				current = loops.front();
				break;
			    }
		    }
	    }*/
    if (current.size() == 0) return ERROR;

    boundary = current;
    return SUCCESS;
    }


/*-------------------Methods inherited from IMRDTM-----------------------------*/

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetCompressionType
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMesh<POINT>::_GenerateSubResolutions()
    {
    return ERROR;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetCompressionType
+----------------------------------------------------------------------------*/
template <class POINT> ScalableMeshCompressionType ScalableMesh<POINT>::_GetCompressionType() const
    {
    return SCM_COMPRESSION_NONE;
    }
  
/*----------------------------------------------------------------------------+
|ScalableMesh::_GetQueryInterface
+----------------------------------------------------------------------------*/
template <class POINT> IScalableMeshPointQueryPtr ScalableMesh<POINT>::_GetQueryInterface(ScalableMeshQueryType                         queryType,                                                                        
                                                                                          BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetBaseGCSPtr,
                                                                                          const DRange3d&                       extentInTargetGCS) const
    {
    IScalableMeshPointQueryPtr scmQueryPtr;
                    
    GCS targetGCS = GetGCSFactory().Create(targetBaseGCSPtr);        

    _GetQueryInterface(queryType);    
    
    // TODO: Remove cast when smart pointers becomes const aware
    ScalableMesh<POINT>* UNCONST_THIS = const_cast<ScalableMesh<POINT>*>(this);

    scmQueryPtr = ScalableMeshPointQuery::GetReprojectionQueryInterface(UNCONST_THIS, queryType, m_sourceGCS, targetGCS, extentInTargetGCS);
             
    return scmQueryPtr;                
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetQueryInterface
+----------------------------------------------------------------------------*/
template <class POINT> IScalableMeshPointQueryPtr ScalableMesh<POINT>::_GetQueryInterface(ScalableMeshQueryType queryType) const
    {
    IScalableMeshPointQueryPtr iScalableMeshQueryPtr;
       
    if ((m_scmIndexPtr != 0) && (m_scmIndexPtr->IsEmpty() == false))
        {
        if (queryType == SCM_QUERY_FULL_RESOLUTION)
            {                                    
            //MST Should be done by using the highest resolution.
            //iScalableMeshQueryPtr = new ScalableMeshFullResolutionQuery(m_mrDTMPointIndexPtr);   
            }
        else
        if (queryType == SCM_QUERY_VIEW_DEPENDENT)
            {                        
            iScalableMeshQueryPtr = new ScalableMeshViewDependentPointQuery<POINT>(&*m_scmIndexPtr);       
            }
        else
        if (queryType == SCM_QUERY_FIX_RESOLUTION_VIEW)
            {        
            iScalableMeshQueryPtr = new ScalableMeshFixResolutionViewPointQuery<POINT>(&*m_scmIndexPtr, m_sourceGCS);
            }
        }
                    
    return iScalableMeshQueryPtr;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetMeshQueryInterface
+----------------------------------------------------------------------------*/
template <class POINT> IScalableMeshMeshQueryPtr ScalableMesh<POINT>::_GetMeshQueryInterface(MeshQueryType queryType) const
    {
    switch (queryType)
        {
        case MESH_QUERY_FULL_RESOLUTION:
            return new ScalableMeshFullResolutionMeshQuery<POINT>(&*m_scmIndexPtr);
        case MESH_QUERY_VIEW_DEPENDENT:
            return new ScalableMeshViewDependentMeshQuery<POINT>(&*m_scmIndexPtr);
        case MESH_QUERY_PLANE_INTERSECT:
            return new ScalableMeshNodePlaneQuery<POINT>(&*m_scmIndexPtr);
        case MESH_QUERY_CONTEXT:
            return new ScalableMeshContextMeshQuery<POINT>(&*m_scmIndexPtr);
        default:
            return new ScalableMeshViewDependentMeshQuery<POINT>(&*m_scmIndexPtr);
        }
    }

template <class POINT> IScalableMeshMeshQueryPtr ScalableMesh<POINT>::_GetMeshQueryInterface(MeshQueryType queryType,
                                                                               BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetBaseGCSPtr,
                                                                               const DRange3d&                       extentInTargetGCS) const
    {
    GCS targetGCS = GetGCSFactory().Create(targetBaseGCSPtr);
    IScalableMeshMeshQueryPtr meshQueryPtr = new ScalableMeshReprojectionMeshQuery<POINT>(_GetMeshQueryInterface(queryType),
                                                                            m_scmIndexPtr,
                                                                 m_sourceGCS,
                                                                 targetGCS,
                                                                 extentInTargetGCS);
    return meshQueryPtr;
    }


/*----------------------------------------------------------------------------+
|ScalableMesh::_GetMeshEditInterface
+----------------------------------------------------------------------------*/
template <class POINT> IScalableMeshNodeRayQueryPtr ScalableMesh<POINT>::_GetNodeQueryInterface() const
    {
    return new ScalableMeshNodeRayQuery<POINT>(&*m_scmIndexPtr);
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetMeshEditInterface
+----------------------------------------------------------------------------*/
template <class POINT> IScalableMeshEditPtr ScalableMesh<POINT>::_GetMeshEditInterface() const
    {
    return ScalableMeshEdit::Create((SMMeshIndex<DPoint3d,DRange3d>*)(&*m_scmIndexPtr));
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetMeshAnalysisInterface
+----------------------------------------------------------------------------*/
template <class POINT> IScalableMeshAnalysisPtr ScalableMesh<POINT>::_GetMeshAnalysisInterface()
    {
    #ifndef LINUX_SCALABLEMESH_BUILD
    return ScalableMeshAnalysis::Create(this);
    #else
    return nullptr;
    #endif
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetNbResolutions
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMesh<POINT>::_GetNbResolutions() const
    {    
    int nbResolutions = 0;
    
    if (m_scmIndexPtr != 0)
        {
        nbResolutions = (int)(m_scmIndexPtr->GetDepth() + 1);
        }        
    
    return nbResolutions;    
    }


template <class POINT> size_t ScalableMesh<POINT>::_GetTerrainDepth() const
    {
    size_t depth = 0;

    if (m_scmIndexPtr != 0)
        {
        depth = m_scmIndexPtr->GetTerrainDepth() != (size_t)-1 ? m_scmIndexPtr->GetTerrainDepth() : m_scmIndexPtr->GetDepth();
        }

    return depth;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::__GetSourceGCS
+----------------------------------------------------------------------------*/
template <class POINT> const GeoCoords::GCS& ScalableMesh<POINT>::_GetGCS() const
    {
    return m_sourceGCS;
    }




/*----------------------------------------------------------------------------+
|ScalableMesh::IsRangeValidInside
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::IsValidInContextOf (const GCS& gcs) const
    {
    if (!gcs.HasGeoRef())
        return true;

    // Now we must validate that the change is possible. If the extent does not fall into the
    // valid domain range of the new GCS, an error will be returned.

    // Obtain the current range points
    DRange3d range;

    // NTERAY: Why isn't _GetRange a const method?
    const_cast<ScalableMesh<POINT>&>(*this)._GetRange(range);

    // Transform both points to latitude/longitude
    GeoPoint dumPoint;
    return BSISUCCESS == gcs.GetGeoRef().GetBase().LatLongFromCartesian(dumPoint, range.low) &&
           BSISUCCESS == gcs.GetGeoRef().GetBase().LatLongFromCartesian(dumPoint, range.high);
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_SetSourceGCS
+----------------------------------------------------------------------------*/
template <class POINT> StatusInt ScalableMesh<POINT>::_SetGCS(const GCS& newGCS)
{
    const GCS& savedGCS = (newGCS.IsNull()) ? GetDefaultGCS() : newGCS;

    //HCPWKT wkt;

    SMStatus wktCreateStatus = SMStatus::S_SUCCESS;
    //wkt = HCPWKT(savedGCS.GetWKT(wktCreateStatus).GetCStr());

    WString extendedWktStr(savedGCS.GetWKT(wktCreateStatus).GetCStr());

    if (WKTKeyword::TYPE_UNKNOWN == GetWktType(extendedWktStr))
    {
        wchar_t wktFlavor[2] = { (wchar_t)ISMStore::WktFlavor_Autodesk, L'\0' };

        extendedWktStr += WString(wktFlavor);
        //wkt = HCPWKT(extendedWktStr.c_str());
    }

    if (SMStatus::S_SUCCESS != wktCreateStatus)
        return BSIERROR;

    // This is called even if there are no GCS provided... In such case the WKT AString
    // is that of the default GCS.
    WString wktStr;
    m_smSQLitePtr->GetWkt(wktStr);
    //HCPWKT oldWkt = HCPWKT(wktStr.c_str());

    if (!m_smSQLitePtr->SetWkt(extendedWktStr.c_str()))
    {
    bool result = m_smSQLitePtr->SetWkt(wktStr.c_str());
        assert(result);

        return BSIERROR;
    }

    m_sourceGCS = savedGCS;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <class POINT> inline bool ScalableMesh<POINT>::IsEmpty () const
    {
    // NTERAY: Not sure if this test is required anymore... If not, we can move this
    //         method to base class.
    if (0 == m_scmIndexPtr)
        return true;

    return EqEps(m_contentExtent.low.x, m_contentExtent.high.x) &&
           EqEps(m_contentExtent.low.y, m_contentExtent.high.y) &&
           EqEps(m_contentExtent.low.z, m_contentExtent.high.z);
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_AddClip
+----------------------------------------------------------------------------*/
template <class POINT> uint64_t ScalableMesh<POINT>::_AddClip(const DPoint3d* pts, size_t ptsSize)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return ((uint64_t)-1);
    if (bsiGeom_getXYPolygonArea(pts, (int)ptsSize) < 0) //need to flip polygon so it's counterclockwise
        {
        uint64_t clipId = 0;
        DPoint3d* flippedPts = new DPoint3d[ptsSize];
        for (size_t pt = 0; pt < ptsSize; ++pt) flippedPts[pt] = pts[ptsSize - 1 - pt];
        clipId = m_scmIndexPtr->GetClipRegistry()->AddClip(flippedPts, ptsSize) + 1;
        delete[] flippedPts;
        return clipId;
        }
    uint64_t id = m_scmIndexPtr->GetClipRegistry()->AddClip(pts, ptsSize)+1;
    SaveEditFiles();
    return id;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_AddClip
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, bool alsoAddOnTerrain)
    {
    bvector<bvector<DPoint3d>> coverageData;
    if (m_scmIndexPtr->GetClipRegistry() == nullptr || pts == nullptr || ptsSize == 0) return false;

    DRange3d extent = DRange3d::From(pts, (int)ptsSize);
    if (extent.Volume() == 0)
        {
        if (extent.XLength() == 0)
            {
            extent.low.x -= 1.e-5;
            extent.high.x += 1.e-5;
            }
        if (extent.YLength() == 0)
            {
            extent.low.y -= 1.e-5;
            extent.high.y += 1.e-5;
            }
        if (extent.ZLength() == 0)
            {
            extent.low.z -= 1.e-5;
            extent.high.z += 1.e-5;
            }
        }

    const DPoint3d* targetPts;
    bvector<DPoint3d> reprojectedPts(ptsSize);
    if (!m_reprojectionTransform.IsIdentity())
        {
        Transform trans;
        trans.InverseOf(m_reprojectionTransform);
        trans.Multiply(&reprojectedPts[0], pts, (int)ptsSize);
        targetPts = reprojectedPts.data();
        extent = DRange3d::From(targetPts, (int)ptsSize);
        }
    else targetPts = pts;

    if (m_scmIndexPtr->GetClipRegistry()->HasClip(clipID)) return false;
    m_scmIndexPtr->GetClipRegistry()->ModifyClip(clipID, targetPts, ptsSize);
    if (!alsoAddOnTerrain || coverageData.empty())
        {
		Transform t = Transform::FromIdentity();
		if (IsCesium3DTiles()) t = GetReprojectionTransform();
        m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_ADD, clipID, extent,true, t);
        }
    else
        {
       /* if (m_terrainP.IsValid())
            {
            m_terrainP->AddClip(targetPts, ptsSize, clipID);
            }*/
        }

    SaveEditFiles();

    return true;
    }


/*----------------------------------------------------------------------------+
|ScalableMesh::_AddClip
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
    {
    const DPoint3d* targetPts;
    bvector<DPoint3d> reprojectedPts(ptsSize);
    if (!m_reprojectionTransform.IsIdentity())
        {
        Transform trans;
        trans.InverseOf(m_reprojectionTransform);
        trans.Multiply(&reprojectedPts[0], pts, (int)ptsSize);
        targetPts = reprojectedPts.data();
        }
    else targetPts = pts;

    DRange3d extent = DRange3d::From(targetPts, (int)ptsSize);
    if (extent.Volume() == 0)
        {
        if (extent.XLength() == 0)
            {
            extent.low.x -= 1.e-5;
            extent.high.x += 1.e-5;
            }
        if (extent.YLength() == 0)
            {
            extent.low.y -= 1.e-5;
            extent.high.y += 1.e-5;
            }
        if (extent.ZLength() == 0)
            {
            extent.low.z -= 1.e-5;
            extent.high.z += 1.e-5;
            }
        }

    if (m_scmIndexPtr->GetClipRegistry()->HasClip(clipID)) return false;
    m_scmIndexPtr->GetClipRegistry()->AddClipWithParameters(clipID, targetPts, ptsSize, geom, type, isActive);

	Transform t = Transform::FromIdentity();
	if (IsCesium3DTiles()) t = GetReprojectionTransform();

    m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_ADD, clipID, extent,true, t);
    SaveEditFiles();
    return true;
    }

template <class POINT> bool ScalableMesh<POINT>::_AddClip(const ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
{
    ClipVectorPtr clipP = ClipVector::CreateCopy(*clip);
    if (!m_reprojectionTransform.IsIdentity())
    {
        Transform trans;
        trans.InverseOf(m_reprojectionTransform);
        clipP->TransformInPlace(trans);
    }

    DRange3d extent;
    clipP->GetRange(extent, nullptr);
    if (extent.Volume() == 0)
    {
        if (extent.XLength() == 0)
        {
            extent.low.x -= 1.e-5;
            extent.high.x += 1.e-5;
        }
        if (extent.YLength() == 0)
        {
            extent.low.y -= 1.e-5;
            extent.high.y += 1.e-5;
        }
        if (extent.ZLength() == 0)
        {
            extent.low.z -= 1.e-5;
            extent.high.z += 1.e-5;
        }
    }

    if (!m_reprojectionTransform.IsIdentity() && IsCesium3DTiles())
    {
        clipP->TransformInPlace(m_reprojectionTransform);
    }
    if (m_scmIndexPtr->GetClipRegistry()->HasClip(clipID)) return false;
    m_scmIndexPtr->GetClipRegistry()->AddClipWithParameters(clipID, clipP, geom, type, isActive);

    Transform t = Transform::FromIdentity();
    if (IsCesium3DTiles()) t = GetReprojectionTransform();

    m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_ADD, clipID, extent, true, t);
    SaveEditFiles();
    return true;
}

/*----------------------------------------------------------------------------+
|ScalableMesh::_ModifyClip
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return false;
    bvector<DPoint3d> clipData;
    m_scmIndexPtr->GetClipRegistry()->GetClip(clipID, clipData);
    DRange3d extent = DRange3d::From(&clipData[0], (int)clipData.size());
    if (extent.Volume() == 0)
        {
        if (extent.XLength() == 0)
            {
            extent.low.x -= 1.e-5;
            extent.high.x += 1.e-5;
            }
        if (extent.YLength() == 0)
            {
            extent.low.y -= 1.e-5;
            extent.high.y += 1.e-5;
            }
        if (extent.ZLength() == 0)
            {
            extent.low.z -= 1.e-5;
            extent.high.z += 1.e-5;
            }
        }

    const DPoint3d* targetPts;
    bvector<DPoint3d> reprojectedPts(ptsSize);
    if (!m_reprojectionTransform.IsIdentity())
        {
        Transform trans;
        trans.InverseOf(m_reprojectionTransform);
        trans.Multiply(&reprojectedPts[0], pts, (int)ptsSize);
        targetPts = reprojectedPts.data();
        }
    else targetPts = pts;
    DRange3d extentNew = DRange3d::From(targetPts, (int)ptsSize);
    extent.Extend(extentNew);

    m_scmIndexPtr->GetClipRegistry()->ModifyClip(clipID, targetPts, ptsSize);

	Transform t = Transform::FromIdentity();
	if (IsCesium3DTiles()) t = GetReprojectionTransform();

    m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_MODIFY, clipID, extent,true, t);
    
    SaveEditFiles();    

    return true;
    }

template <class POINT> bool ScalableMesh<POINT>::_ModifyClip(const ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
{
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return false;
    ClipVectorPtr clipData;
    SMClipGeometryType geom2;
    SMNonDestructiveClipType type2;
    bool isActive2;
    m_scmIndexPtr->GetClipRegistry()->GetClipWithParameters(clipID, clipData,geom2,type2,isActive2);
    DRange3d extent;
    if (!m_reprojectionTransform.IsIdentity() && IsCesium3DTiles())
    {
        Transform trans;
        trans.InverseOf(m_reprojectionTransform);
        clipData->TransformInPlace(trans);
    }
    clipData->GetRange(extent, nullptr);
    if (extent.Volume() == 0)
    {
        if (extent.XLength() == 0)
        {
            extent.low.x -= 1.e-5;
            extent.high.x += 1.e-5;
        }
        if (extent.YLength() == 0)
        {
            extent.low.y -= 1.e-5;
            extent.high.y += 1.e-5;
        }
        if (extent.ZLength() == 0)
        {
            extent.low.z -= 1.e-5;
            extent.high.z += 1.e-5;
        }
    }

    ClipVectorPtr clipP = ClipVector::CreateCopy(*clip);
    if (!m_reprojectionTransform.IsIdentity())
    {
        Transform trans;
        trans.InverseOf(m_reprojectionTransform);
        clipP->TransformInPlace(trans);
    }
    DRange3d extentNew;
    clipP->GetRange(extentNew, nullptr);
    extent.Extend(extentNew);

    if (!m_reprojectionTransform.IsIdentity() && IsCesium3DTiles())
    {
        clipP->TransformInPlace(m_reprojectionTransform);
    }

    m_scmIndexPtr->GetClipRegistry()->ModifyClip(clipID, clipP, geom, type, isActive);

    Transform t = Transform::FromIdentity();
    if (IsCesium3DTiles()) t = GetReprojectionTransform();

    m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_MODIFY, clipID, extent, true, t);

    SaveEditFiles();

    return true;
}

/*----------------------------------------------------------------------------+
|ScalableMesh::_ModifyClip
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
    {
    const DPoint3d* targetPts;
    bvector<DPoint3d> reprojectedPts(ptsSize);
    if (!m_reprojectionTransform.IsIdentity())
        {
        Transform trans;
        trans.InverseOf(m_reprojectionTransform);
        trans.Multiply(&reprojectedPts[0], pts, (int)ptsSize);
        targetPts = reprojectedPts.data();
        }
    else targetPts = pts;

    DRange3d extent = DRange3d::From(targetPts, (int)ptsSize);
    if (extent.Volume() == 0)
        {
        if (extent.XLength() == 0)
            {
            extent.low.x -= 1.e-5;
            extent.high.x += 1.e-5;
            }
        if (extent.YLength() == 0)
            {
            extent.low.y -= 1.e-5;
            extent.high.y += 1.e-5;
            }
        if (extent.ZLength() == 0)
            {
            extent.low.z -= 1.e-5;
            extent.high.z += 1.e-5;
            }
        }

	bvector<DPoint3d> clipData;
	m_scmIndexPtr->GetClipRegistry()->GetClip(clipID, clipData);
	if(!clipData.empty())
		extent.Extend(DRange3d::From(&clipData[0], (int)clipData.size()));

    m_scmIndexPtr->GetClipRegistry()->AddClipWithParameters(clipID, targetPts, ptsSize, geom, type, isActive);

	Transform t = Transform::FromIdentity();
	if (IsCesium3DTiles()) t = GetReprojectionTransform();

    m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_MODIFY, clipID, extent,true, t);
        
    SaveEditFiles();        

    return true;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_RemoveClip
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_RemoveClip(uint64_t clipID)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return false;
    bvector<DPoint3d> clipPolyData;
    m_scmIndexPtr->GetClipRegistry()->GetClip(clipID, clipPolyData);
    if (clipPolyData.empty())
        return true;

    DRange3d extent = DRange3d::From(&clipPolyData[0], (int)clipPolyData.size());

    ClipVectorPtr clipVectorData;
    SMClipGeometryType geom;
    SMNonDestructiveClipType type;
    bool isActive;
    m_scmIndexPtr->GetClipRegistry()->GetClipWithParameters(clipID, clipVectorData, geom, type, isActive);

    if(clipVectorData.IsValid() && !clipVectorData->empty())
        {
        if(!m_reprojectionTransform.IsIdentity() && IsCesium3DTiles())
            {
            Transform trans;
            trans.InverseOf(m_reprojectionTransform);
            clipVectorData->TransformInPlace(trans);
            }

        DRange3d clipVectorRange;
        clipVectorData->GetRange(clipVectorRange, nullptr);
        extent.Extend(clipVectorRange);
        }

    if (extent.Volume() == 0)
        {
        if (extent.XLength() == 0)
            {
            extent.low.x -= 1.e-5;
            extent.high.x += 1.e-5;
            }
        if (extent.YLength() == 0)
            {
            extent.low.y -= 1.e-5;
            extent.high.y += 1.e-5;
            }
        if (extent.ZLength() == 0)
            {
            extent.low.z -= 1.e-5;
            extent.high.z += 1.e-5;
            }
        }
    m_scmIndexPtr->GetClipRegistry()->DeleteClip(clipID);
    m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_DELETE, clipID, extent);
    
    SaveEditFiles();
    
    return true;
    }

template <class POINT> bool ScalableMesh<POINT>::_GetClip(uint64_t clipID, bvector<DPoint3d>& clipData)
{
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return false;
    m_scmIndexPtr->GetClipRegistry()->GetClip(clipID, clipData);
    return !clipData.empty();
}

template <class POINT> bool ScalableMesh<POINT>::_GetClip(uint64_t clipID, ClipVectorPtr& clipData)
{
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return false;
    SMClipGeometryType geom; 
    SMNonDestructiveClipType type; 
    bool isActive;
    m_scmIndexPtr->GetClipRegistry()->GetClipWithParameters(clipID, clipData, geom, type, isActive);
    return clipData.IsValid();
}

template <class POINT> bool ScalableMesh<POINT>::_IsInsertingClips()
    {
    return m_scmIndexPtr->m_isInsertingClips;
    }

template <class POINT> void ScalableMesh<POINT>::_SetIsInsertingClips(bool toggleInsertClips)
    {
    if (nullptr == m_scmIndexPtr || m_scmIndexPtr->GetClipRegistry() == nullptr) return;
    m_scmIndexPtr->GetClipRegistry()->SetAutoCommit(!toggleInsertClips);
    m_scmIndexPtr->m_isInsertingClips = toggleInsertClips;        
    
    SaveEditFiles();
    }

template <class POINT>  bool   ScalableMesh<POINT>::_ShouldInvertClips()
    {
    return m_isInvertingClips;
    }

template <class POINT>  void   ScalableMesh<POINT>::_SetInvertClip(bool invertClips)
    {
    m_isInvertingClips = invertClips;
    }

template <class POINT> void ScalableMesh<POINT>::_ModifyClipMetadata(uint64_t clipId, double importance, int nDimensions)
    {
    if (nullptr == m_scmIndexPtr || m_scmIndexPtr->GetClipRegistry() == nullptr) return;
    m_scmIndexPtr->GetClipRegistry()->SetClipMetadata(clipId, importance, nDimensions);
    
    SaveEditFiles();
    }


template <class POINT>  void ScalableMesh<POINT>::_SynchronizeClipData(const bvector<bpair<uint64_t, bvector<DPoint3d>>>& listOfClips, const bvector<bpair<uint64_t, bvector<bvector<DPoint3d>>>>& listOfSkirts)
    {
    SetIsInsertingClips(true);

    bvector<uint64_t> existingClipIds;
    GetAllClipIds(existingClipIds);
    std::sort(existingClipIds.begin(), existingClipIds.end());

    bset<uint64_t> foundIds;
    
    for (auto& clip : listOfClips)
        {
        uint64_t* addr = std::lower_bound(existingClipIds.begin(), existingClipIds.end(), clip.first);
        if (addr == existingClipIds.end() || *addr != clip.first)
            {
            AddClip(clip.second.data(), clip.second.size(), clip.first);
            }
        else
            {
            foundIds.insert(clip.first);
            ModifyClip(clip.second.data(), clip.second.size(), clip.first);
            }
        }

    for (auto& skirt : listOfSkirts)
        {
        uint64_t* addr = std::lower_bound(existingClipIds.begin(), existingClipIds.end(), skirt.first);
        if (addr == existingClipIds.end() || *addr != skirt.first)
            {
            AddSkirt(skirt.second, skirt.first);
            }
        else
            {
            foundIds.insert(skirt.first);
            ModifySkirt(skirt.second, skirt.first);
            }
        }

    for (auto& id : existingClipIds)
        {
        if (foundIds.count(id) == 0)
            {
            RemoveClip(id);
            RemoveSkirt(id);
            }
        }

    SetIsInsertingClips(false);
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_AddClip
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t clipID, bool alsoAddOnTerrain)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr || skirt.size() == 0 || skirt[0].size() == 0) return false;
    bvector<bvector<DPoint3d>> coverageData;
    m_scmIndexPtr->GetClipRegistry()->GetAllCoveragePolygons(coverageData);
    if (!alsoAddOnTerrain || coverageData.empty())
        {
        bvector<bvector<DPoint3d>> reprojSkirt;
        if (!m_reprojectionTransform.IsIdentity())
            {
            Transform trans;
            trans.InverseOf(m_reprojectionTransform);
            for (auto&vec : skirt)
                {
                bvector<DPoint3d> subskirt(vec.size());
                if (!vec.empty()) trans.Multiply(&subskirt[0], &vec[0], (int)vec.size());
                reprojSkirt.push_back(subskirt);
                }
            }
        else reprojSkirt = skirt;


        DRange3d extent = DRange3d::From(reprojSkirt[0][0]);
        for (auto& vec : reprojSkirt) extent.Extend(vec, nullptr);
        if (m_scmIndexPtr->GetClipRegistry()->HasSkirt(clipID)) return false;
        m_scmIndexPtr->GetClipRegistry()->ModifySkirt(clipID, reprojSkirt);
        m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_ADD, clipID, extent, false);
        }
    else
        {
     /*   if (m_terrainP.IsValid())
            {
            m_terrainP->AddSkirt(skirt, clipID);
            }*/
        }

    SaveEditFiles();

    return true;
    }


template <class POINT> bool ScalableMesh<POINT>::_GetSkirt(uint64_t skirtID, bvector<bvector<DPoint3d>>& skirt)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return false;
    m_scmIndexPtr->GetClipRegistry()->GetSkirt(skirtID, skirt);
    return !skirt.empty();
    }

template <class POINT> bool ScalableMesh<POINT>::_ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t clipID)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr || skirt.size() ==0 || skirt[0].size() ==0) return false;

    bvector<bvector<DPoint3d>> reprojSkirt;
    if (!m_reprojectionTransform.IsIdentity())
        {
        Transform trans;
        trans.InverseOf(m_reprojectionTransform);
        for (auto&vec : skirt)
            {
            bvector<DPoint3d> subskirt(vec.size());
            if (!vec.empty()) trans.Multiply(&subskirt[0], &vec[0], (int)vec.size());
            reprojSkirt.push_back(subskirt);
            }
        }
    else reprojSkirt = skirt;
    DRange3d extent = DRange3d::From(reprojSkirt[0][0]);
    for (auto& vec : reprojSkirt) extent.Extend(vec, nullptr);
    m_scmIndexPtr->GetClipRegistry()->ModifySkirt(clipID, reprojSkirt);
	Transform t = Transform::FromIdentity();
	if (IsCesium3DTiles()) t = GetReprojectionTransform();

    m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_MODIFY, clipID, extent, false, t);
    
    SaveEditFiles();
    
    return true;
    }

template <class POINT> void ScalableMesh<POINT>::_GetAllClipsIds(bvector<uint64_t>& allClipIds)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return;
    m_scmIndexPtr->GetClipRegistry()->GetAllClipsIds(allClipIds);
    }

template <class POINT>  void                         ScalableMesh<POINT>::_SetClipOnOrOff(uint64_t id, bool isActive)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return;
    m_scmIndexPtr->GetClipRegistry()->SetClipOnOrOff(id, isActive);
    
    SaveEditFiles();
    }

template <class POINT>  void                       ScalableMesh<POINT>::_GetIsClipActive(uint64_t id, bool& isActive)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return;
    m_scmIndexPtr->GetClipRegistry()->GetIsClipActive(id, isActive);
    }

template <class POINT>  void                    ScalableMesh<POINT>::_GetClipType(uint64_t id, SMNonDestructiveClipType& type)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return;
    m_scmIndexPtr->GetClipRegistry()->GetClipType(id, type);
    }

template <class POINT>  void                    ScalableMesh<POINT>::_CompactExtraFiles()
   {
	assert(m_scmIndexPtr.GetPtr() != nullptr && m_scmIndexPtr->GetDataStore().IsValid());

	if (m_scmIndexPtr->m_isInsertingClips == true)
		return;

	m_scmIndexPtr->GetDataStore()->CompactProjectFiles();
   }

template <class POINT>  void                    ScalableMesh<POINT>::_WriteExtraFiles()
{
	assert(m_scmIndexPtr.GetPtr() != nullptr && m_scmIndexPtr->GetDataStore().IsValid());

	if (m_scmIndexPtr->m_isInsertingClips == true)
		return;

	m_scmIndexPtr->GetDataStore()->WriteClipDataToProjectFilePath();
}

template <class POINT> void ScalableMesh<POINT>::_GetCurrentlyViewedNodes(bvector<IScalableMeshNodePtr>& nodes)
    {
    nodes = m_viewedNodes;
    }

template <class POINT> void ScalableMesh<POINT>::_SetCurrentlyViewedNodes(const bvector<IScalableMeshNodePtr>& nodes)
    {
    m_viewedNodes = nodes;
    }

template <class POINT> void ScalableMesh<POINT>::SaveEditFiles()
    {        
    if (m_scmIndexPtr.GetPtr() == nullptr || !m_scmIndexPtr->GetDataStore().IsValid())
        return;

    if (m_scmIndexPtr->m_isInsertingClips == true)
        return;

    SMMemoryPool::GetInstance()->RemoveAllItemsOfType(SMStoreDataType::DiffSet, (uint64_t)m_scmIndexPtr.GetPtr());

    m_scmIndexPtr->GetDataStore()->SaveProjectFiles();
    }

template <class POINT> void ScalableMesh<POINT>::_RemoveAllDisplayData()
    {                    
    SMMemoryPool::GetInstance()->RemoveAllItemsOfType(SMStoreDataType::DisplayMesh, (uint64_t)m_scmIndexPtr.GetPtr());
    SMMemoryPool::GetInstance()->RemoveAllItemsOfType(SMStoreDataType::DisplayTexture, (uint64_t)m_scmIndexPtr.GetPtr());
    SMMemoryPool::GetInstanceVideo()->RemoveAllItemsOfType(SMStoreDataType::DisplayMesh, (uint64_t)m_scmIndexPtr.GetPtr());
    SMMemoryPool::GetInstanceVideo()->RemoveAllItemsOfType(SMStoreDataType::DisplayTexture, (uint64_t)m_scmIndexPtr.GetPtr());    
    
    m_scmIndexPtr->TextureManager()->RemoveAllPoolIdForTexture();
    m_scmIndexPtr->TextureManager()->RemoveAllPoolIdForTextureVideo();
    }

template <class POINT> void ScalableMesh<POINT>::_SetEditFilesBasePath(const Utf8String& path)
    {
    m_baseExtraFilesPath = WString(path.c_str(), BentleyCharEncoding::Utf8);
    
	if (m_scmIndexPtr == nullptr) return;
	BeFileName projectFilesPath(m_baseExtraFilesPath.c_str());

	bool result = m_scmIndexPtr->GetDataStore()->SetProjectFilesPath(projectFilesPath);
	assert(result == true);                 
    }

template <class POINT> Utf8String ScalableMesh<POINT>::_GetEditFilesBasePath()
    {
    return Utf8String(m_baseExtraFilesPath);
    }

template <class POINT> void ScalableMesh<POINT>::_GetExtraFileNames(bvector<BeFileName>& extraFileNames) const
    {
    //Clip files
    //NEEDS_WORK_SM : Might be better to get the name from SMSQLiteSisterFile.cpp
#ifdef VANCOUVER_API
    BeFileName fileName(m_baseExtraFilesPath.GetWCharCP());
#else
    BeFileName fileName(m_baseExtraFilesPath);
#endif
    fileName.AppendString(L"_clipDefinitions");

    extraFileNames.push_back(fileName);

    fileName.clear();
#ifdef VANCOUVER_API
    fileName = BeFileName(m_baseExtraFilesPath.GetWCharCP());
#else
    fileName = BeFileName(m_baseExtraFilesPath);
#endif
    fileName.AppendString(L"_clips");

    extraFileNames.push_back(fileName);

    //Coverage terrain files
    bvector<uint64_t> ids;
    GetCoverageIds(ids);

    for (auto& id : ids)
        { 
        //wchar_t idStr[1000];
        //swprintf(idStr, L"%llu", id);
                    
        //Note that the clips file for the coverage terrain are extra files to the coverage terrain.
        fileName.clear();   
        Utf8String coverageName; 
        GetCoverageName(coverageName, id);
     
        GetCoverageTerrainAbsFileName(fileName, m_baseExtraFilesPath, coverageName);
        extraFileNames.push_back(fileName);        
        }    
    }

template <class POINT> IScalableMeshNodePtr ScalableMesh<POINT>::_GetRootNode()
    {
    auto ptr = HFCPtr<SMPointIndexNode<POINT, Extent3dType>>(nullptr);
    if (m_scmIndexPtr == nullptr) 
 #ifndef VANCOUVER_API
        return new ScalableMeshNode<POINT>(ptr);
#else
return  ScalableMeshNode<POINT>::CreateItem(ptr);
#endif
    auto nodeP = m_scmIndexPtr->GetRootNode();
 #ifndef VANCOUVER_API
    return new ScalableMeshNode<POINT>(nodeP);
#else
return  ScalableMeshNode<POINT>::CreateItem(nodeP);
#endif

    }


#ifdef WIP_MESH_IMPORT
template <class POINT> void ScalableMesh<POINT>::_GetAllTextures(bvector<IScalableMeshTexturePtr>& textures)
    {
    size_t nTextures = m_smSQLitePtr->CountTextures();
    textures.resize(nTextures);

    auto meshNode = dynamic_cast<SMMeshIndexNode<POINT,Extent3dType>*>(m_scmIndexPtr->GetRootNode().GetPtr());
    for (size_t i = 1; i <= nTextures; ++i)
        {
        RefCountedPtr<SMMemoryPoolBlobItem<Byte>> texPtr(meshNode->GetTexturePtr(i));

        if (texPtr.IsValid())
            {
            ScalableMeshTexturePtr textureP(ScalableMeshTexture::Create(texPtr));

            if (textureP->GetSize() != 0)
                textures[i-1] = IScalableMeshTexturePtr(textureP.get());
            }
        }
    }
#endif

/*----------------------------------------------------------------------------+
|ScalableMesh::_RemoveClip
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_RemoveSkirt(uint64_t clipID)
    {
    bvector<bvector<DPoint3d>> skirt;
    if (!_GetSkirt(clipID, skirt)) return false;
    DRange3d extent =  DRange3d::From(skirt[0][0]);
    for (auto& vec : skirt) extent.Extend(vec, nullptr);
    m_scmIndexPtr->GetClipRegistry()->DeleteSkirt(clipID);

	Transform t = Transform::FromIdentity();
	if (IsCesium3DTiles()) t = GetReprojectionTransform();

    m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_DELETE, clipID, extent, false, t);
    SaveEditFiles();
    return true;
    }


template <class POINT> IScalableMeshPtr ScalableMesh<POINT>::_GetGroup()
    {
    return m_groupP;
    }

template <class POINT> void ScalableMesh<POINT>::_AddToGroup(IScalableMeshPtr& sMesh, bool isRegionRestricted, const DPoint3d* region, size_t nOfPtsInRegion)
    {
    if (!m_groupP.IsValid())
        {
        m_groupP = ScalableMeshGroup::Create();
        }
    
    dynamic_cast<ScalableMeshGroup*>(m_groupP.get())->AddMember(sMesh, isRegionRestricted, region, nOfPtsInRegion);
    }


template <class POINT> void ScalableMesh<POINT>::_RemoveFromGroup(IScalableMeshPtr& sMesh)
    {
    if (m_groupP.IsValid())
        {
        dynamic_cast<ScalableMeshGroup*>(m_groupP.get())->RemoveMember(sMesh);
        }
    }
/*----------------------------------------------------------------------------+
|ScalableMesh::_GetState
+----------------------------------------------------------------------------*/
template <class POINT> ScalableMeshState ScalableMesh<POINT>::_GetState() const
    {
    if (IsEmpty())
        return SCM_STATE_EMPTY;

    return _InSynchWithSources() ? SCM_STATE_UP_TO_DATE : SCM_STATE_DIRTY;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_InSynchWithSources
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_InSynchWithSources() const
    {
    if (!m_smSQLitePtr.IsValid())
        return false; 

    SourcesDataSQLite sourcesData;
    m_smSQLitePtr->LoadSources(sourcesData);

    const bool InSync = sourcesData.GetLastModifiedTime() <= sourcesData.GetLastSyncTime();
    return InSync;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_LoadSources
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_LoadSources(IDTMSourceCollection& sources) const
    {
    if(!m_smSQLitePtr.IsValid())
        return false;

    SourcesDataSQLite sourcesData;
    m_smSQLitePtr->LoadSources(sourcesData);

    return BENTLEY_NAMESPACE_NAME::ScalableMesh::LoadSources(sources, sourcesData, DocumentEnv(L""));
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetRangeInSpecificGCS
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMesh<POINT>::_GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const
    {
    StatusInt status = SUCCESS;

    DRange3d reprojectedRange;
    status = ReprojectRangeDomainLimited(reprojectedRange, m_contentExtent, const_cast<BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr&>(m_sourceGCS.GetGeoRef().GetBasePtr()), targetGCS);

    lowPt.x = reprojectedRange.low.x;
    highPt.x = reprojectedRange.high.x;
    lowPt.y = reprojectedRange.low.y;
    highPt.y = reprojectedRange.high.y;
    lowPt.z = reprojectedRange.low.z;
    highPt.z = reprojectedRange.high.z;


    return status;


    }



/*----------------------------------------------------------------------------+
|ScalableMesh::_LastSynchronizationCheck
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_LastSynchronizationCheck(time_t& lastCheckTime) const
    {
    if (!m_smSQLitePtr.IsValid())
        return false;

    SourcesDataSQLite sourcesData;
    m_smSQLitePtr->LoadSources(sourcesData);

    lastCheckTime = sourcesData.GetLastModifiedCheckTime();

    return true;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_IsReadOnly
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_IsReadOnly() const
    {
    if (m_smSQLitePtr.IsValid())
        return m_smSQLitePtr->IsReadOnly();
    return true;
    } 

/*----------------------------------------------------------------------------+
|ScalableMesh::_IsShareable
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_IsShareable() const
    {

        return true;

    } 

/*----------------------------------------------------------------------------+
|ScalableMesh::_SaveAs
+----------------------------------------------------------------------------*/
template <class POINT> StatusInt ScalableMesh<POINT>::_SaveAs(const WString& destination, ClipVectorPtr clips, IScalableMeshProgressPtr progress)
    {
#if defined(NEED_SAVE_AS_IN_IMPORT_DLL) && !defined(DGNDB06_API)
    // Create Scalable Mesh at output path
    StatusInt status;
    IScalableMeshNodeCreatorPtr scMeshDestination = IScalableMeshNodeCreator::GetFor(destination.c_str(), status);
    if (SUCCESS != status || !scMeshDestination.IsValid())
        return ERROR;

    IScalableMeshTextureInfoPtr textureInfo = nullptr;
    if (SUCCESS != GetTextureInfo(textureInfo))
        return ERROR;

    // Set global parameters to the new 3sm (this will also create a new index)
    if (SUCCESS != scMeshDestination->SetGCS(m_sourceGCS))
        return ERROR;
    scMeshDestination->SetIsTerrain(IsTerrain());
    scMeshDestination->SetIsSingleFile(!IsCesium3DTiles());
    scMeshDestination->SetTextured(textureInfo->GetTextureType());

    { // Scope the publishing part for proper cleanup of parameters when finished
    SM3SMPublishParamsPtr smParams = new SM3SMPublishParams();

    smParams->SetSource(this);
    smParams->SetDestination(scMeshDestination);
    smParams->SetClips(clips);
    smParams->SetProgress(progress);
    smParams->SetSaveTextures(textureInfo->IsTextureAvailable() && !textureInfo->IsUsingBingMap());

    auto smPublisher = IScalableMeshPublisher::Create(SMPublishType::THREESM);
    if (SUCCESS != smPublisher->Publish(smParams))
        return ERROR;
    }

    scMeshDestination = nullptr;

    if (m_smSQLitePtr->HasSources())
        {
        IScalableMeshSourceCreatorPtr destMeshSourceEdit = IScalableMeshSourceCreator::GetFor(destination.c_str(), status);
        if (status != SUCCESS) 
            return ERROR;

        IDTMSourceCollection sources;
        if (!this->LoadSources(sources))
            return ERROR;

        destMeshSourceEdit->EditSources() = sources;
        destMeshSourceEdit->SetSourcesDirty();

        if (SUCCESS != destMeshSourceEdit->SaveToFile())
            return ERROR;
        }
#else
    assert(!"_SaveAs not yet implemented on this code base");
#endif
    return SUCCESS;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_Generate3DTiles
+----------------------------------------------------------------------------*/
template <class POINT> StatusInt ScalableMesh<POINT>::_Generate3DTiles(const WString& outContainerName, const WString& outDatasetName, SMCloudServerType server, IScalableMeshProgressPtr progress, ClipVectorPtr clips, uint64_t coverageId) const
    {

#if defined(NEED_SAVE_AS_IN_IMPORT_DLL) && !defined(DGNDB06_API)
    if (m_scmIndexPtr == nullptr) return ERROR;

    StatusInt status;

    WString path;
    if (server == SMCloudServerType::Azure)
        {
        // Setup streaming stores to use Azure
        //s_stream_from_disk = false; 
        s_stream_from_wsg = false;

        path += outContainerName + L"/" + outDatasetName;
        }
    else if (server == SMCloudServerType::WSG)
        {
        // Setup streaming stores to use WSG
        //s_stream_from_disk = false;
        s_stream_from_wsg = true;

        path += outContainerName + L"~2F" + outDatasetName;
        }
    else if (server == SMCloudServerType::LocalDiskCURL)
        {
        // Setup streaming stores to use local disk (relative to attached 3sm file location)
        //s_stream_from_disk = true;
        s_stream_using_curl = true;

        const auto smFileName = BeFileName(this->GetPath());
        path += BEFILENAME(GetDirectoryName, smFileName);
        path += L"cloud\\";
        path += BEFILENAME(GetFileNameWithoutExtension, smFileName);
        }
    else
        {
        assert(server == SMCloudServerType::LocalDisk);

        // Setup streaming stores to use local disk (relative to attached 3sm file location)
        //s_stream_from_disk = true;
        path = outContainerName;
        }
    
    //s_stream_from_grouped_store = false;
    m_scmIndexPtr->SetProgressCallback(progress);
    bool hasCoverages = false;
    bvector<SMNodeGroupPtr> coverageTilesets;

    if (coverageId == (uint64_t)-1)
        {
        // Generate 3DTiles tilesets for all coverages
        bvector<uint64_t> ids;
        m_scmIndexPtr->GetClipRegistry()->GetAllCoverageIds(ids);
        hasCoverages = !ids.empty();
        for (auto coverageID : ids)
            {
            Utf8String coverageName;
            m_scmIndexPtr->GetClipRegistry()->GetCoverageName(coverageID, coverageName);

            BeFileName coverageFileName(coverageName.c_str());
            if (BeFileName::DoesPathExist(coverageFileName))
                {
                // Ensure that coverage path is formatted correctly (e.g. remove redundant double backslashes such as \\\\)
                WString coverageFullPathName;
                BeFileName::BeGetFullPathName(coverageFullPathName, coverageFileName.c_str());

                IScalableMeshPtr coverageMeshPtr = nullptr;
                if ((coverageMeshPtr = IScalableMesh::GetFor(coverageFullPathName.c_str(), Utf8String(m_baseExtraFilesPath.c_str()), false, true, true, status)) == nullptr || status != SUCCESS)
                    {
                    BeAssert(false); // Error opening coverage 3sm
                    return status;
                    }

                auto coverageMesh = static_cast<ScalableMesh<POINT>*>(coverageMeshPtr.get());

                // Create directory for coverage tileset output
                BeFileName coverageOutDir(path.c_str());
                coverageOutDir.AppendToPath(BeFileName::GetFileNameWithoutExtension(coverageFileName).c_str());
                coverageOutDir.AppendSeparator();
                if (!BeFileName::DoesPathExist(coverageOutDir) && (status = (StatusInt)BeFileName::CreateNewDirectory(coverageOutDir)) != SUCCESS)
                    {
                    BeAssert(false); // Could not create tileset output directory for coverage
                    return status;
                    }
                if ((status = coverageMesh->_Generate3DTiles(coverageOutDir.c_str(), outDatasetName, server, nullptr /*no progress?*/, clips, coverageID)) != SUCCESS)
                    {
                    BeAssert(false); // Could not publish coverage
                    return status;
                    }
                auto coverageIndex = coverageMesh->m_scmIndexPtr;
                auto root = coverageIndex->GetRootNodeGroup();
                BeAssert(root.IsValid()); // Something wrong in the publish
                coverageTilesets.push_back(root);
                }
            }
        }


    IScalableMeshTextureInfoPtr textureInfo;

    status = ScalableMesh<POINT>::_GetTextureInfo(textureInfo);

    bool outputTexture = true;

    //BingMap texture MUST NOT be baked into the Cesium 3D tile data
    if (status != SUCCESS || textureInfo->IsUsingBingMap())
        outputTexture = false;

    
    status = m_scmIndexPtr->Publish3DTiles(path, m_reprojectionTransform, clips, (uint64_t)(hasCoverages && coverageId == (uint64_t)-1 ? 0 : coverageId), this->_GetGCS().GetGeoRef().GetBasePtr(), outputTexture);
    SMNodeGroupPtr rootTileset = m_scmIndexPtr->GetRootNodeGroup();
    BeAssert(rootTileset.IsValid()); // something wrong in the publish


    for (auto& converageTileset : coverageTilesets)
        {
        // insert tileset as child tileset to the current tileset
        rootTileset->AppendChildGroup(converageTileset);
        converageTileset->Close<Extent3dType>();
        }

    WString wktStr;
    if (m_smSQLitePtr != nullptr)
        m_smSQLitePtr->GetWkt(wktStr);

    rootTileset->GetParameters()->SetWellKnownText(wktStr);

    // Force save of root tileset and take into account coverages
    rootTileset->Close<Extent3dType>();
    return status;
#else
    assert(!"_Generate3DTiles not yet implemented on this code base");
#endif

    return SUCCESS;
    }

template <class POINT>  SMStatus                      ScalableMesh<POINT>::_DetectGroundForRegion(BeFileName& createdTerrain, const BeFileName& coverageTempDataFolder, const bvector<DPoint3d>& coverageData, uint64_t id, IScalableMeshGroundPreviewerPtr groundPreviewer, BaseGCSCPtr& destinationGcs, bool limitResolution, bool reprojectElevation, const BeFileName& dataSourceDir)
    {    

#if NEED_SAVE_AS_IN_IMPORT_DLL
    BeFileName terrainAbsName;

    Utf8String coverageName(createdTerrain);

    GetCoverageTerrainAbsFileName(terrainAbsName, m_baseExtraFilesPath, coverageName);

#ifndef VANCOUVER_API
    assert(!terrainAbsName.DoesPathExist());
#else    
    assert(!BeFileName::DoesPathExist(terrainAbsName.c_str()));
#endif

    if (s_doGroundExtract /*&& m_scmTerrainIndexPtr == nullptr*/)
        {
        IScalableMeshPtr scalableMeshPtr(this);

        m_scmTerrainIndexPtr = 0;
        m_terrainP = 0;
        /*
        int result = _wremove(newPath.c_str());
        assert(result == 0);
        */
        IScalableMeshGroundExtractorPtr smGroundExtractor(IScalableMeshGroundExtractor::Create(terrainAbsName, scalableMeshPtr));

        BaseGCSPtr newDestPtr = (BaseGCS*)destinationGcs.get();
        smGroundExtractor->SetDestinationGcs(newDestPtr);
        smGroundExtractor->SetExtractionArea(coverageData);
        smGroundExtractor->SetGroundPreviewer(groundPreviewer);
        smGroundExtractor->SetLimitTextureResolution(limitResolution);
        smGroundExtractor->SetReprojectElevation(reprojectElevation);

        if (!dataSourceDir.empty())
            {
            smGroundExtractor->SetDataSourceDir(dataSourceDir);
            }
                                
        StatusInt status = smGroundExtractor->ExtractAndEmbed(coverageTempDataFolder);

		if (status != SUCCESS)
			return status == SUCCESS ? SMStatus::S_SUCCESS : SMStatus::S_ERROR;
/*
        Utf8String newBaseEditsFilePath = Utf8String(m_baseExtraFilesPath) + "_terrain_";
        newBaseEditsFilePath.append(std::to_string(id).c_str());
*/

        StatusInt openStatus;
                                                                       
        SMSQLiteFilePtr smSQLiteFile(SMSQLiteFile::Open(WString(terrainAbsName.c_str()), false, openStatus));

        if (openStatus && smSQLiteFile != nullptr)
            {
 /*           m_terrainP = ScalableMesh<DPoint3d>::Open(smSQLiteFile, newPath, newBaseEditsFilePath, openStatus);
            m_terrainP->SetInvertClip(true);
            m_scmTerrainIndexPtr = dynamic_cast<ScalableMesh<DPoint3d>*>(m_terrainP.get())->GetMainIndexP();*/
            }
        }


    createdTerrain = terrainAbsName;
#else
    assert(!"_DetectGroundForRegion not yet implemented on this code base");
#endif
	return SMStatus::S_SUCCESS;
    }


static bool s_doGroundExtract = true; 

template <class POINT> BentleyStatus ScalableMesh<POINT>::_CreateCoverage( const bvector<DPoint3d>& coverageData, uint64_t id, const Utf8String& coverageName)
    {
   // if (m_scmTerrainIndexPtr == nullptr) return ERROR;
    _AddClip(coverageData.data(), coverageData.size(), id, false);
    bvector<bvector<DPoint3d>> skirts;
    skirts.push_back(coverageData);
   // _AddSkirt(skirts, id, false);

/*    DRange3d extent = */DRange3d::From(&coverageData[0], (int)coverageData.size());
    m_scmIndexPtr->GetClipRegistry()->ModifyCoverage(id, coverageData.data(), coverageData.size(), coverageName);

    //m_terrainP->AddClip(coverageData.data(), coverageData.size(), id);

    SaveEditFiles();
    return SUCCESS;
    }

template <class POINT> void ScalableMesh<POINT>::_GetAllCoverages(bvector<bvector<DPoint3d>>& coverageData)
    {
    m_scmIndexPtr->GetClipRegistry()->GetAllCoveragePolygons(coverageData);
    }

template <class POINT> void ScalableMesh<POINT>::_GetCoverageIds(bvector<uint64_t>& ids) const
    {
    if (nullptr == m_scmIndexPtr) return;
    m_scmIndexPtr->GetClipRegistry()->GetAllCoverageIds(ids);
    }

template <class POINT> void ScalableMesh<POINT>::_GetCoverageName(Utf8String& name, uint64_t id) const
    {
    if (nullptr == m_scmIndexPtr) return;
    m_scmIndexPtr->GetClipRegistry()->GetCoverageName(id, name);    
    }

template <class POINT> BentleyStatus ScalableMesh<POINT>::_DeleteCoverage(uint64_t id)
    {
    m_scmIndexPtr->GetClipRegistry()->DeleteCoverage(id);
    _RemoveClip(id);
    SaveEditFiles();
    return SUCCESS;
    }

template <class POINT> IScalableMeshClippingOptions& ScalableMesh<POINT>::_EditClippingOptions()
   {

	return *m_clippingOptions;
   }

template <class POINT> IScalableMeshPtr ScalableMesh<POINT>::_GetTerrainSM()
    {
    return m_terrainP;
    }

template <class POINT>  Transform  ScalableMesh<POINT>::_GetReprojectionTransform() const
    {
    return m_reprojectionTransform;
    }

template <class POINT> BentleyStatus  ScalableMesh<POINT>::_SetReprojection(GeoCoordinates::BaseGCSCR targetCS, TransformCR approximateTransform)
    {
    m_reprojectionTransform = approximateTransform;
    for (size_t i = 0; i < DTMAnalysisType::Qty; ++i)
        {
        auto mat4d = DMatrix4d::From(approximateTransform);
        m_scalableMeshDTM[i]->SetStorageToUors(mat4d);
        }

    return SUCCESS;
    }

#ifdef VANCOUVER_API
template <class POINT> BentleyStatus  ScalableMesh<POINT>::_Reproject(GeoCoordinates::BaseGCSCP targetCS, DgnModelRefP dgnModel)
    {   
    #ifndef LINUX_SCALABLEMESH_BUILD
    if (targetCS == nullptr && this->IsCesium3DTiles() && m_streamingSettings != nullptr && m_streamingSettings->IsGCSStringSet())
        {
        // Fall back on the GCS saved in the SM metadata for Cesium tilesets
        auto ecefGCS = m_sourceGCS;
        if (!LoadGCSFrom(m_streamingSettings->GetGCSString()))
            return BSIERROR; // Error loading layer gcs

                             // Reproject data using this new GCS
        auto newGCS = m_sourceGCS;
        std::swap(m_sourceGCS, ecefGCS);

        if (newGCS.GetGeoRef().GetBasePtr().get() == nullptr)
            return ERROR;
        DgnGCSPtr newDgnGcsPtr(DgnGCS::CreateGCS(newGCS.GetGeoRef().GetBasePtr().get(), dgnModel));
        return this->_Reproject(newDgnGcsPtr.get(), dgnModel);
        }
    #endif
    
    // Greate a GCS from the ScalableMesh
    GeoCoords::GCS gcs(this->GetGCS());
    GeoCoords::Unit unit(gcs.GetHorizontalUnit());

    //TFS#721455 - dgnModel may be an attachment
    DgnModelCP targetModel = dgnModel->AsDgnModelCP();
    if (targetModel == nullptr && dgnModel->AsDgnAttachmentCP() != nullptr)
        targetModel = dgnModel->AsDgnAttachmentCP()->GetDgnModelP();

    assert(targetModel != nullptr);

    if (targetModel == nullptr) return ERROR; //something is wrong with the reference;
    auto& modelInfo = targetModel->GetModelInfo();

    double scaleUorPerMeters = ModelInfo::GetUorPerMeter(&modelInfo) * (this->IsCesium3DTiles() ? 1.0 : unit.GetRatioToBase());

    DPoint3d globalOrigin = modelInfo.GetGlobalOrigin();

    Transform computedTransform = Transform::FromRowValues(scaleUorPerMeters, 0, 0, 0.0,
                                                           0, scaleUorPerMeters, 0, 0.0,
                                                           0, 0, scaleUorPerMeters, 0.0);

    //auto coordInterp = this->IsCesium3DTiles() ? GeoCoordinates::GeoCoordInterpretation::XYZ : GeoCoordinates::GeoCoordInterpretation::Cartesian;
    auto coordInterp = GeoCoordinates::GeoCoordInterpretation::Cartesian;

    if (this->IsCesium3DTiles())
        {
        auto tileToDb = m_streamingSettings->GetTileToDbTransform();
        if (!tileToDb.IsIdentity())
            {
            computedTransform = Transform::FromProduct(computedTransform, tileToDb);
            auto tileToECEF = m_streamingSettings->GetTileToECEFTransform();
            if(!tileToECEF.IsIdentity())
                {
                Transform ecefToTile;
                ecefToTile.InverseOf(tileToECEF);
                computedTransform = Transform::FromProduct(computedTransform, ecefToTile);
                }
            }
        else
            { // tile coordinates are not transformed, therefore they must be interpreted as XYZ coordinates
            coordInterp = GeoCoordinates::GeoCoordInterpretation::XYZ;
            }
        }
	

    if (targetCS == nullptr || !gcs.HasGeoRef())
        {
        BaseGCSPtr targetGcs(BaseGCS::CreateGCS());
                        
        return _SetReprojection(*targetGcs, computedTransform);
        }
    else
        {
        GeoCoordinates::DgnGCSPtr  smGCS = nullptr;
        if (coordInterp == GeoCoordinates::GeoCoordInterpretation::XYZ)
            {
            smGCS = GeoCoordinates::DgnGCS::CreateGCS(L"ll84", dgnModel);
            }
        else
            smGCS = GeoCoordinates::DgnGCS::CreateGCS(gcs.GetGeoRef().GetBasePtr().get(), dgnModel);

        assert(smGCS != nullptr); // Error creating SM GCS from GeoRef for reprojection

        computedTransform = Transform::FromProduct(Transform::From(globalOrigin.x, globalOrigin.y, globalOrigin.z), computedTransform);
        if (!targetCS->IsEquivalent(*smGCS))
            {
            smGCS->SetReprojectElevation(true);

            DRange3d smExtent, smExtentUors;
            this->GetRange(smExtent);
            computedTransform.Multiply(smExtentUors, smExtent);

            DPoint3d extent;
            extent.DifferenceOf(smExtentUors.high, smExtentUors.low);
            Transform       approxTransform;

            auto status = smGCS->GetLocalTransform(&approxTransform, smExtentUors.low, &extent, true/*doRotate*/, true/*doScale*/, coordInterp, static_cast<DgnGCSCR>(*targetCS));

            if (0 == status || 1 == status)
                {
                computedTransform = Transform::FromProduct(approxTransform, computedTransform);
                }
            }
        }

    return _SetReprojection(*targetCS, computedTransform);
    }
#else
template <class POINT> BentleyStatus  ScalableMesh<POINT>::_Reproject(DgnGCSCP targetCS, DgnDbR dgnProject)
    {
#ifdef DGNDB06_API
    assert(!"ERROR - BIM0200 code not ported");
    return BSIERROR;
#else
    if (this->IsCesium3DTiles() && targetCS == nullptr && m_streamingSettings != nullptr && m_streamingSettings->IsGCSStringSet())
        {
        // Fall back on the GCS saved in the SM metadata for Cesium tilesets
        auto ecefGCS = m_sourceGCS;
        if (!LoadGCSFrom(m_streamingSettings->GetGCSString()))
            return BSIERROR; // Error loading layer gcs

                             // Reproject data using this new GCS
        auto newGCS = m_sourceGCS;
        std::swap(m_sourceGCS, ecefGCS);

        if (newGCS.GetGeoRef().GetBasePtr().get() == nullptr)
            return BSIERROR;
        DgnGCSPtr newDgnGcsPtr(DgnGCS::CreateGCS(newGCS.GetGeoRef().GetBasePtr().get(), dgnProject));
        return this->_Reproject(newDgnGcsPtr.get(), dgnProject);
        }

    DPoint3d globalOrigin = dgnProject.GeoLocation().GetGlobalOrigin();

    // Greate a GCS from the ScalableMesh
    GeoCoords::GCS gcs(this->GetGCS());
    GeoCoords::Unit unit(gcs.GetHorizontalUnit());
    double scaleUorPerMeters = unit.GetRatioToBase();

    Transform computedTransform = Transform::FromScaleFactors(scaleUorPerMeters, scaleUorPerMeters, scaleUorPerMeters);
    auto coordInterp = Dgn::GeoCoordInterpretation::Cartesian;
    if (this->IsCesium3DTiles())
        {
        auto tileToDb = m_streamingSettings->GetTileToDbTransform();
        if (!tileToDb.IsIdentity())
            {
            computedTransform = Transform::FromProduct(computedTransform, tileToDb);
            auto tileToECEF = m_streamingSettings->GetTileToECEFTransform();
            if(!tileToECEF.IsIdentity())
                {
                Transform ecefToTile;
                ecefToTile.InverseOf(tileToECEF);
                computedTransform = Transform::FromProduct(computedTransform, ecefToTile);
                }
            }
        else
            { // tile coordinates are not transformed, therefore they must be interpreted as XYZ coordinates
            coordInterp = Dgn::GeoCoordInterpretation::XYZ;
            }
        }

    if (gcs.HasGeoRef())
        {
        DgnGCSPtr  smGCS = nullptr;
        if (coordInterp == Dgn::GeoCoordInterpretation::XYZ)
            {
            smGCS = DgnGCS::CreateGCS(L"ll84", dgnProject);
            }
        else
            smGCS = DgnGCS::CreateGCS(gcs.GetGeoRef().GetBasePtr().get(), dgnProject);

        assert(smGCS != nullptr); // Error creating SM GCS from GeoRef for reprojection

        computedTransform = Transform::FromProduct(Transform::From(globalOrigin.x, globalOrigin.y, globalOrigin.z), computedTransform);
        if (targetCS != nullptr && !targetCS->IsEquivalent(*smGCS))
            {
            smGCS->SetReprojectElevation(true);


            DRange3d smExtent, smExtentUors;
            this->GetRange(smExtent);
            computedTransform.Multiply(smExtentUors, smExtent);

            DPoint3d extent;
            extent.DifferenceOf(smExtentUors.high, smExtentUors.low);
            Transform       approxTransform;

            StatusInt status = smGCS->GetLocalTransform(&approxTransform, smExtentUors.low, &extent, true/*doRotate*/, true/*doScale*/, coordInterp, *targetCS);
            if (0 == status || 1 == status || 25 == status)
                {
                computedTransform = Transform::FromProduct(approxTransform, computedTransform);
                }
            }
        }
    else
        {
        DPoint3d scale = DPoint3d::From(1.0, 1.0, 1.0);
        if (targetCS != nullptr)
            {
            dgnProject.GeoLocation().GetDgnGCS()->UorsFromCartesian(scale, scale);
            scale.DifferenceOf(scale, globalOrigin);
            }

        computedTransform = Transform::FromProduct(Transform::FromScaleFactors(scale.x, scale.y, scale.z), computedTransform);
        }

    return _SetReprojection(*targetCS, computedTransform);
#endif
    }
#endif

template <class POINT> void ScalableMesh<POINT>::_ImportTerrainSM(WString terrainPath)
    {
    StatusInt status;
    m_terrainP = IScalableMesh::GetFor(terrainPath.c_str(), false, true, status);
    if (status != SUCCESS) return;
    auto nodeP = m_terrainP->GetRootNode();

    //create terrain index
    auto pool = SMMemoryPool::GetInstance();
    auto store = m_scmIndexPtr->GetDataStore();
    m_scmTerrainIndexPtr = new MeshIndexType(store,
                                        pool,
                                   10000,
                                   dynamic_cast<ScalableMeshNode<POINT>*>(nodeP.get())->GetNodePtr()->GetFilter(),
                                   true, true, true, true,
                                   dynamic_cast<SMMeshIndexNode<POINT,Extent3dType>*>((dynamic_cast<ScalableMeshNode<POINT>*>(nodeP.get()))->GetNodePtr().GetPtr())->GetMesher2_5d(),
                                   dynamic_cast<SMMeshIndexNode<POINT, Extent3dType>*>((dynamic_cast<ScalableMeshNode<POINT>*>(nodeP.get()))->GetNodePtr().GetPtr())->GetMesher3d()
                                   );
    auto rootNodeP = m_scmTerrainIndexPtr->CreateRootNode();
    dynamic_cast<SMMeshIndexNode<POINT, Extent3dType>*>(rootNodeP.GetPtr())->ImportTreeFrom(nodeP);
    }

#ifdef SCALABLE_MESH_ATP
/*----------------------------------------------------------------------------+
|ScalableMesh::_ChangeGeometricError
+----------------------------------------------------------------------------*/
template <class POINT> StatusInt ScalableMesh<POINT>::_ChangeGeometricError(const WString& outContainerName, const WString& outDatasetName, SMCloudServerType server, const double& newGeometricErrorValue) const
    {
    if (m_scmIndexPtr == nullptr) return ERROR;

#ifndef LINUX_SCALABLEMESH_BUILD
    WString path;
    if (server == SMCloudServerType::Azure)
        {
        // Setup streaming stores to use Azure
        //s_stream_from_disk = false;
        s_stream_from_wsg = false;

        path += outContainerName + L"/" + outDatasetName;
        }
    else if (server == SMCloudServerType::WSG)
        {
        // Setup streaming stores to use WSG
        //s_stream_from_disk = false;
        s_stream_from_wsg = true;

        path += outContainerName + L"~2F" + outDatasetName;
        }
    else if (server == SMCloudServerType::LocalDiskCURL)
        {
        // Setup streaming stores to use local disk (relative to attached 3sm file location)
        //s_stream_from_disk = true;
        s_stream_using_curl = true;

        const auto smFileName = BeFileName(this->GetPath());

#ifndef VANCOUVER_API   
        path += smFileName.GetDirectoryName();
#else
        WChar pathBuffer[5000];
        path += smFileName.GetDirectoryName(pathBuffer);
#endif
        
        path += L"cloud\\";
        
#ifndef VANCOUVER_API   
        path += smFileName.GetFileNameWithoutExtension();
#else        
        path += smFileName.GetFileNameWithoutExtension(pathBuffer);
#endif

        }
    else
        {
        assert(server == SMCloudServerType::LocalDisk);

        // Setup streaming stores to use local disk (relative to attached 3sm file location)
        //s_stream_from_disk = true;
        path = outContainerName;
        }

    //s_stream_from_grouped_store = false;

    return m_scmIndexPtr->ChangeGeometricError(path, true, newGeometricErrorValue);
    #else
    return false;
    #endif
    }

/*----------------------------------------------------------------------------+
|MrDTM::_LoadAllNodeHeaders
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMesh<POINT>::_LoadAllNodeHeaders(size_t& nbLoadedNodes, int level) const
    {    
    m_scmIndexPtr->LoadIndexNodes(nbLoadedNodes, level, true);
    return SUCCESS;
    } 

/*----------------------------------------------------------------------------+
|MrDTM::_LoadAllNodeData
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMesh<POINT>::_LoadAllNodeData(size_t& nbLoadedNodes, int level) const
{
    m_scmIndexPtr->LoadIndexNodes(nbLoadedNodes, level, false);
    return SUCCESS;
}

/*----------------------------------------------------------------------------+
|MrDTM::_GroupNodeHeaders
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMesh<POINT>::_SaveGroupedNodeHeaders(const WString& pi_pOutputDirPath, const short& pi_pGroupMode) const
    {
    if (m_scmIndexPtr == nullptr) return ERROR;
    if (m_smSQLitePtr->IsSingleFile()) return ERROR;

    //s_stream_from_disk = true;
 #ifndef LINUX_SCALABLEMESH_BUILD       
    s_stream_from_grouped_store = false;
    s_is_virtual_grouping = pi_pGroupMode == SMGroupGlobalParameters::VIRTUAL;

    //m_smSQLitePtr->SetVirtualGroups(pi_pGroupMode == SMGroupGlobalParameters::VIRTUAL);
    m_scmIndexPtr->SaveGroupedNodeHeaders(this->GetDataSourceAccount(), pi_pOutputDirPath, pi_pGroupMode, true);
#endif
    return SUCCESS;
    }
#endif  
 
template <class POINT> void ScalableMesh<POINT>::_ReFilter()
    {
    size_t depth = m_scmIndexPtr->GetDepth();
    for (int level = (int)depth-1; level >= 0; level--)
        {
        m_scmIndexPtr->Filter(level);
        }
    m_scmIndexPtr = 0;
    }
/*----------------------------------------------------------------------------+
|ScalableMeshSingleResolutionPointIndexView Method Definition Section - Begin
+----------------------------------------------------------------------------*/
template <class POINT> ScalableMeshSingleResolutionPointIndexView<POINT>::ScalableMeshSingleResolutionPointIndexView(HFCPtr<SMPointIndex<POINT, Extent3dType>> scmPointIndexPtr, 
                                                                                                       int                                              resolutionIndex, 
                                                                                                       GeoCoords::GCS                                   sourceGCS)
: m_sourceGCS(sourceGCS)
    {    
    m_scmIndexPtr = scmPointIndexPtr;
    m_resolutionIndex    = resolutionIndex;    
    }

template <class POINT> ScalableMeshSingleResolutionPointIndexView<POINT>::~ScalableMeshSingleResolutionPointIndexView()
    {
    } 

template <class POINT> void ScalableMeshSingleResolutionPointIndexView<POINT>::_TextureFromRaster(ITextureProviderPtr provider)
    {}

// Inherited from IDTM   
template <class POINT> int64_t ScalableMeshSingleResolutionPointIndexView<POINT>::_GetPointCount()
    {
    return m_scmIndexPtr->GetNbObjectsAtLevel(m_resolutionIndex);
    }

template <class POINT> uint64_t ScalableMeshSingleResolutionPointIndexView<POINT>::_GetNodeCount()
    {
    size_t numNodes = 0;
    m_scmIndexPtr->LoadIndexNodes(numNodes, m_resolutionIndex, true);
    return numNodes;
    }

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_IsTerrain()
    {
    return false;
    }

template <class POINT> int64_t ScalableMeshSingleResolutionPointIndexView<POINT>::_GetBreaklineCount() const
    {
    assert(0);
    return 0;
    }


template <class POINT> BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* ScalableMeshSingleResolutionPointIndexView<POINT>::_GetDTMInterface(DTMAnalysisType type)
    {
    assert(0);
    return 0;
    }

template <class POINT> BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* ScalableMeshSingleResolutionPointIndexView<POINT>::_GetDTMInterface(DMatrix4d& storageToUors, DTMAnalysisType type)
    {
    assert(0);
    return 0;
    }

template <class POINT> BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* ScalableMeshSingleResolutionPointIndexView<POINT>::_GetDTMInterface(DMatrix4d& storageToUors, bvector<DPoint3d>& regionPoints, DTMAnalysisType type)
{
    assert(0);
    return 0;
}



template <class POINT> DTMStatusInt ScalableMeshSingleResolutionPointIndexView<POINT>::_GetRange(DRange3dR range)
    {        
    Extent3dType ExtentPoints = m_scmIndexPtr->GetContentExtent();

    range.low.x = ExtentOp<Extent3dType>::GetXMin(ExtentPoints);
    range.high.x = ExtentOp<Extent3dType>::GetXMax(ExtentPoints);
    range.low.y = ExtentOp<Extent3dType>::GetYMin(ExtentPoints);
    range.high.y = ExtentOp<Extent3dType>::GetYMax(ExtentPoints);
    range.low.z = ExtentOp<Extent3dType>::GetZMin(ExtentPoints);
    range.high.z = ExtentOp<Extent3dType>::GetZMax(ExtentPoints);   
    return DTM_SUCCESS;
    }



template <class POINT> StatusInt ScalableMeshSingleResolutionPointIndexView<POINT>::_GetBoundary(bvector<DPoint3d>& boundary)
    {
    return ERROR;
    }

template <class POINT> uint64_t ScalableMeshSingleResolutionPointIndexView<POINT>::_AddClip(const DPoint3d* pts, size_t ptsSize)
    {
    assert(false && "Not implemented");
    return ((uint64_t)-1);
    }

/*----------------------------------------------------------------------------+
|ScalableMeshSingleResolutionPointIndexView::_GetRangeInSpecificGCS
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMeshSingleResolutionPointIndexView<POINT>::_GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const
    {
    StatusInt status = SUCCESS;

    Extent3dType ExtentPoints = m_scmIndexPtr->GetContentExtent();

    DRange3d initialRange;
    initialRange.low.x = ExtentOp<Extent3dType>::GetXMin(ExtentPoints);
    initialRange.high.x = ExtentOp<Extent3dType>::GetXMax(ExtentPoints);
    initialRange.low.y = ExtentOp<Extent3dType>::GetYMin(ExtentPoints);
    initialRange.high.y = ExtentOp<Extent3dType>::GetYMax(ExtentPoints);
    initialRange.low.z = ExtentOp<Extent3dType>::GetZMin(ExtentPoints);
    initialRange.high.z = ExtentOp<Extent3dType>::GetZMax(ExtentPoints);

    DRange3d reprojectedRange;
    status = ReprojectRangeDomainLimited(reprojectedRange, initialRange, const_cast<BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr&>(m_sourceGCS.GetGeoRef().GetBasePtr()), targetGCS);

    lowPt.x = reprojectedRange.low.x;
    highPt.x = reprojectedRange.high.x;
    lowPt.y = reprojectedRange.low.y;
    highPt.y = reprojectedRange.high.y;
    lowPt.z = reprojectedRange.low.z;
    highPt.z = reprojectedRange.high.z;


    return status;    
    }

// Inherited from IMRDTM     
template <class POINT> Count ScalableMeshSingleResolutionPointIndexView<POINT>::_GetCountInRange (const DRange2d& range, 
                                                                                           const CountType& type, 
                                                                                           const uint64_t& maxNumberCountedPoints) const
    {
    // Not implemented
    assert(false);
    return Count(0,0);
    }

template <class POINT> int ScalableMeshSingleResolutionPointIndexView<POINT>::_GenerateSubResolutions()
    {  
    assert(0);
    return ERROR;
    }

template <class POINT> ScalableMeshCompressionType ScalableMeshSingleResolutionPointIndexView<POINT>::_GetCompressionType() const
    {
    assert(0);
    return SCM_COMPRESSION_NONE;
    }

template <class POINT> int ScalableMeshSingleResolutionPointIndexView<POINT>::_GetNbResolutions() const
    {
    return 1;    
    }

template <class POINT> size_t ScalableMeshSingleResolutionPointIndexView<POINT>::_GetTerrainDepth() const
    {
    return 1;
    }

template <class POINT> IScalableMeshPointQueryPtr ScalableMeshSingleResolutionPointIndexView<POINT>::_GetQueryInterface(ScalableMeshQueryType                queryType,                                                                                                      
                                                                                                                        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS,
                                                                                                                        const DRange3d&                      extentInTargetGCS) const
    {                   
    GeoCoords::GCS targetGeoCoord(GetGCSFactory().Create(targetGCS));           
   
    // TODO: Remove once smart pointers are const aware
    ScalableMeshSingleResolutionPointIndexView<POINT>* UNCONST_THIS = const_cast<ScalableMeshSingleResolutionPointIndexView<POINT>*>(this);

    IScalableMeshPointQueryPtr scmQueryPtr = ScalableMeshPointQuery::GetReprojectionQueryInterface(UNCONST_THIS, queryType, m_sourceGCS, targetGeoCoord, extentInTargetGCS);

    return scmQueryPtr;                
    }

template <class POINT> IScalableMeshPointQueryPtr ScalableMeshSingleResolutionPointIndexView<POINT>::_GetQueryInterface(ScalableMeshQueryType queryType) const
    {    
    IScalableMeshPointQueryPtr iScalableMeshQueryPtr;

    if ((m_scmIndexPtr != 0) && (m_scmIndexPtr->IsEmpty() == false))
        {    
        if (queryType == SCM_QUERY_FULL_RESOLUTION)
            {
            iScalableMeshQueryPtr =  new ScalableMeshFullResolutionPointQuery<POINT>(m_scmIndexPtr, 
                                                                                     m_resolutionIndex);
            }
        else
        if (queryType == SCM_QUERY_VIEW_DEPENDENT)
            {
            iScalableMeshQueryPtr =  new ScalableMeshViewDependentPointQuery<POINT>(m_scmIndexPtr);       
            }

        //SCM_QUERY_FIX_RESOLUTION_VIEW is meaningless for a single resolution view.         
        }
        
    return iScalableMeshQueryPtr;
    }
  
template <class POINT> IScalableMeshMeshQueryPtr ScalableMeshSingleResolutionPointIndexView<POINT>::_GetMeshQueryInterface(MeshQueryType queryType) const
    {
    assert(!"Not supported");

    return IScalableMeshMeshQueryPtr();
    }

template <class POINT> IScalableMeshMeshQueryPtr ScalableMeshSingleResolutionPointIndexView<POINT>::_GetMeshQueryInterface(MeshQueryType queryType,
                                                                                                             BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS,
                                                                                                             const DRange3d&                      extentInTargetGCS) const
    {
    assert(!"Not supported");

    return IScalableMeshMeshQueryPtr();
    }

template <class POINT> IScalableMeshNodeRayQueryPtr ScalableMeshSingleResolutionPointIndexView<POINT>::_GetNodeQueryInterface() const
    {
    assert(!"Not supported");

    return IScalableMeshNodeRayQueryPtr();
    }

template <class POINT> const GeoCoords::GCS& ScalableMeshSingleResolutionPointIndexView<POINT>::_GetGCS() const
    {
    return m_sourceGCS;
    }

template <class POINT> StatusInt ScalableMeshSingleResolutionPointIndexView<POINT>::_SetGCS(const GeoCoords::GCS& sourceGCS)
    {
    assert(!"Not done yet");

    return -1;   
    }

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, bool alsoAddOnTerrain)
    {
    return false;
    }

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID)
    {
    return false;
    }

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_RemoveClip(uint64_t clipID)
    {
    return false;
    }


template <class POINT> void ScalableMeshSingleResolutionPointIndexView<POINT>::_ModifyClipMetadata(uint64_t clipId, double importance, int nDimensions)
    {
    assert(0);
    return;
    }

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t clipID, bool alsoAddOnTerrain)
    {
    return false;
    }

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t clipID)
    {
    return false;
    }

template <class POINT> void ScalableMeshSingleResolutionPointIndexView<POINT>::_GetAllClipsIds(bvector<uint64_t>& allClipIds)
    {

    }

template <class POINT> void ScalableMeshSingleResolutionPointIndexView<POINT>::_GetCurrentlyViewedNodes(bvector<IScalableMeshNodePtr>& nodes)
    {

    }

template <class POINT> void ScalableMeshSingleResolutionPointIndexView<POINT>::_SetCurrentlyViewedNodes(const bvector<IScalableMeshNodePtr>& nodes)
    {

    }

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_RemoveSkirt(uint64_t clipID)
    {
    return false;
    }



template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_IsReadOnly() const
    {    
    //MS Access of the file 
    assert(!"Not done yet");
    return true;
    }

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_IsShareable() const
    {    
    //MS Access of the file 
    assert(!"Not done yet");
    return false;
    }

template <class POINT> ScalableMeshState ScalableMeshSingleResolutionPointIndexView<POINT>::_GetState() const
    {
    assert(!"Not done yet");
    return SCM_STATE_EMPTY; 
    }

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_InSynchWithSources() const
    {    
    assert(!"Should not be called");
    return false;
    }

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_LastSynchronizationCheck(time_t& lastCheckTime) const
    {
    assert(!"Should not be called");
    return false;       
    }

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_IsInsertingClips()
    {
    assert(!"Should not be called");
    return false;
    }

template <class POINT> void ScalableMeshSingleResolutionPointIndexView<POINT>::_SetIsInsertingClips(bool toggleInsertClips)
    {
    assert(!"Should not be called");
    }

/*----------------------------------------------------------------------------+
|ScalableMeshSingleResolutionPointIndexView Method Definition Section - End
+----------------------------------------------------------------------------*/
/*
DTMStatusInt IDTMVolume::ComputeVolumeCutAndFill(double& cut, double& fill, double& area, PolyfaceHeader& intersectingMeshSurface, DRange3d& meshRange, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
    {
    return _ComputeVolumeCutAndFill(cut, fill, area, intersectingMeshSurface,meshRange, volumeMeshVector);
    }

DTMStatusInt IDTMVolume::ComputeVolumeCutAndFill(PolyfaceHeaderPtr& terrainMesh, double& cut, double& fill, PolyfaceHeader& mesh, bool is2d, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
    {
    return _ComputeVolumeCutAndFill(terrainMesh, cut, fill, mesh, is2d, volumeMeshVector);
    }
*/


void edgeCollapseTest(WCharCP param)
    {
#if 0
    FILE* mesh = _wfopen(param, L"rb");

    size_t ct;
    fread(&ct, sizeof(size_t), 1, mesh);
    void* graph = new byte[ct];
    fread(graph, 1, ct, mesh);
    MTGGraph g;
    g.LoadFromBinaryStream(graph, ct);
    size_t npts;
    fread(&npts, sizeof(size_t), 1, mesh);
    std::vector<DPoint3d> pts(npts);
    fread(&pts[0], sizeof(DPoint3d), npts, mesh);
    //CGALEdgeCollapse(&g, pts, 20000000);
    fclose(mesh);
#endif
    }

void edgeCollapsePrintGraph(WCharCP param)
    {
#if 0
    FILE* mesh = _wfopen(param, L"rb");

    size_t ct;
    fread(&ct, sizeof(size_t), 1, mesh);
    void* graph = new byte[ct];
    fread(graph, 1, ct, mesh);
    MTGGraph g;
    g.LoadFromBinaryStream(graph, ct);
    size_t npts;
    fread(&npts, sizeof(size_t), 1, mesh);
    std::vector<DPoint3d> pts(npts);
    fread(&pts[0], sizeof(DPoint3d), npts, mesh);
    fclose(mesh);
    Utf8String path = "E:\\output\\scmesh\\2016-01-28\\";
    Utf8String str1 = "tested";
    PrintGraphWithPointInfo(path, str1, &g, &pts[0], npts);
#endif
    }

void edgeCollapseShowMesh(WCharCP param, PolyfaceQueryP& outMesh)
    {
#if 0
    FILE* mesh = _wfopen(param, L"rb");
    size_t ct;
    fread(&ct, sizeof(size_t), 1, mesh);
    void* graph = new byte[ct];
    fread(graph, 1, ct, mesh);
    MTGGraph g;
    g.LoadFromBinaryStream(graph, ct);
    size_t npts;
    fread(&npts, sizeof(size_t), 1, mesh);
    DPoint3d* pts = new DPoint3d[npts];
    fread(pts, sizeof(DPoint3d), npts, mesh);
    fclose(mesh);
    std::vector<int32_t> indices;
    ExtractMeshIndicesFromGraph(indices, &g);
    int32_t* indicesArray = new int32_t[indices.size()];
    memcpy(indicesArray, &indices[0], indices.size()*sizeof(int32_t));
    for (size_t i = 0; i < npts; ++i) pts[i].Scale(10000);
    outMesh = new PolyfaceQueryCarrier(3, false, indices.size(), npts, pts,indicesArray);
#endif
    }

void AddLoopsFromShape(bvector<bvector<DPoint3d>>& polygons, const HGF2DShape* shape, std::function<void(const bvector<DPoint3d>& element)> afterPolygonAdded)
{

    if (shape->IsComplex())
    {
        for (auto& elem : shape->GetShapeList())
        {
            AddLoopsFromShape(polygons, elem, afterPolygonAdded);
        }
    }
    else if (!shape->IsEmpty())
    {
        HGF2DPositionCollection thePoints;
        shape->Drop(&thePoints, shape->GetTolerance());

        bvector<DPoint3d> vec(thePoints.size());

        for (size_t idx = 0; idx < thePoints.size(); idx++)
        {
            vec[idx].x = thePoints[idx].GetX();
            vec[idx].y = thePoints[idx].GetY();
            vec[idx].z = 0; // As mentionned below the Z is disregarded
        }

        polygons.push_back(vec);
        afterPolygonAdded(vec);
    }
}

void MergePolygonSets(bvector<bvector<DPoint3d>>& polygons)
{
    return MergePolygonSets(polygons, [](const size_t i, const bvector<DPoint3d>& element)
    {
        return true;
    }, [](const bvector<DPoint3d>&element) {});
}

void MergePolygonSets(bvector<bvector<DPoint3d>>& polygons, std::function<bool(const size_t i, const bvector<DPoint3d>& element)> choosePolygonInSet, std::function<void(const bvector<DPoint3d>& element)> afterPolygonAdded)
{
    bvector<bvector<DPoint3d>> newUnifiedPoly;
    HFCPtr<HGF2DCoordSys>   coordSysPtr(new HGF2DCoordSys());
    HFCPtr<HVEShape> allPolyShape = new HVEShape(coordSysPtr);
    bvector<bool> used(polygons.size(), false);

    bvector<bool> available(polygons.size(), false);
    for (auto& poly : polygons)
    {
        available[&poly - &polygons.front()] = choosePolygonInSet(&poly - &polygons.front(), poly);
    }

    //Apparently, intersection on a single vertex, even though it has no bearing on the "inside" section of voids, trips up the Civil triangulation.
    //So we find out and disconnect single vertex intersections first, since they cannot be unified.
    for (auto& poly : polygons)
    {
        if (!available[&poly - &polygons.front()]) continue;
        DRange3d range = DRange3d::From(poly);
        if (poly.empty()) continue;
        bvector<DPoint3d> poly_2d = poly;
        for (auto&pt : poly_2d) pt.z = 0;
        for (auto& poly2 : polygons)
        {
            if (!available[&poly2 - &polygons.front()]) continue;
            if (&poly == &poly2) continue;
            if (poly2.empty()) continue;
            if (!DRange3d::From(poly2).IntersectsWith(range)) continue;
            bvector<DPoint3d> poly2_2d = poly2;
            for (auto&pt : poly2_2d) pt.z = 0;

            //There are cases where the clash functions on non-coplanar 3d polygons says 2 polygons which share a vertex don't clash.
            if (bsiDPoint3dArray_polygonClashXYZ(&poly.front(), (int)poly.size(), &poly2.front(), (int)poly2.size()) ||
                bsiDPoint3dArray_polygonClashXYZ(&poly_2d.front(), (int)poly_2d.size(), &poly2_2d.front(), (int)poly2_2d.size()))
            {
                VuPolygonClassifier vu(1e-8, 0);
                vu.ClassifyAUnionB(poly, poly2);
                bvector<DPoint3d> xyz;
                bvector<bvector<DPoint3d>> faces;
                for (; vu.GetFace(xyz);)
                {
                    if (bsiGeom_getXYPolygonArea(&xyz[0], (int)xyz.size()) < 0) continue;
                    else
                    {
                        //  postFeatureBoundary.push_back(xyz);
                        faces.push_back(xyz);

                    }

                }
                if (faces.size() == 1)
                    continue;
                //compute intersects on single vertices
                bmap<DPoint3d, size_t, DPoint3dZYXTolerancedSortComparison> setOfPts(DPoint3dZYXTolerancedSortComparison(1e-8, 0));
                bvector<DPoint3d> intersectingVertices;
                bvector<bpair<bpair<DSegment3d, DSegment3d>, bpair<DSegment3d, DSegment3d>>> intersectingContext;
                int minConsecutiveIntersectingVertices = INT_MAX;
                int consecutiveIntersectingVertices = 0;
                int nPtsSeen = 0;
                int loopNPts = 0;
                for (auto pt : poly)
                {
                    pt.z = 0;
                    setOfPts.insert(make_bpair(pt, &pt - &poly[0]));
                }
                for (auto& pt : poly2)
                {
                    ++nPtsSeen;
                    DPoint3d pt2d = pt;
                    pt2d.z = 0;
                    if (setOfPts.count(pt2d))
                    {
                        DSegment3d lastSegOn1, nextSegOn1, lastSegOn2, nextSegOn2;
                        lastSegOn1 = DSegment3d::From(setOfPts[pt2d] == 0 ? poly[poly.size() - 2] : poly[setOfPts[pt2d] - 1], poly[setOfPts[pt2d]]);
                        nextSegOn1 = DSegment3d::From(poly[setOfPts[pt2d]], setOfPts[pt2d] == poly.size() - 1 ? poly[1] : poly[setOfPts[pt2d] + 1]);
                        lastSegOn2 = DSegment3d::From(nPtsSeen == 1 ? poly2[poly2.size() - 2] : poly2[nPtsSeen - 2], poly2[nPtsSeen - 1]);
                        nextSegOn2 = DSegment3d::From(poly2[nPtsSeen - 1], nPtsSeen == poly2.size() ? poly2[1] : poly2[nPtsSeen]);

                        intersectingVertices.push_back(pt);
                        intersectingContext.push_back(make_bpair(make_bpair(lastSegOn1, nextSegOn1), make_bpair(lastSegOn2, nextSegOn2)));
                        consecutiveIntersectingVertices++;
                    }
                    else
                    {
                        if (nPtsSeen == 2)
                        {
                            loopNPts = consecutiveIntersectingVertices;
                        }
                        else if (nPtsSeen != 2 && consecutiveIntersectingVertices > 0)
                            minConsecutiveIntersectingVertices = std::min(consecutiveIntersectingVertices, minConsecutiveIntersectingVertices);
                        if (consecutiveIntersectingVertices > 1)
                        {
                            intersectingVertices.resize(intersectingVertices.size() - consecutiveIntersectingVertices);
                            intersectingContext.resize(intersectingContext.size() - consecutiveIntersectingVertices);
                        }
                        consecutiveIntersectingVertices = 0;
                    }
                }

                if (loopNPts != 0)
                {
                    consecutiveIntersectingVertices += loopNPts - 1;
                    if (consecutiveIntersectingVertices > 0)
                        minConsecutiveIntersectingVertices = std::min(consecutiveIntersectingVertices, minConsecutiveIntersectingVertices);
                    consecutiveIntersectingVertices = 0;
                }

                //No single vertex intersection
                if (minConsecutiveIntersectingVertices > 1) continue;
                if (!intersectingVertices.empty())
                {
                    size_t nColinear = 0;
                    for (size_t i = 0; i < intersectingVertices.size(); ++i)
                    {
                        std::vector<DPoint3d> pts = { intersectingContext[i].first.first.point[0],intersectingContext[i].first.first.point[1], intersectingContext[i].second.first.point[0] };
                        if (bsiGeom_isDPoint3dArrayColinear(pts.data(), (int)pts.size(), 1e-8))
                        {
                            nColinear++;
                            continue;
                        }
                        pts = { intersectingContext[i].first.first.point[0],intersectingContext[i].first.first.point[1], intersectingContext[i].second.second.point[1] };
                        if (bsiGeom_isDPoint3dArrayColinear(pts.data(), (int)pts.size(), 1e-8))
                        {
                            nColinear++;
                            continue;
                        }
                        pts = { intersectingContext[i].first.second.point[0],intersectingContext[i].first.second.point[1], intersectingContext[i].second.first.point[0] };
                        if (bsiGeom_isDPoint3dArrayColinear(pts.data(), (int)pts.size(), 1e-8))
                        {
                            nColinear++;
                            continue;
                        }
                        pts = { intersectingContext[i].first.second.point[0],intersectingContext[i].first.second.point[1], intersectingContext[i].second.second.point[1] };
                        if (bsiGeom_isDPoint3dArrayColinear(pts.data(), (int)pts.size(), 1e-8))
                        {
                            nColinear++;
                            continue;
                        }

                    }
                    if (nColinear == intersectingVertices.size())
                        continue;
                    bvector<DPoint3d> withoutIntersect;
                    if (poly.size() < poly2.size())
                    {
                        for (auto& pt : poly)
                        {
                            bool insert = true;
                            for (auto& ptB : intersectingVertices)
                                if (pt.IsEqual (ptB, 1e-8)) insert = false;
                            if (insert) withoutIntersect.push_back(pt);
                        }
                    }
                    else
                    {
                        for (auto& pt : poly2)
                        {
                            bool insert = true;
                            for (auto& ptB : intersectingVertices)
                                if (pt.IsEqual (ptB, 1e-8)) insert = false;
                            if (insert) withoutIntersect.push_back(pt);
                        }
                    }
                    if (poly.size() < poly2.size())
                    {
                        poly = poly2;
                        poly_2d = poly2_2d;
                        range = DRange3d::From(poly);
                    }
                    if (!withoutIntersect.empty() && !withoutIntersect.front().IsEqual (*(&withoutIntersect.back()), 1e-8)) withoutIntersect.push_back(withoutIntersect.front());
                    if (withoutIntersect.size() > 4)
                    {
                        poly2 = withoutIntersect;
                    }
                    else poly2.clear();

                }
            }
        }
    }

    for (auto& poly : polygons)
    {
        if (!available[&poly - &polygons.front()]) continue;
        if (used[&poly - &polygons[0]]) continue;
        if (poly.empty()) continue;
        bvector<DPoint3d> poly_2d = poly;
        for (auto&pt : poly_2d) pt.z = 0;

        //pre-compute the union of polys with this function because apparently sometimes Unify hangs
        for (auto& poly2 : polygons)
        {
            if (!available[&poly2 - &polygons.front()]) continue;
            if (&poly == &poly2) continue;
            if (poly2.empty()) continue;
            if (used[&poly2 - &polygons[0]]) continue;

            bvector<DPoint3d> poly2_2d = poly2;
            for (auto&pt : poly2_2d) pt.z = 0;

            if (bsiDPoint3dArray_polygonClashXYZ(&poly.front(), (int)poly.size(), &poly2.front(), (int)poly2.size())
                || bsiDPoint3dArray_polygonClashXYZ(&poly_2d.front(), (int)poly_2d.size(), &poly2_2d.front(), (int)poly2_2d.size()))
            {
                VuPolygonClassifier vu(1e-8, 0);
                vu.ClassifyAUnionB(poly, poly2);
                bvector<DPoint3d> xyz;
                bvector<bvector<DPoint3d>> faces;
                for (; vu.GetFace(xyz);)
                {
                    if (bsiGeom_getXYPolygonArea(&xyz[0], (int)xyz.size()) < 0) continue;
                    else
                    {
                        //  postFeatureBoundary.push_back(xyz);
                        faces.push_back(xyz);

                    }

                }
                if (faces.size() == 1)
                {
                    poly = faces.front();
                    used[&poly2 - &polygons[0]] = true;
                }
                /*	else
                {
                //compute intersects on vertices
                bset<DPoint3d, DPoint3dZYXTolerancedSortComparison> setOfPts(DPoint3dZYXTolerancedSortComparison(1e-8, 0));
                bvector<DPoint3d> intersectingVertices;
                for (auto& pt : poly)
                setOfPts.insert(pt);
                for (auto& pt : poly2)
                if (setOfPts.count(pt))
                intersectingVertices.push_back(pt);
                if (!intersectingVertices.empty())
                {
                bvector<DPoint3d> withoutIntersect;
                if (poly.size() < poly2.size())
                {
                for (auto& pt : poly)
                {
                bool insert = true;
                for (auto& ptB : intersectingVertices)
                if (pt.IsEqual (ptB, 1e-8)) insert = false;
                if(insert) withoutIntersect.push_back(pt);
                }
                }
                else
                {
                for (auto& pt : poly2)
                {
                bool insert = true;
                for (auto& ptB : intersectingVertices)
                if (pt.IsEqual (ptB, 1e-8)) insert = false;
                if (insert) withoutIntersect.push_back(pt);
                }
                }
                if (poly.size() < poly2.size()) poly = poly2;

                if (!withoutIntersect.empty() && !withoutIntersect.front().IsEqual (*(&withoutIntersect.back()), 1e-8)) withoutIntersect.push_back(withoutIntersect.front());
                if (withoutIntersect.size() > 4)
                {
                poly2 = withoutIntersect;
                }
                else used[&poly2 - &polygons[0]] = true;
                }
                }*/
            }

        }
    }

    //keep unique resulting polygons
    for (auto& poly : polygons)
    {
        if (!available[&poly - &polygons.front()]) continue;
        if (used[&poly - &polygons[0]]) continue;
        if (poly.empty()) continue;
        for (auto& poly2 : polygons)
        {
            if (!available[&poly2 - &polygons.front()]) continue;
            if (&poly == &poly2) continue;
            if (used[&poly2 - &polygons[0]]) continue;
            if (poly2.empty()) continue;

            if (poly2.size() != poly.size()) continue;
            size_t i = 0;
            for (i = 0; i < poly.size(); ++i)
                if (!poly[i].IsEqual (poly2[i], 1e-8))
                    break;
            if (i == poly.size())
                used[&poly2 - &polygons[0]] = true;
        }
    }

    for (auto& poly : polygons)
    {
        if (!available[&poly - &polygons.front()]) continue;
        if (used[&poly - &polygons[0]]) continue;
        if (poly.empty()) continue;

        UntieLoopsFromPolygon(poly);
        if (poly.size() < 3)
            continue;
        HArrayAutoPtr<double> tempBuffer(new double[poly.size() * 2]);

        int bufferInd = 0;

        for (size_t pointInd = 0; pointInd < poly.size(); pointInd++)
        {
            tempBuffer[bufferInd * 2] = poly[pointInd].x;
            tempBuffer[bufferInd * 2 + 1] = poly[pointInd].y;
            bufferInd++;
        }
        HVE2DPolygonOfSegments polygon(poly.size() * 2, tempBuffer, coordSysPtr);

        HFCPtr<HVEShape> subShapePtr = new HVEShape(polygon);
        allPolyShape->Unify(*subShapePtr);
    }

    AddLoopsFromShape(newUnifiedPoly, allPolyShape->GetLightShape(), afterPolygonAdded);
    polygons = newUnifiedPoly;
}

void IScalableMeshMemoryCounts::SetMaximumMemoryUsage(size_t maxNumberOfBytes)
{
    SMMemoryPool::GetInstance()->SetMaxSize(maxNumberOfBytes);
}

void IScalableMeshMemoryCounts::SetMaximumVideoMemoryUsage(size_t maxNumberOfBytes)
{
    SMMemoryPool::GetInstanceVideo()->SetMaxSize(maxNumberOfBytes);
}

size_t IScalableMeshMemoryCounts::GetAmountOfUsedMemory()
{
    return SMMemoryPool::GetInstance()->GetCurrentlyUsed();
}

size_t IScalableMeshMemoryCounts::GetAmountOfUsedVideoMemory()
{
    return SMMemoryPool::GetInstanceVideo()->GetCurrentlyUsed();
}

size_t IScalableMeshMemoryCounts::GetMaximumMemoryUsage()
{
    return SMMemoryPool::GetInstance()->GetMaxSize();
}

size_t IScalableMeshMemoryCounts::GetMaximumVideoMemoryUsage()
{
    return SMMemoryPool::GetInstanceVideo()->GetMaxSize();
}

template class ScalableMesh<DPoint3d>;
template class ScalableMeshSingleResolutionPointIndexView<DPoint3d>;

END_BENTLEY_SCALABLEMESH_NAMESPACE
