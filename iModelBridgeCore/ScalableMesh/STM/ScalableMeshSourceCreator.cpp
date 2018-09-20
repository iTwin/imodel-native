/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourceCreator.cpp $
|    $RCSfile: ScalableMeshSourceCreator.cpp,v $
|   $Revision: 1.90 $
|       $Date: 2015/07/15 10:41:29 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
USING_NAMESPACE_BENTLEY_TERRAINMODEL

#include "ScalableMesh.h"
#include <ScalableMesh/ScalableMeshDefs.h>

#include "ScalableMeshSourceCreator.h"
#include "ScalableMeshSources.h"

#include "ScalableMeshSourcesPersistance.h"
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>
#include <ScalableMesh/IScalableMeshDocumentEnv.h>
#include <ScalableMesh/IScalableMeshSourceVisitor.h>

#include <ScalableMesh/Import/SourceReference.h>

#include <ScalableMesh/Import/Source.h>
#include <ScalableMesh/Import/Importer.h>
#include <ScalableMesh/Import/DataSQLite.h>

#include "../Import/Sink.h"
#include "ScalableMeshSourcesImport.h"

#include "ScalableMeshMesher.h"

#include <ScalableMesh/IScalableMeshPolicy.h>
#include "ScalableMeshSources.h"

#include <ScalableMesh/Import/Error/Source.h>
#include "ScalableMeshTime.h"
#include "Plugins/ScalableMeshClipMaskFilterFactory.h"
#include <ImagePP/all/h/HIMOnDemandMosaic.h>
#include <ImagePP/all/h/HIMMosaic.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include "ScalableMeshQuery.h"
#include "Edits/ClipUtilities.hpp"
#include "ScalableMeshQuadTreeBCLIBFilters.h"
#ifdef MAPBOX_PROTOTYPE
    #include <ImagePP\all\h\HRFMapboxFile.h>
#endif

#include <ImagePP\all\h\HRFiTiffCacheFileCreator.h>
#include <ImagePP\all\h\HRFUtility.h>
#include "MosaicTextureProvider.h"
#include "RasterUtilities.h"

using namespace ISMStore;
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

#ifndef __BENTLEYSTM_BUILD__ 
 bool s_inEditing = false;
 bool s_useThreadsInStitching = true;
 bool s_useThreadsInMeshing = true;
 bool s_useThreadsInTexturing = false;
 bool s_useSpecialTriangulationOnGrids = false;
#endif

 ISMPointIndexFilter<DPoint3d, Extent3dType>* s_filterClass = nullptr;

//ScalableMeshExistingMeshMesher<DPoint3d,DRange3d> s_ExistingMeshMesher;


size_t nGraphPins =0;
size_t nGraphReleases = 0;
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE



bool canCreateFile(const WChar* fileName)
{
	std::ofstream of;
	of.open(fileName);
	if (of.fail()) return false;
	else
	{
		of.close();
		_wremove(fileName);
	}
	return true;
}

// TDORAY: This is a duplicate of version in ScalableMesh.cpp. Find a way to use the same fn.
// TDORAY: Return a ref counted pointer
template<class POINT, class EXTENT>
static ISMPointIndexFilter<POINT, EXTENT>* scm_createFilterFromType(ScalableMeshFilterType filterType)
{
#ifdef VANCOUVER_API
    WString filterTypeStr;

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
#endif
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
    default:
        assert(!"Not supposed to be here");
    }
    return 0;
}

template<class POINT, class EXTENT>
static ISMPointIndexMesher<POINT, EXTENT>* Create2_5dMesherFromType(ScalableMeshMesherType mesherType)
{
    //Only one 2.5d mesher
    assert(mesherType == SCM_MESHER_2D_DELAUNAY);

    return new ScalableMesh2DDelaunayMesher<POINT, EXTENT>();
}


template<class POINT, class EXTENT>
static ISMPointIndexMesher<POINT, EXTENT>* Create3dMesherFromType(ScalableMeshMesherType mesherType)
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

ScalableMeshFilterType scm_getFilterType()
{
    //return SCM_FILTER_CGAL_SIMPLIFIER;
    return SCM_FILTER_DUMB_MESH;
}

ScalableMeshMesherType Get2_5dMesherType()
{
    return SCM_MESHER_2D_DELAUNAY;
}

ScalableMeshMesherType Get3dMesherType()
{
    //return SCM_MESHER_2D_DELAUNAY;    
    //return SCM_MESHER_3D_DELAUNAY;
#ifndef NO_3D_MESH
    return SCM_MESHER_TETGEN;
#else
    return SCM_MESHER_2D_DELAUNAY;
#endif
}


void IScalableMeshSourceCreator::Impl::ConfigureMesherFilter(ISMPointIndexFilter<PointType, PointIndexExtentType>*& pFilter, ISMPointIndexMesher<PointType, PointIndexExtentType>*& pMesher2d, ISMPointIndexMesher<PointType, PointIndexExtentType>*& pMesher3d)
{
   pFilter = 
        scm_createFilterFromType<PointType, PointIndexExtentType>(scm_getFilterType());

    pMesher2d =
        Create2_5dMesherFromType<PointType, PointIndexExtentType>(Get2_5dMesherType());

    pMesher3d =
        Create3dMesherFromType<PointType, PointIndexExtentType>(Get3dMesherType());
}


//static HPMPool* s_rasterMemPool = nullptr;
IScalableMeshSourceCreatorPtr IScalableMeshSourceCreator::GetFor(const WChar*  filePath,
	StatusInt&      status)
{
	RegisterDelayedImporters();

	using namespace ISMStore;
	BeFileName fileName = BeFileName(filePath);

#ifdef VANCOUVER_API
    if (fileName.IsUrl() || (!BeFileName::DoesPathExist(fileName.c_str()) && !canCreateFile(filePath)))
#else
    if (IsUrl(fileName) || (!fileName.DoesPathExist() && !canCreateFile(filePath)))
#endif	
	    {
		status = BSIERROR;
		return 0;
	}

    IScalableMeshSourceCreatorPtr pCreator = new IScalableMeshSourceCreator(new Impl(filePath));

    status = pCreator->m_implP->LoadFromFile();
    if (BSISUCCESS != status)
        return 0;

    return pCreator.get();
    }


IScalableMeshSourceCreatorPtr IScalableMeshSourceCreator::GetFor(const IScalableMeshPtr&    scmPtr,
                                                     StatusInt&          status)
    {
    using namespace ISMStore;

    RegisterDelayedImporters();

    IScalableMeshSourceCreatorPtr pCreator = new IScalableMeshSourceCreator(new Impl(scmPtr));

    status = pCreator->m_implP->LoadFromFile();
    if (BSISUCCESS != status)
        return 0;

    return pCreator.get();
    }


IScalableMeshSourceCreator::IScalableMeshSourceCreator(Impl* implP)
    : IScalableMeshCreator(implP)
    {}


IScalableMeshSourceCreator::~IScalableMeshSourceCreator()
    {
    //Since ScalableMeshCreator::~Impl is implemented in another DLL and its implementation is hidden the code below is require to ensure that the destructors of the 
    //Impl classes inheriting from ScalableMeshCreator::Impl are called.
    if (m_implP.get() != nullptr)
        {
        IScalableMeshSourceCreator::Impl* impl = (IScalableMeshSourceCreator::Impl*)m_implP.release();
        delete impl;
        }
    }

bool IScalableMeshSourceCreator::AreAllSourcesReachable() const
    {
    auto sources = dynamic_cast<IScalableMeshSourceCreator::Impl*>(m_implP.get())->m_sources;
    // Reachable only if all sources are reachable
    return sources.End() == std::find_if(sources.Begin(), sources.End(), not1(mem_fun_ref(&IDTMSource::IsReachable)));
    }

void IScalableMeshSourceCreator::SetSourceImportExtent(const DRange2d& ext)
    {
    dynamic_cast<IScalableMeshSourceCreator::Impl*>(m_implP.get())->m_extent = ext;
    }

void IScalableMeshSourceCreator::SetSourceImportPolygon(const DPoint3d* polygon, size_t nPts)
    {
    if (nPts > 0 && polygon != nullptr)
        dynamic_cast<IScalableMeshSourceCreator::Impl*>(m_implP.get())->m_filterPolygon.assign(polygon, polygon + nPts);
    }

void IScalableMeshSourceCreator::SetCreationMethod(ScalableMeshCreationMethod creationMethod)
    {
    assert(creationMethod >= 0 && creationMethod < SCM_CREATION_METHOD_QTY);
    dynamic_cast<IScalableMeshSourceCreator::Impl*>(m_implP.get())->m_sourceCreationMethod = creationMethod;
    }

void IScalableMeshSourceCreator::SetCreationCompleteness(ScalableMeshCreationCompleteness creationCompleteness)
    {
    assert(creationCompleteness >= 0 && creationCompleteness < SCM_CREATION_COMPLETENESS_QTY);
    dynamic_cast<IScalableMeshSourceCreator::Impl*>(m_implP.get())->m_sourceCreationCompleteness = creationCompleteness;
    }

void IScalableMeshSourceCreator::SetSourcesDirty()
    {
    // Make sure that sources are not seen as up to date anymore
    dynamic_cast<IScalableMeshSourceCreator::Impl*>(m_implP.get())->m_sourcesDirty = true;
    dynamic_cast<IScalableMeshSourceCreator::Impl*>(m_implP.get())->m_lastSourcesModificationTime = Time::CreateActual();
    }

bool IScalableMeshSourceCreator::HasDirtySources() const
    {
    return dynamic_cast<IScalableMeshSourceCreator::Impl*>(m_implP.get())->m_sourcesDirty;
    }

const IDTMSourceCollection& IScalableMeshSourceCreator::GetSources() const
    {
    return dynamic_cast<IScalableMeshSourceCreator::Impl*>(m_implP.get())->m_sources;
    }

IDTMSourceCollection& IScalableMeshSourceCreator::EditSources()
    {
    return dynamic_cast<IScalableMeshSourceCreator::Impl*>(m_implP.get())->m_sources;
    }


StatusInt IScalableMeshSourceCreator::UpdateLastModified()
    {
    return dynamic_cast<IScalableMeshSourceCreator::Impl*>(m_implP.get())->UpdateLastModified();
    }

void IScalableMeshSourceCreator::Impl::InitSources()
    {
    m_sources.RegisterEditListener(*this);
    };

IScalableMeshSourceCreator::Impl::Impl(const WChar* scmFileName)
    : IScalableMeshCreator::Impl(scmFileName),
    m_lastSourcesModificationTime(CreateUnknownModificationTime()),
    m_lastSourcesModificationCheckTime(Time::CreateSmallestPossible()),
    m_sourcesDirty(false),
    m_sourceEnv(CreateSourceEnvFrom(scmFileName)),
    m_extent(DRange2d::NullRange())
    {
    InitSources();
    }

IScalableMeshSourceCreator::Impl::Impl(const IScalableMeshPtr& scmPtr)
    : IScalableMeshCreator::Impl(scmPtr),
    m_lastSourcesModificationTime(CreateUnknownModificationTime()),
    m_lastSourcesModificationCheckTime(Time::CreateSmallestPossible()),
    m_sourcesDirty(false),
    m_sourceEnv(CreateSourceEnvFrom(dynamic_cast<const ScalableMeshBase&>(*m_scmPtr).GetPath())),
    m_extent(DRange2d::NullRange())
    {
    InitSources();
    }   

IScalableMeshSourceCreator::Impl::~Impl()
    {
m_sources.UnregisterEditListener(*this);

    }


DocumentEnv IScalableMeshSourceCreator::Impl::CreateSourceEnvFrom(const WChar* filePath)
    {
    WString currentDirPath(filePath);
    const WString::size_type dirEndPos = currentDirPath.find_last_of(L"\\/");

    if (WString::npos == dirEndPos)
        return DocumentEnv(L"");

    currentDirPath.resize(dirEndPos + 1);
    return DocumentEnv(currentDirPath.c_str());
    }


int IScalableMeshSourceCreator::Impl::CreateScalableMesh(bool isSingleFile, bool restrictLevelForPropagation, bool doPartialUpdate)
    {
    int status = SMStatus::S_SUCCESS;

    //MS : Some cleanup needs to be done here.
    try
        {
        if (m_scmPtr != 0)
            {
            if (SCM_STATE_UP_TO_DATE == m_scmPtr->GetState())
                return SMStatus::S_ERROR;

            // NOTE: Need to be able to recreate : Or the file offers some functions for deleting all its data directory or the file name can be obtained
            }


        SetupFileForCreation(doPartialUpdate);

        if (!m_smSQLitePtr.IsValid() || !m_smSQLitePtr->IsOpen())
            return SMStatus::S_ERROR;


        // Sync only when there are sources with which to sync
        // TR #325614: This special condition provides us with a way of efficiently detecting if STM is empty
        //             as indexes won't be initialized.

        m_smSQLitePtr->SetSingleFile(isSingleFile);

        if (0 < m_sources.GetCount() &&
            S_SUCCESS != SyncWithSources(restrictLevelForPropagation))
            status = SMStatus::S_ERROR;

        // Update last synchronization time
        m_lastSyncTime = Time::CreateActual();
        
        // Ensure that last modified times are up-to-date and that sources are saved.
        if (BSISUCCESS != UpdateLastModified() ||
            (isSingleFile && BSISUCCESS != SaveSources(m_smSQLitePtr)) ||
            BSISUCCESS != SaveGCS())
            status = SMStatus::S_ERROR;

		if (GetProgress()->IsCanceled())
			return SMStatus::S_ERROR_CANCELED_BY_USER;

        }
    catch (...)
        {
        return SMStatus::S_ERROR;
        }

    return status;
    }

void IScalableMeshSourceCreator::Impl::SetupFileForCreation(bool doPartialUpdate)
{
    using namespace ISMStore;

    //File::Ptr filePtr;

    if (doPartialUpdate)
        {
        bool bAllRemoved = true;
        bool bAllAdded = true;
        for (IDTMSourceCollection::const_iterator it = m_sources.Begin(); it != m_sources.End(); it++)
            {
            SourceImportConfig conf = it->GetConfig();
            ScalableMeshData data = conf.GetReplacementSMData();
            if (data.GetUpToDateState() != UpToDateState::REMOVE)
                bAllRemoved = false;
            if (data.GetUpToDateState() != UpToDateState::ADD)
                bAllAdded = false;
            }

        if (bAllRemoved)
            {
            _wremove(m_scmFileName.c_str());
            // Remove sources.
            m_sources.Clear();
            return;
            }
        }
    else
        {
        m_scmPtr = nullptr;
        m_smSQLitePtr = nullptr;

        if (FileExist() && 0 != _wremove(m_scmFileName.c_str()))
            return ;        

        m_smSQLitePtr = IScalableMeshCreator::Impl::GetFile(false);

        if (!m_smSQLitePtr.IsValid() || !m_smSQLitePtr->IsOpen())
            return;

        for (IDTMSourceCollection::iterator it = m_sources.BeginEdit(); it != m_sources.EndEdit(); it++)
            {
            SourceImportConfig conf = it->GetConfig();
            ScalableMeshData data = conf.GetReplacementSMData();
            if (data.GetUpToDateState() == UpToDateState::REMOVE)
                {
                it = m_sources.Remove(it);
                }                
            else 
            if (data.GetUpToDateState() == UpToDateState::MODIFY || data.GetUpToDateState() == UpToDateState::PARTIAL_ADD || data.GetUpToDateState() == UpToDateState::UP_TO_DATE)
                {
                data.SetUpToDateState(UpToDateState::ADD);
                conf.SetReplacementSMData(data);
                }
            }
        }


    // Ensure GCS and sources are save to the file.  
    m_gcsDirty = true;
    m_sourcesDirty = true;   
    //return filePtr;
}

void IScalableMeshSourceCreator::SetUserFilterCallback(MeshUserFilterCallback callback)
    {
//#ifdef WIP_MESH_IMPORT
    if (s_filterClass == nullptr) s_filterClass = new ScalableMeshQuadTreeBCLIB_UserMeshFilter<DPoint3d, PointIndexExtentType>();
    ((ScalableMeshQuadTreeBCLIB_UserMeshFilter<DPoint3d, PointIndexExtentType>*)s_filterClass)->SetCallback(callback);
//#endif 
    }


//NEEDS_WORK_SM : To be removed
static bool s_dumpOctreeNodes = false;
static bool s_mesh = true;
static bool s_filter = true;
static bool s_stitchFilteredByLevel = true;
static bool s_validateIs3dDataState = false;

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

uint64_t IScalableMeshSourceCreator::GetNbImportedPoints()
    {
    return s_getNbImportedPoints;
    }

double IScalableMeshSourceCreator::GetImportPointsDuration()
    {
    return s_getImportPointsDuration;
    }

double IScalableMeshSourceCreator::GetLastPointBalancingDuration()
    {
    return s_getLastPointBalancingDuration;
    }

double IScalableMeshSourceCreator::GetLastMeshBalancingDuration()
    {
    return s_getLastMeshBalancingDuration;
    }

double IScalableMeshSourceCreator::GetLastMeshingDuration()
    {
    return s_getLastMeshingDuration;
    }

double IScalableMeshSourceCreator::GetLastFilteringDuration()
    {
    return s_getLastFilteringDuration;
    }

double IScalableMeshSourceCreator::GetLastStitchingDuration()
    {
    return s_getLastStitchingDuration;
    }

double IScalableMeshSourceCreator::GetLastClippingDuration()
    {
    return s_getLastClippingDuration;
    }

double IScalableMeshSourceCreator::GetLastFinalStoreDuration()
    {
    return s_getLastFinalStoreDuration;
    }


void IScalableMeshSourceCreator::ImportRastersTo(const IScalableMeshPtr& scmPtr)
    {    
    ITextureProviderPtr textureProviderPtr;
    int status = dynamic_cast<IScalableMeshSourceCreator::Impl*>(m_implP.get())->GetTextureProvider(textureProviderPtr);
    scmPtr->TextureFromRaster(textureProviderPtr);
    assert(BSISUCCESS == status);
    }
#endif




/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IScalableMeshSourceCreator::Impl::SyncWithSources(
    bool restrictLevelForPropagation
    )
    {
    using namespace ISMStore;

    HFCPtr<MeshIndexType>          pDataIndex;

    HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment myMemMgr(100, 2000 * sizeof(PointType));

    uint32_t splitThreshold;    

    if (m_sourceCreationMethod == SCM_CREATION_METHOD_BIG_SPLIT_CUT)
        {
        splitThreshold = SM_BIG_SPLIT_THRESHOLD;
        }
    else
        {
        assert(m_sourceCreationMethod == SCM_CREATION_METHOD_ONE_SPLIT);
        splitThreshold = SM_ONE_SPLIT_THRESHOLD;
        }
    
    CreateDataIndex(pDataIndex, true, splitThreshold);

    m_dataIndex = pDataIndex;

    size_t previousDepth = pDataIndex->GetDepth();
    pDataIndex->SetIsTerrain(true);

    const GCS& fileGCS = GetGCS();

#ifdef SCALABLE_MESH_ATP

    s_getImportPointsDuration = -1;
    s_getLastPointBalancingDuration = -1;
    s_getLastMeshBalancingDuration = -1;
    s_getLastMeshingDuration = -1;
    s_getLastFilteringDuration = -1;
    s_getLastStitchingDuration = -1;
    s_getLastClippingDuration = -1;
    s_getLastFinalStoreDuration = -1;
    
    pDataIndex->m_nbInputPoints = 0;

    clock_t startClock = clock();
#endif


    GetProgress()->SetTotalNumberOfSteps(6);
    GetProgress()->ProgressStepProcess() = ScalableMeshStepProcess::PROCESS_GENERATION;

   // std::list<ISMStore::Extent3d64f> listRemoveExtent;
    std::list<DRange3d> listRemoveExtent;
    IDTMSourceCollection::const_iterator it = m_sources.Begin();

    for (IDTMSourceCollection::iterator itSource = m_sources.BeginEdit(); itSource != m_sources.EndEdit(); itSource++)
        {
        SourceImportConfig& conf = itSource->EditConfig();
        ScalableMeshData data = conf.GetReplacementSMData();
        if (data.GetUpToDateState() != UpToDateState::ADD)
            {
            s_inEditing = true;
            }

        if (data.GetUpToDateState() == UpToDateState::REMOVE || data.GetUpToDateState() == UpToDateState::MODIFY)
            {
            DRange3d sourceRange = data.GetExtentByLayer(0);
            /*ISMStore::Extent3d64f removeExtent;
            removeExtent.xMin = sourceRange.low.x;
            removeExtent.xMax = sourceRange.high.x;
            removeExtent.yMin = sourceRange.low.y;
            removeExtent.yMax = sourceRange.high.y;
            removeExtent.zMin = sourceRange.low.z;
            removeExtent.zMax = sourceRange.high.z;*/
            DRange3d removeExtent = sourceRange;

            for (size_t i = 1; i < data.GetLayerCount(); i++)
                {
                DRange3d sourceRangeI = data.GetExtentByLayer((int)i);
                /*removeExtent.xMin = removeExtent.xMin > sourceRange.low.x ? sourceRange.low.x : removeExtent.xMin;
                removeExtent.xMax = removeExtent.xMax > sourceRange.high.x ? removeExtent.xMax : sourceRange.high.x;
                removeExtent.yMin = removeExtent.yMin > sourceRange.low.y ? sourceRange.low.y : removeExtent.yMin;
                removeExtent.yMax = removeExtent.yMax > sourceRange.high.y ? removeExtent.yMax : sourceRange.high.y;
                removeExtent.zMin = removeExtent.zMin > sourceRange.low.z ? sourceRange.low.z : removeExtent.zMin;
                removeExtent.zMax = removeExtent.zMax > sourceRange.high.z ? removeExtent.zMax : sourceRange.high.z;*/
                removeExtent.Extend(sourceRangeI);
                }

            if (data.GetUpToDateState() == UpToDateState::MODIFY)
                {
                data.SetUpToDateState(UpToDateState::ADD);
                conf.SetReplacementSMData(data);
                }

            listRemoveExtent.push_back(removeExtent);
            for (IDTMSourceCollection::iterator itAdd = m_sources.BeginEdit(); itAdd != m_sources.EndEdit(); itAdd++)
                {
                if (itAdd == itSource)
                    continue;
                SourceImportConfig& confAdd = itAdd->EditConfig();
                ScalableMeshData dataAdd = confAdd.GetReplacementSMData();
                if (dataAdd.GetUpToDateState() != UpToDateState::UP_TO_DATE && dataAdd.GetUpToDateState() != UpToDateState::PARTIAL_ADD)
                    continue;
                DRange3d targetRange = dataAdd.GetExtentByLayer(0);
                for (size_t i = 1; i<dataAdd.GetLayerCount(); i++)
                    {
                    DRange3d targetRangeTemp = dataAdd.GetExtentByLayer((int)i);
                    targetRange.UnionOf(targetRange, targetRangeTemp);
                    }
                if (sourceRange.IntersectsWith(targetRange, 3))
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

    if (BSISUCCESS != RemoveSourcesFrom<MeshIndexType>(*pDataIndex, listRemoveExtent))
        return BSIERROR;

    if (GetProgress()->IsCanceled()) return BSISUCCESS;

    // Import sources

    GetProgress()->ProgressStep() = ScalableMeshStep::STEP_IMPORT_SOURCE;
    GetProgress()->ProgressStepIndex() = 1;
    GetProgress()->Progress() = 0.0;
	GetProgress()->UpdateListeners();

    if (BSISUCCESS != ImportSourcesTo(new ScalableMeshStorage<PointType>(*pDataIndex, fileGCS)))
        return BSIERROR;

    if (GetProgress()->IsCanceled()) return BSISUCCESS;
        
#ifndef VANCOUVER_API
//apparently they don't have this here. Either way, we only need the non-convex polygon support for ConceptStation
    if (m_filterPolygon.size() > 0 && !PolygonOps::IsConvex(m_filterPolygon))
        {
            pDataIndex->GetMesher2_5d()->AddClip(m_filterPolygon);
        }
#endif
    GetProgress()->Progress() = 1.0;
	GetProgress()->UpdateListeners();
#ifdef SCALABLE_MESH_ATP
    s_getImportPointsDuration = ((double)clock() - startClock) / CLOCKS_PER_SEC / 60.0;
    s_getNbImportedPoints = pDataIndex->m_nbInputPoints;

    startClock = clock();
#endif
    // Remove and Add sources
    for (IDTMSourceCollection::iterator itEdit = m_sources.BeginEdit(); itEdit != m_sources.End();)
        {
        SourceImportConfig& conf = itEdit->EditConfig();
        ScalableMeshData data = conf.GetReplacementSMData();

        if (data.GetUpToDateState() == UpToDateState::ADD || data.GetUpToDateState() == UpToDateState::PARTIAL_ADD)
            {
            data.SetUpToDateState(UpToDateState::UP_TO_DATE);
            conf.SetReplacementSMData(data);
            }
        if (data.GetUpToDateState() == UpToDateState::REMOVE)
            itEdit = m_sources.Remove(itEdit);
        else
            itEdit++;
        }

    GetProgress()->ProgressStep() = ScalableMeshStep::STEP_BALANCE;
    GetProgress()->ProgressStepIndex() = 2;
    GetProgress()->Progress() = 0.0;
	GetProgress()->UpdateListeners();
    if (pDataIndex->GetRootNode() == 0)
        {
#ifdef SCALABLE_MESH_ATP
        assert(pDataIndex->m_nbInputPoints == 0);
#endif
        return BSISUCCESS;
        }


#ifdef SCALABLE_MESH_ATP
    startClock = clock();
#endif

    if (!restrictLevelForPropagation)
        {
        bool splitNode = false;

        if (m_sourceCreationMethod == SCM_CREATION_METHOD_BIG_SPLIT_CUT)
            splitNode = true;

        // Balance data             
        if (BSISUCCESS != this->template BalanceDown<MeshIndexType>(*pDataIndex, previousDepth, splitNode))
            return BSIERROR;
        }
    else if (!s_inEditing)
        {
        size_t endLevel = pDataIndex->GetMaxFilledLevel();
        pDataIndex->PropagateDataDownImmediately((int)endLevel);
        }

    GetProgress()->Progress() = 1.0;
	GetProgress()->UpdateListeners();
    if (GetProgress()->IsCanceled()) return BSISUCCESS;

#ifdef SCALABLE_MESH_ATP
    s_getLastPointBalancingDuration = ((double)clock() - startClock) / CLOCKS_PER_SEC / 60.0;

    startClock = clock();
#endif


    if (m_sourceCreationCompleteness == SCM_CREATION_COMPLETENESS_INDEX_ONLY)
        {        
        assert(dynamic_cast<SMSQLiteStore<PointIndexExtentType>*>(pDataIndex->GetDataStore().get()) != nullptr);

        SMSQLiteStore<PointIndexExtentType>* pSqliteStore(static_cast<SMSQLiteStore<PointIndexExtentType>*>(pDataIndex->GetDataStore().get()));
        pSqliteStore->SetRemoveTempGenerationFile(false);
        }
    else
    if (m_sourceCreationCompleteness == SCM_CREATION_COMPLETENESS_FULL)
        {
        pDataIndex->GatherCounts();

        GetProgress()->ProgressStep() = ScalableMeshStep::STEP_MESH;
        GetProgress()->ProgressStepIndex() = 3;
        GetProgress()->Progress() = 0.0;
	    GetProgress()->UpdateListeners();

                
        if (s_mesh)
            {
            // Mesh data             
            if (BSISUCCESS != IScalableMeshCreator::Impl::Mesh<MeshIndexType>(*pDataIndex))
                return BSIERROR;        
            }    

#ifdef SCALABLE_MESH_ATP
        s_getLastMeshingDuration = ((double)clock() - startClock) / CLOCKS_PER_SEC / 60.0;
#endif
      
    
        GetProgress()->Progress() = 1.0;
	    GetProgress()->UpdateListeners();
        if (GetProgress()->IsCanceled()) return BSISUCCESS;


#ifdef SCALABLE_MESH_ATP        
        s_getLastStitchingDuration = 0;    
        s_getLastClippingDuration = 0;
        s_getLastMeshBalancingDuration = 0;    
#endif
     

        if (m_sourceCreationMethod == SCM_CREATION_METHOD_BIG_SPLIT_CUT)
            {        
            int depth = (int)pDataIndex->GetDepth();

#ifdef SCALABLE_MESH_ATP    
            startClock = clock();
#endif

            if (BSISUCCESS != IScalableMeshCreator::Impl::Stitch<MeshIndexType>(*pDataIndex, depth, false))
                return BSIERROR;

#ifdef SCALABLE_MESH_ATP    
            s_getLastStitchingDuration += clock() - startClock;
            startClock = clock();
#endif

            pDataIndex->CutTiles(SM_ONE_SPLIT_THRESHOLD);

#ifdef SCALABLE_MESH_ATP
            s_getLastClippingDuration = ((double)clock() - startClock) / CLOCKS_PER_SEC / 60.0;
            startClock = clock();
#endif
     
            // Balance data             
            if (BSISUCCESS != this->template BalanceDown<MeshIndexType>(*pDataIndex, previousDepth, false, false))
                return BSIERROR;

#ifdef SCALABLE_MESH_ATP
            s_getLastMeshBalancingDuration = ((double)clock() - startClock) / CLOCKS_PER_SEC / 60.0;        
#endif
            }

            {

#ifdef SCALABLE_MESH_ATP            
            s_getLastFilteringDuration = 0;
#endif

            int depth = (int)pDataIndex->GetDepth();
      
            GetProgress()->ProgressStep() = ScalableMeshStep::STEP_GENERATE_LOD;
            GetProgress()->ProgressStepIndex() = 4;
            GetProgress()->Progress() = 0.0;
		    GetProgress()->UpdateListeners();

            CachedDataEventTracer::GetInstance()->start();
   
            for (int level = depth; level >= 0; level--)
                {

#ifdef SCALABLE_MESH_ATP    
                startClock = clock();
#endif
                if (BSISUCCESS != IScalableMeshCreator::Impl::Filter<MeshIndexType>(*pDataIndex, level))
                    return BSIERROR;

                if (GetProgress()->IsCanceled()) return BSISUCCESS;

#ifdef SCALABLE_MESH_ATP    
                s_getLastFilteringDuration += clock() - startClock;
                startClock = clock();
#endif

                //The full resolution should already be stitched
                if ((m_sourceCreationMethod != SCM_CREATION_METHOD_BIG_SPLIT_CUT) || (level < depth))
                    {
                    if (BSISUCCESS != IScalableMeshCreator::Impl::Stitch<MeshIndexType>(*pDataIndex, level, false))
                        return BSIERROR;                
                    }

                if (GetProgress()->IsCanceled()) return BSISUCCESS;

#ifdef SCALABLE_MESH_ATP    
                s_getLastStitchingDuration += clock() - startClock;
                startClock = clock();
#endif
                }

            GetProgress()->Progress() = 1.0;
		    GetProgress()->UpdateListeners();
#ifdef SCALABLE_MESH_ATP    
            s_getLastStitchingDuration = s_getLastStitchingDuration / CLOCKS_PER_SEC / 60.0;
            s_getLastFilteringDuration = s_getLastFilteringDuration / CLOCKS_PER_SEC / 60.0;
#endif
                }
            //ShowMessageBoxWithTimes(s_getLastMeshingDuration, s_getLastFilteringDuration, s_getLastStitchingDuration);    


        #if 0

            if (s_validateIs3dDataState)
                {
                vector<DRange3d> source2_5dRanges;
                vector<DRange3d> source3dRanges;

                // Remove and Add sources
                for (IDTMSourceCollection::iterator itVal = m_sources.BeginEdit(); itVal != m_sources.End(); itVal++)
                    {
                    SourceImportConfig& conf = itVal->EditConfig();
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

            if (restrictLevelForPropagation)
                {
                pDataIndex->PropagateFullMeshDown();
                }

            GetProgress()->ProgressStep() = ScalableMeshStep::STEP_TEXTURE;
            GetProgress()->Progress() = 0.0;
            GetProgress()->ProgressStepIndex() = 5;
	        GetProgress()->UpdateListeners();
            ImportRasterSourcesTo(pDataIndex);
            ApplyEditsFromSources(pDataIndex);

            GetProgress()->Progress() = 1.0;
	        GetProgress()->UpdateListeners();
    #ifdef ACTIVATE_TEXTURE_DUMP
            pDataIndex->DumpAllNodeTextures();
    #endif
    #ifdef INDEX_DUMPING_ACTIVATED
            if (s_dumpOctreeNodes)
                {
                //pointIndex.DumpOctTree("D:\\MyDoc\\Scalable Mesh Iteration 7\\Partial Update - Remove\\Log\\NodeAferCreation.xml", false);    
                //pDataIndex->DumpOctTree("C:\\Users\\Thomas.Butzbach\\Documents\\data_scalableMesh\\ATP\\NodeAferCreation.xml", false);
                pDataIndex->DumpOctTree("e:\\output\\scmesh\\NodeAferCreation.xml", false);
                //pDataIndex->DumpOctTree("C:\\Users\\Richard.Bois\\Documents\\ScalableMesh\\Streaming\\QuebecCityMini\\NodeAferCreationAfterTextures.xml", false);
                }
    #endif
    }
    

    GetProgress()->ProgressStep() = ScalableMeshStep::STEP_SAVE;
    GetProgress()->ProgressStepIndex() = 6;
    GetProgress()->Progress() = 0.0;
	GetProgress()->UpdateListeners();

#ifdef SCALABLE_MESH_ATP
    startClock = clock();    
#endif

    pDataIndex->Store();
    m_smSQLitePtr->Save();

#ifdef SCALABLE_MESH_ATP
    s_getLastFinalStoreDuration = ((double)clock() - startClock) / CLOCKS_PER_SEC / 60.0;
#endif

    pDataIndex = 0;

    GetProgress()->Progress() = 1.0;
	GetProgress()->UpdateListeners();

    if (RasterUtilities::s_rasterMemPool != nullptr)
        {
        delete RasterUtilities::s_rasterMemPool;
        RasterUtilities::s_rasterMemPool = nullptr;
        }
           
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IScalableMeshSourceCreator::Impl::ResetLastModified ()
    {
    for (IDTMSourceCollection::iterator sourceIt = m_sources.BeginEditInternal(), sourceEnd = m_sources.EndEditInternal(); sourceIt != sourceEnd; ++sourceIt)
        sourceIt->ResetLastModified();

    m_lastSourcesModificationTime = CreateUnknownModificationTime();
    m_lastSourcesModificationCheckTime = Time::CreateSmallestPossible();
    }

bool IScalableMeshSourceCreator::Impl::IsFileDirty()
    {
    return m_sourcesDirty || m_gcsDirty;
    }

StatusInt IScalableMeshSourceCreator::Impl::Save()
{
    return BSISUCCESS == SaveSources(m_smSQLitePtr) && BSISUCCESS == SaveGCS();
}

StatusInt IScalableMeshSourceCreator::Impl::Load()
{
    return BSISUCCESS == LoadSources(m_smSQLitePtr) && BSISUCCESS == LoadGCS();
}


/*---------------------------------------------------------------------------------**//**
 * @description
 * @bsimethod                                                  Raymond.Gauthier   03/2011
 +---------------+---------------+---------------+---------------+---------------+------*/
int IScalableMeshSourceCreator::Impl::UpdateLastModified()
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
            if (difftime(data.GetTimeFile(), time) < 0)
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

int IScalableMeshSourceCreator::Impl::ApplyEditsFromSources(HFCPtr<MeshIndexType>& pIndex)
    {
    const GCS& fileGCS = GetGCS();
    Sink* sinkP = new ScalableMeshNonDestructiveEditStorage<PointType>(*pIndex, fileGCS);
    SinkPtr sinkPtr(sinkP);
    LocalFileSourceRef sinkSourceRef(m_scmFileName.c_str());

    const ContentDescriptor& targetContentDescriptor(sinkPtr->GetDescriptor());
    assert(1 == targetContentDescriptor.GetLayerCount());

    const GCS& targetGCS((*targetContentDescriptor.LayersBegin())->GetGCS());
    // NEEDS_WORK_SM : PARTIAL_UPDATE :remove
    const ScalableMeshData& targetScalableMeshData = ScalableMeshData::GetNull();

    SourcesImporter sourcesImporter(sinkSourceRef, sinkPtr);

    HFCPtr<HVEClipShape>  resultingClipShapePtr(new HVEClipShape(new HGF2DCoordSys()));

    int status;

    status = TraverseSourceCollectionEditsOnly(sourcesImporter,
                                      m_sources,
                                      resultingClipShapePtr,
                                      targetGCS,
                                      targetScalableMeshData);

    if (BSISUCCESS == status)
        {
        if (SMStatus::S_SUCCESS != sourcesImporter.Import())
            {
            status = BSIERROR;
            }
        }
    return status;
    }


StatusInt IScalableMeshSourceCreator::Impl::GetLocalSourceTextureProvider(ITextureProviderPtr& textureProviderPtr, bvector<IDTMSource*>& filteredSources)
    {       
    HFCPtr<HIMMosaic> pMosaicP(new HIMMosaic(HFCPtr<HGF2DCoordSys>(RasterUtilities::GetWorldCluster()->GetWorldReference(HGF2DWorld_HMRWORLD).GetPtr())));
    HIMMosaic::RasterList rasterList;

    for (auto& source : filteredSources)
    {
        WString path;

        assert(IsUrl(source->GetPath().c_str()) == false);
                
        path = WString(L"file://") + source->GetPath();
        


            // HFCPtr<HGF2DCoordSys>  pLogicalCoordSys;
            //HFCPtr<HRSObjectStore> pObjectStore;
            //HFCPtr<HRFRasterFile>  pRasterFile;
            HFCPtr<HRARaster>      pRaster;
        // HFCPtr<HRAOnDemandRaster> pOnDemandRaster;
        /*
        #ifdef MAPBOX_PROTOTYPE
        HFCPtr<HFCURL> pImageURL(HFCURL::Instanciate(path));

        if (HRFMapBoxCreator::GetInstance()->IsKindOfFile(pImageURL))
        {
        pRasterFile = HRFMapBoxCreator::GetInstance()->Create(pImageURL, HFC_READ_ONLY);
        }
        else
        #endif
        {
        pRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile(HFCURL::Instanciate(path), TRUE);
        }

        pRasterFile = GenericImprove(pRasterFile, HRFiTiffCacheFileCreator::GetInstance(), true, true);

        pLogicalCoordSys = cluster->GetWorldReference(pRasterFile->GetPageWorldIdentificator(0));
        pObjectStore = new HRSObjectStore(s_rasterMemPool,
        pRasterFile,
        0,
        pLogicalCoordSys);

        // Get the raster from the store
        pRaster = pObjectStore->LoadRaster();*/


        GCSCPTR replacementGcsPtr;
        const GCS& gcs(const_cast<SourceImportConfig*>(&source->GetConfig())->GetReplacementGCS());
        

        if (gcs.HasGeoRef())
            {
            replacementGcsPtr = gcs.GetGeoRef().GetBasePtr();                            
            }

        GCSCPTR destinationGcsPtr;

        if (m_gcs.HasGeoRef())
            {
            destinationGcsPtr = m_gcs.GetGeoRef().GetBasePtr();
            }        
        
        HASSERT(m_dataIndex != nullptr);
          
        DRange2d extentInTargetCS(DRange2d::From(m_dataIndex->GetContentExtent()));

        pRaster = RasterUtilities::LoadRaster(path, destinationGcsPtr, extentInTargetCS, replacementGcsPtr);

        HASSERT(pRaster != NULL);        
    
        //NEEDS_WORK_SM: do not do this if raster does not intersect sm extent
        rasterList.push_back(pRaster.GetPtr());
        pRaster = 0;
    }
    pMosaicP->Add(rasterList);
    rasterList.clear();

    textureProviderPtr = new MosaicTextureProvider(pMosaicP);

    return SUCCESS;
    }

StatusInt IScalableMeshSourceCreator::Impl::GetRasterSources(bvector<IDTMSource*>& filteredSources)
    {
    const GCS& fileGCS = GetGCS();
    const ScalableMeshData& targetScalableMeshData = ScalableMeshData::GetNull();

    HFCPtr<HVEClipShape>  resultingClipShapePtr(new HVEClipShape(new HGF2DCoordSys()));

    int status = BSISUCCESS;

    status = TraverseSourceCollectionRasters(filteredSources,
                                             m_sources,
                                             resultingClipShapePtr,
                                             fileGCS,
                                             targetScalableMeshData);

    return status;

    }

StatusInt IScalableMeshSourceCreator::Impl::GetTextureProvider(ITextureProviderPtr& textureProviderPtr)
    {
    bvector<IDTMSource*> filteredSources;

    StatusInt status;

    if (SUCCESS != (status = IScalableMeshSourceCreator::Impl::GetRasterSources(filteredSources)))
        return status;

    if (filteredSources.empty())
        return SUCCESS;

    bool containtStreamingSource = false;

    for (auto& source : filteredSources)
    {
        WString path;

        if (IsUrl(source->GetPath().c_str()))
        {
            containtStreamingSource = true;
            assert(filteredSources.size() == 1); //Currently only support one streaming source (BingMap).
            break;
        }
    }

    if (containtStreamingSource)
    {             
        return GetStreamedTextureProvider(textureProviderPtr, filteredSources[0]->GetPath());
    }
    
    return GetLocalSourceTextureProvider(textureProviderPtr, filteredSources);
    }

int IScalableMeshSourceCreator::Impl::ImportRasterSourcesTo(HFCPtr<MeshIndexType>& pIndex)
    {

    ITextureProviderPtr textureProviderPtr;

    StatusInt status = GetTextureProvider(textureProviderPtr);

    if (BSISUCCESS != status) return BSIERROR;
    if (textureProviderPtr == nullptr) return BSISUCCESS;    
    
    //Image++ is always in meters, so ensure 3SM data are transformed to meter
    Transform unitTransform;

    unitTransform.InitFromRowValues(m_gcs.GetUnit().GetRatioToBase(), 0 ,0, 0,
                                    0, m_gcs.GetUnit().GetRatioToBase(), 0, 0,
                                    0, 0, m_gcs.GetUnit().GetRatioToBase(), 0);
            
    pIndex->TextureFromRaster(textureProviderPtr, unitTransform);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointIndex>       
StatusInt IScalableMeshSourceCreator::Impl::RemoveSourcesFrom(PointIndex& pointIndex, std::list<DRange3d> listRemoveExtent) const
    {
    //NEEDS_WORK_SM : Logic for determining the extent to remove should be here.  
    std::list<DRange3d>::const_iterator it = listRemoveExtent.begin();
    for (std::list<DRange3d>::const_iterator itRemove = listRemoveExtent.begin(); itRemove != listRemoveExtent.end(); itRemove++)
        {
            {
            pointIndex.RemovePoints(*itRemove);
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IScalableMeshSourceCreator::Impl::ImportSourcesTo(Sink* sinkP)
    {
    SinkPtr sinkPtr(sinkP);
    LocalFileSourceRef sinkSourceRef(m_scmFileName.c_str());

    const ContentDescriptor& targetContentDescriptor(sinkPtr->GetDescriptor());
    assert(1 == targetContentDescriptor.GetLayerCount());

    const GCS& targetGCS((*targetContentDescriptor.LayersBegin())->GetGCS());
    // NEEDS_WORK_SM : PARTIAL_UPDATE :remove
    const ScalableMeshData& targetScalableMeshData = ScalableMeshData::GetNull();

    SourcesImporter sourcesImporter(sinkSourceRef, sinkPtr);

    HFCPtr<HVEClipShape>  resultingClipShapePtr(new HVEClipShape(new HGF2DCoordSys()));
    HFCPtr<HVEShape> clip;
    if (!m_extent.IsNull())
        {
        clip = new HVEShape(m_extent.low.x, m_extent.low.y, m_extent.high.x, m_extent.high.y, resultingClipShapePtr->GetCoordSys());
        resultingClipShapePtr->AddClip(clip, false);
        }
    if (!m_filterPolygon.empty())
        {
        bvector<double> vecOfPtCoordinates;
        for (auto& pt : m_filterPolygon)
            {
            vecOfPtCoordinates.push_back(pt.x);
            vecOfPtCoordinates.push_back(pt.y);
            }
        size_t size = m_filterPolygon.size() * 2;
        clip = new HVEShape(&size, &vecOfPtCoordinates[0], resultingClipShapePtr->GetCoordSys());
        resultingClipShapePtr->AddClip(clip, false);
        }

    int status;

    status = TraverseSourceCollection(sourcesImporter,
        m_sources,
        resultingClipShapePtr,
        targetGCS,
        targetScalableMeshData);

    if (BSISUCCESS == status)
        {
        if (SMStatus::S_SUCCESS != sourcesImporter.Import())
            {
            status = BSIERROR;
            }
        }


    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IScalableMeshSourceCreator::Impl::LoadSources(SMSQLiteFilePtr& smSQLiteFile)
{
    if (!smSQLiteFile->HasSources())
        return BSISUCCESS; // No sources were added to the STM.

    // Create SourceDataSQLite to wrap SQLite and Serializer
    SourcesDataSQLite* sourcesData = new SourcesDataSQLite();
    smSQLiteFile->LoadSources(*sourcesData);
    bool success = true;
    success &= BENTLEY_NAMESPACE_NAME::ScalableMesh::LoadSources(m_sources, *sourcesData, m_sourceEnv);

    m_lastSourcesModificationCheckTime = CreateTimeFrom(sourcesData->GetLastModifiedCheckTime());
    m_lastSourcesModificationTime = CreateTimeFrom(sourcesData->GetLastModifiedTime());
    m_lastSyncTime = CreateTimeFrom(sourcesData->GetLastSyncTime());

    return (success) ? BSISUCCESS : BSIERROR;
}

int IScalableMeshSourceCreator::Impl::SaveSources(SMSQLiteFilePtr& smSQLiteFile)
{
    if (!m_sourcesDirty)
        return BSISUCCESS;

    SourcesDataSQLite* sourcesData = new SourcesDataSQLite();


    bool success = true;

    // Create Data
    success &= BENTLEY_NAMESPACE_NAME::ScalableMesh::SaveSources(m_sources, *sourcesData, m_sourceEnv);
    sourcesData->SetLastModifiedCheckTime(GetCTimeFor(m_lastSourcesModificationCheckTime));
    sourcesData->SetLastModifiedTime(GetCTimeFor(m_lastSourcesModificationTime));
    sourcesData->SetLastSyncTime(GetCTimeFor(m_lastSyncTime));

    smSQLiteFile->SaveSource(*sourcesData);

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
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IScalableMeshSourceCreator::Impl::_NotifyOfPublicEdit()
    {
    // Make sure that sources are not seen as up to date anymore
    m_sourcesDirty = true;
    m_lastSourcesModificationTime = Time::CreateActual();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IScalableMeshSourceCreator::Impl::_NotifyOfLastEditUpdate(Time updatedLastEditTime)
    {
    m_lastSourcesModificationTime = (std::max)(m_lastSourcesModificationTime, updatedLastEditTime);
    }


int IScalableMeshSourceCreator::Impl::ImportClipMaskSource(const IDTMSource&       dataSource,
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

        const SourcePtr sourcePtr = Configure(originalSourcePtr, dataSource.GetConfig().GetContentConfig());
        if(0 == sourcePtr.get())
            return BSIERROR;

        // Create importer
        clipShapeStoragePtr->SetIsMaskSource(dataSource.GetSourceType() == DTM_SOURCE_DATA_MASK);

        const ImporterPtr importerPtr = IMPORTER_FACTORY.Create(sourcePtr, &(*clipShapeStoragePtr));
        if(0 == importerPtr.get())
            return BSIERROR;

        // Import
        const SMStatus importStatus = importerPtr->Import(dataSource.GetConfig().GetSequence(), *dataSource.GetConfig().GetConfig());
        if (importStatus != SMStatus::S_SUCCESS)
            return BSIERROR;

        return BSISUCCESS;
        }
    catch (...)
        {
        return BSIERROR;
        }
    }


int IScalableMeshSourceCreator::Impl::TraverseSource(SourcesImporter&                                         importer,
                                          IDTMSource&                                              dataSource,
                                          const HFCPtr<HVEClipShape>&                              clipShapePtr,
                                          const GCS&                                               targetGCS,
                                          const ScalableMeshData&                    targetScalableMeshData) const
    {
    int status = S_SUCCESS;
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

        RefCountedPtr<ImportConfig> importConfig(sourceImportConfig.GetConfig());

        // For the moment we always want to combine imported source layers to first STM layer.
        //importConfig.push_back(DefaultTargetLayerConfig(0));
        importConfig->SetDefaultTargetLayer(0);

        // Ensure that sources that have no GCSs and no user selected GCS may fall-back on the STM's GCS
        //importConfig.push_back(DefaultSourceGCSConfig(targetGCS));
        importConfig->SetDefaultSourceGCS(new GCS(targetGCS));

        // NEEDS_WORK_SM : ensure that sources that have no Extent ?
        //importConfig.push_back(DefaultTargetScalableMeshConfig(targetScalableMeshData));
        importConfig->SetDefaultTargetSMData(new ScalableMeshData(targetScalableMeshData));

        if (!clipShapePtr->IsEmpty())
            {
            CustomFilteringSequence seq;
            seq.push_back(ClipMaskFilterFactory::CreateFrom(clipShapePtr));
            //importConfig.push_back(TargetFiltersConfig(ClipMaskFilterFactory::CreateFrom(clipShapePtr)));
            importConfig->SetTargetFilters(seq);
            }

        SourceRef sourceRef(CreateSourceRefFromIDTMSource(dataSource, m_scmFileName));

#ifndef VANCOUVER_API    
        if (dynamic_cast<DGNLevelByNameSourceRef*>((sourceRef.m_basePtr.get())) != nullptr || dynamic_cast<DGNReferenceLevelByNameSourceRef*>((sourceRef.m_basePtr.get())) != nullptr)
            {
            importConfig->SetClipShape(clipShapePtr);
            importer.AddSDKSource(sourceRef, sourceConfig, importConfig.get(), importSequence, srcImportConfig/*, vecRange*/);
            }
        else
#endif
            {
            importer.AddSource(sourceRef, sourceConfig, importConfig.get(), importSequence, srcImportConfig/*, vecRange*/);
            }
        }
    catch (...)
        {
        status = S_ERROR;
        }

    return status;
    }

int IScalableMeshSourceCreator::Impl::TraverseSourceCollection(SourcesImporter&                                            importer,
                                                    IDTMSourceCollection&                                 sources,
                                                    const HFCPtr<HVEClipShape>&                                 totalClipShapePtr,
                                                    const GCS&                                                  targetGCS,
                                                    const ScalableMeshData&                        targetScalableMeshData)

    {
    int status = S_SUCCESS;

    ClipShapeStoragePtr clipShapeStoragePtr = new ClipShapeStorage(totalClipShapePtr, targetGCS);


    typedef IDTMSourceCollection::reverse_iterator RevSourceIter;

    //The sources need to be parsed in the reverse order.
    for (RevSourceIter sourceIt = sources.rBeginEdit(), sourcesEnd = sources.rEndEdit();
        sourceIt != sourcesEnd && S_SUCCESS == status;
        ++sourceIt)
        {
        IDTMSource& source = *sourceIt;

        if ((source.GetSourceType() == DTM_SOURCE_DATA_CLIP) ||
            (source.GetSourceType() == DTM_SOURCE_DATA_MASK))
            {
            status = ImportClipMaskSource(source,
                clipShapeStoragePtr);
            }
        else if (source.GetSourceType() == DTM_SOURCE_DATA_IMAGE) {}
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

int IScalableMeshSourceCreator::Impl::TraverseSourceCollectionEditsOnly(SourcesImporter&                                            importer,
                                                               IDTMSourceCollection&                                 sources,
                                                               const HFCPtr<HVEClipShape>&                                 totalClipShapePtr,
                                                               const GCS&                                                  targetGCS,
                                                               const ScalableMeshData&                        targetScalableMeshData)

    {
    int status = S_SUCCESS;

    ClipShapeStoragePtr clipShapeStoragePtr = new ClipShapeStorage(totalClipShapePtr, targetGCS);


    typedef IDTMSourceCollection::reverse_iterator RevSourceIter;

    //The sources need to be parsed in the reverse order.
    for (RevSourceIter sourceIt = sources.rBeginEdit(), sourcesEnd = sources.rEndEdit();
         sourceIt != sourcesEnd && S_SUCCESS == status;
         ++sourceIt)
        {
        IDTMSource& source = *sourceIt;

        if ((source.GetSourceType() == DTM_SOURCE_DATA_CLIP) ||
            (source.GetSourceType() == DTM_SOURCE_DATA_MASK))
            {
            status = ImportClipMaskSource(source,
                                          clipShapeStoragePtr);
            }
        else if (source.GetSourceType() == DTM_SOURCE_DATA_HARD_MASK)
            if (dynamic_cast<const IDTMSourceGroup*>(&source) != 0)
                {
                const IDTMSourceGroup& sourceGroup = dynamic_cast<const IDTMSourceGroup&>(source);

                // Copy clip shape so that group clip and mask don't affect global clip shape.
                HFCPtr<HVEClipShape> groupClipShapePtr = new HVEClipShape(*(clipShapeStoragePtr->GetResultingClipShape()));

                status = TraverseSourceCollectionEditsOnly(importer,
                                                  const_cast<IDTMSourceCollection&>(sourceGroup.GetSources()),
                                                  groupClipShapePtr,
                                                  targetGCS,
                                                  targetScalableMeshData);
                }
            else
                {
                // Copy clip shape so that group clip and mask don't affect global clip shape.
                HFCPtr<HVEClipShape> groupClipShapePtr = new HVEClipShape(*(clipShapeStoragePtr->GetResultingClipShape()));
                SourceImportConfig& srcImportConfig = source.EditConfig();
                ScalableMeshData data = srcImportConfig.GetReplacementSMData();
                data.SetUpToDateState(Import::UpToDateState::ADD);
                srcImportConfig.SetReplacementSMData(data);
                status = TraverseSource(importer,
                                        source,
                                        groupClipShapePtr,
                                        targetGCS,
                                        targetScalableMeshData);
                }
        }

    return status;
    }


int IScalableMeshSourceCreator::Impl::TraverseSourceCollectionRasters(bvector<IDTMSource*>&                                  filteredSources,
                                                                        IDTMSourceCollection&                                 sources,
                                                                        const HFCPtr<HVEClipShape>&                                 totalClipShapePtr,
                                                                        const GCS&                                                  targetGCS,
                                                                        const ScalableMeshData&                        targetScalableMeshData)

    {
    int status = S_SUCCESS;

    ClipShapeStoragePtr clipShapeStoragePtr = new ClipShapeStorage(totalClipShapePtr, targetGCS);


    typedef IDTMSourceCollection::reverse_iterator RevSourceIter;

    //The sources need to be parsed in the reverse order.
    for (RevSourceIter sourceIt = sources.rBeginEdit(), sourcesEnd = sources.rEndEdit();
         sourceIt != sourcesEnd && S_SUCCESS == status;
         ++sourceIt)
        {
        IDTMSource& source = *sourceIt;

        if ((source.GetSourceType() == DTM_SOURCE_DATA_CLIP) ||
            (source.GetSourceType() == DTM_SOURCE_DATA_MASK))
            {
            status = ImportClipMaskSource(source,
                                          clipShapeStoragePtr);
            }
        else if (source.GetSourceType() == DTM_SOURCE_DATA_IMAGE)
            {
            if (dynamic_cast<const IDTMSourceGroup*>(&source) != 0)
                {
                const IDTMSourceGroup& sourceGroup = dynamic_cast<const IDTMSourceGroup&>(source);

                // Copy clip shape so that group clip and mask don't affect global clip shape.
                HFCPtr<HVEClipShape> groupClipShapePtr = new HVEClipShape(*(clipShapeStoragePtr->GetResultingClipShape()));

                status = TraverseSourceCollectionRasters(filteredSources,
                                                         const_cast<IDTMSourceCollection&>(sourceGroup.GetSources()),
                                                         groupClipShapePtr,
                                                         targetGCS,
                                                         targetScalableMeshData);
                }
            else
                {
                // Copy clip shape so that group clip and mask don't affect global clip shape.
                HFCPtr<HVEClipShape> groupClipShapePtr = new HVEClipShape(*(clipShapeStoragePtr->GetResultingClipShape()));
                SourceImportConfig& srcImportConfig = source.EditConfig();
                ScalableMeshData data = srcImportConfig.GetReplacementSMData();
                data.SetUpToDateState(Import::UpToDateState::ADD);
                srcImportConfig.SetReplacementSMData(data);
                filteredSources.push_back(&source);
                }
            }
        }

    return status;
    }
END_BENTLEY_SCALABLEMESH_NAMESPACE