/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/FilteringConfig.h $
|    $RCSfile: FilteringConfig.h,v $
|   $Revision: 1.2 $
|       $Date: 2011/09/01 14:07:18 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

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

END_BENTLEY_MRDTM_IMPORT_NAMESPACE