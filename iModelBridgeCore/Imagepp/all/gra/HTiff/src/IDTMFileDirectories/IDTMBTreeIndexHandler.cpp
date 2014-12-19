//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/HTiff/src/IDTMFileDirectories/IDTMBTreeIndexHandler.cpp $
//:>    $RCSfile$
//:>   $Revision$
//:>       $Date$
//:>     $Author$
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <ImagePP/all/h/IDTMFileDirectories/IDTMBTreeIndexHandler.h>

#include <ImagePP/all/h/HTGFFSubDirManager.h>
#include <ImagePP/all/h/HTGFFAttributeManager.h>
#include <ImagePP/all/h/HTGFFPacketManager.h>

#include "IDTMCommonDirTools.h"
#include "../IDTMFileDefinition.h"
#include "ImagePP/all/h/IDTMTypes.h"


#define ARRAY_SIZE(Array) (sizeof(Array)/sizeof(Array[0]))
#define ARRAY_BEGIN(Array) (Array)
#define ARRAY_END(Array) ((Array) + ARRAY_SIZE(Array))


using namespace HTGFF;

namespace IDTMFile {


namespace { // BEGIN unnamed namespace

HSTATICASSERT(8*sizeof(uint32_t) == sizeof(SubNodesTable));


const SubNodesTable DEFAULT_SUB_TILE_TABLE 
    = {SubNodesTable::GetNoSubNodeID (), SubNodesTable::GetNoSubNodeID (), 
       SubNodesTable::GetNoSubNodeID (), SubNodesTable::GetNoSubNodeID (),
       SubNodesTable::GetNoSubNodeID (), SubNodesTable::GetNoSubNodeID (), 
       SubNodesTable::GetNoSubNodeID (), SubNodesTable::GetNoSubNodeID ()};

struct QuadTreeDefault
    {
    explicit                        QuadTreeDefault        ()
        :   subNodesTable(DEFAULT_SUB_TILE_TABLE),
            balanced(false),
            progressive(true),
            splitThreshold((numeric_limits<uint32_t>::max) ()),
            topNode(0),
            nodeStats()
        {
        
        }

    const SubNodesTable             subNodesTable;
    const bool                      balanced;
    const bool                      progressive;
    const size_t                    splitThreshold;
    const NodeID                    topNode;
    const NodeStatistics            nodeStats;

    } QUAD_TREE_DEFAULT;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class SubNodesTypeDef
    {
    enum DimensionRole
        {
        DR_SUB_NODE_0,
        DR_SUB_NODE_1,
        DR_SUB_NODE_2,
        DR_SUB_NODE_3,
        DR_SUB_NODE_4,
        DR_SUB_NODE_5,
        DR_SUB_NODE_6,
        DR_SUB_NODE_7,
        DR_QTY,
        };

public:
    static const DataType&      GetType                ()
        {
        static const Dimension DIMENSIONS[]   =  
            {
            Dimension::CreateFrom(Dimension::TYPE_UINT32,   DR_SUB_NODE_0),
            Dimension::CreateFrom(Dimension::TYPE_UINT32,   DR_SUB_NODE_1),
            Dimension::CreateFrom(Dimension::TYPE_UINT32,   DR_SUB_NODE_2),
            Dimension::CreateFrom(Dimension::TYPE_UINT32,   DR_SUB_NODE_3),
            Dimension::CreateFrom(Dimension::TYPE_UINT32,   DR_SUB_NODE_4),
            Dimension::CreateFrom(Dimension::TYPE_UINT32,   DR_SUB_NODE_5),
            Dimension::CreateFrom(Dimension::TYPE_UINT32,   DR_SUB_NODE_6),
            Dimension::CreateFrom(Dimension::TYPE_UINT32,   DR_SUB_NODE_7),
            };

        static const DataType DATA_TYPE (ARRAY_BEGIN(DIMENSIONS), ARRAY_END(DIMENSIONS));

        return DATA_TYPE;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class NodeStatisticsTypeDef
    {
    enum DimensionRole
        {
        DR_TOTAL_POINT_COUNT,
        DR_TOTAL_LINEAR_FEATURE_COUNT,
        DR_QTY,
        };

public:
    static const DataType&      GetType                ()
        {
        static const Dimension DIMENSIONS[]   =  
            {
            Dimension::CreateFrom(Dimension::TYPE_UINT64,   DR_TOTAL_POINT_COUNT),
            Dimension::CreateFrom(Dimension::TYPE_UINT64,   DR_TOTAL_LINEAR_FEATURE_COUNT),
            };

        static const DataType DATA_TYPE (ARRAY_BEGIN(DIMENSIONS), ARRAY_END(DIMENSIONS));

        return DATA_TYPE;
        }
    };


} // END unnamed namespace 

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubNodesTable::operator== (const SubNodesTable& pi_rRight) const
{
    return equal(&(*this)[0], &(*this)[0] + MAX_QTY, &pi_rRight[0]);
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BTreeIndexHandler::Options::Options (bool            pi_isProgressive,
                                        size_t          pi_splitTreshold)
    :   SpatialIndexDir::Options(SPATIAL_INDEX_TYPE_QUAD_TREE),
        m_isProgressive(pi_isProgressive),
        m_splitTreshold(pi_splitTreshold)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::Options::IsProgressive () const
    {
    return m_isProgressive;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BTreeIndexHandler::Options::GetSplitTreshold () const
    {
    return m_splitTreshold;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::Options::_SaveTo (SpatialIndexDir& pi_rIndex) const
    {
    BTreeIndexHandler Handler(&pi_rIndex);

    return Handler.Create(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::IsCompatibleWith (const SpatialIndexDir& pi_rpIndexDir)
    {
    return SPATIAL_INDEX_TYPE_QUAD_TREE == pi_rpIndexDir.GetType();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BTreeIndexHandler::Ptr BTreeIndexHandler::CreateFrom (SpatialIndexDir* pi_rpIndexDir)
    {
    return CreateFromImpl(pi_rpIndexDir);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BTreeIndexHandler::BTreeIndexHandler (SpatialIndexDir* pi_pIndexDir)
    :   SingleDirHandler(*pi_pIndexDir), 
        m_pIndexDir(pi_pIndexDir),
        m_pContentExtentsDir(0),
        m_pSubNodesDir(0),
        m_pNodesStatsDir(0)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BTreeIndexHandler::~BTreeIndexHandler ()
    {
    const bool Success = Save();
    HASSERT(Success);
    }   

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodeID BTreeIndexHandler::GetTopNode () const
    {
    NodeID TopNodeID;
    if (!m_pIndexDir->AttributeMgr().Get(IDTM_ATTRIBUTEID_SPATIALINDEXDIR_TOPNODEID, TopNodeID))
        {
        HPOSTCONDITION(!"Attribute not found!");
        return GetNullNodeID();
        }

    return TopNodeID;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::SetTopNode (NodeID pi_ID)
    {
    return m_pIndexDir->AttributeMgr().Set<NodeID>(IDTM_ATTRIBUTEID_SPATIALINDEXDIR_TOPNODEID, pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::HasSubNodes (NodeID pi_ID) const
    {
    HPRECONDITION(pi_ID != GetNullNodeID());

    if (SubNodesTable::GetNoSubNodeID() == m_pSubNodesDir->Get(pi_ID)[0])
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const SubNodesTable& BTreeIndexHandler::GetSubNodes (NodeID pi_ID) const
    {
    HPRECONDITION(pi_ID != GetNullNodeID());
    return m_pSubNodesDir->Get(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SubNodesTable& BTreeIndexHandler::EditSubNodes (NodeID pi_ID)
    {
    HPRECONDITION(pi_ID != GetNullNodeID());
    return m_pSubNodesDir->Edit(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::IsBalanced () const
    {
    uint32_t Balanced;
    if (!m_pIndexDir->AttributeMgr().Get(IDTM_ATTRIBUTEID_SPATIALINDEXDIR_BALANCED, Balanced))
        {
        HASSERT(!"Attribute not found");
        return QUAD_TREE_DEFAULT.balanced;
        }
    return 0 == Balanced ? false : true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::SetBalanced (bool pi_Balanced)
    {
    return m_pIndexDir->AttributeMgr().Set<uint32_t>(IDTM_ATTRIBUTEID_SPATIALINDEXDIR_BALANCED, pi_Balanced);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::IsProgressive () const
    {
    uint32_t Progressive;
    if (!m_pIndexDir->AttributeMgr().Get(IDTM_ATTRIBUTEID_SPATIALINDEXDIR_PROGRESSIVE, Progressive))
        {
        HASSERT(!"Attribute not found");
        return QUAD_TREE_DEFAULT.progressive;
        }
    return 0 == Progressive ? false : true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::SetProgressive (bool pi_Progressive)
    {
    return m_pIndexDir->AttributeMgr().Set<uint32_t>(IDTM_ATTRIBUTEID_SPATIALINDEXDIR_PROGRESSIVE, pi_Progressive);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BTreeIndexHandler::GetSplitTreshold () const
    {
    size_t SplitTreshold;
    if (!m_pIndexDir->AttributeMgr().Get(IDTM_ATTRIBUTEID_SPATIALINDEXDIR_SPLIT_TRESHOLD, SplitTreshold))
        {
        HASSERT(!"Attribute not found");
        return QUAD_TREE_DEFAULT.splitThreshold;
        }
    return SplitTreshold;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::SetSplitTreshold (size_t pi_PointCount)
    {
    HPRECONDITION(pi_PointCount <= (numeric_limits<uint32_t>::max)());
    return m_pIndexDir->AttributeMgr().Set(IDTM_ATTRIBUTEID_SPATIALINDEXDIR_SPLIT_TRESHOLD, static_cast<uint32_t>(pi_PointCount));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const Extent3d64f& BTreeIndexHandler::GetContentExtent (NodeID pi_ID) const
    {
    HPRECONDITION(pi_ID != GetNullNodeID());
    return m_pContentExtentsDir->Get(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Extent3d64f& BTreeIndexHandler::EditContentExtent (NodeID pi_ID)
    {
    HPRECONDITION(pi_ID != GetNullNodeID());
    return m_pContentExtentsDir->Edit(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t BTreeIndexHandler::GetTotalPointCount   (NodeID  pi_ID) const
    {
    return m_pNodesStatsDir->Get(pi_ID).m_totalPointCount;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::SetTotalPointCount  (NodeID  pi_ID,
                                                uint64_t pi_count) const
    {
    m_pNodesStatsDir->Edit(pi_ID).m_totalPointCount = pi_count;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t BTreeIndexHandler::GetTotalLinearFeatureCount   (NodeID  pi_ID) const
    {
    return m_pNodesStatsDir->Get(pi_ID).m_totalLinearFeatureCount;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::SetTotalLinearFeatureCount  (NodeID  pi_ID,
                                                        uint64_t pi_count) const
    {
    m_pNodesStatsDir->Edit(pi_ID).m_totalLinearFeatureCount = pi_count;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::_Save ()
    {
    bool Success = true;
    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::_Load ()
    {
    static ExtentsDir::Options EXTENTS_DIR_OPTIONS(IDTM_DEFAULT.extent3d);
    static SubNodesDir::Options SUBNODES_DIR_OPTIONS(QUAD_TREE_DEFAULT.subNodesTable);
    static NodesStatisticsDir::Options NODESSTATS_DIR_OPTIONS(QUAD_TREE_DEFAULT.nodeStats);

    bool Success = true;

    if (0 == m_pContentExtentsDir)
        m_pContentExtentsDir = SubDirMgr<ExtentsDir>(*m_pIndexDir).Get(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_CONTENT_3D_EXTENT_SUBDIR, &EXTENTS_DIR_OPTIONS);
    Success &= 0 != m_pContentExtentsDir;

    if (0 == m_pSubNodesDir)
        m_pSubNodesDir = SubDirMgr<SubNodesDir>(*m_pIndexDir).Get(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_SUBNODES_SUBDIR, &SUBNODES_DIR_OPTIONS);
    Success &= 0 != m_pSubNodesDir;

    if (0 == m_pNodesStatsDir)
        m_pNodesStatsDir = SubDirMgr<NodesStatisticsDir>(*m_pIndexDir).Get(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_STATISTICS_SUBDIR, 
                                                                           &NODESSTATS_DIR_OPTIONS);
    Success &= 0 != m_pNodesStatsDir;

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::Create (const Options& pi_rOptions)
    {
    static ExtentsDir::Options EXTENTS_DIR_OPTIONS(IDTM_DEFAULT.extent3d);
    static SubNodesDir::Options SUBNODES_DIR_OPTIONS(QUAD_TREE_DEFAULT.subNodesTable);
    static NodesStatisticsDir::Options NODESSTATS_DIR_OPTIONS(QUAD_TREE_DEFAULT.nodeStats);

    const CreateConfig ExtentsDirConfig(ExtentTypeDef::GetType(), PacketMgr(*m_pIndexDir).GetCompression());
    const CreateConfig SubNodesDirConfig(SubNodesTypeDef::GetType(), PacketMgr(*m_pIndexDir).GetCompression());
    const CreateConfig NodesStatsDirConfig(NodeStatisticsTypeDef::GetType(), PacketMgr(*m_pIndexDir).GetCompression());

    bool Success = true;


    m_pContentExtentsDir = SubDirMgr<ExtentsDir>(*m_pIndexDir).Create(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_CONTENT_3D_EXTENT_SUBDIR, 
                                                                      ExtentsDirConfig, 
                                                                      &EXTENTS_DIR_OPTIONS);
    Success &= 0 != m_pContentExtentsDir;


    m_pSubNodesDir = SubDirMgr<SubNodesDir>(*m_pIndexDir).Create(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_SUBNODES_SUBDIR, 
                                                                 SubNodesDirConfig, 
                                                                 &SUBNODES_DIR_OPTIONS);
    Success &= 0 != m_pSubNodesDir;


    m_pNodesStatsDir = SubDirMgr<NodesStatisticsDir>(*m_pIndexDir).Create(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_STATISTICS_SUBDIR, 
                                                                          NodesStatsDirConfig,
                                                                          &NODESSTATS_DIR_OPTIONS);
    Success &= 0 != m_pNodesStatsDir;

    // Set default top tile
    Success &= SetTopNode(QUAD_TREE_DEFAULT.topNode);
    Success &= SetBalanced(QUAD_TREE_DEFAULT.balanced);
    Success &= SetProgressive(pi_rOptions.IsProgressive());
    Success &= SetSplitTreshold(pi_rOptions.GetSplitTreshold());

    return Success;
    }




} //End namespace IDTMFile