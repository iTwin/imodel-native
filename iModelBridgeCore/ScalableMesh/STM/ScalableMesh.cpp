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

extern bool   GET_HIGHEST_RES;

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include "InternalUtilityFunctions.h"
#include "ScalableMesh.h"
#include "DTMGraphTileStore.h"
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

#ifdef SCALABLE_MESH_DGN
#include <DgnPlatform\Tools\ConfigurationManager.h>
#endif

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
#ifdef SCALABLE_MESH_DGN
USING_NAMESPACE_BENTLEY_DGNPLATFORM
#endif

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE 

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

int IScalableMesh::GetNbResolutions(DTMQueryDataType queryDataType) const
    {
    return _GetNbResolutions(queryDataType);
    }

IScalableMeshQueryPtr IScalableMesh::GetQueryInterface(DTMQueryType     queryType, 
                                         DTMQueryDataType queryDataType) const
    {
    return _GetQueryInterface(queryType, queryDataType);
    }

IScalableMeshQueryPtr IScalableMesh::GetQueryInterface(DTMQueryType                         queryType, 
                                         DTMQueryDataType                     queryDataType, 
                                         Bentley::GeoCoordinates::BaseGCSPtr& targetGCS,
                                         const DRange3d& extentInTargetGCS) const
    {
    return _GetQueryInterface(queryType, queryDataType, targetGCS, extentInTargetGCS);
    }

IScalableMeshMeshQueryPtr IScalableMesh::GetMeshQueryInterface(MeshQueryType queryType) const
    {
    return _GetMeshQueryInterface(queryType);
    }

IScalableMeshMeshQueryPtr IScalableMesh::GetMeshQueryInterface(MeshQueryType queryType,
                                                 Bentley::GeoCoordinates::BaseGCSPtr& targetGCS,
                                                 const DRange3d& extentInTargetGCS) const
    {
    return _GetMeshQueryInterface(queryType, targetGCS, extentInTargetGCS);
    }

IScalableMeshNodeRayQueryPtr IScalableMesh::GetNodeQueryInterface() const
    {
    return _GetNodeQueryInterface();
    }

IDTMVolume* IScalableMesh::GetDTMVolume()
    {
    return _GetDTMVolume();
    }

const BaseGCSPtr& IScalableMesh::GetBaseGCS() const 
    {
    return _GetGCS().GetGeoRef().GetBasePtr();
    }

StatusInt IScalableMesh::SetBaseGCS(const BaseGCSPtr& baseGCSPtr)
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

