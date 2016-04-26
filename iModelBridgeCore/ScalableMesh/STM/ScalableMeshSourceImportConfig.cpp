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

#include <ScalableMesh/Import/Command/All.h>
#include <ScalableMesh/Import/ImportSequenceVisitor.h>

#include <ScalableMesh/Import/Config/Content/All.h>

#include <ScalableMesh/Import/ContentConfigVisitor.h>

#include <ScalableMesh/Type/IScalableMeshLinear.h>
#include <ScalableMesh/Type/IScalableMeshPoint.h>
#include <ScalableMesh/Type/IScalableMeshTIN.h>
#include <ScalableMesh/Type/IScalableMeshMesh.h>

#include "ScalableMeshEditListener.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceImportConfig::Impl
    {
    EditListener*                   m_editListenerP;
    ImportSequence                  m_sequence;
    ImportConfig                    m_config;
    ContentConfig                   m_contentConfig;
    

    const ImportSequence            m_defaultSequence;
    bool                            m_userSpecifiedSequence;

    explicit                        Impl                   (const ImportSequence&       defaultSequence)
        :   m_editListenerP(0),
            m_sequence(defaultSequence),
            m_defaultSequence(defaultSequence),
            m_userSpecifiedSequence(false)
        {
        }

    explicit                        Impl                   (const Impl&                 rhs)
        :   m_editListenerP(0),
            m_sequence(rhs.m_sequence),
            m_config(rhs.m_config),
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
    assert(!" Are we calling this?");
#if 0
    class CommandVisitor : public IImportSequenceVisitor
        {
        const uint32_t                      m_importedLayer;
        ImportSequence&                 m_sequence;

        virtual void                    _Visit                     (const ImportAllCommand&                     command) override
            {
            m_sequence.push_back(ImportLayerCommand(m_importedLayer));
            }
        virtual void                    _Visit                     (const ImportAllToLayerCommand&              command) override
            {
            m_sequence.push_back(ImportLayerToLayerCommand(m_importedLayer, command.GetTargetLayer()));
            }
        virtual void                    _Visit                     (const ImportAllToLayerTypeCommand&          command) override
            {
            m_sequence.push_back(ImportLayerToLayerTypeCommand(m_importedLayer, command.GetTargetLayer(), command.GetTargetType()));
            }
        virtual void                    _Visit                     (const ImportAllToTypeCommand&               command) override
            {
            m_sequence.push_back(ImportLayerToTypeCommand(m_importedLayer, command.GetTargetType()));
            }

        virtual void                    _Visit                     (const ImportLayerCommand&                   command) override
            {
            if (m_importedLayer == command.GetSourceLayer())
                m_sequence.push_back(command);
            }
        virtual void                    _Visit                     (const ImportLayerToLayerCommand&            command) override
            {
            if (m_importedLayer == command.GetSourceLayer())
                m_sequence.push_back(command);
            }
        virtual void                    _Visit                     (const ImportLayerToLayerTypeCommand&        command) override
            {
            if (m_importedLayer == command.GetSourceLayer())
                m_sequence.push_back(command);
            }
        virtual void                    _Visit                     (const ImportLayerToTypeCommand&             command) override
            {
            if (m_importedLayer == command.GetSourceLayer())
                m_sequence.push_back(command);
            }

        virtual void                    _Visit                     (const ImportLayerTypeCommand&               command) override
            {
            if (m_importedLayer == command.GetSourceLayer())
                m_sequence.push_back(command);
            }
        virtual void                    _Visit                     (const ImportLayerTypeToLayerCommand&        command) override
            {
            if (m_importedLayer == command.GetSourceLayer())
                m_sequence.push_back(command);
            }
        virtual void                    _Visit                     (const ImportLayerTypeToLayerTypeCommand&    command) override
            {
            if (m_importedLayer == command.GetSourceLayer())
                m_sequence.push_back(command);
            }
        virtual void                    _Visit                     (const ImportLayerTypeToTypeCommand&         command) override
            {
            if (m_importedLayer == command.GetSourceLayer())
                m_sequence.push_back(command);
            }

        virtual void                    _Visit                     (const ImportTypeCommand&                    command) override
            {
            return m_sequence.push_back(ImportLayerTypeCommand(m_importedLayer, command.GetSourceType()));
            }
        virtual void                    _Visit                     (const ImportTypeToLayerCommand&             command) override
            {
            return m_sequence.push_back(ImportLayerTypeToLayerCommand(m_importedLayer, command.GetSourceType(), 
                                                                      command.GetTargetLayer()));
            }
        virtual void                    _Visit                     (const ImportTypeToLayerTypeCommand&         command) override
            {
            return m_sequence.push_back(ImportLayerTypeToLayerTypeCommand(m_importedLayer, command.GetSourceType(), 
                                                                          command.GetTargetLayer(), command.GetTargetType()));
            }
        virtual void                    _Visit                     (const ImportTypeToTypeCommand&              command) override
            {
            return m_sequence.push_back(ImportLayerTypeToTypeCommand(m_importedLayer, command.GetSourceType(), 
                                                                     command.GetTargetType()));
            }

    public:
        explicit                        CommandVisitor             (uint32_t                                        importedLayer,
                                                                    ImportSequence&                             sequence)
            :   m_importedLayer(importedLayer),
                m_sequence(sequence)
            {
            }

        };

    CommandVisitor visitor(importedLayer, sequence);
    templateCommands.Accept(visitor);
#endif
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
    m_implP->m_contentConfig.RemoveAllOfType(TypeConfig::s_GetClassID());
    m_implP->m_contentConfig.push_back(TypeConfig(type));
    }

