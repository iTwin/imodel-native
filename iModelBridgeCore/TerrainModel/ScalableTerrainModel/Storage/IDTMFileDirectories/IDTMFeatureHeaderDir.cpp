//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/IDTMFeatureHeaderDir.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureHeaderDir.h>
#include "../IDTMFileDefinition.h"

#include <STMInternal/Storage/HTGFFPacketManager.h>

using namespace HTGFF;

#define ARRAY_SIZE(Array) (sizeof(Array)/sizeof(Array[0]))
#define ARRAY_BEGIN(Array) (Array)
#define ARRAY_END(Array) ((Array) + ARRAY_SIZE(Array))


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
uint32_t FeatureHeaderDir::s_GetVersion ()
    {
    return DIRECTORY_VERSION;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Adapter for IDTM to HGTFF & HGTFF to IDTM feature types.
* @bsiclass                                                  Raymond.Gauthier   5/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class FeatureHeaderTypeDef
    {
public:
    enum DimensionRole
        {
        DR_GROUP_ID,
        DR_FEATURE_TYPE,
        DR_POINT_QUANTITY,
        DR_POINT_OFFSET,
        DR_QTY,
        };

    static const DataType&          GetDataType            (FeatureHeaderTypeID             pi_TypeID);
    static bool                     FindTypeID             (FeatureHeaderTypeID&             po_rTypeID,  
                                                            const DataType&                 pi_rDataType);
private:
    static const Dimension     HEADER_DIMENSIONS[];

    typedef map<DataType, FeatureHeaderTypeID>
    TypeMap;
    typedef TypeMap::value_type     TypePair;

    static const TypePair           DATA_TYPES[FEATURE_HEADER_TYPE_QTY];
    static const TypeMap            DATA_TYPE_MAP;
    };


inline const DataType& FeatureHeaderTypeDef::GetDataType   (FeatureHeaderTypeID  pi_TypeID)
    {
    return DATA_TYPES[pi_TypeID].first;
    }

/*---------------------------------------------------------------------------------**//**
* Defines all feature types in terms of HTGFF dimensions
+---------------+---------------+---------------+---------------+---------------+------*/
const Dimension FeatureHeaderTypeDef::HEADER_DIMENSIONS[] =
    {
    Dimension::CreateFrom(Dimension::TYPE_UINT32, DR_FEATURE_TYPE),
    Dimension::CreateFrom(Dimension::TYPE_UINT32, DR_POINT_OFFSET),
    Dimension::CreateFrom(Dimension::TYPE_UINT32, DR_POINT_QUANTITY),
    Dimension::CreateFrom(Dimension::TYPE_UINT32, DR_GROUP_ID),
    };

const FeatureHeaderTypeDef::TypePair FeatureHeaderTypeDef::DATA_TYPES[FEATURE_HEADER_TYPE_QTY] =
    {
    TypePair(DataType(ARRAY_BEGIN(HEADER_DIMENSIONS),     ARRAY_END(HEADER_DIMENSIONS)),     FEATURE_HEADER_TYPE_FEATURE),
    };

const FeatureHeaderTypeDef::TypeMap FeatureHeaderTypeDef::DATA_TYPE_MAP(ARRAY_BEGIN(DATA_TYPES), ARRAY_END(DATA_TYPES));


bool FeatureHeaderTypeDef::FindTypeID  (FeatureHeaderTypeID&         po_rTypeID,
                                        const DataType&    pi_rDataType)
    {
    TypeMap::const_iterator FoundIt = DATA_TYPE_MAP.find(pi_rDataType);

    if (DATA_TYPE_MAP.end() == FoundIt)
        return false;

    po_rTypeID = FoundIt->second;
    return true;
    }


size_t GetTypeSize (FeatureHeaderTypeID pi_Type)
    {
    HPRECONDITION(FEATURE_HEADER_TYPE_QTY > pi_Type);
    if (FEATURE_HEADER_TYPE_QTY <= pi_Type)
        return 0;

    return FeatureHeaderTypeDef::GetDataType(pi_Type).GetSize();
    }

const DataType& GetTypeDescriptor (FeatureHeaderTypeID pi_Type)
    {
    return FeatureHeaderTypeDef::GetDataType(pi_Type);
    }



FeatureHeaderDir::FeatureHeaderDir ()
    :   Directory(DIRECTORY_VERSION),
        m_HeaderTypeSize(0),
        m_HeaderType(FEATURE_HEADER_TYPE_FEATURE)
    {

    }

size_t FeatureHeaderDir::GetTileMaxHeaderCount () const
    {
    HPRECONDITION(0 == (PacketMgr().GetMaxSize() % (sizeof FeatureHeader)));
    return PacketMgr().GetMaxSize() / (sizeof FeatureHeader);
    }

uint64_t FeatureHeaderDir::CountHeaders () const
    {
    HPRECONDITION(0 == (PacketMgr().GetTotalSize() % (sizeof FeatureHeader)));
    return PacketMgr().GetTotalSize() / (sizeof FeatureHeader);
    }

size_t FeatureHeaderDir::CountHeaders (TileID pi_ID) const
    {
    HPRECONDITION(pi_ID != GetNullTileID());
    HPRECONDITION(0 == (PacketMgr().GetSize(pi_ID) % (sizeof FeatureHeader)));
    return PacketMgr().GetSize(pi_ID) / (sizeof FeatureHeader);
    }


FeatureHeaderTypeID FeatureHeaderDir::GetHeaderType () const
    {
    return m_HeaderType;
    }

bool FeatureHeaderDir::GetHeaders  (TileID  pi_ID,
                                    Packet& po_rHeaders) const
    {
    HPRECONDITION(pi_ID != GetNullTileID());
    return PacketMgr().Get(pi_ID, po_rHeaders);
    }

bool FeatureHeaderDir::SetHeaders  (TileID          pi_ID,
                                    const Packet&   pi_rHeaders)
    {
    HPRECONDITION(pi_ID != GetNullTileID());
    return PacketMgr().Set(pi_ID, pi_rHeaders);
    }

bool FeatureHeaderDir::AddHeaders  (TileID&         po_rID,
                                    const Packet&   pi_rHeaders)
    {
    return PacketMgr().Add(po_rID, pi_rHeaders);
    }

bool FeatureHeaderDir::RemoveHeaders   (TileID  pi_ID)
    {
    HPRECONDITION(pi_ID != GetNullTileID());
    return PacketMgr().Remove(pi_ID);
    }


bool FeatureHeaderDir::_Load (const UserOptions*      pi_pUserOptions)
    {
    bool Success = true;

    m_HeaderTypeSize = PacketMgr().GetType().GetSize();
    HASSERT(0 != m_HeaderTypeSize);

    if (!FeatureHeaderTypeDef::FindTypeID(m_HeaderType, PacketMgr().GetType()))
        {
        HASSERT(!"Unsupported header type");
        m_HeaderType = FEATURE_HEADER_TYPE_FEATURE;
        Success     = false;
        }

    return Success;
    }

} //End namespace IDTMFile