#include <ScalableMeshPCH.h>
#include "../STM/ImagePPHeaders.h"
#include <ScalableMesh/Import/DataSQLite.h>
#include <ScalableMesh/Type/IScalableMeshLinear.h>
#include <ScalableMesh/Type/IScalableMeshPoint.h>
#include <ScalableMesh/Type/IScalableMeshTIN.h>
#include <ScalableMesh/Type/IScalableMeshMesh.h>

#ifdef VANCOUVER_API
#define WSTRING_FROM_CSTR(cstr) WString(cstr)
#else
#define WSTRING_FROM_CSTR(cstr) WString(cstr, BentleyCharEncoding::Utf8)
#endif


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

enum TypeFamilyID
    {
    TFID_POINT,
    TFID_LINEAR,
    TFID_TIN,
    TFID_MESH,

    TFID_QTY,
    };

uint32_t  OutputTypeID(const DataTypeFamily&                       type)
    {
    static const DataTypeFamily POINT_TYPE_FAMILY(PointTypeFamilyCreator().Create());
    static const DataTypeFamily LINEAR_TYPE_FAMILY(LinearTypeFamilyCreator().Create());
    static const DataTypeFamily TIN_TYPE_FAMILY(TINTypeFamilyCreator().Create());
    static const DataTypeFamily MESH_TYPE_FAMILY(MeshTypeFamilyCreator().Create());


    if (POINT_TYPE_FAMILY == type)
        return static_cast<byte>(TFID_POINT);
    else if (LINEAR_TYPE_FAMILY == type)
        return static_cast<byte>(TFID_LINEAR);
    else if (TIN_TYPE_FAMILY == type)
        return static_cast<byte>(TFID_TIN);
    else if (MESH_TYPE_FAMILY == type)
        return static_cast<byte>(TFID_MESH);
    else
        return UINT_MAX;

    }

struct SourceDataSQLite::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
        // ScalableMeshConfigComponent
        ScalableMeshData m_smData;
        WString m_extendedWktStr;
        uint32_t m_flags;
        byte m_typeFamilyID;
        uint32_t m_typeID;
       // uint32_t m_orgCount;
        uint32_t m_layer;
        bvector<ImportCommandData> m_commands;

        // LocalFileSource
        uint32_t m_sourceID;
        byte m_DTMSourceID;
        byte m_sourceType;
        uint32_t m_modelID;
        WString m_modelName;
        uint32_t m_levelID;
        WString m_levelName;
        WString m_rootToRefPersistentPath;
        WString m_referenceName;
        WString m_referenceModelName;

        // moniker
        byte m_monikerType;
        WString m_monikerString;

        std::vector<byte> m_configComponentID;

        //uint32_t m_commandCount;
        //std::vector<byte> m_commandID;

        time_t m_timeLastModified;

        uint32_t m_groupID; // ID 0 : root group, ie without group

    explicit                    Impl()
        : m_smData(ScalableMeshData::GetNull())
        {
        m_extendedWktStr = WSTRING_FROM_CSTR("");
            m_flags = 0;
            m_typeFamilyID = 0;
            m_typeID = 0;
           // m_orgCount = 0;
            m_layer = 0;

            m_sourceID = 0;
            m_DTMSourceID = 0;
            m_sourceType = 0;
            m_modelID = 0;
            m_modelName = WSTRING_FROM_CSTR("");
            m_levelID = 0;
            m_rootToRefPersistentPath = WSTRING_FROM_CSTR("");
            m_referenceName = WSTRING_FROM_CSTR("");
            m_referenceModelName = WSTRING_FROM_CSTR("");
            m_monikerType = 0;
            m_monikerString = WSTRING_FROM_CSTR("");
           // m_configComponentID = std::vector<byte>();
            //m_commandCount = 0;
            //m_commandID = std::vector<byte>();
            m_timeLastModified = 0;
            m_groupID = 0;
        }

    static void                   UpdateForEdit   (ImplPtr& instancePtr)
        {
        if(instancePtr->IsShared())
            instancePtr = new Impl(*instancePtr);
        }
    };

SourceDataSQLite::~SourceDataSQLite()
    {

    }

