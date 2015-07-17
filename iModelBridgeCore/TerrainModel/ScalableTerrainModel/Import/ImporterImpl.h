/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/ImporterImpl.h $
|    $RCSfile: ImporterImpl.h,v $
|   $Revision: 1.8 $
|       $Date: 2011/08/26 18:46:50 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "InputExtractor.h"
#include <ScalableTerrainModel/Import/Plugin/ExtractorRegistry.h>
#include "InternalImporterConfig.h"
#include "InternalContentDescriptor.h"

#include <ScalableTerrainModel/Import/Source.h>
#include "Sink.h"
#include <ScalableTerrainModel/Import/FilterFactory.h>

#include <ScalableTerrainModel/Import/ImportPolicy.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

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

    const MemoryAllocator&      GetAllocator           () const { return *m_allocatorP; }


    void                        Import                 (const ImportSequence&               sequence,
                                                        const Internal::Config&             config);

    void                        Import                 (const ImportCommand&                command,
                                                        const Internal::Config&             config);


    void                        Import                 (UInt                                sourceLayerID,
                                                        const DataTypeFamily&               sourceTypeFamily,
                                                        UInt                                targetLayerID,
                                                        const DataTypeFamily&               targetTypeFamily,
                                                        const Internal::Config&             config);

    void                        Import                 (UInt                                sourceLayerID,
                                                        const DataType&                     sourceType,
                                                        UInt                                targetLayerID,
                                                        const DataTypeFamily&               targetTypeFamily,
                                                        const Internal::Config&             config);

    void                        Import                 (UInt                                sourceLayerID,   
                                                        const DataType&                     sourceType,
                                                        UInt                                targetLayerID,   
                                                        const DataType&                     targetType,
                                                        const Internal::Config&             config);


    FilterCreatorCPtr           GetFilterCreatorFor    (UInt                                sourceLayerID,
                                                        const DataType&                     sourceType,
                                                        UInt                                targetLayerID,
                                                        const DataType&                     targetType,
                                                        const Internal::Config&             config);


    const InputExtractorCreator*
                                GetPluginCreatorFor    (const DataType&                     sourceType);

    static const Internal::Config&        
                                DefaultImportConfig    ();
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE
