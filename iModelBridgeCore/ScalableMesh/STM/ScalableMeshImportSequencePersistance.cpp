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

#include <ScalableMesh/Import/Command/Base.h>

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

   /* uint32_t                        LoadLayer(SourceDataSQLite&      sourceData) const
    {
        uint32_t layerField = sourceData.GetLayer();
        return layerField;
    }*/


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


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const CmdFactory::CreatorItem* CmdFactory::GetCreatorIndex ()
    {

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportCommandBase* CmdFactory::Create(SourceDataSQLite& sourceData) const
{

assert(!"Not supposed to use CommandFactory!");
return nullptr;
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

    bvector<ImportCommandData> serializedCommands;
    for (auto& command : sequence.GetCommands())
        {
        ImportCommandData data(command);
        serializedCommands.push_back(data);
        }
    sourceData.SetOrderedCommands(serializedCommands);

    return true;// visitor.Succeded();
}

const DataTypeFamily*       LoadType(uint32_t id)
    {
    static const DataTypeFamily POINT_TYPE_FAMILY(PointTypeFamilyCreator().Create());
    static const DataTypeFamily LINEAR_TYPE_FAMILY(LinearTypeFamilyCreator().Create());
    static const DataTypeFamily TIN_TYPE_FAMILY(TINTypeFamilyCreator().Create());
    static const DataTypeFamily MESH_TYPE_FAMILY(MeshTypeFamilyCreator().Create());

    switch (id)
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

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImportSequenceSerializer::Deserialize(SourceDataSQLite&      sourceData,
    ImportSequence&     sequence,
    uint32_t                formatVersion) const
{
bvector<ImportCommandData>& data = sourceData.GetOrderedCommands();
for (auto& command : data)
    {
    RefCountedPtr<ImportCommandBase> commandP(new ImportCommandBase());

    if (0 == commandP.get())
        return false;
    if (command.sourceLayerSet) commandP->SetSourceLayer(command.sourceLayerID);
    if (command.targetLayerSet) commandP->SetTargetLayer(command.targetLayerID);
    if (command.sourceTypeSet) commandP->SetSourceType(*LoadType(command.sourceTypeID));
    if (command.targetTypeSet) commandP->SetTargetType(*LoadType(command.targetTypeID));
    sequence.push_back(*commandP);
    }

    return true;
}





END_BENTLEY_SCALABLEMESH_NAMESPACE
