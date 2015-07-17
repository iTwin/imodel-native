/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Config/TargetFilters.h $
|    $RCSfile: TargetFilters.h,v $
|   $Revision: 1.7 $
|       $Date: 2011/11/22 21:57:59 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Import/Config/Base.h>

#include <ScalableTerrainModel/Import/CustomFilterFactory.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TargetFiltersConfig : public ImportConfigComponentMixinBase<TargetFiltersConfig>
    {
private:
    struct                                  Impl;
    std::auto_ptr<Impl>                     m_implP;

public:
    IMPORT_DLLE static ClassID              s_GetClassID                   ();

    IMPORT_DLLE explicit                    TargetFiltersConfig            ();
    IMPORT_DLLE explicit                    TargetFiltersConfig            (const CustomFilterFactory&              filterFactory);

    IMPORT_DLLE virtual                     ~TargetFiltersConfig           ();

    IMPORT_DLLE                             TargetFiltersConfig            (const TargetFiltersConfig&              rhs);

    IMPORT_DLLE void                        push_back                      (const CustomFilterFactory&              filterFactory);

    const CustomFilteringSequence&          GetSequence                    () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE