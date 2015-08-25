/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/InternalImporterConfig.cpp $
|    $RCSfile: InternalImporterConfig.cpp,v $
|   $Revision: 1.3 $
|       $Date: 2011/09/01 14:06:51 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include "InternalImporterConfig.h"
#include <ScalableMesh/Import/Config/All.h>

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

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Config::_Visit (const AttachmentsConfig& config)
    {
    m_importAttachments = config.AreImported();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Config::_Visit (const DefaultSourceGCSConfig& config)
    {
    m_defaultSourceGCSP = &config.Get();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Config::_Visit (const DefaultTargetGCSConfig& config)
    {
    m_defaultTargetGCSP = &config.Get();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Config::_Visit (const DefaultTargetLayerConfig& config)
    {
    m_defaultTargetLayer = config.Get();
    m_hasDefaultTargetLayer = true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Config::_Visit (const DefaultTargetTypeConfig&  config)
    {
    m_defaultTargetTypeP = &config.Get();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Config::_Visit(const DefaultTargetScalableMeshConfig&  config)
{
    m_defaultTargetSMDataP = const_cast<ScalableMeshData*>(&config.Get());
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Config::_Visit (const ImportExtractionConfig&  config)
    {
    m_extractionConfigP = &config.Get();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Config::_Visit (const ImportFilteringConfig& config)
    {
    m_filteringConfigP = &config.Get();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Config::_Visit (const SourceFiltersConfig& config)
    {
    m_sourceFilters = config.GetSequence();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Config::_Visit (const TargetFiltersConfig& config) 
    {
    m_targetFilters = config.GetSequence();
    }

} // END namespace Internal
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
