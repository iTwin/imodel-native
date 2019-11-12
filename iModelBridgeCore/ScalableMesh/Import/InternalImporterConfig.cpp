/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../STM/ImagePPHeaders.h"
#include "InternalImporterConfig.h"
#include <ScalableMesh/Import/ExtractionConfig.h>
#include <ScalableMesh/Import/FilteringConfig.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
namespace Internal {

namespace {
const ExtractionConfig  DEFAULT_EXTRACTION_CONFIG;
const FilteringConfig  DEFAULT_FILTERING_CONFIG;
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Config::Config ()
    :   m_importAttachments(false),
        m_hasDefaultTargetLayer(false),
        m_defaultTargetLayer(0),
        m_defaultTargetTypeP(0),
        m_defaultSourceGCSP(0),
        m_defaultTargetGCSP(0),
    m_defaultTargetSMDataP(0),
        m_extractionConfigP(&DEFAULT_EXTRACTION_CONFIG),
        m_filteringConfigP(&DEFAULT_FILTERING_CONFIG)
    {
    }

} // END namespace Internal
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
