/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>
#include <ScalableTerrainModel/Import/FilteringConfig.h>
#include <ScalableTerrainModel/Import/Metadata.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct FilteringConfig::Impl : public ShareableObjectTypeTrait<Impl>::type
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
FilteringConfig::FilteringConfig ()
    :   m_implP(new Impl)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilteringConfig::~FilteringConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilteringConfig::FilteringConfig (const FilteringConfig& rhs)
    :   m_implP(rhs.m_implP)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilteringConfig& FilteringConfig::operator= (const FilteringConfig& rhs)
    {
    m_implP = rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const MetadataRecord& FilteringConfig::GetMetadataRecord () const
    {
    return m_implP->m_metadataRecord;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataRecord& FilteringConfig::EditMetadataRecord ()
    {
    Impl::UpdateForEdit(m_implP);
    return m_implP->m_metadataRecord;
    }


END_BENTLEY_MRDTM_IMPORT_NAMESPACE