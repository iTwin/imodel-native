#pragma once
#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Import/ScalableMeshData.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

class SourceDataSQLite
{
private:
    struct Impl;
    typedef RefCountedPtr<Impl> ImplPtr;
    ImplPtr m_implP;

    explicit SourceDataSQLite(Impl* implP);

public:

//  SourceDataSQLitetaSQLite() :m_smData(ScalableMeshData::GetNull()) {}
    IMPORT_DLLE SourceDataSQLite(const SourceDataSQLite& rhs);
    IMPORT_DLLE SourceDataSQLite& operator= (const SourceDataSQLite& rhs);
    IMPORT_DLLE ~SourceDataSQLite();

    IMPORT_DLLE static const SourceDataSQLite& GetNull();

    // ScalableMeshConfigComponent
    void SetScalableMeshData(const ScalableMeshData& smData);
    void SetGCS(WString extendedWktStr);
    void SetFlags(uint32_t flags);
    void SetTypeFamilyID(byte typeFamilyID);
    void SetOrgCount(uint32_t orgCount);
    void SetLayer(uint32_t layer);
    void SetComponentCount(uint32_t componentCount);
    void AddConfigComponentID(byte id);
    void SetConfigComponentID(std::vector<byte>& vecID);
    void AddDimensionCount(uint32_t dimensionCount);
    void AddDimensionType(size_t orgIdx, byte dimensionType);
    void AddDimensionRole(size_t orgIdx, byte dimensionRole);
    void AddDimTypeName(size_t orgIdx, WString name); // maybe add ID and initialize vector, because if dimType is known, we didn't ask for name
    void ResizeDimensions(size_t orgSize);
    void SetDimensionCount(std::vector<uint32_t>& vec);
    void SetDimensionType(std::vector<std::vector<byte>>& vec);
    void SetDimensionRole(std::vector<std::vector<byte>>& vec);
    void SetDimensionName(std::vector<std::vector<WString>>& vec);

    uint32_t GetTypeFamilyID();
    WString GetGCS();
    uint32_t GetFlags();
    uint32_t GetOrgCount();
    uint32_t GetLayer();
    uint32_t GetDimensionCount(size_t orgIdx);
    std::vector<uint32_t>& GetVecDimensionCount();
    byte GetDimensionType(size_t orgIdx, size_t dimIdx);
    std::vector<std::vector<byte>>& GetVecDimensionType();
    byte GetDimensionRole(size_t orgIdx, size_t dimIdx);
    std::vector<std::vector<byte>>& GetVecDimensionRole();
    WString GetDimensionTypeName(size_t orgIdx, size_t dimIdx);
    std::vector<std::vector<WString>>& GetVecDimensionName();
    uint32_t GetComponentID(size_t id);
    byte PopConfigComponentID(/*size_t id*/);//POP
    std::vector<byte>& GetConfigComponentID();
    uint32_t GetComponentCount();
    ScalableMeshData GetScalableMeshData();

    // LocalFileSource
    void SetSourceID(uint32_t sourceID);
    void SetDTMSourceID(byte DTMSourceID);
    void SetSourceType(byte sourceType);
    void SetModelID(uint32_t modelID);
    void SetModelName(WString modelName);
    void SetLevelID(uint32_t levelID);
    void SetLevelName(WString levelName);
    void SetRootToRefPersistentPath(WString rootToRefPersistentPath);
    void SetReferenceName(WString referenceName);
    void SetReferenceModelName(WString referenceModelName);

    uint32_t GetSourceID();
    byte GetDTMSourceID();
    byte GetSourceType();
    uint32_t GetModelID();
    WString GetModelName();
    uint32_t GetLevelID();
    WString GetLevelName();
    WString GetRootToRefPersistentPath();
    WString GetReferenceName();
    WString GetReferenceModelName();

    // Moniker
    IMPORT_DLLE void SetMonikerType(byte monikerType);
    IMPORT_DLLE void SetMonikerString(WString monikerString);

    IMPORT_DLLE byte GetMonikerType();
    IMPORT_DLLE WString GetMonikerString();

    void SetCommandCount(uint32_t);

    uint32_t GetCommandCount();

    void SetCommandID(std::vector<byte>& vecID);
    void AddCommandID(byte id);

    std::vector<byte>& GetCommandID();
    byte PopCommandID();


    void SetTimeLastModified(time_t time);

    time_t GetTimeLastModified() const;

    void SetGroupID(uint32_t id);
    uint32_t GetGroupID();
    /*typedef HPU::Packet                  Packet;
    typedef Packet                      SerializedSourcePacket;
    typedef Packet                      ContentConfigPacket;
    typedef Packet                      ImportSequencePacket;
    typedef Packet                      ImportConfigPacket;

    typedef HTGFF::PacketIndexSubDir    SerializedSourcesDir;
    SerializedSourcesDir*               m_pSerializedSources;
    typedef HTGFF::PacketIndexSubDir    ContentConfigsDir;
    ContentConfigsDir*                  m_pContentConfigs;
    typedef HTGFF::PacketIndexSubDir    ImportSequencesDir;
    ImportSequencesDir*                 m_pImportSequences;
    typedef HTGFF::PacketIndexSubDir    ImportConfigsDir;
    ImportConfigsDir*                   m_pImportConfigs;*/

    std::vector<time_t>                 m_lastModifiedTimeStamps;

    /*bool AddSource(time_t pi_lastModified, const SerializedSourcePacket& pi_rSerializedSource,
    const ContentConfigPacket& pi_rContentConfig, const ImportSequencePacket& pi_rImportSequence);
    bool  AddSource(time_t pi_lastModified, const SerializedSourcePacket&   pi_rSerializedSource,
    const ContentConfigPacket& pi_rContentConfig, const ImportSequencePacket& pi_rImportSequence,
    const ImportConfigPacket& pi_rImportConfig);*/

};

class SourcesDataSQLite
{
    //Replace by a list of sourcesDataSQLite
    //and method to add new sources, etc... like SourcesDir
    time_t m_lastModifiedTime;
    time_t m_lastSyncTime;
    time_t m_checkTime;
    uint32_t m_serializedSourceFormatVersion;
    uint32_t m_contentConfigFormatVersion;
    uint32_t m_importSequenceFormatVersion;
    uint32_t m_importConfigFormatVersion;
    uint32_t m_currentSourceID;
    uint32_t m_currentGroupID;
    bool m_isGroup;

    std::vector<SourceDataSQLite> m_sourcesNodes;

public:
    SourcesDataSQLite() : m_currentGroupID(0), m_currentSourceID(0), m_isGroup(false) {}

    void SetLastModifiedCheckTime(time_t    pi_checkTime) { m_checkTime = pi_checkTime; }
    void SetLastModifiedTime(time_t pi_lastModifiedTime) { m_lastModifiedTime = pi_lastModifiedTime; }
    void SetLastSyncTime(time_t pi_lastSyncTime) { m_lastSyncTime = pi_lastSyncTime; }
    void SetSerializedSourceFormatVersion(uint32_t pi_version) { m_serializedSourceFormatVersion = pi_version; }
    void SetContentConfigFormatVersion(uint32_t pi_version) { m_contentConfigFormatVersion = pi_version; }
    void SetImportSequenceFormatVersion(uint32_t pi_version) { m_importSequenceFormatVersion = pi_version; }
    void SetImportConfigFormatVersion(uint32_t pi_version) { m_importConfigFormatVersion = pi_version; }

    time_t GetLastModifiedCheckTime() { return m_checkTime; }
    time_t GetLastModifiedTime() { return m_lastModifiedTime; }
    time_t GetLastSyncTime() { return m_lastSyncTime; }
    uint32_t GetSerializedSourceFormatVersion() { return m_serializedSourceFormatVersion; }
    uint32_t GetContentConfigFormatVersion() { return m_contentConfigFormatVersion; }
    uint32_t GetImportSequenceFormatVersion() { return m_importSequenceFormatVersion; }
    uint32_t GetImportConfigFormatVersion() { return m_importConfigFormatVersion; }

    void SetIsGroup(bool group) { m_isGroup = group; }
    bool IsGroup() { return m_isGroup; }

    void IncreaseCurrentSourceID() { m_currentSourceID++; }
    void IncreaseCurrentGroupID() { m_currentGroupID++; }
    void DecreaseCurrentGroupID() { m_currentGroupID--; }
    void SetCurrentGroupID(uint32_t id) { m_currentGroupID = id; }
    void SetCurrentSourceID(uint32_t id) { m_currentSourceID = id; }

    uint32_t GetCurrentGroupID() { return m_currentGroupID; }
    uint32_t GetCurrentSourceID() { return m_currentSourceID; }

    std::vector<SourceDataSQLite>& GetSourceDataSQLite();

    void AddSourcesNode(SourceDataSQLite& sourceNodeData);
};

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE