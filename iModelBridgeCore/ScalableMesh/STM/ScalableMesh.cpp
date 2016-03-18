/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMesh.cpp $
|    $RCSfile: ScalableMesh.cpp,v $
|   $Revision: 1.106 $
|       $Date: 2012/01/06 16:30:15 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
  
#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
extern bool   GET_HIGHEST_RES;

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include "InternalUtilityFunctions.h"
#include "ScalableMesh.h"
//#include "DTMGraphTileStore.h"
//#include "SMTextureTileStore.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include "ScalableMeshQuadTreeQueries.h"

#include <ScalableMesh/GeoCoords/GCS.h>
#include <STMInternal/GeoCoords/WKTUtils.h>

#include "ScalableMeshQuery.h"
#include "ScalableMeshSourcesPersistance.h"

#include <ScalableMesh/IScalableMeshPolicy.h>
#include <ScalableMesh\IScalableMeshSourceCollection.h>
#include <ScalableMesh\IScalableMeshDocumentEnv.h>
#include <ScalableMesh\IScalableMeshSourceImportConfig.h>
#include <ScalableMesh\IScalableMeshSources.h>

#include "ScalableMeshDraping.h"
#include "ScalableMeshVolume.h"

//#include "Edits\DiffSetTileStore.h"
#include "SMSQLiteDiffsetTileStore.h"
#include "Edits\ClipRegistry.h"
#include "SMSQLitePointTileStore.h"
#include "SMSQLiteGraphTileStore.h"
#include "SMSQLiteIndiceTileStore.h"
#include "SMSQLiteUVStore.h"
#include "SMSQLiteUVIndiceTileStore.h"
#include "SMSQLiteTextureTileStore.h"
#include <Vu\VuApi.h>
#include <Vu\vupoly.fdf>
//#include "CGALEdgeCollapse.h"



ScalableMeshScheduler* s_clipScheduler = nullptr;
std::mutex s_schedulerLock;
/*
#include <msoutput.fdf>
#include <mstypes.h>
*/
/*------------------------------------------------------------------+
| Include COGO definitions                                          |
+------------------------------------------------------------------*/

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

using namespace IDTMFile;

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
#ifdef SM_BESQL_FORMAT
bool s_useSQLFormat = true;
#else
bool s_useSQLFormat = false;
#endif
#define USE_CODE_FOR_ERROR_DETECTION



namespace {

/*----------------------------------------------------------------------------+
| Pool Singleton
+----------------------------------------------------------------------------*/
template <typename POINT> static HFCPtr<HPMCountLimitedPool<POINT> > PoolSingleton()
    {
    static HFCPtr<HPMCountLimitedPool<POINT> > pGlobalPointPool(new HPMCountLimitedPool<POINT>(new HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment(100, 2000 * sizeof(POINT)), 30000000));

    return pGlobalPointPool;
    }

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
_int64 IScalableMesh::GetPointCount()
    {
    return _GetPointCount();
    }

DTMStatusInt IScalableMesh::GetRange(DRange3dR range)
    {
    return _GetRange(range);
    }

int IScalableMesh::GenerateSubResolutions()
    {
    return _GenerateSubResolutions();
    }
            
__int64 IScalableMesh::GetBreaklineCount() const
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

BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* IScalableMesh::GetDTMInterface()
    {
    return _GetDTMInterface();
    }

BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* IScalableMesh::GetDTMInterface(DMatrix4d& storageToUors)
    {
    return _GetDTMInterface(storageToUors);
    }

const BaseGCSCPtr& IScalableMesh::GetBaseGCS() const 
    {
    return _GetGCS().GetGeoRef().GetBasePtr();
    }

StatusInt IScalableMesh::SetBaseGCS(const BaseGCSCPtr& baseGCSPtr)
    {
    GCSFactory::Status createStatus = GCSFactory::S_SUCCESS;
    GCS gcs(GetGCSFactory().Create(baseGCSPtr, createStatus));
    if (GCSFactory::S_SUCCESS != createStatus)
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

bool IScalableMesh::IsProgressive() const
    {
    return _IsProgressive();
    }

bool IScalableMesh::IsReadOnly() const
    {
    return _IsReadOnly();
    }

bool IScalableMesh::IsShareable() const
    {
    return _IsShareable();
    }

bool IScalableMesh::InSynchWithSources() const
    {
    return _InSynchWithSources();
    }

bool IScalableMesh::LastSynchronizationCheck(time_t& lastCheckTime) const
    {
    return _LastSynchronizationCheck(lastCheckTime);
    }

int IScalableMesh::SynchWithSources()
    {
    return _SynchWithSources();
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

bool IScalableMesh::ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID)
    {
    return _ModifyClip(pts, ptsSize, clipID);
    }

void IScalableMesh::ModifyClipMetadata(uint64_t clipId, double importance, int nDimensions)
    {
    return _ModifyClipMetadata(clipId, importance, nDimensions);
    }

bool IScalableMesh::RemoveClip(uint64_t clipID)
    {
    return _RemoveClip(clipID);
    }

void IScalableMesh::SetIsInsertingClips(bool toggleInsertClips)
    {
    return _SetIsInsertingClips(toggleInsertClips);
    }

bool IScalableMesh::AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t clipID)
    {
    return _AddSkirt(skirt, clipID);
    }

bool IScalableMesh::ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t clipID)
    {
    return _ModifySkirt(skirt, clipID);
    }

bool IScalableMesh::RemoveSkirt(uint64_t clipID)
    {
    return _RemoveSkirt(clipID);
    }


#ifdef SCALABLE_MESH_ATP
int IScalableMesh::LoadAllNodeHeaders(size_t& nbLoadedNodes) const
    {
    return _LoadAllNodeHeaders(nbLoadedNodes);
    }
#endif

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
    bool                    openReadOnly,
    bool                    openShareable,
    StatusInt&              status)
{
    status = BSISUCCESS;

    if(0 != _waccess(filePath, 04))
    {
        status = BSISUCCESS;
        return 0; // File not found
    }

    StatusInt openStatus;
    SMSQLiteFilePtr smSQLiteFile(SMSQLiteFile::Open(filePath, openReadOnly, openStatus));
    if(smSQLiteFile == nullptr)
    {
            status = BSIERROR;

        return 0; // Error opening file
    }
    return ScalableMesh<DPoint3d>::Open(smSQLiteFile, filePath, status);
}



/*----------------------------------------------------------------------------+
|IScalableMesh::GetFor
+----------------------------------------------------------------------------*/
IScalableMeshPtr IScalableMesh::GetFor   (const WChar*          filePath,
                            bool                    openReadOnly,
                            bool                    openShareable)
    {
    StatusInt status;
    return GetFor(filePath, openReadOnly, openShareable, status);
    }


/*----------------------------------------------------------------------------+
|IScalableMesh::GetCountInRange
+----------------------------------------------------------------------------*/
Count IScalableMesh::GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const
    {
    return _GetCountInRange(range, type, maxNumberCountedPoints);
    }



/*----------------------------------------------------------------------------+
|ScalableMeshBase::ScalableMeshBase
+----------------------------------------------------------------------------*/

