/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Plugin/SourceV0.h>


BEGIN_BENTLEY_MRDTM_NAMESPACE
namespace Plugin { namespace V0 {

BENTLEYSTM_EXPORT Import::Plugin::V0::SourceBase*         CreateSTMSource                (const WChar*              path,
                                                                                    Import::Log&                log);





}} // END namespace Plugin::V0
END_BENTLEY_MRDTM_NAMESPACE