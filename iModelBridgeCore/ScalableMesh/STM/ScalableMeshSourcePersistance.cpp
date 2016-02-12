/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourcePersistance.cpp $
|    $RCSfile: ScalableMeshSourcePersistance.cpp,v $
|   $Revision: 1.12 $
|       $Date: 2012/02/16 22:19:19 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
   
#include "ScalableMeshSourcePersistance.h"

#include <ScalableMesh/IScalableMeshSourceVisitor.h>
#include <ScalableMesh/IScalableMeshStream.h>
#include <ScalableMesh/Import/DataSQLite.h>

#include "ScalableMeshSources.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

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
#ifdef SCALABLE_MESH_DGN
    virtual bool                _Serialize(const IDTMSource&   source,
        const DocumentEnv&      env,
        SourceDataSQLite&          sourceData) const = 0;
#else
    virtual bool                _Serialize                 (const IDTMSource&   source,
                                                            const DocumentEnv&      env,
                                                            BinaryOStream&          stream) const = 0;
#endif

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
#ifdef SCALABLE_MESH_DGN
    virtual bool                _Serialize(const IDTMSource&   source,
        const DocumentEnv&      env,
        SourceDataSQLite&          sourceData) const override
    {
        assert(0 != dynamic_cast<const SourceT*>(&source));
        return _Serialize(static_cast<const SourceT&>(source), env, sourceData);
    }
    virtual bool                _Serialize(const SourceT&          source,
        const DocumentEnv&      env,
        SourceDataSQLite&          sourceData) const = 0;
#else
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
#endif
    

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMSourceCreator
    {
private:
    friend struct               IDTMSourceFactory;
    
#ifdef SCALABLE_MESH_DGN
    virtual IDTMSource*         _Create(SourceDataSQLite&      sourceData,
        const DocumentEnv&    env) const = 0;
#else
    virtual IDTMSource*         _Create                    (BinaryIStream&      stream,
                                                            const DocumentEnv&    env) const = 0;
#endif
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
#ifdef SCALABLE_MESH_DGN
    IDTMSource*                 Create(SourceDataSQLite&      sourceData,
        const DocumentEnv&    env) const;
#else
    IDTMSource*                 Create                     (BinaryIStream&      stream,
                                                            const DocumentEnv&    env) const;
#endif
    };




namespace { // BEGIN unnamed namespace
#ifdef SCALABLE_MESH_DGN
    bool                            OutputID(SourceDataSQLite&                          sourceData,
        DTMSourceId                             id)
    {
        const byte sourceIdField = static_cast<byte>(id);
        sourceData.SetDTMSourceID(sourceIdField);
        return true;
    }
#else
bool                            OutputID                       (BinaryOStream&                          stream,
                                                                DTMSourceId                             id)
    {
    const byte sourceIdField = static_cast<byte>(id);
    return stream.put(sourceIdField).good();
    }
#endif

#ifdef SCALABLE_MESH_DGN
bool                            OutputSource(SourceDataSQLite&                          sourceData,
    const IDTMSource&                       source,
    const DocumentEnv&                      env)
{
    HSTATICASSERT(DTM_SOURCE_ID_QTY < 256);
    HSTATICASSERT(DTM_SOURCE_DATA_QTY < 256);

    const byte sourceTypeField = static_cast<byte>(source.GetSourceType());
    sourceData.SetSourceType(sourceTypeField);
    return BSISUCCESS == source.GetMoniker().Serialize(sourceData, env);
}




bool                            OutputLocalFile(SourceDataSQLite&                          sourceData,
    const IDTMLocalFileSource&              source,
    const DocumentEnv&                      env)
{
    return OutputSource(sourceData, source, env);
}

bool                            OutputDGNFileModel(SourceDataSQLite&                          sourceData,
    const IDTMDgnModelSource&               source,
    const DocumentEnv&                      env)
{
    sourceData.SetModelID(source.GetModelID());
    sourceData.SetModelName(source.GetModelName());

    return OutputSource(sourceData, source, env);
}

bool                            OutputDGNFileLevel(SourceDataSQLite&                          sourceData,
    const IDTMDgnLevelSource&               source,
    const DocumentEnv&                      env)
{
    sourceData.SetLevelID(source.GetLevelID());
    sourceData.SetLevelName(source.GetLevelName());

    return OutputDGNFileModel(sourceData, source, env);
}



bool                            OutputDGNFileReference(SourceDataSQLite&                          sourceData,
    const IDTMDgnReferenceSource&           source,
    const DocumentEnv&                      env)
{
    // Why we persist in ASCII format???
    //AString rootToRefPersistentPathA(source.GetRootToRefPersistentPath());

    sourceData.SetRootToRefPersistentPath(source.GetRootToRefPersistentPath());
    sourceData.SetReferenceName(source.GetReferenceName());
    sourceData.SetReferenceModelName(source.GetReferenceModelName());

    return OutputDGNFileModel(sourceData, source, env);
}

bool                            OutputDGNFileReferenceLevel(SourceDataSQLite&                          sourceData,
    const IDTMDgnReferenceLevelSource&      source,
    const DocumentEnv&                      env)
{
    sourceData.SetLevelID(source.GetLevelID());
    sourceData.SetLevelName(source.GetLevelName());

    return OutputDGNFileReference(sourceData, source, env);
}
#else
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
    if (!WriteValue<uint32_t>(stream, source.GetModelID()))
        return false;

    if (!WriteStringW(stream, source.GetModelName()))
        return false;

    return OutputSource(stream, source, env);
    }

bool                            OutputDGNFileLevel             (BinaryOStream&                          stream,
                                                                const IDTMDgnLevelSource&               source,
                                                                const DocumentEnv&                      env)
    {
    if (!WriteValue<uint32_t>(stream, source.GetLevelID()))
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
    if (!WriteValue<uint32_t>(stream, source.GetLevelID()))
        return false;

    if (!WriteStringW(stream, source.GetLevelName()))
        return false;

    return OutputDGNFileReference(stream, source, env);
    }
#endif
} // END unnamed namespace




struct IDTMLocalFileSourceSerializer : public IDTMSourceSerializerBase<IDTMLocalFileSource>
    {
private:
#ifdef SCALABLE_MESH_DGN
    virtual bool                _Serialize(const IDTMLocalFileSource&          source,
        const DocumentEnv&                      env,
        SourceDataSQLite&                          sourceData) const
    {
        return OutputID(sourceData, DTM_SOURCE_ID_LOCAL_FILE_V0) &&
            OutputLocalFile(sourceData, source, env);
    }
#else
    virtual bool                _Serialize                 (const IDTMLocalFileSource&          source,
                                                            const DocumentEnv&                      env,
                                                            BinaryOStream&                          stream) const
        {
        return OutputID(stream, DTM_SOURCE_ID_LOCAL_FILE_V0) &&
               OutputLocalFile(stream, source, env);
        }
#endif
    } s_localFileSourceSerializer;


struct IDTMDgnLevelSourceSerializer : public IDTMSourceSerializerBase<IDTMDgnLevelSource>
    {
private:
#ifdef SCALABLE_MESH_DGN
    virtual bool                _Serialize(const IDTMDgnLevelSource&               source,
        const DocumentEnv&                      env,
        SourceDataSQLite&                          sourceData) const
    {
        return OutputID(sourceData, DTM_SOURCE_ID_DGN_LEVEL_V1) &&
            OutputDGNFileLevel(sourceData, source, env);
    }
#else
    virtual bool                _Serialize                 (const IDTMDgnLevelSource&               source,
                                                            const DocumentEnv&                      env,
                                                            BinaryOStream&                          stream) const
        {
        return OutputID(stream, DTM_SOURCE_ID_DGN_LEVEL_V1) && 
               OutputDGNFileLevel(stream, source, env);
        }
#endif
    } s_dgnLevelSourceSerializer;



struct IDTMDgnReferenceLevelSourceSerializer : public IDTMSourceSerializerBase<IDTMDgnReferenceLevelSource>
    {
private:
#ifdef SCALABLE_MESH_DGN
    virtual bool                _Serialize(const IDTMDgnReferenceLevelSource&      source,
        const DocumentEnv&                      env,
        SourceDataSQLite&                          sourceData) const
    {
        return OutputID(sourceData, DTM_SOURCE_ID_DGN_REFERENCE_LEVEL_V1) &&
            OutputDGNFileReferenceLevel(sourceData, source, env);
    }
#else
    virtual bool                _Serialize                 (const IDTMDgnReferenceLevelSource&      source,
                                                            const DocumentEnv&                      env,
                                                            BinaryOStream&                          stream) const
        {
        return OutputID(stream, DTM_SOURCE_ID_DGN_REFERENCE_LEVEL_V1) &&
               OutputDGNFileReferenceLevel(stream, source, env);
        }
#endif
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
#ifdef SCALABLE_MESH_DGN
    explicit                        SourceVisitor(SourceDataSQLite&                          sourceData)
        : m_serializerP(0)
    {

    }
#else
    explicit                        SourceVisitor                  (BinaryOStream&                          stream)
        :   m_serializerP(0)
        {
        
        }
#endif

    const IDTMSourceSerializer* GetFoundSerializer             () const { return m_serializerP; }

    };


#ifdef SCALABLE_MESH_DGN
bool                            LoadSourcePart(SourceDataSQLite&      sourceData,
    const DocumentEnv&    env,
    DTMSourceDataType&  sourceType,
    IMonikerPtr&        monikerPtr)
{
    //const UInt sourceTypeField = stream.get();
    uint32_t sourceTypeField = sourceData.GetSourceType();
    if (sourceTypeField >= DTM_SOURCE_DATA_QTY)
        return false;

    sourceType = static_cast<DTMSourceDataType>(sourceTypeField);
    monikerPtr = IMonikerFactory::GetInstance().Create(sourceData, env);
    if (0 == monikerPtr.get())
        return false;

    return true;
}


bool                            LoadDGNV0ModelPart(SourceDataSQLite&      sourceData,
    WString&            modelName)
{
    WString modelNameField = sourceData.GetModelName();
    //if (!ReadStringW(stream, modelNameField))
    //    return false;

    modelName = modelNameField;
    return true;
}

bool                            LoadDGNV0LevelPart(SourceDataSQLite&      sourceData,
    WString&            levelName)
{
    WString levelNameField = sourceData.GetLevelName();

    levelName = levelNameField;
    return true;
}



bool                            LoadDGNV1ModelPart(SourceDataSQLite&      sourceData,
    uint32_t&             modelID,
    WString&            modelName)
{
    uint32_t modelIDField = sourceData.GetModelID();

    WString modelNameField = sourceData.GetModelName();

    modelID = modelIDField;
    modelName = modelNameField;
    return true;
}


// TDORAY: Deprecated. Remove.
bool                            LoadDGNV0ReferencePart(SourceDataSQLite&      sourceData,
    WString&             referencePersistantPath,
    WString&            referenceName)
{
    WString referencePersistantPathField = sourceData.GetRootToRefPersistentPath();

    WString referenceNameField = sourceData.GetReferenceName();

    referencePersistantPath = referencePersistantPathField;
    referenceName = referenceNameField;
    return true;
}


bool                            LoadDGNV1ReferencePart(SourceDataSQLite&      sourceData,
    WString&             referencePersistantPath,
    WString&            referenceName,
    WString&            referenceModelName)
{
    WString referencePersistantPathField = sourceData.GetRootToRefPersistentPath();

    WString referenceNameField = sourceData.GetReferenceName();

    WString referenceModelNameField = sourceData.GetReferenceModelName();

    referencePersistantPath = referencePersistantPathField;
    referenceName = referenceNameField;
    referenceModelName = referenceModelNameField;
    return true;
}


bool                            LoadDGNV1LevelPart(SourceDataSQLite&      sourceData,
    uint32_t&             levelID,
    WString&            levelName)
{
    uint32_t levelIDField = sourceData.GetLevelID();

    WString levelNameField = sourceData.GetLevelName();

    levelID = levelIDField;
    levelName = levelNameField;
    return true;
}
#else
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
                                                            uint32_t&             modelID,
                                                            WString&            modelName)
    {
    uint32_t modelIDField = 0;
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
                                                            uint32_t&             levelID,
                                                            WString&            levelName)
    {
    uint32_t levelIDField = 0;
    if (!ReadValue(stream, levelIDField))
        return false;

    WString levelNameField;
    if (!ReadStringW(stream, levelNameField))
        return false;

    levelID = levelIDField;
    levelName = levelNameField;
    return true;
    }
#endif




} // END unnamed namespace





struct IDTMLocalFileSourceCreator : public IDTMSourceCreator
    {
#ifdef SCALABLE_MESH_DGN
        virtual IDTMSource*     _Create(SourceDataSQLite&      sourceData,
            const DocumentEnv&    env) const
        {
            DTMSourceDataType   sourceType;
            IMonikerPtr         monikerPtr;

            if (LoadSourcePart(sourceData, env, sourceType, monikerPtr))
                return new IDTMLocalFileSource(new IDTMLocalFileSource::Impl(sourceType, monikerPtr.get()));

            return 0;
        }
#else
    virtual IDTMSource*     _Create                    (BinaryIStream&      stream,
                                                            const DocumentEnv&    env) const
        {
        DTMSourceDataType   sourceType;
        IMonikerPtr         monikerPtr;

        if (LoadSourcePart(stream, env, sourceType, monikerPtr))
            return new IDTMLocalFileSource(new IDTMLocalFileSource::Impl(sourceType, monikerPtr.get()));

        return 0;
        }
#endif

    } s_localFileSourceCreator;

struct IDTMDgnLevelSourceCreator : public IDTMSourceCreator
    {
#ifdef SCALABLE_MESH_DGN
        virtual IDTMSource*     _Create(SourceDataSQLite&      sourceData,
            const DocumentEnv&    env) const
        {
            DTMSourceDataType   sourceType;
            IMonikerPtr         monikerPtr;
            uint32_t              modelID;
            WString             modelName;
            uint32_t              levelID;
            WString             levelName;


            if (LoadDGNV1LevelPart(sourceData, levelID, levelName) &&
                LoadDGNV1ModelPart(sourceData, modelID, modelName) &&
                LoadSourcePart(sourceData, env, sourceType, monikerPtr))
                return new IDTMDgnLevelSource(new IDTMDgnLevelSource::Impl(sourceType,
                    monikerPtr.get(),
                    modelID,
                    modelName.c_str(),
                    levelID,
                    levelName.c_str()));

            return 0;
        }
#else
    virtual IDTMSource*     _Create                    (BinaryIStream&      stream,
                                                            const DocumentEnv&    env) const
        {
        DTMSourceDataType   sourceType;
        IMonikerPtr         monikerPtr;
        uint32_t              modelID;
        WString             modelName;
        uint32_t              levelID;
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
#endif
    } s_dgnLevelSourceCreator;


// TDORAY: Deprecated. Remove
struct IDTMDgnReferenceLevelSourceCreatorV0 : public IDTMSourceCreator
    {
#ifdef SCALABLE_MESH_DGN
        virtual IDTMSource*     _Create(SourceDataSQLite&      sourceData,
            const DocumentEnv&  env) const
        {
            DTMSourceDataType   sourceType;
            IMonikerPtr         monikerPtr;
            uint32_t              modelID;
            WString             modelName;
            WString             referencePersistantPath;
            WString             referenceName;
            uint32_t              levelID;
            WString             levelName;


            if (LoadDGNV1LevelPart(sourceData, levelID, levelName) &&
                LoadDGNV0ReferencePart(sourceData, referencePersistantPath, referenceName) &&
                LoadDGNV1ModelPart(sourceData, modelID, modelName) &&
                LoadSourcePart(sourceData, env, sourceType, monikerPtr))
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
#else
    virtual IDTMSource*     _Create                    (BinaryIStream&      stream,
                                                            const DocumentEnv&  env) const
        {
        DTMSourceDataType   sourceType;
        IMonikerPtr         monikerPtr;
        uint32_t              modelID;
        WString             modelName;
        WString             referencePersistantPath;
        WString             referenceName;
        uint32_t              levelID;
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
#endif
    } s_dgnReferenceLevelSourceCreatorV0;


struct IDTMDgnReferenceLevelSourceCreator : public IDTMSourceCreator
    {
#ifdef SCALABLE_MESH_DGN
        virtual IDTMSource*     _Create(SourceDataSQLite&      sourceData,
            const DocumentEnv&  env) const
        {
            DTMSourceDataType   sourceType;
            IMonikerPtr         monikerPtr;
            uint32_t              modelID;
            WString             modelName;
            WString             referencePersistantPath;
            WString             referenceName;
            WString             referenceModelName;
            uint32_t              levelID;
            WString             levelName;


            if (LoadDGNV1LevelPart(sourceData, levelID, levelName) &&
                LoadDGNV1ReferencePart(sourceData, referencePersistantPath, referenceName, referenceModelName) &&
                LoadDGNV1ModelPart(sourceData, modelID, modelName) &&
                LoadSourcePart(sourceData, env, sourceType, monikerPtr))
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
#else
    virtual IDTMSource*     _Create                    (BinaryIStream&      stream,
                                                            const DocumentEnv&  env) const
        {
        DTMSourceDataType   sourceType;
        IMonikerPtr         monikerPtr;
        uint32_t              modelID;
        WString             modelName;
        WString             referencePersistantPath;
        WString             referenceName;
        WString             referenceModelName;
        uint32_t              levelID;
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
#endif
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
#ifdef SCALABLE_MESH_DGN
IDTMSource* IDTMSourceFactory::Create(SourceDataSQLite&          sourceData,
    const DocumentEnv&        env) const
{
    const UInt sourceIDField = sourceData.GetDTMSourceID();

    if (DTM_SOURCE_ID_QTY <= sourceIDField)
        return 0;

    DTMSourceId sourceID = static_cast<DTMSourceId>(sourceIDField);

    CreatorItem creatorP = GetCreatorIndex()[sourceID];
    if (0 == creatorP)
        return 0;

    return creatorP->_Create(sourceData, env);
}
#else
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
#endif


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef SCALABLE_MESH_DGN
bool SourceSerializer::Serialize(const IDTMSource&   source,
    const DocumentEnv&      env,
    SourceDataSQLite&          sourceData) const
{
    SourceVisitor visitor(sourceData);

    source.Accept(visitor);

    const IDTMSourceSerializer* serializerP = visitor.GetFoundSerializer();

    if (0 == serializerP)
        return false;

    return serializerP->_Serialize(source, env, sourceData);
}

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSourcePtr SourceSerializer::Deserialize(SourceDataSQLite&          sourceData,
    const DocumentEnv&      env,
    UInt                    formatVersion) const
{
    if (SourceSerializer::FORMAT_VERSION != formatVersion)
    {
        assert(!"Invalid version! Need to handle backward compatibility!");
        return 0;
    }

    static const IDTMSourceFactory SOURCE_FACTORY;

    return SOURCE_FACTORY.Create(sourceData, env);
}
#else
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
#endif


END_BENTLEY_SCALABLEMESH_NAMESPACE
