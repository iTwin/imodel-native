/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourceImporter.cpp $
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
#include <ScalableMesh/IScalableMeshSourceImporter.h>
#include <ScalableMesh/IScalableMeshSourceVisitor.h>


#include "Plugins/ScalableMeshClipMaskFilterFactory.h"
#include "Plugins/ScalableMeshIDTMFileTraits.h"

#include "ScalableMeshStorage.h"
#include <ScalableMesh/IScalableMeshStream.h>

#include <ScalableMesh/IScalableMeshPolicy.h>

#include "ScalableMeshSources.h"

#include "InternalUtilityFunctions.h"
#include <STMInternal/GeoCoords/WKTUtils.h>

#include "ScalableMeshSourceImporter.h"

#include <DgnPlatform\Tools\ConfigurationManager.h>



#define SCALABLE_MESH_TIMINGS


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
/*        INTERFACE SECTION - BEGIN                             */
/*==================================================================*/
StatusInt IScalableMeshSourceImporterStorage::AddSource(Time::TimeType     sourceLastModifiedTime, 
                                                 const HPU::Packet& serializedSourcePacket, 
                                                 const HPU::Packet& serializedContentConfigPacket, 
                                                 const HPU::Packet& serializedImportSequencePacket, 
                                                 GroupId            groupId)
    {
    return _AddSource(sourceLastModifiedTime, 
                      serializedSourcePacket, 
                      serializedContentConfigPacket, 
                      serializedImportSequencePacket, 
                      groupId);
    }

 bool IScalableMeshSourceImporterStorage::ReadFirstSource(StatusInt* status)
    {
    return _ReadFirstSource(status);
    }

StatusInt IScalableMeshSourceImporterStorage::GetSourceInfo(Time::TimeType& sourceLastModifiedTime, 
                                                     HPU::Packet&    serializedSourcePacket, 
                                                     HPU::Packet&    serializedContentConfigPacket, 
                                                     HPU::Packet&    serializedImportSequencePacket, 
                                                     GroupId&        groupId)
    {
    return _GetSourceInfo(sourceLastModifiedTime, 
                          serializedSourcePacket, 
                          serializedContentConfigPacket, 
                          serializedImportSequencePacket, 
                          groupId);
    }

bool IScalableMeshSourceImporterStorage::ReadNextSource(StatusInt* status)
    {
    return _ReadNextSource(status);
    }

StatusInt IScalableMeshSourceImporterStorage::ReadGcs(WString& wkt)
    {
    return _ReadGcs(wkt);
    }

StatusInt IScalableMeshSourceImporterStorage::StoreGcs(const WString& wkt)
    {
    return _StoreGcs(wkt);
    }


//NEEDS_WORK_SM_IMPORTER : Should find a better way to do this.
void RegisterDelayedImporters();

IScalableMeshSourceImporterPtr IScalableMeshSourceImporter::Create ()    
    {   
    RegisterDelayedImporters();

    IScalableMeshSourceImporterPtr pSourceImporter = new IScalableMeshSourceImporter(new Impl());
        
    return pSourceImporter;
    }

IScalableMeshSourceImporterPtr IScalableMeshSourceImporter::Create (IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr, 
                                                      StatusInt&                      status)
    {
    IScalableMeshSourceImporterPtr sourceImporterPtr = IScalableMeshSourceImporter::Create ();

    status = sourceImporterPtr->m_implP->LoadFromStorage(sourceImporterStoragePtr);

    return sourceImporterPtr;     
    }

IScalableMeshSourceImporter::IScalableMeshSourceImporter (Impl* implP)
    :   m_implP(implP)
    {
    }


IScalableMeshSourceImporter::~IScalableMeshSourceImporter ()
    {
    }

bool IScalableMeshSourceImporter::AreAllSourcesReachable () const
    {
    // Reachable only if all sources are reachable
    return m_implP->m_sources.End() == std::find_if(m_implP->m_sources.Begin(), m_implP->m_sources.End(), not1(mem_fun_ref(&IDTMSource::IsReachable)));
    }

StatusInt IScalableMeshSourceImporter::Import ()
    {
    return m_implP->Import();
    }

const Bentley::GeoCoordinates::BaseGCSPtr& IScalableMeshSourceImporter::GetBaseGCS () const
    {
    return GetAdvancedGCS().GetGeoRef().GetBasePtr();
    }

const GeoCoords::GCS& IScalableMeshSourceImporter::Impl::GetGCS () const
    {    
    return m_gcs;
    }

const GeoCoords::GCS& IScalableMeshSourceImporter::GetAdvancedGCS() const
    {
    return m_implP->GetGCS();
    }

const GeoCoords::GCS& IScalableMeshSourceImporter::GetGCS () const
    {
    return m_implP->GetGCS();
    }

void IScalableMeshSourceImporter::SetSourcesDirty ()
    {
    // Make sure that sources are not seen as up to date anymore
    m_implP->m_sourcesDirty = true;   
    }

bool IScalableMeshSourceImporter::HasDirtySources () const
    {
    return m_implP->m_sourcesDirty;
    }


StatusInt IScalableMeshSourceImporter::Store(IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr)
    {
    return m_implP->Store(sourceImporterStoragePtr);
    }


StatusInt IScalableMeshSourceImporter::SetCompression(ScalableMeshCompressionType compressionType)
    {
    m_implP->m_compressionType = compressionType;
    return SUCCESS;
    }

StatusInt IScalableMeshSourceImporter::SetBaseGCS (const Bentley::GeoCoordinates::BaseGCSPtr& gcsPtr)
    {
    return SetGCS(GetGCSFactory().Create(gcsPtr));
    }

StatusInt IScalableMeshSourceImporter::SetGCS(const GeoCoords::GCS& gcs)
    {   
    // Do not permit setting null GCS. Use default when it happens.
    m_implP->m_gcs = (gcs.IsNull()) ? GetDefaultGCS() : gcs;
    m_implP->m_gcsDirty = true;

    return 0;
    }

StatusInt IScalableMeshSourceImporter::SetFeatureCallback(WriteFeatureCallbackFP writeFeatureCallbackFP)
    {     
    return m_implP->SetFeatureCallback(writeFeatureCallbackFP);    
    }

StatusInt IScalableMeshSourceImporter::SetPointsCallback(WritePointsCallbackFP writePointsCallbackFP)
    {    
    return m_implP->SetPointsCallback(writePointsCallbackFP);
    }

const IDTMSourceCollection& IScalableMeshSourceImporter::GetSources () const
    {
    return m_implP->m_sources;
    }

IDTMSourceCollection& IScalableMeshSourceImporter::EditSources ()
    {
    return m_implP->m_sources;
    }

/*==================================================================*/
/*        INTERFACE SECTION - END                                   */
/*==================================================================*/

/*----------------------------------------------------------------------------+
|ScalableMeshCreator class
+----------------------------------------------------------------------------*/
IScalableMeshSourceImporter::Impl::Impl()
:       m_gcs(GetDefaultGCS()),                
        m_sourcesDirty(false),
        m_gcsDirty(false),        
        m_compressionType(SCM_COMPRESSION_DEFLATE),
        m_workingLayer(DEFAULT_WORKING_LAYER),
        m_writePointsCallbackFP(0),
        m_writeFeatureCallbackFP(0)
    {
    m_sources.RegisterEditListener(*this);
    }

IScalableMeshSourceImporter::Impl::~Impl()
    {    
    m_sources.UnregisterEditListener(*this);
    }

int IScalableMeshSourceImporter::Impl::Import()
    {
    int status = ERROR;

    if ((m_writePointsCallbackFP != 0) || (m_writeFeatureCallbackFP != 0))
        {
        using namespace IDTMFile;
        
        //MS : Some cleanup needs to be done here.
        try
            {        

            // Sync only when there are sources with which to sync
            // TR #325614: This special condition provides us with a way of efficiently detecting if STM is empty
            //             as indexes won't be initialized.
            if (0 < m_sources.GetCount() && BSISUCCESS == SyncWithSources())
                {
                status = SUCCESS;
                }                            
            }
        catch (...)
            {            
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//NEEDS_WORK_SM : To be removed
static bool s_dumpOctreeNodes = false;
static bool s_mesh = false;
static bool s_filter = false;
static bool s_stitchFilteredByLevel = false;
static bool s_stitch = false;
static bool s_validateIs3dDataState = true;

StatusInt IScalableMeshSourceImporter::Impl::SyncWithSources () 
    {         

#if 0 //NEEDS_WORK_SM_IMPORTER : ?
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

            for(int i = 1; i<data.GetLayerCount(); i++)
                {
                DRange3d sourceRange = data.GetExtentByLayer(i);
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
                for(int i = 1; i<dataAdd.GetLayerCount(); i++)
                    {
                    DRange3d targetRangeTemp = dataAdd.GetExtentByLayer(i);
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
#endif



    // Remove sources which have been removed or modified
#if 0 //NEEDS_WORK_SM_IMPORTER : ?
    if (BSISUCCESS != RemoveSourcesFrom<PointIndexType>(pointIndex, listRemoveExtent))
        return BSIERROR;          
#endif

    // Import sources
    if (BSISUCCESS != ImportSourcesTo(new GenericStorage<DPoint3d>(m_writePointsCallbackFP, m_writeFeatureCallbackFP, GetGCS())))
        return BSIERROR;


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

    return BSISUCCESS;
    }

StatusInt IScalableMeshSourceImporter::Impl::SetFeatureCallback(WriteFeatureCallbackFP writeFeatureCallbackFP)
    {     
    m_writeFeatureCallbackFP = writeFeatureCallbackFP;
    return SUCCESS;
    }

StatusInt IScalableMeshSourceImporter::Impl::SetPointsCallback(WritePointsCallbackFP writePointsCallbackFP)
    {
    m_writePointsCallbackFP = writePointsCallbackFP;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
#if 0 //NEEDS_WORK_SM_IMPORTER
template <typename PointIndex>       
StatusInt IScalableMeshSourceImporter::Impl::RemoveSourcesFrom(PointIndex& pointIndex, list<IDTMFile::Extent3d64f> listRemoveExtent) const
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
#endif
/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IScalableMeshSourceImporter::Impl::ImportSourcesTo (Sink* sinkP)
    {   
    SinkPtr sinkPtr(sinkP);
    //NEEDS_WORK_SM_IMPORTER : GenericSourceRef?        
    LocalFileSourceRef sinkSourceRef(L"");

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
#if 0
IDTMFile::File::Ptr IScalableMeshSourceImporter::Impl::SetupFileForCreation ()
    {
    using namespace IDTMFile;

    
    //NEEDS_WORK_SM_IMPORTER : Where to put those?
    bool bAllRemoved = true;
    bool bAllAdded = true;
    for (IDTMSourceCollection::const_iterator it = m_sources.Begin(); it != m_sources.End(); it++)
        {
        SourceImportConfig conf = it->GetConfig();
        ScalableMeshData data = conf.GetReplacementSMData();
        if(data.GetUpToDateState() != UpToDateState::REMOVE)
            bAllRemoved = false;
        if(data.GetUpToDateState() != UpToDateState::ADD)
            bAllAdded = false;
        }


    // Ensure GCS and sources are save to the file.
    m_gcsDirty = true;
    m_sourcesDirty = true;

    return filePtr;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
#if 0
int IScalableMeshSourceImporter::Impl::UpdateLastModified()
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

            //NEEDS_WORK_SM_IMPORTER : Where to put those?
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
#endif


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IScalableMeshSourceImporter::Impl::LoadSources (IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr)
    {
        /*
    using namespace IDTMFile;

    if (!file.GetRootDir()->HasSourcesDir())
        return BSISUCCESS; // No sources were added to the STM.

    const IDTMFile::SourcesDir* sourceDirPtr = file.GetRootDir()->GetSourcesDir();
    if (0 == sourceDirPtr)
        return BSIERROR; // Could not load existing sources dir
*/
    bool success = true;

    //NEEDS_WORK_SM_IMPORTER : ??
    DocumentEnv dummyDocumentEnv(L"");

    success &= Bentley::ScalableMesh::LoadSources(m_sources, sourceImporterStoragePtr, dummyDocumentEnv);
    /*
    m_lastSourcesModificationCheckTime = CreateTimeFrom(sourceDirPtr->GetLastModifiedCheckTime());
    m_lastSourcesModificationTime = CreateTimeFrom(sourceDirPtr->GetLastModifiedTime());
    m_lastSyncTime = CreateTimeFrom(sourceDirPtr->GetLastSyncTime());
    */
    return (success) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IScalableMeshSourceImporter::Impl::SaveSources (IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr)
    {    
    if (!m_sourcesDirty)
        return BSISUCCESS;

    /*
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
        */

    bool success = true;

    //NEEDS_WORK_SM_IMPORTER : ??
    DocumentEnv dummyDoc(L"");

    success &= Bentley::ScalableMesh::SaveSources(m_sources, sourceImporterStoragePtr, dummyDoc);
    /*NEEDS_WORK_SM_IMPORTER : TODO
    success &= sourceDirPtr->SetLastModifiedCheckTime(GetCTimeFor(m_lastSourcesModificationCheckTime));
    success &= sourceDirPtr->SetLastModifiedTime(GetCTimeFor(m_lastSourcesModificationTime));
    success &= sourceDirPtr->SetLastSyncTime(GetCTimeFor(m_lastSyncTime));
    */

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
StatusInt IScalableMeshSourceImporter::Impl::LoadGCS (IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr)    
    {    
    WString wktStr;

    StatusInt status = sourceImporterStoragePtr->ReadGcs(wktStr);

    assert(status == SUCCESS);   

    if (wktStr.size() == 0)
        {
        // Use default GCS when no WKT is found
        m_gcs = GetDefaultGCS();
        return BSISUCCESS;
        }

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
StatusInt IScalableMeshSourceImporter::Impl::SaveGCS(IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr)
    {
    //using namespace IDTMFile;

    if (!m_gcsDirty)
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
        wchar_t wktFlavor[2] = {(wchar_t)IDTMFile::WktFlavor_Autodesk, L'\0'};

        extendedWktStr += WString(wktFlavor);            
        }      

    if (sourceImporterStoragePtr->StoreGcs(extendedWktStr) != SUCCESS)
        return BSIERROR;
    
    m_gcsDirty = false;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IScalableMeshSourceImporter::Impl::LoadFromStorage  (IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr)
    {        
    if (BSISUCCESS != LoadSources(sourceImporterStoragePtr) ||
        BSISUCCESS != LoadGCS(sourceImporterStoragePtr))
        return BSIERROR;

    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IScalableMeshSourceImporter::Impl::Store(IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr)
    {            
    if (!m_sourcesDirty && !m_gcsDirty)
        return BSISUCCESS; // Nothing to save    

    //NEEDS_WORK_SM_IMPORTER Why no checking correct dirytiness flag?
    if (BSISUCCESS != SaveSources(sourceImporterStoragePtr) ||
        BSISUCCESS != SaveGCS(sourceImporterStoragePtr))
        return BSIERROR;    
      
    return BSISUCCESS; 
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IScalableMeshSourceImporter::Impl::_NotifyOfPublicEdit ()
    {
    // Make sure that sources are not seen as up to date anymore
    m_sourcesDirty = true;   
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IScalableMeshSourceImporter::Impl::_NotifyOfLastEditUpdate (Time updatedLastEditTime)
    {
    //m_lastSourcesModificationTime = (std::max)(m_lastSourcesModificationTime, updatedLastEditTime);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef CreateSourceRefFromIDTMSource (const IDTMSource& source)
    {
    struct Visitor : IDTMSourceVisitor
        {
        auto_ptr<SourceRef>         m_sourceRefP;

        explicit                    Visitor            ()            
            {
            }

        virtual void                _Visit             (const IDTMLocalFileSource&  source) override
            {
            StatusInt status = BSISUCCESS;
            const WChar* path = source.GetPath(status);
            if (BSISUCCESS != status)
                throw SourceNotFoundException();
           
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

    Visitor visitor;
    source.Accept(visitor);

    if (0 == visitor.m_sourceRefP.get())
        throw CustomException(L"Unable to create source Ref from IDTMSource!");

    visitor.m_sourceRefP->SetDtmSource(source.Clone());

    return *visitor.m_sourceRefP;
    }

int IScalableMeshSourceImporter::Impl::ImportClipMaskSource  (const IDTMSource&       dataSource,
                                                       const ClipShapeStoragePtr&  clipShapeStoragePtr) const
    {
    try
        {
        static const SourceFactory SOURCE_FACTORY(GetSourceFactory());
        static const ImporterFactory IMPORTER_FACTORY(GetImporterFactory());


        // Create sourceRef
        SourceRef srcRef = CreateSourceRefFromIDTMSource(dataSource);

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


int IScalableMeshSourceImporter::Impl::TraverseSource  (SourcesImporter&                                         importer,
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

        SourceRef sourceRef(CreateSourceRefFromIDTMSource(dataSource));


        importer.AddSource(sourceRef, sourceConfig, importConfig, importSequence, srcImportConfig/*, vecRange*/);
        }
    catch (...)
        {
        status = BSIERROR;
        }

    return status;
    }

int IScalableMeshSourceImporter::Impl::TraverseSourceCollection  (SourcesImporter&                                            importer,
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

void IScalableMeshSourceImporter::Impl::ShowMessageBoxWithTimes(double meshingDuration, double filteringDuration, double stitchingDuration)
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