ScalableMeshBase::ScalableMeshBase(SMSQLiteFilePtr& smSQliteFile,
    const WString&             filePath)
    : m_workingLayer(DEFAULT_WORKING_LAYER),
    m_sourceGCS(GetDefaultGCS()),
    m_path(filePath),
    m_smSQLitePtr(smSQliteFile)
{
    memset(&m_contentExtent, 0, sizeof(m_contentExtent));

    HPRECONDITION(smSQliteFile != 0);
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

bool ScalableMeshBase::LoadGCSFrom()
{
    WString wktStr;
    if (m_smSQLitePtr->GetWkt(wktStr)) // if true, ScalableMesh has not Wkt
        return true;

    IDTMFile::WktFlavor fileWktFlavor = GetWKTFlavor(&wktStr, wktStr);
    BaseGCS::WktFlavor  wktFlavor;

    bool result = MapWktFlavorEnum(wktFlavor, fileWktFlavor);

    assert(result);

    GCSFactory::Status gcsCreateStatus;
    GCS gcs(GetGCSFactory().Create(wktStr.c_str(), wktFlavor, gcsCreateStatus));

    if (GCSFactory::S_SUCCESS != gcsCreateStatus)
        return false;

    using std::swap;
    swap(m_sourceGCS, gcs);

    return true;
}


/*----------------------------------------------------------------------------+
|ScalableMesh::ScalableMesh
+----------------------------------------------------------------------------*/

template <class POINT> ScalableMesh<POINT>::ScalableMesh(SMSQLiteFilePtr& smSQLiteFile, const WString& path)
    : ScalableMeshBase(smSQLiteFile, path),
    m_areDataCompressed(false),
    m_computeTileBoundary(false),
    m_minScreenPixelsPerPoint(MEAN_SCREEN_PIXELS_PER_POINT)
    { 
    }


/*----------------------------------------------------------------------------+
|ScalableMesh::~ScalableMesh
+----------------------------------------------------------------------------*/
template <class POINT> ScalableMesh<POINT>::~ScalableMesh()
    {
    Close();
    }

template <class POINT> unsigned __int64 ScalableMesh<POINT>::CountPointsInExtent(YProtPtExtentType& extent, 
                                                                          HFCPtr<SMPointIndexNode<POINT, YProtPtExtentType>> nodePtr,
                                                                          const unsigned __int64& maxNumberCountedPoints) const
    {
    const unsigned __int64 maxCount = maxNumberCountedPoints <= 0 ? std::numeric_limits<unsigned __int64>::max() : maxNumberCountedPoints;

    if(ExtentOp<YProtPtExtentType>::Overlap(nodePtr->GetContentExtent(),extent))
        {
        if(nodePtr->IsLeaf())
            {
            if (ExtentOp<YProtPtExtentType>::Contains2d(nodePtr->GetContentExtent(), extent))
                {
                // Approximate number of points. The points are assumed to be evenly distributed in the tile extent.
                // This could potentially lead to poor performance if the points are concentrated in specific areas of
                // the tile extent.
                double nodeExtentDistanceX = ExtentOp<YProtPtExtentType>::GetWidth(nodePtr->GetContentExtent());
                double nodeExtentDistanceY = ExtentOp<YProtPtExtentType>::GetHeight(nodePtr->GetContentExtent());
                double nodeExtentArea = nodeExtentDistanceX * nodeExtentDistanceY;
                double extentDistanceX = ExtentOp<YProtPtExtentType>::GetWidth(extent);
                double extentDistanceY = ExtentOp<YProtPtExtentType>::GetHeight(extent);
                double extentArea = extentDistanceX * extentDistanceY;
                return unsigned __int64(nodePtr->GetCount()*(extentArea/nodeExtentArea));
                }
            else {
                double nodeExtentDistanceX = ExtentOp<YProtPtExtentType>::GetWidth(nodePtr->GetContentExtent());
                double nodeExtentDistanceY = ExtentOp<YProtPtExtentType>::GetHeight(nodePtr->GetContentExtent());
                double nodeExtentArea = nodeExtentDistanceX * nodeExtentDistanceY;
                // Create intersection extent
                double xAxisValues[] = { ExtentOp<YProtPtExtentType>::GetXMin(nodePtr->GetContentExtent()), ExtentOp<YProtPtExtentType>::GetXMax(nodePtr->GetContentExtent()),
                    ExtentOp<YProtPtExtentType>::GetXMin(extent), ExtentOp<YProtPtExtentType>::GetXMax(extent) };
                vector<double> xAxisValuesV (xAxisValues,xAxisValues+sizeof(xAxisValues)/ sizeof(double)) ;
                sort(xAxisValuesV.begin(),xAxisValuesV.end());
                double yAxisValues[] = { ExtentOp<YProtPtExtentType>::GetYMin(nodePtr->GetContentExtent()), ExtentOp<YProtPtExtentType>::GetYMax(nodePtr->GetContentExtent()), 
                    ExtentOp<YProtPtExtentType>::GetYMin(extent), ExtentOp<YProtPtExtentType>::GetYMax(extent) };
                vector<double> yAxisValuesV (yAxisValues,yAxisValues+sizeof(yAxisValues)/ sizeof(double)) ;
                sort(yAxisValuesV.begin(),yAxisValuesV.end());
                YProtPtExtentType intersectionExtent = ExtentOp<YProtPtExtentType>::Create(xAxisValuesV[1],yAxisValuesV[1],xAxisValuesV[2],yAxisValuesV[2]);
                double intersectionArea = ExtentOp<YProtPtExtentType>::GetWidth(intersectionExtent)*ExtentOp<YProtPtExtentType>::GetHeight(intersectionExtent);
                //return nodePtr->GetCount();
                return unsigned __int64( (intersectionArea != 0 && nodeExtentArea != 0) ? nodePtr->GetCount()*(intersectionArea/nodeExtentArea) : 0);
                }
            }
        else if(nodePtr->GetSubNodeNoSplit() != NULL)
            {
            return CountPointsInExtent(extent,nodePtr->GetSubNodeNoSplit(),maxCount);
            }
        else
            {
            unsigned __int64 count = 0;
            for(size_t nodeIndex = 0; nodeIndex < nodePtr->GetNumberOfSubNodesOnSplit() && count < maxCount; nodeIndex++)
                {
                count += CountPointsInExtent(extent,nodePtr->m_apSubNodes[nodeIndex],maxCount);
                }
            return count;
            }
        }
    else return 0;
    }

template <class POINT> unsigned __int64 ScalableMesh<POINT>::CountLinearsInExtent(YProtFeatureExtentType& extent, unsigned __int64 maxFeatures) const
{
    assert(false && "Not supported!");
    return -1;
};

template <class POINT> Count ScalableMesh<POINT>::_GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const
    {
    Count qty (0,0);
    switch (type)
        {
        case COUNTTYPE_POINTS:
        case COUNTTYPE_BOTH:
            {
            HFCPtr<SMPointIndexNode<POINT, YProtPtExtentType>> nodePtr = m_scmIndexPtr->GetRootNode();
            YProtPtExtentType pointExtent;
            ExtentOp<YProtPtExtentType>::SetXMax(pointExtent, range.high.x);
            ExtentOp<YProtPtExtentType>::SetXMin(pointExtent, range.low.x);
            ExtentOp<YProtPtExtentType>::SetYMax(pointExtent, range.high.y);
            ExtentOp<YProtPtExtentType>::SetYMin(pointExtent, range.low.y);
            ExtentOp<YProtPtExtentType>::SetZMax(pointExtent, 0.0);
            ExtentOp<YProtPtExtentType>::SetZMin(pointExtent, 0.0);
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
IScalableMeshPtr ScalableMesh<POINT>::Open(SMSQLiteFilePtr& smSQLiteFile,
                                    const WString&     filePath,
                                    StatusInt&              status)
{
     ScalableMesh<POINT>* scmPtr = new ScalableMesh<POINT>(smSQLiteFile, filePath);
    IScalableMeshPtr scmP(scmPtr);

    status = scmPtr->Open();
    return (BSISUCCESS == status ? scmP : 0);
}

/*----------------------------------------------------------------------------+
|ScalableMesh::Open
+----------------------------------------------------------------------------*/
static bool s_dropNodes = false;
static bool s_checkHybridNodeState = false;
template <class POINT> int ScalableMesh<POINT>::Open()
    {
    m_scalableMeshDTM = ScalableMeshDTM::Create(this);

    try 
        {
        //m_smSQLitePtr->Open(m_path);
        // If there are no masterHeader => file empty ?
        bool notEmpty = m_smSQLitePtr->HasMasterHeader();
        if (!notEmpty)
            return BSISUCCESS; // File is empty.

        if (!LoadGCSFrom())
            return BSIERROR; // Error loading layer gcs

        bool hasPoints = m_smSQLitePtr->HasPoints(); // NEEDS_WORKS : add AddPOints To DataSQLite, and test if there are somes points in the database.



        HFCPtr<TileStoreType> pTileStore;
        HFCPtr<StreamingStoreType>  pStreamingTileStore;
        HFCPtr<SMStreamingPointTaggedTileStore<int32_t, YProtPtExtentType >> pStreamingIndiceTileStore;
        HFCPtr<SMStreamingPointTaggedTileStore<DPoint2d, YProtPtExtentType >> pStreamingUVTileStore;
        HFCPtr<SMStreamingPointTaggedTileStore<int32_t, YProtPtExtentType >> pStreamingUVsIndicesTileStore;

        HFCPtr<SMPointTileStore<int32_t, YProtPtExtentType >> pIndiceTileStore;
        HFCPtr<SMPointTileStore<DPoint2d, YProtPtExtentType >> pUVTileStore;
        HFCPtr<SMPointTileStore<int32_t, YProtPtExtentType >> pUVsIndicesTileStore;
        HFCPtr<IHPMPermanentStore<Byte, float, float>> pTextureTileStore;
        HFCPtr<IHPMPermanentStore<MTGGraph, Byte, Byte>> pGraphTileStore;
        bool isSingleFile = true;
        

        pTileStore = new SMSQLitePointTileStore<POINT, YProtPtExtentType>(m_smSQLitePtr);
        isSingleFile = m_smSQLitePtr->IsSingleFile();

        if (hasPoints || !isSingleFile)
            {    
         //   HFCPtr<HPMCountLimitedPool<POINT> > pMemoryPool(PoolSingleton<POINT>());

         //NEEDS_WORK_SM - Why correct filter is not saved?                                             
         //auto_ptr<ISMPointIndexFilter<POINT, YProtPtExtentType>> filterP(CreatePointIndexFilter(featureDir));
            auto_ptr<ISMMeshIndexFilter<POINT, YProtPtExtentType>> filterP(new ScalableMeshQuadTreeBCLIBMeshFilter1<POINT, YProtPtExtentType>());

                if (!isSingleFile)
                    {
                    // NEEDS_WORK_SM - Path should not depend on the existence of an stm file
                    auto position = m_path.find_last_of(L".stm");
                    auto filenameWithoutExtension = m_path.substr(0, position - 3);
                    // NEEDS_WORK_SM - Remove hardcoded azure dataset name
                    WString azureDatasetName(L"quebeccity\\");
                    // NEEDS_WORK_SM - Check existence of the following directories
                    WString streamingFilePath = (s_stream_from_disk ? m_path.substr(0, position - 3) + L"_stream\\" : L"");
                    WString groupedStreamingFilePath = (s_stream_from_disk ? filenameWithoutExtension + L"_grouped_stream\\" : azureDatasetName + L"headers\\");
                    WString point_store_path = (s_stream_from_disk ? streamingFilePath + L"point_store\\" : azureDatasetName + L"points\\");
                    WString indice_store_path = (s_stream_from_disk ? streamingFilePath + L"indice_store\\" : azureDatasetName + L"indices\\");
                    WString uv_store_path = streamingFilePath + L"uv_store\\";
                    WString uvIndice_store_path = streamingFilePath + L"uvIndice_store\\";
                    WString texture_store_path = streamingFilePath + L"texture_store\\";

                    // NEEDS_WORK_SM - Need to stream textures as well
                    pStreamingTileStore = new StreamingStoreType(point_store_path, groupedStreamingFilePath, AreDataCompressed());
                    pStreamingIndiceTileStore = new SMStreamingPointTaggedTileStore< int32_t, YProtPtExtentType>(indice_store_path, groupedStreamingFilePath, AreDataCompressed());
                    pStreamingUVTileStore = new SMStreamingPointTaggedTileStore< DPoint2d, YProtPtExtentType>(uv_store_path, groupedStreamingFilePath, AreDataCompressed());
                    pStreamingUVsIndicesTileStore = new SMStreamingPointTaggedTileStore<int32_t, YProtPtExtentType >(uvIndice_store_path, groupedStreamingFilePath, AreDataCompressed());
                    pTextureTileStore = new StreamingTextureTileStore(texture_store_path.c_str(), 4);
                    m_scmIndexPtr = new PointIndexType(ScalableMeshMemoryPools<POINT>::Get()->GetPointPool(),
                                                            &*pStreamingTileStore,
                                                            ScalableMeshMemoryPools<POINT>::Get()->GetPtsIndicePool(),
                                                            &*pStreamingIndiceTileStore,
                                                            ScalableMeshMemoryPools<POINT>::Get()->GetGraphPool(),
                                                            new SMSQLiteGraphTileStore((dynamic_cast<SMSQLitePointTileStore<POINT, YProtPtExtentType>*>(pTileStore.GetPtr()))->GetDbConnection()),

                                                            ScalableMeshMemoryPools<POINT>::Get()->GetTexturePool(),

                                                            pTextureTileStore,

                                                            ScalableMeshMemoryPools<POINT>::Get()->GetUVPool(),
                                                            &*pStreamingUVTileStore,
                                                            ScalableMeshMemoryPools<POINT>::Get()->GetUVsIndicesPool(),
                                                            &*pStreamingUVsIndicesTileStore,
                                                            10000,
                                                            filterP.get(),
                                                            false,
                                                            false,
                                                            false,
                                                            0,
                                                            0);
                    if (s_save_grouped_store && !m_scmIndexPtr->IsEmpty())
                        {

                        auto groupedStreamingFilePath = m_path.substr(0, position - 3) + L"_grouped_stream\\";
                        m_scmIndexPtr->SaveCloudReady(groupedStreamingFilePath, point_store_path);
                        }

                    }
                else
                    {
                    //                    size_t n = 4;

                    pIndiceTileStore = new SMSQLiteIndiceTileStore<YProtPtExtentType >(m_smSQLitePtr);
                    pUVTileStore = new SMSQLiteUVTileStore<YProtPtExtentType >((dynamic_cast<SMSQLitePointTileStore<POINT, YProtPtExtentType>*>(pTileStore.GetPtr()))->GetDbConnection());
                    pUVsIndicesTileStore = new SMSQLiteUVIndiceTileStore<YProtPtExtentType >((dynamic_cast<SMSQLitePointTileStore<POINT, YProtPtExtentType>*>(pTileStore.GetPtr()))->GetDbConnection());
                    pTextureTileStore = new SMSQLiteTextureTileStore((dynamic_cast<SMSQLitePointTileStore<POINT, YProtPtExtentType>*>(pTileStore.GetPtr()))->GetDbConnection());
                    pGraphTileStore = new SMSQLiteGraphTileStore((dynamic_cast<SMSQLitePointTileStore<POINT, YProtPtExtentType>*>(pTileStore.GetPtr()))->GetDbConnection());


                    m_scmIndexPtr = new PointIndexType(//pMemoryPool, 
                                                       ScalableMeshMemoryPools<POINT>::Get()->GetPointPool(),
                                                       &*pTileStore,
                                                       ScalableMeshMemoryPools<POINT>::Get()->GetPtsIndicePool(),
                                                       &*pIndiceTileStore,
                                                       ScalableMeshMemoryPools<POINT>::Get()->GetGraphPool(),
                                                       //new HPMIndirectCountLimitedPool<MTGGraph>(new HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment(100, 2000*sizeof(POINT)), 600000000),
                                                       &*pGraphTileStore,
                                                       ScalableMeshMemoryPools<POINT>::Get()->GetTexturePool(),
                                                       &*pTextureTileStore,
                                                       ScalableMeshMemoryPools<POINT>::Get()->GetUVPool(),
                                                       &*pUVTileStore,
                                                       ScalableMeshMemoryPools<POINT>::Get()->GetUVsIndicesPool(),
                                                       &*pUVsIndicesTileStore,
                                                       10000,
                                                       filterP.get(),
                                                       false,
                                                       false,
                                                       false,
                                                       0,
                                                       0);  //NEEDS_WORK_SM - Should we store mesher information in STM file like filter?
                    }

                m_scmIndexPtr->m_useSTMFormat = false;
            WString clipFilePath = m_path;
            clipFilePath.append(L"_clips"); 
           // IDTMFile::File::Ptr clipFilePtr = IDTMFile::File::Create(clipFilePath.c_str());
            HFCPtr<IHPMPermanentStore<DifferenceSet, Byte, Byte>> store = new SMSQLiteDiffsetTileStore(clipFilePath, 0);//DiffSetTileStore(clipFilePath, 0);
            //store->StoreMasterHeader(NULL,0);
            m_scmIndexPtr->SetClipStore(store);
            auto pool = ScalableMeshMemoryPools<POINT>::Get()->GetDiffSetPool();
            m_scmIndexPtr->SetClipPool(pool);
            WString clipFileDefPath = m_path;
            clipFileDefPath.append(L"_clipDefinitions");
            ClipRegistry* registry = new ClipRegistry(clipFileDefPath);
            m_scmIndexPtr->SetClipRegistry(registry);
            filterP.release();

#ifdef INDEX_DUMPING_ACTIVATED
            if (s_dropNodes)
                {                                        
              //  m_scmIndexPtr->DumpOctTree("D:\\MyDoc\\Scalable Mesh Iteration 8\\PartialUpdate\\Neighbor\\Log\\nodeAfterOpen.xml", false); 
                m_scmIndexPtr->DumpOctTree("e:\\output\\scmesh\\nodeAfterOpen.xml", false);      
           //     m_scmMPointIndexPtr->ValidateNeighbors();
                }
#endif
            }


#ifndef NDEBUG

        if (s_checkHybridNodeState)
        {
            if (!m_smSQLitePtr->HasSources())
                return BSISUCCESS; // No sources were added to the STM

            IDTMSourceCollection sources;
            DocumentEnv sourceEnv(L"");

            SourcesDataSQLite* sourcesData = new SourcesDataSQLite();
            m_smSQLitePtr->LoadSources(*sourcesData);

            bool success = BENTLEY_NAMESPACE_NAME::ScalableMesh::LoadSources(sources, *sourcesData, sourceEnv);
            assert(success == true);


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
    m_scmIndexPtr = 0;

    if(m_smSQLitePtr != nullptr)
        m_smSQLitePtr->Close();
    m_smSQLitePtr = nullptr;

    
    return SUCCESS;        
    }

template <class POINT>
DRange3d ScalableMesh<POINT>::ComputeTotalExtentFor   (const PointIndexType*   pointIndexP)
    {
    typedef ExtentOp<YProtPtExtentType>         PtExtentOpType;

    DRange3d totalExtent;
    memset(&totalExtent, 0, sizeof(totalExtent));

    if ((pointIndexP != 0) && (!pointIndexP->IsEmpty()))
        {
        YProtPtExtentType ExtentPoints = pointIndexP->GetContentExtent();
        totalExtent.low.x = PtExtentOpType::GetXMin(ExtentPoints);
        totalExtent.high.x = PtExtentOpType::GetXMax(ExtentPoints);
        totalExtent.low.y = PtExtentOpType::GetYMin(ExtentPoints);
        totalExtent.high.y = PtExtentOpType::GetYMax(ExtentPoints);
        totalExtent.low.z = PtExtentOpType::GetZMin(ExtentPoints);
        totalExtent.high.z = PtExtentOpType::GetZMax(ExtentPoints);
        }

    return totalExtent;
    }
#if 0
/*----------------------------------------------------------------------------+
|ScalableMesh::CreatePointIndexFilter
+----------------------------------------------------------------------------*/
template <class POINT> ISMPointIndexFilter<POINT, YProtPtExtentType>* ScalableMesh<POINT>::CreatePointIndexFilter(IDTMFile::UniformFeatureDir* pointDirPtr) const
    {
    HPRECONDITION(pointDirPtr != 0);

    const IDTMFile::FilteringDir* filteringDir = pointDirPtr->GetFilteringDir();
    IDTMFile::SpatialIndexDir* indexDirP = pointDirPtr->GetSpatialIndexDir();

    if (0 == filteringDir || 0 == indexDirP)
        return 0;

    BTreeIndexHandler::Ptr indexHandler = BTreeIndexHandler::CreateFrom(indexDirP);

    if (0 == indexHandler)
        return 0;
 

    ISMPointIndexFilter<POINT, YProtPtExtentType>* pFilter = 0;                            
    IDTMFile::FilterType filteringType = filteringDir->GetType();

    if (indexHandler->IsProgressive())
        {
        switch (filteringType)
            {
            case IDTMFile::FILTER_TYPE_DUMB :
                pFilter = new ScalableMeshQuadTreeBCLIBProgressiveFilter1<POINT, YProtPtExtentType>();                           
                break;            
            default : 
                assert(0);
            }               
        }
    else
        {
        switch (filteringType)
            {
            case FILTER_TYPE_DUMB :
                pFilter = new ScalableMeshQuadTreeBCLIBFilter1<POINT, YProtPtExtentType>();                           
                break;                        
            default : 
                assert(0);
            }               
        }        

    return pFilter;
    }
#endif

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

/*----------------------------------------------------------------------------+
|ScalableMesh::AddBreaklineSet
+----------------------------------------------------------------------------*/
#if 0
template <class POINT> void ScalableMesh<POINT>::AddBreaklineSet(list<HFCPtr<HVEDTMLinearFeature> >& breaklineList, 
                                                          BC_DTM_OBJ*                         po_pBcDtmObj)
    {   
    int status;

    if (breaklineList.size() > 0)
        {        
        list<HFCPtr<HVEDTMLinearFeature> >::iterator BreaklineIter    = breaklineList.begin();
        list<HFCPtr<HVEDTMLinearFeature> >::iterator BreaklineIterEnd = breaklineList.end();

        uint32_t TileNumber = 0;

        while (BreaklineIter != BreaklineIterEnd) 
            {
            DPoint3d*     linePts = new DPoint3d[(*BreaklineIter)->GetSize()];        

            for (size_t indexPoints = 0 ; indexPoints < (*BreaklineIter)->GetSize(); indexPoints++)
                {
                linePts[indexPoints].x = (*BreaklineIter)->GetPoint(indexPoints).GetX();
                linePts[indexPoints].y = (*BreaklineIter)->GetPoint(indexPoints).GetY();
                linePts[indexPoints].z = (*BreaklineIter)->GetPoint(indexPoints).GetZ();
                }

            status = bcdtmObject_storeDtmFeatureInDtmObject(po_pBcDtmObj, (DTMFeatureType)(*BreaklineIter)->GetFeatureType() /*DTMFeatureType::Breakline*/, TileNumber, 1, &po_pBcDtmObj->nullFeatureId, linePts, (uint32_t)(*BreaklineIter)->GetSize());
            BreaklineIter++;
            TileNumber++;
            }         
        }
    }
#endif
/*-------------------Methods inherited from IDTM-----------------------------*/


int64_t ScalableMeshDTM::_GetPointCount()
    {
    return m_scMesh->GetPointCount();
    }

BcDTMP ScalableMeshDTM::_GetBcDTM()
{
return 0;
};

DTMStatusInt ScalableMeshDTM::_GetBoundary(DTMPointArray& result)
{
return DTM_ERROR;
}

IDTMDrapingP ScalableMeshDTM::_GetDTMDraping()
    {
    // assert(0);
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

DTMStatusInt ScalableMeshDTM::_CalculateSlopeArea(double&, double&, const DPoint3d*, int)
    {
    return DTM_ERROR;
    }

bool ScalableMeshDTM::_GetTransformation(TransformR)
    {
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

template <class POINT> BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* ScalableMesh<POINT>::_GetDTMInterface()
 {
    return m_scalableMeshDTM.get();
 }

template <class POINT> BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* ScalableMesh<POINT>::_GetDTMInterface(DMatrix4d& storageToUors)
    {
    m_scalableMeshDTM->SetStorageToUors(storageToUors);
    return m_scalableMeshDTM.get();
    }



/*----------------------------------------------------------------------------+
|ScalableMesh::GetRootNode
+----------------------------------------------------------------------------*/
template <class POINT> HFCPtr<SMPointIndexNode<POINT, YProtPtExtentType>> ScalableMesh<POINT>::GetRootNode()
    {
    HFCPtr<SMPointIndexNode<POINT, YProtPtExtentType>> pRootNode;

    if (m_scmIndexPtr != 0)
        {
        pRootNode = m_scmIndexPtr->GetRootNode();
        }

    return pRootNode;    
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetPointCount
+----------------------------------------------------------------------------*/
template <class POINT> __int64 ScalableMesh<POINT>::_GetPointCount() 
    {                
    __int64 nbPoints = 0;

    if (m_scmIndexPtr != 0)
        {
        nbPoints += m_scmIndexPtr->GetCount();
        }
    
    //MST : Need to add the number of points in the linear index.    
    return nbPoints;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetBreaklineCount
+----------------------------------------------------------------------------*/

template <class POINT> __int64 ScalableMesh<POINT>::_GetBreaklineCount() const
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
|ScalableMesh::_GetNodeQueryInterface
+----------------------------------------------------------------------------*/
template <class POINT> IScalableMeshNodeRayQueryPtr ScalableMesh<POINT>::_GetNodeQueryInterface() const
    {
    return new ScalableMeshNodeRayQuery<POINT>(&*m_scmIndexPtr);
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

    GCS::Status wktCreateStatus = GCS::S_SUCCESS;
    //wkt = HCPWKT(savedGCS.GetWKT(wktCreateStatus).GetCStr());

    WString extendedWktStr(savedGCS.GetWKT(wktCreateStatus).GetCStr());

    if (WKTKeyword::TYPE_UNKNOWN == GetWktType(extendedWktStr))
    {
        wchar_t wktFlavor[2] = { (wchar_t)IDTMFile::WktFlavor_Autodesk, L'\0' };

        extendedWktStr += WString(wktFlavor);
        //wkt = HCPWKT(extendedWktStr.c_str());
    }

    if (GCS::S_SUCCESS != wktCreateStatus)
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
    return m_scmIndexPtr->GetClipRegistry()->AddClip(pts, ptsSize)+1;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_AddClip
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return false;
    DRange3d extent = DRange3d::From(pts, (int)ptsSize);
   /* bvector<int> idx;
    bvector<DPoint3d> points;
    vu_triangulateSpacePolygonExt(&idx, &points, const_cast<DPoint3d*>(pts), (int)ptsSize, 1e-6, true);
    bvector<DPoint3d> poly;
    bvector<DPoint3d> current;
    for (auto& i : idx)
        {
        if (i > 0) current.push_back(points[i - 1]);
        else if (current.size() > poly.size())
            {
            poly = current;
            current.clear();
            }
        }
    m_scmIndexPtr->GetClipRegistry()->ModifyClip(clipID, &poly[0], poly.size());*/
    m_scmIndexPtr->GetClipRegistry()->ModifyClip(clipID, pts, ptsSize);
    m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_ADD, clipID, extent);
    return true;
    /*if (bsiGeom_getXYPolygonArea(pts, (int)ptsSize) < 0) //need to flip polygon so it's counterclockwise
        {
        uint64_t clipId = 0;
        DPoint3d* flippedPts = new DPoint3d[ptsSize];
        for (size_t pt = 0; pt < ptsSize; ++pt) flippedPts[pt] = pts[ptsSize - 1 - pt];
        clipId = m_scmIndexPtr->GetClipRegistry()->AddClip(flippedPts, ptsSize) + 1;
        delete[] flippedPts;
        return clipId;
        }
    return m_scmIndexPtr->GetClipRegistry()->AddClip(pts, ptsSize) + 1;*/
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_ModifyClip
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return false;
    DRange3d extent = DRange3d::From(pts, (int)ptsSize);
   /* bvector<int> idx;
    bvector<DPoint3d> points;
    vu_triangulateSpacePolygonExt(&idx, &points, const_cast<DPoint3d*>(pts), (int)ptsSize, 1e-6, true);
    bvector<DPoint3d> poly;
    bvector<DPoint3d> current;
    for (auto& i : idx)
        {
        if (i > 0) current.push_back(points[i - 1]);
        else if (current.size() > poly.size())
            {
            poly = current;
            current.clear();
            }
        }
    m_scmIndexPtr->GetClipRegistry()->ModifyClip(clipID, &poly[0], poly.size());*/
    m_scmIndexPtr->GetClipRegistry()->ModifyClip(clipID, pts, ptsSize);
    m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_MODIFY, clipID, extent);
    return true;
    /*if (bsiGeom_getXYPolygonArea(pts, (int)ptsSize) < 0) //need to flip polygon so it's counterclockwise
        {
        uint64_t clipId = 0;
        DPoint3d* flippedPts = new DPoint3d[ptsSize];
        for (size_t pt = 0; pt < ptsSize; ++pt) flippedPts[pt] = pts[ptsSize - 1 - pt];
        clipId = m_scmIndexPtr->GetClipRegistry()->AddClip(flippedPts, ptsSize) + 1;
        delete[] flippedPts;
        return clipId;
        }
    return m_scmIndexPtr->GetClipRegistry()->AddClip(pts, ptsSize) + 1;*/
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_RemoveClip
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_RemoveClip(uint64_t clipID)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return false;
    bvector<DPoint3d> clipData;
    m_scmIndexPtr->GetClipRegistry()->GetClip(clipID, clipData);
    DRange3d extent = DRange3d::From(&clipData[0], (int)clipData.size());
    m_scmIndexPtr->GetClipRegistry()->DeleteClip(clipID);
    m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_DELETE, clipID, extent);
    return true;
    /*if (bsiGeom_getXYPolygonArea(pts, (int)ptsSize) < 0) //need to flip polygon so it's counterclockwise
        {
        uint64_t clipId = 0;
        DPoint3d* flippedPts = new DPoint3d[ptsSize];
        for (size_t pt = 0; pt < ptsSize; ++pt) flippedPts[pt] = pts[ptsSize - 1 - pt];
        clipId = m_scmIndexPtr->GetClipRegistry()->AddClip(flippedPts, ptsSize) + 1;
        delete[] flippedPts;
        return clipId;
        }
    return m_scmIndexPtr->GetClipRegistry()->AddClip(pts, ptsSize) + 1;*/
    }

template <class POINT> void ScalableMesh<POINT>::_SetIsInsertingClips(bool toggleInsertClips)
    {
    if (nullptr == m_scmIndexPtr || m_scmIndexPtr->GetClipRegistry() == nullptr) return;
    m_scmIndexPtr->GetClipRegistry()->GetFile()->m_autocommit = !toggleInsertClips;
    if (!toggleInsertClips) m_scmIndexPtr->RefreshMergedClips();
    }

template <class POINT> void ScalableMesh<POINT>::_ModifyClipMetadata(uint64_t clipId, double importance, int nDimensions)
    {
    if (nullptr == m_scmIndexPtr || m_scmIndexPtr->GetClipRegistry() == nullptr) return;
    m_scmIndexPtr->GetClipRegistry()->SetClipMetadata(clipId, importance, nDimensions);
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_AddClip
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t clipID)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr || skirt.size() == 0 || skirt[0].size() == 0) return false;
    DRange3d extent = DRange3d::From(skirt[0][0]);
    for (auto& vec : skirt) extent.Extend(vec, nullptr);
    m_scmIndexPtr->GetClipRegistry()->ModifySkirt(clipID, skirt);
    m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_ADD, clipID, extent, false);
    return true;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_ModifyClip
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t clipID)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr || skirt.size() ==0 || skirt[0].size() ==0) return false;
    DRange3d extent = DRange3d::From(skirt[0][0]);
    for (auto& vec : skirt) extent.Extend(vec, nullptr);
    m_scmIndexPtr->GetClipRegistry()->ModifySkirt(clipID, skirt);
    m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_MODIFY, clipID, extent, false);
    return true;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_RemoveClip
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_RemoveSkirt(uint64_t clipID)
    {
    if (m_scmIndexPtr->GetClipRegistry() == nullptr) return false;
    bvector<bvector<DPoint3d>> skirt;
    m_scmIndexPtr->GetClipRegistry()->GetSkirt(clipID, skirt);
    DRange3d extent = DRange3d::From(skirt[0][0]);
    for (auto& vec : skirt) extent.Extend(vec, nullptr);
    m_scmIndexPtr->GetClipRegistry()->DeleteClip(clipID);
    m_scmIndexPtr->PerformClipAction(ClipAction::ACTION_DELETE, clipID, extent, false);
    return true;
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
        //NEEDS_WORK_SM: Get LastModifiedTime from sqlite file
        return false;

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
        return false;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_SynchWithSources
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMesh<POINT>::_SynchWithSources()
    {
    assert(!"Not done yet");
    return false;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_IsProgressive
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_IsProgressive() const
    {
    assert(!"Not done yet");
    return true;
    } 

/*----------------------------------------------------------------------------+
|ScalableMesh::_IsReadOnly
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_IsReadOnly() const
    {

        return false;

    } 

/*----------------------------------------------------------------------------+
|ScalableMesh::_IsShareable
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_IsShareable() const
    {

        return true;

    } 

/*----------------------------------------------------------------------------+
|MrDTM::_LoadAllNodeHeaders
+----------------------------------------------------------------------------*/
#ifdef SCALABLE_MESH_ATP
template <class POINT> int ScalableMesh<POINT>::_LoadAllNodeHeaders(size_t& nbLoadedNodes) const
    {    
    m_scmIndexPtr->LoadTree(nbLoadedNodes);    
    return SUCCESS;
    } 
#endif
/*----------------------------------------------------------------------------+
|ScalableMeshSingleResolutionPointIndexView Method Definition Section - Begin
+----------------------------------------------------------------------------*/
template <class POINT> ScalableMeshSingleResolutionPointIndexView<POINT>::ScalableMeshSingleResolutionPointIndexView(HFCPtr<SMPointIndex<POINT, YProtPtExtentType>> scmPointIndexPtr, 
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

// Inherited from IDTM   
template <class POINT> __int64 ScalableMeshSingleResolutionPointIndexView<POINT>::_GetPointCount()
    {
    return m_scmIndexPtr->GetNbObjectsAtLevel(m_resolutionIndex);
    }


template <class POINT> __int64 ScalableMeshSingleResolutionPointIndexView<POINT>::_GetBreaklineCount() const
    {
    assert(0);
    return 0;
    }


template <class POINT> BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* ScalableMeshSingleResolutionPointIndexView<POINT>::_GetDTMInterface()
    {
    assert(0);
    return 0;
    }

template <class POINT> BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM* ScalableMeshSingleResolutionPointIndexView<POINT>::_GetDTMInterface(DMatrix4d& storageToUors)
    {
    assert(0);
    return 0;
    }


template <class POINT> DTMStatusInt ScalableMeshSingleResolutionPointIndexView<POINT>::_GetRange(DRange3dR range)
    {        
    YProtPtExtentType ExtentPoints = m_scmIndexPtr->GetContentExtent();

    range.low.x = ExtentOp<YProtPtExtentType>::GetXMin(ExtentPoints);
    range.high.x = ExtentOp<YProtPtExtentType>::GetXMax(ExtentPoints);
    range.low.y = ExtentOp<YProtPtExtentType>::GetYMin(ExtentPoints);
    range.high.y = ExtentOp<YProtPtExtentType>::GetYMax(ExtentPoints);
    range.low.z = ExtentOp<YProtPtExtentType>::GetZMin(ExtentPoints);
    range.high.z = ExtentOp<YProtPtExtentType>::GetZMax(ExtentPoints);   
    return DTM_SUCCESS;
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

    YProtPtExtentType ExtentPoints = m_scmIndexPtr->GetContentExtent();

    DRange3d initialRange;
    initialRange.low.x = ExtentOp<YProtPtExtentType>::GetXMin(ExtentPoints);
    initialRange.high.x = ExtentOp<YProtPtExtentType>::GetXMax(ExtentPoints);
    initialRange.low.y = ExtentOp<YProtPtExtentType>::GetYMin(ExtentPoints);
    initialRange.high.y = ExtentOp<YProtPtExtentType>::GetYMax(ExtentPoints);
    initialRange.low.z = ExtentOp<YProtPtExtentType>::GetZMin(ExtentPoints);
    initialRange.high.z = ExtentOp<YProtPtExtentType>::GetZMax(ExtentPoints);

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
                                                                                           const unsigned __int64& maxNumberCountedPoints) const
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

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_IsProgressive() const
    {
    assert(0);
    return false;
    }



template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID)
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

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t clipID)
    {
    return false;
    }

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t clipID)
    {
    return false;
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

template <class POINT> int ScalableMeshSingleResolutionPointIndexView<POINT>::_SynchWithSources()
    {
    assert(!"Should not be called");
    return -1;       
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
    }

void edgeCollapsePrintGraph(WCharCP param)
    {
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
    }

void edgeCollapseShowMesh(WCharCP param, PolyfaceQueryP& outMesh)
    {
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
    }

template class ScalableMesh<DPoint3d>;
template class ScalableMeshSingleResolutionPointIndexView<DPoint3d>;

END_BENTLEY_SCALABLEMESH_NAMESPACE
