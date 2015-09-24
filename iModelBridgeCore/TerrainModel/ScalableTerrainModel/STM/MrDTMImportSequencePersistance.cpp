/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMImportSequencePersistance.cpp $
|    $RCSfile: MrDTMImportSequencePersistance.cpp,v $
|   $Revision: 1.10 $
|       $Date: 2012/02/16 22:19:29 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include "MrDTMImportSequencePersistance.h"

#include <ScalableTerrainModel/Import/ImportSequenceVisitor.h>
#include <ScalableTerrainModel/Import/Command/All.h>

#include <ScalableTerrainModel/Type/IMrDTMLinear.h>
#include <ScalableTerrainModel/Type/IMrDTMPoint.h>
#include <ScalableTerrainModel/Type/IMrDTMTIN.h>
#include <ScalableTerrainModel/Type/IMrDTMMesh.h>

#include <ScalableTerrainModel/IMrDTMStream.h>


USING_NAMESPACE_BENTLEY_MRDTM_IMPORT


BEGIN_BENTLEY_MRDTM_NAMESPACE

/*
 * Driver current version
 */ 
const UInt ImportSequenceSerializer::FORMAT_VERSION = 0;


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
    BinaryOStream&                  m_stream;
    bool                            m_success;

    bool                            OutputCommandID            (CommandSerializationID                      id)
        {
        assert(id < CSID_QTY);
        return m_stream.put(static_cast<byte>(id)).good();
        }

    bool                            OutputLayer                (UInt                                        layer)
        {
        const uint32_t layerField = layer;
        return WriteValue(m_stream, layerField);
        }

    bool                            OutputType                 (const DataTypeFamily&                       type)
        {
        static const DataTypeFamily POINT_TYPE_FAMILY(PointTypeFamilyCreator().Create());
        static const DataTypeFamily LINEAR_TYPE_FAMILY(LinearTypeFamilyCreator().Create());
        static const DataTypeFamily TIN_TYPE_FAMILY(TINTypeFamilyCreator().Create());
        static const DataTypeFamily MESH_TYPE_FAMILY(MeshTypeFamilyCreator().Create());

        bool success = true;

        if (POINT_TYPE_FAMILY == type)
            m_stream.put(static_cast<byte>(TFID_POINT));
        else if (LINEAR_TYPE_FAMILY == type)
            m_stream.put(static_cast<byte>(TFID_LINEAR));
        else if (TIN_TYPE_FAMILY == type)
            m_stream.put(static_cast<byte>(TFID_TIN));
        else if (MESH_TYPE_FAMILY == type)
            m_stream.put(static_cast<byte>(TFID_MESH));
        else
            success = false;

        return success && m_stream.good();
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
    explicit                        CmdVisitor                 (BinaryOStream&                              stream)
        :   m_stream(stream),
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
    
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const = 0;
protected:
    virtual                     ~CmdCreator                () = 0 {}


    const DataTypeFamily*       LoadType                   (BinaryIStream&      stream) const
        {
        static const DataTypeFamily POINT_TYPE_FAMILY(PointTypeFamilyCreator().Create());
        static const DataTypeFamily LINEAR_TYPE_FAMILY(LinearTypeFamilyCreator().Create());
        static const DataTypeFamily TIN_TYPE_FAMILY(TINTypeFamilyCreator().Create());
        static const DataTypeFamily MESH_TYPE_FAMILY(MeshTypeFamilyCreator().Create());

        const UInt DataTypeFamilyField = stream.get();
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

    UInt                        LoadLayer                  (BinaryIStream&      stream) const
        {
        uint32_t layerField = INVALID_LAYER;
        ReadValue(stream, layerField);
        return layerField;
        }

    static const UInt           INVALID_LAYER;
    };

const UInt CmdCreator::INVALID_LAYER(numeric_limits<UInt>::max());

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
    ImportCommandBase*          Create                     (BinaryIStream&      stream) const;
};



struct ImportAllCommandCreator : CmdCreator
    {
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        return new ImportAllCommand();
        }
    } s_ImportAllCommandCreator;

struct ImportAllToLayerCommandCreator : CmdCreator
    {
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const UInt targetLayer = LoadLayer(stream);
        if (INVALID_LAYER == targetLayer)
            return 0;

        return new ImportAllToLayerCommand(targetLayer);
        }
    } s_ImportAllToLayerCommandCreator;

struct ImportAllToLayerTypeCommandCreator : CmdCreator
{
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const UInt targetLayer = LoadLayer(stream);
        const DataTypeFamily* targetTypeP = LoadType(stream);

        if (INVALID_LAYER == targetLayer || 
            0 == targetTypeP)
            return 0;

        return new ImportAllToLayerTypeCommand(targetLayer, *targetTypeP);
        }
    } s_ImportAllToLayerTypeCommandCreator;

struct ImportAllToTypeCommandCreator : CmdCreator
    {
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const DataTypeFamily* targetTypeP = LoadType(stream);
        if (0 == targetTypeP)
            return 0;

        return new ImportAllToTypeCommand(*targetTypeP);
        }
    } s_ImportAllToTypeCommandCreator;

struct ImportLayerCommandCreator : CmdCreator
    {
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const UInt sourceLayer = LoadLayer(stream);
        if (INVALID_LAYER == sourceLayer)
            return 0;

        return new ImportLayerCommand(sourceLayer);
        }
    } s_ImportLayerCommandCreator;

struct ImportLayerToLayerCommandCreator : CmdCreator
    {
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const UInt sourceLayer = LoadLayer(stream);
        const UInt targetLayer = LoadLayer(stream);
        if (INVALID_LAYER == sourceLayer ||
            INVALID_LAYER == targetLayer)
            return 0;

        return new ImportLayerToLayerCommand(sourceLayer, targetLayer);
        }
    } s_ImportLayerToLayerCommandCreator;

struct ImportLayerToLayerTypeCommandCreator : CmdCreator
    {
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const UInt sourceLayer = LoadLayer(stream);
        const UInt targetLayer = LoadLayer(stream);
        const DataTypeFamily* targetTypeP = LoadType(stream);

        if (INVALID_LAYER == sourceLayer ||
            INVALID_LAYER == targetLayer ||
            0 == targetTypeP)
            return 0;

        return new ImportLayerToLayerTypeCommand(sourceLayer, targetLayer, *targetTypeP);
        }
    } s_ImportLayerToLayerTypeCommandCreator;

struct ImportLayerToTypeCommandCreator : CmdCreator
    {
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const UInt sourceLayer = LoadLayer(stream);
        const DataTypeFamily* targetTypeP = LoadType(stream);

        if (INVALID_LAYER == sourceLayer ||
            0 == targetTypeP)
            return 0;

        return new ImportLayerToTypeCommand(sourceLayer, *targetTypeP);
        }
    } s_ImportLayerToTypeCommandCreator;

struct ImportLayerTypeCommandCreator : CmdCreator
    {
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const UInt sourceLayer = LoadLayer(stream);
        const DataTypeFamily* sourceTypeP = LoadType(stream);

        if (INVALID_LAYER == sourceLayer ||
            0 == sourceTypeP)
            return 0;

        return new ImportLayerTypeCommand(sourceLayer, *sourceTypeP);
        }
    } s_ImportLayerTypeCommandCreator;

struct ImportLayerTypeToLayerCommandCreator : CmdCreator
    {
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const UInt sourceLayer = LoadLayer(stream);
        const DataTypeFamily* sourceTypeP = LoadType(stream);
        const UInt targetLayer = LoadLayer(stream);

        if (INVALID_LAYER == sourceLayer ||
            0 == sourceTypeP ||
            INVALID_LAYER == targetLayer)
            return 0;

        return new ImportLayerTypeToLayerCommand(sourceLayer, *sourceTypeP, targetLayer);
        }
    } s_ImportLayerTypeToLayerCommandCreator;

struct ImportLayerTypeToLayerTypeCommandCreator : CmdCreator
    {
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const UInt sourceLayer = LoadLayer(stream);
        const DataTypeFamily* sourceTypeP = LoadType(stream);
        const UInt targetLayer = LoadLayer(stream);
        const DataTypeFamily* targetTypeP = LoadType(stream);

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
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const UInt sourceLayer = LoadLayer(stream);
        const DataTypeFamily* sourceTypeP = LoadType(stream);
        const DataTypeFamily* targetTypeP = LoadType(stream);

        if (INVALID_LAYER == sourceLayer ||
            0 == sourceTypeP ||
            0 == targetTypeP)
            return 0;

        return new ImportLayerTypeToTypeCommand(sourceLayer, *sourceTypeP, *targetTypeP);
        }
    } s_ImportLayerTypeToTypeCommandCreator;

struct ImportTypeCommandCreator : CmdCreator
    {
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const DataTypeFamily* sourceTypeP = LoadType(stream);
        if (0 == sourceTypeP)
            return 0;

        return new ImportTypeCommand(*sourceTypeP);
        }
    } s_ImportTypeCommandCreator;

struct ImportTypeToLayerCommandCreator : CmdCreator
    {
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const DataTypeFamily* sourceTypeP = LoadType(stream);
        const UInt targetLayer = LoadLayer(stream);

        if (0 == sourceTypeP ||
            INVALID_LAYER == targetLayer)
            return 0;

        return new ImportTypeToLayerCommand(*sourceTypeP, targetLayer);
        }
    } s_ImportTypeToLayerCommandCreator;

struct ImportTypeToLayerTypeCommandCreator : CmdCreator
    {
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const DataTypeFamily* sourceTypeP = LoadType(stream);
        const UInt targetLayer = LoadLayer(stream);
        const DataTypeFamily* targetTypeP = LoadType(stream);

        if (0 == sourceTypeP ||
            INVALID_LAYER == targetLayer ||
            0 == targetTypeP)
            return 0;

        return new ImportTypeToLayerTypeCommand(*sourceTypeP, targetLayer, *targetTypeP);
        }
    } s_ImportTypeToLayerTypeCommandCreator;

struct ImportTypeToTypeCommandCreator : CmdCreator
    {
    virtual ImportCommandBase*  _Create                    (BinaryIStream&      stream) const override
        {
        const DataTypeFamily* sourceTypeP = LoadType(stream);
        const DataTypeFamily* targetTypeP = LoadType(stream);

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
ImportCommandBase* CmdFactory::Create (BinaryIStream& stream) const
    {
    const UInt commandIDField = stream.get();

    if (CSID_QTY <= commandIDField)
        return 0;

    CommandSerializationID commandID = static_cast<CommandSerializationID>(commandIDField);

    CreatorItem creatorP = GetCreatorIndex()[commandID];
    assert(0 != creatorP);
    
    return creatorP->_Create(stream);
    }


} // END unnamed namespace







/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImportSequenceSerializer::Serialize   (const ImportSequence&   sequence,
                                            BinaryOStream&          stream) const
    {
    const uint32_t commandCountField = (uint32_t)sequence.GetCount();

    if (0 == commandCountField)
        return true;// Generate empty packet so that it improves load performances

    CmdVisitor visitor(stream);

    WriteValue(stream, commandCountField);
    sequence.Accept(visitor);

    return visitor.Succeded() && stream.good();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImportSequenceSerializer::Deserialize (BinaryIStream&      stream,
                                            ImportSequence&     sequence,
                                            UInt                formatVersion) const
    {
    if (ImportSequenceSerializer::FORMAT_VERSION != formatVersion)
        {
        assert(!"Invalid version! Need to handle backward compatibility!");
        return false;
        }

    uint32_t commandCountField = 0;
    if (!ReadValue(stream, commandCountField))
        return false;

    static const CmdFactory COMMAND_FACTORY;

    for (uint32_t i = 0; i < commandCountField; ++i)
        {
        RefCountedPtr<ImportCommandBase> commandP(COMMAND_FACTORY.Create(stream));

        if (0 == commandP.get())
            return false;
        if (!stream.good())
            return false;

        sequence.push_back(*commandP);
        }

    return true;
    }





END_BENTLEY_MRDTM_NAMESPACE