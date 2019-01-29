/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/ContentConfigs.cpp $
|    $RCSfile: ContentConfigs.cpp,v $
|   $Revision: 1.9 $
|       $Date: 2011/11/18 15:50:54 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>
#include <ScalableTerrainModel/Import/Config/Content/All.h>
#include "ContentConfigComponentMixinBaseImpl.h"

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

GCSConfig::ClassID GCSConfig::s_GetClassID () { return super_class::s_GetClassID(); }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCSConfig::GCSConfig   (const GCSConfig& rhs)
    :   super_class(rhs),
        m_gcs(rhs.m_gcs),
        m_implP(0)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCSConfig::GCSConfig (const GCS& gcs)
    :   m_gcs(gcs),
        m_implP(0)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCSConfig::~GCSConfig ()
    {

    }

GCSExtendedConfig::ClassID GCSExtendedConfig::s_GetClassID () { return super_class::s_GetClassID(); }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCSExtendedConfig::GCSExtendedConfig (const GCSExtendedConfig&  rhs)
    :   super_class(rhs),
        m_gcs(rhs.m_gcs),
        m_flags(rhs.m_flags),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCSExtendedConfig::GCSExtendedConfig (const GCS&    gcs)
    :   m_gcs(gcs),
        m_flags(0),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCSExtendedConfig::~GCSExtendedConfig ()
    {
    }


GCSLocalAdjustmentConfig::ClassID GCSLocalAdjustmentConfig::s_GetClassID ()  { return super_class::s_GetClassID(); }
   

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCSLocalAdjustmentConfig::GCSLocalAdjustmentConfig (const GCSLocalAdjustmentConfig& rhs)
    :   m_transform(rhs.m_transform),
        m_flags(rhs.m_flags),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCSLocalAdjustmentConfig::GCSLocalAdjustmentConfig (const LocalTransform&   transform)
    :   m_transform(transform),
        m_flags(0),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCSLocalAdjustmentConfig::~GCSLocalAdjustmentConfig ()
    {
    }


TypeConfig::ClassID TypeConfig::s_GetClassID () { return super_class::s_GetClassID(); }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConfig::TypeConfig (const TypeConfig& rhs)
    :   super_class(rhs),
        m_type(rhs.m_type),
        m_flags(rhs.m_flags),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConfig::TypeConfig (const DataType&  type)
    :   m_type(type),
        m_flags(0),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConfig::~TypeConfig ()
    {
    }


LayerConfig::ClassID LayerConfig::s_GetClassID () { return super_class::s_GetClassID(); }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct LayerConfig::Impl : public ShareableObjectTypeTrait<LayerConfig::Impl>::type
    {
    typedef bvector<ContentConfigComponent>
                            ComponentList;
    ComponentList           m_components;

    explicit                Impl                   ()
        {
        }

    static void         UpdateForEdit                (ImplPtr&    instancePtr)
        {
        // Copy on write when config is shared
        if (instancePtr->IsShared())
            instancePtr = new Impl(*instancePtr);
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerConfig::LayerConfig (UInt layerID)
    :   m_flags(0),
        m_layerID(layerID),
        m_implP(new Impl) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerConfig::LayerConfig (const LayerConfig& rhs)
    :   super_class(rhs),
        m_layerID(rhs.m_layerID),
        m_flags(rhs.m_flags),
        m_implP(rhs.m_implP)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerConfig::~LayerConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerConfig& LayerConfig::push_back (const ContentConfigComponent&  config)
    {
    Impl::UpdateForEdit(m_implP);
    m_implP->m_components.push_back(config);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LayerConfig& LayerConfig::push_back (const ContentConfigComponentBase&  config)
    {
    Impl::UpdateForEdit(m_implP);
    m_implP->m_components.push_back(ContentConfigComponent(config));
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void LayerConfig::Accept (ILayerConfigVisitor& visitor) const
    {
    struct AcceptVisitor
        {
        ILayerConfigVisitor& m_visitor;

        explicit AcceptVisitor (ILayerConfigVisitor& visitor) : m_visitor(visitor) {}
        void operator () (const ContentConfigComponent& config) const { config.Accept(m_visitor); }
        };

    std::for_each(m_implP->m_components.begin(), m_implP->m_components.end(), AcceptVisitor(visitor));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LayerConfig::IsEmpty () const
    {
    return m_implP->m_components.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t LayerConfig::GetCount () const
    {
    return m_implP->m_components.size();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void LayerConfig::RemoveAllOfType (ComponentClassID classID)
    {
    struct HasClassID
        {
        ComponentClassID        m_classID;
        explicit                HasClassID                 (ComponentClassID                classID) : m_classID(classID) {}

        bool                    operator()                 (const ContentConfigComponent&   rhs) const
            {
            return rhs.GetClassID() == m_classID;
            }
        };

    Impl::UpdateForEdit(m_implP);
    m_implP->m_components.erase(std::remove_if(m_implP->m_components.begin(), m_implP->m_components.end(), HasClassID(classID)),
                                m_implP->m_components.end());
    }

END_BENTLEY_MRDTM_IMPORT_NAMESPACE