/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Config/Extraction.h $
|    $RCSfile: Extraction.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/11/22 21:57:54 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Import/Config/Base.h>

#include <ScalableTerrainModel/Import/ExtractionConfig.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportExtractionConfig : public ImportConfigComponentMixinBase<ImportExtractionConfig>
    {
private:
    ExtractionConfig                        m_config;

public:
    IMPORT_DLLE static ClassID              s_GetClassID                   ();

    IMPORT_DLLE explicit                    ImportExtractionConfig         (const ExtractionConfig&                 config);
    IMPORT_DLLE virtual                     ~ImportExtractionConfig        ();

    IMPORT_DLLE                             ImportExtractionConfig         (const ImportExtractionConfig&           rhs);

    const ExtractionConfig&                 Get                            () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE