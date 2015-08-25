/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshCreator.cpp $
|    $RCSfile: ScalableMeshCreator.cpp,v $
|   $Revision: 1.90 $
|       $Date: 2012/01/27 16:45:29 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>

USING_NAMESPACE_BENTLEY_TERRAINMODEL

#include "ScalableMeshCreator.h"
#include "DTMGraphTileStore.h"
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

#include <ScalableMesh/Import/Config/All.h>

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

#include <DgnPlatform\Tools\ConfigurationManager.h>
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


/*==================================================================*/
/*        MRDTM CREATOR SECTION - BEGIN                             */
/*==================================================================*/
void RegisterPODImportPlugin();

void RegisterDelayedImporters()
    {    
    static bool s_areRegistered = false;

    if (!s_areRegistered)
        {
        RegisterPODImportPlugin();
        s_areRegistered = true;
        }
    }

IScalableMeshCreatorPtr IScalableMeshCreator::GetFor (const WChar*  filePath,
                                        StatusInt&      status)
    {
    RegisterDelayedImporters();

    using namespace IDTMFile;

    if (!fileExist(filePath))
        {
        status = BSISUCCESS;
        return new IScalableMeshCreator(new Impl(filePath)); // Return early. File does not exist.
        }

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

StatusInt IScalableMeshCreator::Create ()
    {
    return m_implP->CreateScalableMesh();
    }



const Bentley::GeoCoordinates::BaseGCSPtr& IScalableMeshCreator::GetBaseGCS () const
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



StatusInt IScalableMeshCreator::SetBaseGCS (const Bentley::GeoCoordinates::BaseGCSPtr& gcsPtr)
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
:       m_gcs(GetDefaultGCS()),
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

#ifdef SCALABLE_MESH_DGN
    WString smStoreDgnDbStr;
    m_isDgnDb = false;

    //FeatureIndexType linearIndex;
    bool isBlobMode = false;
    if (BSISUCCESS == ConfigurationManager::GetVariable(smStoreDgnDbStr, L"SM_STORE_DGNDB"))
        {
        m_isDgnDb = _wtoi(smStoreDgnDbStr.c_str()) > 0;
        isBlobMode = _wtoi(smStoreDgnDbStr.c_str()) > 1;
        }
    if(m_isDgnDb)
        {
        bool status = IDgnDbScalableMesh::Initialize(m_dgnScalableMeshPtr, isBlobMode);
        if(!status)
            throw;
        }
#endif
    }

IScalableMeshCreator::Impl::Impl(const IScalableMeshPtr& scmPtr)
    :   m_gcs(GetDefaultGCS()),
        m_scmPtr(scmPtr),
        m_lastSyncTime(Time::CreateSmallestPossible()),
       // m_lastSourcesModificationTime(CreateUnknownModificationTime()),
       // m_lastSourcesModificationCheckTime(Time::CreateSmallestPossible()),
      //  m_sourcesDirty(false),
        m_gcsDirty(false),
      //  m_sourceEnv(CreateSourceEnvFrom(dynamic_cast<const ScalableMeshBase&>(*m_scmPtr).GetPath())),
        m_compressionType(SCM_COMPRESSION_DEFLATE),
        m_workingLayer(DEFAULT_WORKING_LAYER)
    {
   // m_sources.RegisterEditListener(*this);
#ifdef SCALABLE_MESH_DGN
    WString smStoreDgnDbStr;
    m_isDgnDb = false;

    //FeatureIndexType linearIndex;
    bool isBlobMode;
    if (BSISUCCESS == ConfigurationManager::GetVariable(smStoreDgnDbStr, L"SM_STORE_DGNDB"))
        {
        m_isDgnDb = _wtoi(smStoreDgnDbStr.c_str()) > 0;
        isBlobMode = _wtoi(smStoreDgnDbStr.c_str()) > 1;
        }
    if(m_isDgnDb)
        {
        bool status = IDgnDbScalableMesh::Initialize(m_dgnScalableMeshPtr, isBlobMode);
        if(!status)
            throw;
        }
#endif
    }

