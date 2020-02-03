/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include "InputExtractor.h"
#include <ScalableMesh/Import/Plugin/ExtractorRegistry.h>
#include "InternalImporterConfig.h"
#include "InternalContentDescriptor.h"

#include <ScalableMesh/Import/Source.h>
#include "Sink.h"
#include <ScalableMesh/Import/FilterFactory.h>

#include <ScalableMesh/Import/ImportPolicy.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct ImportSequence;
struct ImportCommand;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImporterImpl
    {
    typedef bvector<InputExtractorCreator>
                                PluginCreatorList;

    PluginCreatorList           m_pluginCreators;
    SourceCPtr                  m_pSource;
    SinkPtr                     m_sinkPtr;

    std::auto_ptr<TypeSelectionPolicy>
                                m_sourceTypeSelectionPolicy;
    std::auto_ptr<TypeSelectionPolicy>
                                m_targetTypeSelectionPolicy;
    std::auto_ptr<MemoryAllocator>   m_allocatorP;

    Internal::ContentDesc       m_sourceDesc;
    Internal::ContentDesc       m_targetDesc;

    FilterFactory               m_filterFactory;
    Log&                 m_warningLog;


    explicit                    ImporterImpl           (const Plugin::ExtractorRegistry::CreatorRange&
                                                                                            foundCreatorRange,
                                                        const SourceCPtr&                   sourcePtr,
                                                        const SinkPtr&                      sinkPtr,
                                                        const ImportPolicy&                 policy,
                                                        const FilterFactory&                filterFactory,
                                                        Log&                         log);

                                ~ImporterImpl          ();

    void                        CreateExtent           (DRange3d& range, PacketGroup& dstSource);
        void                    Filter(std::vector<DRange3d> vecRangeFilter, PacketGroup& dstSource);

    const MemoryAllocator&      GetAllocator           () const { return *m_allocatorP; }


    void                        Import                 (const ImportSequence&               sequence,
                                                        const Internal::Config&             config);

    void                        Import                 (const ImportCommand&                command,
                                                        const Internal::Config&             config);


    void                        Import                 (uint32_t                                sourceLayerID,
                                                        const DataTypeFamily&               sourceTypeFamily,
                                                        uint32_t                                targetLayerID,
                                                        const DataTypeFamily&               targetTypeFamily,
                                                        const Internal::Config&             config);

    void                        Import                 (uint32_t                                sourceLayerID,
                                                        const DataType&                     sourceType,
                                                        uint32_t                                targetLayerID,
                                                        const DataTypeFamily&               targetTypeFamily,
                                                        const Internal::Config&             config);

    void                        Import                 (uint32_t                                sourceLayerID,   
                                                        const DataType&                     sourceType,
                                                        uint32_t                                targetLayerID,   
                                                        const DataType&                     targetType,
                                                        const Internal::Config&             config);


    FilterCreatorCPtr           GetFilterCreatorFor    (uint32_t                                sourceLayerID,
                                                        const DataType&                     sourceType,
                                                        uint32_t                                targetLayerID,
                                                        const DataType&                     targetType,
                                                        const Internal::Config&             config);


    const InputExtractorCreator*
                                GetPluginCreatorFor    (const DataType&                     sourceType);

    static const Internal::Config&        
                                DefaultImportConfig    ();
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
