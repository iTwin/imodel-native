/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshCreator.cpp $
|    $RCSfile: ScalableMeshCreator.cpp,v $
|   $Revision: 1.90 $
|       $Date: 2012/01/27 16:45:29 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
USING_NAMESPACE_BENTLEY_TERRAINMODEL

#include "ScalableMeshCreator.h"
//#include "DTMGraphTileStore.h"
//#include "SMTextureTileStore.h"
#include <ScalableMesh/GeoCoords/GCS.h>

/*----------------------------------------------------------------------+
| Include internal dependencies                                         |
+----------------------------------------------------------------------*/
#include <STMInternal/Foundations/PrivateStringTools.h>

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include "ScalableMesh.h"
#include <ScalableMesh/ScalableMeshDefs.h>

#include <ScalableMesh/Memory/Allocation.h>
#include <ScalableMesh/Import/SourceReference.h>

#include <ScalableMesh/Import/Source.h>
#include <ScalableMesh/Import/Importer.h>

#include <ScalableMesh/Import/Error/Source.h>

#include "../Import/Sink.h"
#include "ScalableMeshSourcesImport.h"

#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include <ScalableMesh/GeoCoords/Reprojection.h>
#include "ScalableMeshTime.h"
#include "ScalableMeshSourcesPersistance.h"
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>
#include <ScalableMesh/IScalableMeshDocumentEnv.h>
#include <ScalableMesh/IScalableMeshSourceVisitor.h>

#include "Plugins/ScalableMeshClipMaskFilterFactory.h"
#include "Plugins/ScalableMeshIDTMFileTraits.h"

#include "ScalableMeshStorage.h"
#include <ScalableMesh/IScalableMeshStream.h>

#include <ScalableMesh/IScalableMeshPolicy.h>

#include "ScalableMeshSources.h"

#include "InternalUtilityFunctions.h"
#include <STMInternal/GeoCoords/WKTUtils.h>

#include "ScalableMeshMesher.h"
//#include "Edits\DiffSetTileStore.h"
#include "Edits\ClipRegistry.h"
//#include <DgnPlatform\Tools\ConfigurationManager.h>
#include "Edits/ClipUtilities.hpp"

#include "SMSQLitePointTileStore.h"
#include "SMSQLiteIndiceTileStore.h"
#include "SMSQLiteGraphTileStore.h"
#include "SMSQLiteUVStore.h"
#include "SMSQLiteUVIndiceTileStore.h"
#include "SMSQLiteTextureTileStore.h"
#include <ImagePP\all\h\HIMMosaic.h>



#define SCALABLE_MESH_TIMINGS

//NEEDS_WORK_SM : Temp global variable probably only for debug purpose, not sure we want to know if we are in editing.
extern bool s_inEditing = false; 

using namespace IDTMFile;
USING_NAMESPACE_BENTLEY_DGNPLATFORM

   
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

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

namespace {

const size_t DEFAULT_WORKING_LAYER = 0;




inline const GCS& GetDefaultGCS ()
    {
    static const GCS DEFAULT_GCS(GetGCSFactory().Create(GeoCoords::Unit::GetMeter()));
    return DEFAULT_GCS;
    }



}

#ifdef SM_BESQL_FORMAT
//NEEDS_WORK_SM : Should be part of the index instead to avoid having very big odd values.
std::atomic<uint64_t> s_nextNodeID = 0;
#endif


/*==================================================================*/
/*        MRDTM CREATOR SECTION - BEGIN                             */
/*==================================================================*/
void RegisterPODImportPlugin();

void RegisterDelayedImporters()
    {    
    static bool s_areRegistered = false;

    if (!s_areRegistered)
        {
        //RegisterPODImportPlugin();
        s_areRegistered = true;
        }
    }

IScalableMeshCreatorPtr IScalableMeshCreator::GetFor (const WChar*  filePath,
                                        StatusInt&      status)
    {
    RegisterDelayedImporters();

    using namespace IDTMFile;


    IScalableMeshCreatorPtr pCreator = new IScalableMeshCreator(new Impl(filePath));

    status = pCreator->m_implP->LoadFromFile();
    if (BSISUCCESS != status)
        return 0;

    return pCreator.get();
    }


IScalableMeshCreatorPtr IScalableMeshCreator::GetFor (const IScalableMeshPtr&    scmPtr,
                                        StatusInt&          status)
    {
    using namespace IDTMFile;

    RegisterDelayedImporters();

    IScalableMeshCreatorPtr pCreator = new IScalableMeshCreator(new Impl(scmPtr));

    status = pCreator->m_implP->LoadFromFile();
    if (BSISUCCESS != status)
        return 0;

    return pCreator.get();
    }

/*IScalableMeshCreatorPtr IScalableMeshCreator::GetFor (const WChar* filePath)
    {
    StatusInt dummyStatus;
    return GetFor(filePath, dummyStatus);
    }

IScalableMeshCreatorPtr IScalableMeshCreator::GetFor (const IScalableMeshPtr& scmPtr)
    {
    StatusInt dummyStatus;
    return GetFor(scmPtr, dummyStatus);
    }*/


IScalableMeshCreator::IScalableMeshCreator (Impl* implP)
    :   m_implP(implP)
    {        
    }


IScalableMeshCreator::~IScalableMeshCreator ()
    {
    }
#if 0
bool IScalableMeshCreator::AreAllSourcesReachable () const
    {
    // Reachable only if all sources are reachable
    return m_implP->m_sources.End() == std::find_if(m_implP->m_sources.Begin(), m_implP->m_sources.End(), not1(mem_fun_ref(&IDTMSource::IsReachable)));
    }

#endif

StatusInt IScalableMeshCreator::Create (bool isSingleFile)
    {
    return m_implP->CreateScalableMesh(isSingleFile);
    }



const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& IScalableMeshCreator::GetBaseGCS () const
    {
    return GetAdvancedGCS().GetGeoRef().GetBasePtr();
    }

const GeoCoords::GCS& IScalableMeshCreator::Impl::GetGCS () const
    {
    if (0 != m_scmPtr.get())
        return m_scmPtr->GetGCS();

    return m_gcs;
    }

const GeoCoords::GCS& IScalableMeshCreator::GetAdvancedGCS() const
    {
    return m_implP->GetGCS();
    }

const GeoCoords::GCS& IScalableMeshCreator::GetGCS () const
    {
    return m_implP->GetGCS();
    }


#ifdef SCALABLE_MESH_ATP

static unsigned __int64 s_getNbImportedPoints = 0;
static double s_getImportPointsDuration = -1; 
static double s_getLastBalancingDuration = -1;
static double s_getLastMeshingDuration = -1;
static double s_getLastFilteringDuration = -1;
static double s_getLastStitchingDuration = -1;

unsigned __int64 IScalableMeshCreator::GetNbImportedPoints()
    {
    return s_getNbImportedPoints;
    }

double IScalableMeshCreator::GetImportPointsDuration()
    {
    return s_getImportPointsDuration;
    }

double IScalableMeshCreator::GetLastBalancingDuration()
    {
    return s_getLastBalancingDuration;
    }

double IScalableMeshCreator::GetLastMeshingDuration()
    {
    return s_getLastMeshingDuration;
    }

double IScalableMeshCreator::GetLastFilteringDuration()
    {
    return s_getLastFilteringDuration;
    }

