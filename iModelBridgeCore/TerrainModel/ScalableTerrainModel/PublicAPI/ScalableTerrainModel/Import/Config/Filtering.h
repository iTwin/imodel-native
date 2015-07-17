/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Config/Filtering.h $
|    $RCSfile: Filtering.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/11/22 21:57:56 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Import/Config/Base.h>

#include <ScalableTerrainModel/Import/FilteringConfig.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportFilteringConfig : public ImportConfigComponentMixinBase<ImportFilteringConfig>
    {
private:
    FilteringConfig                         m_config;

public:
    IMPORT_DLLE static ClassID              s_GetClassID                   ();

    IMPORT_DLLE explicit                    ImportFilteringConfig          (const FilteringConfig&                  config);
    IMPORT_DLLE virtual                     ~ImportFilteringConfig         ();

    IMPORT_DLLE                             ImportFilteringConfig          (const ImportFilteringConfig&            rhs);

    const FilteringConfig&                  Get                            () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE