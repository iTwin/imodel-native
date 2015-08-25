/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/ImportSequence.cpp $
|    $RCSfile: ImportSequence.cpp,v $
|   $Revision: 1.5 $
|       $Date: 2011/07/20 20:22:14 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/Import/ImportSequence.h>
#include <ScalableMesh/Import/Command/Base.h>


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportSequence::Impl : public ShareableObjectTypeTrait<ImportSequence::Impl>::type
    {
    typedef bvector<ImportCommand>
                            CommandList;
    CommandList             m_commands;

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
ImportSequence::ImportSequence ()
    :   m_pImpl(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImportSequence::~ImportSequence ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImportSequence::ImportSequence (const ImportSequence& pi_rRight)
    :   m_pImpl(pi_rRight.m_pImpl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImportSequence& ImportSequence::operator= (const ImportSequence& pi_rRight)
    {
    m_pImpl = pi_rRight.m_pImpl;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportSequence::push_back (const ImportCommand& pi_rConfig)
    {
    Impl::UpdateForEdit(m_pImpl);
    m_pImpl->m_commands.push_back(pi_rConfig);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportSequence::push_back (const ImportCommandBase& pi_rConfig)
    {
    Impl::UpdateForEdit(m_pImpl);
    m_pImpl->m_commands.push_back(ImportCommand(pi_rConfig));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportSequence::Accept (IImportSequenceVisitor& pi_rVisitor) const
    {
    struct AcceptVisitor
        {
        IImportSequenceVisitor& m_rVisitor;

        explicit AcceptVisitor (IImportSequenceVisitor& pi_rVisitor) : m_rVisitor(pi_rVisitor) {}

        void operator () (const ImportCommand& pi_rConfig) const
            {
            pi_rConfig.Accept(m_rVisitor);
            }
        };

    std::for_each(m_pImpl->m_commands.begin(), m_pImpl->m_commands.end(), AcceptVisitor(pi_rVisitor));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImportSequence::IsEmpty () const
    {
    return m_pImpl->m_commands.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ImportSequence::GetCount  () const
    {
    return m_pImpl->m_commands.size();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportCommand::ImportCommand (const ImportCommandBase& command)
    :   m_basePtr(command._Clone())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportCommand::ImportCommand (const ImportCommand& rhs)
    :   m_basePtr(rhs.m_basePtr)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportCommand& ImportCommand::operator= (const ImportCommand& rhs)
    {
    m_basePtr = rhs.m_basePtr;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportCommand::~ImportCommand ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportCommand::ClassID ImportCommand::GetClassID () const
    {
    return m_basePtr->_GetClassID();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportCommand::Accept (IImportSequenceVisitor& visitor) const
    {
    m_basePtr->_Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportCommandBase::ImportCommandBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportCommandBase::~ImportCommandBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportCommandBase::ImportCommandBase (const ImportCommandBase& rhs)
    {
    }

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
