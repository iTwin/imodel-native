/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/ContentConfig.cpp $
|    $RCSfile: ContentConfig.cpp,v $
|   $Revision: 1.9 $
|       $Date: 2011/11/18 15:50:52 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include <ScalableMesh/Import/ContentConfig.h>
#include <ScalableMesh/Import/Config/Content/Base.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentConfig::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    typedef bvector<ContentConfigComponent>
                            ConfigPartList;
    ConfigPartList          m_components;

    explicit                Impl                   ()
        {
        }


    static void         UpdateForEdit                (ImplPtr&    pi_rpInstance)
        {
        // Copy on write when config is shared
        if (pi_rpInstance->IsShared())
            pi_rpInstance = new Impl(*pi_rpInstance);
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfig::ContentConfig ()
    :   m_pImpl(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfig::~ContentConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfig::ContentConfig (const ContentConfig& pi_rRight)
    :   m_pImpl(pi_rRight.m_pImpl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfig& ContentConfig::operator= (const ContentConfig& pi_rRight)
    {
    m_pImpl = pi_rRight.m_pImpl;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentConfig::push_back (const ContentConfigComponent& pi_rConfig)
    {
    Impl::UpdateForEdit(m_pImpl);
    m_pImpl->m_components.push_back(pi_rConfig);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentConfig::push_back (const ContentConfigComponentBase& pi_rConfig)
    {
    Impl::UpdateForEdit(m_pImpl);
    m_pImpl->m_components.push_back(ContentConfigComponent(pi_rConfig));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentConfig::Accept (IContentConfigVisitor& pi_rVisitor) const
    {
    struct AcceptVisitor
        {
        IContentConfigVisitor& m_rVisitor;

        explicit AcceptVisitor (IContentConfigVisitor& pi_rVisitor) : m_rVisitor(pi_rVisitor) {}
        void operator () (const ContentConfigComponent& pi_rConfig) const { pi_rConfig.Accept(m_rVisitor); }
        };

    std::for_each(m_pImpl->m_components.begin(), m_pImpl->m_components.end(), AcceptVisitor(pi_rVisitor));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentConfig::IsEmpty () const
    {
    return m_pImpl->m_components.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ContentConfig::GetCount () const
    {
    return m_pImpl->m_components.size();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentConfig::RemoveAllOfType (ComponentClassID classID)
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

    Impl::UpdateForEdit(m_pImpl);
    m_pImpl->m_components.erase(std::remove_if(m_pImpl->m_components.begin(), m_pImpl->m_components.end(), HasClassID(classID)),
                                m_pImpl->m_components.end());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigComponent::ContentConfigComponent (const ContentConfigComponentBase& config)
    :   m_basePtr(config._Clone()),
        m_classID(config._GetClassID())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigComponent::ContentConfigComponent (const ContentConfigComponent& rhs)
    :   m_basePtr(rhs.m_basePtr),
        m_classID(rhs.m_classID)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigComponent& ContentConfigComponent::operator= (const ContentConfigComponent& rhs)
    {
    m_basePtr = rhs.m_basePtr;
    m_classID = rhs.m_classID;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigComponent::~ContentConfigComponent ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentConfigComponent::Accept (IContentConfigVisitor& visitor) const
    {
    m_basePtr->_Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentConfigComponent::Accept (ILayerConfigVisitor& visitor) const
    {
    m_basePtr->_Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigComponentBase::ContentConfigComponentBase ()
    :   m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigComponentBase::~ContentConfigComponentBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigComponentBase::ContentConfigComponentBase (const ContentConfigComponentBase& rhs)
    :   m_implP(0)
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigPolicy::ContentConfigPolicy ()
    :   m_flags(0),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigPolicy::~ContentConfigPolicy ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigPolicy::ContentConfigPolicy (const ContentConfigPolicy& rhs)
    :   m_flags(rhs.m_flags),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigPolicy& ContentConfigPolicy::operator= (const ContentConfigPolicy& rhs)
    {
    m_flags = rhs.m_flags;
    return *this;
    }


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
