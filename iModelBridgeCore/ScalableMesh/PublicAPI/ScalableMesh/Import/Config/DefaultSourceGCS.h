/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Config/DefaultSourceGCS.h $
|    $RCSfile: DefaultSourceGCS.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/11/22 21:57:48 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Import/Config/Base.h>
#include <ScalableMesh/GeoCoords/GCS.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

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


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
