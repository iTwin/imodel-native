/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshImportSequencePersistance.cpp $
|    $RCSfile: ScalableMeshImportSequencePersistance.cpp,v $
|   $Revision: 1.10 $
|       $Date: 2012/02/16 22:19:29 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
#include "ScalableMeshImportSequencePersistance.h"

#include <ScalableMesh/Import/ImportSequenceVisitor.h>
#include <ScalableMesh/Import/Command/All.h>

#include <ScalableMesh/Type/IScalableMeshLinear.h>
#include <ScalableMesh/Type/IScalableMeshPoint.h>
#include <ScalableMesh/Type/IScalableMeshTIN.h>
#include <ScalableMesh/Type/IScalableMeshMesh.h>

#include <ScalableMesh/IScalableMeshStream.h>


USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

       
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*
 * Driver current version
 */ 
const uint32_t ImportSequenceSerializer::FORMAT_VERSION = 0;


namespace { // Unnamed namespace


enum CommandSerializationID
    {
    CSID_ImportAllCommand,
    CSID_ImportAllToLayerCommand,
    CSID_ImportAllToLayerTypeCommand,
    CSID_ImportAllToTypeCommand,

    CSID_ImportLayerCommand,
    CSID_ImportLayerToLayerCommand,
    CSID_ImportLayerToLayerTypeCommand,
    CSID_ImportLayerToTypeCommand,

    CSID_ImportLayerTypeCommand,
    CSID_ImportLayerTypeToLayerCommand,
    CSID_ImportLayerTypeToLayerTypeCommand,
    CSID_ImportLayerTypeToTypeCommand,

    CSID_ImportTypeCommand,
    CSID_ImportTypeToLayerCommand,
    CSID_ImportTypeToLayerTypeCommand,
    CSID_ImportTypeToTypeCommand,

    CSID_QTY,
    };

enum TypeFamilyID
    {
    TFID_POINT,
    TFID_LINEAR,
    TFID_TIN,
    TFID_MESH,