IScalableMeshCreator::Impl::~Impl()
    {
    //m_pDataIndex->DumpOctTree("D:\\NodeAferCreation.xml", false);
    if (m_pDataIndex) m_pDataIndex = 0;
    StatusInt status = SaveToFile();
    assert(BSISUCCESS == status);

    //m_sources.UnregisterEditListener(*this);
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
    WString filterTypeStr;

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
        }


    switch (filterType)
        {
        case SCM_FILTER_DUMB:
            return new ScalableMeshQuadTreeBCLIBFilter1<POINT, EXTENT>();
        case SCM_FILTER_TILE:
            return new ScalableMeshQuadTreeBCLIBFilter2<POINT, EXTENT>();
        case SCM_FILTER_TIN:
            return new ScalableMeshQuadTreeBCLIBFilter3<POINT, EXTENT>();
        case SCM_FILTER_PROGRESSIVE_DUMB:
            return new ScalableMeshQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT>();
        case SCM_FILTER_PROGRESSIVE_TILE:
            return new ScalableMeshQuadTreeBCLIBProgressiveFilter2<POINT, EXTENT>();
        case SCM_FILTER_PROGRESSIVE_TIN:
            return new ScalableMeshQuadTreeBCLIBProgressiveFilter3<POINT, EXTENT>();
        case SCM_FILTER_DUMB_MESH:
            return new ScalableMeshQuadTreeBCLIBMeshFilter1<POINT, EXTENT>();
        case SCM_FILTER_GARLAND_SIMPLIFIER:
            return new ScalableMeshQuadTreeBCLIB_GarlandMeshFilter<POINT, EXTENT>();
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
    WString mesherTypeStr;

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
        }

    switch (mesherType)
        {
        case SCM_MESHER_2D_DELAUNAY:
            return new ScalableMesh2DDelaunayMesher<POINT, EXTENT>();
        case SCM_MESHER_LMS_MARCHING_CUBE:
            return new ScalableMeshAPSSOutOfCoreMesher<POINT, EXTENT>();
        case SCM_MESHER_3D_DELAUNAY:
            return new ScalableMesh3DDelaunayMesher<POINT, EXTENT> (false);
        case SCM_MESHER_TETGEN:
            return new ScalableMesh3DDelaunayMesher<POINT, EXTENT> (true);
        default:
            assert(!"Not supposed to be here");
        }
    return 0;
    }

ScalableMeshFilterType scm_getFilterType ()
    {
    //NEEDS_WORK_SM - No progressive for mesh
    //return SCM_FILTER_PROGRESSIVE_DUMB;
    //return SCM_FILTER_DUMB;
#ifndef NO_3D_MESH
    return SCM_FILTER_GARLAND_SIMPLIFIER;
#else
    return SCM_FILTER_DUMB_MESH;
#endif
    }

ScalableMeshMesherType Get2_5dMesherType ()
    {    
    return SCM_MESHER_2D_DELAUNAY;
    }

ScalableMeshMesherType Get3dMesherType ()
    {    
    //return SCM_MESHER_2D_DELAUNAY;
    //return SCM_MESHER_LMS_MARCHING_CUBE;
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

int IScalableMeshCreator::Impl::CreateScalableMesh()
    {    
    int status = BSIERROR;
#if 0
    //MS : Some cleanup needs to be done here.
    try
        {
        if (m_scmPtr != 0)
            {
            if (SCM_STATE_UP_TO_DATE == m_scmPtr->GetState())
                return BSIERROR;

            // NOTE: Need to be able to recreate : Or the file offers some functions for deleting all its data directory or the file name can be obtained
            }

        File::Ptr filePtr = SetupFileForCreation();
        if (0 == filePtr)
            return BSIERROR;

#ifdef SCALABLE_MESH_DGN
        if(m_isDgnDb)
            {
            IDTMFile::AccessMode accessMode = filePtr->GetAccessMode();
            Bentley::WString filename = m_scmFileName;
            DgnDbFilename(filename);
            assert(accessMode.m_HasCreateAccess);
                m_dgnScalableMeshPtr->Create(filename.GetWCharCP());
            }
#endif

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
#endif
    return status;
    }



/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
#if 0
//NEEDS_WORK_SM : To be removed
static bool s_dumpOctreeNodes = false;
static bool s_mesh = true;
static bool s_filter = true;
static bool s_stitchFilteredByLevel = true;
static bool s_validateIs3dDataState = false;
#endif


StatusInt IScalableMeshCreator::Impl::CreateDataIndex (HFCPtr<IndexType>&                                    pDataIndex, 
                                                       const IDTMFile::File::Ptr&                            filePtr,
                                                       HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment& myMemMgr) 
    {    

    //Create the storage object.
    typedef SMPointTaggedTileStore<PointType,
        PointIndexExtentType>             TileStoreType;

#ifdef SCALABLE_MESH_DGN
    typedef DgnTaggedTileStore<HFCPtr<HVEDTMLinearFeature>,
        PointType,
        PointIndexExtentType,
        HGF3DCoord<double>,
        FeatureIndexExtentType >     DgnStoreType;
#endif    
           

    IDTMFile::AccessMode         accessMode;    
    HFCPtr<TileStoreType> pFinalTileStore;
#ifdef SCALABLE_MESH_DGN
    HFCPtr<DgnStoreType>  pFinalDgnTileStore;
#endif

    // HFC_CREATE_ONLY if all sources are ADD (if one remove or one uptodate => read_write)
    accessMode = filePtr->GetAccessMode();
    s_inEditing = false;
/*    for (IDTMSourceCollection::const_iterator it = m_sources.Begin(); it != m_sources.End(); it++)
        {
        SourceImportConfig conf = it->GetConfig();
        ScalableMeshData data = conf.GetReplacementSMData();
        if(data.GetUpToDateState() != UpToDateState::ADD)
            {
            accessMode = HFC_READ_WRITE;
            s_inEditing = true;  
            }
        }*/

//    pFinalTileStore = new TileStoreType(filePtr, accessMode, (MRDTM_COMPRESSION_DEFLATE == m_compressionType));

    ISMPointIndexFilter<PointType, PointIndexExtentType>* pFilter =
        scm_createFilterFromType<PointType, PointIndexExtentType>(scm_getFilterType());

    ISMPointIndexMesher<PointType, PointIndexExtentType>* pMesher2_5d =
                Create2_5dMesherFromType<PointType, PointIndexExtentType>(Get2_5dMesherType());

    ISMPointIndexMesher<PointType, PointIndexExtentType>* pMesher3d =
                Create3dMesherFromType<PointType, PointIndexExtentType>(Get3dMesherType());
        
#ifdef SCALABLE_MESH_DGN
    if(m_isDgnDb)
        {
        pFinalDgnTileStore = new DgnStoreType(m_dgnScalableMeshPtr, filePtr, accessMode, (SCM_COMPRESSION_DEFLATE == m_compressionType));
        pDataIndex = new IndexType(new HPMCountLimitedPool<PointType>(&myMemMgr, 20000000),
            &*pFinalDgnTileStore,
            new HPMIndirectCountLimitedPool<MTGGraph>(/*&myMemMgr,*/ 100000000),
            new DgnGraphTileStore(m_dgnScalableMeshPtr, filePtr, 0),
            10000,
            pFilter,
            false, false, 
            pMesher2_5d, 
            pMesher3d);

        }
    else
#endif
        {
        pFinalTileStore = new TileStoreType(filePtr,(SCM_COMPRESSION_DEFLATE == m_compressionType));
      pDataIndex = new IndexType(ScalableMeshMemoryPools<PointType>::Get()->GetPointPool(),
            &*pFinalTileStore,
            ScalableMeshMemoryPools<PointType>::Get()->GetGraphPool(),
            new DTMGraphTileStore(filePtr, 0),
                                   10000,
            pFilter,
            false, false, 
            pMesher2_5d, 
            pMesher3d);

        }  

#ifdef SCALABLE_MESH_DGN
    if(m_isDgnDb)
        linearIndexPtr = new FeatureIndexType(&*pFinalDgnTileStore, 400, false);
    else
#endif

    return SUCCESS;
    }

#if 0
StatusInt IScalableMeshCreator::Impl::SyncWithSources (const IDTMFile::File::Ptr& filePtr) 
    {     
    using namespace IDTMFile;

    HFCPtr<IndexType>          pDataIndex;

    HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment myMemMgr (100, 2000 * sizeof(PointType));
    
    CreateDataIndex (pDataIndex, filePtr, myMemMgr);
   
    size_t previousDepth = pDataIndex->GetDepth();
    
    const GCS& fileGCS = GetGCS();

#ifdef SCALABLE_MESH_ATP

    s_getImportPointsDuration = -1; 
    s_getLastBalancingDuration = -1;
    s_getLastMeshingDuration = -1;
    s_getLastFilteringDuration = -1;
    s_getLastStitchingDuration = -1;
    pDataIndex->m_nbInputPoints = 0;

    clock_t startClock = clock();    
#endif




    std::list<IDTMFile::Extent3d64f> listRemoveExtent;
    IDTMSourceCollection::const_iterator it = m_sources.Begin();

    for (IDTMSourceCollection::iterator it = m_sources.BeginEdit(); it != m_sources.EndEdit(); it++)
        {
        SourceImportConfig& conf = it->EditConfig();
        ScalableMeshData data = conf.GetReplacementSMData();

        if (data.GetUpToDateState() == UpToDateState::REMOVE || data.GetUpToDateState() == UpToDateState::MODIFY)
            {
            DRange3d sourceRange = data.GetExtentByLayer(0);
            IDTMFile::Extent3d64f removeExtent;
            removeExtent.xMin = sourceRange.low.x;
            removeExtent.xMax = sourceRange.high.x;
            removeExtent.yMin = sourceRange.low.y;
            removeExtent.yMax = sourceRange.high.y;
            removeExtent.zMin = sourceRange.low.z;
            removeExtent.zMax = sourceRange.high.z;

            for(size_t i = 1; i < data.GetLayerCount(); i++)
                {
                DRange3d sourceRange = data.GetExtentByLayer((int)i);
                removeExtent.xMin = removeExtent.xMin > sourceRange.low.x ? sourceRange.low.x : removeExtent.xMin;
                removeExtent.xMax = removeExtent.xMax > sourceRange.high.x ? removeExtent.xMax : sourceRange.high.x;
                removeExtent.yMin = removeExtent.yMin > sourceRange.low.y ? sourceRange.low.y : removeExtent.yMin;
                removeExtent.yMax = removeExtent.yMax > sourceRange.high.y ? removeExtent.yMax : sourceRange.high.y;
                removeExtent.zMin = removeExtent.zMin > sourceRange.low.z ? sourceRange.low.z : removeExtent.zMin;
                removeExtent.zMax = removeExtent.zMax > sourceRange.high.z ? removeExtent.zMax : sourceRange.high.z;
                }
            
            if(data.GetUpToDateState() == UpToDateState::MODIFY)
                {
                data.SetUpToDateState(UpToDateState::ADD);
                conf.SetReplacementSMData(data);
                }

            listRemoveExtent.push_back(removeExtent);
            for(IDTMSourceCollection::iterator itAdd = m_sources.BeginEdit(); itAdd != m_sources.EndEdit(); itAdd++)
                {
                if(itAdd == it)
                    continue;
                SourceImportConfig& confAdd = itAdd->EditConfig();
                ScalableMeshData dataAdd = confAdd.GetReplacementSMData();
                if(dataAdd.GetUpToDateState() != UpToDateState::UP_TO_DATE && dataAdd.GetUpToDateState() != UpToDateState::PARTIAL_ADD)
                    continue;
                DRange3d targetRange = dataAdd.GetExtentByLayer(0);
                for(size_t i = 1; i<dataAdd.GetLayerCount(); i++)
                    {
                    DRange3d targetRangeTemp = dataAdd.GetExtentByLayer((int)i);
                    targetRange.UnionOf(targetRange,targetRangeTemp);
                    }
                if(sourceRange.IntersectsWith(targetRange,3))
                    {
                    // save extent in vector and the iterator.
                    dataAdd.SetUpToDateState(PARTIAL_ADD);
                    dataAdd.PushBackVectorRangeAdd(sourceRange);
                    confAdd.SetReplacementSMData(dataAdd);
                    }
                }
            }
        }




    // Remove sources which have been removed or modified

    if (BSISUCCESS != RemoveSourcesFrom<IndexType>(*pDataIndex, listRemoveExtent))
        return BSIERROR;

    // Import sources

    if (BSISUCCESS != ImportSourcesTo(new ScalableMeshStorage<PointType>(*pDataIndex, fileGCS)))
        return BSIERROR;

#ifdef SCALABLE_MESH_ATP
    s_getImportPointsDuration = ((double)clock() - startClock) / CLOCKS_PER_SEC / 60.0;
    s_getNbImportedPoints = pDataIndex->m_nbInputPoints;

    startClock = clock();
#endif

    // Remove and Add sources
    for (IDTMSourceCollection::iterator it = m_sources.BeginEdit(); it != m_sources.End();)
        {
        SourceImportConfig& conf = it->EditConfig();
        ScalableMeshData data = conf.GetReplacementSMData();

        if (data.GetUpToDateState() == UpToDateState::ADD || data.GetUpToDateState() == UpToDateState::PARTIAL_ADD)
            {
            data.SetUpToDateState(UpToDateState::UP_TO_DATE);
            conf.SetReplacementSMData(data);
            }
        if (data.GetUpToDateState() == UpToDateState::REMOVE)
            it = m_sources.Remove(it);
        else
            it++;
        }

    //True when only linear feature are imported.
    if (pDataIndex->GetRootNode() == 0)
        {
        assert(pDataIndex->m_nbInputPoints == 0);

        return BSISUCCESS;
        }

    // Balance data             
    if (BSISUCCESS != BalanceDown(*pDataIndex, previousDepth))
        return BSIERROR;

#ifdef SCALABLE_MESH_ATP
    s_getLastBalancingDuration = ((double)clock() - startClock) / CLOCKS_PER_SEC / 60.0;                        

    startClock = clock();
#endif

    if (s_mesh)
        {
        // Mesh data             
        if (BSISUCCESS != Mesh(*pDataIndex))
            return BSIERROR;        
        }

#ifdef SCALABLE_MESH_ATP
    s_getLastMeshingDuration = ((double)clock() - startClock) / CLOCKS_PER_SEC / 60.0;                        
#endif

    if (s_stitchFilteredByLevel == false)
        {
#ifdef SCALABLE_MESH_ATP    
        startClock = clock();
#endif

        if (s_filter)
            {
            // Filter data (in order to create sub-resolutions)
            if (BSISUCCESS != Filter(*pDataIndex,  -1))
                return BSIERROR;
            }

#ifdef SCALABLE_MESH_ATP
        s_getLastFilteringDuration = ((double)clock() - startClock) / CLOCKS_PER_SEC / 60.0;                        

        startClock = clock();
#endif
         /* 
        if (BSISUCCESS != Stitch(pointIndex, -1))
            return BSIERROR; 
            */

#ifdef SCALABLE_MESH_ATP
        s_getLastStitchingDuration = ((double)clock() - startClock) / CLOCKS_PER_SEC / 60.0;                        
#endif
        }
    else
        {
#ifdef SCALABLE_MESH_ATP    
        s_getLastStitchingDuration = 0;
        s_getLastFilteringDuration = 0;
#endif

        size_t depth = pDataIndex->GetDepth();

        for (int level = (int)depth; level >= 0; level--)
            {

#ifdef SCALABLE_MESH_ATP    
            startClock = clock();
#endif
            if (BSISUCCESS != Filter(*pDataIndex, level))
                return BSIERROR;

#ifdef SCALABLE_MESH_ATP    
            s_getLastFilteringDuration += clock() - startClock;
            startClock = clock();
#endif
            if (BSISUCCESS != Stitch(*pDataIndex, level, false))
                return BSIERROR;

#ifdef SCALABLE_MESH_ATP    
            s_getLastStitchingDuration += clock() - startClock;
            startClock = clock();
#endif


            }

#ifdef SCALABLE_MESH_ATP    
        s_getLastStitchingDuration = s_getLastStitchingDuration / CLOCKS_PER_SEC / 60.0;                        
        s_getLastFilteringDuration = s_getLastFilteringDuration / CLOCKS_PER_SEC / 60.0;  
#endif
        }

    //ShowMessageBoxWithTimes(s_getLastMeshingDuration, s_getLastFilteringDuration, s_getLastStitchingDuration);    

#ifdef INDEX_DUMPING_ACTIVATED
    if (s_dumpOctreeNodes)
        {        
        //pointIndex.DumpOctTree("D:\\MyDoc\\Scalable Mesh Iteration 7\\Partial Update - Remove\\Log\\NodeAferCreation.xml", false);    
        pDataIndex->DumpOctTree("C:\\Users\\Thomas.Butzbach\\Documents\\data_scalableMesh\\ATP\\NodeAferCreation.xml", false);  
        pDataIndex->DumpOctTree("D:\\NodeAferCreation.xml", false);  
        }        
#endif

#ifndef NDEBUG

    if (s_validateIs3dDataState)
        {
        vector<DRange3d> source2_5dRanges;
        vector<DRange3d> source3dRanges;

        // Remove and Add sources
        for (IDTMSourceCollection::iterator it = m_sources.BeginEdit(); it != m_sources.End(); it++)
            {
            SourceImportConfig& conf = it->EditConfig();
            ScalableMeshData data = conf.GetReplacementSMData();

            assert(data.GetExtent().size() > 0);
             
            if (data.IsRepresenting3dData() == SMis3D::is3D)
                {               
                source3dRanges.insert(source3dRanges.begin(), data.GetExtent().begin(), data.GetExtent().end());
                }        
            else
                {
                source2_5dRanges.insert(source2_5dRanges.begin(), data.GetExtent().begin(), data.GetExtent().end());                
                }
            }

        pDataIndex->ValidateIs3dDataStates(source2_5dRanges, source3dRanges);    
        }   
#endif

    pDataIndex = 0;
      
    return BSISUCCESS;
    }




namespace {

    template <typename PointIndexT>
    struct PointIndexTypeTrait
        {
        // Default: Fail
        };

    template <typename PointT, typename ExtentT>
    struct PointIndexTypeTrait<SMPointIndex<PointT, ExtentT>>
        {
        typedef PointT  PointType;
        typedef ExtentT ExtentType;
        };

    template <typename PointT, typename ExtentT>
    struct PointIndexTypeTrait<SMMeshIndex<PointT, ExtentT>>
        {
        typedef PointT  PointType;
        typedef ExtentT ExtentType;
        };
    }
#endif
#if 0
/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointIndex>       
StatusInt IScalableMeshCreator::Impl::RemoveSourcesFrom(PointIndex& pointIndex, list<IDTMFile::Extent3d64f> listRemoveExtent) const
    {
    //NEEDS_WORK_SM : Logic for determining the extent to remove should be here.  
    std::list<IDTMFile::Extent3d64f>::const_iterator it = listRemoveExtent.begin();
    for (std::list<IDTMFile::Extent3d64f>::const_iterator it = listRemoveExtent.begin(); it != listRemoveExtent.end(); it++)
        {
            {
            pointIndex.RemovePoints(*it);
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IScalableMeshCreator::Impl::ImportSourcesTo (Sink* sinkP)
    {
    SinkPtr sinkPtr(sinkP);
    LocalFileSourceRef sinkSourceRef(m_scmFileName.c_str());

    const ContentDescriptor& targetContentDescriptor(sinkPtr->GetDescriptor());
    assert(1 == targetContentDescriptor.GetLayerCount());

    const GCS& targetGCS(targetContentDescriptor.LayersBegin()->GetGCS());
    // NEEDS_WORK_SM : PARTIAL_UPDATE :remove
    const ScalableMeshData& targetScalableMeshData = ScalableMeshData::GetNull();

    SourcesImporter sourcesImporter(sinkSourceRef, sinkPtr);

    HFCPtr<HVEClipShape>  resultingClipShapePtr(new HVEClipShape(new HGF2DCoordSys()));

    int status;

    status = TraverseSourceCollection(sourcesImporter,
        m_sources,
        resultingClipShapePtr,
        targetGCS,
        targetScalableMeshData);

    if (BSISUCCESS == status)
        {
        if (SourcesImporter::S_SUCCESS != sourcesImporter.Import())
            {
            status = BSIERROR;
            }
        }


    return status;
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
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
IDTMFile::File::Ptr IScalableMeshCreator::Impl::GetFile (const IDTMFile::AccessMode&  accessMode)
    {
    using namespace IDTMFile;
    File::Ptr filePtr;

    //Create the storage object.
    if (m_scmPtr == 0)
        {
        assert(!m_scmFileName.empty());
        filePtr = File::Open(m_scmFileName.c_str(), accessMode);
        }
    else
        {
        filePtr = dynamic_cast<const ScalableMeshBase&>(*m_scmPtr).GetIDTMFile();
        }

    return filePtr;
    }



/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/

IDTMFile::File::Ptr IScalableMeshCreator::Impl::SetupFileForCreation()
    {
    using namespace IDTMFile;

    File::Ptr filePtr;
    filePtr = GetFile(HFC_CREATE_ONLY);
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
    return filePtr;
    }
#if 0

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IScalableMeshCreator::Impl::UpdateLastModified()
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
            // NEEDS_WORK_SM : ScalableMeshData -> get time compare to file time.
            SourceImportConfig& conf = sourceIt->EditConfig();

            ScalableMeshData data = conf.GetReplacementSMData();
            
            const IDTMLocalFileSource& localFileSource(static_cast<const IDTMLocalFileSource&>(*sourceIt));
            const WChar*  localFileNameTemp = localFileSource.GetPath();

            //struct tm* clock;
            struct stat attrib;

            char fileName[2048];
            wcstombs(fileName, localFileNameTemp, localFileSource.GetURL().GetPath().GetMaxLocaleCharBytes());
            stat(fileName, &attrib);
            //clock = gmtime(&(attrib.st_mtime));
            //time_t time = mktime(clock);
            time_t time = attrib.st_mtime;
            if(difftime(data.GetTimeFile(),time) < 0)
                {
                if (data.GetUpToDateState() == UpToDateState::UP_TO_DATE && data.GetTimeFile() != 0)
                    data.SetUpToDateState(UpToDateState::MODIFY);
                data.SetTimeFile(time);
                conf.SetReplacementSMData(data);
                }
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
void IScalableMeshCreator::Impl::ResetLastModified ()
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
int IScalableMeshCreator::Impl::LoadSources (const IDTMFile::File& file)
    {
    using namespace IDTMFile;

    if (!file.GetRootDir()->HasSourcesDir())
        return BSISUCCESS; // No sources were added to the STM.

    const IDTMFile::SourcesDir* sourceDirPtr = file.GetRootDir()->GetSourcesDir();
    if (0 == sourceDirPtr)
        return BSIERROR; // Could not load existing sources dir

    bool success = true;
    success &= Bentley::ScalableMesh::LoadSources(m_sources, *sourceDirPtr, m_sourceEnv);

    m_lastSourcesModificationCheckTime = CreateTimeFrom(sourceDirPtr->GetLastModifiedCheckTime());
    m_lastSourcesModificationTime = CreateTimeFrom(sourceDirPtr->GetLastModifiedTime());
    m_lastSyncTime = CreateTimeFrom(sourceDirPtr->GetLastSyncTime());

    return (success) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IScalableMeshCreator::Impl::SaveSources (IDTMFile::File& file)
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

    success &= Bentley::ScalableMesh::SaveSources(m_sources, *sourceDirPtr, m_sourceEnv);
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
#endif
/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IScalableMeshCreator::Impl::LoadGCS (const IDTMFile::File&   file)
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
int IScalableMeshCreator::Impl::SaveGCS(IDTMFile::File& file)
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
#if 0
/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IScalableMeshCreator::Impl::SaveSources()
    {
    using namespace IDTMFile;

    File::Ptr filePtr = GetFile(FileExist() ? HFC_READ_WRITE : HFC_READ_WRITE_CREATE);
    if (0 == filePtr)
        return BSIERROR;

    return SaveSources(*filePtr);
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IScalableMeshCreator::Impl::LoadFromFile  ()
    {
    using namespace IDTMFile;

    File::Ptr filePtr = GetFile(HFC_SHARE_READ_ONLY | HFC_READ_ONLY);
    if (0 == filePtr)
        return BSIERROR;

    if (/*BSISUCCESS != LoadSources(*filePtr) ||
        BSISUCCESS != LoadGCS(*filePtr)*/!Load(filePtr))
        return BSIERROR;

    return BSISUCCESS;
    }

bool IScalableMeshCreator::Impl::IsFileDirty()
    {
    return m_gcsDirty;
    }


StatusInt IScalableMeshCreator::Impl::Save(IDTMFile::File::Ptr& filePtr)
    {
    return BSISUCCESS == SaveGCS(*filePtr);
    }

StatusInt IScalableMeshCreator::Impl::Load(IDTMFile::File::Ptr& filePtr)
    {
    return BSISUCCESS == LoadGCS(*filePtr);
    }
/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IScalableMeshCreator::Impl::SaveToFile()
    {
    using namespace IDTMFile;

    const bool fileExists = FileExist();

    if (fileExists && !IsFileDirty())
        return BSISUCCESS; // Nothing to save

    File::Ptr filePtr = GetFile(fileExists ? HFC_READ_WRITE : HFC_READ_WRITE_CREATE);
    if (0 == filePtr)
        return BSIERROR;

    if (!Save(filePtr))
        return BSIERROR;

    return filePtr->Save() ? BSISUCCESS : BSIERROR;
    }

#if 0
/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IScalableMeshCreator::Impl::_NotifyOfPublicEdit ()
    {
    // Make sure that sources are not seen as up to date anymore
    m_sourcesDirty = true;
    m_lastSourcesModificationTime = Time::CreateActual();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IScalableMeshCreator::Impl::_NotifyOfLastEditUpdate (Time updatedLastEditTime)
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

            LocalFileSourceRef localSourceRef(path);

            m_sourceRefP.reset(new SourceRef(localSourceRef));
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

    visitor.m_sourceRefP->SetDtmSource(source.Clone());

    return *visitor.m_sourceRefP;
    }

int IScalableMeshCreator::Impl::ImportClipMaskSource  (const IDTMSource&       dataSource,
                                                const ClipShapeStoragePtr&  clipShapeStoragePtr) const
    {
    try
        {
        static const SourceFactory SOURCE_FACTORY(GetSourceFactory());
        static const ImporterFactory IMPORTER_FACTORY(GetImporterFactory());


        // Create sourceRef
        SourceRef srcRef = CreateSourceRefFromIDTMSource(dataSource, m_scmFileName);

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


int IScalableMeshCreator::Impl::TraverseSource  (SourcesImporter&                                         importer,
                                          IDTMSource&                                              dataSource,
                                          const HFCPtr<HVEClipShape>&                              clipShapePtr,
                                          const GCS&                                               targetGCS,
                                          const ScalableMeshData&                    targetScalableMeshData) const
    {
    StatusInt status = BSISUCCESS;
    try
        {
        const SourceImportConfig& sourceImportConfig = dataSource.GetConfig();

        const ContentConfig& sourceConfig (sourceImportConfig.GetContentConfig());
        const ImportSequence& importSequence(sourceImportConfig.GetSequence());

        // NEEDS_WORK_SM : test if source is ADD mode.
        SourceImportConfig& srcImportConfig = dataSource.EditConfig();
        ScalableMeshData data = srcImportConfig.GetReplacementSMData();

        UpToDateState state = data.GetUpToDateState();
        if(state != UpToDateState::ADD && state != UpToDateState::PARTIAL_ADD)
            return status;

        ImportConfig importConfig(sourceImportConfig.GetConfig());

        // For the moment we always want to combine imported source layers to first STM layer.
        importConfig.push_back(DefaultTargetLayerConfig(0));

        // Ensure that sources that have no GCSs and no user selected GCS may fall-back on the STM's GCS
        importConfig.push_back(DefaultSourceGCSConfig(targetGCS));

        // NEEDS_WORK_SM : ensure that sources that have no Extent ?
        importConfig.push_back(DefaultTargetScalableMeshConfig(targetScalableMeshData));

        if (!clipShapePtr->IsEmpty())
            importConfig.push_back(TargetFiltersConfig(ClipMaskFilterFactory::CreateFrom(clipShapePtr)));

        SourceRef sourceRef(CreateSourceRefFromIDTMSource(dataSource, m_scmFileName));


        importer.AddSource(sourceRef, sourceConfig, importConfig, importSequence, srcImportConfig/*, vecRange*/);
        }
    catch (...)
        {
        status = BSIERROR;
        }

    return status;
    }

int IScalableMeshCreator::Impl::TraverseSourceCollection  (SourcesImporter&                                            importer,
                                                    IDTMSourceCollection&                                 sources,
                                                    const HFCPtr<HVEClipShape>&                                 totalClipShapePtr,
                                                    const GCS&                                                  targetGCS,
                                                    const ScalableMeshData&                        targetScalableMeshData)

    {
    int status = BSISUCCESS;

    ClipShapeStoragePtr clipShapeStoragePtr = new ClipShapeStorage(totalClipShapePtr, targetGCS);


    typedef IDTMSourceCollection::reverse_iterator RevSourceIter;

    //The sources need to be parsed in the reverse order.
    for (RevSourceIter sourceIt = sources.rBeginEdit(), sourcesEnd = sources.rEndEdit();
        sourceIt != sourcesEnd && BSISUCCESS == status;
        ++sourceIt)
        {
        IDTMSource& source = *sourceIt;

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
                    const_cast<IDTMSourceCollection&>(sourceGroup.GetSources()),
                    groupClipShapePtr,
                    targetGCS,
                    targetScalableMeshData);
                }
            else
                {
                // Copy clip shape so that group clip and mask don't affect global clip shape.
                HFCPtr<HVEClipShape> groupClipShapePtr = new HVEClipShape(*(clipShapeStoragePtr->GetResultingClipShape()));

                status = TraverseSource(importer,
                    source,
                    groupClipShapePtr,
                    targetGCS,
                    targetScalableMeshData);
                }
        }

    return status;
    }

#endif

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
#if 0
IScalableMeshNodePtr IScalableMeshCreator::Impl::AddChildNode (const IScalableMeshNodePtr& parentNode, 
                                                               StatusInt&                  status)
    {
    if (parentNode == 0)
        {
        if (m_pDataIndex == 0)
            {         
            File::Ptr filePtr = SetupFileForCreation();
            if (0 == filePtr)
                {
                status = BSIERROR;
                return IScalableMeshNodePtr();                
                }

    #ifdef SCALABLE_MESH_DGN
            if(m_isDgnDb)
                {
                IDTMFile::AccessMode accessMode = filePtr->GetAccessMode();
                Bentley::WString filename = m_scmFileName;
                DgnDbFilename(filename);
                assert(accessMode.m_HasCreateAccess);
                    m_dgnScalableMeshPtr->Create(filename.GetWCharCP());
                }
    #endif


            //NEEDS_WORK_SM : Try put it in CreateDataIndex as sharedptr
            HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment myMemMgr (100, 2000 * sizeof(PointType));

            StatusInt status = CreateDataIndex (m_pDataIndex, filePtr, myMemMgr);
            assert(status == SUCCESS && m_pDataIndex != 0);
            }

       // m_pDataIndex

       // m_pDataIndex-
        }   

    if (parentNode != 0)    
        {
        
        }
    else
        {

        }




    //HFCPtr<SMPointIndexNode<POINT, EXTENT> >       GetRootNode() const;

    status = BSISUCCESS;
    return IScalableMeshNodePtr();  
    }
#endif

/*==================================================================*/
/*        MRDTM CREATOR SECTION - END                               */
/*==================================================================*/

END_BENTLEY_SCALABLEMESH_NAMESPACE