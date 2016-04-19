/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Plugin/IScalableMeshSTMSource.h $
|    $RCSfile: IScalableMeshSTMSource.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/18 15:51:39 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Plugin/SourceV0.h>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
namespace Plugin { namespace V0 {

BENTLEY_SM_EXPORT Import::Plugin::V0::SourceBase*         CreateSTMSource                (const WChar*              path,
                                                                                    Import::Log&                log);





}} // END namespace Plugin::V0
END_BENTLEY_SCALABLEMESH_NAMESPACE
