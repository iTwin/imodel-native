//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/SourcesDir.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/SourcesDir.h>
#include <STMInternal/Storage/HTGFFAttributeManager.h>

#include "../IDTMFileDefinition.h"

namespace IDTMFile {

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesDir::~SourcesDir ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesDir::SourcesDir ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesDir::TimeType SourcesDir::GetLastModifiedCheckTime () const
    {
    TimeType time = 0;
    AttributeMgr().Get(IDTM_ATTRIBUTEID_SOURCESDIR_LAST_MODIFIED_CHECK_TIME, time);

    return time;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourcesDir::SetLastModifiedCheckTime (TimeType    pi_checkTime)
    {
    return AttributeMgr().Set(IDTM_ATTRIBUTEID_SOURCESDIR_LAST_MODIFIED_CHECK_TIME, pi_checkTime);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesDir::TimeType SourcesDir::GetLastSyncTime () const
    {
    TimeType time = 0;
    AttributeMgr().Get(IDTM_ATTRIBUTEID_SOURCESDIR_LAST_SYNC_TIME, time);
    return time;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourcesDir::SetLastSyncTime (TimeType pi_lastSyncTime)
    {
    return AttributeMgr().Set(IDTM_ATTRIBUTEID_SOURCESDIR_LAST_SYNC_TIME, pi_lastSyncTime);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourcesDir::ClearAll ()
    {
    bool Success = true;

    Success &= SourceNodeDir::ClearAll();

    AttributeMgr().Remove(IDTM_ATTRIBUTEID_SOURCESDIR_LAST_SYNC_TIME);
    AttributeMgr().Remove(IDTM_ATTRIBUTEID_SOURCESDIR_LAST_MODIFIED_CHECK_TIME);

    AttributeMgr().Remove(IDTM_ATTRIBUTEID_SOURCESDIR_SERIALIZED_SOURCE_FORMAT_VERSION);
    AttributeMgr().Remove(IDTM_ATTRIBUTEID_SOURCESDIR_CONTENT_CONFIG_FORMAT_VERSION);
    AttributeMgr().Remove(IDTM_ATTRIBUTEID_SOURCESDIR_IMPORT_SEQUENCE_FORMAT_VERSION);
    AttributeMgr().Remove(IDTM_ATTRIBUTEID_SOURCESDIR_IMPORT_CONFIG_FORMAT_VERSION);

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourcesDir::SetSerializedSourceFormatVersion (uint32_t pi_version)
    {
    return AttributeMgr().Set(IDTM_ATTRIBUTEID_SOURCESDIR_SERIALIZED_SOURCE_FORMAT_VERSION, pi_version);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourcesDir::SetContentConfigFormatVersion (uint32_t pi_version)
    {
    return AttributeMgr().Set(IDTM_ATTRIBUTEID_SOURCESDIR_CONTENT_CONFIG_FORMAT_VERSION, pi_version);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourcesDir::SetImportSequenceFormatVersion (uint32_t pi_version)
    {
    return AttributeMgr().Set(IDTM_ATTRIBUTEID_SOURCESDIR_IMPORT_SEQUENCE_FORMAT_VERSION, pi_version);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourcesDir::SetImportConfigFormatVersion (uint32_t pi_version)
    {
    return AttributeMgr().Set(IDTM_ATTRIBUTEID_SOURCESDIR_IMPORT_CONFIG_FORMAT_VERSION, pi_version);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t SourcesDir::GetSerializedSourceFormatVersion () const
    {
    uint32_t version(0); // Default to 0 if not found.
    AttributeMgr().Get(IDTM_ATTRIBUTEID_SOURCESDIR_SERIALIZED_SOURCE_FORMAT_VERSION, version);
    return version;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t SourcesDir::GetContentConfigFormatVersion () const
    {
    uint32_t version(0); // Default to 0 if not found.
    AttributeMgr().Get(IDTM_ATTRIBUTEID_SOURCESDIR_CONTENT_CONFIG_FORMAT_VERSION, version);
    return version;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t SourcesDir::GetImportSequenceFormatVersion () const
    {
    uint32_t version(0); // Default to 0 if not found.
    AttributeMgr().Get(IDTM_ATTRIBUTEID_SOURCESDIR_IMPORT_SEQUENCE_FORMAT_VERSION, version);
    return version;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t SourcesDir::GetImportConfigFormatVersion () const
    {
    uint32_t version(0); // Default to 0 if not found.
    AttributeMgr().Get(IDTM_ATTRIBUTEID_SOURCESDIR_IMPORT_CONFIG_FORMAT_VERSION, version);
    return version;
    }


} //End namespace IDTMFile