int IScalableMesh::GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, Bentley::GeoCoordinates::BaseGCSPtr& targetGCS) const
    {
    return _GetRangeInSpecificGCS(lowPt, highPt, targetGCS);
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
IScalableMeshPtr IScalableMesh::GetFor   (const WChar*          filePath,
                            bool                    openReadOnly,
                            bool                    openShareable,
                            StatusInt&              status)
    {
    const UInt workingLayer = DEFAULT_WORKING_LAYER;
    
    status = BSISUCCESS;            
 
    if (0 != _waccess(filePath, 04))
        {
        status = BSISUCCESS; 
        return 0; // File not found
        }
        
    const AccessMode accessMode(GetAccessModeFor(openReadOnly, openShareable));

    StatusInt openStatus;

    IDTMFile::File::Ptr iDTMFilePtr(IDTMFile::File::Open(filePath, accessMode, openStatus));

    if (0 == iDTMFilePtr)
        {
        if (openStatus == DTMFILE_UNSUPPORTED_VERSION)

            status = DTMSTATUS_UNSUPPORTED_VERSION;

        else

            status = BSIERROR;

            

        return 0; // Error opening file
        }
           
    if (!iDTMFilePtr->GetRootDir()->HasLayerDir(workingLayer))
        {
        //MST For now returns success until a layer is create which contains the 
        //    data source information.
        return ScalableMesh<IDTMFile::Point3d64f>::Open(iDTMFilePtr, filePath, status);
        }

    const IDTMFile::LayerDir* layerDir = iDTMFilePtr->GetRootDir()->GetLayerDir (workingLayer);
    if (0 == layerDir)
        {
        status = BSIERROR;
        return 0; // Error accessing working layer
        }

    if (!layerDir->HasUniformFeatureDir(SMPointTaggedTileStore<IDTMFile::Point3d64f, YProtPtExtentType>::MASS_POINT_FEATURE_TYPE))
        {
        //MST : The point type might be incorrect depending on the filtering chosen. 
        //      The point type should probably be stored in the layer.
        return ScalableMesh<IDTMFile::Point3d64f>::Open(iDTMFilePtr, filePath, status);
        }

    const IDTMFile::UniformFeatureDir* featureDir = 
        layerDir->GetUniformFeatureDir(SMPointTaggedTileStore<IDTMFile::Point3d64f, YProtPtExtentType>::MASS_POINT_FEATURE_TYPE);

    if (0 == featureDir)
        {
        status = BSIERROR;
        return 0; // Error accessing specified feature dir
        }

    IScalableMeshPtr iScmPtr;

    const IDTMFile::PointTypeID ptType = featureDir->GetPointType();
    switch(ptType)
        {
        case IDTMFile::POINT_TYPE_XYZf64 :                   
            iScmPtr = ScalableMesh<IDTMFile::Point3d64f>::Open(iDTMFilePtr, filePath, status);
            break;
        case IDTMFile::POINT_TYPE_XYZMf64 :            
            iScmPtr = ScalableMesh<IDTMFile::Point3d64fM64f>::Open(iDTMFilePtr, filePath, status);
            break;
        default : 
            status = BSIERROR;
            assert(!"This point type is not yet supported");
            break;
        }    
                     
    return iScmPtr;   
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

#ifdef SCALABLE_MESH_DGN
static bool DgnDbFilename(Bentley::WString& stmFilename)
    {
    Bentley::WString dgndbFilename;
    //stmFilename
    size_t size = stmFilename.ReplaceAll(L".stm", L".dgndb");
    assert(size==1);
    return true;
    }
#endif

/*----------------------------------------------------------------------------+
|ScalableMeshBase::ScalableMeshBase
+----------------------------------------------------------------------------*/
ScalableMeshBase::ScalableMeshBase   (IDTMFile::File::Ptr&            filePtr, 
                        const WString&             filePath)
    :   m_iDTMFilePtr(filePtr),
        m_workingLayer(DEFAULT_WORKING_LAYER),
        m_sourceGCS(GetDefaultGCS()),
        m_path(filePath)
    {
    memset(&m_contentExtent, 0, sizeof(m_contentExtent));

    HPRECONDITION(filePtr != 0);
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
const IDTMFile::File::Ptr& ScalableMeshBase::GetIDTMFile () const
    {
    return m_iDTMFilePtr;
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
bool ScalableMeshBase::LoadGCSFrom (const IDTMFile::LayerDir& layerDir)
    {
    if (!layerDir.HasWkt())
        return true;

    const HCPWKT wkt(layerDir.GetWkt());
    if (wkt.IsEmpty())
        return false;

    WString wktStr(wkt.GetCStr());
    
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
template <class POINT> ScalableMesh<POINT>::ScalableMesh(IDTMFile::File::Ptr& iDTMFilePtr, const WString& path)
    :   ScalableMeshBase(iDTMFilePtr, path),
        m_areDataCompressed(false),
        m_computeTileBoundary(false),
        m_minScreenPixelsPerPoint(MEAN_SCREEN_PIXELS_PER_POINT)
    {
#ifdef SCALABLE_MESH_DGN
    // Test if dgndb exist
    Bentley::WString filename = m_path;
    DgnDbFilename(filename);    
    
    {
    std::wifstream infile(filename.c_str());
    
    m_isDgnDB = infile.good();
    }    
    if(m_isDgnDB)
        {
        // NEEDS_WORK_SM : check if in open we load the right mode : BLOB or Table
        bool status = IDgnDbScalableMesh::Initialize(m_dgnScalableMeshPtr, false);
        if(!status)
            throw;
        m_dgnScalableMeshPtr->Open(filename.GetWCharCP());
        }
#endif
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
                double nodeExtentDistanceX = nodePtr->GetContentExtent().xMax - nodePtr->GetContentExtent().xMin;
                double nodeExtentDistanceY = nodePtr->GetContentExtent().yMax - nodePtr->GetContentExtent().yMin;
                double nodeExtentArea = nodeExtentDistanceX * nodeExtentDistanceY;
                double extentDistanceX = extent.xMax - extent.xMin;
                double extentDistanceY = extent.yMax - extent.yMin;
                double extentArea = extentDistanceX * extentDistanceY;
                return unsigned __int64(nodePtr->GetCount()*(extentArea/nodeExtentArea));
                }
            else {
                double nodeExtentDistanceX = nodePtr->GetContentExtent().xMax - nodePtr->GetContentExtent().xMin;
                double nodeExtentDistanceY = nodePtr->GetContentExtent().yMax - nodePtr->GetContentExtent().yMin;
                double nodeExtentArea = nodeExtentDistanceX * nodeExtentDistanceY;
                // Create intersection extent
                double xAxisValues[] = {nodePtr->GetContentExtent().xMin,nodePtr->GetContentExtent().xMax,extent.xMin,extent.xMax};
                vector<double> xAxisValuesV (xAxisValues,xAxisValues+sizeof(xAxisValues)/ sizeof(double)) ;
                sort(xAxisValuesV.begin(),xAxisValuesV.end());
                double yAxisValues[] = {nodePtr->GetContentExtent().yMin,nodePtr->GetContentExtent().yMax,extent.yMin,extent.yMax};
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
            HFCPtr<SMPointIndexNode<POINT, YProtPtExtentType>> nodePtr = m_scmPointIndexPtr->GetRootNode();
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
ScalableMesh<POINT>* ScalableMesh<POINT>::Open   (IDTMFile::File::Ptr&    filePtr, 
                                    const WString&     filePath, 
                                    StatusInt&              status)
    {
    auto_ptr<ScalableMesh<POINT>> scmP(new ScalableMesh<POINT>(filePtr, filePath));                    

    status = scmP->Open();
    return (BSISUCCESS == status ? scmP.release() : 0);
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::Open
+----------------------------------------------------------------------------*/
static bool s_dropNodes = false; 
static bool s_checkHybridNodeState = false;
template <class POINT> int ScalableMesh<POINT>::Open()
    {       
    HPRECONDITION(m_iDTMFilePtr != 0);

    s_inEditing = false;   

    if (!m_iDTMFilePtr->GetRootDir()->HasLayerDir(m_workingLayer))
        return BSISUCCESS; // File is empty.

    IDTMFile::LayerDir* layerDir = m_iDTMFilePtr->GetRootDir()->GetLayerDir(m_workingLayer);
    if (0 == layerDir)
        return BSIERROR; // Could not access existing layer

    try 
        {       
        if (!LoadGCSFrom(*layerDir))
            return BSIERROR; // Error loading layer gcs

        const bool hasPoints = layerDir->HasUniformFeatureDir(SMPointTaggedTileStore<POINT, YProtPtExtentType>::MASS_POINT_FEATURE_TYPE);
        const bool hasFeatures = layerDir->CountMixedFeatureDirs() > 0;

        HFCPtr<TileStoreType> pTileStore;
#ifdef SCALABLE_MESH_DGN
        HFCPtr<DgnStoreType>  pDgnTileStore;
#endif

        if (hasPoints)
            {    
         //   HFCPtr<HPMCountLimitedPool<POINT> > pMemoryPool(PoolSingleton<POINT>());

            IDTMFile::UniformFeatureDir* featureDir =
                layerDir->GetUniformFeatureDir(SMPointTaggedTileStore<POINT, YProtPtExtentType>::MASS_POINT_FEATURE_TYPE);  

            if (0 == featureDir)
                return BSIERROR;

            //NEEDS_WORK_SM - Why correct filter is not saved?                                             
            //auto_ptr<ISMPointIndexFilter<POINT, YProtPtExtentType>> filterP(CreatePointIndexFilter(featureDir));
            auto_ptr<ISMMeshIndexFilter<POINT, YProtPtExtentType>> filterP(new ScalableMeshQuadTreeBCLIBMeshFilter1<POINT, YProtPtExtentType>());

            if (0 == filterP.get())
                return BSIERROR;

            m_areDataCompressed = (featureDir->GetPointCompressType().GetType() != HTGFF::Compression::TYPE_NONE);

#ifdef SCALABLE_MESH_DGN

            if(m_isDgnDB)
                {
                pDgnTileStore = new DgnStoreType(m_dgnScalableMeshPtr, m_iDTMFilePtr, HFC_READ_ONLY | HFC_SHARE_READ_ONLY, AreDataCompressed());

                m_scmPointIndexPtr = new PointIndexType(pMemoryPool, 
                    &*pDgnTileStore, 
                    new HPMIndirectCountLimitedPool<MTGGraph>(new HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment(100, 2000*sizeof(POINT)), 600000000),
                    new DgnGraphTileStore(m_dgnScalableMeshPtr, m_iDTMFilePtr, 0),
                    10000, 
                    filterP.get(),
                    true, 
                    false, 
                    0, 
                    0);  //NEEDS_WORK_SM - Should we store mesher information in STM file like filter?
                }
            else
#endif
                {
                pTileStore = new TileStoreType(m_iDTMFilePtr, AreDataCompressed());
   
                m_scmPointIndexPtr = new PointIndexType(//pMemoryPool, 
                                                        ScalableMeshMemoryPools<POINT>::Get()->GetPointPool(),
                                                          &*pTileStore, 
                                                          ScalableMeshMemoryPools<POINT>::Get()->GetGraphPool(),
                                                          //new HPMIndirectCountLimitedPool<MTGGraph>(new HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment(100, 2000*sizeof(POINT)), 600000000),
                                                          new DTMGraphTileStore(m_iDTMFilePtr, 0),
                                                          10000, 
                                                          filterP.get(),
                                                          false, 
                                                          false, 
                                                          0, 
                                                          0);  //NEEDS_WORK_SM - Should we store mesher information in STM file like filter?
                }

            filterP.release();

#ifdef INDEX_DUMPING_ACTIVATED
            if (s_dropNodes)
                {                                        
              //  m_scmPointIndexPtr->DumpOctTree("D:\\MyDoc\\Scalable Mesh Iteration 8\\PartialUpdate\\Neighbor\\Log\\nodeAfterOpen.xml", false); 
                m_scmPointIndexPtr->DumpOctTree("D:\\nodeAfterOpen.xml", false);      
           //     m_scmMPointIndexPtr->ValidateNeighbors();
                }
#endif
            }


#ifndef NDEBUG
        
        if (s_checkHybridNodeState)
            {
            if (!m_iDTMFilePtr->GetRootDir()->HasSourcesDir())
            return BSISUCCESS; // No sources were added to the STM.

            const IDTMFile::SourcesDir* sourceDirPtr = m_iDTMFilePtr->GetRootDir()->GetSourcesDir();
            if (0 == sourceDirPtr)
                return BSIERROR; // Could not load existing sources dir
            
            IDTMSourceCollection sources;            
            DocumentEnv          sourceEnv(L"");

            bool success = Bentley::ScalableMesh::LoadSources(sources, *sourceDirPtr, sourceEnv);
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

                m_scmPointIndexPtr->ValidateIs3dDataStates(source2_5dRanges, source3dRanges);    

                sourceIter++;
                }                       
            }            
#endif
                       
        if (hasFeatures)
            {
#ifndef SCALABLE_MESH_DGN
            bool m_isDgnDB = false;
#endif
            if (pTileStore == 0 && !m_isDgnDB)
                {
                pTileStore = new TileStoreType(m_iDTMFilePtr, AreDataCompressed());
                }
#ifdef SCALABLE_MESH_DGN
            else if (pDgnTileStore == 0 && m_isDgnDB)
                {
                pDgnTileStore = new DgnStoreType(m_dgnScalableMeshPtr, m_iDTMFilePtr, HFC_READ_ONLY | HFC_SHARE_READ_ONLY, AreDataCompressed());
               // m_mrDTMLinearIndexPtr = new LinearIndexType (&*pDgnTileStore, 400, false);
                }
#endif
           
            
            }

        m_contentExtent = ComputeTotalExtentFor(&*m_scmPointIndexPtr);

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
    m_scmPointIndexPtr = 0;   
    m_iDTMFilePtr = 0;
#ifdef SCALABLE_MESH_DGN
    m_dgnScalableMeshPtr = 0;
#endif
    
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
            case IDTMFile::FILTER_TYPE_TILE :
                pFilter = new ScalableMeshQuadTreeBCLIBProgressiveFilter2<POINT, YProtPtExtentType>();      
                break;
            case IDTMFile::FILTER_TYPE_TIN :                     
                pFilter = new ScalableMeshQuadTreeBCLIBProgressiveFilter3<POINT, YProtPtExtentType>();      
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
            case FILTER_TYPE_TILE :
                pFilter = new ScalableMeshQuadTreeBCLIBFilter2<POINT, YProtPtExtentType>();      
                break;
            case FILTER_TYPE_TIN :                     
                pFilter = new ScalableMeshQuadTreeBCLIBFilter3<POINT, YProtPtExtentType>();      
                break;
            default : 
                assert(0);
            }               
        }        

    return pFilter;
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
    assert(m_iDTMFilePtr != 0);
    
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

/*-------------------Methods inherited from IDTM-----------------------------*/

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetDTMDraping
+----------------------------------------------------------------------------*/
template <class POINT> IDTMDrapingP ScalableMesh<POINT>::_GetDTMDraping() 
    {
   // assert(0);
    return new ScalableMeshDraping(this);
    }

template <class POINT> IDTMDrainageP    ScalableMesh<POINT>::_GetDTMDrainage()
    {
    assert(0);
    return 0;
    }

template <class POINT> IDTMContouringP  ScalableMesh<POINT>::_GetDTMContouring()
    {
    assert(0);
    return 0;
    }

template <class POINT> IDTMVolumeP ScalableMesh<POINT>::_GetDTMVolume()
    {
    return new ScalableMeshVolume(this);
    }

template <class POINT> DTMStatusInt ScalableMesh<POINT>::_CalculateSlopeArea (double&, double&, const DPoint3d*, int)
    {
    return DTM_ERROR;
    }

template <class POINT> bool ScalableMesh<POINT>::_GetTransformation(TransformR)
    {
    return true;
    }

template <class POINT> DTMStatusInt ScalableMesh<POINT>::_GetTransformDTM(DTMPtr& transformedDTM, TransformCR)
    {
    return DTM_ERROR;
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetPointCount
+----------------------------------------------------------------------------*/
template <class POINT> __int64 ScalableMesh<POINT>::_GetPointCount() 
    {        
    
    
    __int64 nbPoints = 0;

    if (m_scmPointIndexPtr != 0)
        {
        nbPoints += m_scmPointIndexPtr->GetCount();
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


/*----------------------------------------------------------------------------+
|ScalableMesh::_GetBcDTM
+----------------------------------------------------------------------------*/
template <class POINT> BcDTMP ScalableMesh<POINT>::_GetBcDTM() 
    {    
    return 0;
    };
 
/*----------------------------------------------------------------------------+
|ScalableMesh::_GetBoundary
+----------------------------------------------------------------------------*/
template <class POINT> DTMStatusInt ScalableMesh<POINT>::_GetBoundary(DTMPointArray& result) 
    {
    return DTM_ERROR; 
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
template <class POINT> IScalableMeshQueryPtr ScalableMesh<POINT>::_GetQueryInterface(DTMQueryType                         queryType, 
                                                                       DTMQueryDataType                     queryDataType, 
                                                                       Bentley::GeoCoordinates::BaseGCSPtr& targetBaseGCSPtr,
                                                                       const DRange3d&                       extentInTargetGCS) const
    {
    IScalableMeshQueryPtr scmQueryPtr;
                    
    GCS targetGCS = GetGCSFactory().Create(targetBaseGCSPtr);        

    _GetQueryInterface(queryType, queryDataType);    
    
    // TODO: Remove cast when smart pointers becomes const aware
    ScalableMesh<POINT>* UNCONST_THIS = const_cast<ScalableMesh<POINT>*>(this);

    scmQueryPtr = ScalableMeshQuery::GetReprojectionQueryInterface(UNCONST_THIS, queryType, queryDataType, m_sourceGCS, targetGCS, extentInTargetGCS);
             
    return scmQueryPtr;                
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetQueryInterface
+----------------------------------------------------------------------------*/
template <class POINT> IScalableMeshQueryPtr ScalableMesh<POINT>::_GetQueryInterface(DTMQueryType queryType, DTMQueryDataType queryDataType) const
    {
    IScalableMeshQueryPtr iScalableMeshQueryPtr;

    if (queryDataType == DTM_QUERY_DATA_POINT)
        {
        if ((m_scmPointIndexPtr != 0) && (m_scmPointIndexPtr->IsEmpty() == false))
            {
            if (queryType == DTM_QUERY_FULL_RESOLUTION)
                {                                    
                //MST Should be done by using the highest resolution.
                //iScalableMeshQueryPtr = new ScalableMeshFullResolutionQuery(m_mrDTMPointIndexPtr);   
                }
            else
            if (queryType == DTM_QUERY_VIEW_DEPENDENT)
                {                        
                iScalableMeshQueryPtr = new ScalableMeshViewDependentPointQuery<POINT>(&*m_scmPointIndexPtr);       
                }
            else
            if (queryType == DTM_QUERY_FIX_RESOLUTION_VIEW)
                {        
                iScalableMeshQueryPtr = new ScalableMeshFixResolutionViewPointQuery<POINT>(&*m_scmPointIndexPtr, m_sourceGCS);
                }
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
            return new ScalableMeshFullResolutionMeshQuery<POINT>(&*m_scmPointIndexPtr);
        case MESH_QUERY_VIEW_DEPENDENT:
            return new ScalableMeshViewDependentMeshQuery<POINT>(&*m_scmPointIndexPtr);
        case MESH_QUERY_PLANE_INTERSECT:
            return new ScalableMeshNodePlaneQuery<POINT>(&*m_scmPointIndexPtr);
        default:
            return new ScalableMeshViewDependentMeshQuery<POINT>(&*m_scmPointIndexPtr);
        }
    }

template <class POINT> IScalableMeshMeshQueryPtr ScalableMesh<POINT>::_GetMeshQueryInterface(MeshQueryType queryType,
                                                                               Bentley::GeoCoordinates::BaseGCSPtr& targetBaseGCSPtr,
                                                                               const DRange3d&                       extentInTargetGCS) const
    {
    GCS targetGCS = GetGCSFactory().Create(targetBaseGCSPtr);
    IScalableMeshMeshQueryPtr meshQueryPtr = new ScalableMeshReprojectionMeshQuery<POINT>(_GetMeshQueryInterface(queryType),
                                                                            m_scmPointIndexPtr,
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
    return new ScalableMeshNodeRayQuery<POINT>(&*m_scmPointIndexPtr);
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetNbResolutions
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMesh<POINT>::_GetNbResolutions(DTMQueryDataType queryDataType) const
    {    
    int nbResolutions = 0;

    if (queryDataType == DTM_QUERY_DATA_POINT)
        {
        if (m_scmPointIndexPtr != 0)
            {
            nbResolutions = (int)(m_scmPointIndexPtr->GetDepth() + 1);
            }        
        }


    return nbResolutions;    
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
    IDTMFile::LayerDir* const layerDir = (m_iDTMFilePtr->GetRootDir()->HasLayerDir(m_workingLayer)) ? 
                                                m_iDTMFilePtr->GetRootDir()->GetLayerDir(m_workingLayer) : 
                                                m_iDTMFilePtr->GetRootDir()->CreateLayerDir(m_workingLayer);

    if (0 == layerDir)
        return BSIERROR;

    const GCS& savedGCS = (newGCS.IsNull()) ? GetDefaultGCS() : newGCS;

    HCPWKT wkt;
   
    GCS::Status wktCreateStatus = GCS::S_SUCCESS;
    wkt = HCPWKT(savedGCS.GetWKT(wktCreateStatus).GetCStr());

    WString extendedWktStr(wkt.GetCStr());

    if (WKTKeyword::TYPE_UNKNOWN == GetWktType(extendedWktStr))
        {
        wchar_t wktFlavor[2] = {(wchar_t)IDTMFile::WktFlavor_Autodesk, L'\0'};

        extendedWktStr += WString(wktFlavor);        
        wkt = HCPWKT(extendedWktStr.c_str());
        }    

    if (GCS::S_SUCCESS != wktCreateStatus)
        return BSIERROR;

    // This is called even if there are no GCS provided ... In such case the WKT AString
    // is that of the default GCS.
    HCPWKT oldWkt = layerDir->GetWkt();

    if (!layerDir->SetWkt(wkt))
        {
        bool result = layerDir->SetWkt(oldWkt);
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
    if (0 == m_scmPointIndexPtr)
        return true;

    return EqEps(m_contentExtent.low.x, m_contentExtent.high.x) &&
           EqEps(m_contentExtent.low.y, m_contentExtent.high.y) &&
           EqEps(m_contentExtent.low.z, m_contentExtent.high.z);
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
    assert(0 != m_iDTMFilePtr);
    const SourcesDir* sourceDirPtr = m_iDTMFilePtr->GetRootDir()->GetSourcesDir();
    if (0 == sourceDirPtr)
        return false;

    const bool InSync = sourceDirPtr->GetLastModifiedTime() < sourceDirPtr->GetLastSyncTime();
    return InSync;    
    }

/*----------------------------------------------------------------------------+
|ScalableMesh::_GetRangeInSpecificGCS
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMesh<POINT>::_GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, Bentley::GeoCoordinates::BaseGCSPtr& targetGCS) const
    {
    StatusInt status = SUCCESS;

    DRange3d reprojectedRange;
    status = ReprojectRangeDomainLimited(reprojectedRange, m_contentExtent, (BaseGCSPtr&)m_sourceGCS.GetGeoRef().GetBasePtr(), targetGCS);

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
    assert(0 != m_iDTMFilePtr);

    const SourcesDir* sourceDirPtr = m_iDTMFilePtr->GetRootDir()->GetSourcesDir();
    if (0 == sourceDirPtr)
        {
        lastCheckTime = 0; // Make it very remote in time
        return false;
        }

    lastCheckTime = sourceDirPtr->GetLastModifiedCheckTime();
    return true;
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
    return (m_iDTMFilePtr->GetAccessMode().m_HasWriteAccess == false) && 
           (m_iDTMFilePtr->GetAccessMode().m_HasWriteShare == false);
    } 

/*----------------------------------------------------------------------------+
|ScalableMesh::_IsShareable
+----------------------------------------------------------------------------*/
template <class POINT> bool ScalableMesh<POINT>::_IsShareable() const
    {    
    return (m_iDTMFilePtr->GetAccessMode().m_HasReadShare == false) && 
           (m_iDTMFilePtr->GetAccessMode().m_HasWriteShare == false);                
    } 

/*----------------------------------------------------------------------------+
|MrDTM::_LoadAllNodeHeaders
+----------------------------------------------------------------------------*/
#ifdef SCALABLE_MESH_ATP
template <class POINT> int ScalableMesh<POINT>::_LoadAllNodeHeaders(size_t& nbLoadedNodes) const
    {    
    //m_scmPointIndexPtr->LoadTree(nbLoadedNodes);    
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
    m_scmPointIndexPtr = scmPointIndexPtr;
    m_resolutionIndex    = resolutionIndex;    
    }

template <class POINT> ScalableMeshSingleResolutionPointIndexView<POINT>::~ScalableMeshSingleResolutionPointIndexView()
    {
    } 

// Inherited from IDTM   
template <class POINT> __int64 ScalableMeshSingleResolutionPointIndexView<POINT>::_GetPointCount()
    {
    return m_scmPointIndexPtr->GetNbObjectsAtLevel(m_resolutionIndex);
    }


template <class POINT> __int64 ScalableMeshSingleResolutionPointIndexView<POINT>::_GetBreaklineCount() const
    {
    assert(0);
    return 0;
    }

template <class POINT> IDTMDrapingP ScalableMeshSingleResolutionPointIndexView<POINT>::_GetDTMDraping()
    {
    assert(0);
    return 0;
    }

template <class POINT> IDTMDrainageP    ScalableMeshSingleResolutionPointIndexView<POINT>::_GetDTMDrainage()
    {
    assert(0);
    return 0;
    }

template <class POINT> IDTMContouringP  ScalableMeshSingleResolutionPointIndexView<POINT>::_GetDTMContouring()
    {
    assert(0);
    return 0;
    }

template <class POINT> IDTMVolumeP  ScalableMeshSingleResolutionPointIndexView<POINT>::_GetDTMVolume()
    {
    assert(0);
    return 0;
    }

template <class POINT> DTMStatusInt ScalableMeshSingleResolutionPointIndexView<POINT>::_CalculateSlopeArea (double&, double&, const DPoint3d*, int)
    {
    return DTM_ERROR;
    }

template <class POINT> bool ScalableMeshSingleResolutionPointIndexView<POINT>::_GetTransformation(TransformR)
    {
    return true;
    }

template <class POINT> DTMStatusInt ScalableMeshSingleResolutionPointIndexView<POINT>::_GetTransformDTM(DTMPtr& transformedDTM, TransformCR)
    {
    return DTM_ERROR;
    }

template <class POINT> DTMStatusInt ScalableMeshSingleResolutionPointIndexView<POINT>::_GetRange(DRange3dR range)
    {        
    YProtPtExtentType ExtentPoints = m_scmPointIndexPtr->GetContentExtent();

    range.low.x = ExtentOp<YProtPtExtentType>::GetXMin(ExtentPoints);
    range.high.x = ExtentOp<YProtPtExtentType>::GetXMax(ExtentPoints);
    range.low.y = ExtentOp<YProtPtExtentType>::GetYMin(ExtentPoints);
    range.high.y = ExtentOp<YProtPtExtentType>::GetYMax(ExtentPoints);
    range.low.z = ExtentOp<YProtPtExtentType>::GetZMin(ExtentPoints);
    range.high.z = ExtentOp<YProtPtExtentType>::GetZMax(ExtentPoints);   
    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------------+
|ScalableMeshSingleResolutionPointIndexView::_GetRangeInSpecificGCS
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMeshSingleResolutionPointIndexView<POINT>::_GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, Bentley::GeoCoordinates::BaseGCSPtr& targetGCS) const
    {
    StatusInt status = SUCCESS;

    YProtPtExtentType ExtentPoints = m_scmPointIndexPtr->GetContentExtent();

    DRange3d initialRange;
    initialRange.low.x = ExtentOp<YProtPtExtentType>::GetXMin(ExtentPoints);
    initialRange.high.x = ExtentOp<YProtPtExtentType>::GetXMax(ExtentPoints);
    initialRange.low.y = ExtentOp<YProtPtExtentType>::GetYMin(ExtentPoints);
    initialRange.high.y = ExtentOp<YProtPtExtentType>::GetYMax(ExtentPoints);
    initialRange.low.z = ExtentOp<YProtPtExtentType>::GetZMin(ExtentPoints);
    initialRange.high.z = ExtentOp<YProtPtExtentType>::GetZMax(ExtentPoints);

    DRange3d reprojectedRange;
    status = ReprojectRangeDomainLimited(reprojectedRange, initialRange, (BaseGCSPtr&)m_sourceGCS.GetGeoRef().GetBasePtr(), targetGCS);

    lowPt.x = reprojectedRange.low.x;
    highPt.x = reprojectedRange.high.x;
    lowPt.y = reprojectedRange.low.y;
    highPt.y = reprojectedRange.high.y;
    lowPt.z = reprojectedRange.low.z;
    highPt.z = reprojectedRange.high.z;


    return status;    
    }

template <class POINT> BcDTMP ScalableMeshSingleResolutionPointIndexView<POINT>::_GetBcDTM()
    {
    assert(0);
    return 0;
    }

template <class POINT> DTMStatusInt ScalableMeshSingleResolutionPointIndexView<POINT>::_GetBoundary(DTMPointArray&)
    {
    assert(0);
    return DTM_ERROR;
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

template <class POINT> int ScalableMeshSingleResolutionPointIndexView<POINT>::_GetNbResolutions(DTMQueryDataType queryDataType) const
    {
    int nbResolutions;

    if (queryDataType == DTM_QUERY_DATA_POINT)
        {
        nbResolutions = 1;
        }
    else
        {
        nbResolutions = 0;
        }

    return nbResolutions;
    }

template <class POINT> IScalableMeshQueryPtr ScalableMeshSingleResolutionPointIndexView<POINT>::_GetQueryInterface(DTMQueryType                         queryType, 
                                                                                                     DTMQueryDataType                     queryDataType, 
                                                                                                     Bentley::GeoCoordinates::BaseGCSPtr& targetGCS,
                                                                                                     const DRange3d&                      extentInTargetGCS) const
    {                   
    GeoCoords::GCS targetGeoCoord(GetGCSFactory().Create(targetGCS));           
   
    // TODO: Remove once smart pointers are const aware
    ScalableMeshSingleResolutionPointIndexView<POINT>* UNCONST_THIS = const_cast<ScalableMeshSingleResolutionPointIndexView<POINT>*>(this);

    IScalableMeshQueryPtr scmQueryPtr = ScalableMeshQuery::GetReprojectionQueryInterface(UNCONST_THIS, queryType, queryDataType, m_sourceGCS, targetGeoCoord, extentInTargetGCS);

    return scmQueryPtr;                
    }

template <class POINT> IScalableMeshQueryPtr ScalableMeshSingleResolutionPointIndexView<POINT>::_GetQueryInterface(DTMQueryType     queryType, 
                                                                                                     DTMQueryDataType queryDataType) const
    {    
    IScalableMeshQueryPtr iScalableMeshQueryPtr;

    if (queryDataType == DTM_QUERY_DATA_POINT && (m_scmPointIndexPtr != 0) && (m_scmPointIndexPtr->IsEmpty() == false))
        {    
        if (queryType == DTM_QUERY_FULL_RESOLUTION)
            {
            iScalableMeshQueryPtr =  new ScalableMeshFullResolutionPointQuery<POINT>(m_scmPointIndexPtr, 
                                                                       m_resolutionIndex);
            }
        else
        if (queryType == DTM_QUERY_VIEW_DEPENDENT)
            {
            iScalableMeshQueryPtr =  new ScalableMeshViewDependentPointQuery<POINT>(m_scmPointIndexPtr);       
            }

        //DTM_QUERY_FIX_RESOLUTION_VIEW is meaningless for a single resolution view.         
        }
        
    return iScalableMeshQueryPtr;
    }
  
template <class POINT> IScalableMeshMeshQueryPtr ScalableMeshSingleResolutionPointIndexView<POINT>::_GetMeshQueryInterface(MeshQueryType queryType) const
    {
    assert(!"Not supported");

    return IScalableMeshMeshQueryPtr();
    }

template <class POINT> IScalableMeshMeshQueryPtr ScalableMeshSingleResolutionPointIndexView<POINT>::_GetMeshQueryInterface(MeshQueryType queryType,
                                                                                                             Bentley::GeoCoordinates::BaseGCSPtr& targetGCS,
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

/*----------------------------------------------------------------------------+
|ScalableMeshSingleResolutionPointIndexView Method Definition Section - End
+----------------------------------------------------------------------------*/

DTMStatusInt IDTMVolume::ComputeVolumeCutAndFill(double& cut, double& fill, double& area, PolyfaceHeader& intersectingMeshSurface, DRange3d& meshRange, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
    {
    return _ComputeVolumeCutAndFill(cut, fill, area, intersectingMeshSurface,meshRange, volumeMeshVector);
    }

DTMStatusInt IDTMVolume::ComputeVolumeCutAndFill(PolyfaceHeaderPtr& terrainMesh, double& cut, double& fill, PolyfaceHeader& mesh, bool is2d, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
    {
    return _ComputeVolumeCutAndFill(terrainMesh, cut, fill, mesh, is2d, volumeMeshVector);
    }
template class ScalableMesh<IDTMFile::Point3d64f>;
//template class ScalableMesh<IDTMFile::Point3d64fM64f>;
//template class ScalableMeshSingleResolutionPointIndexView<IDTMFile::Point3d64f>;
//template class ScalableMeshSingleResolutionPointIndexView<IDTMFile::Point3d64fM64f>;

END_BENTLEY_SCALABLEMESH_NAMESPACE