    TFID_QTY,
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class CmdVisitor : public IImportSequenceVisitor
    {

        SourceDataSQLite&               m_sourceData;
        bool                            m_success;

        bool                            OutputCommandID(CommandSerializationID                      id)
        {
            assert(id < CSID_QTY);
            m_sourceData.AddCommandID(id);
            return true;
            //return m_stream.put(static_cast<byte>(id)).good();
        }

        bool                            OutputLayer(uint32_t                                        layer)
        {
            const uint32_t layerField = layer;
            m_sourceData.SetLayer(layerField);
            return true;
            //return WriteValue(m_stream, layerField);
        }

        bool                            OutputType(const DataTypeFamily&                       type)
        {
            static const DataTypeFamily POINT_TYPE_FAMILY(PointTypeFamilyCreator().Create());
            static const DataTypeFamily LINEAR_TYPE_FAMILY(LinearTypeFamilyCreator().Create());
            static const DataTypeFamily TIN_TYPE_FAMILY(TINTypeFamilyCreator().Create());
            static const DataTypeFamily MESH_TYPE_FAMILY(MeshTypeFamilyCreator().Create());

            bool success = true;

            if (POINT_TYPE_FAMILY == type)
                m_sourceData.SetTypeFamilyID(static_cast<byte>(TFID_POINT));
            else if (LINEAR_TYPE_FAMILY == type)
                m_sourceData.SetTypeFamilyID(static_cast<byte>(TFID_LINEAR));
            else if (TIN_TYPE_FAMILY == type)
                m_sourceData.SetTypeFamilyID(static_cast<byte>(TFID_TIN));
            else if (MESH_TYPE_FAMILY == type)
                m_sourceData.SetTypeFamilyID(static_cast<byte>(TFID_MESH));
            else
                success = false;

            return success;
        }


    virtual void                    _Visit                     (const ImportAllCommand&                     command) override
        {
        m_success &= OutputCommandID(CSID_ImportAllCommand);
        }
    virtual void                    _Visit                     (const ImportAllToLayerCommand&              command) override
        {
        m_success &= OutputCommandID(CSID_ImportAllToLayerCommand);
        m_success &= OutputLayer(command.GetTargetLayer());
        }
    virtual void                    _Visit                     (const ImportAllToLayerTypeCommand&          command) override
        {
        m_success &= OutputCommandID(CSID_ImportAllToLayerTypeCommand);
        m_success &= OutputLayer(command.GetTargetLayer());
        m_success &= OutputType(command.GetTargetType());
        }
    virtual void                    _Visit                     (const ImportAllToTypeCommand&               command) override
        {
        m_success &= OutputCommandID(CSID_ImportAllToTypeCommand);
        m_success &= OutputType(command.GetTargetType());
        }

    virtual void                    _Visit                     (const ImportLayerCommand&                   command) override
        {
        m_success &= OutputCommandID(CSID_ImportLayerCommand);
        m_success &= OutputLayer(command.GetSourceLayer());
        }
    virtual void                    _Visit                     (const ImportLayerToLayerCommand&            command) override
        {
        m_success &= OutputCommandID(CSID_ImportLayerToLayerCommand);
        m_success &= OutputLayer(command.GetSourceLayer());
        m_success &= OutputLayer(command.GetTargetLayer());
        }
    virtual void                    _Visit                     (const ImportLayerToLayerTypeCommand&        command) override
        {
        m_success &= OutputCommandID(CSID_ImportLayerToLayerTypeCommand);
        m_success &= OutputLayer(command.GetSourceLayer());
        m_success &= OutputLayer(command.GetTargetLayer());
        m_success &= OutputType(command.GetTargetType());
        }
    virtual void                    _Visit                     (const ImportLayerToTypeCommand&             command) override
        {
        m_success &= OutputCommandID(CSID_ImportLayerToTypeCommand);
        m_success &= OutputLayer(command.GetSourceLayer());
        m_success &= OutputType(command.GetTargetType());
        }

    virtual void                    _Visit                     (const ImportLayerTypeCommand&               command) override
        {
        m_success &= OutputCommandID(CSID_ImportLayerTypeCommand);
        m_success &= OutputLayer(command.GetSourceLayer());
        m_success &= OutputType(command.GetSourceType());
        }
    virtual void                    _Visit                     (const ImportLayerTypeToLayerCommand&        command) override
        {
        m_success &= OutputCommandID(CSID_ImportLayerTypeToLayerCommand);
        m_success &= OutputLayer(command.GetSourceLayer());
        m_success &= OutputType(command.GetSourceType());
        m_success &= OutputLayer(command.GetTargetLayer());
        }
    virtual void                    _Visit                     (const ImportLayerTypeToLayerTypeCommand&    command) override
        {
        m_success &= OutputCommandID(CSID_ImportLayerTypeToLayerTypeCommand);
        m_success &= OutputLayer(command.GetSourceLayer());
        m_success &= OutputType(command.GetSourceType());
        m_success &= OutputLayer(command.GetTargetLayer());
        m_success &= OutputType(command.GetTargetType());
        }
    virtual void                    _Visit                     (const ImportLayerTypeToTypeCommand&         command) override
        {
        m_success &= OutputCommandID(CSID_ImportLayerTypeToTypeCommand);
        m_success &= OutputLayer(command.GetSourceLayer());
        m_success &= OutputType(command.GetSourceType());
        m_success &= OutputType(command.GetTargetType());
        }

    virtual void                    _Visit                     (const ImportTypeCommand&                    command) override
        {
        m_success &= OutputCommandID(CSID_ImportTypeCommand);
        m_success &= OutputType(command.GetSourceType());
        }
    virtual void                    _Visit                     (const ImportTypeToLayerCommand&             command) override
        {
        m_success &= OutputCommandID(CSID_ImportTypeToLayerCommand);
        m_success &= OutputType(command.GetSourceType());
        m_success &= OutputLayer(command.GetTargetLayer());
        }
    virtual void                    _Visit                     (const ImportTypeToLayerTypeCommand&         command) override
        {
        m_success &= OutputCommandID(CSID_ImportTypeToLayerTypeCommand);
        m_success &= OutputType(command.GetSourceType());
        m_success &= OutputLayer(command.GetTargetLayer());
        m_success &= OutputType(command.GetTargetType());
        }
    virtual void                    _Visit                     (const ImportTypeToTypeCommand&              command) override
        {
        m_success &= OutputCommandID(CSID_ImportTypeToTypeCommand);
        m_success &= OutputType(command.GetSourceType());
        m_success &= OutputType(command.GetTargetType());
        }

public:

    explicit                        CmdVisitor(SourceDataSQLite&                              sourceData)
        : m_sourceData(sourceData),
        m_success(true)
    {
    }


    bool                            Succeded                   () const { return m_success; }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct CmdCreator
    {
private:
    friend struct               CmdFactory;

    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const = 0;

protected:
    virtual                     ~CmdCreator                () = 0 {}


    const DataTypeFamily*       LoadType(SourceDataSQLite&      sourceData) const
    {
        static const DataTypeFamily POINT_TYPE_FAMILY(PointTypeFamilyCreator().Create());
        static const DataTypeFamily LINEAR_TYPE_FAMILY(LinearTypeFamilyCreator().Create());
        static const DataTypeFamily TIN_TYPE_FAMILY(TINTypeFamilyCreator().Create());
        static const DataTypeFamily MESH_TYPE_FAMILY(MeshTypeFamilyCreator().Create());

        const uint32_t DataTypeFamilyField = sourceData.GetTypeFamilyID();
        switch (DataTypeFamilyField)
        {
        case TFID_POINT:
            return &POINT_TYPE_FAMILY;
        case TFID_LINEAR:
            return &LINEAR_TYPE_FAMILY;
        case TFID_TIN:
            return &TIN_TYPE_FAMILY;
        case TFID_MESH:
            return &MESH_TYPE_FAMILY;
        default:
            return 0;
        }
    }

    uint32_t                        LoadLayer(SourceDataSQLite&      sourceData) const
    {
        uint32_t layerField = sourceData.GetLayer();
        return layerField;
    }


    static const uint32_t           INVALID_LAYER;
    };

const uint32_t CmdCreator::INVALID_LAYER(numeric_limits<uint32_t>::max());

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct CmdFactory
    {
private:
    typedef const CmdCreator*   CreatorItem;

    static const CreatorItem*   GetCreatorIndex            ();

public:
    ImportCommandBase*          Create(SourceDataSQLite&      sourceData) const;

};



struct ImportAllCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        return new ImportAllCommand();
    }
} s_ImportAllCommandCreator;

struct ImportAllToLayerCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const uint32_t targetLayer = LoadLayer(sourceData);
        if (INVALID_LAYER == targetLayer)
            return 0;

        return new ImportAllToLayerCommand(targetLayer);
    }
} s_ImportAllToLayerCommandCreator;

struct ImportAllToLayerTypeCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const uint32_t targetLayer = LoadLayer(sourceData);
        const DataTypeFamily* targetTypeP = LoadType(sourceData);

        if (INVALID_LAYER == targetLayer ||
            0 == targetTypeP)
            return 0;

        return new ImportAllToLayerTypeCommand(targetLayer, *targetTypeP);
    }
} s_ImportAllToLayerTypeCommandCreator;

struct ImportAllToTypeCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const DataTypeFamily* targetTypeP = LoadType(sourceData);
        if (0 == targetTypeP)
            return 0;

        return new ImportAllToTypeCommand(*targetTypeP);
    }
} s_ImportAllToTypeCommandCreator;

struct ImportLayerCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const uint32_t sourceLayer = LoadLayer(sourceData);
        if (INVALID_LAYER == sourceLayer)
            return 0;

        return new ImportLayerCommand(sourceLayer);
    }
} s_ImportLayerCommandCreator;

struct ImportLayerToLayerCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const uint32_t sourceLayer = LoadLayer(sourceData);
        const uint32_t targetLayer = LoadLayer(sourceData);
        if (INVALID_LAYER == sourceLayer ||
            INVALID_LAYER == targetLayer)
            return 0;

        return new ImportLayerToLayerCommand(sourceLayer, targetLayer);
    }
} s_ImportLayerToLayerCommandCreator;

struct ImportLayerToLayerTypeCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const uint32_t sourceLayer = LoadLayer(sourceData);
        const uint32_t targetLayer = LoadLayer(sourceData);
        const DataTypeFamily* targetTypeP = LoadType(sourceData);

        if (INVALID_LAYER == sourceLayer ||
            INVALID_LAYER == targetLayer ||
            0 == targetTypeP)
            return 0;

        return new ImportLayerToLayerTypeCommand(sourceLayer, targetLayer, *targetTypeP);
    }
} s_ImportLayerToLayerTypeCommandCreator;

struct ImportLayerToTypeCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const uint32_t sourceLayer = LoadLayer(sourceData);
        const DataTypeFamily* targetTypeP = LoadType(sourceData);

        if (INVALID_LAYER == sourceLayer ||
            0 == targetTypeP)
            return 0;

        return new ImportLayerToTypeCommand(sourceLayer, *targetTypeP);
    }
} s_ImportLayerToTypeCommandCreator;

struct ImportLayerTypeCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const uint32_t sourceLayer = LoadLayer(sourceData);
        const DataTypeFamily* sourceTypeP = LoadType(sourceData);

        if (INVALID_LAYER == sourceLayer ||
            0 == sourceTypeP)
            return 0;

        return new ImportLayerTypeCommand(sourceLayer, *sourceTypeP);
    }
} s_ImportLayerTypeCommandCreator;

struct ImportLayerTypeToLayerCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const uint32_t sourceLayer = LoadLayer(sourceData);
        const DataTypeFamily* sourceTypeP = LoadType(sourceData);
        const uint32_t targetLayer = LoadLayer(sourceData);

        if (INVALID_LAYER == sourceLayer ||
            0 == sourceTypeP ||
            INVALID_LAYER == targetLayer)
            return 0;

        return new ImportLayerTypeToLayerCommand(sourceLayer, *sourceTypeP, targetLayer);
    }
} s_ImportLayerTypeToLayerCommandCreator;

struct ImportLayerTypeToLayerTypeCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const uint32_t sourceLayer = LoadLayer(sourceData);
        const DataTypeFamily* sourceTypeP = LoadType(sourceData);
        const uint32_t targetLayer = LoadLayer(sourceData);
        const DataTypeFamily* targetTypeP = LoadType(sourceData);

        if (INVALID_LAYER == sourceLayer ||
            0 == sourceTypeP ||
            INVALID_LAYER == targetLayer ||
            0 == targetTypeP)
            return 0;

        return new ImportLayerTypeToLayerTypeCommand(sourceLayer, *sourceTypeP, targetLayer, *targetTypeP);
    }
} s_ImportLayerTypeToLayerTypeCommandCreator;

struct ImportLayerTypeToTypeCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const uint32_t sourceLayer = LoadLayer(sourceData);
        const DataTypeFamily* sourceTypeP = LoadType(sourceData);
        const DataTypeFamily* targetTypeP = LoadType(sourceData);

        if (INVALID_LAYER == sourceLayer ||
            0 == sourceTypeP ||
            0 == targetTypeP)
            return 0;

        return new ImportLayerTypeToTypeCommand(sourceLayer, *sourceTypeP, *targetTypeP);
    }
} s_ImportLayerTypeToTypeCommandCreator;

struct ImportTypeCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const DataTypeFamily* sourceTypeP = LoadType(sourceData);
        if (0 == sourceTypeP)
            return 0;

        return new ImportTypeCommand(*sourceTypeP);
    }
} s_ImportTypeCommandCreator;

struct ImportTypeToLayerCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const DataTypeFamily* sourceTypeP = LoadType(sourceData);
        const uint32_t targetLayer = LoadLayer(sourceData);

        if (0 == sourceTypeP ||
            INVALID_LAYER == targetLayer)
            return 0;

        return new ImportTypeToLayerCommand(*sourceTypeP, targetLayer);
    }
} s_ImportTypeToLayerCommandCreator;

struct ImportTypeToLayerTypeCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const DataTypeFamily* sourceTypeP = LoadType(sourceData);
        const uint32_t targetLayer = LoadLayer(sourceData);
        const DataTypeFamily* targetTypeP = LoadType(sourceData);

        if (0 == sourceTypeP ||
            INVALID_LAYER == targetLayer ||
            0 == targetTypeP)
            return 0;

        return new ImportTypeToLayerTypeCommand(*sourceTypeP, targetLayer, *targetTypeP);
    }
} s_ImportTypeToLayerTypeCommandCreator;

struct ImportTypeToTypeCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create(SourceDataSQLite&      sourceData) const override
    {
        const DataTypeFamily* sourceTypeP = LoadType(sourceData);
        const DataTypeFamily* targetTypeP = LoadType(sourceData);

        if (0 == sourceTypeP ||
            0 == targetTypeP)
            return 0;

        return new ImportTypeToTypeCommand(*sourceTypeP, *targetTypeP);
    }
} s_ImportTypeToTypeCommandCreator;




/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const CmdFactory::CreatorItem* CmdFactory::GetCreatorIndex ()
    {
    static CreatorItem      CREATORS_INDEX[CSID_QTY] = 
        {
        &s_ImportAllCommandCreator,
        &s_ImportAllToLayerCommandCreator,
        &s_ImportAllToLayerTypeCommandCreator,
        &s_ImportAllToTypeCommandCreator,

        &s_ImportLayerCommandCreator,
        &s_ImportLayerToLayerCommandCreator,
        &s_ImportLayerToLayerTypeCommandCreator,
        &s_ImportLayerToTypeCommandCreator,

        &s_ImportLayerTypeCommandCreator,
        &s_ImportLayerTypeToLayerCommandCreator,
        &s_ImportLayerTypeToLayerTypeCommandCreator,
        &s_ImportLayerTypeToTypeCommandCreator,

        &s_ImportTypeCommandCreator,
        &s_ImportTypeToLayerCommandCreator,
        &s_ImportTypeToLayerTypeCommandCreator,
        &s_ImportTypeToTypeCommandCreator,
        };
    return CREATORS_INDEX;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportCommandBase* CmdFactory::Create(SourceDataSQLite& sourceData) const
{
    const uint32_t commandIDField = sourceData.PopCommandID();

    if (CSID_QTY <= commandIDField)
        return 0;

    CommandSerializationID commandID = static_cast<CommandSerializationID>(commandIDField);

    CreatorItem creatorP = GetCreatorIndex()[commandID];
    assert(0 != creatorP);

    return creatorP->_Create(sourceData);
}



} // END unnamed namespace







/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImportSequenceSerializer::Serialize(const ImportSequence&   sequence,
    SourceDataSQLite&          sourceData) const
{
    const uint32_t commandCountField = (uint32_t)sequence.GetCount();

    if (0 == commandCountField)
        return true;// Generate empty packet so that it improves load performances

    CmdVisitor visitor(sourceData);

    //WriteValue(sourceData, commandCountField);
    sourceData.SetCommandCount(commandCountField);
    sequence.Accept(visitor);

    return visitor.Succeded();
}


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImportSequenceSerializer::Deserialize(SourceDataSQLite&      sourceData,
    ImportSequence&     sequence,
    uint32_t                formatVersion) const
{
    if (ImportSequenceSerializer::FORMAT_VERSION != formatVersion)
    {
        assert(!"Invalid version! Need to handle backward compatibility!");
        return false;
    }

    uint32_t commandCountField = sourceData.GetCommandCount();

    static const CmdFactory COMMAND_FACTORY;

    for (uint32_t i = 0; i < commandCountField; ++i)
    {
        RefCountedPtr<ImportCommandBase> commandP(COMMAND_FACTORY.Create(sourceData));

        if (0 == commandP.get())
            return false;

        sequence.push_back(*commandP);
    }

    return true;
}





END_BENTLEY_SCALABLEMESH_NAMESPACE
