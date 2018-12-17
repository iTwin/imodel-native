/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Plugin/IMrDTMSTMSource.h $
|    $RCSfile: IMrDTMSTMSource.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/18 15:51:39 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Plugin/SourceV0.h>


BEGIN_BENTLEY_MRDTM_NAMESPACE
namespace Plugin { namespace V0 {

BENTLEYSTM_EXPORT Import::Plugin::V0::SourceBase*         CreateSTMSource                (const WChar*              path,
                                                                                    Import::Log&                log);





}} // END namespace Plugin::V0
END_BENTLEY_MRDTM_NAMESPACE