/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshCreator.cpp $
|    $RCSfile: ScalableMeshCreator.cpp,v $
|   $Revision: 1.90 $
|       $Date: 2012/01/27 16:45:29 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
USING_NAMESPACE_BENTLEY_TERRAINMODEL

#include "ScalableMeshCreator.h"
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


#include <ScalableMesh/IScalableMeshPolicy.h>

#include "ScalableMeshSources.h"

#include "InternalUtilityFunctions.h"
#include <STMInternal/GeoCoords/WKTUtils.h>

#include "ScalableMeshMesher.h"
#include "Edits\ClipRegistry.h"
//#include <DgnPlatform\Tools\ConfigurationManager.h>
#include "Edits/ClipUtilities.hpp"

#include "Stores\SMStreamingDataStore.h"
#include <ImagePP\all\h\HIMMosaic.h>
#ifndef VANCOUVER_API
#include <DgnPlatform\DesktopTools\ConfigurationManager.h>
#else
#include <DgnPlatform\Tools\ConfigurationManager.h>
#endif

#include "MosaicTextureProvider.h"
#include "StreamTextureProvider.h"

#define SCALABLE_MESH_TIMINGS

//NEEDS_WORK_SM : Temp global variable probably only for debug purpose, not sure we want to know if we are in editing.
bool s_inEditing = false; 
bool s_useThreadsInStitching = false;
bool s_useThreadsInMeshing = false;
bool s_useThreadsInFiltering = false;
bool s_useThreadsInTexturing = false;
bool s_useSpecialTriangulationOnGrids = false;
size_t s_nCreatedNodes = 0;
//extern DataSourceManager s_dataSourceManager;

using namespace ISMStore;


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

ISMPointIndexFilter<DPoint3d, Extent3dType>* s_filter = nullptr;

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
        //RegisterPODImportPlugin();
        s_areRegistered = true;
        }
    }

IScalableMeshCreatorPtr IScalableMeshCreator::GetFor (const WChar*  filePath,
                                        StatusInt&      status)
    {
    RegisterDelayedImporters();

    using namespace ISMStore;


    IScalableMeshCreatorPtr pCreator = new IScalableMeshCreator(new Impl(filePath));

    status = pCreator->m_implP->LoadFromFile();
    if (BSISUCCESS != status)
        return 0;

    return pCreator.get();
    }


IScalableMeshCreatorPtr IScalableMeshCreator::GetFor (const IScalableMeshPtr&    scmPtr,
                                        StatusInt&          status)
    {
    using namespace ISMStore;

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
  

SMStatus IScalableMeshCreator::Create (bool isSingleFile, bool restrictLevelForPropagation, bool doPartialUpdate)
    {
    return (SMStatus)m_implP->CreateScalableMesh(isSingleFile, restrictLevelForPropagation, doPartialUpdate);
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

static uint64_t s_getNbImportedPoints = 0;
static double s_getImportPointsDuration = -1; 
static double s_getLastPointBalancingDuration = -1;
static double s_getLastMeshBalancingDuration = -1;
static double s_getLastMeshingDuration = -1;
static double s_getLastFilteringDuration = -1;
static double s_getLastStitchingDuration = -1;
static double s_getLastClippingDuration = -1;
static double s_getLastFinalStoreDuration = -1;

uint64_t IScalableMeshCreator::GetNbImportedPoints()
    {
    return s_getNbImportedPoints;
    }

double IScalableMeshCreator::GetImportPointsDuration()
    {
    return s_getImportPointsDuration;
    }

double IScalableMeshCreator::GetLastPointBalancingDuration()
    {
    return s_getLastPointBalancingDuration;
    }

double IScalableMeshCreator::GetLastMeshBalancingDuration()
    {
    return s_getLastMeshBalancingDuration;
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

double IScalableMeshCreator::GetLastClippingDuration()
    {
    return s_getLastClippingDuration;
    }

double IScalableMeshCreator::GetLastFinalStoreDuration()
    {
    return s_getLastFinalStoreDuration;
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


StatusInt IScalableMeshCreator::SaveToFile()
    {
    return m_implP->SaveToFile();
    }

StatusInt IScalableMeshCreator::SetCompression(ScalableMeshCompressionType compressionType)
    {
    m_implP->m_compressionType = compressionType;
    return SUCCESS;
    }

StatusInt   IScalableMeshCreator::SetTextureMosaic(MOSAIC_TYPE* mosaicP)
    {
    return m_implP->SetTextureMosaic(mosaicP);
    }


StatusInt   IScalableMeshCreator::SetTextureProvider(ITextureProviderPtr provider)
    {
    return m_implP->SetTextureProvider(provider);
    }

StatusInt IScalableMeshCreator::SetTextureStreamFromUrl(WString url)
    {
    return m_implP->SetTextureStreamFromUrl(url);
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
bool IScalableMeshCreator::IsCanceled()
    {
    return m_implP->IsCanceled();
    }

void IScalableMeshCreator::Cancel()
    {
    return m_implP->Cancel();
    }
#endif

IScalableMeshProgress* IScalableMeshCreator::GetProgress()
    {
    return &*m_implP->GetProgress();
    }

bool  IScalableMeshCreator::IsShareable()
    {
    return m_implP->IsShareable();
    }

void  IScalableMeshCreator::SetShareable(bool isShareable)
    {
    return m_implP->SetShareable(isShareable);
    }

/*----------------------------------------------------------------------------+
|ScalableMeshCreator class
+----------------------------------------------------------------------------*/
IScalableMeshCreator::Impl::Impl(const WChar* scmFileName)
    : m_gcs(GetDefaultGCS()),
    m_scmFileName(scmFileName),
    m_lastSyncTime(Time::CreateSmallestPossible()),
    m_gcsDirty(false),
    //   m_sourceEnv(CreateSourceEnvFrom(scmFileName)),
    m_compressionType(SCM_COMPRESSION_DEFLATE),
    m_workingLayer(DEFAULT_WORKING_LAYER),
    m_isShareable(false),
    m_isCanceled(false),
    m_progress(new ScalableMeshProgress())
    {
    WString smStoreDgnDbStr;
    m_isDgnDb = false;

    s_useThreadsInMeshing = true;
    s_useThreadsInStitching = true;
    s_useThreadsInFiltering = true;
    m_progress->ProgressStep() = ScalableMeshStep::STEP_NOT_STARTED;
    m_progress->ProgressStepIndex() = 0;
    m_progress->Progress() = 0;
    m_progress->ProgressStepProcess() = ScalableMeshStepProcess::PROCESS_INACTIVE;
    m_progress->SetTotalNumberOfSteps(0);
    }

IScalableMeshCreator::Impl::Impl(const IScalableMeshPtr& scmPtr)
    :   m_gcs(GetDefaultGCS()),
        m_scmPtr(scmPtr),
        m_lastSyncTime(Time::CreateSmallestPossible()),
        m_gcsDirty(false),     
        m_compressionType(SCM_COMPRESSION_DEFLATE),
        m_workingLayer(DEFAULT_WORKING_LAYER),
        m_isShareable(false),
        m_isCanceled(false),
        m_progress(new ScalableMeshProgress())
    {  
    WString smStoreDgnDbStr;
    m_isDgnDb = false;
	s_useThreadsInMeshing = true;
	s_useThreadsInStitching = true;
	s_useThreadsInFiltering = true;
	m_progress->ProgressStep() = ScalableMeshStep::STEP_NOT_STARTED;
	m_progress->ProgressStepIndex() = 0;
	m_progress->Progress() = 0;
	m_progress->ProgressStepProcess() = ScalableMeshStepProcess::PROCESS_INACTIVE;
	m_progress->SetTotalNumberOfSteps(0);
    }

IScalableMeshCreator::Impl::~Impl()
    {
    //m_pDataIndex->DumpOctTree("D:\\NodeAferCreation.xml", false);
    if (m_pDataIndex) m_pDataIndex = 0;
    StatusInt status = SaveToFile();
    assert(BSISUCCESS == status);
    m_scmPtr = 0;
    }   

StatusInt IScalableMeshCreator::Impl::SetTextureMosaic(MOSAIC_TYPE* mosaicP)
    {
    if (m_scmPtr.get() == nullptr) return ERROR;

    GetProgress()->ProgressStep() = ScalableMeshStep::STEP_TEXTURE;
    GetProgress()->SetTotalNumberOfSteps(1);
    GetProgress()->ProgressStepProcess() = ScalableMeshStepProcess::PROCESS_TEXTURING;
    GetProgress()->ProgressStepIndex() = 1;
    GetProgress()->Progress() = 0.0;
	GetProgress()->UpdateListeners();
    ((ScalableMesh<DPoint3d>*)m_scmPtr.get())->GetMainIndexP()->SetProgressCallback(GetProgress());
    ((ScalableMesh<DPoint3d>*)m_scmPtr.get())->GetMainIndexP()->GatherCounts();

    mosaicP->IncrementRef();
    HFCPtr<HIMMosaic> mosaicPtr(mosaicP);
    ITextureProviderPtr textureProviderPtr = new MosaicTextureProvider(mosaicPtr);
    ((ScalableMesh<DPoint3d>*)m_scmPtr.get())->GetMainIndexP()->SetTextured(SMTextureType::Embedded);
    m_scmPtr->TextureFromRaster(textureProviderPtr);
    textureProviderPtr = nullptr;
    mosaicPtr = nullptr;
    mosaicP->DecrementRef();

    GetProgress()->Progress() = 1.0;
	GetProgress()->UpdateListeners();
    return SUCCESS;
    }
   

StatusInt IScalableMeshCreator::Impl::GetStreamedTextureProvider(ITextureProviderPtr& textureStreamProviderPtr, const WString& url)
    {
    DRange3d range;

    if (m_scmPtr.IsValid())
        m_scmPtr->GetRange(range);
    else
        {
        assert(m_dataIndex != nullptr);
        range = ScalableMesh<DPoint3d>::ComputeTotalExtentFor(m_dataIndex.GetPtr());
        }

    //TFS# 761652 - Avoid reprojection at this stage so that the minimum pixel resolution is consistent whatever the reprojection is.
    BaseGCSCPtr cs; //(GetGCS().GetGeoRef().GetBasePtr();)
    DRange2d extent2d = DRange2d::From(range);
    HFCPtr<HRARASTER> streamingRaster(RasterUtilities::LoadRaster(url, cs, extent2d));

    if (streamingRaster == nullptr)
        {
        return ERROR;
        }
    double ratioToMeterH = GetGCS().GetHorizontalUnit().GetRatioToBase();
    double ratioToMeterV = GetGCS().GetVerticalUnit().GetRatioToBase();

    Transform unitTransform;

#ifdef VANCOUVER_API
    unitTransform.InitIdentity();
    unitTransform.form3d[0][0] = ratioToMeterH;
    unitTransform.form3d[1][1] = ratioToMeterH;
    unitTransform.form3d[2][2] = ratioToMeterV;
#else  
    unitTransform.InitFromScaleFactors(ratioToMeterH, ratioToMeterH, ratioToMeterV);
#endif
   
    unitTransform.Multiply(range.low, range.low);
    unitTransform.Multiply(range.high, range.high);    

    if (m_scmPtr.IsValid())
        { 
        ((ScalableMesh<DPoint3d>*)m_scmPtr.get())->GetMainIndexP()->SetTextured(SMTextureType::Streaming);
        }
    else
        {
        assert(m_dataIndex != nullptr);
        m_dataIndex->SetTextured(SMTextureType::Streaming);
        }
        
    textureStreamProviderPtr = new StreamTextureProvider(streamingRaster, range);

    return SUCCESS;
    }

StatusInt IScalableMeshCreator::Impl::SetTextureStreamFromUrl(WString url)
    {
    if (m_scmPtr.get() == nullptr) return ERROR;

	GetProgress()->ProgressStep() = ScalableMeshStep::STEP_TEXTURE;
	GetProgress()->SetTotalNumberOfSteps(1);
	GetProgress()->ProgressStepProcess() = ScalableMeshStepProcess::PROCESS_TEXTURING_STREAMED;
	GetProgress()->ProgressStepIndex() = 1;
	GetProgress()->Progress() = 0.0;
	GetProgress()->UpdateListeners();

    
    ITextureProviderPtr textureStreamProviderPtr;
    StatusInt status;

    if (SUCCESS != (status = IScalableMeshCreator::Impl::GetStreamedTextureProvider(textureStreamProviderPtr, url)))
        return status;

    ((ScalableMesh<DPoint3d>*)m_scmPtr.get())->GetMainIndexP()->SetProgressCallback(GetProgress());
    ((ScalableMesh<DPoint3d>*)m_scmPtr.get())->GetMainIndexP()->GatherCounts();
    m_scmPtr->TextureFromRaster(textureStreamProviderPtr);

	GetProgress()->Progress() = 1.0;
	GetProgress()->UpdateListeners();
    return SUCCESS;
    }

StatusInt IScalableMeshCreator::Impl::SetTextureProvider(ITextureProviderPtr provider)
    {
    if (m_scmPtr.get() == nullptr) return ERROR;
    m_scmPtr->TextureFromRaster(provider);
    return SUCCESS;
    }

bool  IScalableMeshCreator::Impl::IsShareable()
{
    return m_isShareable;
}

void  IScalableMeshCreator::Impl::SetShareable(bool isShareable)
{
    if (m_smSQLitePtr.IsValid())
        return;
    m_isShareable = isShareable;    
}

ScalableMeshDb* IScalableMeshCreator::Impl::GetDatabaseFile()
{
    if (!m_smSQLitePtr.IsValid())
        return nullptr;
    return m_smSQLitePtr->GetDb();
}

/*
ScalableMeshFilterType scm_getFilterType ()
    {    
    //return SCM_FILTER_CGAL_SIMPLIFIER;
    return SCM_FILTER_DUMB_MESH;
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
*/

bool scm_isProgressiveFilter ()
    {
    return true;
    }

bool scm_isCompressed ()
    {
    return true;
    }

/*---------------------------------------------------------------------------------**//**
                                                                                      * @description
                                                                                      * @bsimethod                                                  Raymond.Gauthier   03/2011
                                                                                      +---------------+---------------+---------------+---------------+---------------+------*/
SourceRef CreateSourceRefFromIDTMSource(const IDTMSource& source, const WString& stmPath)
{
    struct Visitor : IDTMSourceVisitor
    {
        auto_ptr<SourceRef>         m_sourceRefP;
        const WString&              m_stmPath;

        explicit                    Visitor(const WString&                  stmPath)
            : m_stmPath(stmPath)
        {
        }

        virtual void                _Visit(const IDTMLocalFileSource&  source) override
        {
            StatusInt status = BSISUCCESS;
            const WChar* path = source.GetPath(status);
            if (BSISUCCESS != status)
                throw SourceNotFoundException();
#ifndef LINUX_SCALABLEMESH_BUILD
            if (0 == wcsicmp(path, m_stmPath.c_str()))
#else
            //Not case-insensitive here because as a rule paths aren't on Linux. Willing to reconsider if sufficient benefit.
            if (0 == wcscmp(path, m_stmPath.c_str()))
#endif
                throw CustomException(L"STM and source are the same");

            LocalFileSourceRef localSourceRef(path);

            m_sourceRefP.reset(new SourceRef(localSourceRef));
        }
        virtual void                _Visit(const IDTMDgnLevelSource&       source) override
        {
            StatusInt status = BSISUCCESS;
            if (BSISUCCESS != status)
                throw SourceNotFoundException();

            m_sourceRefP.reset(new SourceRef(DGNLevelByNameSourceRef(source.GetPath(),
                source.GetModelName(),
                source.GetLevelName())));
        }

        virtual void                _Visit(const IDTMDgnReferenceSource&       source) override
        {
            throw CustomException(L"Not supported");
        }
        virtual void                _Visit(const IDTMDgnReferenceLevelSource&  source) override
        {
            StatusInt status = BSISUCCESS;
            if (BSISUCCESS != status)
                throw SourceNotFoundException();

            m_sourceRefP.reset(new SourceRef(DGNReferenceLevelByIDSourceRef(source.GetPath(),
                source.GetModelID(),
                source.GetRootToRefPersistentPath(),
                source.GetLevelID())));
        }

        virtual void                _Visit(const IDTMDgnModelSource&            source) override
        {
            throw CustomException(L"Not supported");
        }
        virtual void                _Visit(const IDTMSourceGroup&      source) override
        {
      
        }
    };

    Visitor visitor(stmPath);
    source.Accept(visitor);

    if (0 == visitor.m_sourceRefP.get())
        throw CustomException(L"Unable to create source Ref from IDTMSource!");

    visitor.m_sourceRefP->SetDtmSource(source.Clone());

    return *visitor.m_sourceRefP;
}


int IScalableMeshCreator::Impl::CreateScalableMesh(bool isSingleFile, bool restrictLevelForPropagation, bool doPartialUpdate)
    {    
    int status = BSIERROR;
    return status;
    }

void IScalableMeshCreator::Impl::ConfigureMesherFilter(ISMPointIndexFilter<PointType, PointIndexExtentType>*& pFilter, ISMPointIndexMesher<PointType, PointIndexExtentType>*& pMesher2d, ISMPointIndexMesher<PointType, PointIndexExtentType>*& pMesher3d)
{

}

void IScalableMeshCreator::Impl::SetThreadingOptions(bool useThreadsInMeshing, bool useThreadsInStitching, bool useThreadsInFiltering)
    {    
    s_useThreadsInMeshing = useThreadsInMeshing;
    s_useThreadsInStitching = useThreadsInStitching;
    s_useThreadsInFiltering = useThreadsInFiltering;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IScalableMeshCreator::Impl::CreateDataIndex (HFCPtr<MeshIndexType>& pDataIndex, 
                                                       bool                   needBalancing, 
                                                       uint32_t               splitThreshold) 
    {                                    
    HFCPtr<IScalableMeshDataStore<MTGGraph, Byte, Byte>> pGraphTileStore;
    bool isSingleFile = true;

    isSingleFile = m_smSQLitePtr->IsSingleFile();
            
    // HFC_CREATE_ONLY if all sources are ADD (if one remove or one uptodate => read_write)
    //accessMode = filePtr->GetAccessMode();
    s_inEditing = false;


   /* ISMPointIndexFilter<PointType, PointIndexExtentType>* pFilter = s_filter != nullptr? s_filter:
        scm_createFilterFromType<PointType, PointIndexExtentType>(scm_getFilterType());

    ISMPointIndexMesher<PointType, PointIndexExtentType>* pMesher2_5d =
                Create2_5dMesherFromType<PointType, PointIndexExtentType>(Get2_5dMesherType());

    ISMPointIndexMesher<PointType, PointIndexExtentType>* pMesher3d =
                Create3dMesherFromType<PointType, PointIndexExtentType>(Get3dMesherType());*/
       
    ISMPointIndexFilter<PointType, PointIndexExtentType>* pFilter = nullptr;
    ISMPointIndexMesher<PointType, PointIndexExtentType>* pMesher2_5d = nullptr;
    ISMPointIndexMesher<PointType, PointIndexExtentType>* pMesher3d = nullptr;
    ConfigureMesherFilter(pFilter, pMesher2_5d, pMesher3d);

    if (!isSingleFile)
        {
        // SM_NEEDS_WORK_STREAMING : path should not depend on path to stm file. It should be a parameter to the creation process?
        auto position = m_scmFileName.find_last_of(L".3sm");
        int positionShift = 3;
    
        if (position == string::npos)
            {            
            position = m_scmFileName.find_last_of(L".stm2");
            positionShift = 4;
            }

        WString streamingFilePath = m_scmFileName.substr(0, position - positionShift) + L"_stream\\";
        BeFileName path(streamingFilePath.c_str());
        BeFileNameStatus createStatus = BeFileName::CreateNewDirectory(path);
        if (createStatus != BeFileNameStatus::Success && createStatus != BeFileNameStatus::AlreadyExists)
            {
            return ERROR;
            }

         #ifndef LINUX_SCALABLEMESH_BUILD      
        ISMDataStoreTypePtr<Extent3dType> dataStore(new SMStreamingStore<Extent3dType>(streamingFilePath, (SCM_COMPRESSION_DEFLATE == m_compressionType), true));
        
        pDataIndex = new MeshIndexType(dataStore, 
                                       ScalableMeshMemoryPools<PointType>::Get()->GetGenericPool(),                                                                                                                                                                                         
                                       splitThreshold,
                                       pFilter,
                                       needBalancing, false, false, true,
                                       pMesher2_5d,
                                       pMesher3d);

        Utf8String fileNameStr;
        m_smSQLitePtr->GetFileName(fileNameStr);
        BeFileName fileNameDir(fileNameStr.c_str());

#ifdef VANCOUVER_API
        BeFileName newFileNameDir(BeFileName::GetDirectoryName(fileNameDir).GetWCharCP());
        newFileNameDir.append(BeFileName::GetFileNameWithoutExtension(fileNameDir));
#else
        BeFileName newFileNameDir = fileNameDir.GetDirectoryName();
        newFileNameDir.append(fileNameDir.GetFileNameWithoutExtension());
#endif

        dataStore->SetProjectFilesPath(newFileNameDir);
        pDataIndex->SetSingleFile(false);
        pDataIndex->SetGenerating(true);
        #endif

       
        }
    else 
        {
        ISMDataStoreTypePtr<Extent3dType> dataStore(new SMSQLiteStore<Extent3dType>(m_smSQLitePtr));

        pDataIndex = new MeshIndexType(dataStore,
                                       ScalableMeshMemoryPools<PointType>::Get()->GetGenericPool(),
                                       splitThreshold,
                                       pFilter,
                                       needBalancing, false, false, true,
                                       pMesher2_5d,
                                       pMesher3d);

        Utf8String fileNameStr;
        m_smSQLitePtr->GetFileName(fileNameStr);
        BeFileName fileNameDir(fileNameStr.c_str());
#ifdef VANCOUVER_API
        BeFileName newFileNameDir(BeFileName::GetDirectoryName(fileNameDir).GetWCharCP());
        newFileNameDir.append(BeFileName::GetFileNameWithoutExtension(fileNameDir));
#else
        BeFileName newFileNameDir = fileNameDir.GetDirectoryName();
        newFileNameDir.append(fileNameDir.GetFileNameWithoutExtension());
#endif
        dataStore->SetProjectFilesPath(newFileNameDir);
        pDataIndex->SetGenerating(true);        
        }           
    if (pDataIndex != nullptr) pDataIndex->SetProgressCallback(GetProgress());

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
                success = m_smSQLitePtr->Open(m_scmFileName, false, m_isShareable); // open in read/write
        }
        else
           m_smSQLitePtr = dynamic_cast<const ScalableMeshBase&>(*m_scmPtr).GetDbFile();
    }
    else
        assert(m_smSQLitePtr->IsOpen() || m_smSQLitePtr->IsShared());

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

    ISMStore::WktFlavor fileWktFlavor = GetWKTFlavor(&wktStr, wktStr);

    BaseGCS::WktFlavor wktFlavor;

    bool result = MapWktFlavorEnum(wktFlavor, fileWktFlavor);

    assert(result);

    SMStatus gcsFromWKTStatus = SMStatus::S_SUCCESS;
    GCS fileGCS(GetGCSFactory().Create(wktStr.c_str(), wktFlavor, gcsFromWKTStatus));
    if (SMStatus::S_SUCCESS != gcsFromWKTStatus)
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

    SMStatus wktCreateStatus = SMStatus::S_SUCCESS;
    const WKT wkt(newSourceGCS.GetWKT(wktCreateStatus));
    if (SMStatus::S_SUCCESS != wktCreateStatus)
        return BSIERROR;

    WString extendedWktStr(wkt.GetCStr());

    if (WKTKeyword::TYPE_UNKNOWN == GetWktType(extendedWktStr))
    {
        wchar_t wktFlavor[2] = { (wchar_t)ISMStore::WktFlavor_Autodesk, L'\0' };

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

#ifndef VANCOUVER_API
    if (m_isShareable)
        {
        ScalableMeshDb* smDb(m_smSQLitePtr->GetDb());
        assert(smDb != nullptr);

        bool wasTransactionAbandoned;
        smDb->CommitTransaction();
        smDb->CloseShared(wasTransactionAbandoned);
        }
#endif


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

bool IScalableMeshCreator::Impl::IsCanceled()
    {
    return m_isCanceled;
    }

void IScalableMeshCreator::Impl::Cancel()
    {
    m_isCanceled = true;
    if (m_dataIndex) m_dataIndex->SetCanceled(true);
    }

IScalableMeshProgressPtr IScalableMeshCreator::Impl::GetProgress()
    {
    return m_progress;
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
#if _WIN32
    string meshing = to_string(meshingDuration);
    string filtering = to_string(filteringDuration);
    string stitching = to_string(stitchingDuration);

    string msg = "Meshing: "; msg += meshing;   msg += " min.\n";
    msg += "Filtering: "; msg += filtering; msg += " min.\n";
    msg += "Stitching: "; msg += stitching; msg += " min.\n";

    MessageBoxA(NULL, msg.c_str(), "Information", MB_ICONINFORMATION | MB_OK);
#endif
    }

/*==================================================================*/
/*        MRDTM CREATOR SECTION - END                               */
/*==================================================================*/

END_BENTLEY_SCALABLEMESH_NAMESPACE