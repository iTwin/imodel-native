/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/InternalImporterConfig.h $
|    $RCSfile: InternalImporterConfig.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/09/01 14:06:52 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Import/ContentConfigVisitor.h>
#include <ScalableTerrainModel/Import/ImportConfigVisitor.h>

#include <ScalableTerrainModel/Import/DataType.h>
#include <ScalableTerrainModel/GeoCoords/GCS.h>

#include <ScalableTerrainModel/Import/CustomFilterFactory.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

struct ContentDescriptor;
struct LayerDescriptor;
struct ExtractionConfig;
struct FilteringConfig;

namespace Internal {



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class Config : public IImportConfigVisitor
    {
    CustomFilteringSequence         m_sourceFilters;
    CustomFilteringSequence         m_targetFilters;

    bool                            m_importAttachments;

    bool                            m_hasDefaultTargetLayer;
    UInt                            m_defaultTargetLayer;

    const DataTypeFamily*           m_defaultTargetTypeP;

    const GCS*                      m_defaultSourceGCSP;
    const GCS*                      m_defaultTargetGCSP;

    const ExtractionConfig*         m_extractionConfigP;
    const FilteringConfig*          m_filteringConfigP;

    virtual void                    _Visit                     (const AttachmentsConfig&        config) override;
    virtual void                    _Visit                     (const DefaultSourceGCSConfig&   config) override;
    virtual void                    _Visit                     (const DefaultTargetGCSConfig&   config) override;
    virtual void                    _Visit                     (const DefaultTargetLayerConfig& config) override;
    virtual void                    _Visit                     (const DefaultTargetTypeConfig&  config) override;
    virtual void                    _Visit                     (const ImportExtractionConfig&   config) override;
    virtual void                    _Visit                     (const ImportFilteringConfig&    config) override;
    virtual void                    _Visit                     (const SourceFiltersConfig&      config) override;
    virtual void                    _Visit                     (const TargetFiltersConfig&      config) override;

public:
    explicit                        Config                     ();
    

    bool                            HasDefaultTargetLayer      () const { return m_hasDefaultTargetLayer; }
    UInt                            GetDefaultTargetLayer      () const { return m_defaultTargetLayer; }

    bool                            HasDefaultTargetType       () const { return 0 != m_defaultTargetTypeP; }
    const DataTypeFamily&           GetDefaultTargetType       () const { return *m_defaultTargetTypeP; }

    bool                            HasDefaultSourceGCS        () const { return 0 != m_defaultSourceGCSP; }
    const GCS&                      GetDefaultSourceGCS        () const { return *m_defaultSourceGCSP; }

    bool                            HasDefaultTargetGCS        () const { return 0 != m_defaultTargetGCSP; }
    const GCS&                      GetDefaultTargetGCS        () const { return *m_defaultTargetGCSP; }

    const ExtractionConfig&         GetExtractionConfig        () const { return *m_extractionConfigP; }

    const FilteringConfig&          GetFilteringConfig         () const { return *m_filteringConfigP; }

    const CustomFilteringSequence&  GetSourceFilters           () const { return m_sourceFilters; }
    const CustomFilteringSequence&  GetTargetFilters           () const { return m_targetFilters; }
    };


} // END namespace Internal
END_BENTLEY_MRDTM_IMPORT_NAMESPACE