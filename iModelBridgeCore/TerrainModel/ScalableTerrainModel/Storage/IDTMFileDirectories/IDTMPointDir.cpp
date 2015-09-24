//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/IDTMPointDir.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/IDTMPointDir.h>
#include "IDTMCommonDirTools.h"

#include "../IDTMFileDefinition.h"

#include <STMInternal/Storage/HTGFFPacketManager.h>
#include <STMInternal/Storage/HTGFFSubDirManager.h>
#include <STMInternal/Storage/HTGFFAttributeManager.h>

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
uint32_t PointDir::s_GetVersion ()
    {
    return DIRECTORY_VERSION;
    }


namespace { // BEGIN unnamed namespace

/*---------------------------------------------------------------------------------**//**
* @description  Adapter for IDTM to HGTFF & HGTFF to IDTM points types.
* @bsiclass                                                  Raymond.Gauthier   5/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class IDTMPointTypeDef
    {
public:
    enum DimensionRole
        {
        DR_GROUP_ID,
        DR_COORD_X,
        DR_COORD_Y,
        DR_COORD_Z,
        DR_COLOR_COMP_R,
        DR_COLOR_COMP_G,
        DR_COLOR_COMP_B,
        DR_COLOR_COMP_I,
        DR_SIGNIFICANCE_LEVEL,
        DR_QTY,
        };

    static const DataType&      GetDataType            (PointTypeID                 pi_TypeID);
    static bool                 FindTypeID             (PointTypeID&                po_rTypeID,
                                                        const DataType&             pi_rDataType);

private:
    static const Dimension     XYZ_DIMENSIONS[];
    static const Dimension     XYZRGBI_DIMENSIONS[];
    static const Dimension     XYZG_DIMENSIONS[];
    static const Dimension     XYZM_DIMENSIONS[];
    static const Dimension     XYZMG_DIMENSIONS[];
    static const Dimension     XY_DIMENSIONS[];
    static const Dimension     XYRGBI_DIMENSIONS[];
    static const Dimension     XYG_DIMENSIONS[];
    static const Dimension     INT_DIMENSIONS[];

    typedef map<DataType, PointTypeID>
    TypeMap;
    typedef TypeMap::value_type     TypePair;

    static const TypePair           DATA_TYPES[POINT_TYPE_QTY];
    static const TypeMap            DATA_TYPE_MAP;
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline const DataType& IDTMPointTypeDef::GetDataType (PointTypeID  pi_TypeID)
    {
    return DATA_TYPES[pi_TypeID].first;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool IDTMPointTypeDef::FindTypeID  (PointTypeID&            po_rTypeID,
                                    const DataType&    pi_rDataType)
    {
    TypeMap::const_iterator FoundIt = DATA_TYPE_MAP.find(pi_rDataType);

    if (DATA_TYPE_MAP.end() == FoundIt)
        return false;

    po_rTypeID = FoundIt->second;
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Defines all points types in terms of HTGFF dimensions
+---------------+---------------+---------------+---------------+---------------+------*/
const Dimension IDTMPointTypeDef::XYZ_DIMENSIONS[] =
    {
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_X),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_Y),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_Z),
    };

const Dimension IDTMPointTypeDef::XYZRGBI_DIMENSIONS[]  =
    {
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_X),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_Y),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_Z),
    Dimension::CreateFrom(Dimension::TYPE_UINT8,      DR_COLOR_COMP_R),
    Dimension::CreateFrom(Dimension::TYPE_UINT8,      DR_COLOR_COMP_G),
    Dimension::CreateFrom(Dimension::TYPE_UINT8,      DR_COLOR_COMP_B),
    Dimension::CreateFrom(Dimension::TYPE_UINT8,      DR_COLOR_COMP_I),
    };

const Dimension IDTMPointTypeDef::XYZG_DIMENSIONS[]    =
    {
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_X),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_Y),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_Z),
    Dimension::CreateFrom(Dimension::TYPE_UINT32,     DR_GROUP_ID),
    };

const Dimension IDTMPointTypeDef::XYZM_DIMENSIONS[]    =
    {
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_X),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_Y),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_Z),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_SIGNIFICANCE_LEVEL),
    };

const Dimension IDTMPointTypeDef::XYZMG_DIMENSIONS[]    =
    {
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_X),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_Y),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_Z),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_SIGNIFICANCE_LEVEL),
    Dimension::CreateFrom(Dimension::TYPE_UINT32,     DR_GROUP_ID),
    };

const Dimension IDTMPointTypeDef::XY_DIMENSIONS[]       =
    {
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_X),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_Y),
    };

const Dimension IDTMPointTypeDef::XYRGBI_DIMENSIONS[]   =
    {
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_X),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_Y),
    Dimension::CreateFrom(Dimension::TYPE_UINT8,      DR_COLOR_COMP_R),
    Dimension::CreateFrom(Dimension::TYPE_UINT8,      DR_COLOR_COMP_G),
    Dimension::CreateFrom(Dimension::TYPE_UINT8,      DR_COLOR_COMP_B),
    Dimension::CreateFrom(Dimension::TYPE_UINT8,      DR_COLOR_COMP_I),
    };

const Dimension IDTMPointTypeDef::XYG_DIMENSIONS[]     =
    {
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_X),
    Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_COORD_Y),
    Dimension::CreateFrom(Dimension::TYPE_UINT32,     DR_GROUP_ID),
    };

const Dimension IDTMPointTypeDef::INT_DIMENSIONS[] =
    {
    Dimension::CreateFrom(Dimension::TYPE_UINT32, DR_GROUP_ID),
    };


const IDTMPointTypeDef::TypePair IDTMPointTypeDef::DATA_TYPES[POINT_TYPE_QTY] =
    {
    TypePair(DataType(ARRAY_BEGIN(XYZ_DIMENSIONS),     ARRAY_END(XYZ_DIMENSIONS)),     POINT_TYPE_XYZf64),
    TypePair(DataType(ARRAY_BEGIN(XYZRGBI_DIMENSIONS), ARRAY_END(XYZRGBI_DIMENSIONS)), POINT_TYPE_XYZf64RGBIi8),
    TypePair(DataType(ARRAY_BEGIN(XYZG_DIMENSIONS),    ARRAY_END(XYZG_DIMENSIONS)),    POINT_TYPE_XYZf64Gi32),
    TypePair(DataType(ARRAY_BEGIN(XYZM_DIMENSIONS),    ARRAY_END(XYZM_DIMENSIONS)),    POINT_TYPE_XYZMf64),
    TypePair(DataType(ARRAY_BEGIN(XYZMG_DIMENSIONS),   ARRAY_END(XYZMG_DIMENSIONS)),   POINT_TYPE_XYZMf64Gi32),
    TypePair(DataType(ARRAY_BEGIN(XY_DIMENSIONS),      ARRAY_END(XY_DIMENSIONS)),      POINT_TYPE_XYf64),
    TypePair(DataType(ARRAY_BEGIN(XYRGBI_DIMENSIONS),  ARRAY_END(XYRGBI_DIMENSIONS)),  POINT_TYPE_XYf64RGBIi8),
    TypePair(DataType(ARRAY_BEGIN(XYG_DIMENSIONS),     ARRAY_END(XYG_DIMENSIONS)),     POINT_TYPE_XYf64Gi32),
    TypePair(DataType(ARRAY_BEGIN(INT_DIMENSIONS), ARRAY_END(INT_DIMENSIONS)), POINT_TYPE_INTEGER)
    };

const IDTMPointTypeDef::TypeMap IDTMPointTypeDef::DATA_TYPE_MAP(ARRAY_BEGIN(DATA_TYPES), ARRAY_END(DATA_TYPES));



