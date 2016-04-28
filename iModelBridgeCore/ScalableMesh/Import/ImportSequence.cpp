/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/ImportSequence.cpp $
|    $RCSfile: ImportSequence.cpp,v $
|   $Revision: 1.5 $
|       $Date: 2011/07/20 20:22:14 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/Import/ImportSequence.h>
#include <ScalableMesh/Import/Command/Base.h>
#include <ScalableMesh\Type\IScalableMeshPoint.h>


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


bvector<ImportCommand>& ImportSequence::GetCommands() const
    {
    return m_pImpl->m_commands;
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


uint32_t                                 ImportCommand::GetSourceLayer() const
    {
    return m_basePtr->m_sourceLayer;
    }

uint32_t                                 ImportCommand::GetTargetLayer() const
    {
    return m_basePtr->m_targetLayer;
    }

const DataTypeFamily&                    ImportCommand::GetSourceType() const
    {
    return m_basePtr->m_sourceType;
    }

const DataTypeFamily&                    ImportCommand::GetTargetType() const
    {
    return m_basePtr->m_targetType;
    }

bool                                 ImportCommand::IsSourceLayerSet() const
    {
    return m_basePtr->m_sourceLayerSet;
    }

bool                                 ImportCommand::IsTargetLayerSet() const
    {
    return m_basePtr->m_targetLayerSet;
    }

bool                                 ImportCommand::IsSourceTypeSet() const
    {
    return m_basePtr->m_sourceTypeSet;
    }

bool                                 ImportCommand::IsTargetTypeSet() const
    {
    return m_basePtr->m_targetTypeSet;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportCommandBase::ImportCommandBase ():
m_sourceType(PointTypeFamilyCreator().Create()), m_targetType(PointTypeFamilyCreator().Create())
    {
    m_sourceTypeSet = false;
    m_targetTypeSet = false;
    m_sourceLayerSet = false;
    m_targetLayerSet = false;
    }

ImportCommandBase::ImportCommandBase(const ImportCommand& cmd) :
m_sourceType(PointTypeFamilyCreator().Create()), m_targetType(PointTypeFamilyCreator().Create())
    {
    if (cmd.IsSourceLayerSet())
        {
        m_sourceLayerSet = true;
        m_sourceLayer = cmd.GetSourceLayer();
        }
    else m_sourceLayerSet = false;
    if (cmd.IsTargetLayerSet())
        {
        m_targetLayerSet = true;
        m_targetLayer = cmd.GetTargetLayer();
        }
    else m_targetLayerSet = false;

    if (cmd.IsSourceTypeSet())
        {
        m_sourceTypeSet = true;
        m_sourceType = cmd.GetSourceType();
        }
    else m_sourceLayerSet = false;
    if (cmd.IsTargetLayerSet())
        {
        m_targetTypeSet = true;
        m_targetType = cmd.GetTargetType();
        }
    else m_targetTypeSet = false;
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
    :
    m_sourceType(rhs.m_sourceType), m_targetType(rhs.m_targetType), m_sourceLayer(rhs.m_sourceLayer), m_targetLayer(rhs.m_targetLayer), m_sourceTypeSet(rhs.m_sourceTypeSet),
    m_targetTypeSet(rhs.m_targetTypeSet), m_sourceLayerSet(rhs.m_sourceLayerSet), m_targetLayerSet(rhs.m_targetLayerSet)
    {
    }

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
