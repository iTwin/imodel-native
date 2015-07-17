/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/ExtractionConfig.cpp $
|    $RCSfile: ExtractionConfig.cpp,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/05 14:06:18 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>
#include <ScalableTerrainModel/Import/ExtractionConfig.h>
#include <ScalableTerrainModel/Import/Metadata.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExtractionConfig::Impl : public ShareableObjectTypeTrait<ExtractionConfig::Impl>::type
    {
    // NTERAY: Optimize by making this a pointer an lazy initialize it as it is optionnal.
    MetadataRecord              m_metadataRecord;

    explicit                    Impl                       ()
        {
        }

    static void                 UpdateForEdit              (ImplPtr&    pi_rpInstance)
        {
        // Copy on write when config is shared
        if (pi_rpInstance->IsShared())
            pi_rpInstance = new Impl(*pi_rpInstance);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExtractionConfig::ExtractionConfig ()
    :   m_implP(new Impl)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExtractionConfig::~ExtractionConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExtractionConfig::ExtractionConfig (const ExtractionConfig& rhs)
    :   m_implP(rhs.m_implP)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExtractionConfig& ExtractionConfig::operator= (const ExtractionConfig& rhs)
    {
    m_implP = rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const MetadataRecord& ExtractionConfig::GetMetadataRecord () const
    {
    return m_implP->m_metadataRecord;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataRecord& ExtractionConfig::EditMetadataRecord ()
    {
    Impl::UpdateForEdit(m_implP);
    return m_implP->m_metadataRecord;
    }


END_BENTLEY_MRDTM_IMPORT_NAMESPACE