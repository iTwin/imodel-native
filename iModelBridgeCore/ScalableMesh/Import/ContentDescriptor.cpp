/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/ContentDescriptor.cpp $
|    $RCSfile: ContentDescriptor.cpp,v $
|   $Revision: 1.11 $
|       $Date: 2011/11/18 15:50:56 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include <ScalableMesh/Import/ContentDescriptor.h>

#include <ScalableMesh/Import/Metadata.h>
#include <ScalableMesh/Import/Attachment.h>

#include <ScalableMesh/Import/SourceReference.h>
#include <STMInternal/Foundations/PrivateStringTools.h>

#include <ScalableMesh/Import/ContentConfig.h>
#include <ScalableMesh/Import/Config/Content/All.h>
#include <ScalableMesh/Import/ContentConfigVisitor.h>

#include <ScalableMesh/Foundations/Exception.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentDescriptorImpl : public ShareableObjectTypeTrait<ContentDescriptorImpl>::type
    {
    typedef bvector<LayerDescriptor>             LayerList;

    WString                                     m_name;
    LayerList                                   m_layers;
    bool                                        m_holdsIncompleteTypes;    
    bool                                        m_canRepresent3dData;
    bool                                        m_isPod;

    // NTERAY: Optimize by making this a pointer an lazy initialize it as it is optionnal.
    MetadataRecord                              m_metadataRecord;

    // Capabilities

    explicit                                    ContentDescriptorImpl          (const WChar*                     name)
        :   m_name(name),
            m_holdsIncompleteTypes(false),
            m_isPod(false)
        {
        
        }

    explicit                                    ContentDescriptorImpl          (const WChar*                     name,
                                                                                const LayerDescriptor&           layer, 
                                                                                bool                             canRepresent3dData)
        :   m_name(name),
            m_holdsIncompleteTypes(layer.HoldsIncompleteTypes()), 
            m_canRepresent3dData(canRepresent3dData),
            m_isPod(layer.GetScalableMeshData().IsGroundDetection())
        {
        m_layers.push_back(layer);
        }

    static bool                                 HoldsIncompleteTypes           (const bvector<LayerDescriptor>&      layers)
        {
        return layers.end() != std::find_if(layers.begin(), layers.end(), std::mem_fun_ref(&LayerDescriptor::HoldsIncompleteTypes));
        }

    static void                                 UpdateForEdit                  (SharedPtrTypeTrait<ContentDescriptorImpl>::type&    
                                                                                                                    pi_rpInstance)
        {
        // Copy on write when config is shared
        if (pi_rpInstance->IsShared())
            pi_rpInstance = new ContentDescriptorImpl(*pi_rpInstance);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::~ContentDescriptor ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::ContentDescriptor (const ContentDescriptor& pi_rRight)
    :   m_pImpl(pi_rRight.m_pImpl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor& ContentDescriptor::operator= (const ContentDescriptor& pi_rRight)
    {
    m_pImpl = pi_rRight.m_pImpl;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::ContentDescriptor   (const WChar*          name)
    :   m_pImpl(new ContentDescriptorImpl(name))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::ContentDescriptor   (const WChar*           name,
                                        const LayerDescriptor& layer, 
                                        bool                   canRepresent3dData)
    :   m_pImpl(new ContentDescriptorImpl(name, layer, canRepresent3dData))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::Add (const LayerDescriptor& pi_rLayer)
    {
    ContentDescriptorImpl::UpdateForEdit(m_pImpl);
    m_pImpl->m_layers.push_back(pi_rLayer);
    m_pImpl->m_holdsIncompleteTypes |= pi_rLayer.HoldsIncompleteTypes();
    m_pImpl->m_isPod |= pi_rLayer.GetScalableMeshData().IsGroundDetection();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::push_back (const LayerDescriptor& layer)
    {
    Add(layer);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& ContentDescriptor::GetName () const
    {
    return m_pImpl->m_name;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* ContentDescriptor::GetNameCStr () const
    {
    return m_pImpl->m_name.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
UInt ContentDescriptor::GetLayerCount () const
    {
    return (UInt) m_pImpl->m_layers.size();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::const_iterator ContentDescriptor::LayersBegin () const
    {
    return &*m_pImpl->m_layers.begin();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::const_iterator ContentDescriptor::LayersEnd () const
    {
    return &*m_pImpl->m_layers.end();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::const_iterator ContentDescriptor::FindLayerFor (LayerID pi_layerID) const
    {
    if (pi_layerID >= GetLayerCount())
        return LayersEnd();

    return &*(m_pImpl->m_layers.begin() + pi_layerID);
    }

ContentDescriptor::iterator ContentDescriptor::FindLayerFor(LayerID pi_layerID)
{
    if (pi_layerID >= GetLayerCount())
        return (iterator)LayersEnd();

    return &*(m_pImpl->m_layers.begin() + pi_layerID);
}

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::LayerID ContentDescriptor::GetLayerIDFor (const_iterator pi_rIter) const
    {
    if (pi_rIter < LayersBegin() || pi_rIter >= LayersEnd())
        {
        assert(!"Invalid iterator");
        return 0; // TDORAY: Throw???
        }

    return LayerID(std::distance(LayersBegin(), pi_rIter));
    }

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Raymond.Gauthier   04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::HoldsIncompleteTypes () const
    {
    return m_pImpl->m_holdsIncompleteTypes;
    }

bool ContentDescriptor::IsPod() const
    {
    return m_pImpl->m_isPod;
    }

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Mathieu.St-Pierre   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::CanRepresent3dData () const
    {
    return m_pImpl->m_canRepresent3dData;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const MetadataRecord& ContentDescriptor::GetMetadataRecord () const
    {
    return m_pImpl->m_metadataRecord;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataRecord& ContentDescriptor::EditMetadataRecord ()
    {
    ContentDescriptorImpl::UpdateForEdit(m_pImpl);
    return m_pImpl->m_metadataRecord;
    }




/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct LayerDescriptorImpl : public ShareableObjectTypeTrait<LayerDescriptorImpl>::type
    {
    typedef bvector<TypeDescriptor>             TypeList;
    typedef bvector<SourceRef>                  SourceRefList;

    WString                                  m_name;
    TypeList                                    m_storedTypes;
    bool                                        m_holdsIncompleteTypes;

    GCS                                         m_gcs;
    std::auto_ptr<DRange3d>                     m_pExtent;
    ScalableMeshData                            m_scalableMeshData;

    // NTERAY: Optimize by making this a pointer an lazy initialize it as it is optionnal.
    AttachmentRecord                            m_attachmentRecord;
    // NTERAY: Optimize by making this a pointer an lazy initialize it as it is optionnal.
    MetadataRecord                              m_metadataRecord;

    explicit                                    LayerDescriptorImpl            (const WChar*                     pi_name,
                                                                                const DataTypeSet&                  storedTypes,
                                                                                const GCS&                          pi_rGCS,
                                                                                const DRange3d*                     pi_pExtent,
                                                                                const ScalableMeshData&             pi_data)
            :   m_name(pi_name),
                m_storedTypes(storedTypes.begin(), storedTypes.end()),
                m_holdsIncompleteTypes(HoldsIncompleteTypes(m_storedTypes)),
                m_gcs(pi_rGCS),
                m_pExtent((0 != pi_pExtent) ? new DRange3d(*pi_pExtent) : 0),
                m_scalableMeshData(pi_data)

        {
        std::sort(m_storedTypes.begin(), m_storedTypes.end());
        }

    static bool                                 HoldsIncompleteTypes           (const TypeList&                     types)
        {
        struct IsIncomplete
            {
            bool operator () (const TypeDescriptor& typeDesc) const { return !typeDesc.GetType().IsComplete(); }
            };
        return types.end() != std::find_if(types.begin(), types.end(), IsIncomplete());
        }

    static void                                 UpdateForEdit                  (SharedPtrTypeTrait<LayerDescriptorImpl>::type&     
                                                                                                                    pi_rpInstance)
        {
        // Copy on write when config is shared
        if (pi_rpInstance->IsShared())
            pi_rpInstance = new LayerDescriptorImpl(*pi_rpInstance);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
LayerDescriptor::~LayerDescriptor ()
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
LayerDescriptor::LayerDescriptor (const LayerDescriptor& pi_rRight)
    :   m_pImpl(pi_rRight.m_pImpl)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
LayerDescriptor& LayerDescriptor::operator= (const LayerDescriptor& pi_rRight)

    {
    m_pImpl = pi_rRight.m_pImpl;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerDescriptor::LayerDescriptor   (const WChar*                      name,
                                    const DataTypeSet&                  storedTypes,
                                    const GCS&                          rGCS,
                                    const DRange3d*                     extentP,
                                    const ScalableMeshData&             rData)
    :   m_pImpl(new LayerDescriptorImpl(name, storedTypes, rGCS, extentP, rData))
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& LayerDescriptor::GetName () const
    {
    return m_pImpl->m_name;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* LayerDescriptor::GetNameCStr () const
    {
    return m_pImpl->m_name.c_str();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
UInt LayerDescriptor::GetTypeCount () const
    {
    return (UInt) m_pImpl->m_storedTypes.size();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerDescriptor::const_iterator LayerDescriptor::TypesBegin () const
    {
    return &*m_pImpl->m_storedTypes.begin();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerDescriptor::const_iterator LayerDescriptor::TypesEnd () const
    {
    return &*m_pImpl->m_storedTypes.end();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerDescriptor::const_iterator LayerDescriptor::FindTypeFor (const DataTypeFamily& typeFamily) const
    {
    return &*std::find(m_pImpl->m_storedTypes.begin(), m_pImpl->m_storedTypes.end(), typeFamily);
    }

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool LayerDescriptor::HoldsIncompleteTypes () const
    {
    return m_pImpl->m_holdsIncompleteTypes;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool LayerDescriptor::HasExtent () const
    {
    return 0 != m_pImpl->m_pExtent.get();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const DRange3d& LayerDescriptor::GetExtent () const
    {
    if (!HasExtent())
        m_pImpl->m_pExtent.reset(new DRange3d());
    return *m_pImpl->m_pExtent;
    }

const ScalableMeshData& LayerDescriptor::GetScalableMeshData() const
    {
    return m_pImpl->m_scalableMeshData;
    }

void LayerDescriptor::SetScalableMeshData(ScalableMeshData& data)
    {
    m_pImpl->m_scalableMeshData = data;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const GCS& LayerDescriptor::GetGCS () const
    {
    return m_pImpl->m_gcs;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const AttachmentRecord& LayerDescriptor::GetAttachmentRecord () const
    {
    return m_pImpl->m_attachmentRecord;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentRecord& LayerDescriptor::EditAttachmentRecord ()
    {
    LayerDescriptorImpl::UpdateForEdit(m_pImpl);
    return m_pImpl->m_attachmentRecord;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const MetadataRecord& LayerDescriptor::GetMetadataRecord () const
    {
    return m_pImpl->m_metadataRecord;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataRecord& LayerDescriptor::EditMetadataRecord ()
    {
    LayerDescriptorImpl::UpdateForEdit(m_pImpl);
    return m_pImpl->m_metadataRecord;
    }




namespace {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LayerCfgVisitor : public ILayerConfigVisitor
    {
    friend class                    ContentDesc;
    friend class                    ContentCfgVisitor;

    template <typename ConfigT>
    struct                          VisitConfig;
    
    LayerDescriptorImpl*            m_implP;

    virtual void                    _Visit                     (const GCSConfig&                    config) override;
    virtual void                    _Visit                     (const GCSExtendedConfig&            config) override;
    virtual void                    _Visit                     (const TypeConfig&                   config) override;
    virtual void                    _Visit                     (const ScalableMeshConfig&           config) override;
    virtual void                    _Visit                     (const GCSLocalAdjustmentConfig&     config) override;

public:
    explicit                        LayerCfgVisitor            (LayerDescriptorImpl&                layerDescImpl) : m_implP(&layerDescImpl) {}

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void LayerCfgVisitor::_Visit (const GCSConfig& config) 
    {
    m_implP->m_gcs = config.GetGCS();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void LayerCfgVisitor::_Visit (const GCSExtendedConfig& config)
    {
    if (m_implP->m_gcs.HasGeoRef() && config.IsExistingPreservedIfGeoreferenced())
        return;

    if (!m_implP->m_gcs.IsNull() && config.IsExistingPreservedIfLocalCS())
        return;

    if (!m_implP->m_gcs.HasLocalTransform() || !config.IsPrependedToExistingLocalTransform())
        {
        m_implP->m_gcs = config.GetGCS();
        return;
        }

    // We suppose that user has already taken care of adapting his gcs to the input units
    // of the existing transform
    GCS newGCS(config.GetGCS());

    if (GCS::S_SUCCESS != newGCS.AppendLocalTransform(m_implP->m_gcs.GetLocalTransform()))
        throw CustomException(L"Error creating GCS!");

    swap(m_implP->m_gcs, newGCS);
    }


/* // TDORAY: Take a look at whether it is possible to integrate this to GCSExtendedConfig Visit
void LayerCfgVisitor::_Visit (const GCSExtendedConfig& config)
    {
    const GCS& originalGCS = m_implP->m_gcs;
    const GCS& configGCS = config.GetGCS();

    if (originalGCS.IsNull())
        {
        m_implP->m_gcs = configGCS;
        return;
        }

    // Don't support overriding existing spatial reference. Use GCSOverrideConfig
    // instead.
    if (originalGCS.HasGeospatialReference())
        throw CustomException(L"Could not override existing spatial reference!");

    const bool configHasLocalTransform = configGCS.HasLocalTransform();
    const bool equivalentUnits = HaveEquivalentUnits(originalGCS, configGCS);
    const bool originalHasLocalTransform = originalGCS.HasLocalTransform();


    if (!configHasLocalTransform && equivalentUnits && !originalHasLocalTransform)
        {
        m_implP->m_gcs = configGCS;
        return;
        }

    if (!equivalentUnits && !HaveCompatibleUnits(originalGCS, configGCS))
        throw CustomException(L"Incompatible units!");

    LocalTransform localTransform(configHasLocalTransform ? configGCS.GetLocalTransform() : LocalTransform::GetIdentity());
        
    if (!equivalentUnits)
        localTransform.Append(CreateUnitRectificationTransform(configGCS.GetHorizontalUnit(), 
                                                               configGCS.GetVerticalUnit(),
                                                               originalGCS.GetHorizontalUnit(), 
                                                               originalGCS.GetVerticalUnit()));

    if (originalHasLocalTransform)
        localTransform.Append(originalGCS.GetLocalTransform());

    static const GCSFactory FACTORY;
    GCSFactory::Status creationStatus;
    GCS newGCS(FACTORY.Create(configGCS, localTransform, creationStatus));

    if (GCSFactory::S_SUCCESS != creationStatus)
        throw CustomException(L"Error creating GCS!");

    swap(m_implP->m_gcs, newGCS);
    }
*/

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void LayerCfgVisitor::_Visit (const TypeConfig& config)
    {
    const DataType& type = config.GetType();

    LayerDescriptorImpl::TypeList::iterator foundTypeIt = std::find (m_implP->m_storedTypes.begin(), m_implP->m_storedTypes.end(), type.GetFamily());

    if (m_implP->m_storedTypes.end() == foundTypeIt)
        throw CustomException(L"Replacement type not found!");

    if (foundTypeIt->GetType().GetDimensionOrgCount() != type.GetDimensionOrgCount())
        throw CustomException(L"Mismatching dimension org count!");

    *foundTypeIt = type;

    m_implP->m_holdsIncompleteTypes = LayerDescriptorImpl::HoldsIncompleteTypes(m_implP->m_storedTypes);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void LayerCfgVisitor::_Visit(const ScalableMeshConfig& config)
{
    // We suppose that user has already taken care of adapting his gcs to the input units
    // of the existing transform
        const ScalableMeshData& data = config.GetScalableMeshData();

        m_implP->m_scalableMeshData = data;
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void LayerCfgVisitor::_Visit (const GCSLocalAdjustmentConfig& config)
    {
    if (GCS::S_SUCCESS != m_implP->m_gcs.AppendLocalTransform(config.GetTransform()))
        throw CustomException(L"Could not locally adjust existing GCS!");
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class ContentCfgVisitor : public IContentConfigVisitor
    {
    typedef bvector<LayerCfgVisitor> LayerList;

    ContentDescriptorImpl*          m_implP;
    LayerList                       m_layers;

   
    virtual void                    _Visit                     (const GCSConfig&                    config) override;
    virtual void                    _Visit                     (const GCSExtendedConfig&            config) override;
    virtual void                    _Visit                     (const TypeConfig&                   config) override;
    virtual void                    _Visit                     (const ScalableMeshConfig&           config) override;

    virtual void                    _Visit                     (const LayerConfig&                  config) override;

    virtual void                    _Visit                     (const GCSLocalAdjustmentConfig&     config) override;

    template <typename ConfigT>
    void                            PropagateToAllLayers       (const ConfigT&                      config);

public:
    explicit                        ContentCfgVisitor          (ContentDescriptorImpl&              impl) : m_implP(&impl) {}

    void                            AddLayer                   (LayerDescriptorImpl&                impl) 
        { m_layers.push_back(LayerCfgVisitor(impl)); }

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ConfigT>
struct LayerCfgVisitor::VisitConfig
    {
    const ConfigT&      m_config;
    explicit            VisitConfig                (const ConfigT&      config) : m_config(config) {}

    void                operator ()                (LayerCfgVisitor&    layer) { layer._Visit(m_config); }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ConfigT>
void ContentCfgVisitor::PropagateToAllLayers (const ConfigT& config)
    {
    std::for_each(m_layers.begin(), m_layers.end(), LayerCfgVisitor::VisitConfig<ConfigT>(config));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentCfgVisitor::_Visit (const LayerConfig& config)
    {
    LayerList::iterator layerIt = m_layers.begin() + config.GetID();

    if (m_layers.end() <= layerIt)
        throw CustomException(L"Invalid layer!");

    config.Accept(*layerIt);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentCfgVisitor::_Visit (const GCSConfig& config)
    {
    PropagateToAllLayers(config);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentCfgVisitor::_Visit (const GCSExtendedConfig& config)
    {
    PropagateToAllLayers(config);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentCfgVisitor::_Visit (const TypeConfig& config)
    {
    PropagateToAllLayers(config);
    m_implP->m_holdsIncompleteTypes = ContentDescriptorImpl::HoldsIncompleteTypes(m_implP->m_layers);
    }

void ContentCfgVisitor::_Visit(const ScalableMeshConfig& config)
{
    PropagateToAllLayers(config);
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentCfgVisitor::_Visit (const GCSLocalAdjustmentConfig& config)
    {
    PropagateToAllLayers(config);
    }

}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Status ContentDescriptor::Configure (const ContentConfig&    config,
                                                        Log&                    log)
    {
    return Configure(config, ContentConfigPolicy(), log);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Status ContentDescriptor::Configure (const ContentConfig&            config,
                                                        const ContentConfigPolicy&   policy,
                                                        Log&                            log)
    {
    ContentDescriptorImpl::UpdateForEdit(m_pImpl);

    ContentCfgVisitor visitor(*m_pImpl);

    typedef ContentDescriptorImpl::LayerList::iterator LayerIt;

    for (LayerIt layerIt = m_pImpl->m_layers.begin(), layersEnd = m_pImpl->m_layers.end();
         layerIt != layersEnd;
         ++layerIt)
        {
        LayerDescriptorImpl::UpdateForEdit(layerIt->m_pImpl);
        visitor.AddLayer(*layerIt->m_pImpl);
        }

    try 
        {
        config.Accept(visitor);
        return S_SUCCESS;
        }
    catch (const Exception&)
        {
        return S_ERROR;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeDescriptor::TypeDescriptor     (const DataType& type)
    :   m_type(type),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeDescriptor& TypeDescriptor::operator= (const DataType& type)
    {
    m_type = type;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeDescriptor::~TypeDescriptor ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeDescriptor::TypeDescriptor (const TypeDescriptor&   rhs)
    :   m_type(rhs.m_type),
        m_implP(0)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeDescriptor& TypeDescriptor::operator= (const TypeDescriptor& rhs)
    {
    m_type = rhs.m_type;
    return *this;
    }

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
