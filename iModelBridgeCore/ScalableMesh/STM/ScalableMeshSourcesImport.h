/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourcesImport.h $
|    $RCSfile: ScalableMeshSourcesImport.h,v $
|   $Revision: 1.12 $
|       $Date: 2011/08/26 18:47:44 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel/TerrainModel.h>

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Foundations/Warning.h>
#include <ScalableMesh/Memory/Allocation.h>
#include <ScalableMesh/IScalableMeshSources.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct Sink;
struct ImportSequence;
struct ImportConfig;
struct ContentConfig;
struct SourceRef;
struct DataTypeFamily;
struct LocalFileSourceRef;
struct ImporterFactory;
struct SourceFactory;
struct IDTMSource;

typedef SharedPtrTypeTrait<Sink>::type   SinkPtr;
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourcesImporter : private Import::Uncopyable
    {
private:
    struct                              Impl;
    std::auto_ptr<Impl>                 m_implP;

public:
    enum Status
        {
        S_SUCCESS,
        S_ERROR,
        S_QTY,
        };


    explicit                            SourcesImporter                    (const Import::LocalFileSourceRef&       sinkSourceRef,
                                                                            const Import::SinkPtr&                  sinkPtr);

    
                                        ~SourcesImporter                   ();
    

    void                                AddSource                          (const Import::SourceRef&                sourceRef,
                                                                            const Import::ContentConfig&            contentConfig,
                                                                            const Import::ImportConfig*             config,
                                                                            const Import::ImportSequence&           sequence,
                                                                            SourceImportConfig&                     sourceImportConf);

    bool                                IsEmpty                            () const;


    Status                              Import                             () const;

    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
