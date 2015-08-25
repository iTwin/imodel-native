/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/ExtractionConfig.h $
|    $RCSfile: ExtractionConfig.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/08/05 14:06:16 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct MetadataRecord;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExtractionConfig
    {
private:
    struct Impl;
    typedef SharedPtrTypeTrait<Impl>::type
                                        ImplPtr;
    ImplPtr                             m_implP;

public:
    IMPORT_DLLE explicit                ExtractionConfig                   ();
    IMPORT_DLLE                         ~ExtractionConfig                  ();

    IMPORT_DLLE                         ExtractionConfig                   (const ExtractionConfig&                 rhs);
    IMPORT_DLLE ExtractionConfig&       operator=                          (const ExtractionConfig&                 rhs);

    IMPORT_DLLE const MetadataRecord&   GetMetadataRecord                  () const;
    IMPORT_DLLE MetadataRecord&         EditMetadataRecord                 ();
    };


struct Source;
struct DataType;

/*---------------------------------------------------------------------------------**//**
* @description  TDORAY: Move elsewhere (in a query header)
* @bsiclass                                                  Raymond.Gauthier   7/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExtractionQuery : private Uncopyable
    {
private:
    UInt                                        m_layer;
    const DataType&                             m_type;
public:
    explicit                                    ExtractionQuery                            (UInt                    layer,
                                                                                            const DataType&         type)
        :   m_type(type), m_layer(layer) { }

    UInt                                        GetLayer                                   () const { return m_layer; }
    const DataType&                             GetType                                    () const { return m_type; }
    };



END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
