/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourceImportConfig.cpp $
|    $RCSfile: ScalableMeshSourceImportConfig.cpp,v $
|   $Revision: 1.21 $
|       $Date: 2012/01/30 17:24:44 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>

#include <ScalableMesh/Import/ImportConfig.h>
#include <ScalableMesh/Import/ContentConfig.h>
#include <ScalableMesh/Import/ImportSequence.h>

#include <ScalableMesh/Import/Command/Base.h>

#include <ScalableMesh/Import/Config/Content/All.h>


#include <ScalableMesh/Type/IScalableMeshLinear.h>
#include <ScalableMesh/Type/IScalableMeshPoint.h>
#include <ScalableMesh/Type/IScalableMeshTIN.h>
#include <ScalableMesh/Type/IScalableMeshMesh.h>
#include "..\Import\InternalImporterConfig.h"
#include "ScalableMeshEditListener.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceImportConfig::Impl
    {
    friend class Internal::Config;
    EditListener*                   m_editListenerP;
    ImportSequence                  m_sequence;
    RefCountedPtr<ImportConfig>                    m_config;
    ContentConfig                   m_contentConfig;
    

    const ImportSequence            m_defaultSequence;
    bool                            m_userSpecifiedSequence;

    explicit                        Impl                   (const ImportSequence&       defaultSequence)
        :   m_editListenerP(0),
            m_sequence(defaultSequence),
            m_config(new Internal::Config()),
            m_defaultSequence(defaultSequence),
            m_userSpecifiedSequence(false)
        {
        }

    explicit                        Impl                   (const Impl&                 rhs)
        :   m_editListenerP(0),
            m_sequence(rhs.m_sequence),
            m_config(rhs.m_config.get()),
            m_contentConfig(rhs.m_contentConfig),
            m_defaultSequence(rhs.m_defaultSequence),
            m_userSpecifiedSequence(rhs.m_userSpecifiedSequence)
        {
        }



    void                            OnSequenceAddition     ()
        {
        if (!m_userSpecifiedSequence)
            {
            assert(!m_sequence.IsEmpty());
            m_sequence = ImportSequence(); // Reset sequence.
            m_userSpecifiedSequence = true;
            }
        }

    void                                OnPublicEdit       ()
        {
        if (0 != m_editListenerP)
            m_editListenerP->NotifyOfPublicEdit();
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceImportConfig::SourceImportConfig (const ImportSequence& defaultSequence)
    :   m_implP(new Impl(defaultSequence))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceImportConfig::SourceImportConfig (const SourceImportConfig& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceImportConfig::~SourceImportConfig ()
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceImportConfig::RegisterEditListener (EditListener& listener)
    {
    assert(0 == m_implP->m_editListenerP);
    m_implP->m_editListenerP = &listener;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceImportConfig::UnregisterEditListener (const EditListener& listener)
    {
    assert(m_implP->m_editListenerP == &listener);
    m_implP->m_editListenerP = 0;
    }

namespace {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void                        AppendImportLayerCommandsToExistingSequence    (const ImportSequence&           templateCommands,
                                                                            uint32_t                            importedLayer,
                                                                            ImportSequence&                 sequence)
    {
    for (auto& command : templateCommands.GetCommands())
        {
        ImportCommandBase commandWithLayer = command;
        if (!command.IsSourceLayerSet()) commandWithLayer.SetSourceLayer(importedLayer);
        if (!command.IsSourceLayerSet() || command.GetSourceLayer() == importedLayer) sequence.push_back(commandWithLayer);
        }

    }
}


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceImportConfig::AddImportedLayer (uint32_t layerID)
    {
    m_implP->OnPublicEdit();
    m_implP->OnSequenceAddition();
    AppendImportLayerCommandsToExistingSequence(m_implP->m_defaultSequence, layerID, m_implP->m_sequence);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceImportConfig::SetReplacementType (const Import::DataType& type)
    {
    m_implP->OnPublicEdit();

    m_implP->m_contentConfig.SetTypeConfig(TypeConfig(type));
    }

void SourceImportConfig::SetReplacementSMData(const Import::ScalableMeshData& data)
{
    m_implP->OnPublicEdit();

    m_implP->m_contentConfig.SetScalableMeshConfig(ScalableMeshConfig(data));

}

const ScalableMeshData& SourceImportConfig::GetReplacementSMData() const
{

    return m_implP->m_contentConfig.GetScalableMeshConfig().GetScalableMeshData();
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceImportConfig::SetReplacementGCS (const GCS& gcs)
    {
    m_implP->OnPublicEdit();

    GCSConfig config(gcs);
    m_implP->m_contentConfig.SetGCSConfig(config);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceImportConfig::SetReplacementGCS (const GeoCoords::GCS&   gcs,
                                            bool                    prependToExistingLocalTransform,
                                            bool                    preserveExistingIfGeoreferenced,
                                            bool                    preserveExistingIfLocalCS)
    {
    m_implP->OnPublicEdit();

    GCSConfig config(gcs);
    config.PrependToExistingLocalTransform(prependToExistingLocalTransform).
        PreserveExistingIfGeoreferenced(preserveExistingIfGeoreferenced).
        PreserveExistingIfLocalCS(preserveExistingIfLocalCS);
    m_implP->m_contentConfig.SetGCSConfig(config);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const GCS& SourceImportConfig::GetReplacementGCS ()
    {

    return  m_implP->m_contentConfig.GetGCSConfig().GetGCS();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportConfig* SourceImportConfig::GetConfig () const
    {
    return m_implP->m_config.get();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const ImportSequence& SourceImportConfig::GetSequence () const
    {
    assert(!m_implP->m_sequence.IsEmpty());
    return m_implP->m_sequence;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const ContentConfig& SourceImportConfig::GetContentConfig () const
    {
    return m_implP->m_contentConfig;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceImportConfig::SetContentConfig (const ContentConfig& config)
    {
    m_implP->OnPublicEdit();
    m_implP->m_contentConfig = config;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceImportConfig::SetSequence (const ImportSequence& sequence)
    {
    m_implP->OnPublicEdit();

    assert(!sequence.IsEmpty());
    m_implP->m_sequence = sequence;
    m_implP->m_userSpecifiedSequence = true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceImportConfig::SetConfig (const ImportConfig* config)
    {
    m_implP->OnPublicEdit();

    m_implP->m_config = const_cast<ImportConfig*>(config);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceImportConfig::SetInternalContentConfig (const Import::ContentConfig& config)
    {
    m_implP->m_contentConfig = config;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceImportConfig::SetInternalSequence (const Import::ImportSequence& sequence)
    {
    assert(!sequence.IsEmpty());
    m_implP->m_sequence = sequence;
    m_implP->m_userSpecifiedSequence = true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceImportConfig::SetInternalConfig (const Import::ImportConfig* config)
    {
    m_implP->m_config = const_cast<Import::ImportConfig*>(config);
    }



END_BENTLEY_SCALABLEMESH_NAMESPACE
