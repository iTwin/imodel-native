/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMSourcesPersistance.cpp $
|    $RCSfile: MrDTMSourcesPersistance.cpp,v $
|   $Revision: 1.19 $
|       $Date: 2011/11/21 17:01:01 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include "MrDTMSourcesPersistance.h"

#include "MrDTMSources.h"


#include <ScalableTerrainModel/IMrDTMSourceImportConfig.h>
#include <ScalableTerrainModel/IMrDTMStream.h>

#include "MrDTMSourcePersistance.h"
#include "MrDTMImportSequencePersistance.h"
#include "MrDTMContentConfigPersistance.h"

using namespace std;
using namespace IDTMFile;
USING_NAMESPACE_BENTLEY_MRDTM_IMPORT

BEGIN_BENTLEY_MRDTM_NAMESPACE


/*
 * Save privates
 */
namespace { // Unamed namespace

struct FormatVersions
    {
    UInt                            serializedSource;
    UInt                            contentConfig;
    UInt                            importSequence;
    UInt                            importConfig;
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



    bstringstream               m_serializationStream;
    BinaryOStream               m_stream;

    bstring                     m_serializedSource;
    HPU::Packet                 m_serializedSourcePacket;

    bstring                     m_serializedContentConfig;
    HPU::Packet                 m_serializedContentConfigPacket;

    bstring                     m_serializedImportSequence;
    HPU::Packet                 m_serializedImportSequencePacket;

    const DocumentEnv&          m_sourceEnv;

    bool                        Save                                   (const IDTMSource&               source, 
                                                                        SourceSequenceDir&              sources);

    bool                        Save                                   (const IDTMSourceCollection&     sources, 
                                                                        SourceNodeDir&                  sourceNode);

public:
    explicit                    SourcesSaver                           (const DocumentEnv&              sourceEnv);

    
    bool                        SaveRoot                               (const IDTMSourceCollection&     sources, 
                                                                        SourcesDir&                     sourcesDir);

    };


/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesSaver::SourcesSaver (const DocumentEnv&  sourceEnv)
    :   m_serializationStream(bstringstream::in | bstringstream::out | bstringstream::binary | bstringstream::app),
        m_stream(m_serializationStream),
        m_sourceEnv(sourceEnv)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourcesSaver::SaveRoot    (const IDTMSourceCollection&     sources, 
                                SourcesDir&                     sourcesDir)
    {
    if (!sourcesDir.SetSerializedSourceFormatVersion(CURRENT_FORMAT_VERSIONS.serializedSource) ||
        !sourcesDir.SetContentConfigFormatVersion(CURRENT_FORMAT_VERSIONS.contentConfig) ||
        !sourcesDir.SetImportSequenceFormatVersion(CURRENT_FORMAT_VERSIONS.importSequence) ||
        !sourcesDir.SetImportConfigFormatVersion(CURRENT_FORMAT_VERSIONS.importConfig))
        return false;

    return Save(sources, sourcesDir);
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourcesSaver::Save (const IDTMSource& source, SourceSequenceDir& sourceSequenceDir)
    {
    bool success = true;

    const Time& lastModified = source.GetLastModified();


    // Serializing source
        {
        m_serializationStream.str(bstring());
        assert(m_serializationStream.good());

        static const SourceSerializer SERIALIZER;
        if (!SERIALIZER.Serialize(source, m_sourceEnv, m_stream))
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
        if (SERIALIZER.Serialize(source.GetConfig().GetContentConfig(), m_stream))
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
        if (SERIALIZER.Serialize(source.GetConfig().GetSequence(), m_stream))
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

    sourceSequenceDir.AddSource(GetCTimeFor(lastModified), m_serializedSourcePacket, m_serializedContentConfigPacket, m_serializedImportSequencePacket);
    return success;
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourcesSaver::Save    (const IDTMSourceCollection&     sources, 
                            SourceNodeDir&                  sourceNodeDir)
    {
    bool success = true;

    SourceSequenceDir* sourceSequenceDirPtr = 0;

    for (IDTMSourceCollection::const_iterator sourceIt = sources.Begin(), sourceEnd = sources.End(); 
         sourceIt != sourceEnd; 
         ++sourceIt)
        {
        const IDTMSource& dataSource = *sourceIt;
        
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

    return success;
    }


} // END Unamed namespace



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SaveSources   (const IDTMSourceCollection&     sources,
                    SourcesDir&                     sourcesDir,
                    const DocumentEnv&              sourceEnv)
    {
    try
        {
        SourcesSaver sourceSaver(sourceEnv);
        return sourceSaver.SaveRoot(sources, sourcesDir);
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
    BinaryIStream                   m_stream;

    bstring                         m_serializedData;

    HPU::Packet                     m_serializedSourcePacket;
    HPU::Packet                     m_serializedContentConfigPacket;
    HPU::Packet                     m_serializedImportSequencePacket;

    const DocumentEnv&              m_sourceEnv;

    FormatVersions                  m_fileFormatVersions;


    IDTMSourcePtr                   CreateSource                   (const SourceSequenceDir::Source&    source);

    void                            Load                           (IDTMSourceCollection&               sources, 
                                                                    const SourceSequenceDir&            sourcesDir);
    
    void                            Load                           (IDTMSourceCollection&               sources, 
                                                                    const SourceNodeDir&                sourceNode);

public:
    explicit                        SourcesLoader                  (const DocumentEnv&                  sourceEnv);

    void                            LoadRoot                       (IDTMSourceCollection&               sources, 
                                                                    const SourcesDir&                   sourcesDir);

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesLoader::SourcesLoader (const DocumentEnv&  sourceEnv)
    :   m_serializationStream(bstringstream::in | bstringstream::out | bstringstream::binary | bstringstream::app),
        m_stream(m_serializationStream),
        m_sourceEnv(sourceEnv),
        m_fileFormatVersions(CURRENT_FORMAT_VERSIONS)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourcesLoader::LoadRoot   (IDTMSourceCollection&   sources, 
                                const SourcesDir&       sourcesDir)
    {
    // Load file format versions
    m_fileFormatVersions.serializedSource = sourcesDir.GetSerializedSourceFormatVersion();
    m_fileFormatVersions.contentConfig = sourcesDir.GetContentConfigFormatVersion();
    m_fileFormatVersions.importSequence = sourcesDir.GetImportSequenceFormatVersion();
    m_fileFormatVersions.importConfig = sourcesDir.GetImportConfigFormatVersion();

    // Check that format versions found are anterior or equal to current driver's
    if (CURRENT_FORMAT_VERSIONS.serializedSource < m_fileFormatVersions.serializedSource ||
        CURRENT_FORMAT_VERSIONS.contentConfig < m_fileFormatVersions.contentConfig ||
        CURRENT_FORMAT_VERSIONS.importSequence < m_fileFormatVersions.importSequence ||
        CURRENT_FORMAT_VERSIONS.importConfig < m_fileFormatVersions.importConfig)
        {
        throw runtime_error("Found more recent format version. Driver cannot be forward compatible!");
        }
    
    Load(sources, sourcesDir);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourcePtr SourcesLoader::CreateSource (const SourceSequenceDir::Source& source)
    {
    if (!source.GetSerializedSource(m_serializedSourcePacket))
        throw runtime_error("Could not fetch serialized source packet!");
    if (!source.GetContentConfig(m_serializedContentConfigPacket))
        throw runtime_error("Could not fetch serialized content config");
    if (!source.GetImportSequence(m_serializedImportSequencePacket))
        throw runtime_error("Could not fetch serialized import sequence!");

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
        dataSourcePtr = DESERIALIZER.Deserialize(m_stream, m_sourceEnv, m_fileFormatVersions.serializedSource);

        if (0 == dataSourcePtr.get())
            throw runtime_error("Error creating data source!");


        Time lastModified = CreateTimeFrom(source.GetLastModified());
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
        if (!DESERIALIZER.Deserialize(m_stream, config, m_fileFormatVersions.contentConfig))
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
        if (!DESERIALIZER.Deserialize(m_stream, sequence, m_fileFormatVersions.importSequence))
            throw runtime_error("Error creating import sequence!");

        dataSourcePtr->EditConfig().SetInternalSequence(sequence);

        }

    return dataSourcePtr;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourcesLoader::Load   (IDTMSourceCollection&           sources, 
                            const SourceSequenceDir&        sourceSequenceDir)
    {
    typedef SourceSequenceDir::SourceCIter const_iterator;

    for (const_iterator sourceIt = sourceSequenceDir.SourcesBegin(), sourceEnd = sourceSequenceDir.SourcesEnd();
         sourceIt != sourceEnd;
         ++sourceIt)
        {
        sources.AddInternal(CreateSource(*sourceIt));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourcesLoader::Load   (IDTMSourceCollection&       sources, 
                            const SourceNodeDir&        sourceNodeDir)
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
    }


} // END Unamed namespace


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LoadSources   (IDTMSourceCollection&   sources,
                    const SourcesDir&       sourcesDir,
                    const DocumentEnv&      sourceEnv)
    {
    try
        {
        SourcesLoader loader(sourceEnv);
        loader.LoadRoot(sources, sourcesDir);

        return true;
        }
    catch (const exception&)
        {
        //const WChar* msg = ex.what();
        return false;
        }
    }

END_BENTLEY_MRDTM_NAMESPACE