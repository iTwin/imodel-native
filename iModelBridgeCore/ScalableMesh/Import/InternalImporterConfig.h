/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/InternalImporterConfig.h $
|    $RCSfile: InternalImporterConfig.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/09/01 14:06:52 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Definitions.h>


#include <ScalableMesh/Import/DataType.h>
#include <ScalableMesh/Import/ScalableMeshData.h>
#include <ScalableMesh/Import/ImportConfig.h>
#include <ScalableMesh/GeoCoords/GCS.h>

#include <ScalableMesh/Import/CustomFilterFactory.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct ContentDescriptor;
struct LayerDescriptor;
struct ExtractionConfig;
struct FilteringConfig;

namespace Internal {



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
    class Config : public ImportConfig
    {
    CustomFilteringSequence         m_sourceFilters;
    CustomFilteringSequence         m_targetFilters;

    bool                            m_importAttachments;

    bool                            m_hasDefaultTargetLayer;
    uint32_t                            m_defaultTargetLayer;

    const DataTypeFamily*           m_defaultTargetTypeP;

    const GCS*                      m_defaultSourceGCSP;
    const GCS*                      m_defaultTargetGCSP;

    const ScalableMeshData*        m_defaultTargetSMDataP;

    const ExtractionConfig*         m_extractionConfigP;
    const FilteringConfig*          m_filteringConfigP;


    virtual void _SetAreAttachmentsImported(bool importAttachments) override
        {
        m_importAttachments = importAttachments;
        }
    virtual void _SetDefaultTargetLayer(uint32_t layer) override
        {
        m_defaultTargetLayer = layer;
        m_hasDefaultTargetLayer = true;
        }
    virtual void _SetDefaultTargetType(const DataTypeFamily* typeP)
        {
        m_defaultTargetTypeP = typeP;
        }

    virtual void _SetDefaultSourceGCS(const GCS* gcsP)
        {
        m_defaultSourceGCSP = gcsP;
        }
    virtual void _SetDefaultTargetGCS(const GCS* gcsP)
        {
        m_defaultTargetGCSP = gcsP;
        }

    virtual void _SetDefaultTargetSMData(const ScalableMeshData* smDataP)
        {
        m_defaultTargetSMDataP = smDataP;
        }

    virtual void _SetExtractionConfig(const ExtractionConfig* extractionConfigP)
        {
        m_extractionConfigP = extractionConfigP;
        }
    virtual void _SetFilteringConfig(const FilteringConfig* filteringConfigP)
        {
        m_filteringConfigP = filteringConfigP;
        }

    virtual void _SetSourceFilters(const CustomFilteringSequence& sourceFilters)
        {
        m_sourceFilters = sourceFilters;
        }
    virtual void _SetTargetFilters(const CustomFilteringSequence& targetFilters) 
        {
        m_targetFilters = targetFilters;
        }


public:
    explicit                        Config                     ();
    

    bool                            HasDefaultTargetLayer      () const { return m_hasDefaultTargetLayer; }
    uint32_t                            GetDefaultTargetLayer      () const { return m_defaultTargetLayer; }

    bool                            HasDefaultTargetType       () const { return 0 != m_defaultTargetTypeP; }
    const DataTypeFamily&           GetDefaultTargetType       () const { return *m_defaultTargetTypeP; }

    bool                            HasDefaultTargetSMData () const { return 0 != m_defaultTargetSMDataP; }
    const ScalableMeshData&        GetDefaultTargetSMData       () const { return *m_defaultTargetSMDataP; }

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
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
