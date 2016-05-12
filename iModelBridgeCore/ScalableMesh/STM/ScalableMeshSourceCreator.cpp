/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourceCreator.cpp $
|    $RCSfile: ScalableMeshSourceCreator.cpp,v $
|   $Revision: 1.90 $
|       $Date: 2015/07/15 10:41:29 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

#include <ScalableMesh/Import/Config/All.h>
#include <ScalableMesh/IScalableMeshStream.h>

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
#include "SMSQLiteFeatureTileStore.h"
using namespace IDTMFile;
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
extern bool s_inEditing;
bool s_useThreadsInStitching = false;
bool s_useThreadsInMeshing = false;
bool s_useThreadsInFiltering = false;
size_t s_nCreatedNodes = 0;
bool s_useSpecialTriangulationOnGrids = false;
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


static HPMPool* s_rasterMemPool = nullptr;
IScalableMeshSourceCreatorPtr IScalableMeshSourceCreator::GetFor(const WChar*  filePath,
StatusInt&      status)
    {
    RegisterDelayedImporters();

    using namespace IDTMFile;

    IScalableMeshSourceCreatorPtr pCreator = new IScalableMeshSourceCreator(new Impl(filePath));

    status = pCreator->m_implP->LoadFromFile();
    if (BSISUCCESS != status)
        return 0;

    return pCreator.get();
    }


IScalableMeshSourceCreatorPtr IScalableMeshSourceCreator::GetFor(const IScalableMeshPtr&    scmPtr,
                                                     StatusInt&          status)
    {
    using namespace IDTMFile;

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
    {}

bool IScalableMeshSourceCreator::AreAllSourcesReachable() const
    {
    auto sources = dynamic_cast<IScalableMeshSourceCreator::Impl*>(m_implP.get())->m_sources;
    // Reachable only if all sources are reachable
    return sources.End() == std::find_if(sources.Begin(), sources.End(), not1(mem_fun_ref(&IDTMSource::IsReachable)));
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
    m_sourceEnv(CreateSourceEnvFrom(scmFileName))
    {
    InitSources();
    }

IScalableMeshSourceCreator::Impl::Impl(const IScalableMeshPtr& scmPtr)
    : IScalableMeshCreator::Impl(scmPtr),
    m_lastSourcesModificationTime(CreateUnknownModificationTime()),
    m_lastSourcesModificationCheckTime(Time::CreateSmallestPossible()),
    m_sourcesDirty(false),
    m_sourceEnv(CreateSourceEnvFrom(dynamic_cast<const ScalableMeshBase&>(*m_scmPtr).GetPath()))
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


int IScalableMeshSourceCreator::Impl::CreateScalableMesh(bool isSingleFile)
    {
    int status = BSISUCCESS;

    //MS : Some cleanup needs to be done here.
    try
        {
        if (m_scmPtr != 0)
            {
            if (SCM_STATE_UP_TO_DATE == m_scmPtr->GetState())
                return BSIERROR;

            // NOTE: Need to be able to recreate : Or the file offers some functions for deleting all its data directory or the file name can be obtained
            }


        SetupFileForCreation();

        // Sync only when there are sources with which to sync
        // TR #325614: This special condition provides us with a way of efficiently detecting if STM is empty
        //             as indexes won't be initialized.

        m_smSQLitePtr->SetSingleFile(isSingleFile);

        if (0 < m_sources.GetCount() &&
            BSISUCCESS != SyncWithSources())
            return BSIERROR;


        // Update last synchronization time
        m_lastSyncTime = Time::CreateActual();
        
        // Ensure that last modified times are up-to-date and that sources are saved.
        if (BSISUCCESS != UpdateLastModified() ||
            (isSingleFile && BSISUCCESS != SaveSources(m_smSQLitePtr)) ||
            BSISUCCESS != SaveGCS())
            status = BSIERROR;

        }
    catch (...)
        {
        return BSIERROR;
        }

    return status;
    }

void IScalableMeshSourceCreator::Impl::SetupFileForCreation()
{
    using namespace IDTMFile;

    //File::Ptr filePtr;
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

    // Ensure GCS and sources are save to the file.
    m_gcsDirty = true;
    m_sourcesDirty = true;
    //return filePtr;
}


//NEEDS_WORK_SM : To be removed
static bool s_dumpOctreeNodes = false;
static bool s_mesh = true;
static bool s_filter = true;
static bool s_stitchFilteredByLevel = true;
static bool s_validateIs3dDataState = false;

#ifdef SCALABLE_MESH_ATP

static unsigned __int64 s_getNbImportedPoints = 0;
static double s_getImportPointsDuration = -1;
static double s_getLastBalancingDuration = -1;
static double s_getLastMeshingDuration = -1;
static double s_getLastFilteringDuration = -1;
static double s_getLastStitchingDuration = -1;

unsigned __int64 IScalableMeshSourceCreator::GetNbImportedPoints()
    {
    return s_getNbImportedPoints;
    }

double IScalableMeshSourceCreator::GetImportPointsDuration()
    {
    return s_getImportPointsDuration;
    }

double IScalableMeshSourceCreator::GetLastBalancingDuration()
    {
    return s_getLastBalancingDuration;
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
#endif
/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IScalableMeshSourceCreator::Impl::SyncWithSources(

    )
    {
    using namespace IDTMFile;

    HFCPtr<IndexType>          pDataIndex;

    HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment myMemMgr(100, 2000 * sizeof(PointType));

    CreateDataIndex(pDataIndex, myMemMgr, true);


    size_t previousDepth = pDataIndex->GetDepth();
    pDataIndex->SetIsTerrain(true);

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




   // std::list<IDTMFile::Extent3d64f> listRemoveExtent;
    std::list<DRange3d> listRemoveExtent;
    IDTMSourceCollection::const_iterator it = m_sources.Begin();

    for (IDTMSourceCollection::iterator it = m_sources.BeginEdit(); it != m_sources.EndEdit(); it++)
        {
        SourceImportConfig& conf = it->EditConfig();
        ScalableMeshData data = conf.GetReplacementSMData();
        if (data.GetUpToDateState() != UpToDateState::ADD)
            {
            s_inEditing = true;
            }

        if (data.GetUpToDateState() == UpToDateState::REMOVE || data.GetUpToDateState() == UpToDateState::MODIFY)
            {
            DRange3d sourceRange = data.GetExtentByLayer(0);
            /*IDTMFile::Extent3d64f removeExtent;
            removeExtent.xMin = sourceRange.low.x;
            removeExtent.xMax = sourceRange.high.x;
            removeExtent.yMin = sourceRange.low.y;
            removeExtent.yMax = sourceRange.high.y;
            removeExtent.zMin = sourceRange.low.z;
            removeExtent.zMax = sourceRange.high.z;*/
            DRange3d removeExtent = sourceRange;

            for (size_t i = 1; i < data.GetLayerCount(); i++)
                {
                DRange3d sourceRange = data.GetExtentByLayer((int)i);
                /*removeExtent.xMin = removeExtent.xMin > sourceRange.low.x ? sourceRange.low.x : removeExtent.xMin;
                removeExtent.xMax = removeExtent.xMax > sourceRange.high.x ? removeExtent.xMax : sourceRange.high.x;
                removeExtent.yMin = removeExtent.yMin > sourceRange.low.y ? sourceRange.low.y : removeExtent.yMin;
                removeExtent.yMax = removeExtent.yMax > sourceRange.high.y ? removeExtent.yMax : sourceRange.high.y;
                removeExtent.zMin = removeExtent.zMin > sourceRange.low.z ? sourceRange.low.z : removeExtent.zMin;
                removeExtent.zMax = removeExtent.zMax > sourceRange.high.z ? removeExtent.zMax : sourceRange.high.z;*/
                removeExtent.Extend(sourceRange);
                }

            if (data.GetUpToDateState() == UpToDateState::MODIFY)
                {
                data.SetUpToDateState(UpToDateState::ADD);
                conf.SetReplacementSMData(data);
                }

            listRemoveExtent.push_back(removeExtent);
            for (IDTMSourceCollection::iterator itAdd = m_sources.BeginEdit(); itAdd != m_sources.EndEdit(); itAdd++)
                {
                if (itAdd == it)
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



    typedef SMPointTaggedTileStore<int32_t,
        PointIndexExtentType>             TileStoreType;
    typedef SMPointTileStore<int32_t,
        PointIndexExtentType>             GenericTileStoreType;
    WString name = m_scmFileName;
    WString featureFilePath = name.append(L"_feature"); //temporary file, deleted after generation
    //IDTMFile::File::Ptr featureFilePtr = IDTMFile::File::Create(featureFilePath.c_str());
    SMSQLiteFilePtr sqliteFeatureFile = new SMSQLiteFile();
    sqliteFeatureFile->Create(featureFilePath);
    HFCPtr<GenericTileStoreType>  pFeatureTileStore = new SMSQLiteFeatureTileStore<PointIndexExtentType>(sqliteFeatureFile);//new TileStoreType(featureFilePtr, (SCM_COMPRESSION_DEFLATE == m_compressionType));
    //pFeatureTileStore->StoreMasterHeader(NULL, 0);
    pDataIndex->SetFeatureStore(pFeatureTileStore);
    auto pool = ScalableMeshMemoryPools<PointType>::Get()->GetFeaturePool();
    pDataIndex->SetFeaturePool(pool);
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
    if (BSISUCCESS != this->template BalanceDown<IndexType>(*pDataIndex, previousDepth))
        return BSIERROR;

#ifdef SCALABLE_MESH_ATP
    s_getLastBalancingDuration = ((double)clock() - startClock) / CLOCKS_PER_SEC / 60.0;

    startClock = clock();
#endif

    if (s_mesh)
        {
        // Mesh data             
        if (BSISUCCESS != IScalableMeshCreator::Impl::Mesh<IndexType>(*pDataIndex))
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
            if (BSISUCCESS != IScalableMeshCreator::Impl::Filter<IndexType>(*pDataIndex, -1))
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
            if (BSISUCCESS != IScalableMeshCreator::Impl::Filter<IndexType>(*pDataIndex, level))
                return BSIERROR;

#ifdef SCALABLE_MESH_ATP    
            s_getLastFilteringDuration += clock() - startClock;
            startClock = clock();
#endif
           // if (level == (int)depth)
                {
                if (BSISUCCESS != IScalableMeshCreator::Impl::Stitch<IndexType>(*pDataIndex, level, false))
                    return BSIERROR;
                }

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
    ImportRasterSourcesTo(pDataIndex);
    ApplyEditsFromSources(pDataIndex);

#ifdef ACTIVATE_TEXTURE_DUMP
    pDataIndex->DumpAllNodeTextures();
#endif
#ifdef INDEX_DUMPING_ACTIVATED
    if (s_dumpOctreeNodes)
        {
        //pointIndex.DumpOctTree("D:\\MyDoc\\Scalable Mesh Iteration 7\\Partial Update - Remove\\Log\\NodeAferCreation.xml", false);    
        //pDataIndex->DumpOctTree("C:\\Users\\Thomas.Butzbach\\Documents\\data_scalableMesh\\ATP\\NodeAferCreation.xml", false);
        pDataIndex->DumpOctTree("e:\\output\\scmesh\\NodeAferCreation.xml", false);
        }
#endif

    if (!pDataIndex->IsSingleFile() && s_save_grouped_store && !pDataIndex->IsEmpty())
        {
        // Make sure node headers are stored
        pDataIndex->Store();

        auto position = m_scmFileName.find_last_of(L".stm");
        auto filenameWithoutExtension = m_scmFileName.substr(0, position - 3);
        WString streamingFilePath = filenameWithoutExtension + L"_stream\\";
        auto groupedStreamingFilePath = filenameWithoutExtension + L"_grouped_stream\\";
        WString point_store_path = streamingFilePath + L"point_store\\";
        pDataIndex->SaveCloudReady(groupedStreamingFilePath, point_store_path);
        }
//    auto& store = pDataIndex->GetClipStore();
    pDataIndex = 0;
    //store->Close();
    if (s_rasterMemPool != nullptr) delete s_rasterMemPool;
    //featureFilePtr->Close();
   // featureFilePtr = 0;
    sqliteFeatureFile->Close();
    sqliteFeatureFile = 0;
    char* outPath = new char[featureFilePath.GetMaxLocaleCharBytes()];
    std::remove(featureFilePath.ConvertToLocaleChars(outPath));
    delete[] outPath;
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

int IScalableMeshSourceCreator::Impl::ApplyEditsFromSources(HFCPtr<IndexType>& pIndex)
    {
    const GCS& fileGCS = GetGCS();
    Sink* sinkP = new ScalableMeshNonDestructiveEditStorage<PointType>(*pIndex, fileGCS);
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

    status = TraverseSourceCollectionEditsOnly(sourcesImporter,
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

int IScalableMeshSourceCreator::Impl::ImportRasterSourcesTo(HFCPtr<IndexType>& pIndex)
    {
    const GCS& fileGCS = GetGCS();
    const ScalableMeshData& targetScalableMeshData = ScalableMeshData::GetNull();

    HFCPtr<HVEClipShape>  resultingClipShapePtr(new HVEClipShape(new HGF2DCoordSys()));

    int status = BSISUCCESS;
    bvector<IDTMSource*> filteredSources;
    status = TraverseSourceCollectionRasters(filteredSources,
                                               m_sources,
                                               resultingClipShapePtr,
                                               fileGCS,
                                               targetScalableMeshData);
    s_rasterMemPool = new HPMPool(30000, HPMPool::None);
    auto cluster = new HGFHMRStdWorldCluster();
    HFCPtr<HIMMosaic> mosaicP = new HIMMosaic(HFCPtr<HGF2DCoordSys>(cluster->GetWorldReference(HGF2DWorld_HMRWORLD).GetPtr()));
    HIMMosaic::RasterList rasterList;
    for (auto& source : filteredSources)
        {
        //const ILocalFileMoniker* moniker(dynamic_cast<const ILocalFileMoniker*>(&source->GetPath()));
        WString path = WString(L"file://") + source->GetPath();
        HFCPtr<HGF2DCoordSys>  pLogicalCoordSys;
        HFCPtr<HRSObjectStore> pObjectStore;
        HFCPtr<HRFRasterFile>  pRasterFile;
        HFCPtr<HRARaster>      pRaster;
       // HFCPtr<HRAOnDemandRaster> pOnDemandRaster;
        pRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile(HFCURL::Instanciate(path), TRUE);
        pLogicalCoordSys = cluster->GetWorldReference(pRasterFile->GetPageWorldIdentificator(0));
        pObjectStore = new HRSObjectStore(s_rasterMemPool,
                                          pRasterFile,
                                          0,
                                          pLogicalCoordSys);

        // Get the raster from the store
        pRaster = pObjectStore->LoadRaster();
       // pOnDemandRaster = new HRAOnDemandRaster(rasterMemPool, pRaster->IsOpaque(), pRaster->GetEffectiveShape(), ,new HPSWorldCluster(), HGF2DWorld_HMRWORLD, , pRaster->HasLookAhead(), false, false);
        HASSERT(pRaster != NULL);
        //NEEDS_WORK_SM: do not do this if raster does not intersect sm extent
        rasterList.push_back(pRaster.GetPtr());
        pRaster = 0;
        }
    mosaicP->Add(rasterList);
    rasterList.clear();
    pIndex->TextureFromRaster(mosaicP.GetPtr());
    delete cluster;
    return status;
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
    for (std::list<DRange3d>::const_iterator it = listRemoveExtent.begin(); it != listRemoveExtent.end(); it++)
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
StatusInt IScalableMeshSourceCreator::Impl::ImportSourcesTo(Sink* sinkP)
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


int IScalableMeshSourceCreator::Impl::TraverseSource(SourcesImporter&                                         importer,
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

int IScalableMeshSourceCreator::Impl::TraverseSourceCollection(SourcesImporter&                                            importer,
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