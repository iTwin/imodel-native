/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourcesImport.cpp $
|    $RCSfile: ScalableMeshSourcesImport.cpp,v $
|   $Revision: 1.19 $
|       $Date: 2011/10/20 18:47:55 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
#include "ScalableMeshSourcesImport.h"

#include <ScalableMesh/Import/Source.h>
#include <ScalableMesh/Import/Importer.h>

#include <ScalableMesh/Import/ContentDescriptor.h>
#include <ScalableMesh/Import/Attachment.h>

#include <ScalableMesh/Import/DataTypeFamily.h>
#include <ScalableMesh/Import/SourceReferenceVisitor.h>


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
        RefCountedPtr<const ImportConfig>                m_importConfig;
        ImportSequence              m_importSequence;
        SourceImportConfig*         m_sourceImportConf;

        explicit                    SourceItem                         (const SourceRef&                        sourceRef,
                                                                        const ContentConfig&                    contentConfig,
                                                                        const ImportConfig*                     importConfig,
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
                                    const ImportConfig*     config,
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
                                                              *sourceItem.m_importConfig);

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
                                                                            uint32_t                            parentLayer)
    {
    ImportSequence seq;
    for (auto& command : sequence.GetCommands())
        {
        if (command.IsSourceLayerSet())
            {
            if (parentLayer == command.GetSourceLayer()) seq.push_back(command);
            }
        else seq.push_back(command);
        }
    return seq;
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

        const AttachmentRecord& attachments = (*layerIt)->GetAttachmentRecord();

        for (AttachmentRecord::const_iterator attachmentIt = attachments.begin(), attachmentsEnd = attachments.end();
             attachmentIt != attachmentsEnd;
             ++attachmentIt)
            {
            using namespace rel_ops;

            // Make sure STM and source are not the same.
            //LocalFileRefVisitor visitor;
            //attachmentIt->GetSourceRef().Accept(visitor);
            LocalFileSourceRef* refP = dynamic_cast<LocalFileSourceRef*>(attachmentIt->GetSourceRef().m_basePtr.get());

            if (0 != refP &&
                *refP == m_sinkSourceRef)
                continue;

            attachmentsImporter.AddSource(attachmentIt->GetSourceRef(), sourceItem.m_contentConfig, sourceItem.m_importConfig.get(), attachmentImportSequence, *sourceItem.m_sourceImportConf);
            }
        }
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
