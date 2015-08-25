/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourcesImport.cpp $
|    $RCSfile: ScalableMeshSourcesImport.cpp,v $
|   $Revision: 1.19 $
|       $Date: 2011/10/20 18:47:55 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include "ScalableMeshSourcesImport.h"

#include <ScalableMesh/Import/Source.h>
#include <ScalableMesh/Import/Importer.h>

#include <ScalableMesh/Import/ContentDescriptor.h>
#include <ScalableMesh/Import/Attachment.h>

#include <ScalableMesh/Import/DataTypeFamily.h>
#include <ScalableMesh/Import/SourceReferenceVisitor.h>

#include <ScalableMesh/Import/ImportSequenceVisitor.h>
#include <ScalableMesh/Import/Command/All.h>

#include "../Import/Sink.h"

#include <ScalableMesh/IScalableMeshPolicy.h>
   

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
     
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourcesImporter::Impl
    {
    struct SourceItem
        {
        SourceRef                   m_sourceRef;
        ContentConfig               m_contentConfig;
        ImportConfig                m_importConfig;
        ImportSequence              m_importSequence;
        SourceImportConfig*         m_sourceImportConf;

        explicit                    SourceItem                         (const SourceRef&                        sourceRef,
                                                                        const ContentConfig&                    contentConfig,
                                                                        const ImportConfig&                     importConfig,
                                                                        const ImportSequence&                   importSequence,
                                                                        SourceImportConfig&                     sourceImportConf)
            :   m_sourceRef(sourceRef),
                m_contentConfig(contentConfig),
                m_importConfig(importConfig),
                m_importSequence(importSequence),
                m_sourceImportConf(&sourceImportConf)
            {
            }

        };

    const LocalFileSourceRef        m_sinkSourceRef;
    SinkPtr                         m_sinkPtr;

    typedef vector<SourceItem>      SourceList;
    SourceList                      m_sources;

    explicit                        Impl                               (const LocalFileSourceRef&               sinkSourceRef,
                                                                        const SinkPtr&                          sinkPtr)
        :   m_sinkSourceRef(sinkSourceRef),
            m_sinkPtr(sinkPtr)
        {
        }


    Status                          Import                             ();

    Status                          ImportSource                       (SourceItem&                       sourceItem,
                                                                        SourcesImporter&                        attachmentsImporter);

    void                            AddAttachments                     (const Source&                           source,
                                                                        SourceItem&                       sourceItem,
                                                                        SourcesImporter&                        attachmentsImporter);

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesImporter::SourcesImporter   (const LocalFileSourceRef&   sinkSourceRef,
                                    const SinkPtr&              storagePtr)
    :   m_implP(new Impl(sinkSourceRef, storagePtr))
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesImporter::~SourcesImporter ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourcesImporter::AddSource    (const SourceRef&        sourceRef,
                                    const ContentConfig&    contentConfig,
                                    const ImportConfig&     config,
                                    const ImportSequence&   sequence,
                                    SourceImportConfig&     sourceImportConf)
    {

    ScalableMeshData data = sourceImportConf.GetReplacementSMData();

    if (data.IsRepresenting3dData() == SMis3D::is3D)
        {
        m_implP->m_sources.insert(m_implP->m_sources.begin(), Impl::SourceItem(sourceRef, contentConfig, config, sequence, sourceImportConf));
        }
    else
        {
        m_implP->m_sources.push_back(Impl::SourceItem(sourceRef, contentConfig, config, sequence, sourceImportConf));
        }            
    }

bool SourcesImporter::IsEmpty () const
    {
    return m_implP->m_sources.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesImporter::Status SourcesImporter::Import () const
    {
    return m_implP->Import();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesImporter::Status SourcesImporter::Impl::Import ()
    {
    SourcesImporter attachmentsImporter(m_sinkSourceRef, m_sinkPtr);

    for (SourceList::iterator sourceIt = m_sources.begin(), sourcesEnd = m_sources.end(); sourceIt != sourcesEnd; ++sourceIt)
        {
        SourcesImporter::Status status = ImportSource(*sourceIt, attachmentsImporter);
        if (S_SUCCESS != status)
            return S_ERROR;
        }

    if (attachmentsImporter.IsEmpty())
        return S_SUCCESS;

    Status attachmentImportStatus = attachmentsImporter.Import();
    if (S_SUCCESS != attachmentImportStatus)
        return S_ERROR;

    return S_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesImporter::Status SourcesImporter::Impl::ImportSource   (SourceItem&    sourceItem,
                                                               SourcesImporter&     attachmentsImporter)
    {
    static const SourceFactory SOURCE_FACTORY(GetSourceFactory());
    static const ImporterFactory IMPORTER_FACTORY(GetImporterFactory());

    const SourcePtr originalSourcePtr(SOURCE_FACTORY.Create(sourceItem.m_sourceRef));
    if (0 == originalSourcePtr.get())
        return S_ERROR;

    const SourcePtr sourcePtr(Configure(originalSourcePtr, 
                                        sourceItem.m_contentConfig,
                                        GetLog()));
    if (0 == sourcePtr.get())
        return S_ERROR;

    sourcePtr->SetImportConfig(sourceItem.m_sourceImportConf);

    const ImporterPtr importerPtr = IMPORTER_FACTORY.Create(sourcePtr, 
                                                            m_sinkPtr);
    if (0 == importerPtr.get())
        return S_ERROR;

    const Importer::Status importStatus = importerPtr->Import(sourceItem.m_importSequence, 
                                                              sourceItem.m_importConfig);

    sourceItem.m_sourceImportConf = sourcePtr->GetSourceImportConfig();

    if (importStatus != Importer::S_SUCCESS)
        return S_ERROR;


    AddAttachments(*sourcePtr, sourceItem, attachmentsImporter);

    return S_SUCCESS;
    }

namespace {


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportSequence              CreateAttachmentImportSequence                 (const ImportSequence&           sequence,
                                                                            UInt                            parentLayer)
    {
    class CommandVisitor : public IImportSequenceVisitor
        {
        const UInt                      m_parentLayer;
        ImportSequence                  m_sequence;

        virtual void                    _Visit                     (const ImportAllCommand&                     command) override
            {
            m_sequence.push_back(command);
            }
        virtual void                    _Visit                     (const ImportAllToLayerCommand&              command) override
            {
            m_sequence.push_back(command);
            }
        virtual void                    _Visit                     (const ImportAllToLayerTypeCommand&          command) override
            {
            m_sequence.push_back(command);
            }
        virtual void                    _Visit                     (const ImportAllToTypeCommand&               command) override
            {
            m_sequence.push_back(command);
            }

        virtual void                    _Visit                     (const ImportLayerCommand&                   command) override
            {
            if (m_parentLayer == command.GetSourceLayer())
                m_sequence.push_back(ImportAllCommand());
            }
        virtual void                    _Visit                     (const ImportLayerToLayerCommand&            command) override
            {
            if (m_parentLayer == command.GetSourceLayer())
                m_sequence.push_back(ImportAllToLayerCommand(command.GetTargetLayer()));
            }
        virtual void                    _Visit                     (const ImportLayerToLayerTypeCommand&        command) override
            {
            if (m_parentLayer == command.GetSourceLayer())
                m_sequence.push_back(ImportAllToLayerTypeCommand(command.GetTargetLayer(), command.GetTargetType()));
            }
        virtual void                    _Visit                     (const ImportLayerToTypeCommand&             command) override
            {
            if (m_parentLayer == command.GetSourceLayer())
                m_sequence.push_back(ImportAllToTypeCommand(command.GetTargetType()));
            }

        virtual void                    _Visit                     (const ImportLayerTypeCommand&               command) override
            {
            if (m_parentLayer == command.GetSourceLayer())
                m_sequence.push_back(ImportTypeCommand(command.GetSourceType()));
            }
        virtual void                    _Visit                     (const ImportLayerTypeToLayerCommand&        command) override
            {
            if (m_parentLayer == command.GetSourceLayer())
                m_sequence.push_back(ImportTypeToLayerCommand(command.GetSourceType(), command.GetTargetLayer()));
            }
        virtual void                    _Visit                     (const ImportLayerTypeToLayerTypeCommand&    command) override
            {
            if (m_parentLayer == command.GetSourceLayer())
                m_sequence.push_back(ImportTypeToLayerTypeCommand(command.GetSourceType(), command.GetTargetLayer(), command.GetTargetType()));
            }
        virtual void                    _Visit                     (const ImportLayerTypeToTypeCommand&         command) override
            {
            if (m_parentLayer == command.GetSourceLayer())
                m_sequence.push_back(ImportTypeToTypeCommand(command.GetSourceType(), command.GetTargetType()));
            }

        virtual void                    _Visit                     (const ImportTypeCommand&                    command) override
            {
            return m_sequence.push_back(command);
            }
        virtual void                    _Visit                     (const ImportTypeToLayerCommand&             command) override
            {
            return m_sequence.push_back(command);
            }
        virtual void                    _Visit                     (const ImportTypeToLayerTypeCommand&         command) override
            {
            return m_sequence.push_back(command);
            }
        virtual void                    _Visit                     (const ImportTypeToTypeCommand&              command) override
            {
            return m_sequence.push_back(command);
            }

    public:
        explicit                        CommandVisitor             (UInt                                        parentLayer)
            :   m_parentLayer(parentLayer)
            {
            }

        const ImportSequence&           GetSequence                () const { return m_sequence; }
        };

    CommandVisitor visitor(parentLayer);
    sequence.Accept(visitor);
    return visitor.GetSequence();
    }
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourcesImporter::Impl::AddAttachments (const Source&       source,
                                            SourceItem&   sourceItem,
                                            SourcesImporter&    attachmentsImporter)
    {
    struct LocalFileRefVisitor : SourceRefVisitor
        {
        const LocalFileSourceRef* m_sourceRefP;

        explicit LocalFileRefVisitor() 
            : m_sourceRefP(0) 
            {
            }

        virtual void _Visit(const LocalFileSourceRef&   sourceRef) override
            {
            m_sourceRefP = &sourceRef;
            }

        virtual void _Visit(const DGNElementSourceRef&     sourceRef) override
            {
            const LocalFileSourceRef* localFileRefP = sourceRef.GetLocalFileP();
            if (0 != localFileRefP)
                m_sourceRefP = localFileRefP;
            }
        };

    // NTERAY: This is a bad way to do it. We should either let the importer import attachments or
    // visit the source's importSequence in order to generate the attachment's import sequence.

    const ContentDescriptor& contentDesc = source.GetDescriptor();


    for (ContentDescriptor::const_iterator layerIt = contentDesc.LayersBegin(), layersEnd = contentDesc.LayersEnd(); 
         layerIt != layersEnd; 
         ++layerIt)
        {
        ImportSequence attachmentImportSequence(CreateAttachmentImportSequence(sourceItem.m_importSequence, contentDesc.GetLayerIDFor(layerIt)));

        if (attachmentImportSequence.IsEmpty())
            continue; // Nothing to import

        const AttachmentRecord& attachments = layerIt->GetAttachmentRecord();

        for (AttachmentRecord::const_iterator attachmentIt = attachments.begin(), attachmentsEnd = attachments.end();
             attachmentIt != attachmentsEnd;
             ++attachmentIt)
            {
            using namespace rel_ops;

            // Make sure STM and source are not the same.
            LocalFileRefVisitor visitor;
            attachmentIt->GetSourceRef().Accept(visitor);
            
            if (0 != visitor.m_sourceRefP && 
                *visitor.m_sourceRefP == m_sinkSourceRef)
                continue;

            attachmentsImporter.AddSource(attachmentIt->GetSourceRef(), sourceItem.m_contentConfig, sourceItem.m_importConfig, attachmentImportSequence, *sourceItem.m_sourceImportConf);
            }
        }
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
