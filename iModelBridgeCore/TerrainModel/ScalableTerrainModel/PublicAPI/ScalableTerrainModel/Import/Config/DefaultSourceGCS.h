/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Config/DefaultSourceGCS.h $
|    $RCSfile: DefaultSourceGCS.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/11/22 21:57:48 $
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
struct DefaultSourceGCSConfig : public ImportConfigComponentMixinBase<DefaultSourceGCSConfig>
    {
private:
    GCS                                     m_gcs;
    void*                                   m_implP; // Reserved some space for further use

public:
    IMPORT_DLLE static ClassID              s_GetClassID                   ();

    IMPORT_DLLE explicit                    DefaultSourceGCSConfig         (const GCS&                              gcs);
    IMPORT_DLLE virtual                     ~DefaultSourceGCSConfig        ();

    IMPORT_DLLE                             DefaultSourceGCSConfig         (const DefaultSourceGCSConfig&           rhs);

    const GCS&                              Get                            () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE