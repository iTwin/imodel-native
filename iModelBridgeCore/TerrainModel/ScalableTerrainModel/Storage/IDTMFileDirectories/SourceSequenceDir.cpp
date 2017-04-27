//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/SourceSequenceDir.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ScalableTerrainModelPCH.h>



#include <STMInternal/Storage/IDTMFileDirectories/SourceSequenceDir.h>
#include "IDTMCommonDirTools.h"

#include "../IDTMFileDefinition.h"

#include <STMInternal/Storage/HTGFFPacketManager.h>
#include <STMInternal/Storage/HTGFFSubDirManager.h>

#include <STMInternal/Storage/HTGFFPacketIdIter.h>

using namespace HTGFF;
using namespace HPU;


namespace {
/* 
 * VERSIONNING 
 * Version 0:
 *      Current.
 */

const uint32_t DIRECTORY_VERSION = 0;
}

namespace IDTMFile {

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t SourceSequenceDir::s_GetVersion ()
    {
    return DIRECTORY_VERSION;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceSequenceDir::SourceID SourceSequenceDir::SourceEditor::GetID () const
    {
    return GetPacketID();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceSequenceDir::SourceEditor::GetSerializedSource (SerializedSourcePacket& po_rMoniker) const
    {
    return GetBase().m_pSerializedSources->Get(GetID(), po_rMoniker);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceSequenceDir::SourceEditor::GetContentConfig (ContentConfigPacket& po_rContentConfig) const
    {
    return GetBase().m_pContentConfigs->Get(GetID(), po_rContentConfig);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceSequenceDir::SourceEditor::GetImportSequence (ImportSequencePacket& po_rImportSequence) const
    {
    return GetBase().m_pImportSequences->Get(GetID(), po_rImportSequence);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceSequenceDir::SourceEditor::GetImportConfig  (ImportConfigPacket& po_rImportConfig) const
    {
    return GetBase().m_pImportConfigs->Get(GetID(), po_rImportConfig);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceSequenceDir::TimeType SourceSequenceDir::SourceEditor::GetLastModified () const
    {
    return ArrayHelper().GetIndex(GetBase().m_lastModifiedTimeStamps, GetID(), TimeType());
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceSequenceDir::SourceSequenceDir ()
    :   Directory(DIRECTORY_VERSION),
        m_pSerializedSources(0),
        m_pContentConfigs(0),
        m_pImportSequences(0),
        m_pImportConfigs(0)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceSequenceDir::~SourceSequenceDir ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceSequenceDir::_Create(const CreateConfig&     pi_rCreateConfig,
                                const UserOptions*      pi_pUserOptions)
    {
    bool Success = true;

    static const CreateConfig BinaryAttributeConfig(DataType::CreateByte(), Compression::Deflate::Create());
    static const CreateConfig StringAttributeConfig(DataType::CreateChar(), Compression::Deflate::Create());
    static const CreateConfig WStringAttributeConfig(DataType::CreateWideChar(), Compression::Deflate::Create());

    m_pSerializedSources = SubDirMgr<SerializedSourcesDir>().Create(IDTM_DIRECTORYID_SOURCESEQUENCEDIR_SERIALIZEDSOURCES_SUBDIR, BinaryAttributeConfig);
    Success &= 0 != m_pSerializedSources;

    m_pContentConfigs = SubDirMgr<ContentConfigsDir>().Create(IDTM_DIRECTORYID_SOURCESEQUENCEDIR_CONTENTCONFIGS_SUBDIR, BinaryAttributeConfig);
    Success &= 0 != m_pContentConfigs;

    m_pImportSequences = SubDirMgr<ImportSequencesDir>().Create(IDTM_DIRECTORYID_SOURCESEQUENCEDIR_IMPORTSEQUENCES_SUBDIR, BinaryAttributeConfig);
    Success &= 0 != m_pImportSequences;

    m_pImportConfigs = SubDirMgr<ImportConfigsDir>().Create(IDTM_DIRECTORYID_SOURCESEQUENCEDIR_IMPORTCONFIGS_SUBDIR, BinaryAttributeConfig);
    Success &= 0 != m_pImportConfigs;


    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceSequenceDir::_Load (const UserOptions*       pi_pUserOptions)
    {
    bool Success = true;

    if (0 == m_pSerializedSources)
        m_pSerializedSources = SubDirMgr<SerializedSourcesDir>().Get(IDTM_DIRECTORYID_SOURCESEQUENCEDIR_SERIALIZEDSOURCES_SUBDIR);
    Success &= 0 != m_pSerializedSources;

    if (0 == m_pContentConfigs)
        m_pContentConfigs = SubDirMgr<ContentConfigsDir>().Get(IDTM_DIRECTORYID_SOURCESEQUENCEDIR_CONTENTCONFIGS_SUBDIR);
    Success &= 0 != m_pContentConfigs;

    if (0 == m_pImportSequences)
        m_pImportSequences = SubDirMgr<ImportSequencesDir>().Get(IDTM_DIRECTORYID_SOURCESEQUENCEDIR_IMPORTSEQUENCES_SUBDIR);
    Success &= 0 != m_pImportSequences;

    if (0 == m_pImportConfigs)
        m_pImportConfigs = SubDirMgr<ImportConfigsDir>().Get(IDTM_DIRECTORYID_SOURCESEQUENCEDIR_IMPORTCONFIGS_SUBDIR);
    Success &= 0 != m_pImportConfigs;

    Success &= AttributeHelper<uint64_t>().LoadIfPresent(*this, IDTM_ATTRIBUTEID_SOURCESEQUENCEDIR_LAST_MODIFIED_TIME_STAMPS, m_lastModifiedTimeStamps);

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceSequenceDir::_Save ()
    {
    bool Success = true;
    Success &= AttributeHelper<uint64_t>().SaveIfChanged(*this, IDTM_ATTRIBUTEID_SOURCESEQUENCEDIR_LAST_MODIFIED_TIME_STAMPS, m_lastModifiedTimeStamps);

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceSequenceDir::SourceCIter SourceSequenceDir::SourcesBegin () const
    {
    return PacketIter<CSource>::Create(m_pSerializedSources->beginId(), *this);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceSequenceDir::SourceCIter SourceSequenceDir::SourcesEnd () const
    {
    return PacketIter<CSource>::Create(m_pSerializedSources->endId(), *this);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceSequenceDir::SourceIter SourceSequenceDir::SourcesBegin ()
    {
    return PacketIter<Source>::Create(m_pSerializedSources->beginId(), *this);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceSequenceDir::SourceIter SourceSequenceDir::SourcesEnd ()
    {
    return PacketIter<Source>::Create(m_pSerializedSources->endId(), *this);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceSequenceDir::AddSource  (const SerializedSourcePacket&   pi_rSerializedSource,
                                    const WString&                  pi_rGCSWKTOverride,
                                    TimeType                        pi_lastModified)
    {
    bool Success = true;
    HASSERT(!"Deprecated!");

    PacketID id;
    Success &= m_pSerializedSources->Add(id, pi_rSerializedSource);

    HASSERT(id == m_lastModifiedTimeStamps.GetSize());
    m_lastModifiedTimeStamps.push_back(pi_lastModified);

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceSequenceDir::AddSource  (TimeType                        pi_lastModified,
                                    const SerializedSourcePacket&   pi_rSerializedSource,
                                    const ContentConfigPacket&      pi_rContentConfig,
                                    const ImportSequencePacket&     pi_rImportSequence,
                                    const ImportConfigPacket&       pi_rImportConfig)
    {
    bool Success = true;

    PacketID id;
    Success &= m_pSerializedSources->Add(id, pi_rSerializedSource);

    HASSERT(id == m_lastModifiedTimeStamps.GetSize());
    m_lastModifiedTimeStamps.push_back(pi_lastModified);

    Success &= m_pContentConfigs->Set(id, pi_rContentConfig);
    Success &= m_pImportSequences->Set(id, pi_rImportSequence);
    Success &= m_pImportConfigs->Set(id, pi_rImportConfig);

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceSequenceDir::AddSource  (TimeType                        pi_lastModified,
                                    const SerializedSourcePacket&   pi_rSerializedSource,
                                    const ContentConfigPacket&      pi_rContentConfig,
                                    const ImportSequencePacket&     pi_rImportSequence)
    {
    static const HPU::Packet EMPTY_PACKET;
    return AddSource(pi_lastModified, pi_rSerializedSource, pi_rContentConfig, pi_rImportSequence, EMPTY_PACKET);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SourceSequenceDir::ClearAll ()
    {
    return SubDirMgr().RemoveSubDirs();
    }

} //End namespace IDTMFile