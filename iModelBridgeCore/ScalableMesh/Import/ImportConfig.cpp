/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/ImportConfig.cpp $
|    $RCSfile: ImportConfig.cpp,v $
|   $Revision: 1.6 $
|       $Date: 2011/07/20 20:22:18 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include <ScalableMesh/Import/ImportConfig.h>
#include <ScalableMesh/Import/Config/Base.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportConfig::Impl : public ShareableObjectTypeTrait<ImportConfig::Impl>::type
    {
    typedef bvector<ImportConfigComponent>
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
ImportConfig::ImportConfig ()
    :   m_pImpl(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImportConfig::~ImportConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImportConfig::ImportConfig (const ImportConfig& pi_rRight)
    :   m_pImpl(pi_rRight.m_pImpl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImportConfig& ImportConfig::operator= (const ImportConfig& pi_rRight)
    {
    m_pImpl = pi_rRight.m_pImpl;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportConfig::push_back (const ImportConfigComponent& pi_rConfig)
    {
    Impl::UpdateForEdit(m_pImpl);

    m_pImpl->m_components.push_back(pi_rConfig);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportConfig::push_back (const ImportConfigComponentBase& pi_rConfig)
    {
    Impl::UpdateForEdit(m_pImpl);

    m_pImpl->m_components.push_back(ImportConfigComponent(pi_rConfig));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportConfig::Accept (IImportConfigVisitor& pi_rVisitor) const
    {
    struct AcceptVisitor
        {
        IImportConfigVisitor& m_rVisitor;

        explicit AcceptVisitor (IImportConfigVisitor& pi_rVisitor) : m_rVisitor(pi_rVisitor) {}

        void operator () (const ImportConfigComponent& pi_rConfig) const
            {
            pi_rConfig.Accept(m_rVisitor);
            }
        };

    std::for_each(m_pImpl->m_components.begin(), m_pImpl->m_components.end(), AcceptVisitor(pi_rVisitor));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportConfig::RemoveAllOfType (ComponentClassID classID)
    {
    struct HasClassID
        {
        ComponentClassID        m_classID;
        explicit                HasClassID                 (ComponentClassID                classID) : m_classID(classID) {}

        bool                    operator()                 (const ImportConfigComponent&    rhs) const
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
ImportConfigComponent::ImportConfigComponent (const ImportConfigComponentBase& config)
    :   m_basePtr(config._Clone()),
        m_classID(config._GetClassID())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportConfigComponent::ImportConfigComponent (const ImportConfigComponent& rhs)
    :   m_basePtr(rhs.m_basePtr),
        m_classID(rhs.m_classID)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportConfigComponent& ImportConfigComponent::operator= (const ImportConfigComponent& rhs)
    {
    m_basePtr = rhs.m_basePtr;
    m_classID = rhs.m_classID;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportConfigComponent::~ImportConfigComponent ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportConfigComponent::Accept (IImportConfigVisitor& visitor) const
    {
    m_basePtr->_Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportConfigComponentBase::ImportConfigComponentBase ()
    :   m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportConfigComponentBase::~ImportConfigComponentBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportConfigComponentBase::ImportConfigComponentBase (const ImportConfigComponentBase& rhs)
    :   m_implP(0)
    {
    }

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
