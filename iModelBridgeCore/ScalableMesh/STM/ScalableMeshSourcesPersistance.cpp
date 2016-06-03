/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourcesPersistance.cpp $
|    $RCSfile: ScalableMeshSourcesPersistance.cpp,v $
|   $Revision: 1.19 $
|       $Date: 2011/11/21 17:01:01 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
#include "ScalableMeshSourcesPersistance.h"

#include "ScalableMeshSources.h"


#include <ScalableMesh/IScalableMeshSourceImportConfig.h>
#include <ScalableMesh\IScalableMeshSourceImporter.h>



#include "ScalableMeshSourcePersistance.h"
#include "ScalableMeshImportSequencePersistance.h"
#include "ScalableMeshContentConfigPersistance.h"

using namespace std;
using namespace IDTMFile;
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
#pragma warning(disable:4702)

/*
 * Save privates
 */
namespace { // Unamed namespace

struct FormatVersions
    {
    uint32_t                            serializedSource;
    uint32_t                            contentConfig;
    uint32_t                            importSequence;
    uint32_t                            importConfig;
    };


/*
 * Driver current format versions
 */
const FormatVersions                CURRENT_FORMAT_VERSIONS = 
    {
    SourceSerializer::FORMAT_VERSION, // Serialized sources
    ContentConfigSerializer::FORMAT_VERSION, // Content config
    ImportSequenceSerializer::FORMAT_VERSION, // Import sequence
    0, // Import config (no driver yet)
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class SourcesSaver
    {
    typedef basic_stringstream<byte> 
                                bstringstream;
    typedef basic_string<byte>  bstring;
    
    IScalableMeshSourceImporterStorage::GroupId m_nextGroupId;

    bstringstream               m_serializationStream;

    SourceDataSQLite            m_sourceData;


    bstring                     m_serializedSource;
    HPU::Packet                 m_serializedSourcePacket;

    bstring                     m_serializedContentConfig;
    HPU::Packet                 m_serializedContentConfigPacket;

    bstring                     m_serializedImportSequence;
    HPU::Packet                 m_serializedImportSequencePacket;

    const DocumentEnv&          m_sourceEnv;

    bool                        Save                                   (const IDTMSource&               source,
                                                                        SourceDataSQLite&              sourcesData);

    bool                        Save                                   (const IDTMSourceCollection&     sources,
                                                                        SourcesDataSQLite&              sourcesData);


    bool                        Save                                   (const IDTMSource&                source,
                                                                        IScalableMeshSourceImporterStoragePtr&  sourceImporterStoragePtr,
                                                                        IScalableMeshSourceImporterStorage::GroupId groupId);

    bool                        Save                                   (const IDTMSourceCollection&      sources, 
                                                                        IScalableMeshSourceImporterStoragePtr&         sourceStorage,
                                                                        IScalableMeshSourceImporterStorage::GroupId groupId = NO_GROUP_ID);
    
public:
    explicit                    SourcesSaver                           (const DocumentEnv&              sourceEnv);

    bool                        SaveRoot                               (const IDTMSourceCollection&     sources,
                                                                        SourcesDataSQLite&              sourcesData);

    bool                        SaveRoot                               (const IDTMSourceCollection&     sources, 
                                                                        IScalableMeshSourceImporterStoragePtr&        sourcesStorage);
    };


/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesSaver::SourcesSaver(const DocumentEnv&  sourceEnv)
    : m_serializationStream(bstringstream::in | bstringstream::out | bstringstream::binary | bstringstream::app),
//    m_stream(m_serializationStream),
    m_sourceData(SourceDataSQLite::GetNull()),
    m_sourceEnv(sourceEnv),
    m_nextGroupId(0)
{
}


/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourcesSaver::SaveRoot(const IDTMSourceCollection&     sources,
                            SourcesDataSQLite&              sourcesData)
{
    /*sourcesData.SetSerializedSourceFormatVersion(CURRENT_FORMAT_VERSIONS.serializedSource);
    sourcesData.SetContentConfigFormatVersion(CURRENT_FORMAT_VERSIONS.contentConfig);
    sourcesData.SetImportSequenceFormatVersion(CURRENT_FORMAT_VERSIONS.importSequence);
    sourcesData.SetImportConfigFormatVersion(CURRENT_FORMAT_VERSIONS.importConfig);*/

    return Save(sources, sourcesData);
}



bool SourcesSaver::SaveRoot    (const IDTMSourceCollection&     sources, 
                                IScalableMeshSourceImporterStoragePtr&        sourcesStorage)
    {
        /*NEEDS_WORK_SM_IMPORTER : Do we want this?
    if (!sourcesDir.SetSerializedSourceFormatVersion(CURRENT_FORMAT_VERSIONS.serializedSource) ||
        !sourcesDir.SetContentConfigFormatVersion(CURRENT_FORMAT_VERSIONS.contentConfig) ||
        !sourcesDir.SetImportSequenceFormatVersion(CURRENT_FORMAT_VERSIONS.importSequence) ||
        !sourcesDir.SetImportConfigFormatVersion(CURRENT_FORMAT_VERSIONS.importConfig))
        return false;
        */

    return Save(sources, sourcesStorage);
    }

//This saves the appropriate metadata for differently-reachable sources
template <class SourceType> void SaveSourceMetadata(const SourceType& source, SourceDataSQLite& sourceData)   
    {

    sourceData.SetDTMSourceID(DTMSourceId::DTM_SOURCE_ID_LOCAL_FILE_V0);
    }

template <> void SaveSourceMetadata<IDTMDgnLevelSource>(const IDTMDgnLevelSource& source, SourceDataSQLite& sourceData)
    {
    sourceData.SetDTMSourceID(DTMSourceId::DTM_SOURCE_ID_DGN_LEVEL_V1);
    sourceData.SetLevelID(source.GetLevelID());
    sourceData.SetLevelName(source.GetLevelName());
    }

template <>void  SaveSourceMetadata<IDTMDgnModelSource>(const IDTMDgnModelSource& source, SourceDataSQLite& sourceData)
    {
    sourceData.SetModelID(source.GetModelID());
    sourceData.SetModelName(source.GetModelName());
    }

template <>void SaveSourceMetadata<IDTMDgnReferenceSource>(const IDTMDgnReferenceSource& source, SourceDataSQLite& sourceData)
    {
    sourceData.SetRootToRefPersistentPath(source.GetRootToRefPersistentPath());
    sourceData.SetReferenceName(source.GetReferenceName());
    sourceData.SetReferenceModelName(source.GetReferenceModelName());
    }

template <> void SaveSourceMetadata<IDTMDgnReferenceLevelSource>(const IDTMDgnReferenceLevelSource& source, SourceDataSQLite& sourceData)
    {
    sourceData.SetDTMSourceID(DTMSourceId::DTM_SOURCE_ID_DGN_REFERENCE_LEVEL_V1);
    sourceData.SetLevelID(source.GetLevelID());
    sourceData.SetLevelName(source.GetLevelName());
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourcesSaver::Save(const IDTMSource& source,
                        SourceDataSQLite& sourceData)
{
    bool success = true;

    const Time& lastModified = source.GetLastModified();


    // Serializing source
    /*{
        m_serializationStream.str(bstring());
        assert(m_serializationStream.good());

        static const SourceSerializer SERIALIZER;
        if (!SERIALIZER.Serialize(source, m_sourceEnv, m_sourceData))
        {
            assert(!"Log warning! Unable to serialize source!");
            return false; // Skip the source.
        }

        m_serializedSource = m_serializationStream.str();
        m_serializedSourcePacket.Wrap(&m_serializedSource[0], m_serializedSource.size());
    }*/
    //common info
    m_sourceData.SetSourceType(source.GetSourceType());
    m_sourceData.SetMonikerString(source.GetPath());

    SaveSourceMetadata(source, m_sourceData);

    const IDTMDgnLevelSource* dgnLevelSource = dynamic_cast<const IDTMDgnLevelSource*>(&source);
    if (nullptr != dgnLevelSource)
        SaveSourceMetadata(*dgnLevelSource, m_sourceData);

    const IDTMDgnModelSource* dgnModelSource = dynamic_cast<const IDTMDgnModelSource*>(&source);
    if (nullptr != dgnModelSource)
        SaveSourceMetadata(*dgnModelSource, m_sourceData);

    const IDTMDgnReferenceSource* dgnRefSource = dynamic_cast<const IDTMDgnReferenceSource*>(&source);
    if (nullptr != dgnRefSource)
        SaveSourceMetadata(*dgnRefSource, m_sourceData);

    const IDTMDgnReferenceLevelSource* dgnRefLSource = dynamic_cast<const IDTMDgnReferenceLevelSource*>(&source);
    if (nullptr != dgnRefLSource)
        SaveSourceMetadata(*dgnRefLSource, m_sourceData);


    // Serializing content config
    {
        m_serializationStream.str(bstring());

        static const ContentConfigSerializer SERIALIZER;
        if (SERIALIZER.Serialize(source.GetConfig().GetContentConfig(), m_sourceData))
        {
            m_serializedContentConfig = m_serializationStream.str();
            m_serializedContentConfigPacket.Wrap(&m_serializedContentConfig[0], m_serializedContentConfig.size());
        }
        else
        {
            assert(!"Log warning! Unable to serialize config!");
            success = false;
            m_serializedContentConfigPacket.Clear();
        }
    }

    // Serializing import sequence
    {
        m_serializationStream.str(bstring());

        ImportSequenceSerializer SERIALIZER;
        if (SERIALIZER.Serialize(source.GetConfig().GetSequence(), m_sourceData))

        {
            m_serializedImportSequence = m_serializationStream.str();
            m_serializedImportSequencePacket.Wrap(&m_serializedImportSequence[0], m_serializedImportSequence.size());
        }
        else
        {
            assert(!"Log warning! Unable to import sequence!");
            success = false;
            m_serializedImportSequencePacket.Clear();
        }
    }

    sourceData = m_sourceData;
    sourceData.SetTimeLastModified(GetCTimeFor(lastModified));

    return success;
    }

bool SourcesSaver::Save(const IDTMSourceCollection&     sources,
                        SourcesDataSQLite& sourcesData)
{
    // save group ??? needs work, but later...
    
    bool success = true;
    //int sourceID = 0;
    //int subGroupID = 0;
    for (IDTMSourceCollection::const_iterator sourceIt = sources.Begin(), sourceEnd = sources.End();
    sourceIt != sourceEnd;
        ++sourceIt)
    {
        const IDTMSource& dataSource = *sourceIt;
        if (dataSource.GetSourceType() == DTM_SOURCE_DATA_IMAGE) continue;
        if (0 != dynamic_cast<const IDTMSourceGroup*>(&dataSource))
        {
            const IDTMSourceGroup& rSubCollection = dynamic_cast<const IDTMSourceGroup&>(dataSource);

            //SourcesDataSQLite subSourcesGroupData;

            //SourcesDataSQLite subSourcesGroupData = sourcesData.AddGroupNode();
            /*if (nullptr == subSourcesGroupData)
                throw runtime_error("Could not add group node dir");*/

            //subSourceData.SetLastModifiedTime(GetCTimeFor(rSubCollection.GetLastModified()));
            sourcesData.SetIsGroup(true);
            success &= Save(rSubCollection.GetImpl().GetSources(), sourcesData);
            sourcesData.IncreaseCurrentGroupID();
            sourcesData.SetIsGroup(false);
            //sourcesData.DecreaseCurrentGroupID();
            //subSourcesGroupData.SetGroupID(subGroupID);
            //sourcesData.AddGroupNode(subSourcesGroupData);
            //subGroupID++;
        }
        else
        {
            
            /*if (nullptr == subSourceData)
                throw runtime_error("Could not add sources node dir");*/

            /*sourceData = sourcesData->GetSources();
            if (nullptr == sourceData)
                throw runtime_error("Could not get source sequence dir");*/
            SourceDataSQLite subSourceData = SourceDataSQLite::GetNull();
            success &= Save(dataSource, subSourceData);
            subSourceData.SetSourceID(sourcesData.GetCurrentSourceID());
            subSourceData.SetGroupID(sourcesData.IsGroup() ? sourcesData.GetCurrentGroupID() : NO_GROUP_ID);
            sourcesData.IncreaseCurrentSourceID();
            sourcesData.AddSourcesNode(subSourceData);
        }
    }
    /*
    SourceSequenceDir* sourceSequenceDirPtr = 0;

    for (IDTMSourceCollection::const_iterator sourceIt = sources.Begin(), sourceEnd = sources.End();
    sourceIt != sourceEnd;
        ++sourceIt)
    {
        const IDTMSource& dataSource = *sourceIt;
        if (dataSource.GetSourceType() == DTM_SOURCE_DATA_IMAGE) continue;
        if (0 != dynamic_cast<const IDTMSourceGroup*>(&dataSource))
        {
            const IDTMSourceGroup& rSubCollection = dynamic_cast<const IDTMSourceGroup&>(dataSource);

            sourceSequenceDirPtr = 0;
            SourceNodeDir* subNodePtr = sourceNodeDir.AddGroupNode();
            if (0 == subNodePtr)
                throw runtime_error("Could not add group node dir");

            subNodePtr->SetLastModifiedTime(GetCTimeFor(rSubCollection.GetLastModified()));

            success &= Save(rSubCollection.GetImpl().GetSources(), *subNodePtr);
        }
        else
        {
            if (0 == sourceSequenceDirPtr)
            {
                SourceNodeDir* subNodePtr = sourceNodeDir.AddSourcesNode();
                if (0 == subNodePtr)
                    throw runtime_error("Could not add sources node dir");

                sourceSequenceDirPtr = subNodePtr->GetSources();
                if (0 == sourceSequenceDirPtr)
                    throw runtime_error("Could not get source sequence dir");

            }

            success &= Save(dataSource, *sourceSequenceDirPtr);
        }
    }
    */
    return success;
}



bool SourcesSaver::Save (const IDTMSource& source, IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr, IScalableMeshSourceImporterStorage::GroupId groupId)
    {
    bool success = true;

    const Time& lastModified = source.GetLastModified();


    // Serializing source
        {
        m_serializationStream.str(bstring());
        assert(m_serializationStream.good());

        static const SourceSerializer SERIALIZER;

        if (!SERIALIZER.Serialize(source, m_sourceEnv, m_sourceData))

            {
            assert(!"Log warning! Unable to serialize source!");
            return false; // Skip the source.
            }

        m_serializedSource = m_serializationStream.str();
        m_serializedSourcePacket.Wrap(&m_serializedSource[0], m_serializedSource.size());
        }
        
    // Serializing content config
        {
        m_serializationStream.str(bstring());

        static const ContentConfigSerializer SERIALIZER;

        if (SERIALIZER.Serialize(source.GetConfig().GetContentConfig(), m_sourceData))

            {
            m_serializedContentConfig = m_serializationStream.str();
            m_serializedContentConfigPacket.Wrap(&m_serializedContentConfig[0], m_serializedContentConfig.size());
            }
        else
            {
            assert(!"Log warning! Unable to serialize config!");
            success = false;
            m_serializedContentConfigPacket.Clear();
            }
        }

    // Serializing import sequence
        {
        m_serializationStream.str(bstring());

        ImportSequenceSerializer SERIALIZER;

        if (SERIALIZER.Serialize(source.GetConfig().GetSequence(), m_sourceData))

            {
            m_serializedImportSequence = m_serializationStream.str();
            m_serializedImportSequencePacket.Wrap(&m_serializedImportSequence[0], m_serializedImportSequence.size());
            }
        else
            {
            assert(!"Log warning! Unable to import sequence!");
            success = false;
            m_serializedImportSequencePacket.Clear();
            }
        }

    sourceImporterStoragePtr->AddSource(GetCTimeFor(lastModified), m_serializedSourcePacket, m_serializedContentConfigPacket, m_serializedImportSequencePacket, groupId);
    return success;
    }


bool SourcesSaver::Save    (const IDTMSourceCollection&      sources, 
                            IScalableMeshSourceImporterStoragePtr&         sourceStorage,
                            IScalableMeshSourceImporterStorage::GroupId groupId)
    {
    bool success = true;
   
    for (IDTMSourceCollection::const_iterator sourceIt = sources.Begin(), sourceEnd = sources.End(); 
         sourceIt != sourceEnd; 
         ++sourceIt)
        {
        const IDTMSource& dataSource = *sourceIt;
        
        if (0 != dynamic_cast<const IDTMSourceGroup*>(&dataSource))
            {
            const IDTMSourceGroup& rSubCollection = dynamic_cast<const IDTMSourceGroup&>(dataSource);                        
                                    
            success &= Save(rSubCollection.GetImpl().GetSources(), sourceStorage, m_nextGroupId);

            m_nextGroupId++;
            }
        else
            {            
            success &= Save(dataSource, sourceStorage, NO_GROUP_ID);
            }
        }

    return success;
    }
} // END Unamed namespace



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/

bool SaveSources(const IDTMSourceCollection&     sources,
    SourcesDataSQLite&                           sourcesData,
    const DocumentEnv&                           sourceEnv)
{
    try
    {
        SourcesSaver sourceSaver(sourceEnv);
        return sourceSaver.SaveRoot(sources, sourcesData);
    }
    catch (const exception&)
    {
        //const WChar* msg = ex.what();
        return false;
    }
}

//NEEDS_WORK_SM_IMPORTER : Should eventually remove the version with a SourcesDir
bool SaveSources   (const IDTMSourceCollection&     sources,
                    IScalableMeshSourceImporterStoragePtr&        sourceImporterStoragePtr,                    
                    const DocumentEnv&              sourceEnv)
    {
    try
        {
        SourcesSaver sourceSaver(sourceEnv);
        return sourceSaver.SaveRoot(sources, sourceImporterStoragePtr);
        }
    catch (const exception&)
        {
        //const WChar* msg = ex.what();
        return false;
        }
    }

/*
 * Load privates
 */
namespace { // Unamed namespace

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class SourcesLoader
    {
    typedef basic_stringstream<byte> 
                                    bstringstream;
    typedef basic_string<byte>      bstring;

    bstringstream                   m_serializationStream;


    bstring                         m_serializedData;

    HPU::Packet                     m_serializedSourcePacket;
    HPU::Packet                     m_serializedContentConfigPacket;
    HPU::Packet                     m_serializedImportSequencePacket;

    const DocumentEnv&              m_sourceEnv;

    FormatVersions                  m_fileFormatVersions;

    IDTMSourcePtr                   CreateSource                   (SourceDataSQLite& sourceData);

    IDTMSourcePtr                   CreateSource                   (IScalableMeshSourceImporterStoragePtr&       sourceImporterStoragePtr, 
                                                                    IScalableMeshSourceImporterStorage::GroupId& groupId);
    
    void                            Load                           (IDTMSourceCollection&               sources,
                                                                    SourcesDataSQLite&                   sourcesData);


    void                            Load                           (IDTMSourceCollection&               sources, 
                                                                    IScalableMeshSourceImporterStoragePtr&     sourceImporterStoragePtr);

public:
    explicit                        SourcesLoader                  (const DocumentEnv&                  sourceEnv);

    void                            LoadRoot                       (IDTMSourceCollection&               sources,
                                                                    SourcesDataSQLite&                  sourcesData);


    void                            LoadRoot                       (IDTMSourceCollection&           sources, 
                                                                    IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr);    
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesLoader::SourcesLoader(const DocumentEnv&  sourceEnv)
    : m_serializationStream(bstringstream::in | bstringstream::out | bstringstream::binary | bstringstream::app),
//    m_stream(m_serializationStream),
//m_sourceData(SourceDataSQLite::GetNull()),
    m_sourceEnv(sourceEnv),
    m_fileFormatVersions(CURRENT_FORMAT_VERSIONS)
{
}


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/

void SourcesLoader::LoadRoot(IDTMSourceCollection&   sources,
    SourcesDataSQLite&       sourcesData)
{

    Load(sources, sourcesData);
}


void SourcesLoader::LoadRoot   (IDTMSourceCollection&           sources, 
                                IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr)
    {
    // Load file format versions
    
    Load(sources, sourceImporterStoragePtr);    
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourcePtr SourcesLoader::CreateSource(SourceDataSQLite& sourceData)
{
    IDTMSourcePtr dataSourcePtr;
    // Deserialize source
    {
        /*m_serializationStream.str(bstring());
        assert(m_serializationStream.good());

        copy(m_serializedSourcePacket.Begin(), m_serializedSourcePacket.End(),
            ostreambuf_iterator<byte>(m_serializationStream));*/

        static const SourceSerializer DESERIALIZER;
        dataSourcePtr = DESERIALIZER.Deserialize(sourceData, m_sourceEnv, m_fileFormatVersions.serializedSource);

        if (0 == dataSourcePtr.get())
            throw runtime_error("Error creating data source!");

        Time lastModified = CreateTimeFrom(sourceData.GetTimeLastModified());
        dataSourcePtr->SetLastModified(lastModified);
    }

    // Deserialize content config
    //if (!m_serializedContentConfigPacket.IsEmpty())
    {
       /* m_serializationStream.str(bstring());

        copy(m_serializedContentConfigPacket.Begin(), m_serializedContentConfigPacket.End(),
            ostreambuf_iterator<byte>(m_serializationStream));*/

        ContentConfig config;

        static const ContentConfigSerializer DESERIALIZER;
        if (!DESERIALIZER.Deserialize(sourceData, config, m_fileFormatVersions.contentConfig))
            throw runtime_error("Error creating content config!");

        dataSourcePtr->EditConfig().SetInternalContentConfig(config);
    }

    // Deserialize import sequence
    //if (!m_serializedImportSequencePacket.IsEmpty())
    {
        /*m_serializationStream.str(bstring());

        copy(m_serializedImportSequencePacket.Begin(), m_serializedImportSequencePacket.End(),
            ostreambuf_iterator<byte>(m_serializationStream));*/

        ImportSequence sequence;

        static const ImportSequenceSerializer DESERIALIZER;
        if (!DESERIALIZER.Deserialize(sourceData, sequence, m_fileFormatVersions.importSequence))
            throw runtime_error("Error creating import sequence!");

        dataSourcePtr->EditConfig().SetInternalSequence(sequence);
    }

    return dataSourcePtr;
}


IDTMSourcePtr SourcesLoader::CreateSource (IScalableMeshSourceImporterStoragePtr&       sourceImporterStoragePtr, 
                                           IScalableMeshSourceImporterStorage::GroupId& groupId)
    {

        
    Time::TimeType                       sourceLastModifiedTime;

    StatusInt status = sourceImporterStoragePtr->GetSourceInfo(sourceLastModifiedTime, 
                                                               m_serializedSourcePacket, 
                                                               m_serializedContentConfigPacket, 
                                                               m_serializedImportSequencePacket, 
                                                               groupId);

    assert(status == SUCCESS);

    IDTMSourcePtr dataSourcePtr;
    // Deserialize source
        {
        m_serializationStream.str(bstring());
        assert(m_serializationStream.good());

        copy(m_serializedSourcePacket.Begin(), m_serializedSourcePacket.End(), 
             ostreambuf_iterator<byte>(m_serializationStream));

        #ifndef NDEBUG
        m_serializedData = m_serializationStream.str();
        #endif

        static const SourceSerializer DESERIALIZER;

//        dataSourcePtr = DESERIALIZER.Deserialize(sourceData, m_sourceEnv, m_fileFormatVersions.serializedSource);
        assert(false);


        if (0 == dataSourcePtr.get())
            throw runtime_error("Error creating data source!");

        Time lastModified = CreateTimeFrom(sourceLastModifiedTime);
        dataSourcePtr->SetLastModified(lastModified);
        }

    // Deserialize content config
    if (!m_serializedContentConfigPacket.IsEmpty())
        {
        m_serializationStream.str(bstring());

        copy(m_serializedContentConfigPacket.Begin(), m_serializedContentConfigPacket.End(), 
             ostreambuf_iterator<byte>(m_serializationStream));

        #ifndef NDEBUG
        m_serializedData = m_serializationStream.str();
        #endif

        ContentConfig config;

        static const ContentConfigSerializer DESERIALIZER;

            throw runtime_error("Error creating content config!");

        dataSourcePtr->EditConfig().SetInternalContentConfig(config);
        }

    // Deserialize import sequence
    if (!m_serializedImportSequencePacket.IsEmpty())
        {
        m_serializationStream.str(bstring());

        copy(m_serializedImportSequencePacket.Begin(), m_serializedImportSequencePacket.End(), 
             ostreambuf_iterator<byte>(m_serializationStream));

        #ifndef NDEBUG
        m_serializedData = m_serializationStream.str();
        #endif

        ImportSequence sequence;

        static const ImportSequenceSerializer DESERIALIZER;

            throw runtime_error("Error creating import sequence!");

        dataSourcePtr->EditConfig().SetInternalSequence(sequence);
        }

    return dataSourcePtr;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/

void SourcesLoader::Load(IDTMSourceCollection&           sources,
    SourcesDataSQLite&        sourcesData)
{
    IScalableMeshSourceImporterStorage::GroupId currentGroupId = NO_GROUP_ID;
    IScalableMeshSourceImporterStorage::GroupId groupId;
    IDTMSourceGroupPtr pGroup;
    for (auto sourceData : sourcesData.GetSourceDataSQLite())
    {
        groupId = sourceData.GetGroupID();

        IDTMSourcePtr sourcePtr(CreateSource(sourceData));

        if (groupId == NO_GROUP_ID)
        {
            sources.AddInternal(sourcePtr);
            currentGroupId = NO_GROUP_ID;
        }
        else
        {
            if (currentGroupId == NO_GROUP_ID)
            {
                currentGroupId = groupId;
                pGroup = IDTMSourceGroup::Create();
                assert(0 != pGroup.get());
                sources.AddInternal(pGroup.get());
            }
            else
            {
                assert(currentGroupId == groupId);
            }
            pGroup->GetImpl().GetSources().AddInternal(sourcePtr);
        }
    }
    /*typedef SourceSequenceDir::SourceCIter const_iterator;

    for (const_iterator sourceIt = sourceSequenceDir.SourcesBegin(), sourceEnd = sourceSequenceDir.SourcesEnd();
    sourceIt != sourceEnd;
        ++sourceIt)
    {
        sources.AddInternal(CreateSource(*sourceIt));
    }*/
}

/*
void SourcesLoader::Load(IDTMSourceCollection&       sources,
    SourcesDataSQLite&        sourcesData)
{
    typedef SourceNodeDir::NodeCIter const_iterator;

    for (const_iterator subNodeIt = sourceNodeDir.SubNodesBegin(), subNodeEnd = sourceNodeDir.SubNodesEnd();
    subNodeIt != subNodeEnd;
        ++subNodeIt)
    {
        const SourceNodeDir* subNodeP = subNodeIt->Get();
        if (0 == subNodeP)
            throw runtime_error("Could not access node directory");

        if (subNodeP->IsSourcesNode())
        {
            const SourceSequenceDir* pSourceSequence = subNodeP->GetSources();
            if (0 == pSourceSequence)
                throw runtime_error("Could not access sources");

            Load(sources, *pSourceSequence);
        }
        else
        {
            IDTMSourceGroupPtr pGroup = IDTMSourceGroup::Create();
            assert(0 != pGroup.get());

            pGroup->SetLastModified(CreateTimeFrom(subNodeP->GetLastModifiedTime()));

            sources.AddInternal(pGroup.get());

            Load(pGroup->GetImpl().GetSources(), *subNodeP);
        }
    }
}*/


void SourcesLoader::Load   (IDTMSourceCollection&           sources, 
                            IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr)
    {        
    if (sourceImporterStoragePtr->ReadFirstSource())
        {                
        IScalableMeshSourceImporterStorage::GroupId currentGroupId = NO_GROUP_ID;
        IScalableMeshSourceImporterStorage::GroupId groupId;
        IDTMSourceGroupPtr pGroup;
        
        do 
            {
            IDTMSourcePtr sourcePtr(CreateSource(sourceImporterStoragePtr, groupId));
                        
            if (groupId == NO_GROUP_ID)
                {
                sources.AddInternal(sourcePtr);
                currentGroupId = NO_GROUP_ID;
                }
            else
                {
                if (currentGroupId == NO_GROUP_ID) 
                    {
                    currentGroupId = groupId;
                    pGroup = IDTMSourceGroup::Create();
                    assert(0 != pGroup.get());

                    //NEEDS_WORK_SM_IMPORTER : Needed?
                    //pGroup->SetLastModified(CreateTimeFrom(subNodeP->GetLastModifiedTime()));

                    sources.AddInternal(pGroup.get());
                    }
                else
                    {
                    //Order must be preserved.
                    assert(currentGroupId == groupId);
                    }

                pGroup->GetImpl().GetSources().AddInternal(sourcePtr);                
                }
            
            } while (sourceImporterStoragePtr->ReadNextSource());     
        }    
    }

} // END Unamed namespace


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/

bool LoadSources(IDTMSourceCollection&   sources,
    SourcesDataSQLite& sourcesData,
    const DocumentEnv&      sourceEnv)
{
    try
    {
        SourcesLoader loader(sourceEnv);
        loader.LoadRoot(sources, sourcesData);

        return true;
    }
    catch (const exception&)
    {
        //const WChar* msg = ex.what();
        return false;
    }
}


//NEEDS_WORK_SM_IMPORTER : Should be eventually the only sources
bool LoadSources   (IDTMSourceCollection&           sources,
                    IScalableMeshSourceImporterStoragePtr& sourceImporterStoragePtr,
                    const DocumentEnv&              sourceEnv)
    {
    try
        {
        SourcesLoader loader(sourceEnv);
        loader.LoadRoot(sources, sourceImporterStoragePtr);

        return true;
        }
    catch (const exception&)
        {
        //const WChar* msg = ex.what();
        return false;
        }
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