/*const ScalableMeshData& ScalableMeshData::GetNull()
    {
    static const ScalableMeshData NULL_ScalableMeshData(new Impl(std::vector<DRange3d>(), time_t()));
    return NULL_ScalableMeshData;
    }*/

SourceDataSQLite::SourceDataSQLite(Impl* implP)
    : m_implP(implP)
    {
    }

SourceDataSQLite::SourceDataSQLite(const SourceDataSQLite& rhs)
    {
    m_implP = new Impl();

    m_implP->m_smData = rhs.m_implP->m_smData;
    m_implP->m_extendedWktStr = rhs.m_implP->m_extendedWktStr;
    m_implP->m_flags = rhs.m_implP->m_flags;
    m_implP->m_typeFamilyID = rhs.m_implP->m_typeFamilyID;
    m_implP->m_typeID = rhs.m_implP->m_typeID;
   // m_implP->m_orgCount = rhs.m_implP->m_orgCount;
    m_implP->m_layer = rhs.m_implP->m_layer;

    m_implP->m_sourceID = rhs.m_implP->m_sourceID;
    m_implP->m_DTMSourceID = rhs.m_implP->m_DTMSourceID;
    m_implP->m_sourceType = rhs.m_implP->m_sourceType;
    m_implP->m_modelID = rhs.m_implP->m_modelID;
    m_implP->m_modelName = rhs.m_implP->m_modelName;
    m_implP->m_levelID = rhs.m_implP->m_levelID;
    m_implP->m_levelName = rhs.m_implP->m_levelName;
    m_implP->m_rootToRefPersistentPath = rhs.m_implP->m_rootToRefPersistentPath;
    m_implP->m_referenceName = rhs.m_implP->m_referenceName;
    m_implP->m_referenceModelName = rhs.m_implP->m_referenceModelName;

    m_implP->m_monikerType = rhs.m_implP->m_monikerType;
    m_implP->m_monikerString = rhs.m_implP->m_monikerString;

    m_implP->m_timeLastModified = rhs.m_implP->m_timeLastModified;
    //m_implP->m_commandCount = rhs.m_implP->m_commandCount;
    //m_implP->m_commandID = rhs.m_implP->m_commandID;
    m_implP->m_commands = rhs.m_implP->m_commands;

    m_implP->m_configComponentID = rhs.m_implP->m_configComponentID;

    m_implP->m_groupID = rhs.m_implP->m_groupID;
    }

SourceDataSQLite& SourceDataSQLite::operator=(const SourceDataSQLite&    rhs)
    {
    m_implP = rhs.m_implP;
    return *this;
    }

const SourceDataSQLite& SourceDataSQLite::GetNull()
{
    static const SourceDataSQLite NULL_SourceDataSQLite(new Impl());
    return NULL_SourceDataSQLite;
}

// ScalableMeshConfigComponent
void SourceDataSQLite::SetScalableMeshData(const ScalableMeshData& smData)
{
    m_implP->m_smData = smData;
}

void SourceDataSQLite::SetGCS(WString extendedWktStr)
{
    m_implP->m_extendedWktStr = extendedWktStr;
}
void SourceDataSQLite::SetFlags(uint32_t flags)
{
    m_implP->m_flags = flags;
}

void SourceDataSQLite::SetTypeFamilyID(byte typeFamilyID)
{
    m_implP->m_typeFamilyID = typeFamilyID;
}

void SourceDataSQLite::SetLayer(uint32_t layer)
{
    m_implP->m_layer = layer;
}


uint32_t SourceDataSQLite::GetTypeFamilyID()
{
    return m_implP->m_typeFamilyID;
}

WString SourceDataSQLite::GetGCS()
{
    return m_implP->m_extendedWktStr;
}

uint32_t SourceDataSQLite::GetFlags()
{
    return m_implP->m_flags;
}

ScalableMeshData SourceDataSQLite::GetScalableMeshData()
{
    return m_implP->m_smData;
}

uint32_t SourceDataSQLite::GetTypeID()
    {
    return m_implP->m_typeID;
    }
void SourceDataSQLite::SetTypeID(uint32_t typeID)
    {
    m_implP->m_typeID = typeID;
    }

// LocalFileSource
void SourceDataSQLite::SetSourceID(uint32_t sourceID)
{
    m_implP->m_sourceID = sourceID;
}

void SourceDataSQLite::SetDTMSourceID(byte DTMSourceID)
{
    m_implP->m_DTMSourceID = DTMSourceID;
}

void SourceDataSQLite::SetSourceType(byte sourceType)
{
    m_implP->m_sourceType = sourceType;
}

void SourceDataSQLite::SetModelID(uint32_t modelID)
{
    m_implP->m_modelID = modelID;
}

void SourceDataSQLite::SetModelName(WString modelName)
{
    m_implP->m_modelName = modelName;
}

void SourceDataSQLite::SetLevelID(uint32_t levelID)
{
    m_implP->m_levelID = levelID;
}

void SourceDataSQLite::SetLevelName(WString levelName)
{
    m_implP->m_levelName = levelName;
}

void SourceDataSQLite::SetRootToRefPersistentPath(WString rootToRefPersistentPath)
{
    m_implP->m_rootToRefPersistentPath = rootToRefPersistentPath;
}

void SourceDataSQLite::SetReferenceName(WString referenceName)
{
    m_implP->m_referenceName = referenceName;
}

void SourceDataSQLite::SetReferenceModelName(WString referenceModelName)
{
    m_implP->m_referenceModelName = referenceModelName;
}

uint32_t SourceDataSQLite::GetSourceID()
{
    return m_implP->m_sourceID;
}

byte SourceDataSQLite::GetDTMSourceID()
{
    return m_implP->m_DTMSourceID;
}

byte SourceDataSQLite::GetSourceType()
{
    return m_implP->m_sourceType;
}

uint32_t SourceDataSQLite::GetModelID() 
{
    return m_implP->m_modelID;
}

WString SourceDataSQLite::GetModelName()
{
    return m_implP->m_modelName;
}

uint32_t SourceDataSQLite::GetLevelID()
{
    return m_implP->m_levelID;
}

WString SourceDataSQLite::GetLevelName()
{
    return m_implP->m_levelName;
}

WString SourceDataSQLite::GetRootToRefPersistentPath()
{
    return m_implP->m_rootToRefPersistentPath;
}

WString SourceDataSQLite::GetReferenceName()
{
    return m_implP->m_referenceName;
}

WString SourceDataSQLite::GetReferenceModelName()
{
    return m_implP->m_referenceModelName;
}

// Moniker
void SourceDataSQLite::SetMonikerType(byte monikerType)
{
    m_implP->m_monikerType = monikerType;
}

byte SourceDataSQLite::GetMonikerType()
{
    return m_implP->m_monikerType;
}

void SourceDataSQLite::SetMonikerString(WString monikerString)
{
    m_implP->m_monikerString = monikerString;
}

WString SourceDataSQLite::GetMonikerString()
{
    return m_implP->m_monikerString;
}


/*void SourceDataSQLite::SetCommandCount(uint32_t commandCount)
{
    m_implP->m_commandCount = commandCount;
}

uint32_t SourceDataSQLite::GetCommandCount()
{
    return m_implP->m_commandCount;
}*/


void SourceDataSQLite::SetTimeLastModified(time_t time)
{
    m_implP->m_timeLastModified = time;
}

time_t SourceDataSQLite::GetTimeLastModified() const
{
    return m_implP->m_timeLastModified;
}


void SourceDataSQLite::SetGroupID(uint32_t id)
{
    m_implP->m_groupID = id;
}

uint32_t SourceDataSQLite::GetGroupID()
{
    return m_implP->m_groupID;
}

/*SourceDataSQLite&*/void SourcesDataSQLite::AddSourcesNode(SourceDataSQLite& sourceNodeData)
{
    //SourceDataSQLite sourceNodeData = SourceDataSQLite::GetNull();
    m_sourcesNodes.push_back(sourceNodeData);
    //return m_sourcesNodes[m_sourcesNodes.size() - 1];
}

void SourceDataSQLite::SetOrderedCommands(const bvector<ImportCommandData>& commands)
    {
    m_implP->m_commands = commands;
    }

bvector<ImportCommandData>& SourceDataSQLite::GetOrderedCommands()
    {
    return m_implP->m_commands;
    }

std::vector<SourceDataSQLite>& SourcesDataSQLite::GetSourceDataSQLite()
{
    return m_sourcesNodes;
}


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
