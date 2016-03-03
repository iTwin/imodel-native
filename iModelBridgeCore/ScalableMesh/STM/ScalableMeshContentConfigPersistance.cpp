/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshContentConfigPersistance.cpp $
|    $RCSfile: ScalableMeshContentConfigPersistance.cpp,v $
|   $Revision: 1.19 $
|       $Date: 2012/02/16 22:19:31 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include "ImagePPHeaders.h"
//#include "InternalUtilityFunctions.h"

#include "ScalableMeshContentConfigPersistance.h"

#include <ScalableMesh/GeoCoords/GCS.h>

#include <ScalableMesh/Import/ContentConfigVisitor.h>
#include <ScalableMesh/Import/Config/Content/All.h>
#include <ScalableMesh/Import/DataTypeDescription.h>

#include <ScalableMesh/Type/IScalableMeshLinear.h>
#include <ScalableMesh/Type/IScalableMeshPoint.h>
#include <ScalableMesh/Type/IScalableMeshTIN.h>
#include <ScalableMesh/Type/IScalableMeshMesh.h>

#include <ScalableMesh/IScalableMeshStream.h>

#include <ScalableMesh/IScalableMeshPolicy.h>

#include <STMInternal/GeoCoords/WKTUtils.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

    

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*
 * Driver current version
 */ 
const uint32_t ContentConfigSerializer::FORMAT_VERSION = 0;


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
    CCSID_ScalableMeshConfig,
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

    virtual bool                _Serialize(const ContentConfigComponentBase&   component,
        SourceDataSQLite&                      sourceData) const = 0;


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

    virtual bool                _Serialize(const ContentConfigComponentBase&   component,
        SourceDataSQLite&                      sourceData) const override
    {
        assert(0 != dynamic_cast<const ComponentT*>(&component));
        return _Serialize(static_cast<const ComponentT&>(component), sourceData);
    }
    virtual bool                _Serialize(const ComponentT&                   component,
        SourceDataSQLite&                      sourceData) const = 0;


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

    bool                                Run(SourceDataSQLite&                      sourceData) const
    {
        return m_serializerP->_Serialize(*m_componentP, sourceData);
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


bool                                OutputCommandID(SourceDataSQLite&                              sourceData,
    ConfigComponentSerializationID              id)
{
    assert(id < CCSID_QTY);
    sourceData.AddConfigComponentID(id);
    return true;
}



bool                                OutputTypeFamily(SourceDataSQLite&                              sourceData,
    const DataTypeFamily&                       type)
{
    static const DataTypeFamily POINT_TYPE_FAMILY(PointTypeFamilyCreator().Create());
    static const DataTypeFamily LINEAR_TYPE_FAMILY(LinearTypeFamilyCreator().Create());
    static const DataTypeFamily TIN_TYPE_FAMILY(TINTypeFamilyCreator().Create());
    static const DataTypeFamily MESH_TYPE_FAMILY(MeshTypeFamilyCreator().Create());


    bool success = true;

    if (POINT_TYPE_FAMILY == type)
        sourceData.SetTypeFamilyID(static_cast<byte>(TFID_POINT));
    else if (LINEAR_TYPE_FAMILY == type)
        sourceData.SetTypeFamilyID(static_cast<byte>(TFID_POINT));
    else if (TIN_TYPE_FAMILY == type)
        sourceData.SetTypeFamilyID(static_cast<byte>(TFID_TIN));
    else if (MESH_TYPE_FAMILY == type)
        sourceData.SetTypeFamilyID(static_cast<byte>(TFID_MESH));
    else
        success = false;

    return success;
}



bool                                OutputType(SourceDataSQLite&                              sourceData,
    const DataType&                             type)
{
    bool success = true;
    // Serialize type family
    if (!OutputTypeFamily(sourceData, type.GetFamily()))
        return false;

    // Serialize type
    const DimensionOrgGroup& orgGroup = type.GetOrgGroup();

    assert(0 < orgGroup.GetTypeSize());
    //assert(orgGroup.IsComplete());

    const uint32_t orgCountField = static_cast<uint32_t>(orgGroup.GetSize());
    sourceData.SetOrgCount(orgCountField);
    size_t orgIdx = 0;
    sourceData.ResizeDimensions(orgCountField);
    for (DimensionOrgGroup::const_iterator orgIt = orgGroup.begin(), orgsEnd = orgGroup.end();
    orgIt != orgsEnd;
        ++orgIt)
    {
        const uint32_t dimensionCountField = static_cast<uint32_t>(orgIt->GetSize());
        sourceData.AddDimensionCount(dimensionCountField);

        for (DimensionOrg::const_iterator dimIt = orgIt->begin(), dimsEnd = orgIt->end();
        dimIt != dimsEnd;
            ++dimIt)
        {
            const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType& dimType = dimIt->GetType();

            const byte dimensionTypeField = static_cast<byte>(dimType.GetID());
            const byte dimensionRoleField = static_cast<byte>(dimIt->GetRole());
            sourceData.AddDimensionType(orgIdx, dimensionTypeField);
            sourceData.AddDimensionRole(orgIdx, dimensionRoleField);
            //if (dimType.IsCustom())
                sourceData.AddDimTypeName(orgIdx, dimType.GetName());

            if (!success)
                return false;
        }
        orgIdx++;
    }
    return success;
}



bool OutputGCS(SourceDataSQLite& sourceData, const GCS& gcs)
{
    GCS::Status status = GCS::S_SUCCESS;
    WKT gcsWKT(gcs.GetWKT(status));
    if (GCS::S_SUCCESS != status)
        return false;

    WString extendedWktStr(gcsWKT.GetCStr());
    wchar_t wktFlavor[2] = { (wchar_t)IDTMFile::WktFlavor_Autodesk, L'\0' };

    extendedWktStr += WString(wktFlavor);

    sourceData.SetGCS(extendedWktStr);
    return true;
}


bool                                OutputLayer(SourceDataSQLite&                              sourceData,
    uint32_t                                        layer)
{
    const uint32_t layerField = layer;
    sourceData.SetLayer(layerField);
    return true;
}



bool OutputScalableMeshConfig(SourceDataSQLite& sourceData, const ScalableMeshData& data)
{
    sourceData.SetScalableMeshData(data);
    return true;
}



struct GCSConfigComponentSerializer : public ComponentSerializerMixinBase<GCSConfig>
    {
private:

    virtual bool                _Serialize(const GCSConfig&                    component,
        SourceDataSQLite&                      sourceData) const override
    {
        const GCS& gcs = component.GetGCS();
        return OutputCommandID(sourceData, GetSerializationID()) && OutputGCS(sourceData, gcs);
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

    virtual bool                _Serialize(const GCSExtendedConfig&            component,
        SourceDataSQLite&                      sourceData) const override
    {
        const GCS& gcs = component.GetGCS();
        uint32_t flagsField(0);

        SetBitsTo(flagsField, 0x1, component.IsPrependedToExistingLocalTransform());
        SetBitsTo(flagsField, 0x2, component.IsExistingPreservedIfGeoreferenced());
        SetBitsTo(flagsField, 0x4, component.IsExistingPreservedIfLocalCS());

        sourceData.SetFlags(flagsField);
        return OutputCommandID(sourceData, GetSerializationID()) && OutputGCS(sourceData, gcs);
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

    virtual bool                _Serialize(const GCSLocalAdjustmentConfig&     component,
        SourceDataSQLite&                      sourceData) const override
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

    virtual bool                _Serialize(const TypeConfig&                   component,
        SourceDataSQLite&                      sourceData) const override
    {
        return OutputCommandID(sourceData, GetSerializationID()) && OutputType(sourceData, component.GetType());
    }


public:

    virtual                     ConfigComponentSerializationID GetSerializationID() const override
        {
        return CCSID_Type;
        }

    } s_TypeConfigComponentSerializer;

struct ScalableMeshConfigComponentSerializer : public ComponentSerializerMixinBase < ScalableMeshConfig >
{
private:

    virtual bool                _Serialize(const ScalableMeshConfig&                   component,
        SourceDataSQLite&                      sourceData) const override
    {
        sourceData.SetScalableMeshData(component.GetScalableMeshData());
        return OutputCommandID(sourceData, GetSerializationID());
    }



public:

    virtual                     ConfigComponentSerializationID GetSerializationID() const override
    {
        return CCSID_ScalableMeshConfig;
    }

} s_ScalableMeshConfigComponentSerializer;


const struct LayerConfigComponentSerializer : public ComponentSerializerMixinBase<LayerConfig>
    {
private:

    virtual bool                _Serialize(const LayerConfig&                  component,
        SourceDataSQLite&                      sourceData) const override;



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

    virtual void                    _Visit                        (const ScalableMeshConfig&                       config) override
    {
        m_commands.push_back(SerializeCommand(s_ScalableMeshConfigComponentSerializer, config));
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

    virtual void                    _Visit                        (const ScalableMeshConfig&                          config) override
    {
        m_commands.push_back(SerializeCommand(s_ScalableMeshConfigComponentSerializer, config));
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

bool                                GenericSerialize(const SerializeCommandList&                 serializeCommands,
    SourceDataSQLite&                              sourceData)

    {
    struct RunCommand
        {

            SourceDataSQLite& m_sourceData;
            explicit RunCommand(SourceDataSQLite& sourceData) : m_sourceData(sourceData) {}

            bool operator () (const SerializeCommand& command) { return !command.Run(m_sourceData); }

        };

    const uint32_t componentCountField = (uint32_t)serializeCommands.size();

    sourceData.SetComponentCount(componentCountField);
    return (serializeCommands.end() == find_if(serializeCommands.begin(), serializeCommands.end(), RunCommand(sourceData)));

    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LayerConfigComponentSerializer::_Serialize(const LayerConfig&  component,
    SourceDataSQLite&      sourceData) const
{
    LayerCfgVisitor visitor;
    component.Accept(visitor);

    return OutputCommandID(sourceData, GetSerializationID()) && OutputLayer(sourceData, component.GetID()) &&
        GenericSerialize(visitor.GetSerializeCommands(), sourceData);
}


} // END unnamed namespace 


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentConfigSerializer::Serialize(const ContentConfig&    config,
    SourceDataSQLite&          sourceData) const
{
    if (0 == config.GetCount())
        return true; // Generate empty packet so that it improves load performances

    ContentCfgVisitor visitor;
    config.Accept(visitor);

    return GenericSerialize(visitor.GetSerializeCommands(), sourceData);
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

    virtual Component*          _Create(SourceDataSQLite&      dataSource) const = 0;

protected:


    virtual                     ~ComponentCreator          () = 0 {}


    static const DataTypeFamily*
        LoadTypeFamily(SourceDataSQLite&      sourceData)
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

    static uint32_t                 LoadLayer(SourceDataSQLite&      sourceData)
    {
        uint32_t layerField = INVALID_LAYER;
        //ReadValue(stream, layerField);
        return layerField;
    }

    static const uint32_t           INVALID_LAYER;
    };

const uint32_t ComponentCreator::INVALID_LAYER(numeric_limits<uint32_t>::max());


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

    ContentConfigComponentBase* Create(bool& isNewerSerializationID, SourceDataSQLite&      sourceData) const;

};



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct UnsupportedConfigCreator : public ComponentCreator
    {
        virtual Component*          _Create(SourceDataSQLite&      sourceData) const override
        {
            return 0;
        }

    } s_UnsupportedConfigCreator;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/

bool                            CreateGCSFrom(GCS&                gcs,
    SourceDataSQLite&      sourceData)
{
    WString gcsWKT = sourceData.GetGCS();

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

        virtual Component*          _Create(SourceDataSQLite&      sourceData) const override
        {
            GCS gcs(GCS::GetNull());
            if (!CreateGCSFrom(gcs, sourceData))
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

        virtual Component*          _Create(SourceDataSQLite&      sourceData) const override
        {
            GCS gcs(GCS::GetNull());
            if (!CreateGCSFrom(gcs, sourceData))
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

        virtual Component*          _Create(SourceDataSQLite&      sourceData) const override
        {
            GCS gcs(GCS::GetNull());
            if (!CreateGCSFrom(gcs, sourceData))
                return 0;

            uint8_t flagsField = sourceData.GetFlags();

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

        virtual Component*          _Create(SourceDataSQLite&      sourceData) const override
        {
            WString gcsWKT = sourceData.GetGCS();

            uint32_t flagsField = sourceData.GetFlags();

            IDTMFile::WktFlavor fileWktFlavor = GetWKTFlavor(&gcsWKT, gcsWKT);

            BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::WktFlavor baseGcsWktFlavor;

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

        virtual Component*          _Create(SourceDataSQLite&      sourceData) const override
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
        virtual Component*          _Create(SourceDataSQLite&      sourceData) const override
        {
            // Load type family
            const DataTypeFamily* typeFamilyP = LoadTypeFamily(sourceData);
            if (0 == typeFamilyP)
                return 0;

            // Load type
            uint32_t orgCountField = sourceData.GetOrgCount();
            if (0 == orgCountField)
                return 0;

            DimensionOrgGroup orgGroup(orgCountField);

            for (uint32_t orgIdx = 0; orgIdx < orgCountField; ++orgIdx)
            {
                uint32_t dimensionCountField = sourceData.GetDimensionCount(orgIdx); // Maybe need some index because we have vector
                if (0 == dimensionCountField)
                    return 0;

                DimensionOrg org(dimensionCountField);

                for (uint32_t dimIdx = 0; dimIdx < dimensionCountField; ++dimIdx)
                {
                    const uint32_t dimensionTypeField = sourceData.GetDimensionType(orgIdx, dimIdx);
                    const uint32_t dimensionRoleField = sourceData.GetDimensionRole(orgIdx, dimIdx);

                    if (typeFamilyP->GetRoleQty() <= dimensionRoleField)
                        return 0;

                    const DimensionRole dimRole = static_cast<DimensionRole>(dimensionRoleField);

                    if (BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::ID_QTY > dimensionTypeField)
                    {
                        org.push_back(DimensionDef(BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::GetFor(static_cast<BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::ID>(dimensionTypeField)),
                            dimRole));
                    }
                    else if (BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::ID_CUSTOM == dimensionTypeField)
                    {
                        WString typeName = sourceData.GetDimensionTypeName(orgIdx, dimIdx);

                        org.push_back(DimensionDef(BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::GetFor(typeName.c_str()), dimRole));
                    }
                    else
                    {
                        return 0;
                    }
                }

                orgGroup.push_back(org);
            }

            /*if (!stream.good())
                return 0;*/

            static const DataTypeFactory TYPE_FACTORY;
            return new TypeConfig(TYPE_FACTORY.Create(*typeFamilyP, orgGroup));
        }
    } s_TypeConfigCreator;

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct ScalableMeshConfigCreator : ComponentCreator
{

    virtual Component*          _Create(SourceDataSQLite&      sourceData) const override
    {
        return new ScalableMeshConfig(ScalableMeshData(sourceData.GetScalableMeshData()));
    }

} s_ScalableMeshConfigCreator;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct LayerConfigCreator : ComponentCreator
    {

        virtual Component*          _Create(SourceDataSQLite&      sourceData) const override;

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
        &s_ScalableMeshConfigCreator,
        };
    
    static_assert(CCSID_QTY == sizeof(CREATORS_INDEX)/sizeof(CREATORS_INDEX[0]), "");

    return CREATORS_INDEX;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigComponentBase* ComponentFactory::Create(bool& isNewerSerializationID, SourceDataSQLite& sourceData) const
{
    const uint32_t componentIDField = sourceData.PopConfigComponentID();

    if (CCSID_QTY <= componentIDField)
    {
        isNewerSerializationID = true;
        return 0;
    }

    isNewerSerializationID = false;

    ConfigComponentSerializationID componentID = static_cast<ConfigComponentSerializationID>(componentIDField);

    CreatorItem creatorP = GetCreatorIndex()[componentID];
    assert(0 != creatorP);

    return creatorP->_Create(sourceData);
}



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ConfigT>
bool                                GenericDeserialize(SourceDataSQLite&                  sourceData,
    ConfigT&                        config)
{
    uint32_t componentCountField = sourceData.GetComponentCount();

    static const ComponentFactory COMPONENT_FACTORY;

    for (uint32_t i = 0; i < componentCountField; ++i)
    {
        bool isNewerSerializationID;

        RefCountedPtr<ContentConfigComponentBase> componentP(COMPONENT_FACTORY.Create(isNewerSerializationID, sourceData));

        //Skip unknown config
        if (isNewerSerializationID == true)
        {
            break;
        }

        if (0 == componentP.get())
            return false;

        config.push_back(*componentP);
    }

    return true;
}


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerConfigCreator::Component* LayerConfigCreator::_Create(SourceDataSQLite&      sourceData) const
{
    const uint32_t layerID = LoadLayer(sourceData);
    if (INVALID_LAYER == layerID)
        return 0;

    auto_ptr<LayerConfig> configP(new LayerConfig(layerID));
    if (!GenericDeserialize(sourceData, *configP))
        return 0;

    return configP.release();
}



} // END unnamed namespace


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/

bool ContentConfigSerializer::Deserialize(SourceDataSQLite&      sourceData,
    ContentConfig&      config,
    uint32_t                formatVersion) const
{
    if (ContentConfigSerializer::FORMAT_VERSION != formatVersion)
    {
        assert(!"Invalid version! Need to handle backward compatibility!");
        return false;
    }

    return GenericDeserialize(sourceData, config);
}




END_BENTLEY_SCALABLEMESH_NAMESPACE
