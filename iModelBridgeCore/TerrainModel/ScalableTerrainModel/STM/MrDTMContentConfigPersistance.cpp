/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMContentConfigPersistance.cpp $
|    $RCSfile: MrDTMContentConfigPersistance.cpp,v $
|   $Revision: 1.19 $
|       $Date: 2012/02/16 22:19:31 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>


#include "InternalUtilityFunctions.h"

#include "MrDTMContentConfigPersistance.h"

#include <ScalableTerrainModel/GeoCoords/GCS.h>

#include <ScalableTerrainModel/Import/ContentConfigVisitor.h>
#include <ScalableTerrainModel/Import/Config/Content/All.h>
#include <ScalableTerrainModel/Import/DataTypeDescription.h>

#include <ScalableTerrainModel/Type/IMrDTMLinear.h>
#include <ScalableTerrainModel/Type/IMrDTMPoint.h>
#include <ScalableTerrainModel/Type/IMrDTMTIN.h>
#include <ScalableTerrainModel/Type/IMrDTMMesh.h>

#include <ScalableTerrainModel/IMrDTMStream.h>

#include <ScalableTerrainModel/IMrDTMPolicy.h>

#include <STMInternal/GeoCoords/WKTUtils.h>

USING_NAMESPACE_BENTLEY_MRDTM_GEOCOORDINATES
USING_NAMESPACE_BENTLEY_MRDTM_IMPORT

    

BEGIN_BENTLEY_MRDTM_NAMESPACE

/*
 * Driver current version
 */ 
const UInt ContentConfigSerializer::FORMAT_VERSION = 0;


namespace { // Unnamed namespace

    
//IMPORTANT : Never change the ID since it is save in the STM file. 
//New ID must be added at the end of the list just before CCSID_QTY.
enum ConfigComponentSerializationID
    {
    CCSID_GCS,
    CCSID_Type,
    CCSID_Layer,
    CCSID_REMOVED_Unit,
    CCSID_GCS_ExtendedV0,
    CCSID_GCS_ExtendedV2, 
    CCSID_GCS_ExtendedV1,
    CCSID_GCS_LocalAdjustment,        
    CCSID_QTY,
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ComponentSerializerBase
    {
private:
    friend class                SerializeCommand;

    virtual bool                _Serialize                                 (const ContentConfigComponentBase&   component,
                                                                            BinaryOStream&                      stream) const = 0;

public:
    virtual                     ~ComponentSerializerBase      () = 0 {}

    virtual                     ConfigComponentSerializationID GetSerializationID() const = 0;    

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
struct ComponentSerializerMixinBase : public ComponentSerializerBase
    {
private:
    virtual bool                _Serialize                                 (const ContentConfigComponentBase&   component,
                                                                            BinaryOStream&                      stream) const override
        {
        assert(0 != dynamic_cast<const ComponentT*>(&component));
        return _Serialize(static_cast<const ComponentT&>(component), stream);
        }
    virtual bool                _Serialize                                 (const ComponentT&                   component,
                                                                            BinaryOStream&                      stream) const = 0;    

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class SerializeCommand
    {
    const ComponentSerializerBase*      m_serializerP;
    const ContentConfigComponentBase*   m_componentP;
public:
    explicit                            SerializeCommand                   (const ComponentSerializerBase&      serializer,
                                                                            const ContentConfigComponentBase&   component)
        :   m_serializerP(&serializer),
            m_componentP(&component)
        {
        }

    bool                                Run                                (BinaryOStream&                      stream) const
        {
        return m_serializerP->_Serialize(*m_componentP, stream);
        }

     const ComponentSerializerBase*     GetComponentSerializer() const 
        {
        return m_serializerP;
        }
    };


typedef vector<SerializeCommand>    SerializeCommandList;


enum TypeFamilyID
    {
    TFID_POINT,
    TFID_LINEAR,
    TFID_TIN,
    TFID_MESH,
    TFID_QTY,
    };

bool                                OutputCommandID            (BinaryOStream&                              stream,
                                                                ConfigComponentSerializationID              id)
    {
    assert(id < CCSID_QTY);
    return stream.put(static_cast<byte>(id)).good();
    }


bool                                OutputTypeFamily           (BinaryOStream&                              stream,
                                                                const DataTypeFamily&                       type)
    {
    static const DataTypeFamily POINT_TYPE_FAMILY(PointTypeFamilyCreator().Create());
    static const DataTypeFamily LINEAR_TYPE_FAMILY(LinearTypeFamilyCreator().Create());
    static const DataTypeFamily TIN_TYPE_FAMILY(TINTypeFamilyCreator().Create());
    static const DataTypeFamily MESH_TYPE_FAMILY(MeshTypeFamilyCreator().Create());


    bool success = true;

    if (POINT_TYPE_FAMILY == type)
        stream.put(static_cast<byte>(TFID_POINT));
    else if (LINEAR_TYPE_FAMILY == type)
        stream.put(static_cast<byte>(TFID_POINT));
    else if (TIN_TYPE_FAMILY == type)
        stream.put(static_cast<byte>(TFID_TIN));
    else if (MESH_TYPE_FAMILY == type)
        stream.put(static_cast<byte>(TFID_MESH));
    else
        success = false;

    return success && stream.good();
    }

bool                                OutputType                 (BinaryOStream&                              stream,
                                                                const DataType&                             type)
    {
    // Serialize type family
    if (!OutputTypeFamily(stream, type.GetFamily()))
        return false;

    // Serialize type
    const DimensionOrgGroup& orgGroup = type.GetOrgGroup();

    assert(0 < orgGroup.GetTypeSize());
    //assert(orgGroup.IsComplete());

    const UInt32 orgCountField = static_cast<UInt32>(orgGroup.GetSize());
    if (!WriteValue(stream, orgCountField))
        return false;

    for (DimensionOrgGroup::const_iterator orgIt = orgGroup.begin(), orgsEnd = orgGroup.end(); 
         orgIt != orgsEnd;
         ++orgIt)
        {
        const UInt32 dimensionCountField = static_cast<UInt32>(orgIt->GetSize());
        if (!WriteValue(stream, dimensionCountField))
            return false;

        for (DimensionOrg::const_iterator dimIt = orgIt->begin(), dimsEnd = orgIt->end();
             dimIt != dimsEnd;
             ++dimIt)
            {
            const DimensionType& dimType = dimIt->GetType();

            const byte dimensionTypeField = static_cast<byte>(dimType.GetID());
            const byte dimensionRoleField = static_cast<byte>(dimIt->GetRole());
            stream.put(dimensionTypeField);
            stream.put(dimensionRoleField);
            if (dimType.IsCustom())
                WriteStringW(stream, dimType.GetName());

            if (!stream.good())
                return false;
            }
        }
    return stream.good();
    }


bool                                OutputGCS                  (BinaryOStream&                              stream,
                                                                const GCS&                                  gcs)
    {
    GCS::Status status = GCS::S_SUCCESS;
    WKT gcsWKT(gcs.GetWKT(status));
    if (GCS::S_SUCCESS != status)
        return false;

    WString extendedWktStr(gcsWKT.GetCStr());
    wchar_t wktFlavor[2] = {(wchar_t)IDTMFile::WktFlavor_Autodesk, L'\0'};

    extendedWktStr += WString(wktFlavor);        

    return WriteStringW(stream, extendedWktStr.c_str());
    }

bool                                OutputLayer                (BinaryOStream&                              stream,
                                                                UInt                                        layer)
    {
    const UInt32 layerField = layer;
    return WriteValue(stream, layerField);
    }


struct GCSConfigComponentSerializer : public ComponentSerializerMixinBase<GCSConfig>
    {
private:
    virtual bool                _Serialize                                 (const GCSConfig&                    component,
                                                                            BinaryOStream&                      stream) const override
        {
        const GCS& gcs = component.GetGCS();
        return OutputCommandID(stream, GetSerializationID()) &&
               OutputGCS(stream, gcs);
        }

public:

    virtual                     ConfigComponentSerializationID GetSerializationID() const override
        {
        return CCSID_GCS;
        }

    } s_GCSConfigComponentSerializer;

struct GCSExtendedConfigComponentSerializer : public ComponentSerializerMixinBase<GCSExtendedConfig>
    {
private:
    virtual bool                _Serialize                                 (const GCSExtendedConfig&            component,
                                                                            BinaryOStream&                      stream) const override
        {
        const GCS& gcs = component.GetGCS();
        UInt32 flagsField(0);
            
        SetBitsTo(flagsField, 0x1, component.IsPrependedToExistingLocalTransform());
        SetBitsTo(flagsField, 0x2, component.IsExistingPreservedIfGeoreferenced());
        SetBitsTo(flagsField, 0x4, component.IsExistingPreservedIfLocalCS());  
                           
        return OutputCommandID(stream, GetSerializationID()) &&
               OutputGCS(stream, gcs) &&
               WriteValue(stream, flagsField);
        }

public:

    virtual                     ConfigComponentSerializationID GetSerializationID() const override
        {
        return CCSID_GCS_ExtendedV2;
        }

    } s_GCSExtendedConfigComponentSerializer;



const struct GCSLocalAdjustmentConfigComponentSerializer : public ComponentSerializerMixinBase<GCSLocalAdjustmentConfig>
    {
private:
    virtual bool                _Serialize                                 (const GCSLocalAdjustmentConfig&     component,
                                                                            BinaryOStream&                      stream) const override
        {
        //Not sure what it was used for, but now that some of our software has been released with 
        //knowledge of CCSID_GCS_LocalAdjustment we cannot do anything with this (i.e. : must recreate a new config 
        //if we eventually want to persist what this serializer was meant to persist. 
        assert(!"Never implemented!"); 
        return false;
        }

public:

    virtual                     ConfigComponentSerializationID GetSerializationID() const override
        {
        return CCSID_GCS_LocalAdjustment;
        }

    } s_GCSLocalAdjustmentConfigComponentSerializer;


struct TypeConfigComponentSerializer : public ComponentSerializerMixinBase<TypeConfig>
    {
private:
    virtual bool                _Serialize                                 (const TypeConfig&                   component,
                                                                            BinaryOStream&                      stream) const override
        {
        return OutputCommandID(stream, GetSerializationID()) &&
               OutputType(stream, component.GetType());
        }

public:

    virtual                     ConfigComponentSerializationID GetSerializationID() const override
        {
        return CCSID_Type;
        }

    } s_TypeConfigComponentSerializer;


const struct LayerConfigComponentSerializer : public ComponentSerializerMixinBase<LayerConfig>
    {
private:
    virtual bool                _Serialize                                 (const LayerConfig&                  component,
                                                                            BinaryOStream&                      stream) const override;

public:

    virtual                     ConfigComponentSerializationID GetSerializationID() const override
        {
        return CCSID_Layer;
        }

    } s_LayerConfigComponentSerializer;



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LayerCfgVisitor : public ILayerConfigVisitor
    {                         
    SerializeCommandList            m_commands;


    virtual void                    _Visit                     (const GCSConfig&                        config) override
        {
        m_commands.push_back(SerializeCommand(s_GCSConfigComponentSerializer, config));
        }

    virtual void                    _Visit                     (const GCSExtendedConfig&                config) override
        {
        m_commands.push_back(SerializeCommand(s_GCSExtendedConfigComponentSerializer, config));
        }

    virtual void                    _Visit                     (const TypeConfig&                       config) override
        {
        m_commands.push_back(SerializeCommand(s_TypeConfigComponentSerializer, config));
        }

    virtual void                    _Visit                     (const GCSLocalAdjustmentConfig&         config) override
        {
        m_commands.push_back(SerializeCommand(s_GCSLocalAdjustmentConfigComponentSerializer, config));
        }

public:
    const SerializeCommandList&     GetSerializeCommands       () const { return m_commands; }

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class ContentCfgVisitor : public IContentConfigVisitor
    {
    private : 
    
    SerializeCommandList            m_commands;
    
    static bool SortPredicate(SerializeCommand& command1, SerializeCommand& command2)
        {            
        assert((command1.GetComponentSerializer() != 0) && (command2.GetComponentSerializer() != 0));
    
        return (command1.GetComponentSerializer()->GetSerializationID() < command2.GetComponentSerializer()->GetSerializationID());            
        }
    
    void SortCommands()
        {
        std::sort(m_commands.begin(), m_commands.end(), ContentCfgVisitor::SortPredicate);                        
        }
    
    virtual void                    _Visit                     (const GCSConfig&                            config) override
        {
        m_commands.push_back(SerializeCommand(s_GCSConfigComponentSerializer, config));
        }

   virtual void                    _Visit                     (const GCSExtendedConfig&                     config) override
        {
        m_commands.push_back(SerializeCommand(s_GCSExtendedConfigComponentSerializer, config));
        }

    virtual void                    _Visit                     (const TypeConfig&                           config) override
        {
        m_commands.push_back(SerializeCommand(s_TypeConfigComponentSerializer, config));
        }

    virtual void                    _Visit                     (const LayerConfig&                          config) override
        {
        m_commands.push_back(SerializeCommand(s_LayerConfigComponentSerializer, config));
        }

    virtual void                    _Visit                     (const GCSLocalAdjustmentConfig&             config) override
        {
        m_commands.push_back(SerializeCommand(s_GCSLocalAdjustmentConfigComponentSerializer, config));
        }

public:
    const SerializeCommandList&     GetSerializeCommands       ()  
        { 
        SortCommands();
        return m_commands; 
        }
    };




/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool                                GenericSerialize           (const SerializeCommandList&                 serializeCommands,
                                                                BinaryOStream&                              stream)
    {
    struct RunCommand
        {
        BinaryOStream& m_stream;
        explicit RunCommand (BinaryOStream& stream) : m_stream(stream) {}

        bool operator () (const SerializeCommand& command) { return !command.Run(m_stream); }
        };

    const UInt32 componentCountField = (UInt32)serializeCommands.size();

    return WriteValue(stream, componentCountField) &&
           (serializeCommands.end() == find_if(serializeCommands.begin(), serializeCommands.end(), RunCommand(stream))) && 
           stream.good();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LayerConfigComponentSerializer::_Serialize    (const LayerConfig&  component,
                                                    BinaryOStream&      stream) const
    {
    LayerCfgVisitor visitor;
    component.Accept(visitor);

    return OutputCommandID(stream, GetSerializationID()) && 
           OutputLayer(stream, component.GetID()) &&
           GenericSerialize(visitor.GetSerializeCommands(), stream);
    }

} // END unnamed namespace 


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentConfigSerializer::Serialize    (const ContentConfig&    config,
                                            BinaryOStream&          stream) const
    {
    if (0 == config.GetCount())
        return true; // Generate empty packet so that it improves load performances
    
    ContentCfgVisitor visitor;
    config.Accept(visitor);

    return GenericSerialize(visitor.GetSerializeCommands(), stream);
    }






namespace { // BEGIN unnamed namespace


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ComponentCreator
    {
    typedef ContentConfigComponentBase
                                Component;

private:
    friend struct               ComponentFactory;
    
    virtual Component*          _Create                    (BinaryIStream&      stream) const = 0;
protected:


    virtual                     ~ComponentCreator          () = 0 {}


    static const DataTypeFamily*
                                LoadTypeFamily             (BinaryIStream&      stream)
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

    static UInt                 LoadLayer                  (BinaryIStream&      stream)
        {
        UInt32 layerField = INVALID_LAYER;
        ReadValue(stream, layerField);
        return layerField;
        }

    static const UInt           INVALID_LAYER;
    };

const UInt ComponentCreator::INVALID_LAYER(numeric_limits<UInt>::max());


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ComponentFactory
    {
private:
    typedef const ComponentCreator*   
                                CreatorItem;

    static const CreatorItem*   GetCreatorIndex            ();

public:
    ContentConfigComponentBase* Create                     (bool& isNewerSerializationID, BinaryIStream&      stream) const;
};



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct UnsupportedConfigCreator : public ComponentCreator
    {
    virtual Component*          _Create                    (BinaryIStream&      stream) const override
        {
        return 0;
        }
    } s_UnsupportedConfigCreator;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool                            CreateGCSFrom              (GCS&                gcs, 
                                                            BinaryIStream&      stream)
    {
    WString gcsWKT;
    if (!ReadStringW(stream, gcsWKT))
        return false;

    GCSFactory::Status gcsFromWKTStatus = GCSFactory::S_SUCCESS;
    gcs = GetGCSFactory().Create(gcsWKT.c_str(), gcsFromWKTStatus);

    return GCSFactory::S_SUCCESS == gcsFromWKTStatus;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct GCSConfigCreator : public ComponentCreator
    {
    virtual Component*          _Create                    (BinaryIStream&      stream) const override
        {
        GCS gcs(GCS::GetNull());
        if (!CreateGCSFrom(gcs, stream))
            return 0;

        return new GCSConfig(gcs);
        }
    } s_GCSConfigCreator;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct GCSExtendedConfigCreatorV0 : public ComponentCreator
    {
    virtual Component*          _Create                    (BinaryIStream&      stream) const override
        {
        GCS gcs(GCS::GetNull());
        if (!CreateGCSFrom(gcs, stream))
            return 0;

        auto_ptr<GCSExtendedConfig> configP(new GCSExtendedConfig(gcs));

        configP->PrependToExistingLocalTransform(true);

        return configP.release();
        }
    } s_GCSOverrideConfigCreatorV0;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct GCSExtendedConfigCreatorV1 : public ComponentCreator
    {
    virtual Component*          _Create                    (BinaryIStream&      stream) const override
        {
        GCS gcs(GCS::GetNull());
        if (!CreateGCSFrom(gcs, stream))
            return 0;

        UInt8 flagsField;
        if (!ReadValue(stream, flagsField))
            return 0;
        
        auto_ptr<GCSExtendedConfig> configP(new GCSExtendedConfig(gcs));

        configP->PrependToExistingLocalTransform(HasBitsOn(flagsField, 0x1));

        return configP.release();
        }
    } s_GCSExtendedConfigCreatorV1;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct GCSExtendedConfigCreatorV2 : public ComponentCreator
    {
    virtual Component*          _Create                    (BinaryIStream&      stream) const override
        {
        WString gcsWKT;
        if (!ReadStringW(stream, gcsWKT))
            return 0;
        
        UInt32 flagsField;
        if (!ReadValue(stream, flagsField))
            return 0;
                           
        IDTMFile::WktFlavor fileWktFlavor = GetWKTFlavor(&gcsWKT, gcsWKT);        
            
        Bentley::GeoCoordinates::BaseGCS::WktFlavor baseGcsWktFlavor;

        bool result = MapWktFlavorEnum(baseGcsWktFlavor, fileWktFlavor);
        assert(result == true);
       
        GCS gcs(GCS::GetNull());

        GCSFactory::Status gcsFromWKTStatus = GCSFactory::S_SUCCESS;
        gcs = GetGCSFactory().Create(gcsWKT.c_str(), baseGcsWktFlavor, gcsFromWKTStatus);

        if (GCSFactory::S_SUCCESS != gcsFromWKTStatus)
            return 0;
        
        auto_ptr<GCSExtendedConfig> configP(new GCSExtendedConfig(gcs));

        configP->
            PrependToExistingLocalTransform(HasBitsOn(flagsField, 0x1)).
            PreserveExistingIfGeoreferenced(HasBitsOn(flagsField, 0x2)).
            PreserveExistingIfLocalCS(HasBitsOn(flagsField, 0x4));
        
        return configP.release();
        }
    } s_GCSExtendedConfigCreatorV2;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct GCSLocalAdjustmentConfigCreator : public ComponentCreator
    {
    virtual Component*          _Create                    (BinaryIStream&      stream) const override
        {
        assert(!"Implement!"); //TDORAY: Implement
        return 0;
        }
    } s_GCSLocalAdjustmentConfigCreator;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct TypeConfigCreator : ComponentCreator
    {
    virtual Component*          _Create                    (BinaryIStream&      stream) const override
        {
        // Load type family
        const DataTypeFamily* typeFamilyP = LoadTypeFamily(stream);
        if (0 == typeFamilyP)
            return 0;

        // Load type
        UInt32 orgCountField = 0;
        if (!ReadValue(stream, orgCountField) || 0 == orgCountField)
            return 0;

        DimensionOrgGroup orgGroup(orgCountField);

        for (UInt32 orgIdx = 0; orgIdx < orgCountField; ++orgIdx)
            {
            UInt32 dimensionCountField = 0;
            if (!ReadValue(stream, dimensionCountField) || 0 == dimensionCountField)
                return 0;

            DimensionOrg org(dimensionCountField);

            for (UInt32 dimIdx = 0; dimIdx < dimensionCountField; ++dimIdx)
                {
                const UInt dimensionTypeField = stream.get();
                const UInt dimensionRoleField = stream.get();

                if (!stream.good())
                    return 0;

               if (typeFamilyP->GetRoleQty() <= dimensionRoleField)
                    return 0;

                const DimensionRole dimRole = static_cast<DimensionRole>(dimensionRoleField);

                if (DimensionType::ID_QTY > dimensionTypeField)
                    {
                    org.push_back(DimensionDef(DimensionType::GetFor(static_cast<DimensionType::ID>(dimensionTypeField)),
                                               dimRole));
                    }
                else if (DimensionType::ID_CUSTOM == dimensionTypeField)
                    {
                    WString typeName;
                    if (!ReadStringW(stream, typeName))
                        return 0;

                    org.push_back(DimensionDef(DimensionType::GetFor(typeName.c_str()),dimRole));
                    }
                else
                    {
                    return 0;
                    }
                }

            orgGroup.push_back(org);
            }
        
        if (!stream.good())
            return 0;

        static const DataTypeFactory TYPE_FACTORY;
        return new TypeConfig(TYPE_FACTORY.Create(*typeFamilyP, orgGroup));
        }
    } s_TypeConfigCreator;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct LayerConfigCreator : ComponentCreator
    {
    virtual Component*          _Create                    (BinaryIStream&      stream) const override;
    } s_LayerConfigCreator;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const ComponentFactory::CreatorItem* ComponentFactory::GetCreatorIndex ()
    {
    static const CreatorItem    CREATORS_INDEX[] = 
        {
        &s_GCSConfigCreator,
        &s_TypeConfigCreator,
        &s_LayerConfigCreator,
        &s_UnsupportedConfigCreator,
        &s_GCSOverrideConfigCreatorV0,
        &s_GCSExtendedConfigCreatorV2,
        &s_GCSExtendedConfigCreatorV1,
        &s_GCSLocalAdjustmentConfigCreator,
        };
    
    static_assert(CCSID_QTY == sizeof(CREATORS_INDEX)/sizeof(CREATORS_INDEX[0]), "");

    return CREATORS_INDEX;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigComponentBase* ComponentFactory::Create (bool& isNewerSerializationID, BinaryIStream& stream) const
    {
    const UInt componentIDField = stream.get();

    if (CCSID_QTY <= componentIDField)
        {
        isNewerSerializationID = true;
        return 0;
        }

    isNewerSerializationID = false;

    ConfigComponentSerializationID componentID = static_cast<ConfigComponentSerializationID>(componentIDField);

    CreatorItem creatorP = GetCreatorIndex()[componentID];
    assert(0 != creatorP);
    
    return creatorP->_Create(stream);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ConfigT>
bool                                GenericDeserialize         (BinaryIStream&                  stream,
                                                                ConfigT&                        config)
    {
    UInt32 componentCountField = 0;
    if (!ReadValue(stream, componentCountField))
        return false;

    static const ComponentFactory COMPONENT_FACTORY;
    
    for (UInt32 i = 0; i < componentCountField; ++i)
        {
        bool isNewerSerializationID;

        RefCountedPtr<ContentConfigComponentBase> componentP(COMPONENT_FACTORY.Create(isNewerSerializationID, stream));

        //Skip unknown config
        if (isNewerSerializationID == true)
            {            
            break;
            }       

        if (0 == componentP.get())
            return false;
        if (!stream.good())
            return false;

        config.push_back(*componentP);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerConfigCreator::Component* LayerConfigCreator::_Create (BinaryIStream&      stream) const
    {
    const UInt layerID = LoadLayer(stream);
    if (INVALID_LAYER == layerID)
        return 0;

    auto_ptr<LayerConfig> configP(new LayerConfig(layerID));
    if (!GenericDeserialize(stream, *configP))
        return 0;

    return configP.release();
    }



} // END unnamed namespace


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentConfigSerializer::Deserialize  (BinaryIStream&      stream,
                                            ContentConfig&      config,
                                            UInt                formatVersion) const
    {
    if (ContentConfigSerializer::FORMAT_VERSION != formatVersion)
        {
        assert(!"Invalid version! Need to handle backward compatibility!");
        return false;
        }

    return GenericDeserialize(stream, config);
    }



END_BENTLEY_MRDTM_NAMESPACE