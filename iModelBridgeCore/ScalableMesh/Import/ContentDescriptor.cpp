/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/ContentDescriptor.cpp $
|    $RCSfile: ContentDescriptor.cpp,v $
|   $Revision: 1.11 $
|       $Date: 2011/11/18 15:50:56 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../STM/ImagePPHeaders.h"
#include <ScalableMesh/Import/ContentDescriptor.h>

#include <ScalableMesh/Import/Metadata.h>
#include <ScalableMesh/Import/Attachment.h>

#include <ScalableMesh/Import/SourceReference.h>
#include <STMInternal/Foundations/PrivateStringTools.h>

#include <ScalableMesh/Import/ContentConfig.h>
#include <ScalableMesh/Import/Config/Content/All.h>

#include <ScalableMesh/Foundations/Exception.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct LayerDescriptorImpl : ILayerDescriptor//public ShareableObjectTypeTrait<LayerDescriptorImpl>::type
    {
    typedef bvector<TypeDescriptor>             TypeList;
    //typedef bvector<SourceRef>                  SourceRefList;
 protected:
    WString                                  m_name;
    TypeList                                    m_storedTypes;
    bool                                        m_holdsIncompleteTypes;

    GCS                                         m_gcs;
    //std::auto_ptr<DRange3d>                     m_pExtent;
    DRange3d                                    m_extent;
    ScalableMeshData                            m_scalableMeshData;

    // NTERAY: Optimize by making this a pointer an lazy initialize it as it is optionnal.
    AttachmentRecord                            m_attachmentRecord;
    // NTERAY: Optimize by making this a pointer an lazy initialize it as it is optionnal.
    MetadataRecord                              m_metadataRecord;

    virtual const WString&               _GetName() const override
        {
        return m_name;
        }

    virtual const bvector<TypeDescriptor>& _GetTypes() const override
        {
        return m_storedTypes;
        }

    virtual bool                            _HoldsIncompleteTypes() const override
        {
        return m_holdsIncompleteTypes;
        }

    virtual const DRange3d&                 _GetExtent() const override
        {
        return m_extent;
        }

    virtual const ScalableMeshData&         _GetScalableMeshData() const override
        {
        return m_scalableMeshData;
        }

    virtual const GCS&                      _GetGCS() const override
        {
        return m_gcs;
        }

    virtual const AttachmentRecord&         _GetAttachmentRecord() const override
        {
        return m_attachmentRecord;
        }

    virtual const MetadataRecord&           _GetMetadataRecord() const override
        {
        return m_metadataRecord;
        }


    virtual ILayerDescriptor&               _SetScalableMeshData(const ScalableMeshData& data) override
        {
        m_scalableMeshData = data;
        return *this;
        }

    virtual ILayerDescriptor&               _SetAttachmentRecord(const AttachmentRecord& attachments) override
        {
        m_attachmentRecord = attachments;
        return *this;
        }

    virtual ILayerDescriptor&               _SetMetadataRecord(const MetadataRecord& metadata) override
        {
        m_metadataRecord = metadata;
        return *this;
        }

    virtual ILayerDescriptor&         _SetGCS(const GCS& gcs) override
        {
        m_gcs = gcs;
        return *this;
        }

public:

    explicit                               LayerDescriptorImpl(const WChar*                     pi_name,
                                                               const DataTypeSet&                  storedTypes,
                                                               const GCS&                          pi_rGCS,
                                                               const DRange3d*                     pi_pExtent,
                                                               const ScalableMeshData&             pi_data)
                                                               : m_name(pi_name),
                                                               m_storedTypes(storedTypes.begin(), storedTypes.end()),
                                                               m_holdsIncompleteTypes(HoldsIncompleteTypes(m_storedTypes)),
                                                               m_gcs(pi_rGCS),
                                                               //m_pExtent((0 != pi_pExtent) ? new DRange3d(*pi_pExtent) : 0),
                                                               m_extent(DRange3d::NullRange()),
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

    /*static void                                 UpdateForEdit                  (SharedPtrTypeTrait<LayerDescriptorImpl>::type&     
                                                                                                                    pi_rpInstance)
        {
        // Copy on write when config is shared
        if (pi_rpInstance->IsShared())
            pi_rpInstance = new LayerDescriptorImpl(*pi_rpInstance);
        }*/
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentDescriptorImpl : public ShareableObjectTypeTrait<ContentDescriptorImpl>::type
    {
    typedef bvector<RefCountedPtr<ILayerDescriptor>>             LayerList;

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
                                                                                const RefCountedPtr<ILayerDescriptor>&           layer,
                                                                                bool                             canRepresent3dData)
        :   m_name(name),
            m_holdsIncompleteTypes(layer->HoldsIncompleteTypes()), 
            m_canRepresent3dData(canRepresent3dData),
            m_isPod(layer->GetScalableMeshData().IsGroundDetection())
        {
        m_layers.push_back(layer);
        }

    static bool                                 HoldsIncompleteTypes           (const bvector<RefCountedPtr<ILayerDescriptor>>&      layers)
        {
        return layers.end() != std::find_if(layers.begin(), layers.end(), /*std::mem_fun_ref(&LayerDescriptorImpl::HoldsIncompleteTypes)*/
                                            [] (const RefCountedPtr<ILayerDescriptor>& layer)
            {
            return layer->HoldsIncompleteTypes();
            });
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
                                        const RefCountedPtr<ILayerDescriptor>& layer,
                                        bool                   canRepresent3dData)
    :   m_pImpl(new ContentDescriptorImpl(name, layer, canRepresent3dData))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::Add (const RefCountedPtr<ILayerDescriptor>& pi_pLayer)
    {
    ContentDescriptorImpl::UpdateForEdit(m_pImpl);
    m_pImpl->m_layers.push_back(pi_pLayer);
    m_pImpl->m_holdsIncompleteTypes |= pi_pLayer->HoldsIncompleteTypes();
    m_pImpl->m_isPod |= pi_pLayer->GetScalableMeshData().IsGroundDetection();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::push_back(const RefCountedPtr<ILayerDescriptor>& layer)
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
uint32_t ContentDescriptor::GetLayerCount () const
    {
    return (uint32_t) m_pImpl->m_layers.size();
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




RefCountedPtr<ILayerDescriptor> ILayerDescriptor::CreateLayerDescriptor(const WChar*                      name,
                                 const DataTypeSet&                  storedTypes,
                                 const GCS&                          rGCS,
                                 const DRange3d*                     extentP,
                                 const ScalableMeshData&             rData)
                                 
    {
    return new LayerDescriptorImpl(name, storedTypes, rGCS, extentP, rData);
    }


namespace {


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

}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SMStatus ContentDescriptor::Configure(const ContentConfig&    config,
                                                        Log&                    log)
    {
    return Configure(config, ContentConfigPolicy(), log);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SMStatus ContentDescriptor::Configure(const ContentConfig&            config,
                                                        const ContentConfigPolicy&   policy,
                                                        Log&                            log)
    {
    ContentDescriptorImpl::UpdateForEdit(m_pImpl);

    for (auto& layer : m_pImpl->m_layers)
        {
        layer->Configure(config);
        }
    return S_SUCCESS;
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