double IScalableMeshCreator::GetLastStitchingDuration()
    {
    return s_getLastStitchingDuration;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time IScalableMeshCreator::GetLastSyncTime () const
    {
    return m_implP->m_lastSyncTime;
    }

#if 0
Time IScalableMeshCreator::GetLastModified() const
    {
    return m_implP->m_lastSourcesModificationTime;
    }

Time IScalableMeshCreator::GetLastModifiedCheckTime () const
    {
    return m_implP->m_lastSourcesModificationCheckTime;
    }



StatusInt IScalableMeshCreator::UpdateLastModified()
    {
    return m_implP->UpdateLastModified();
    }

void IScalableMeshCreator::SetSourcesDirty ()
    {
    // Make sure that sources are not seen as up to date anymore
    m_implP->m_sourcesDirty = true;
    m_implP->m_lastSourcesModificationTime = Time::CreateActual();
    }

bool IScalableMeshCreator::HasDirtySources () const
    {
    return m_implP->m_sourcesDirty;
    }
#endif
StatusInt IScalableMeshCreator::SaveToFile()
    {
    return m_implP->SaveToFile();
    }

StatusInt IScalableMeshCreator::SetCompression(ScalableMeshCompressionType compressionType)
    {
    m_implP->m_compressionType = compressionType;
    return SUCCESS;
    }

StatusInt   IScalableMeshCreator::SetTextureMosaic(HIMMosaic* mosaicP)
    {
    return m_implP->SetTextureMosaic(mosaicP);
    }


StatusInt IScalableMeshCreator::SetBaseGCS (const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& gcsPtr)
    {
    return SetGCS(GetGCSFactory().Create(gcsPtr));
    }

StatusInt IScalableMeshCreator::SetGCS(const GeoCoords::GCS& gcs)
    {
    if (0 != m_implP->m_scmPtr.get())
        return m_implP->m_scmPtr->SetGCS(gcs);

    // Do not permit setting null GCS. Use default when it happens.
    m_implP->m_gcs = (gcs.IsNull()) ? GetDefaultGCS() : gcs;
    m_implP->m_gcsDirty = true;

    return 0;
    }
#if 0
const IDTMSourceCollection& IScalableMeshCreator::GetSources () const
    {
    return m_implP->m_sources;
    }

IDTMSourceCollection& IScalableMeshCreator::EditSources ()
    {
    return m_implP->m_sources;
    }
#endif
#if 0
IScalableMeshNodePtr IScalableMeshCreator::AddChildNode (const IScalableMeshNodePtr& parentNode, 
                                                         StatusInt&                  status)
    {
    return m_implP->AddChildNode (parentNode, status);
    }
#endif
/*----------------------------------------------------------------------------+
|ScalableMeshCreator class
+----------------------------------------------------------------------------*/
IScalableMeshCreator::Impl::Impl(const WChar* scmFileName)
    : m_gcs(GetDefaultGCS()),
    m_scmFileName(scmFileName),
    m_lastSyncTime(Time::CreateSmallestPossible()),
    // m_lastSourcesModificationTime(CreateUnknownModificationTime()),
    // m_lastSourcesModificationCheckTime(Time::CreateSmallestPossible()),
    //  m_sourcesDirty(false),
    m_gcsDirty(false),
    //   m_sourceEnv(CreateSourceEnvFrom(scmFileName)),
    m_compressionType(SCM_COMPRESSION_DEFLATE),
    m_workingLayer(DEFAULT_WORKING_LAYER)
    {
    // m_sources.RegisterEditListener(*this);
    //m_dgnScalableMesh.Create();


    WString smStoreDgnDbStr;
    m_isDgnDb = false;

    //FeatureIndexType linearIndex;
   /* bool isBlobMode = true;
    if (BSISUCCESS == ConfigurationManager::GetVariable(smStoreDgnDbStr, L"SM_STORE_DGNDB"))
        {
        m_isDgnDb = _wtoi(smStoreDgnDbStr.c_str()) > 0;
        isBlobMode = _wtoi(smStoreDgnDbStr.c_str()) > 1;
        }


    WString multithreadStr;
    if (BSISUCCESS == ConfigurationManager::GetVariable(multithreadStr, L"SM_MULTITHREAD_GENERATION"))
        {
        if (_wtoi(multithreadStr.c_str()) > 0)
            {
            s_useThreadsInMeshing = true;
            s_useThreadsInStitching = true;
            s_useThreadsInFiltering = true;
            }
        }*/
    s_useThreadsInMeshing = true;
    s_useThreadsInStitching = false;
    s_useThreadsInFiltering = true;
    }

IScalableMeshCreator::Impl::Impl(const IScalableMeshPtr& scmPtr)
    :   m_gcs(GetDefaultGCS()),
        m_scmPtr(scmPtr),
        m_lastSyncTime(Time::CreateSmallestPossible()),
        m_gcsDirty(false),     
        m_compressionType(SCM_COMPRESSION_DEFLATE),
        m_workingLayer(DEFAULT_WORKING_LAYER)
    {
  

    WString smStoreDgnDbStr;
    m_isDgnDb = false;

    //FeatureIndexType linearIndex;
    /*bool isBlobMode;
    if (BSISUCCESS == ConfigurationManager::GetVariable(smStoreDgnDbStr, L"SM_STORE_DGNDB"))
        {
        m_isDgnDb = _wtoi(smStoreDgnDbStr.c_str()) > 0;
        isBlobMode = _wtoi(smStoreDgnDbStr.c_str()) > 1;
        }*/


    }

IScalableMeshCreator::Impl::~Impl()
    {
    //m_pDataIndex->DumpOctTree("D:\\NodeAferCreation.xml", false);
    if (m_pDataIndex) m_pDataIndex = 0;
    StatusInt status = SaveToFile();
    assert(BSISUCCESS == status);
    m_scmPtr = 0;
    //m_sources.UnregisterEditListener(*this);
    }

StatusInt IScalableMeshCreator::Impl::SetTextureMosaic(HIMMosaic* mosaicP)
    {
    if (m_scmPtr.get() == nullptr) return ERROR;
    m_scmPtr->TextureFromRaster(mosaicP);
    return SUCCESS;
    }

/*----------------------------------------------------------------------------+
|IScalableMeshCreator::Impl::CreateSourceEnvFrom
+----------------------------------------------------------------------------*/
/*DocumentEnv IScalableMeshCreator::Impl::CreateSourceEnvFrom  (const WChar* filePath)
    {
    WString currentDirPath(filePath);
    const WString::size_type dirEndPos = currentDirPath.find_last_of(L"\\/");

    if (WString::npos == dirEndPos)
        return DocumentEnv(L"");

    currentDirPath.resize(dirEndPos + 1);
    return DocumentEnv(currentDirPath.c_str());
    }
    */
 


// TDORAY: This is a duplicate of version in ScalableMesh.cpp. Find a way to use the same fn.
// TDORAY: Return a ref counted pointer
template<class POINT, class EXTENT>
static ISMPointIndexFilter<POINT, EXTENT>* scm_createFilterFromType (ScalableMeshFilterType filterType)
    {
    /*WString filterTypeStr;

    //NEEDS_WORK_SM : Document all environments variable like this one somewhere (e.g. : pw:\\Alpo.bentley.com:alpo-bentleygeospatial\Documents\Raster Products\General\environment variables.doc)
    if (BSISUCCESS == ConfigurationManager::GetVariable(filterTypeStr, L"SM_FILTER_TYPE"))
        {
        ScalableMeshFilterType filterTypeOverwrite = (ScalableMeshFilterType)_wtoi(filterTypeStr.c_str());
        if (filterTypeOverwrite >= 0 && filterTypeOverwrite < SCM_FILTER_QTY)
            {
            filterType = filterTypeOverwrite;
            }
        else
            {
            assert(!"Unknown filter type");
            }        
        }*/


    switch (filterType)
        {
        case SCM_FILTER_DUMB:
            return new ScalableMeshQuadTreeBCLIBFilter1<POINT, EXTENT>();                
        case SCM_FILTER_PROGRESSIVE_DUMB:
            return new ScalableMeshQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT>();        
        case SCM_FILTER_DUMB_MESH:
            return new ScalableMeshQuadTreeBCLIBMeshFilter1<POINT, EXTENT>();        
        case SCM_FILTER_CGAL_SIMPLIFIER:
            return new ScalableMeshQuadTreeBCLIB_CGALMeshFilter<POINT, EXTENT>();
        default :
            assert(!"Not supposed to be here");
        }
    return 0;
    }

template<class POINT, class EXTENT>
static ISMPointIndexMesher<POINT, EXTENT>* Create2_5dMesherFromType (ScalableMeshMesherType mesherType)
    {    
    //Only one 2.5d mesher
    assert(mesherType == SCM_MESHER_2D_DELAUNAY);

    return new ScalableMesh2DDelaunayMesher<POINT, EXTENT>();    
    }


template<class POINT, class EXTENT>
static ISMPointIndexMesher<POINT, EXTENT>* Create3dMesherFromType (ScalableMeshMesherType mesherType)
    {    
    /*WString mesherTypeStr;

    if (BSISUCCESS == ConfigurationManager::GetVariable(mesherTypeStr, L"SM_3D_MESHER_TYPE"))
        {
        ScalableMeshMesherType mesherTypeOverwrite = (ScalableMeshMesherType)_wtoi(mesherTypeStr.c_str());
        if (mesherTypeOverwrite >= 0 && mesherTypeOverwrite < SCM_MESHER_QTY)
            {
            mesherType = mesherTypeOverwrite;
            }
        else
            {
            assert(!"Unknown mesher type");
            }        
        }*/

    switch (mesherType)
        {
        case SCM_MESHER_2D_DELAUNAY:
            return new ScalableMesh2DDelaunayMesher<POINT, EXTENT>();
            /*        
        case SCM_MESHER_3D_DELAUNAY:
            return new ScalableMesh3DDelaunayMesher<POINT, EXTENT> (false);
        case SCM_MESHER_TETGEN:
            return new ScalableMesh3DDelaunayMesher<POINT, EXTENT> (true);*/
        default:
            assert(!"Not supposed to be here");
        }
    return 0;
    }

ScalableMeshFilterType scm_getFilterType ()
    {
    //NEEDS_WORK_SM - No progressive for mesh
    //return SCM_FILTER_PROGRESSIVE_DUMB;    
    return SCM_FILTER_CGAL_SIMPLIFIER;
    //return SCM_FILTER_DUMB_MESH;
    }

ScalableMeshMesherType Get2_5dMesherType ()
    {    
    return SCM_MESHER_2D_DELAUNAY;
    }

ScalableMeshMesherType Get3dMesherType ()
    {    
    //return SCM_MESHER_2D_DELAUNAY;    
    //return SCM_MESHER_3D_DELAUNAY;
#ifndef NO_3D_MESH
    return SCM_MESHER_TETGEN;
#else
return SCM_MESHER_2D_DELAUNAY;
#endif
    }

bool scm_isProgressiveFilter ()
    {
    return true;
    }

bool scm_isCompressed ()
    {
    return true;
    }


bool DgnDbFilename(BENTLEY_NAMESPACE_NAME::WString& stmFilename)
            {
            BENTLEY_NAMESPACE_NAME::WString dgndbFilename;
            //stmFilename
            size_t size = stmFilename.ReplaceAll(L".stm", L".dgndb");
            assert(size==1);
            return true;
            }


int IScalableMeshCreator::Impl::CreateScalableMesh(bool isSingleFile)
    {    
    int status = BSIERROR;
    return status;
    }



/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/


StatusInt IScalableMeshCreator::Impl::CreateDataIndex (HFCPtr<MeshIndexType>&                                    pDataIndex, 

                                                       HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment& myMemMgr,
                                                       bool needBalancing) 
    {    

    typedef SMSQLitePointTileStore<PointType,
        PointIndexExtentType>             TileStoreType;

  

    typedef SMStreamingPointTaggedTileStore<
        PointType,
        PointIndexExtentType >        StreamingStoreType;

        
    HFCPtr<TileStoreType> pFinalTileStore;
    HFCPtr<IScalableMeshDataStore<MTGGraph, Byte, Byte>> pGraphTileStore;
    bool isSingleFile = true;

    isSingleFile = m_smSQLitePtr->IsSingleFile();



    HFCPtr<StreamingStoreType>  pStreamingTileStore;
    HFCPtr<SMStreamingPointTaggedTileStore<int32_t, PointIndexExtentType>> pStreamingIndiceTileStore;
    HFCPtr<SMStreamingPointTaggedTileStore<DPoint2d, PointIndexExtentType>> pStreamingUVTileStore;
    HFCPtr<SMStreamingPointTaggedTileStore<int32_t, PointIndexExtentType>> pStreamingUVsIndicesTileStore;
    HFCPtr<StreamingTextureTileStore> pStreamingTextureTileStore;

    HFCPtr<SMSQLiteIndiceTileStore<YProtPtExtentType >> pIndiceTileStore;
    HFCPtr<SMSQLiteUVTileStore<YProtPtExtentType >> pUVTileStore;
    HFCPtr<SMSQLiteUVIndiceTileStore<YProtPtExtentType >> pUVsIndicesTileStore;
    HFCPtr<SMSQLiteTextureTileStore> pTextureTileStore;


    // HFC_CREATE_ONLY if all sources are ADD (if one remove or one uptodate => read_write)
    //accessMode = filePtr->GetAccessMode();
    s_inEditing = false;


    ISMPointIndexFilter<PointType, PointIndexExtentType>* pFilter =
        scm_createFilterFromType<PointType, PointIndexExtentType>(scm_getFilterType());

    ISMPointIndexMesher<PointType, PointIndexExtentType>* pMesher2_5d =
                Create2_5dMesherFromType<PointType, PointIndexExtentType>(Get2_5dMesherType());

    ISMPointIndexMesher<PointType, PointIndexExtentType>* pMesher3d =
                Create3dMesherFromType<PointType, PointIndexExtentType>(Get3dMesherType());
        

            // SM_NEEDS_WORK : replace all tilestore by SQLiteTileStore

        if (!isSingleFile)
            {
            auto position = m_scmFileName.find_last_of(L".stm");
            WString streamingFilePath = m_scmFileName.substr(0, position - 3) + L"_stream\\";
            if (0 == CreateDirectoryW(streamingFilePath.c_str(), NULL))
                {
                assert(ERROR_PATH_NOT_FOUND != GetLastError());
                }
            WString point_store_path = streamingFilePath + L"point_store\\";
            WString indice_store_path = streamingFilePath + L"indice_store\\";
            WString uv_store_path = streamingFilePath + L"uv_store\\";
            WString uvIndice_store_path = streamingFilePath + L"uvIndice_store\\"; 
            WString texture_store_path = streamingFilePath + L"texture_store\\";

            pStreamingTileStore = new StreamingStoreType(point_store_path, WString(), (SCM_COMPRESSION_DEFLATE == m_compressionType));
            // SM_NEEDS_WORKS : layerID 
            pStreamingIndiceTileStore = new SMStreamingPointTaggedTileStore< int32_t, PointIndexExtentType>(indice_store_path, WString(), (SCM_COMPRESSION_DEFLATE == m_compressionType));
            pStreamingUVTileStore = new SMStreamingPointTaggedTileStore< DPoint2d, PointIndexExtentType>(uv_store_path, WString(), (SCM_COMPRESSION_DEFLATE == m_compressionType));
            pStreamingUVsIndicesTileStore = new SMStreamingPointTaggedTileStore< int32_t, PointIndexExtentType>(uvIndice_store_path, WString(), (SCM_COMPRESSION_DEFLATE == m_compressionType));
            pStreamingTextureTileStore = new StreamingTextureTileStore(texture_store_path.c_str());
            
            pDataIndex = new MeshIndexType(ScalableMeshMemoryPools<PointType>::Get()->GetGenericPool(),                                       
                                       &*pStreamingTileStore,                                       
                                       &*pStreamingIndiceTileStore,
                                       ScalableMeshMemoryPools<PointType>::Get()->GetGraphPool(),
                                       pGraphTileStore = new SMSQLiteGraphTileStore(m_smSQLitePtr),                                       
                                       &*pStreamingTextureTileStore,                                                                              
                                       &*pStreamingUVTileStore,                                       
                                       &*pStreamingUVsIndicesTileStore,
                                       10000,
                                       pFilter,
                                       needBalancing, false, false,
                                       pMesher2_5d,
                                       pMesher3d);
            pDataIndex->SetSingleFile(false);
            pDataIndex->SetGenerating(true);
           // WString clipFilePath = m_scmFileName;
            //clipFilePath.append(L"_clips");
            //IDTMFile::File::Ptr clipFilePtr = IDTMFile::File::Create(clipFilePath.c_str());
            /*HFCPtr<IScalableMeshDataStore<DifferenceSet, Byte, Byte>> store = new DiffSetTileStore(clipFilePath, 0, true);
            store->StoreMasterHeader(NULL, 0);
            pDataIndex->SetClipStore(store);
            auto pool = ScalableMeshMemoryPools<POINT>::Get()->GetDiffSetPool();
            pDataIndex->SetClipPool(pool);*/
            WString clipFileDefPath = m_scmFileName;
            clipFileDefPath.append(L"_clipDefinitions");
            ClipRegistry* registry = new ClipRegistry(clipFileDefPath);
            pDataIndex->SetClipRegistry(registry);
            }
        else {
//            size_t n =4;

            WString dbFilePath = m_scmFileName;
            dbFilePath.ReplaceI(L".stm", L".db");
            pFinalTileStore = new TileStoreType(m_smSQLitePtr);
            pIndiceTileStore = new SMSQLiteIndiceTileStore<YProtPtExtentType >(m_smSQLitePtr);

            pUVTileStore = new SMSQLiteUVTileStore<YProtPtExtentType >(m_smSQLitePtr);
            pUVsIndicesTileStore = new SMSQLiteUVIndiceTileStore<YProtPtExtentType >(m_smSQLitePtr);
            pTextureTileStore = new SMSQLiteTextureTileStore(m_smSQLitePtr);
            pGraphTileStore = new SMSQLiteGraphTileStore(m_smSQLitePtr);

            //pIndiceTileStore = new TileStoreType(filePtr, (SCM_COMPRESSION_DEFLATE == m_compressionType));
            pDataIndex = new MeshIndexType(ScalableMeshMemoryPools<PointType>::Get()->GetGenericPool(),                                       
                                       &*pFinalTileStore,                                       
                                       &*pIndiceTileStore,
                                       ScalableMeshMemoryPools<PointType>::Get()->GetGraphPool(),
                                       pGraphTileStore,                                       
                                       &*pTextureTileStore,                                       
                                       &*pUVTileStore,                                       
                                       &*pUVsIndicesTileStore,
                                       10000,
                                       pFilter,
                                       needBalancing, false, false,
                                       pMesher2_5d,
                                       pMesher3d);
            pDataIndex->SetGenerating(true);
            //WString clipFilePath = m_scmFileName;
            //clipFilePath.append(L"_clips");
            //IDTMFile::File::Ptr clipFilePtr = IDTMFile::File::Create(clipFilePath.c_str());
            /*HFCPtr<IScalableMeshDataStore<DifferenceSet, Byte, Byte>> store = new DiffSetTileStore(clipFilePath, 0, true);
            //store->StoreMasterHeader(NULL, 0);
            pDataIndex->SetClipStore(store);
            auto pool = ScalableMeshMemoryPools<POINT>::Get()->GetDiffSetPool();
            pDataIndex->SetClipPool(pool);*/
            WString clipFileDefPath = m_scmFileName;
            clipFileDefPath.append(L"_clipDefinitions");
            ClipRegistry* registry = new ClipRegistry(clipFileDefPath);
            pDataIndex->SetClipRegistry(registry);
            }
        
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
#if 0
template <typename PointType, typename PointIndex, typename LinearIndex>
StatusInt IScalableMeshCreator::Impl::AddPointOverviewOfLinears (PointIndex&  pointIndex,
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

        uint32_t              tileNumber = 0;
        HAutoPtr<PointType> linePts;
        size_t               linePtsMaxSize = 0;

        int globalLinearPointInd = 0;

        while (linearIter != linearIterEnd)
            {
            if (linePtsMaxSize < (*linearIter)->GetSize())
                {
                linePtsMaxSize = (*linearIter)->GetSize();
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
                bool result = pointIndex.AddArray(linePts.get(), nbLinePts, false);

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
#endif
/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool IScalableMeshCreator::Impl::FileExist () const
    {
    if (0 != m_scmPtr.get())
        return true;

    return fileExist(m_scmFileName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/

SMSQLiteFilePtr IScalableMeshCreator::Impl::GetFile(bool fileExists)
{
    bool success = true;

    if (m_smSQLitePtr == nullptr)
    {
        //m_smSQLitePtr = new SMSQLiteFile();

        // Create the storage object and databases
        if (m_scmPtr == 0)
        {
            assert(!m_scmFileName.empty());
            m_smSQLitePtr = new SMSQLiteFile();
            if (!fileExists)
                success = m_smSQLitePtr->Create(m_scmFileName);
            else
                success = m_smSQLitePtr->Open(m_scmFileName, false); // open in read/write
        }
        else
           m_smSQLitePtr = dynamic_cast<const ScalableMeshBase&>(*m_scmPtr).GetDbFile();
    }
    else
        assert(m_smSQLitePtr->IsOpen());
    return m_smSQLitePtr;
}


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/

void IScalableMeshCreator::Impl::SetupFileForCreation()
{
    m_gcsDirty = true;
}

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IScalableMeshCreator::Impl::LoadGCS()
{
    if(!m_smSQLitePtr->HasWkt())
    {
        // Use default GCS when no WKT is found
        m_gcs = GetDefaultGCS();
        return BSISUCCESS;
    }

    WString wktStr;
    m_smSQLitePtr->GetWkt(wktStr);

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

int IScalableMeshCreator::Impl::SaveGCS()
{
    if(!m_gcsDirty)
        return BSISUCCESS;

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
        wchar_t wktFlavor[2] = { (wchar_t)IDTMFile::WktFlavor_Autodesk, L'\0' };

        extendedWktStr += WString(wktFlavor);
    }

    if (!m_smSQLitePtr->SetWkt(extendedWktStr.c_str()))
        return BSIERROR;
    
    m_gcsDirty = false;
    return BSISUCCESS;
}


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IScalableMeshCreator::Impl::LoadFromFile  ()
    {


        m_smSQLitePtr = GetFile(fileExist(m_scmFileName.c_str()));
        

        if (!Load())
            return BSIERROR;

    return BSISUCCESS;
    }

bool IScalableMeshCreator::Impl::IsFileDirty()
    {
    return m_gcsDirty;
    }




StatusInt IScalableMeshCreator::Impl::Load()
{
    return BSISUCCESS == LoadGCS();
}

StatusInt IScalableMeshCreator::Impl::Save()
{
    return BSISUCCESS == SaveGCS();
}

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IScalableMeshCreator::Impl::SaveToFile()
    {
        const bool fileExists = FileExist();
        
        if (fileExists && !IsFileDirty())
            return BSISUCCESS; // Nothing to save

        if (GetFile(fileExists) == 0)
            return BSIERROR;

        /*if (!test)
            return BSIERROR;*/

        if (!Save())
            return BSIERROR;

        return BSISUCCESS;

    }


void IScalableMeshCreator::Impl::ShowMessageBoxWithTimes(double meshingDuration, double filteringDuration, double stitchingDuration)
    {
    string meshing = to_string(meshingDuration);
    string filtering = to_string(filteringDuration);
    string stitching = to_string(stitchingDuration);

    string msg = "Meshing: "; msg += meshing;   msg += " min.\n";
    msg += "Filtering: "; msg += filtering; msg += " min.\n";
    msg += "Stitching: "; msg += stitching; msg += " min.\n";

    MessageBoxA(NULL, msg.c_str(), "Information", MB_ICONINFORMATION | MB_OK);
    }

/*==================================================================*/
/*        MRDTM CREATOR SECTION - END                               */
/*==================================================================*/

END_BENTLEY_SCALABLEMESH_NAMESPACE