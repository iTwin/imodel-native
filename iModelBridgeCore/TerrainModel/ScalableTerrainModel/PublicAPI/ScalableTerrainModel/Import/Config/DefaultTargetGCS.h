/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Config/DefaultTargetGCS.h $
|    $RCSfile: DefaultTargetGCS.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/11/22 21:57:49 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Import/Config/Base.h>
#include <ScalableTerrainModel/GeoCoords/GCS.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DefaultTargetGCSConfig : public ImportConfigComponentMixinBase<DefaultTargetGCSConfig>
    {
private:
    GCS                                     m_gcs;
    void*                                   m_implP; // Reserved some space for further use

public:
    IMPORT_DLLE static ClassID              s_GetClassID                   ();

    IMPORT_DLLE explicit                    DefaultTargetGCSConfig         (const GCS&                              gcs);
    IMPORT_DLLE virtual                     ~DefaultTargetGCSConfig        ();

    IMPORT_DLLE                             DefaultTargetGCSConfig         (const DefaultTargetGCSConfig&           rhs);

    const GCS&                              Get                            () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE