//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/SourceNodeDir.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ScalableTerrainModelPCH.h>



#include <STMInternal/Storage/IDTMFileDirectories/SourceNodeDir.h>
#include "IDTMCommonDirTools.h"

#include "../IDTMFileDefinition.h"

#include <STMInternal/Storage/HTGFFSubDirManager.h>
#include <STMInternal/Storage/HTGFFAttributeManager.h>

using namespace HTGFF;

namespace {

// Check that time_t has the expected size
HSTATICASSERT(sizeof(time_t) == sizeof(int64_t));


/* 
 * VERSIONNING 
 * Version 0: 
 *      Was used in development stage. Will remain unsupported as it could not have been produced by clients (only internally).
 * Version 1:
 *      Current.
 */

const uint32_t DIRECTORY_VERSION = 1;


}


namespace IDTMFile {


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t SourceNodeDir::s_GetVersion ()
    {
    return DIRECTORY_VERSION;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceNodeDir* SourceNodeDir::Create ()
    {
    return new SourceNodeDir;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceNodeDir::SourceNodeDir ()
    :   Directory(DIRECTORY_VERSION),
        m_pSources(0)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceNodeDir::~SourceNodeDir ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceNodeDir::_Create (const CreateConfig&    pi_rCreateConfig,
                             const UserOptions*     pi_pUserOptions)
    {
    const CreateConfig DirCreateConfig(DataType::CreateVoid(), Compression::None::Create());

    const Options& options = pi_pUserOptions->SafeReinterpretAs<Options>();

    bool Success = true;
    if (options.m_isSource)
        {
        m_pSources = SubDirMgr<SourceSequenceDir>().Create(IDTM_DIRECTORYID_SOURCENODEDIR_SOURCES_SUBDIR, DirCreateConfig);
        Success &= (0 != m_pSources);
        }

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceNodeDir::_Load (const UserOptions*      pi_pUserOptions)
    {
    if (GetVersion() < DIRECTORY_VERSION)
        return false;

    bool Success = true;

    if (0 == m_pSources && AttributeMgr().IsPresent(IDTM_DIRECTORYID_SOURCENODEDIR_SOURCES_SUBDIR))
        {
        m_pSources = SubDirMgr<SourceSequenceDir>().Get(IDTM_DIRECTORYID_SOURCENODEDIR_SOURCES_SUBDIR);
        Success &= (0 != m_pSources);
        }

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceNodeDir::_Save ()
    {
    bool Success = true;

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceNodeDir::IsSourcesNode () const
    {
    return 0 != m_pSources;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceNodeDir::ClearAll ()
    {
    bool Success = true;

    Success &= SubDirMgr().RemoveSubDirs(IDTM_DIRECTORYID_SOURCENODEDIR_NODE_SUBDIRS);
    AttributeMgr().Remove(IDTM_ATTRIBUTEID_SOURCENODEDIR_LAST_MODIFIED_TIME);

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceNodeDir::TimeType SourceNodeDir::GetLastModifiedTime () const
    {
    TimeType time = 0;
    AttributeMgr().Get(IDTM_ATTRIBUTEID_SOURCENODEDIR_LAST_MODIFIED_TIME, time);

    return time;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceNodeDir::SetLastModifiedTime (TimeType pi_lastModifiedTime)
    {
    return AttributeMgr().Set(IDTM_ATTRIBUTEID_SOURCENODEDIR_LAST_MODIFIED_TIME, pi_lastModifiedTime);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const SourceSequenceDir* SourceNodeDir::GetSources () const
    {
    return m_pSources;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceSequenceDir* SourceNodeDir::GetSources ()
    {
    return m_pSources;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceNodeDir::NodeCIter SourceNodeDir::SubNodesBegin () const
    {
    return SubDirIterMgr<NodeCIter::value_type>().begin(IDTM_DIRECTORYID_SOURCENODEDIR_NODE_SUBDIRS, 
                                                        SubDirMgr<SourceNodeDir>());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceNodeDir::NodeCIter SourceNodeDir::SubNodesEnd () const
    {
    return SubDirIterMgr<NodeCIter::value_type>().end(IDTM_DIRECTORYID_SOURCENODEDIR_NODE_SUBDIRS, 
                                                      SubDirMgr<SourceNodeDir>());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceNodeDir::NodeIter SourceNodeDir::SubNodesBegin ()
    {
    return SubDirIterMgr<NodeIter::value_type>().begin(IDTM_DIRECTORYID_SOURCENODEDIR_NODE_SUBDIRS, 
                                                       SubDirMgr<SourceNodeDir>());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceNodeDir::NodeIter SourceNodeDir::SubNodesEnd ()
    {
    return SubDirIterMgr<NodeIter::value_type>().end(IDTM_DIRECTORYID_SOURCENODEDIR_NODE_SUBDIRS, 
                                                     SubDirMgr<SourceNodeDir>());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceNodeDir* SourceNodeDir::AddGroupNode ()
    {
    static const Options OPTIONS(false);
    const CreateConfig DirCreateConfig;

    return SubDirMgr<SourceNodeDir>().Add(IDTM_DIRECTORYID_SOURCENODEDIR_NODE_SUBDIRS, DirCreateConfig, &OPTIONS);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceNodeDir* SourceNodeDir::AddSourcesNode ()
    {
    static const Options OPTIONS(true);
    const CreateConfig DirCreateConfig;

    return SubDirMgr<SourceNodeDir>().Add(IDTM_DIRECTORYID_SOURCENODEDIR_NODE_SUBDIRS, DirCreateConfig, &OPTIONS);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const SourceNodeDir* SourceNodeDir::SubNodeEditor::Get () const
    {
    return GetBase().Get(GetSubDirTagID(), GetSubDirIDIter());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceNodeDir* SourceNodeDir::SubNodeEditor::Get ()
    {
    return GetBase().Get(GetSubDirTagID(), GetSubDirIDIter());
    }


} //End namespace IDTMFile