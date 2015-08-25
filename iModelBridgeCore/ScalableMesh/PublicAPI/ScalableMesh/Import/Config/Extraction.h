/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Config/Extraction.h $
|    $RCSfile: Extraction.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/11/22 21:57:54 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Import/Config/Base.h>

#include <ScalableMesh/Import/ExtractionConfig.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

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


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
