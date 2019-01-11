/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/InputExtractor.cpp $
|    $RCSfile: InputExtractor.cpp,v $
|   $Revision: 1.9 $
|       $Date: 2012/02/16 00:37:05 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include "SourcePlugin.h"
#include "InputExtractor.h"

#include "Sink.h"

#include <ScalableTerrainModel/Import/Plugin/InputExtractorV0.h>
#include <ScalableTerrainModel/Import/Plugin/SourceV0.h>

#include "PluginRegistryHelper.h"
#include "InternalSourceHandler.h"

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct InputExtractorBase::Impl
    {   
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InputExtractorBase::InputExtractorBase ()
    :   m_pImpl(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InputExtractorBase::~InputExtractorBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool InputExtractorBase::_Finalize ()
    {
    return true;/*Default empty implementation*/
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct InputExtractorCreatorBase::Impl
    {
    explicit                                Impl                                   ()
        {
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InputExtractorCreatorBase::InputExtractorCreatorBase ()
    :   m_pImpl(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InputExtractorCreatorBase::~InputExtractorCreatorBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void InputExtractorCreatorBase::_AdaptOutputType (DataType& type) const
    {
    /* Do nothing */
    }


END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE




BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InputExtractor::InputExtractor (Base* implP) 
    :   m_baseP(implP) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InputExtractor::~InputExtractor ()
    {
    try 
        {
        m_baseP->_Finalize();
        }
    catch (...)
        {
        assert(!"Finalize threw!");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void InputExtractor::Read ()
    {
    m_baseP->_Read();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool InputExtractor::Next ()
    {
    return m_baseP->_Next();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InputExtractorCreator::InputExtractorCreator (const Base& impl)
    :   m_baseP(&impl),
        m_sourceClassID(impl._GetSourceClassID())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InputExtractorCreator::~InputExtractorCreator () 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool InputExtractorCreator::Supports (const DataType& pi_rType) const
    {
    return m_baseP->_Supports(pi_rType);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void InputExtractorCreator::AdaptOutputType (DataType& type) const
    {
    return m_baseP->_AdaptOutputType(type);
    }

RawCapacities InputExtractorCreator::GetOutputCapacities   (Source&                 source,
                                                            const ExtractionQuery& selection) const
    {
    return m_baseP->_GetOutputCapacities(InternalSourceHandler::GetOriginalBaseFor(source), 
                                         source, 
                                         selection);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InputExtractorPtr InputExtractorCreator::Create    (Source&                 source,
                                                    const ExtractionQuery& selection,
                                                    PacketGroup&            rawEntities,
                                                    const ExtractionConfig& config,
                                                    Log&             log) const
    {
    std::auto_ptr<InputExtractor::Base> pImporter(m_baseP->_Create(InternalSourceHandler::GetOriginalBaseFor(source), 
                                                                   source, 
                                                                   selection, 
                                                                   config, 
                                                                   log));
    if (0 == pImporter.get())
        return 0;

    pImporter->_Assign(rawEntities);

    return new InputExtractor(pImporter.release());
    }






END_BENTLEY_MRDTM_IMPORT_NAMESPACE


BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExtractorRegistryImpl : public PluginRegistry<InputExtractorCreator>
    {

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExtractorRegistry::ExtractorRegistry ()
    :   m_implP(new ExtractorRegistryImpl)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExtractorRegistry::~ExtractorRegistry ()
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExtractorRegistry& ExtractorRegistry::GetInstance ()
    {
    static ExtractorRegistry SINGLETON;
    return SINGLETON;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExtractorRegistry::CreatorRange ExtractorRegistry::FindAppropriateCreator (SourceClassID pi_sourceClassID) const
    {
    struct CreatorLess : std::binary_function<InputExtractorCreator, InputExtractorCreator, bool>
        {
        bool operator () (const InputExtractorCreator& lhs, const InputExtractorCreator& rhs) const
            { return lhs.GetSourceClassID() < rhs.GetSourceClassID(); }

        bool operator () (const InputExtractorCreator& lhs, InputExtractorCreator::SourceClassID rhs) const
            { return lhs.GetSourceClassID() < rhs; }

        bool operator () (InputExtractorCreator::SourceClassID lhs, const InputExtractorCreator& rhs) const
            { return lhs < rhs.GetSourceClassID(); }
        };


    ExtractorRegistryImpl::CreatorListRange range = m_implP->FindCreatorsFor(pi_sourceClassID, CreatorLess());

    return std::make_pair(&*range.first, &*range.second);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExtractorRegistry::V0ID ExtractorRegistry::Register (const V0Creator& creator)
    {
    return m_implP->Register(InputExtractorCreator(creator)); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtractorRegistry::Unregister (V0ID creatorID)
    {
    m_implP->Unregister(creatorID);
    }


END_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE
