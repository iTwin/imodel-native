/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/ImportConfigs.cpp $
|    $RCSfile: ImportConfigs.cpp,v $
|   $Revision: 1.10 $
|       $Date: 2011/10/21 17:32:21 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>
#include <ScalableTerrainModel/Import/Config/All.h>
#include "ImportConfigComponentMixinBaseImpl.h"

#include <ScalableTerrainModel/Import/CustomFilterFactory.h>


BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

AttachmentsConfig::ClassID AttachmentsConfig::s_GetClassID () { return super_class::s_GetClassID(); }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentsConfig::AttachmentsConfig (const AttachmentsConfig& rhs)
    :   super_class(rhs),
        m_imported(rhs.m_imported),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentsConfig::AttachmentsConfig (bool imported)
    :   m_imported(imported),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentsConfig::~AttachmentsConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool AttachmentsConfig::AreImported () const
    {
    return m_imported;
    }


DefaultSourceGCSConfig::ClassID DefaultSourceGCSConfig::s_GetClassID () { return super_class::s_GetClassID(); }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultSourceGCSConfig::DefaultSourceGCSConfig (const DefaultSourceGCSConfig& rhs)
    :   super_class(rhs),
        m_gcs(rhs.m_gcs),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultSourceGCSConfig::DefaultSourceGCSConfig (const GCS& gcs)
    :   m_gcs(gcs),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultSourceGCSConfig::~DefaultSourceGCSConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const GCS& DefaultSourceGCSConfig::Get () const
    {
    return m_gcs;
    }


DefaultTargetGCSConfig::ClassID DefaultTargetGCSConfig::s_GetClassID () { return super_class::s_GetClassID(); }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultTargetGCSConfig::DefaultTargetGCSConfig (const DefaultTargetGCSConfig& rhs)
    :   super_class(rhs),
        m_gcs(rhs.m_gcs),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultTargetGCSConfig::DefaultTargetGCSConfig (const GCS& gcs)
    :   m_gcs(gcs),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultTargetGCSConfig::~DefaultTargetGCSConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const GCS& DefaultTargetGCSConfig::Get () const
    {
    return m_gcs;
    }


DefaultTargetLayerConfig::ClassID DefaultTargetLayerConfig::s_GetClassID () { return super_class::s_GetClassID(); }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultTargetLayerConfig::DefaultTargetLayerConfig (const DefaultTargetLayerConfig& rhs)
    :   super_class(rhs),
        m_layer(rhs.m_layer),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultTargetLayerConfig::DefaultTargetLayerConfig (UInt layer)
    :   m_layer(layer),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultTargetLayerConfig::~DefaultTargetLayerConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
UInt DefaultTargetLayerConfig::Get () const
    {
    return m_layer;
    }

DefaultTargetTypeConfig::ClassID DefaultTargetTypeConfig::s_GetClassID () { return super_class::s_GetClassID(); }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultTargetTypeConfig::DefaultTargetTypeConfig (const DefaultTargetTypeConfig& rhs)
    :   super_class(rhs),
        m_type(rhs.m_type),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultTargetTypeConfig::DefaultTargetTypeConfig (const DataTypeFamily& type)
    :   m_type(type),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultTargetTypeConfig::~DefaultTargetTypeConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& DefaultTargetTypeConfig::Get () const
    {
    return m_type;
    }

ImportExtractionConfig::ClassID ImportExtractionConfig::s_GetClassID () { return super_class::s_GetClassID(); }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportExtractionConfig::ImportExtractionConfig (const ImportExtractionConfig& rhs)
    :   m_config(rhs.m_config)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportExtractionConfig::ImportExtractionConfig (const ExtractionConfig& config)
    :   m_config(config)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportExtractionConfig::~ImportExtractionConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const ExtractionConfig& ImportExtractionConfig::Get () const
    {
    return m_config;
    }




ImportFilteringConfig::ClassID ImportFilteringConfig::s_GetClassID () { return super_class::s_GetClassID(); }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportFilteringConfig::ImportFilteringConfig (const ImportFilteringConfig& rhs)
    :   m_config(rhs.m_config)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportFilteringConfig::ImportFilteringConfig (const FilteringConfig& config)
    :   m_config(config)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportFilteringConfig::~ImportFilteringConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const FilteringConfig& ImportFilteringConfig::Get () const
    {
    return m_config;
    }



SourceFiltersConfig::ClassID SourceFiltersConfig::s_GetClassID () { return super_class::s_GetClassID(); }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceFiltersConfig::Impl
    {
    CustomFilteringSequence        m_sequence;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceFiltersConfig::SourceFiltersConfig (const SourceFiltersConfig& rhs)
    :   super_class(rhs),
        m_implP(new Impl(*rhs.m_implP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceFiltersConfig::SourceFiltersConfig ()
    :   m_implP(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceFiltersConfig::SourceFiltersConfig (const CustomFilterFactory& filter) 
    :   m_implP(new Impl)
    {
    m_implP->m_sequence.push_back(filter);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceFiltersConfig::~SourceFiltersConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceFiltersConfig::push_back (const CustomFilterFactory& filter)
    {
    m_implP->m_sequence.push_back(filter);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const CustomFilteringSequence& SourceFiltersConfig::GetSequence () const
    {
    return m_implP->m_sequence;
    }


TargetFiltersConfig::ClassID TargetFiltersConfig::s_GetClassID () { return super_class::s_GetClassID(); }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TargetFiltersConfig::Impl
    {
    CustomFilteringSequence        m_sequence;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TargetFiltersConfig::TargetFiltersConfig (const TargetFiltersConfig& rhs)
    :   super_class(rhs),
        m_implP(new Impl(*rhs.m_implP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TargetFiltersConfig::TargetFiltersConfig ()
    :   m_implP(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TargetFiltersConfig::TargetFiltersConfig (const CustomFilterFactory& filter) 
    :   m_implP(new Impl)
    {
    m_implP->m_sequence.push_back(filter);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TargetFiltersConfig::~TargetFiltersConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void TargetFiltersConfig::push_back (const CustomFilterFactory& filter)
    {
    m_implP->m_sequence.push_back(filter);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const CustomFilteringSequence& TargetFiltersConfig::GetSequence () const
    {
    return m_implP->m_sequence;
    }






END_BENTLEY_MRDTM_IMPORT_NAMESPACE