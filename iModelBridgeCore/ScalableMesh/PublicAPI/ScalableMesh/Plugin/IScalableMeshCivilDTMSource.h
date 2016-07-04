/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Plugin/IScalableMeshCivilDTMSource.h $
|    $RCSfile: IScalableMeshCivilDTMSource.h,v $
|   $Revision: 1.6 $
|       $Date: 2011/11/09 18:10:59 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Plugin/SourceV0.h>

#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/bcDTMClass.h>
#include <TerrainModel/TerrainModel.h>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
namespace Plugin { namespace V0 {

BENTLEY_SM_EXPORT Import::Plugin::V0::SourceBase*         CreateCivilDTMSource               (BcDTMR dtm, 
                                                                                              Import::Log& log);

BENTLEY_SM_EXPORT Import::Plugin::V0::SourceBase*         CreateCivilDTMSource               (BcDTMR dtm,
                                                                                              const GeoCoords::GCS&           gcs,
                                                                                              Import::Log&                    log);


}} // END namespace Plugin::V0
END_BENTLEY_SCALABLEMESH_NAMESPACE
