/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMCreator.cpp $
|    $RCSfile: MrDTMCreator.cpp,v $
|   $Revision: 1.90 $
|       $Date: 2012/01/27 16:45:29 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableTerrainModelPCH.h>

#include "MrDTMCreator.h"

#include <ScalableTerrainModel/GeoCoords/GCS.h>

/*----------------------------------------------------------------------+
| Include internal dependencies                                         |
+----------------------------------------------------------------------*/
#include <STMInternal/Foundations/PrivateStringTools.h>

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include "MrDTM.h"
#include <ScalableTerrainModel/MrDTMDefs.h>

#include <ScalableTerrainModel/Memory/Allocation.h>
#include <ScalableTerrainModel/Import/SourceReference.h>

#include <ScalableTerrainModel/Import/Source.h>
#include <ScalableTerrainModel/Import/Importer.h>

#include <ScalableTerrainModel/Import/Config/All.h>

#include <ScalableTerrainModel/Import/Error/Source.h>

#include "../Import/Sink.h"
#include "MrDTMSourcesImport.h"

#include "MrDTMQuadTreeBCLIBFilters.h"
#include <ScalableTerrainModel/GeoCoords/Reprojection.h>
#include "MrDTMTime.h"
#include "MrDTMSourcesPersistance.h"
#include <ScalableTerrainModel/IMrDTMSourceImportConfig.h>
#include <ScalableTerrainModel/IMrDTMDocumentEnv.h>
#include <ScalableTerrainModel/IMrDTMSourceVisitor.h>

#include "Plugins/MrDTMClipMaskFilterFactory.h"
#include "Plugins/MrDTMIDTMFileTraits.h"

#include "MrDTMStorage.h"
#include <ScalableTerrainModel/IMrDTMStream.h>

#include <ScalableTerrainModel/IMrDTMPolicy.h>

#include "MrDTMSources.h"

#include "InternalUtilityFunctions.h"
#include <STMInternal/GeoCoords/WKTUtils.h>

/*----------------------------------------------+
| Constant definitions                          |
+----------------------------------------------*/

/*----------------------------------------------+
| Private type definitions                      |
+----------------------------------------------*/
#define MAX_NUM_POINTS_FOR_LINEAR_OVERVIEW 15000

/*==================================================================*/
/*                                                                  */
/*          INTERNAL FUNCTIONS                                      */
/*                                                                  */
/*==================================================================*/

USING_NAMESPACE_BENTLEY_MRDTM_IMPORT
USING_NAMESPACE_BENTLEY_MRDTM_GEOCOORDINATES

BEGIN_BENTLEY_MRDTM_NAMESPACE

namespace {

const size_t DEFAULT_WORKING_LAYER = 0;


//MS Duplicate code should be removed.
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


inline bool fileExist(const WChar* fileName)
    {
    ifstream file(fileName, ios::in | ios::binary);
    return file.good() && (char_traits<char>::eof() != file.peek());
    }


inline const GCS& GetDefaultGCS ()
    {
    static const GCS DEFAULT_GCS(GetGCSFactory().Create(GeoCoords::Unit::GetMeter()));
    return DEFAULT_GCS;
    }



}


/*==================================================================*/
/*        MRDTM CREATOR SECTION - BEGIN                             */
/*==================================================================*/


IMrDTMCreatorPtr IMrDTMCreator::GetFor (const WChar*  filePath,
                                        StatusInt&      status)
    {
    using namespace IDTMFile;

    if (!fileExist(filePath))
        {
        status = BSISUCCESS;
        return new IMrDTMCreator(new Impl(filePath)); // Return early. File does not exist.
        }

    IMrDTMCreatorPtr pCreator = new IMrDTMCreator(new Impl(filePath));

    status = pCreator->m_implP->LoadFromFile();
    if (BSISUCCESS != status)
        return 0;

    return pCreator.get();
    }


IMrDTMCreatorPtr IMrDTMCreator::GetFor (const IMrDTMPtr&    mrDTMPtr,
                                        StatusInt&          status)
    {
    using namespace IDTMFile;

    IMrDTMCreatorPtr pCreator = new IMrDTMCreator(new Impl(mrDTMPtr));

    status = pCreator->m_implP->LoadFromFile();
    if (BSISUCCESS != status)
        return 0;

    return pCreator.get();
    }


IMrDTMCreatorPtr IMrDTMCreator::GetFor (const WChar* filePath)
    {
    StatusInt dummyStatus;
    return GetFor(filePath, dummyStatus);
    }

IMrDTMCreatorPtr IMrDTMCreator::GetFor (const IMrDTMPtr& mrDTMPtr)
    {
    StatusInt dummyStatus;
    return GetFor(mrDTMPtr, dummyStatus);
    }


IMrDTMCreator::IMrDTMCreator (Impl* implP)
    :   m_implP(implP)
    {
    }


IMrDTMCreator::~IMrDTMCreator ()
    {
    }

bool IMrDTMCreator::AreAllSourcesReachable () const
    {
    // Reachable only if all sources are reachable
    return m_implP->m_sources.End() == std::find_if(m_implP->m_sources.Begin(), m_implP->m_sources.End(), not1(mem_fun_ref(&IDTMSource::IsReachable)));
    }

StatusInt IMrDTMCreator::Create ()
    {
    return m_implP->CreateMrDTM();
    }



const Bentley::GeoCoordinates::BaseGCSPtr& IMrDTMCreator::GetBaseGCS () const
    {
    return GetAdvancedGCS().GetGeoRef().GetBasePtr();
    }

const GeoCoords::GCS& IMrDTMCreator::Impl::GetGCS () const
    {
    if (0 != m_mrDTMPtr.get())
        return m_mrDTMPtr->GetGCS();

    return m_gcs;
    }

const GeoCoords::GCS& IMrDTMCreator::GetAdvancedGCS() const
    {
    return m_implP->GetGCS();
    }

const GeoCoords::GCS& IMrDTMCreator::GetGCS () const
    {
    return m_implP->GetGCS();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time IMrDTMCreator::GetLastSyncTime () const
    {
    return m_implP->m_lastSyncTime;
    }


Time IMrDTMCreator::GetLastModified() const
    {
    return m_implP->m_lastSourcesModificationTime;
    }

Time IMrDTMCreator::GetLastModifiedCheckTime () const
    {
    return m_implP->m_lastSourcesModificationCheckTime;
    }

StatusInt IMrDTMCreator::UpdateLastModified()
    {
    return m_implP->UpdateLastModified();
    }

void IMrDTMCreator::SetSourcesDirty ()
    {
    // Make sure that sources are not seen as up to date anymore
    m_implP->m_sourcesDirty = true;
    m_implP->m_lastSourcesModificationTime = Time::CreateActual();
    }

bool IMrDTMCreator::HasDirtySources () const
    {
    return m_implP->m_sourcesDirty;
    }

StatusInt IMrDTMCreator::SaveToFile()
    {
    return m_implP->SaveToFile();
    }

StatusInt IMrDTMCreator::SetCompression(MrDTMCompressionType compressionType)
    {
    m_implP->m_compressionType = compressionType;
    return SUCCESS;
    }



StatusInt IMrDTMCreator::SetBaseGCS (const Bentley::GeoCoordinates::BaseGCSPtr& gcsPtr)
    {
    return SetGCS(GetGCSFactory().Create(gcsPtr));
    }

StatusInt IMrDTMCreator::SetGCS(const GeoCoords::GCS& gcs)
    {
    if (0 != m_implP->m_mrDTMPtr.get())
        return m_implP->m_mrDTMPtr->SetGCS(gcs);

    // Do not permit setting null GCS. Use default when it happens.
    m_implP->m_gcs = (gcs.IsNull()) ? GetDefaultGCS() : gcs;
    m_implP->m_gcsDirty = true;

    return 0;
    }

const IDTMSourceCollection& IMrDTMCreator::GetSources () const
    {
    return m_implP->m_sources;
    }

IDTMSourceCollection& IMrDTMCreator::EditSources ()
    {
    return m_implP->m_sources;
    }

/*----------------------------------------------------------------------------+
|MrDTMCreator class
+----------------------------------------------------------------------------*/
IMrDTMCreator::Impl::Impl(const WChar* mrDtmFileName)
:       m_gcs(GetDefaultGCS()),
        m_mrDtmFileName(mrDtmFileName),
        m_lastSyncTime(Time::CreateSmallestPossible()),
        m_lastSourcesModificationTime(CreateUnknownModificationTime()),
        m_lastSourcesModificationCheckTime(Time::CreateSmallestPossible()),
        m_sourcesDirty(false),
        m_gcsDirty(false),
        m_sourceEnv(CreateSourceEnvFrom(mrDtmFileName)),
        m_compressionType(MRDTM_COMPRESSION_DEFLATE),
        m_workingLayer(DEFAULT_WORKING_LAYER)
    {
    m_sources.RegisterEditListener(*this);
    }

IMrDTMCreator::Impl::Impl(const IMrDTMPtr& mrDTMPtr)
    :   m_gcs(GetDefaultGCS()),
        m_mrDTMPtr(mrDTMPtr),
        m_lastSyncTime(Time::CreateSmallestPossible()),
        m_lastSourcesModificationTime(CreateUnknownModificationTime()),
        m_lastSourcesModificationCheckTime(Time::CreateSmallestPossible()),
        m_sourcesDirty(false),
        m_gcsDirty(false),
        m_sourceEnv(CreateSourceEnvFrom(dynamic_cast<const MrDTMBase&>(*m_mrDTMPtr).GetPath())),
        m_compressionType(MRDTM_COMPRESSION_DEFLATE),
        m_workingLayer(DEFAULT_WORKING_LAYER)
    {
    m_sources.RegisterEditListener(*this);
    }

IMrDTMCreator::Impl::~Impl()
    {
    StatusInt status = SaveToFile();
    assert(BSISUCCESS == status);

    m_sources.UnregisterEditListener(*this);
    }

/*----------------------------------------------------------------------------+
|IMrDTMCreator::Impl::CreateSourceEnvFrom
+----------------------------------------------------------------------------*/
DocumentEnv IMrDTMCreator::Impl::CreateSourceEnvFrom  (const WChar* filePath)
    {
    WString currentDirPath(filePath);
    const WString::size_type dirEndPos = currentDirPath.find_last_of(L"\\/");

    if (WString::npos == dirEndPos)
        return DocumentEnv(L"");

    currentDirPath.resize(dirEndPos + 1);
    return DocumentEnv(currentDirPath.c_str());
    }


#if 0
static HFCPtr<HVE2DPolygonOfSegments> computeConvexHull    (const DPoint3d*      inputPts,
                                                            size_t          inputPtsQty)
    {
    static const int        edgeOption = 1;
    //Use a very large value for max side so that we get the best representative hull.
    static const double     maxSide = 5000000000000;
    //Use a very large value for tolerance so that the triangulation process
    //doesn`t remove points.
    static const double     tolerance = 0.0000000001;

    BC_DTM_OBJ* dtmP = 0;

    if (0 != bcdtmObject_createDtmObject(&dtmP))
        return 0;


    DPoint3d *hullPtsP = 0;
    long numHullPts;

    if (0 != bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,
                                                    const_cast<DPoint3d*>(inputPts), (long)inputPtsQty) ||
        0 != bcdtmObject_setTriangulationParametersDtmObject(dtmP,tolerance,tolerance,edgeOption,maxSide) ||
        0 != bcdtmObject_triangulateDtmObject(dtmP) ||
        0 != bcdtmList_extractHullDtmObject(dtmP, &hullPtsP, &numHullPts))
        {
        bcdtmObject_destroyDtmObject(&dtmP);
        return 0;
        }


    bcdtmObject_destroyDtmObject(&dtmP);

    //Adjust view dependent metrics considering the convex hull.
    HArrayAutoPtr<double> pHullPts(new double[numHullPts * 2]);

    for (int hullPtInd = 0, bufferInd = 0; hullPtInd < numHullPts; hullPtInd++)
        {
        pHullPts[bufferInd] = hullPtsP[hullPtInd].x;
        bufferInd++;
        pHullPts[bufferInd] = hullPtsP[hullPtInd].y;
        bufferInd++;
        }
    bcdtmMath_freeArcPoints(&hullPtsP);
    //TDORAY: Use bcdtmUtl_freeMemory instead??

    return new HVE2DPolygonOfSegments (numHullPts * 2, pHullPts, HFCPtr<HGF2DCoordSys>(new HGF2DCoordSys()));
    }
#endif


// TDORAY: This is a duplicate of version in MrDTM.cpp. Find a way to use the same fn.
// TDORAY: Return a ref counted pointer
template<class POINT, class EXTENT>
static IHGFPointIndexFilter<POINT, EXTENT>* mrdtm_createFilterFromType (MrDTMFilterType filterType)
    {
    switch (filterType)
        {
        case MRDTM_FILTER_DUMB :
            return new MrDTMQuadTreeBCLIBFilter1<POINT, EXTENT>();
        case MRDTM_FILTER_TILE :
            return new MrDTMQuadTreeBCLIBFilter2<POINT, EXTENT>();
        case MRDTM_FILTER_TIN :
            return new MrDTMQuadTreeBCLIBFilter3<POINT, EXTENT>();
        case MRDTM_FILTER_PROGRESSIVE_DUMB :
            return new MrDTMQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT>();
        case MRDTM_FILTER_PROGRESSIVE_TILE :
            return new MrDTMQuadTreeBCLIBProgressiveFilter2<POINT, EXTENT>();
        case MRDTM_FILTER_PROGRESSIVE_TIN :
            return new MrDTMQuadTreeBCLIBProgressiveFilter3<POINT, EXTENT>();
        default :
            assert(!"Not supposed to be here");
        }
        return 0;
    }


MrDTMFilterType mrdtm_getFilterType ()
    {
    return MRDTM_FILTER_PROGRESSIVE_DUMB;
    }

bool mrdtm_isProgressiveFilter ()
    {
    return true;
    }

bool mrdtm_isCompressed ()
    {
    return true;
    }

int IMrDTMCreator::Impl::CreateMrDTM()
    {
    using namespace IDTMFile;

    int status = BSISUCCESS;

    //MS : Some cleanup needs to be done here.
    try
        {
        if (m_mrDTMPtr != 0)
            {
            if (MRDTM_STATE_UP_TO_DATE == m_mrDTMPtr->GetState())
                return BSIERROR;

            // NOTE: Need to be able to recreate : Or the file offers some functions for deleting all its data directory or the file name can be obtained
            }

        File::Ptr filePtr = SetupFileForCreation();
        if (0 == filePtr)
            return BSIERROR;


        // Sync only when there are sources with which to sync
        // TR #325614: This special condition provides us with a way of efficiently detecting if STM is empty
        //             as indexes won't be initialized.
        if (0 < m_sources.GetCount() &&
            BSISUCCESS != SyncWithSources(filePtr))
            return BSIERROR;


        // Update last synchronization time
        m_lastSyncTime = Time::CreateActual();

        // Ensure that last modified times are up-to-date and that sources are saved.
        if (BSISUCCESS != UpdateLastModified() ||
            BSISUCCESS != SaveSources(*filePtr) ||
            BSISUCCESS != SaveGCS(*filePtr))
            status = BSIERROR;
        }
    catch (...)
        {
        return BSIERROR;
        }

    return status;
    }



/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IMrDTMCreator::Impl::SyncWithSources (const IDTMFile::File::Ptr& filePtr) const
    {
    using namespace IDTMFile;

    typedef Point3d64f                                              PointType;
    typedef Extent3d64f                                             PointIndexExtentType;
    typedef HGF3DExtent<double>                                     FeatureIndexExtentType;

    //Create the storage object.
    typedef DTMTaggedTileStore<HFCPtr<HVEDTMLinearFeature>,
                               PointType,
                               PointIndexExtentType,
                               HGF3DCoord<double>,
                               FeatureIndexExtentType >             TileStoreType;

    typedef HGFPointIndex <PointType, PointIndexExtentType>         PointIndexType;

    typedef HGFFeatureIndex<HFCPtr<HVEDTMLinearFeature>,
                            HGF3DCoord<double>,
                            FeatureIndexExtentType >                FeatureIndexType;

    HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment myMemMgr (100, 2000 * sizeof(PointType));

    HFCPtr<TileStoreType> pFinalTileStore = new TileStoreType(filePtr, HFC_CREATE_ONLY, (MRDTM_COMPRESSION_DEFLATE == m_compressionType));


    IHGFPointIndexFilter<PointType, PointIndexExtentType>* pFilter =
                mrdtm_createFilterFromType<PointType, PointIndexExtentType>(mrdtm_getFilterType());

    PointIndexType pointIndex(new HPMCountLimitedPool<PointType>(&myMemMgr, 20000000),
                                   &*pFinalTileStore,
                                   50000,
                                   pFilter,
                                   false, false);

    FeatureIndexType linearIndex(&*pFinalTileStore, 400, false);

    const GCS& fileGCS = GetGCS();

    // Import sources
    if (BSISUCCESS != ImportSourcesTo(new MrDTMStorage<PointType>(pointIndex, linearIndex, fileGCS)))
        return BSIERROR;

    // TR 328701 - Add some points describing an overview of the linear features.
    if (BSISUCCESS != AddPointOverviewOfLinears<PointType>(pointIndex, linearIndex))
        return BSIERROR;

    // Filter data (in order to create sub-resolutions)
    if (BSISUCCESS != Filter(pointIndex, linearIndex))
        return BSIERROR;

    return BSISUCCESS;
    }




namespace {

template <typename PointIndexT>
struct PointIndexTypeTrait
    {
    // Default: Fail
    };

template <typename PointT, typename ExtentT>
struct PointIndexTypeTrait<HGFPointIndex<PointT, ExtentT>>
    {
    typedef PointT  PointType;
    typedef ExtentT ExtentType;
    };


}


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IMrDTMCreator::Impl::ImportSourcesTo (Sink* sinkP) const
    {
    SinkPtr sinkPtr(sinkP);
    LocalFileSourceRef sinkSourceRef(m_mrDtmFileName.c_str());

    const ContentDescriptor& targetContentDescriptor(sinkPtr->GetDescriptor());
    assert(1 == targetContentDescriptor.GetLayerCount());

    const GCS& targetGCS(targetContentDescriptor.LayersBegin()->GetGCS());

    SourcesImporter sourcesImporter(sinkSourceRef, sinkPtr);

    HFCPtr<HVEClipShape>  resultingClipShapePtr(new HVEClipShape(new HGF2DCoordSys()));
    if (BSISUCCESS != TraverseSourceCollection(sourcesImporter,
                                               m_sources,
                                               resultingClipShapePtr,
                                               targetGCS))
        return BSIERROR;

    if (SourcesImporter::S_SUCCESS != sourcesImporter.Import())
        return BSIERROR;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType, typename PointIndex, typename LinearIndex>
StatusInt IMrDTMCreator::Impl::AddPointOverviewOfLinears (PointIndex&  pointIndex,
                                                          LinearIndex& linearIndex)
    {
    YProtFeatureExtentType linearExtent = linearIndex.GetContentExtent();

    list<HFCPtr<HVEDTMLinearFeature>> linearList;

    linearIndex.GetIn(linearExtent, linearList);

    int status = SUCCESS;

    if (linearList.size() > 0)
        {
        //Compute the number of points
        list<HFCPtr<HVEDTMLinearFeature> >::iterator linearIter    = linearList.begin();
        list<HFCPtr<HVEDTMLinearFeature> >::iterator linearIterEnd = linearList.end();

        size_t nbOfLinearPoints = 0;

        while (linearIter != linearIterEnd)
            {
            nbOfLinearPoints += (*linearIter)->GetSize();
            linearIter++;
            }

        int decimationStep;

        if (nbOfLinearPoints > MAX_NUM_POINTS_FOR_LINEAR_OVERVIEW)
        {
            //Should result in a number of points roughly in the range of 11250 - 22500 points
            decimationStep = round((double)nbOfLinearPoints / MAX_NUM_POINTS_FOR_LINEAR_OVERVIEW);
        }
        else
        {
            decimationStep = 1;
        }

        linearIter    = linearList.begin();
        linearIterEnd = linearList.end();

        UInt32              tileNumber = 0;
        HAutoPtr<PointType> linePts;
        size_t               linePtsMaxSize = 0;

        int globalLinearPointInd = 0;

        while (linearIter != linearIterEnd)
            {
            if (linePtsMaxSize < (*linearIter)->GetSize())
                {
                linePtsMaxSize = (*linearIter)->GetSize();
                //linePts = new Point3d64f[linePtsMaxSize];
                linePts = new PointType[linePtsMaxSize];
                }

            size_t nbLinePts = 0;

            for (size_t indexPoints = 0 ; indexPoints < (*linearIter)->GetSize(); indexPoints++)
                {
                if (globalLinearPointInd % decimationStep == 0)
                    {
                    linePts[nbLinePts].x = (*linearIter)->GetPoint(indexPoints).GetX();
                    linePts[nbLinePts].y = (*linearIter)->GetPoint(indexPoints).GetY();
                    linePts[nbLinePts].z = (*linearIter)->GetPoint(indexPoints).GetZ();
                    nbLinePts++;
                    }

                globalLinearPointInd++;
                }

            if (nbLinePts > 0)
                {
                bool result = pointIndex.AddArray(linePts.get(), nbLinePts);

                assert(result == true);
                }

            if (status != SUCCESS)
                {
                break;
                }

            linearIter++;
            tileNumber++;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointIndex, typename LinearIndex>
StatusInt IMrDTMCreator::Impl::Filter  (PointIndex&     pointIndex,
                                        LinearIndex&    linearIndex)
    {
    pointIndex.Filter();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool IMrDTMCreator::Impl::FileExist () const
    {
    if (0 != m_mrDTMPtr.get())
        return true;

    return fileExist(m_mrDtmFileName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMFile::File::Ptr IMrDTMCreator::Impl::GetFile (const IDTMFile::AccessMode&  accessMode)
    {
    using namespace IDTMFile;
    File::Ptr filePtr;

    //Create the storage object.
    if (m_mrDTMPtr == 0)
        {
        assert(!m_mrDtmFileName.empty());
        filePtr = File::Open(m_mrDtmFileName.c_str(), accessMode);
        }
    else
        {
        filePtr = dynamic_cast<const MrDTMBase&>(*m_mrDTMPtr).GetIDTMFile();
        }

    return filePtr;
    }



/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMFile::File::Ptr IMrDTMCreator::Impl::SetupFileForCreation ()
    {
    using namespace IDTMFile;

    //If the file exists delete it.
    if (FileExist() && 0 != _wremove(m_mrDtmFileName.c_str()))
        return 0;

    File::Ptr filePtr = GetFile(HFC_CREATE_ONLY);
    if (0 == filePtr)
        return 0;

    // Ensure that layer directory is present
    if (!filePtr->GetRootDir()->HasLayerDir(m_workingLayer) &&
        0 == filePtr->GetRootDir()->CreateLayerDir(m_workingLayer))
        {
        return 0; // Failed to create layer dir
        }

    // Ensure source dir is present
    if (!filePtr->GetRootDir()->HasSourcesDir() &&
        0 == filePtr->GetRootDir()->CreateSourcesDir())
        {
        return 0; // Failed to create source dir
        }


    // Ensure GCS and sources are save to the file.
    m_gcsDirty = true;
    m_sourcesDirty = true;

    return filePtr;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IMrDTMCreator::Impl::UpdateLastModified()
    {
    StatusInt status = BSISUCCESS;

    Time lastModified = Time::CreateSmallestPossible();

    for (IDTMSourceCollection::iterator sourceIt = m_sources.BeginEditInternal(), sourceEnd = m_sources.EndEditInternal(); sourceIt != sourceEnd; ++sourceIt)
        {
        StatusInt sourceStatus = sourceIt->InternalUpdateLastModified();

        if (BSISUCCESS != sourceStatus)
            {
            status = BSIERROR;
            lastModified = CreateUnknownModificationTime();
            }
        else
            {
            lastModified = (std::max) (lastModified, sourceIt->GetLastModified());
            }
        }

    m_lastSourcesModificationTime = (std::max)(m_lastSourcesModificationTime, lastModified);
    m_lastSourcesModificationCheckTime = Time::CreateActual();

    m_sourcesDirty = true;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IMrDTMCreator::Impl::ResetLastModified ()
    {
    for (IDTMSourceCollection::iterator sourceIt = m_sources.BeginEditInternal(), sourceEnd = m_sources.EndEditInternal(); sourceIt != sourceEnd; ++sourceIt)
        sourceIt->ResetLastModified();

    m_lastSourcesModificationTime = CreateUnknownModificationTime();
    m_lastSourcesModificationCheckTime = Time::CreateSmallestPossible();
    }




/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IMrDTMCreator::Impl::LoadSources (const IDTMFile::File& file)
    {
    using namespace IDTMFile;

    if (!file.GetRootDir()->HasSourcesDir())
        return BSISUCCESS; // No sources were added to the STM.

    const IDTMFile::SourcesDir* sourceDirPtr = file.GetRootDir()->GetSourcesDir();
    if (0 == sourceDirPtr)
        return BSIERROR; // Could not load existing sources dir

    bool success = true;
    success &= Bentley::MrDTM::LoadSources(m_sources, *sourceDirPtr, m_sourceEnv);

    m_lastSourcesModificationCheckTime = CreateTimeFrom(sourceDirPtr->GetLastModifiedCheckTime());
    m_lastSourcesModificationTime = CreateTimeFrom(sourceDirPtr->GetLastModifiedTime());
    m_lastSyncTime = CreateTimeFrom(sourceDirPtr->GetLastSyncTime());

    return (success) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IMrDTMCreator::Impl::SaveSources (IDTMFile::File& file)
    {
    using namespace IDTMFile;

    if (!m_sourcesDirty)
        return BSISUCCESS;



    SourcesDir* sourceDirPtr = 0;

    if (file.GetRootDir()->HasSourcesDir())
        {
        sourceDirPtr = file.GetRootDir()->GetSourcesDir();
        if (0 == sourceDirPtr)
            return BSIERROR; // Error loading existing source dir

        sourceDirPtr->ClearAll(); // Clear existing data
        }
    else
        {
        sourceDirPtr = file.GetRootDir()->CreateSourcesDir();
        if (0 == sourceDirPtr)
            return BSIERROR; // Error creating source dir
        }


    bool success = true;

    success &= Bentley::MrDTM::SaveSources(m_sources, *sourceDirPtr, m_sourceEnv);
    success &= sourceDirPtr->SetLastModifiedCheckTime(GetCTimeFor(m_lastSourcesModificationCheckTime));
    success &= sourceDirPtr->SetLastModifiedTime(GetCTimeFor(m_lastSourcesModificationTime));
    success &= sourceDirPtr->SetLastSyncTime(GetCTimeFor(m_lastSyncTime));

    if (success)
        {
        m_sourcesDirty = false;
        return BSISUCCESS;
        }
    else
        {
        return BSIERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IMrDTMCreator::Impl::LoadGCS (const IDTMFile::File&   file)
    {
    using namespace IDTMFile;

    if (!file.GetRootDir()->HasLayerDir(m_workingLayer))
        return BSISUCCESS; // Ok, layer not yet created

    const LayerDir* layerDirP = file.GetRootDir()->GetLayerDir(m_workingLayer);
    if (0 == layerDirP)
        return BSIERROR;

    if (!layerDirP->HasWkt())
        {
        // Use default GCS when no WKT is found
        m_gcs = GetDefaultGCS();
        return BSISUCCESS;
        }

    HCPWKT wkt(layerDirP->GetWkt());

    WString wktStr(wkt.GetCStr());

    IDTMFile::WktFlavor fileWktFlavor = GetWKTFlavor(&wktStr, wktStr);
    
    BaseGCS::WktFlavor wktFlavor;

    bool result = MapWktFlavorEnum(wktFlavor, fileWktFlavor);

    assert(result);    

    GCSFactory::Status gcsFromWKTStatus = GCSFactory::S_SUCCESS;
    GCS fileGCS(GetGCSFactory().Create(wktStr.c_str(), wktFlavor, gcsFromWKTStatus));
    if (GCSFactory::S_SUCCESS != gcsFromWKTStatus)
        return BSIERROR;

    assert(!m_gcsDirty);
    using std::swap;
    swap(m_gcs, fileGCS);

    return BSISUCCESS;
    }



/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IMrDTMCreator::Impl::SaveGCS(IDTMFile::File& file)
    {
    using namespace IDTMFile;

    if (!m_gcsDirty)
        return BSISUCCESS;

    LayerDir* layerDirP = (file.GetRootDir()->HasLayerDir(m_workingLayer)) ?
                                file.GetRootDir()->GetLayerDir(m_workingLayer) :
                                file.GetRootDir()->CreateLayerDir(m_workingLayer);

    if (0 == layerDirP)
        return BSIERROR; // Could not access or create working layer dir

    const GCS& newSourceGCS = m_gcs;

    if (newSourceGCS.IsNull())
        {
        assert(!"Unexpected!");
        return BSIERROR;
        }

    GCS::Status wktCreateStatus = GCS::S_SUCCESS;
    const WKT wkt(newSourceGCS.GetWKT(wktCreateStatus));
    if (GCS::S_SUCCESS != wktCreateStatus)
        return BSIERROR;   
       
    WString extendedWktStr(wkt.GetCStr());    
   
    if (WKTKeyword::TYPE_UNKNOWN == GetWktType(extendedWktStr))
        {
        wchar_t wktFlavor[2] = {(wchar_t)IDTMFile::WktFlavor_Autodesk, L'\0'};

        extendedWktStr += WString(wktFlavor);            
        }      
    
    if (!layerDirP->SetWkt(HCPWKT(extendedWktStr.c_str())))
        return BSIERROR;

    m_gcsDirty = false;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IMrDTMCreator::Impl::SaveSources()
    {
    using namespace IDTMFile;

    File::Ptr filePtr = GetFile(FileExist() ? HFC_READ_WRITE : HFC_READ_WRITE_CREATE);
    if (0 == filePtr)
        return BSIERROR;

    return SaveSources(*filePtr);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IMrDTMCreator::Impl::LoadFromFile  ()
    {
    using namespace IDTMFile;

    File::Ptr filePtr = GetFile(HFC_SHARE_READ_ONLY | HFC_READ_ONLY);
    if (0 == filePtr)
        return BSIERROR;

    if (BSISUCCESS != LoadSources(*filePtr) ||
        BSISUCCESS != LoadGCS(*filePtr))
        return BSIERROR;

    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IMrDTMCreator::Impl::SaveToFile()
    {
    using namespace IDTMFile;

    const bool fileExists = FileExist();
    const bool fileDirty = m_sourcesDirty || m_gcsDirty;

    if (fileExists && !fileDirty)
        return BSISUCCESS; // Nothing to save

    File::Ptr filePtr = GetFile(fileExists ? HFC_READ_WRITE : HFC_READ_WRITE_CREATE);
    if (0 == filePtr)
        return BSIERROR;

    if (BSISUCCESS != SaveSources(*filePtr) ||
        BSISUCCESS != SaveGCS(*filePtr))
        return BSIERROR;

    return filePtr->Save() ? BSISUCCESS : BSIERROR;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IMrDTMCreator::Impl::_NotifyOfPublicEdit ()
    {
    // Make sure that sources are not seen as up to date anymore
    m_sourcesDirty = true;
    m_lastSourcesModificationTime = Time::CreateActual();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IMrDTMCreator::Impl::_NotifyOfLastEditUpdate (Time updatedLastEditTime)
    {
    m_lastSourcesModificationTime = (std::max)(m_lastSourcesModificationTime, updatedLastEditTime);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef CreateSourceRefFromIDTMSource (const IDTMSource& source, const WString& stmPath)
    {
    struct Visitor : IDTMSourceVisitor
        {
        auto_ptr<SourceRef>         m_sourceRefP;
        const WString&              m_stmPath;

        explicit                    Visitor            (const WString&                  stmPath)
            :   m_stmPath(stmPath)
            {
            }

        virtual void                _Visit             (const IDTMLocalFileSource&  source) override
            {
            StatusInt status = BSISUCCESS;
            const WChar* path = source.GetPath(status);
            if (BSISUCCESS != status)
                throw SourceNotFoundException();

            if (0 == wcsicmp(path, m_stmPath.c_str()))
                throw CustomException(L"STM and source are the same");

            m_sourceRefP.reset(new SourceRef(LocalFileSourceRef(path)));
            }
        virtual void                _Visit             (const IDTMDgnLevelSource&       source) override
            {
            StatusInt status = BSISUCCESS;
            if (BSISUCCESS != status)
                throw SourceNotFoundException();

            m_sourceRefP.reset(new SourceRef(DGNLevelByIDSourceRef(source.GetPath(),
                                                                   source.GetModelID(),
                                                                   source.GetLevelID())));
            }

        virtual void                _Visit             (const IDTMDgnReferenceSource&       source) override
            {
            throw CustomException(L"Not supported");
            }
        virtual void                _Visit             (const IDTMDgnReferenceLevelSource&  source) override
            {
            StatusInt status = BSISUCCESS;
            if (BSISUCCESS != status)
                throw SourceNotFoundException();

            m_sourceRefP.reset(new SourceRef(DGNReferenceLevelByIDSourceRef(source.GetPath(),
                                                                            source.GetModelID(),
                                                                            source.GetRootToRefPersistentPath(),
                                                                            source.GetLevelID())));
            }

        virtual void                _Visit             (const IDTMDgnModelSource&            source) override
            {
            throw CustomException(L"Not supported");
            }
        virtual void                _Visit             (const IDTMSourceGroup&      source) override
            {
            /* Do nothing */
            }
        };

    Visitor visitor(stmPath);
    source.Accept(visitor);

    if (0 == visitor.m_sourceRefP.get())
        throw CustomException(L"Unable to create source Ref from IDTMSource!");

    return *visitor.m_sourceRefP;
    }

int IMrDTMCreator::Impl::ImportClipMaskSource  (const IDTMSource&       dataSource,
                                                const ClipShapeStoragePtr&  clipShapeStoragePtr) const
    {
    try
        {
        static const SourceFactory SOURCE_FACTORY(GetSourceFactory());
        static const ImporterFactory IMPORTER_FACTORY(GetImporterFactory());


        // Create sourceRef
        SourceRef srcRef = CreateSourceRefFromIDTMSource(dataSource, m_mrDtmFileName);

        // Create source
        const SourcePtr originalSourcePtr = SOURCE_FACTORY.Create(srcRef);
        if(0 == originalSourcePtr.get())
            return BSIERROR;

        const SourcePtr sourcePtr = Configure(originalSourcePtr, dataSource.GetConfig().GetContentConfig(), GetLog());
        if(0 == sourcePtr.get())
            return BSIERROR;

        // Create importer
        clipShapeStoragePtr->SetIsMaskSource(dataSource.GetSourceType() == DTM_SOURCE_DATA_MASK);

        const ImporterPtr importerPtr = IMPORTER_FACTORY.Create(sourcePtr, &(*clipShapeStoragePtr));
        if(0 == importerPtr.get())
            return BSIERROR;

        // Import
        const Importer::Status importStatus = importerPtr->Import(dataSource.GetConfig().GetSequence(), dataSource.GetConfig().GetConfig());
        if(importStatus != Importer::S_SUCCESS)
            return BSIERROR;

        return BSISUCCESS;
        }
    catch (...)
        {
        return BSIERROR;
        }
    }


int IMrDTMCreator::Impl::TraverseSource  (SourcesImporter&                                         importer,
                                          const IDTMSource&                                    dataSource,
                                          const HFCPtr<HVEClipShape>&                              clipShapePtr,
                                          const GCS&                                               targetGCS) const
    {
    StatusInt status = BSISUCCESS;
    try
        {
        const SourceImportConfig& sourceImportConfig = dataSource.GetConfig();

        const ContentConfig& sourceConfig (sourceImportConfig.GetContentConfig());
        const ImportSequence& importSequence(sourceImportConfig.GetSequence());


        ImportConfig importConfig(sourceImportConfig.GetConfig());

        // For the moment we always want to combine imported source layers to first STM layer.
        importConfig.push_back(DefaultTargetLayerConfig(0));

        // Ensure that sources that have no GCSs and no user selected GCS may fall-back on the STM's GCS
        importConfig.push_back(DefaultSourceGCSConfig(targetGCS));

        if (!clipShapePtr->IsEmpty())
            importConfig.push_back(TargetFiltersConfig(ClipMaskFilterFactory::CreateFrom(clipShapePtr)));

        SourceRef sourceRef(CreateSourceRefFromIDTMSource(dataSource, m_mrDtmFileName));

        importer.AddSource(sourceRef, sourceConfig, importConfig, importSequence);
        }
    catch (...)
        {
        status = BSIERROR;
        }

    return status;
    }

int IMrDTMCreator::Impl::TraverseSourceCollection  (SourcesImporter&                                            importer,
                                                    const IDTMSourceCollection&                                 sources,
                                                    const HFCPtr<HVEClipShape>&                                 totalClipShapePtr,
                                                    const GCS&                                                  targetGCS) const

    {
    int status = BSISUCCESS;

    ClipShapeStoragePtr clipShapeStoragePtr = new ClipShapeStorage(totalClipShapePtr, targetGCS);


    typedef IDTMSourceCollection::const_reverse_iterator RevSourceIter;

    //The sources need to be parsed in the reverse order.
    for (RevSourceIter sourceIt = sources.rBegin(), sourcesEnd = sources.rEnd();
         sourceIt != sourcesEnd && BSISUCCESS == status;
         ++sourceIt)
        {
        const IDTMSource& source = *sourceIt;

        if ((source.GetSourceType() == DTM_SOURCE_DATA_CLIP) ||
            (source.GetSourceType() == DTM_SOURCE_DATA_MASK))
            {
            status = ImportClipMaskSource(source,
                                          clipShapeStoragePtr);
            }
        else
        if (dynamic_cast<const IDTMSourceGroup*>(&source) != 0)
            {
            const IDTMSourceGroup& sourceGroup = dynamic_cast<const IDTMSourceGroup&>(source);

            // Copy clip shape so that group clip and mask don't affect global clip shape.
            HFCPtr<HVEClipShape> groupClipShapePtr = new HVEClipShape(*(clipShapeStoragePtr->GetResultingClipShape()));

            status = TraverseSourceCollection(importer,
                                              sourceGroup.GetSources(),
                                              groupClipShapePtr,
                                              targetGCS);
            }
        else
            {
            // Copy clip shape so that group clip and mask don't affect global clip shape.
            HFCPtr<HVEClipShape> groupClipShapePtr = new HVEClipShape(*(clipShapeStoragePtr->GetResultingClipShape()));

            status = TraverseSource(importer,
                                    source,
                                    groupClipShapePtr,
                                    targetGCS);
            }
        }

    return status;
    }


/*==================================================================*/
/*        MRDTM CREATOR SECTION - END                               */
/*==================================================================*/

END_BENTLEY_MRDTM_NAMESPACE