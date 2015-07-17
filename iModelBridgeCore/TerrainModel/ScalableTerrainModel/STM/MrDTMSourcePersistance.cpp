/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMSourcePersistance.cpp $
|    $RCSfile: MrDTMSourcePersistance.cpp,v $
|   $Revision: 1.12 $
|       $Date: 2012/02/16 22:19:19 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include "MrDTMSourcePersistance.h"

#include <ScalableTerrainModel/IMrDTMSourceVisitor.h>
#include <ScalableTerrainModel/IMrDTMStream.h>

#include "MrDTMSources.h"

BEGIN_BENTLEY_MRDTM_NAMESPACE

/*
 * Driver current version
 */ 
const UInt SourceSerializer::FORMAT_VERSION = 0;

namespace {
enum DTMSourceId
    {        
    DTM_SOURCE_ID_BASE_V0, 
    DTM_SOURCE_ID_LOCAL_FILE_V0, 
    DTM_SOURCE_ID_IN_MEMORY_BASE_V0, 
    DTM_SOURCE_ID_DGN_V0, 
    DTM_SOURCE_ID_DGN_LEVEL_V0, 
    DTM_SOURCE_ID_IN_MEMORY_DGN_LEVEL_V0,
    DTM_SOURCE_ID_BC_OBJ_V0,
    DTM_SOURCE_ID_GROUP_V0,   
    DTM_SOURCE_ID_DGN_V1, 
    DTM_SOURCE_ID_DGN_REFERENCE_LEVEL_V0, 
    DTM_SOURCE_ID_DGN_LEVEL_V1,
    DTM_SOURCE_ID_DGN_REFERENCE_LEVEL_V1,

    DTM_SOURCE_ID_QTY
    };

}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMSourceSerializer
    {
private:
    friend struct               SourceSerializer;

    virtual bool                _Serialize                 (const IDTMSource&   source,
                                                            const DocumentEnv&      env,
                                                            BinaryOStream&          stream) const = 0;

public:
    virtual                     ~IDTMSourceSerializer  () = 0 {}

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SourceT>
struct IDTMSourceSerializerBase : public IDTMSourceSerializer
    {
private:
    virtual bool                _Serialize                 (const IDTMSource&   source,
                                                            const DocumentEnv&      env,
                                                            BinaryOStream&          stream) const override
        {
        assert(0 != dynamic_cast<const SourceT*>(&source));
        return _Serialize(static_cast<const SourceT&>(source), env, stream);
        }
    virtual bool                _Serialize                 (const SourceT&          source,
                                                            const DocumentEnv&      env,
                                                            BinaryOStream&          stream) const = 0;
    

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMSourceCreator
    {
private:
    friend struct               IDTMSourceFactory;
    
    virtual IDTMSource*         _Create                    (BinaryIStream&      stream,
                                                            const DocumentEnv&    env) const = 0;
protected:


    virtual                     ~IDTMSourceCreator     () = 0 {}
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMSourceFactory
    {
    typedef const IDTMSourceCreator*   
                                CreatorItem;

    static const CreatorItem*   GetCreatorIndex            ();

public:
    IDTMSource*                 Create                     (BinaryIStream&      stream,
                                                            const DocumentEnv&    env) const;
    };




namespace { // BEGIN unnamed namespace

bool                            OutputID                       (BinaryOStream&                          stream,
                                                                DTMSourceId                             id)
    {
    const byte sourceIdField = static_cast<byte>(id);
    return stream.put(sourceIdField).good();
    }


bool                            OutputSource                   (BinaryOStream&                          stream,
                                                                const IDTMSource&                       source,
                                                                const DocumentEnv&                      env)
    {
    HSTATICASSERT(DTM_SOURCE_ID_QTY < 256);
    HSTATICASSERT(DTM_SOURCE_DATA_QTY < 256);

    const byte sourceTypeField = static_cast<byte>(source.GetSourceType());
    return stream.put(sourceTypeField).good() &&
           BSISUCCESS == source.GetMoniker().Serialize(stream, env);
    }




bool                            OutputLocalFile                (BinaryOStream&                          stream,
                                                                const IDTMLocalFileSource&              source,
                                                                const DocumentEnv&                      env)
    {
    return OutputSource(stream, source, env);
    }

bool                            OutputDGNFileModel             (BinaryOStream&                          stream,
                                                                const IDTMDgnModelSource&               source,
                                                                const DocumentEnv&                      env)
    {
    if (!WriteValue<UInt32>(stream, source.GetModelID()))
        return false;

    if (!WriteStringW(stream, source.GetModelName()))
        return false;

    return OutputSource(stream, source, env);
    }

bool                            OutputDGNFileLevel             (BinaryOStream&                          stream,
                                                                const IDTMDgnLevelSource&               source,
                                                                const DocumentEnv&                      env)
    {
    if (!WriteValue<UInt32>(stream, source.GetLevelID()))
        return false;

    if (!WriteStringW(stream, source.GetLevelName()))
        return false;

    return OutputDGNFileModel(stream, source, env);
    }



bool                            OutputDGNFileReference         (BinaryOStream&                          stream,
                                                                const IDTMDgnReferenceSource&           source,
                                                                const DocumentEnv&                      env)
    {
    // Why we persist in ASCII format???
    AString rootToRefPersistentPathA(source.GetRootToRefPersistentPath());
    if (!WriteStringA(stream, rootToRefPersistentPathA))
        return false;

    if (!WriteStringW(stream, source.GetReferenceName()))
        return false;

    if (!WriteStringW(stream, source.GetReferenceModelName()))
        return false;

    return OutputDGNFileModel(stream, source, env);
    }

bool                            OutputDGNFileReferenceLevel    (BinaryOStream&                          stream,
                                                                const IDTMDgnReferenceLevelSource&      source,
                                                                const DocumentEnv&                      env)
    {
    if (!WriteValue<UInt32>(stream, source.GetLevelID()))
        return false;

    if (!WriteStringW(stream, source.GetLevelName()))
        return false;

    return OutputDGNFileReference(stream, source, env);
    }

} // END unnamed namespace




struct IDTMLocalFileSourceSerializer : public IDTMSourceSerializerBase<IDTMLocalFileSource>
    {
private:

    virtual bool                _Serialize                 (const IDTMLocalFileSource&          source,
                                                            const DocumentEnv&                      env,
                                                            BinaryOStream&                          stream) const
        {
        return OutputID(stream, DTM_SOURCE_ID_LOCAL_FILE_V0) &&
               OutputLocalFile(stream, source, env);
        }
    } s_localFileSourceSerializer;


struct IDTMDgnLevelSourceSerializer : public IDTMSourceSerializerBase<IDTMDgnLevelSource>
    {
private:

    virtual bool                _Serialize                 (const IDTMDgnLevelSource&               source,
                                                            const DocumentEnv&                      env,
                                                            BinaryOStream&                          stream) const
        {
        return OutputID(stream, DTM_SOURCE_ID_DGN_LEVEL_V1) && 
               OutputDGNFileLevel(stream, source, env);
        }
    } s_dgnLevelSourceSerializer;



struct IDTMDgnReferenceLevelSourceSerializer : public IDTMSourceSerializerBase<IDTMDgnReferenceLevelSource>
    {
private:

    virtual bool                _Serialize                 (const IDTMDgnReferenceLevelSource&      source,
                                                            const DocumentEnv&                      env,
                                                            BinaryOStream&                          stream) const
        {
        return OutputID(stream, DTM_SOURCE_ID_DGN_REFERENCE_LEVEL_V1) &&
               OutputDGNFileReferenceLevel(stream, source, env);
        }
    } s_dgnReferenceLevelSourceSerializer;




namespace { // BEGIN unnamed namespace

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceVisitor : public IDTMSourceVisitor
    {
private:
    const IDTMSourceSerializer* m_serializerP;
    StatusInt                       m_status;


    virtual void                    _Visit                         (const IDTMLocalFileSource&          source)
        {
        m_serializerP = &s_localFileSourceSerializer;
        }

    virtual void                    _Visit                         (const IDTMDgnModelSource&                source)
        {
        assert(!"Unsupported yet!");
        }

    virtual void                    _Visit                         (const IDTMDgnReferenceSource&           source)
        {
        assert(!"Unsupported yet!");
        }

    virtual void                    _Visit                         (const IDTMDgnLevelSource&               source)
        {
        m_serializerP = &s_dgnLevelSourceSerializer;
        }

    virtual void                    _Visit                         (const IDTMDgnReferenceLevelSource&      source)
        {
        m_serializerP = &s_dgnReferenceLevelSourceSerializer;
        }


    virtual void                    _Visit                         (const IDTMSourceGroup&              source)
        {
        assert(!"Unsupported yet!");
        }

public:
    explicit                        SourceVisitor                  (BinaryOStream&                          stream)
        :   m_serializerP(0)
        {
        
        }

    const IDTMSourceSerializer* GetFoundSerializer             () const { return m_serializerP; }

    };



bool                            LoadSourcePart             (BinaryIStream&      stream,
                                                            const DocumentEnv&    env,
                                                            DTMSourceDataType&  sourceType,
                                                            IMonikerPtr&        monikerPtr)
    {
    const UInt sourceTypeField = stream.get();
    if (sourceTypeField >= DTM_SOURCE_DATA_QTY)
        return false;

    sourceType = static_cast<DTMSourceDataType>(sourceTypeField);
    monikerPtr = IMonikerFactory::GetInstance().Create(stream, env);
    if (0 == monikerPtr.get())
        return false;

    return true;
    }


bool                            LoadDGNV0ModelPart         (BinaryIStream&      stream,
                                                            WString&            modelName)
    {
    WString modelNameField;
    if (!ReadStringW(stream, modelNameField))
        return false;

    modelName = modelNameField;
    return true;
    }

bool                            LoadDGNV0LevelPart         (BinaryIStream&      stream,
                                                            WString&            levelName)
    {
    WString levelNameField;
    if (!ReadStringW(stream, levelNameField))
        return false;

    levelName = levelNameField;
    return true;
    }



bool                            LoadDGNV1ModelPart         (BinaryIStream&      stream,
                                                            UInt32&             modelID,
                                                            WString&            modelName)
    {
    UInt32 modelIDField = 0;
    if (!ReadValue(stream, modelIDField))
        return false;

    WString modelNameField;
    if (!ReadStringW(stream, modelNameField))
        return false;


    modelID = modelIDField;
    modelName = modelNameField;
    return true;
    }


// TDORAY: Deprecated. Remove.
bool                            LoadDGNV0ReferencePart     (BinaryIStream&      stream,
                                                            WString&             referencePersistantPath,
                                                            WString&            referenceName)
    {
    AString referencePersistantPathField;
    if (!ReadStringA(stream, referencePersistantPathField))
        return false;

    WString referenceNameField;
    if (!ReadStringW(stream, referenceNameField))
        return false;

    referencePersistantPath.AssignA(referencePersistantPathField.c_str());
    referenceName = referenceNameField;
    return true;
    }


bool                            LoadDGNV1ReferencePart     (BinaryIStream&      stream,
                                                            WString&             referencePersistantPath,
                                                            WString&            referenceName,
                                                            WString&            referenceModelName)
    {
    AString referencePersistantPathField;
    if (!ReadStringA(stream, referencePersistantPathField))
        return false;

    WString referenceNameField;
    if (!ReadStringW(stream, referenceNameField))
        return false;

    WString referenceModelNameField;
    if (!ReadStringW(stream, referenceModelNameField))
        return false;

    referencePersistantPath.AssignA(referencePersistantPathField.c_str());
    referenceName = referenceNameField;
    referenceModelName = referenceModelNameField;
    return true;
    }


bool                            LoadDGNV1LevelPart         (BinaryIStream&      stream,
                                                            UInt32&             levelID,
                                                            WString&            levelName)
    {
    UInt32 levelIDField = 0;
    if (!ReadValue(stream, levelIDField))
        return false;

    WString levelNameField;
    if (!ReadStringW(stream, levelNameField))
        return false;

    levelID = levelIDField;
    levelName = levelNameField;
    return true;
    }





} // END unnamed namespace





struct IDTMLocalFileSourceCreator : public IDTMSourceCreator
    {
    virtual IDTMSource*     _Create                    (BinaryIStream&      stream,
                                                            const DocumentEnv&    env) const
        {
        DTMSourceDataType   sourceType;
        IMonikerPtr         monikerPtr;

        if (LoadSourcePart(stream, env, sourceType, monikerPtr))
            return new IDTMLocalFileSource(new IDTMLocalFileSource::Impl(sourceType, monikerPtr.get()));

        return 0;
        }

    } s_localFileSourceCreator;

struct IDTMDgnLevelSourceCreator : public IDTMSourceCreator
    {
    virtual IDTMSource*     _Create                    (BinaryIStream&      stream,
                                                            const DocumentEnv&    env) const
        {
        DTMSourceDataType   sourceType;
        IMonikerPtr         monikerPtr;
        UInt32              modelID;
        WString             modelName;
        UInt32              levelID;
        WString             levelName;


        if (LoadDGNV1LevelPart(stream, levelID, levelName) &&
            LoadDGNV1ModelPart(stream, modelID, modelName) &&
            LoadSourcePart(stream, env, sourceType, monikerPtr))
            return new IDTMDgnLevelSource(new IDTMDgnLevelSource::Impl(sourceType, 
                                                                       monikerPtr.get(), 
                                                                       modelID, 
                                                                       modelName.c_str(),
                                                                       levelID,
                                                                       levelName.c_str()));

        return 0;
        }
    } s_dgnLevelSourceCreator;


// TDORAY: Deprecated. Remove
struct IDTMDgnReferenceLevelSourceCreatorV0 : public IDTMSourceCreator
    {
    virtual IDTMSource*     _Create                    (BinaryIStream&      stream,
                                                            const DocumentEnv&  env) const
        {
        DTMSourceDataType   sourceType;
        IMonikerPtr         monikerPtr;
        UInt32              modelID;
        WString             modelName;
        WString             referencePersistantPath;
        WString             referenceName;
        UInt32              levelID;
        WString             levelName;


        if (LoadDGNV1LevelPart(stream, levelID, levelName) &&
            LoadDGNV0ReferencePart(stream, referencePersistantPath, referenceName) &&
            LoadDGNV1ModelPart(stream, modelID, modelName) &&
            LoadSourcePart(stream, env, sourceType, monikerPtr))
            return new IDTMDgnReferenceLevelSource(new IDTMDgnReferenceLevelSource::Impl(sourceType, 
                                                                                         monikerPtr.get(), 
                                                                                         modelID, 
                                                                                         modelName.c_str(),
                                                                                         referencePersistantPath.c_str(), 
                                                                                         referenceName.c_str(),
                                                                                         L"",
                                                                                         levelID,
                                                                                         levelName.c_str()));

        return 0;
        }
    } s_dgnReferenceLevelSourceCreatorV0;


struct IDTMDgnReferenceLevelSourceCreator : public IDTMSourceCreator
    {
    virtual IDTMSource*     _Create                    (BinaryIStream&      stream,
                                                            const DocumentEnv&  env) const
        {
        DTMSourceDataType   sourceType;
        IMonikerPtr         monikerPtr;
        UInt32              modelID;
        WString             modelName;
        WString             referencePersistantPath;
        WString             referenceName;
        WString             referenceModelName;
        UInt32              levelID;
        WString             levelName;


        if (LoadDGNV1LevelPart(stream, levelID, levelName) &&
            LoadDGNV1ReferencePart(stream, referencePersistantPath, referenceName, referenceModelName) &&
            LoadDGNV1ModelPart(stream, modelID, modelName) &&
            LoadSourcePart(stream, env, sourceType, monikerPtr))
            return new IDTMDgnReferenceLevelSource(new IDTMDgnReferenceLevelSource::Impl(sourceType, 
                                                                                         monikerPtr.get(), 
                                                                                         modelID, 
                                                                                         modelName.c_str(),
                                                                                         referencePersistantPath.c_str(), 
                                                                                         referenceName.c_str(),
                                                                                         referenceModelName.c_str(),
                                                                                         levelID,
                                                                                         levelName.c_str()));

        return 0;
        }
    } s_dgnReferenceLevelSourceCreator;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const IDTMSourceFactory::CreatorItem* IDTMSourceFactory::GetCreatorIndex ()
    {
    static const CreatorItem    CREATORS_INDEX[] = 
        {
        0, 
        &s_localFileSourceCreator, 
        0, 
        0, 
        0, 
        0,
        0,
        0,  
        0,
        &s_dgnReferenceLevelSourceCreatorV0,
        &s_dgnLevelSourceCreator,
        &s_dgnReferenceLevelSourceCreator,
        };

    static_assert(DTM_SOURCE_ID_QTY == sizeof(CREATORS_INDEX)/sizeof(CREATORS_INDEX[0]), "");

    return CREATORS_INDEX;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSource* IDTMSourceFactory::Create  (BinaryIStream&          stream,
                                                const DocumentEnv&        env) const
    {
    const UInt sourceIDField = stream.get();

    if (DTM_SOURCE_ID_QTY <= sourceIDField)
        return 0;

    DTMSourceId sourceID = static_cast<DTMSourceId>(sourceIDField);

    CreatorItem creatorP = GetCreatorIndex()[sourceID];
    if (0 == creatorP)
        return 0;
    
    return creatorP->_Create(stream, env);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceSerializer::Serialize   (const IDTMSource&   source,
                                    const DocumentEnv&      env,
                                    BinaryOStream&          stream) const
    {
    SourceVisitor visitor (stream);

    source.Accept(visitor);

    const IDTMSourceSerializer* serializerP = visitor.GetFoundSerializer();
    
    if (0 == serializerP)
        return false;

    return serializerP->_Serialize(source, env, stream);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourcePtr SourceSerializer::Deserialize    (BinaryIStream&          stream,
                                                const DocumentEnv&      env,
                                                UInt                    formatVersion) const
    {
    if (SourceSerializer::FORMAT_VERSION != formatVersion)
        {
        assert(!"Invalid version! Need to handle backward compatibility!");
        return 0;
        }

    static const IDTMSourceFactory SOURCE_FACTORY;

    return SOURCE_FACTORY.Create(stream, env);
    }



END_BENTLEY_MRDTM_NAMESPACE