void SourceImportConfig::SetReplacementSMData(const Import::ScalableMeshData& data)
{
    m_implP->OnPublicEdit();
    m_implP->m_contentConfig.RemoveAllOfType(ScalableMeshConfig::s_GetClassID());
    m_implP->m_contentConfig.push_back(ScalableMeshConfig(data));
}

const ScalableMeshData& SourceImportConfig::GetReplacementSMData() const
{
    struct SMDataVisitor : ContentConfigVisitor
    {
        const ScalableMeshData*      m_foundSMData;
        explicit        SMDataVisitor() : m_foundSMData(&ScalableMeshData::GetNull()) {}

        virtual void    _Visit(const ScalableMeshConfig& config) override
        {
            m_foundSMData = &config.GetScalableMeshData();
        }

    };

    SMDataVisitor visitor;
    if (m_implP.get() == nullptr) return ScalableMeshData::GetNull();
    m_implP->m_contentConfig.Accept(visitor);

    assert(0 != visitor.m_foundSMData);
    return *visitor.m_foundSMData;
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceImportConfig::SetReplacementGCS (const GCS& gcs)
    {
    m_implP->OnPublicEdit();
    m_implP->m_contentConfig.RemoveAllOfType(GCSConfig::s_GetClassID());
    m_implP->m_contentConfig.RemoveAllOfType(GCSExtendedConfig::s_GetClassID());

    m_implP->m_contentConfig.push_back(GCSConfig(gcs));
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
    m_implP->m_contentConfig.RemoveAllOfType(GCSConfig::s_GetClassID());
    m_implP->m_contentConfig.RemoveAllOfType(GCSExtendedConfig::s_GetClassID());

    //Always want to use the extented config for wktFlavor
    m_implP->m_contentConfig.push_back
        (
        GCSExtendedConfig(gcs).PrependToExistingLocalTransform(prependToExistingLocalTransform).
                               PreserveExistingIfGeoreferenced(preserveExistingIfGeoreferenced).
                               PreserveExistingIfLocalCS(preserveExistingIfLocalCS)
        );
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const GCS& SourceImportConfig::GetReplacementGCS ()
    {
    struct GCSVisitor : ContentConfigVisitor
        {
        const GCS*      m_foundGCSP;
        explicit        GCSVisitor             ()   :   m_foundGCSP(&GCS::GetNull()) {}

        virtual void    _Visit                  (const GCSConfig& config) override
            {
            m_foundGCSP = &config.GetGCS();
            }
        virtual void    _Visit                  (const GCSExtendedConfig& config) override
            {
            m_foundGCSP = &config.GetGCS();
            }

        };

    GCSVisitor visitor;
    m_implP->m_contentConfig.Accept(visitor);

    assert(0 != visitor.m_foundGCSP);
    return *visitor.m_foundGCSP;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                    Raymond.Gauthier  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const ImportConfig& SourceImportConfig::GetConfig () const
    {
    return m_implP->m_config;
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
void SourceImportConfig::SetConfig (const ImportConfig& config)
    {
    m_implP->OnPublicEdit();

    m_implP->m_config = config;
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
void SourceImportConfig::SetInternalConfig (const Import::ImportConfig& config)
    {
    m_implP->m_config = config;
    }



END_BENTLEY_SCALABLEMESH_NAMESPACE
