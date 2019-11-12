/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Plugin/SourceV0.h>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
namespace Plugin { namespace V0 {

    BENTLEY_SM_IMPORT_EXPORT Import::Plugin::V0::SourceBase*         CreateSTMSource                (const WChar*              path,
                                                                                    Import::Log&                log);





}} // END namespace Plugin::V0
END_BENTLEY_SCALABLEMESH_NAMESPACE
