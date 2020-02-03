/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct MetadataRecord;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct FilteringConfig
    {
private:
    struct Impl;
    typedef SharedPtrTypeTrait<Impl>::type
                                        ImplPtr;
    ImplPtr                             m_implP;

public:
    IMPORT_DLLE explicit                FilteringConfig                    ();
    IMPORT_DLLE                         ~FilteringConfig                   ();

    IMPORT_DLLE                         FilteringConfig                    (const FilteringConfig&              rhs);
    IMPORT_DLLE FilteringConfig&        operator=                          (const FilteringConfig&              rhs);

    IMPORT_DLLE const MetadataRecord&   GetMetadataRecord                  () const;
    IMPORT_DLLE MetadataRecord&         EditMetadataRecord                 ();
    };

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
