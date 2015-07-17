/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTM.cpp $
|    $RCSfile: MrDTM.cpp,v $
|   $Revision: 1.106 $
|       $Date: 2012/01/06 16:30:15 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
  
#include <ScalableTerrainModelPCH.h>

extern bool   GET_HIGHEST_RES;

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include "MrDTM.h"
#include "MrDTMQuadTreeBCLIBFilters.h"
#include "MrDTMQuadTreeQueries.h"

#include <ScalableTerrainModel/GeoCoords/GCS.h>
#include <STMInternal/GeoCoords/WKTUtils.h>

#include "MrDTMQuery.h"

#include <ScalableTerrainModel/IMrDTMPolicy.h>

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

USING_NAMESPACE_BENTLEY_MRDTM_IMPORT
USING_NAMESPACE_BENTLEY_MRDTM_GEOCOORDINATES

BEGIN_BENTLEY_MRDTM_NAMESPACE 

#define USE_CODE_FOR_ERROR_DETECTION

struct ToBcPtConverter
    {
    DPoint3d operator () (const IDTMFile::Point3d64fM64f& inputPt) const
        {
        DPoint3d outPt = {inputPt.x, inputPt.y, inputPt.z};
        // DPoint3d outPt = {inputPt.x, inputPt.y, 0};
        return outPt;
        }
    
    DPoint3d operator () (const IDTMFile::Point3d64f& inputPt) const
        {
        DPoint3d outPt = {inputPt.x, inputPt.y, inputPt.z};
        // DPoint3d outPt = {inputPt.x, inputPt.y, 0};
        return outPt;
        }

    DPoint3d operator () (const HGF3DCoord<double>& inputPt) const
        {   
        DPoint3d outPt = {inputPt.GetX(), inputPt.GetY(), inputPt.GetZ()};
        //  DPoint3d outPt = {inputPt.GetX(), inputPt.GetY(), 0};
        return outPt;
        }
    };

namespace {

/*----------------------------------------------------------------------------+
| Pool Singleton
+----------------------------------------------------------------------------*/
template <typename POINT> static HFCPtr<HPMCountLimitedPool<POINT> > PoolSingleton()
    {
    static HFCPtr<HPMCountLimitedPool<POINT> > pGlobalPointPool(new HPMCountLimitedPool<POINT>(new HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment(100, 2000 * sizeof(POINT)), 300000));

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
|IMrDTM Method Definition Section - Begin
+----------------------------------------------------------------------------*/
int IMrDTM::GenerateSubResolutions()
    {
    return _GenerateSubResolutions();
    }
            
__int64 IMrDTM::GetBreaklineCount() const
    {
    return _GetBreaklineCount();
    }
 
MrDTMCompressionType IMrDTM::GetCompressionType() const
    {
    return _GetCompressionType();
    }

int IMrDTM::GetNbResolutions(DTMQueryDataType queryDataType) const
    {
    return _GetNbResolutions(queryDataType);
    }

IMrDTMQueryPtr IMrDTM::GetQueryInterface(DTMQueryType     queryType, 
                                         DTMQueryDataType queryDataType) const
    {
    return _GetQueryInterface(queryType, queryDataType);
    }

IMrDTMQueryPtr IMrDTM::GetQueryInterface(DTMQueryType                         queryType, 
                                         DTMQueryDataType                     queryDataType, 
                                         Bentley::GeoCoordinates::BaseGCSPtr& targetGCS,
                                         const DRange3d& extentInTargetGCS) const
    {
    return _GetQueryInterface(queryType, queryDataType, targetGCS, extentInTargetGCS);
    }



const BaseGCSPtr& IMrDTM::GetBaseGCS() const 
    {
    return _GetGCS().GetGeoRef().GetBasePtr();
    }

StatusInt IMrDTM::SetBaseGCS(const BaseGCSPtr& baseGCSPtr)
    {
    GCSFactory::Status createStatus = GCSFactory::S_SUCCESS;
    GCS gcs(GetGCSFactory().Create(baseGCSPtr, createStatus));
    if (GCSFactory::S_SUCCESS != createStatus)
        return BSIERROR;

    return _SetGCS(gcs);
    }


const GeoCoords::GCS& IMrDTM::GetGCS() const
    {
    return _GetGCS();
    }

StatusInt IMrDTM::SetGCS(const GeoCoords::GCS& gcs)
    {
    return _SetGCS(gcs);
    }

MrDTMState IMrDTM::GetState() const
    {
    return _GetState();
    }

bool IMrDTM::IsProgressive() const
    {
    return _IsProgressive();
    }

bool IMrDTM::IsReadOnly() const
    {
    return _IsReadOnly();
    }

bool IMrDTM::IsShareable() const
    {
    return _IsShareable();
    }

bool IMrDTM::InSynchWithSources() const
    {
    return _InSynchWithSources();
    }

bool IMrDTM::LastSynchronizationCheck(time_t& lastCheckTime) const
    {
    return _LastSynchronizationCheck(lastCheckTime);
    }

int IMrDTM::SynchWithSources()
    {
    return _SynchWithSources();
    } 

int IMrDTM::GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, Bentley::GeoCoordinates::BaseGCSPtr& targetGCS) const
    {
    return _GetRangeInSpecificGCS(lowPt, highPt, targetGCS);
    }


/*----------------------------------------------------------------------------+
|IMrDTM Method Definition Section - End
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
|IMrDTM::GetFor
+----------------------------------------------------------------------------*/
IMrDTMPtr IMrDTM::GetFor   (const WChar*          filePath,
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
        return MrDTM<IDTMFile::Point3d64f>::Open(iDTMFilePtr, filePath, status);
        }

    const IDTMFile::LayerDir* layerDir = iDTMFilePtr->GetRootDir()->GetLayerDir (workingLayer);
    if (0 == layerDir)
        {
        status = BSIERROR;
        return 0; // Error accessing working layer
        }

    if (!layerDir->HasUniformFeatureDir(DTMPointTaggedTileStore<IDTMFile::Point3d64f, YProtPtExtentType>::MASS_POINT_FEATURE_TYPE))
        {
        //MST : The point type might be incorrect depending on the filtering chosen. 
        //      The point type should probably be stored in the layer.
        return MrDTM<IDTMFile::Point3d64f>::Open(iDTMFilePtr, filePath, status);
        }

    const IDTMFile::UniformFeatureDir* featureDir = 
        layerDir->GetUniformFeatureDir(DTMPointTaggedTileStore<IDTMFile::Point3d64f, YProtPtExtentType>::MASS_POINT_FEATURE_TYPE);

    if (0 == featureDir)
        {
        status = BSIERROR;
        return 0; // Error accessing specified feature dir
        }

    IMrDTMPtr iMrDtmPtr;

    const IDTMFile::PointTypeID ptType = featureDir->GetPointType();
    switch(ptType)
        {
        case IDTMFile::POINT_TYPE_XYZf64 :                   
            iMrDtmPtr = MrDTM<IDTMFile::Point3d64f>::Open(iDTMFilePtr, filePath, status);
            break;
        case IDTMFile::POINT_TYPE_XYZMf64 :
            iMrDtmPtr = MrDTM<IDTMFile::Point3d64fM64f>::Open(iDTMFilePtr, filePath, status);
            break;
        default : 
            status = BSIERROR;
            assert(!"This point type is not yet supported");
            break;
        }    
                    
    return iMrDtmPtr;   
    }


/*----------------------------------------------------------------------------+
|IMrDTM::GetFor
+----------------------------------------------------------------------------*/
IMrDTMPtr IMrDTM::GetFor   (const WChar*          filePath,
                            bool                    openReadOnly,
                            bool                    openShareable)
    {
    StatusInt status;
    return GetFor(filePath, openReadOnly, openShareable, status);
    }

/*----------------------------------------------------------------------------+
|IMrDTM::GetCountInRange
+----------------------------------------------------------------------------*/
Count IMrDTM::GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const
    {
    return _GetCountInRange(range, type, maxNumberCountedPoints);
    }


/*----------------------------------------------------------------------------+
|MrDTMBase::MrDTMBase
+----------------------------------------------------------------------------*/
MrDTMBase::MrDTMBase   (IDTMFile::File::Ptr&            filePtr, 
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
|MrDTMBase::~MrDTMBase
+----------------------------------------------------------------------------*/
MrDTMBase::~MrDTMBase ()
    {
    }

/*----------------------------------------------------------------------------+
|MrDTMBase::GetIDTMFile
+----------------------------------------------------------------------------*/
const IDTMFile::File::Ptr& MrDTMBase::GetIDTMFile () const
    {
    return m_iDTMFilePtr;
    }

/*----------------------------------------------------------------------------+
|MrDTMBase::GetPath
+----------------------------------------------------------------------------*/
const WChar* MrDTMBase::GetPath () const
    {
    return m_path.c_str();
    }

/*----------------------------------------------------------------------------+
|MrDTMBase::LoadGCSFrom
+----------------------------------------------------------------------------*/
bool MrDTMBase::LoadGCSFrom (const IDTMFile::LayerDir& layerDir)
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
|MrDTM::MrDTM
+----------------------------------------------------------------------------*/
template <class POINT> MrDTM<POINT>::MrDTM(IDTMFile::File::Ptr& iDTMFilePtr, const WString& path)
    :   MrDTMBase(iDTMFilePtr, path),
        m_areDataCompressed(false),
        m_computeTileBoundary(false),
        m_minScreenPixelsPerPoint(MEAN_SCREEN_PIXELS_PER_POINT)
    {    

    }

/*----------------------------------------------------------------------------+
|MrDTM::~MrDTM
+----------------------------------------------------------------------------*/
template <class POINT> MrDTM<POINT>::~MrDTM()
    {
    Close();
    }

template <class POINT> unsigned __int64 MrDTM<POINT>::CountPointsInExtent(YProtPtExtentType& extent, 
                                                                          HFCPtr<HGFPointIndexNode<POINT, YProtPtExtentType>> nodePtr,
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

template <class POINT> unsigned __int64 MrDTM<POINT>::CountLinearsInExtent(YProtFeatureExtentType& extent, unsigned __int64 maxFeatures) const
{
    list<HFCPtr<HVEDTMLinearFeature>> featureList, cutFeatureList;
	m_mrDTMLinearIndexPtr->GetIn(extent, featureList);
    // Now we must cut the linears to the extent
    int nPts(5);                     
    HArrayAutoPtr<double> tmpPts (new double[nPts * 2]);

    tmpPts[0] = extent.GetXMin();
    tmpPts[1] = extent.GetYMin();
    tmpPts[2] = extent.GetXMin();
    tmpPts[3] = extent.GetYMax();
    tmpPts[4] = extent.GetXMax();
    tmpPts[5] = extent.GetYMax();
    tmpPts[6] = extent.GetXMax();
    tmpPts[7] = extent.GetYMin();
    tmpPts[8] = tmpPts[0];
    tmpPts[9] = tmpPts[1];

    HFCPtr<HGF2DCoordSys> pCoordSys(new HGF2DCoordSys());                                            
    HFCPtr<HVE2DPolygonOfSegments> queryPolyLine = new HVE2DPolygonOfSegments(nPts * 2, tmpPts, pCoordSys);                                

    CutLinears (featureList, cutFeatureList, queryPolyLine);

    unsigned __int64 nbLinearsInExtent = cutFeatureList.size();
	if(nbLinearsInExtent > maxFeatures) return nbLinearsInExtent;
    // Count the total number of points in the features list
	unsigned __int64 nbPoints = 0;
	for(list<HFCPtr<HVEDTMLinearFeature>>::iterator it = cutFeatureList.begin(); it != cutFeatureList.end() && nbPoints <= maxFeatures; it++)
	{
		nbPoints+=(*it)->GetSize();
	}
	return nbPoints;
};

template <class POINT> Count MrDTM<POINT>::_GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const
    {
    Count qty (0,0);
    switch (type)
        {
        case COUNTTYPE_POINTS:
        case COUNTTYPE_BOTH:
            {
            HFCPtr<HGFPointIndexNode<POINT, YProtPtExtentType>> nodePtr = m_mrDTMPointIndexPtr->GetRootNode();
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
|MrDTM::Open
+----------------------------------------------------------------------------*/
template <class POINT>
MrDTM<POINT>* MrDTM<POINT>::Open   (IDTMFile::File::Ptr&    filePtr, 
                                    const WString&     filePath, 
                                    StatusInt&              status)
    {
    auto_ptr<MrDTM<POINT>> mrDtmP(new MrDTM<POINT>(filePtr, filePath));                    

    status = mrDtmP->Open();
    return (BSISUCCESS == status ? mrDtmP.release() : 0);
    }

/*----------------------------------------------------------------------------+
|MrDTM::Open
+----------------------------------------------------------------------------*/
template <class POINT> int MrDTM<POINT>::Open()
    {       
    HPRECONDITION(m_iDTMFilePtr != 0);

    if (!m_iDTMFilePtr->GetRootDir()->HasLayerDir(m_workingLayer))
        return BSISUCCESS; // File is empty.

    IDTMFile::LayerDir* layerDir = m_iDTMFilePtr->GetRootDir()->GetLayerDir(m_workingLayer);
    if (0 == layerDir)
        return BSIERROR; // Could not access existing layer

    try 
        {       
        if (!LoadGCSFrom(*layerDir))
            return BSIERROR; // Error loading layer gcs

        const bool hasPoints = layerDir->HasUniformFeatureDir(DTMPointTaggedTileStore<POINT, YProtPtExtentType>::MASS_POINT_FEATURE_TYPE);
        const bool hasFeatures = layerDir->CountMixedFeatureDirs() > 0;

        HFCPtr<TileStoreType> pTileStore;

        if (hasPoints)
            {    
            HFCPtr<HPMCountLimitedPool<POINT> > pMemoryPool(PoolSingleton<POINT>());

            IDTMFile::UniformFeatureDir* featureDir =
                layerDir->GetUniformFeatureDir(DTMPointTaggedTileStore<POINT, YProtPtExtentType>::MASS_POINT_FEATURE_TYPE);  

            if (0 == featureDir)
                return BSIERROR;

            //MST                                              
            auto_ptr<IHGFPointIndexFilter<POINT, YProtPtExtentType>> filterP(CreatePointIndexFilter(featureDir));

            if (0 == filterP.get())
                return BSIERROR;

            m_areDataCompressed = (featureDir->GetPointCompressType().GetType() != HTGFF::Compression::TYPE_NONE);

            pTileStore = new TileStoreType(m_iDTMFilePtr, HFC_READ_ONLY | HFC_SHARE_READ_ONLY, AreDataCompressed());
   
            m_mrDTMPointIndexPtr = new PointIndexType(pMemoryPool, 
                                                      &*pTileStore, 
                                                      40000, 
                                                      filterP.release(),
                                                      true, 
                                                      false);  
            }
                       
        if (hasFeatures)
            {
            if (pTileStore == 0)
                pTileStore = new TileStoreType(m_iDTMFilePtr, HFC_READ_ONLY | HFC_SHARE_READ_ONLY, AreDataCompressed());
           
            m_mrDTMLinearIndexPtr = new LinearIndexType (&*pTileStore, 400, false);
            }


        m_contentExtent = ComputeTotalExtentFor(&*m_mrDTMPointIndexPtr, &*m_mrDTMLinearIndexPtr);


        return BSISUCCESS;  
        }
    catch(...)
        {
        return BSIERROR;             
        }
    }

/*----------------------------------------------------------------------------+
|MrDTM::Close
+----------------------------------------------------------------------------*/    
template <class POINT> int MrDTM<POINT>::Close
(
)
    {
    m_mrDTMPointIndexPtr = 0;   
    m_mrDTMLinearIndexPtr = 0;   
    m_iDTMFilePtr = 0;
    
    return SUCCESS;        
    }


template <class POINT>
DRange3d MrDTM<POINT>::ComputeTotalExtentFor   (const PointIndexType*   pointIndexP,
                                                const LinearIndexType*  linearIndexP)
    {
    typedef ExtentOp<YProtPtExtentType>         PtExtentOpType;
    typedef ExtentOp<YProtFeatureExtentType>    LinExtentOpType;

    DRange3d totalExtent;
    memset(&totalExtent, 0, sizeof(totalExtent));

    if ((pointIndexP != 0) && (!pointIndexP->IsEmpty()) && (linearIndexP != 0) && (!linearIndexP->IsEmpty()))
        {
        YProtPtExtentType ExtentPoints = pointIndexP->GetContentExtent();
        YProtFeatureExtentType ExtentFeatures = linearIndexP->GetContentExtent();

        totalExtent.low.x = min(PtExtentOpType::GetXMin(ExtentPoints), LinExtentOpType::GetXMin(ExtentFeatures));
        totalExtent.high.x = max(PtExtentOpType::GetXMax(ExtentPoints), LinExtentOpType::GetXMax(ExtentFeatures));
        totalExtent.low.y = min(PtExtentOpType::GetYMin(ExtentPoints), LinExtentOpType::GetYMin(ExtentFeatures));
        totalExtent.high.y = max(PtExtentOpType::GetYMax(ExtentPoints), LinExtentOpType::GetYMax(ExtentFeatures));
        totalExtent.low.z = min(PtExtentOpType::GetZMin(ExtentPoints), LinExtentOpType::GetZMin(ExtentFeatures));
        totalExtent.high.z = max(PtExtentOpType::GetZMax(ExtentPoints), LinExtentOpType::GetZMax(ExtentFeatures));
        }
    else if ((pointIndexP != 0) && (!pointIndexP->IsEmpty()))
        {
        YProtPtExtentType ExtentPoints = pointIndexP->GetContentExtent();
        totalExtent.low.x = PtExtentOpType::GetXMin(ExtentPoints);
        totalExtent.high.x = PtExtentOpType::GetXMax(ExtentPoints);
        totalExtent.low.y = PtExtentOpType::GetYMin(ExtentPoints);
        totalExtent.high.y = PtExtentOpType::GetYMax(ExtentPoints);
        totalExtent.low.z = PtExtentOpType::GetZMin(ExtentPoints);
        totalExtent.high.z = PtExtentOpType::GetZMax(ExtentPoints);
        }
    else if ((linearIndexP != 0) && (!linearIndexP->IsEmpty()))
        {
        YProtFeatureExtentType ExtentFeatures = linearIndexP->GetContentExtent();
        totalExtent.low.x = LinExtentOpType::GetXMin(ExtentFeatures);
        totalExtent.high.x = LinExtentOpType::GetXMax(ExtentFeatures);
        totalExtent.low.y = LinExtentOpType::GetYMin(ExtentFeatures);
        totalExtent.high.y = LinExtentOpType::GetYMax(ExtentFeatures);
        totalExtent.low.z = LinExtentOpType::GetZMin(ExtentFeatures);
        totalExtent.high.z = LinExtentOpType::GetZMax(ExtentFeatures);
        }

    return totalExtent;
    }


/*----------------------------------------------------------------------------+
|MrDTM::CreatePointIndexFilter
+----------------------------------------------------------------------------*/
template <class POINT> IHGFPointIndexFilter<POINT, YProtPtExtentType>* MrDTM<POINT>::CreatePointIndexFilter(IDTMFile::UniformFeatureDir* pointDirPtr) const
    {
    HPRECONDITION(pointDirPtr != 0);

    const IDTMFile::FilteringDir* filteringDir = pointDirPtr->GetFilteringDir();
    IDTMFile::SpatialIndexDir* indexDirP = pointDirPtr->GetSpatialIndexDir();

    if (0 == filteringDir || 0 == indexDirP)
        return 0;

    BTreeIndexHandler::Ptr indexHandler = BTreeIndexHandler::CreateFrom(indexDirP);

    if (0 == indexHandler)
        return 0;
 

    IHGFPointIndexFilter<POINT, YProtPtExtentType>* pFilter = 0;                            
    IDTMFile::FilterType filteringType = filteringDir->GetType();

    if (indexHandler->IsProgressive())
        {
        switch (filteringType)
            {
            case IDTMFile::FILTER_TYPE_DUMB :
                pFilter = new MrDTMQuadTreeBCLIBProgressiveFilter1<POINT, YProtPtExtentType>();                           
                break;
            case IDTMFile::FILTER_TYPE_TILE :
                pFilter = new MrDTMQuadTreeBCLIBProgressiveFilter2<POINT, YProtPtExtentType>();      
                break;
            case IDTMFile::FILTER_TYPE_TIN :                     
                pFilter = new MrDTMQuadTreeBCLIBProgressiveFilter3<POINT, YProtPtExtentType>();      
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
                pFilter = new MrDTMQuadTreeBCLIBFilter1<POINT, YProtPtExtentType>();                           
                break;
            case FILTER_TYPE_TILE :
                pFilter = new MrDTMQuadTreeBCLIBFilter2<POINT, YProtPtExtentType>();      
                break;
            case FILTER_TYPE_TIN :                     
                pFilter = new MrDTMQuadTreeBCLIBFilter3<POINT, YProtPtExtentType>();      
                break;
            default : 
                assert(0);
            }               
        }        

    return pFilter;
    }


// TDORAY: This is a duplicate of the version in file creator. Try to eliminate this redundancy.
struct ToBcPtConverter;

/*----------------------------------------------------------------------------+
|MrDTM::ComputeTileBoundaryDuringQuery
+----------------------------------------------------------------------------*/
template <class POINT> void MrDTM<POINT>::ComputeTileBoundaryDuringQuery(bool doCompute)
    {
    m_computeTileBoundary = doCompute;
    }

/*----------------------------------------------------------------------------+
|MrDTM::SetMinScreenPixelsPerPoint
+----------------------------------------------------------------------------*/
template <class POINT> void MrDTM<POINT>::SetMinScreenPixelsPerPoint(double pi_minScreenPixelsPerPoint)
    {    
    m_minScreenPixelsPerPoint = pi_minScreenPixelsPerPoint;   
    }

/*----------------------------------------------------------------------------+
|MrDTM::GetMinScreenPixelsPerPoint
+----------------------------------------------------------------------------*/
template <class POINT> double MrDTM<POINT>::GetMinScreenPixelsPerPoint()
    {
    return m_minScreenPixelsPerPoint;
    }

/*----------------------------------------------------------------------------+
|MrDTM::AreDataCompressed
+----------------------------------------------------------------------------*/
template <class POINT> bool MrDTM<POINT>::AreDataCompressed()
    {
    assert(m_iDTMFilePtr != 0);
    
    return m_areDataCompressed;
    }

/*----------------------------------------------------------------------------+
|MrDTM::CreateSpatialIndexFromExtents
+----------------------------------------------------------------------------*/
template <class POINT> void MrDTM<POINT>::CreateSpatialIndexFromExtents(list<HVE2DSegment>& pi_rpBreaklineList, 
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

        UInt32 TileNumber = 0;
          
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
|MrDTM::AddBreaklineSet
+----------------------------------------------------------------------------*/
template <class POINT> void MrDTM<POINT>::AddBreaklineSet(list<HFCPtr<HVEDTMLinearFeature> >& breaklineList, 
                                                          BC_DTM_OBJ*                         po_pBcDtmObj)
    {   
    int status;

    if (breaklineList.size() > 0)
        {        
        list<HFCPtr<HVEDTMLinearFeature> >::iterator BreaklineIter    = breaklineList.begin();
        list<HFCPtr<HVEDTMLinearFeature> >::iterator BreaklineIterEnd = breaklineList.end();

        UInt32 TileNumber = 0;

        while (BreaklineIter != BreaklineIterEnd) 
            {
            DPoint3d*     linePts = new DPoint3d[(*BreaklineIter)->GetSize()];        

            for (size_t indexPoints = 0 ; indexPoints < (*BreaklineIter)->GetSize(); indexPoints++)
                {
                linePts[indexPoints].x = (*BreaklineIter)->GetPoint(indexPoints).GetX();
                linePts[indexPoints].y = (*BreaklineIter)->GetPoint(indexPoints).GetY();
                linePts[indexPoints].z = (*BreaklineIter)->GetPoint(indexPoints).GetZ();
                }

            status = bcdtmObject_storeDtmFeatureInDtmObject(po_pBcDtmObj, (DTMFeatureType)(*BreaklineIter)->GetFeatureType() /*DTMFeatureType::Breakline*/, TileNumber, 1, &po_pBcDtmObj->nullFeatureId, linePts, (UInt32)(*BreaklineIter)->GetSize());
            BreaklineIter++;
            TileNumber++;
            }         
        }
    }

/*-------------------Methods inherited from IDTM-----------------------------*/

/*----------------------------------------------------------------------------+
|MrDTM::_GetDTMDraping
+----------------------------------------------------------------------------*/
template <class POINT> IDTMDrapingP MrDTM<POINT>::_GetDTMDraping() 
    {
    assert(0);
    return 0; //Not supported yet
    }

template <class POINT> IDTMDrainageP    MrDTM<POINT>::_GetDTMDrainage()
    {
    assert(0);
    return 0;
    }

template <class POINT> IDTMContouringP  MrDTM<POINT>::_GetDTMContouring()
    {
    assert(0);
    return 0;
    }

template <class POINT> DTMStatusInt MrDTM<POINT>::_CalculateSlopeArea (double&, double&, const DPoint3d*, int)
    {
    return DTM_ERROR;
    }

template <class POINT> bool MrDTM<POINT>::_GetTransformation(TransformR)
    {
    return true;
    }

template <class POINT> DTMStatusInt MrDTM<POINT>::_GetTransformDTM(DTMPtr& transformedDTM, TransformCR)
    {
    return DTM_ERROR;
    }

/*----------------------------------------------------------------------------+
|MrDTM::_GetPointCount
+----------------------------------------------------------------------------*/
template <class POINT> __int64 MrDTM<POINT>::_GetPointCount() 
    {        
    __int64 nbPoints = 0;

    if (m_mrDTMPointIndexPtr != 0)
        {
        nbPoints += m_mrDTMPointIndexPtr->GetCount();
        }
    
    //MST : Need to add the number of points in the linear index.    
    return nbPoints;
    }

/*----------------------------------------------------------------------------+
|MrDTM::_GetBreaklineCount
+----------------------------------------------------------------------------*/
template <class POINT> __int64 MrDTM<POINT>::_GetBreaklineCount() const
    {
    __int64 nbBreaklines = 0;

    if (m_mrDTMLinearIndexPtr != 0)
        {
      //  assert(!"Check if the nb of points is returned or the number of linear element");

        nbBreaklines += m_mrDTMLinearIndexPtr->GetCount();    

        }

    return nbBreaklines;
    }

/*----------------------------------------------------------------------------+
|MrDTM::_GetRange
+----------------------------------------------------------------------------*/
template <class POINT> DTMStatusInt MrDTM<POINT>::_GetRange(DRange3dR range) 
    {
    range = m_contentExtent;
    return DTM_SUCCESS;
    }



/*----------------------------------------------------------------------------+
|MrDTM::_GetBcDTM
+----------------------------------------------------------------------------*/
template <class POINT> BcDTMP MrDTM<POINT>::_GetBcDTM() 
    {    
    return 0;
    };
 
/*----------------------------------------------------------------------------+
|MrDTM::_GetBoundary
+----------------------------------------------------------------------------*/
template <class POINT> DTMStatusInt MrDTM<POINT>::_GetBoundary(DTMPointArray& result) 
    {
    return DTM_ERROR; 
    }

/*-------------------Methods inherited from IMRDTM-----------------------------*/

/*----------------------------------------------------------------------------+
|MrDTM::_GetCompressionType
+----------------------------------------------------------------------------*/
template <class POINT> int MrDTM<POINT>::_GenerateSubResolutions()
    {
    return ERROR;
    }

/*----------------------------------------------------------------------------+
|MrDTM::_GetCompressionType
+----------------------------------------------------------------------------*/
template <class POINT> MrDTMCompressionType MrDTM<POINT>::_GetCompressionType() const
    {
    return MRDTM_COMPRESSION_NONE;
    }
  
/*----------------------------------------------------------------------------+
|MrDTM::_GetQueryInterface
+----------------------------------------------------------------------------*/
template <class POINT> IMrDTMQueryPtr MrDTM<POINT>::_GetQueryInterface(DTMQueryType                         queryType, 
                                                                       DTMQueryDataType                     queryDataType, 
                                                                       Bentley::GeoCoordinates::BaseGCSPtr& targetBaseGCSPtr,
                                                                       const DRange3d&                       extentInTargetGCS) const
    {
    IMrDTMQueryPtr mrDtmQueryPtr;
                    
    GCS targetGCS = GetGCSFactory().Create(targetBaseGCSPtr);        

    _GetQueryInterface(queryType, queryDataType);    
    
    // TODO: Remove cast when smart pointers becomes const aware
    MrDTM<POINT>* UNCONST_THIS = const_cast<MrDTM<POINT>*>(this);

    mrDtmQueryPtr = MrDTMQuery::GetReprojectionQueryInterface(UNCONST_THIS, queryType, queryDataType, m_sourceGCS, targetGCS, extentInTargetGCS);
             
    return mrDtmQueryPtr;                
    }

/*----------------------------------------------------------------------------+
|MrDTM::_GetQueryInterface
+----------------------------------------------------------------------------*/
template <class POINT> IMrDTMQueryPtr MrDTM<POINT>::_GetQueryInterface(DTMQueryType queryType, DTMQueryDataType queryDataType) const
    {
    IMrDTMQueryPtr iMrDTMQueryPtr;

    if (queryDataType == DTM_QUERY_DATA_POINT)
        {
        if ((m_mrDTMPointIndexPtr != 0) && (m_mrDTMPointIndexPtr->IsEmpty() == false))
            {
            if (queryType == DTM_QUERY_FULL_RESOLUTION)
                {                                    
                //MST Should be done by using the highest resolution.
                //iMrDTMQueryPtr = new MrDTMFullResolutionQuery(m_mrDTMPointIndexPtr);   
                }
            else
            if (queryType == DTM_QUERY_VIEW_DEPENDENT)
                {                        
                iMrDTMQueryPtr = new MrDTMViewDependentPointQuery<POINT>(m_mrDTMPointIndexPtr);       
                }
            else
            if (queryType == DTM_QUERY_FIX_RESOLUTION_VIEW)
                {        
                iMrDTMQueryPtr = new MrDTMFixResolutionViewPointQuery<POINT>(m_mrDTMPointIndexPtr, m_sourceGCS);        
                }
            }
        }
    else
        {
        assert(queryDataType == DTM_QUERY_DATA_LINEAR);

        if ((m_mrDTMLinearIndexPtr != 0) && (m_mrDTMLinearIndexPtr->IsEmpty() == false))
            {
            if (queryType == DTM_QUERY_FULL_RESOLUTION)
                {                                                
                iMrDTMQueryPtr = new MrDTMFullResolutionLinearQuery(m_mrDTMLinearIndexPtr);       
                }
            }
        }
               
    return iMrDTMQueryPtr;
    }

/*----------------------------------------------------------------------------+
|MrDTM::_GetNbResolutions
+----------------------------------------------------------------------------*/
template <class POINT> int MrDTM<POINT>::_GetNbResolutions(DTMQueryDataType queryDataType) const
    {    
    int nbResolutions = 0;

    if (queryDataType == DTM_QUERY_DATA_POINT)
        {
        if (m_mrDTMPointIndexPtr != 0)
            {
            nbResolutions = (int)(m_mrDTMPointIndexPtr->GetDepth() + 1);
            }        
        }
    else
    if (queryDataType == DTM_QUERY_DATA_LINEAR)
        {
        if (m_mrDTMLinearIndexPtr != 0)
            {
            nbResolutions = (int)(m_mrDTMLinearIndexPtr->GetDepth() + 1);
            }            
        }

    return nbResolutions;    
    }

/*----------------------------------------------------------------------------+
|MrDTM::__GetSourceGCS
+----------------------------------------------------------------------------*/
template <class POINT> const GeoCoords::GCS& MrDTM<POINT>::_GetGCS() const
    {
    return m_sourceGCS;
    }




/*----------------------------------------------------------------------------+
|MrDTM::IsRangeValidInside
+----------------------------------------------------------------------------*/
template <class POINT> bool MrDTM<POINT>::IsValidInContextOf (const GCS& gcs) const
    {
    if (!gcs.HasGeoRef())
        return true;

    // Now we must validate that the change is possible. If the extent does not fall into the
    // valid domain range of the new GCS, an error will be returned.

    // Obtain the current range points
    DRange3d range;

    // NTERAY: Why isn't _GetRange a const method?
    const_cast<MrDTM<POINT>&>(*this)._GetRange(range);

    // Transform both points to latitude/longitude
    GeoPoint dumPoint;
    return BSISUCCESS == gcs.GetGeoRef().GetBase().LatLongFromCartesian(dumPoint, range.low) &&
           BSISUCCESS == gcs.GetGeoRef().GetBase().LatLongFromCartesian(dumPoint, range.high);
    }

/*----------------------------------------------------------------------------+
|MrDTM::_SetSourceGCS
+----------------------------------------------------------------------------*/
template <class POINT> StatusInt MrDTM<POINT>::_SetGCS(const GCS& newGCS)
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
template <class POINT> inline bool MrDTM<POINT>::IsEmpty () const
    {
    // NTERAY: Not sure if this test is required anymore... If not, we can move this
    //         method to base class.
    if (0 == m_mrDTMPointIndexPtr && 0 == m_mrDTMLinearIndexPtr)
        return true;

    return EqEps(m_contentExtent.low.x, m_contentExtent.high.x) &&
           EqEps(m_contentExtent.low.y, m_contentExtent.high.y) &&
           EqEps(m_contentExtent.low.z, m_contentExtent.high.z);
    }


/*----------------------------------------------------------------------------+
|MrDTM::_GetState
+----------------------------------------------------------------------------*/
template <class POINT> MrDTMState MrDTM<POINT>::_GetState() const
    {
    if (IsEmpty())
        return MRDTM_STATE_EMPTY;

    return _InSynchWithSources() ? MRDTM_STATE_UP_TO_DATE : MRDTM_STATE_DIRTY;
    }

/*----------------------------------------------------------------------------+
|MrDTM::_InSynchWithSources
+----------------------------------------------------------------------------*/
template <class POINT> bool MrDTM<POINT>::_InSynchWithSources() const
    {
    assert(0 != m_iDTMFilePtr);
    const SourcesDir* sourceDirPtr = m_iDTMFilePtr->GetRootDir()->GetSourcesDir();
    if (0 == sourceDirPtr)
        return false;

    const bool InSync = sourceDirPtr->GetLastModifiedTime() < sourceDirPtr->GetLastSyncTime();
    return InSync;    
    }

/*----------------------------------------------------------------------------+
|MrDTM::_GetRangeInSpecificGCS
+----------------------------------------------------------------------------*/
template <class POINT> int MrDTM<POINT>::_GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, Bentley::GeoCoordinates::BaseGCSPtr& targetGCS) const
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
|MrDTM::_LastSynchronizationCheck
+----------------------------------------------------------------------------*/
template <class POINT> bool MrDTM<POINT>::_LastSynchronizationCheck(time_t& lastCheckTime) const
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
|MrDTM::_SynchWithSources
+----------------------------------------------------------------------------*/
template <class POINT> int MrDTM<POINT>::_SynchWithSources()
    {
    assert(!"Not done yet");
    return false;
    }

/*----------------------------------------------------------------------------+
|MrDTM::_IsProgressive
+----------------------------------------------------------------------------*/
template <class POINT> bool MrDTM<POINT>::_IsProgressive() const
    {
    assert(!"Not done yet");
    return true;
    } 

/*----------------------------------------------------------------------------+
|MrDTM::_IsReadOnly
+----------------------------------------------------------------------------*/
template <class POINT> bool MrDTM<POINT>::_IsReadOnly() const
    {            
    return (m_iDTMFilePtr->GetAccessMode().m_HasWriteAccess == false) && 
           (m_iDTMFilePtr->GetAccessMode().m_HasWriteShare == false);
    } 

/*----------------------------------------------------------------------------+
|MrDTM::_IsShareable
+----------------------------------------------------------------------------*/
template <class POINT> bool MrDTM<POINT>::_IsShareable() const
    {    
    return (m_iDTMFilePtr->GetAccessMode().m_HasReadShare == false) && 
           (m_iDTMFilePtr->GetAccessMode().m_HasWriteShare == false);                
    } 

/*----------------------------------------------------------------------------+
|MrDTMSingleResolutionPointIndexView Method Definition Section - Begin
+----------------------------------------------------------------------------*/
template <class POINT> MrDTMSingleResolutionPointIndexView<POINT>::MrDTMSingleResolutionPointIndexView(HFCPtr<HGFPointIndex<POINT, YProtPtExtentType>>  mrDTMPointIndexPtr, 
                                                                                                       int                                              resolutionIndex, 
                                                                                                       GeoCoords::GCS                                   sourceGCS)
: m_sourceGCS(sourceGCS)
    {    
    m_mrDTMPointIndexPtr = mrDTMPointIndexPtr;
    m_resolutionIndex    = resolutionIndex;    
    }

template <class POINT> MrDTMSingleResolutionPointIndexView<POINT>::~MrDTMSingleResolutionPointIndexView()
    {
    } 

// Inherited from IDTM   
template <class POINT> __int64 MrDTMSingleResolutionPointIndexView<POINT>::_GetPointCount()
    {
    return m_mrDTMPointIndexPtr->GetNbObjectsAtLevel(m_resolutionIndex);
    }

template <class POINT> __int64 MrDTMSingleResolutionPointIndexView<POINT>::_GetBreaklineCount() const
    {
    assert(0);
    return 0;
    }

template <class POINT> IDTMDrapingP MrDTMSingleResolutionPointIndexView<POINT>::_GetDTMDraping()
    {
    assert(0);
    return 0;
    }

template <class POINT> IDTMDrainageP    MrDTMSingleResolutionPointIndexView<POINT>::_GetDTMDrainage()
    {
    assert(0);
    return 0;
    }

template <class POINT> IDTMContouringP  MrDTMSingleResolutionPointIndexView<POINT>::_GetDTMContouring()
    {
    assert(0);
    return 0;
    }

template <class POINT> DTMStatusInt MrDTMSingleResolutionPointIndexView<POINT>::_CalculateSlopeArea (double&, double&, const DPoint3d*, int)
    {
    return DTM_ERROR;
    }

template <class POINT> bool MrDTMSingleResolutionPointIndexView<POINT>::_GetTransformation(TransformR)
    {
    return true;
    }

template <class POINT> DTMStatusInt MrDTMSingleResolutionPointIndexView<POINT>::_GetTransformDTM(DTMPtr& transformedDTM, TransformCR)
    {
    return DTM_ERROR;
    }

template <class POINT> DTMStatusInt MrDTMSingleResolutionPointIndexView<POINT>::_GetRange(DRange3dR range)
    {        
    YProtPtExtentType ExtentPoints = m_mrDTMPointIndexPtr->GetContentExtent();

    range.low.x = ExtentOp<YProtPtExtentType>::GetXMin(ExtentPoints);
    range.high.x = ExtentOp<YProtPtExtentType>::GetXMax(ExtentPoints);
    range.low.y = ExtentOp<YProtPtExtentType>::GetYMin(ExtentPoints);
    range.high.y = ExtentOp<YProtPtExtentType>::GetYMax(ExtentPoints);
    range.low.z = ExtentOp<YProtPtExtentType>::GetZMin(ExtentPoints);
    range.high.z = ExtentOp<YProtPtExtentType>::GetZMax(ExtentPoints);   
    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------------+
|MrDTMSingleResolutionPointIndexView::_GetRangeInSpecificGCS
+----------------------------------------------------------------------------*/
template <class POINT> int MrDTMSingleResolutionPointIndexView<POINT>::_GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, Bentley::GeoCoordinates::BaseGCSPtr& targetGCS) const
    {
    StatusInt status = SUCCESS;

    YProtPtExtentType ExtentPoints = m_mrDTMPointIndexPtr->GetContentExtent();

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

template <class POINT> BcDTMP MrDTMSingleResolutionPointIndexView<POINT>::_GetBcDTM()
    {
    assert(0);
    return 0;
    }

template <class POINT> DTMStatusInt MrDTMSingleResolutionPointIndexView<POINT>::_GetBoundary(DTMPointArray&)
    {
    assert(0);
    return DTM_ERROR;
    }

// Inherited from IMRDTM     
template <class POINT> Count MrDTMSingleResolutionPointIndexView<POINT>::_GetCountInRange (const DRange2d& range, 
                                                                                           const CountType& type, 
                                                                                           const unsigned __int64& maxNumberCountedPoints) const
    {
    // Not implemented
    assert(false);
    return Count(0,0);
    }

template <class POINT> int MrDTMSingleResolutionPointIndexView<POINT>::_GenerateSubResolutions()
    {  
    assert(0);
    return ERROR;
    }

template <class POINT> MrDTMCompressionType MrDTMSingleResolutionPointIndexView<POINT>::_GetCompressionType() const
    {
    assert(0);
    return MRDTM_COMPRESSION_NONE;
    }

template <class POINT> int MrDTMSingleResolutionPointIndexView<POINT>::_GetNbResolutions(DTMQueryDataType queryDataType) const
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

template <class POINT> IMrDTMQueryPtr MrDTMSingleResolutionPointIndexView<POINT>::_GetQueryInterface(DTMQueryType                         queryType, 
                                                                                                     DTMQueryDataType                     queryDataType, 
                                                                                                     Bentley::GeoCoordinates::BaseGCSPtr& targetGCS,
                                                                                                     const DRange3d&                      extentInTargetGCS) const
    {                   
    GeoCoords::GCS targetGeoCoord(GetGCSFactory().Create(targetGCS));           
   
    // TODO: Remove once smart pointers are const aware
    MrDTMSingleResolutionPointIndexView<POINT>* UNCONST_THIS = const_cast<MrDTMSingleResolutionPointIndexView<POINT>*>(this);

    IMrDTMQueryPtr mrDtmQueryPtr = MrDTMQuery::GetReprojectionQueryInterface(UNCONST_THIS, queryType, queryDataType, m_sourceGCS, targetGeoCoord, extentInTargetGCS);

    return mrDtmQueryPtr;                
    }

template <class POINT> IMrDTMQueryPtr MrDTMSingleResolutionPointIndexView<POINT>::_GetQueryInterface(DTMQueryType     queryType, 
                                                                                                     DTMQueryDataType queryDataType) const
    {    
    IMrDTMQueryPtr iMrDTMQueryPtr;

    if (queryDataType == DTM_QUERY_DATA_POINT && (m_mrDTMPointIndexPtr != 0) && (m_mrDTMPointIndexPtr->IsEmpty() == false))
        {    
        if (queryType == DTM_QUERY_FULL_RESOLUTION)
            {
            iMrDTMQueryPtr =  new MrDTMFullResolutionPointQuery<POINT>(m_mrDTMPointIndexPtr, 
                                                                       m_resolutionIndex);
            }
        else
        if (queryType == DTM_QUERY_VIEW_DEPENDENT)
            {
            iMrDTMQueryPtr =  new MrDTMViewDependentPointQuery<POINT>(m_mrDTMPointIndexPtr);       
            }

        //DTM_QUERY_FIX_RESOLUTION_VIEW is meaningless for a single resolution view.         
        }
        
    return iMrDTMQueryPtr;
    }

template <class POINT> const GeoCoords::GCS& MrDTMSingleResolutionPointIndexView<POINT>::_GetGCS() const
    {
    return m_sourceGCS;
    }

template <class POINT> StatusInt MrDTMSingleResolutionPointIndexView<POINT>::_SetGCS(const GeoCoords::GCS& sourceGCS)
    {
    assert(!"Not done yet");

    return -1;   
    }

template <class POINT> bool MrDTMSingleResolutionPointIndexView<POINT>::_IsProgressive() const
    {
    assert(0);
    return false;
    }

template <class POINT> bool MrDTMSingleResolutionPointIndexView<POINT>::_IsReadOnly() const
    {    
    //MS Access of the file 
    assert(!"Not done yet");
    return true;
    }

template <class POINT> bool MrDTMSingleResolutionPointIndexView<POINT>::_IsShareable() const
    {    
    //MS Access of the file 
    assert(!"Not done yet");
    return false;
    }

template <class POINT> MrDTMState MrDTMSingleResolutionPointIndexView<POINT>::_GetState() const
    {
    assert(!"Not done yet");
    return MRDTM_STATE_EMPTY; 
    }

template <class POINT> bool MrDTMSingleResolutionPointIndexView<POINT>::_InSynchWithSources() const
    {    
    assert(!"Should not be called");
    return false;
    }

template <class POINT> bool MrDTMSingleResolutionPointIndexView<POINT>::_LastSynchronizationCheck(time_t& lastCheckTime) const
    {
    assert(!"Should not be called");
    return false;       
    }

template <class POINT> int MrDTMSingleResolutionPointIndexView<POINT>::_SynchWithSources()
    {
    assert(!"Should not be called");
    return -1;       
    }

/*----------------------------------------------------------------------------+
|MrDTMSingleResolutionPointIndexView Method Definition Section - End
+----------------------------------------------------------------------------*/

template class MrDTM<IDTMFile::Point3d64f>;
//template class MrDTM<IDTMFile::Point3d64fM64f>;
//template class MrDTMSingleResolutionPointIndexView<IDTMFile::Point3d64f>;
//template class MrDTMSingleResolutionPointIndexView<IDTMFile::Point3d64fM64f>;

END_BENTLEY_MRDTM_NAMESPACE