/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class ResolutionTypeDef
    {
    enum DimensionRole
        {
        DR_Resolution,
        DR_QTY,
        };

public:
    static const DataType&      GetType                ()
        {
        static const DataType DATA_TYPE (Dimension::CreateFrom(Dimension::TYPE_UINT32, DR_Resolution));
        return DATA_TYPE;
        }
    };

} // END unnamed namespace


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GetTypeSize (PointTypeID pi_Type)
    {
    HPRECONDITION(POINT_TYPE_QTY > pi_Type);
    if (POINT_TYPE_QTY <= pi_Type)
        return 0;

    return IDTMPointTypeDef::GetDataType(pi_Type).GetSize();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& GetTypeDescriptor (PointTypeID pi_Type)
    {
    return IDTMPointTypeDef::GetDataType(pi_Type);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
PointDir::PointDir ()
    :   Directory(DIRECTORY_VERSION),
        m_pExtentsDir(0),
        m_pResolutionsDir(0),
        m_PointTypeSize(0),
        m_PointType(POINT_TYPE_XYZf64)

    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointDir::_Create     (const CreateConfig&     pi_rCreateConfig,
                            const UserOptions*      pi_pUserOptions)
    {
    static ExtentsDir::Options EXTENTS_DIR_OPTIONS(IDTM_DEFAULT.extent3d);
    static ResolutionsDir::Options RESOLUTIONS_DIR_OPTIONS(IDTM_DEFAULT.resolution);

    const CreateConfig ExtentsDirConfig(ExtentTypeDef::GetType(), GetCompressType());
    const CreateConfig ResolutionsDirConfig(ResolutionTypeDef::GetType(), GetCompressType());

    bool Success = true;

    m_pExtentsDir = SubDirMgr<ExtentsDir>().Create(IDTM_DIRECTORYID_POINTDIR_TILES_3D_EXTENT_SUBDIR, ExtentsDirConfig, &EXTENTS_DIR_OPTIONS);
    Success &= 0 != m_pExtentsDir;

    m_pResolutionsDir = SubDirMgr<ResolutionsDir>().Create(IDTM_DIRECTORYID_POINTDIR_TILES_RESOLUTION_SUBDIR, ResolutionsDirConfig, &RESOLUTIONS_DIR_OPTIONS);
    Success &= 0 != m_pResolutionsDir;

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Load tile directory
* @return       true on success, false otherwise
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointDir::_Load (const UserOptions*      pi_pUserOptions)
    {
    static ExtentsDir::Options EXTENTS_DIR_OPTIONS(IDTM_DEFAULT.extent3d);
    static ResolutionsDir::Options RESOLUTIONS_DIR_OPTIONS(IDTM_DEFAULT.resolution);

    bool Success = true;

    if (0 == m_pExtentsDir)
        m_pExtentsDir = SubDirMgr<ExtentsDir>().Get(IDTM_DIRECTORYID_POINTDIR_TILES_3D_EXTENT_SUBDIR, &EXTENTS_DIR_OPTIONS);
    Success &= 0 != m_pExtentsDir;

    if (0 == m_pResolutionsDir)
        m_pResolutionsDir = SubDirMgr<ResolutionsDir>().Get(IDTM_DIRECTORYID_POINTDIR_TILES_RESOLUTION_SUBDIR, &RESOLUTIONS_DIR_OPTIONS);
    Success &= 0 != m_pResolutionsDir;

    m_PointTypeSize = PacketMgr().GetType().GetSize();
    HASSERT(0 != m_PointTypeSize);

    if (!IDTMPointTypeDef::FindTypeID(m_PointType, PacketMgr().GetType()))
        {
        HASSERT(!"Unsupported point type");
        m_PointType = POINT_TYPE_XYZf64;
        Success     = false;
        }


    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Save tile directory
* @return       true on success, false otherwise
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointDir::_Save ()
    {
    bool Success = true;
    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const Extent3d64f& PointDir::GetExtent (TileID  pi_ID) const
    {
    HPRECONDITION(pi_ID != GetNullTileID());
    return m_pExtentsDir->Get(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Extent3d64f& PointDir::EditExtent (TileID pi_ID)
    {
    HPRECONDITION(pi_ID != GetNullTileID());
    return m_pExtentsDir->Edit(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ResolutionID PointDir::GetResolution (TileID pi_ID) const
    {
    HPRECONDITION(pi_ID != GetNullTileID());
    return m_pResolutionsDir->Get(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void PointDir::SetResolution   (TileID       pi_ID,
                                ResolutionID pi_Resolution)
    {
    HPRECONDITION(pi_ID != GetNullTileID());
    m_pResolutionsDir->Edit(pi_ID) = pi_Resolution;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PointDir::CountTiles () const
    {
    return PacketMgr().GetCount();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
PointTypeID PointDir::GetPointType () const

    {
    return m_PointType;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PointDir::GetTileMaxPointCount () const
    {
    HPRECONDITION(0 == (PacketMgr().GetMaxSize() % m_PointTypeSize));
    return PacketMgr().GetMaxSize() / m_PointTypeSize;
    }


/*---------------------------------------------------------------------------------**//**
* @description  Returns the quantity of points stored in the whole directory
* @return       Quantity of points stored for the whole directory.
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t PointDir::CountPoints () const
    {
    HPRECONDITION(0 == (PacketMgr().GetTotalSize() % m_PointTypeSize));
    return PacketMgr().GetTotalSize() / m_PointTypeSize;
    }


/*---------------------------------------------------------------------------------**//**
* @description  Returns the quantity of points stored in the specified tile
* @param        IN  pi_ID           The tile for which we want to request information
* @return       Quantity of points stored for the specified tile. 0 is also returned
*               when an inexistent/invalid tile is specified.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PointDir::CountPoints (TileID pi_ID) const
    {
    HPRECONDITION(0 == (PacketMgr().GetSize(pi_ID) % m_PointTypeSize));
    HPRECONDITION(pi_ID != GetNullTileID());
    return PacketMgr().GetSize(pi_ID) / m_PointTypeSize;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const Compression& PointDir::GetCompressType () const
    {
    return PacketMgr().GetCompression();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
double PointDir::GetCompressionRatio () const
    {
    return PacketMgr().GetCompressionRatio();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointDir::GetPoints       (TileID          pi_ID,
                                Packet&         po_rPoints) const
    {
    HPRECONDITION(pi_ID != GetNullTileID());
    return PacketMgr().Get(pi_ID, po_rPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointDir::SetPoints       (TileID          pi_ID,
                                const Packet&   pi_rPoints)
    {
    HPRECONDITION(pi_ID != GetNullTileID());
    return PacketMgr().Set(pi_ID, pi_rPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointDir::AddPoints       (TileID&         po_rID,
                                const Packet&   pi_rPoints)
    {
    return PacketMgr().Add(po_rID, pi_rPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointDir::RemovePoints    (TileID          pi_ID)
    {
    HPRECONDITION(pi_ID != GetNullTileID());
    return PacketMgr().Remove(pi_ID);
    }





} //End namespace IDTMFile