/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Importer.h $
|    $RCSfile: Importer.h,v $
|   $Revision: 1.16 $
|       $Date: 2011/09/07 14:20:51 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Import/ContentConfig.h>
#include <ScalableTerrainModel/Import/ImportSequence.h>
#include <ScalableTerrainModel/Import/ImportConfig.h>

BEGIN_BENTLEY_MRDTM_MEMORY_NAMESPACE
struct MemoryAllocator;
END_BENTLEY_MRDTM_MEMORY_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE
struct ExtractorRegistry;
struct TypeConversionFilterRegistry;
struct ReprojectionFilterRegistry;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Forward declarations
+---------------+---------------+---------------+---------------+---------------+------*/
struct Importer;
struct ImporterImpl;
struct ImportPolicy;
struct Source;
struct DataTypeFamily;
struct DataType;
struct Sink;
struct FilterFactory;

typedef SharedPtrTypeTrait<Importer>::type  ImporterPtr;
typedef SharedPtrTypeTrait<const Source>::type  
                                            SourceCPtr;
typedef SharedPtrTypeTrait<Sink>::type      SinkPtr;

/*---------------------------------------------------------------------------------**//**
* @description  // TDORAY: Was dated.. Rewrite
*
*               Guarantee:
*               - No exception is thrown by any method of this interface
*
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImporterFactory : private Unassignable
    {
private:
    struct                                  Impl;
    SharedPtrTypeTrait<const Impl>::type    m_pImpl;

public:
    typedef Plugin::ExtractorRegistry       ExtractorRegistry;


    IMPORT_DLLE explicit                    ImporterFactory            (Log&                            log = GetDefaultLog());

    IMPORT_DLLE explicit                    ImporterFactory            (const ImportPolicy&             policy,
                                                                        Log&                            log = GetDefaultLog());

    IMPORT_DLLE explicit                    ImporterFactory            (const ImportPolicy&             policy,
                                                                        const ExtractorRegistry&        extractorRegistry,
                                                                        const FilterFactory&            filterFactory,
                                                                        Log&                            log = GetDefaultLog());


    IMPORT_DLLE                             ImporterFactory            (const ImporterFactory&          rhs);
    IMPORT_DLLE                             ~ImporterFactory           ();


    IMPORT_DLLE ImporterPtr                 Create                     (const SourceCPtr&               sourcePtr,
                                                                        const SinkPtr&                  sinkPtr) const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* 
*               Guarantee:
*               - No exception is thrown by any method of this interface
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Importer : private Uncopyable, public ShareableObjectTypeTrait<Importer>::type
    {
    enum Status
        {
        S_SUCCESS,
        S_ERROR,
        S_QTY,
        };

private:
    friend struct                           ImporterFactory;
    
    std::auto_ptr<ImporterImpl>             m_pImpl;

    explicit                                Importer                   (ImporterImpl*                   implP);

    ImporterImpl&                           GetImpl                    ();
    const ImporterImpl&                     GetImpl                    () const;

public:
    IMPORT_DLLE                             ~Importer                  ();


    IMPORT_DLLE Status                      Import                     (const ImportSequence&           sequence,
                                                                        const ImportConfig&             config);

    IMPORT_DLLE Status                      Import                     (const ImportSequence&           sequence);

    IMPORT_DLLE Status                      Import                     (const ImportCommand&            command,
                                                                        const ImportConfig&             config);

    IMPORT_DLLE Status                      Import                     (const ImportCommand&            command);
    };




END_BENTLEY_MRDTM_IMPORT_NAMESPACE

