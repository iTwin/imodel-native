//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/IDTMBTreeIndexHandler.cpp $
//:>    $RCSfile$
//:>   $Revision$
//:>       $Date$
//:>     $Author$
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/IDTMBTreeIndexHandler.h>

#include <STMInternal/Storage/HTGFFSubDirManager.h>
#include <STMInternal/Storage/HTGFFAttributeManager.h>
#include <STMInternal/Storage/HTGFFPacketManager.h>

#include "IDTMCommonDirTools.h"
#include "../IDTMFileDefinition.h"
#include <STMInternal/Storage/IDTMTypes.h>


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
            nodeStats(), 
            nodeMeshIDs(),
            parentNode(0),
            meshComponentsVarData()
        {
        
        }

    const SubNodesTable             subNodesTable;
    const bool                      balanced;
    const bool                      progressive;
    const size_t                    splitThreshold;
    const NodeID                    topNode;
    const NodeStatistics            nodeStats;
    const NodeMeshStatistics            nodeMeshStats;
    const NeighborNodesTable        neighborNodesTable;        
    const NeighborNodeInfo          neighborNodesVarDataTable; //NEEDS_WORK_SM : Strange
    const NodeID                    parentNode;
	const MeshComponentsList        meshComponentsList;
    const int                       meshComponentsVarData;

    const NodeMeshIDs               nodeMeshIDs;
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

class NodeMeshStatisticsTypeDef
    {
    enum DimensionRole
        {
        DR_TOTAL_MESH_INDEXES_COUNT,
        DR_MESH_COMPONENTS_COUNT,
        DR_ARE_POINTS_3D,
        DR_PADDING_BYTE,
        DR_PADDING_SHORT,
        DR_QTY,
        };

    public:
        static const DataType&      GetType()
            {
            static const Dimension DIMENSIONS[] =
                {
                Dimension::CreateFrom(Dimension::TYPE_UINT64, DR_TOTAL_MESH_INDEXES_COUNT),
                Dimension::CreateFrom(Dimension::TYPE_UINT64, DR_MESH_COMPONENTS_COUNT),
                Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_ARE_POINTS_3D),
                Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_PADDING_BYTE),
                Dimension::CreateFrom(Dimension::TYPE_UINT16, DR_PADDING_SHORT),
                };

            static const DataType DATA_TYPE(ARRAY_BEGIN(DIMENSIONS), ARRAY_END(DIMENSIONS));

            return DATA_TYPE;
            }
    };

class NodesMeshIDsTypeDef
    {
    enum DimensionRole
        {
        DR_GRAPH_ID,
        DR_QTY,
        };

    public:
        static const DataType&      GetType()
            {
            static const Dimension DIMENSIONS[] =
                {
                Dimension::CreateFrom(Dimension::TYPE_UINT32, DR_GRAPH_ID) //NEEDS_WORK_SM - Shouldn't the node id be 64 bits?
                };

            static const DataType DATA_TYPE(ARRAY_BEGIN(DIMENSIONS), ARRAY_END(DIMENSIONS));

            return DATA_TYPE;
            }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Mathieu.St-Pierre   12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
class NeighborNodesTypeDef
    {
        
    enum DimensionRole
        {
        DR_NEIGHBOR_NODE_NB_VAR_DATA,              
        DR_NEIGHBOR_NODE_VAR_DATA_ID,  
        DR_SUB_NODE_0,
        DR_SUB_NODE_1,
        DR_SUB_NODE_2,
        DR_SUB_NODE_3,
        DR_SUB_NODE_4,
        DR_SUB_NODE_5,
        DR_SUB_NODE_6,
        DR_SUB_NODE_7,
        DR_SUB_NODE_8,
        DR_SUB_NODE_9,
        DR_SUB_NODE_10,
        DR_SUB_NODE_11,
        DR_SUB_NODE_12,
        DR_SUB_NODE_13,
        DR_SUB_NODE_14,
        DR_SUB_NODE_15,
        DR_SUB_NODE_16,
        DR_SUB_NODE_17,
        DR_SUB_NODE_18,
        DR_SUB_NODE_19,
        DR_SUB_NODE_20,
        DR_SUB_NODE_21,
        DR_SUB_NODE_22,
        DR_SUB_NODE_23,
        DR_SUB_NODE_24,
        DR_SUB_NODE_25,        
        DR_STRUCTURE_ALIGNMENT,        
        DR_QTY,
        };

public:
    static const DataType&      GetType                ()
        {
        //NEEDS_WORK_SM - Shouldn't the node id be 64 bits?
        static const Dimension DIMENSIONS[]   =  
            {       
            Dimension::CreateFrom(Dimension::TYPE_UINT32,   DR_NEIGHBOR_NODE_NB_VAR_DATA),           
            Dimension::CreateFrom(Dimension::TYPE_UINT32,   DR_NEIGHBOR_NODE_VAR_DATA_ID),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_0),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_1),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_2),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_3),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_4),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_5),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_6),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_7),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_8),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_9),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_10),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_11),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_12),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_13), 
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_14),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_15),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_16),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_17),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_18),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_19),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_20),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_21),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_22),             
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_23),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_24),           
            Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_SUB_NODE_25),                       
            Dimension::CreateFrom(Dimension::TYPE_UINT16, DR_STRUCTURE_ALIGNMENT),                       
            };
          
        static const DataType DATA_TYPE (ARRAY_BEGIN(DIMENSIONS), ARRAY_END(DIMENSIONS));

        return DATA_TYPE;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Mathieu.St-Pierre   12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
//NEEDS_WORK_SM - Probably not needed.
class NeighborNodesVarDataTypeDef
    {
    enum DimensionRole
        {        
        VAR_DATA_PTR,
        DR_QTY
        };

public:
    static const DataType&      GetType                ()
        {
        //NEEDS_WORK_SM - Shouldn't the node id be 64 bits?
        static const Dimension DIMENSIONS[]   =  
            {       
            Dimension::CreateFrom(Dimension::TYPE_UINT32,   VAR_DATA_PTR),           
            };

        static const DataType DATA_TYPE (ARRAY_BEGIN(DIMENSIONS), ARRAY_END(DIMENSIONS));

        return DATA_TYPE;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Mathieu.St-Pierre   12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
class ParentNodeTypeDef
    {
    enum DimensionRole
        {
        DR_PARENT_NODE,       
        DR_QTY,
        };

public:
    static const DataType&      GetType                ()
        {
        static const Dimension DIMENSIONS[]   =  
            {
            Dimension::CreateFrom(Dimension::TYPE_UINT32,   DR_PARENT_NODE)            
            };

        static const DataType DATA_TYPE (ARRAY_BEGIN(DIMENSIONS), ARRAY_END(DIMENSIONS));

        return DATA_TYPE;
        }
    };

class MeshComponentsTypeDef
    {
    PacketID m_variableDataID;
    enum DimensionRole
        {
        //DR_MIN = (DimensionRole)0,
        //DR_QTY = (DimensionRole) MeshComponentsList::MAX_QTY,
        DR_NB_VAR_DATA,
        DR_ID_FOR_COMPONENTS_LIST,
        DR_QTY
        };

    public:
        static const DataType&      GetType()
            {
           // static Dimension DIMENSIONS[MeshComponentsList::MAX_QTY];
           /* for (size_t i = 0; i < MeshComponentsList::MAX_QTY; i++)
                {
                DIMENSIONS[i] = Dimension::CreateFrom(Dimension::TYPE_UINT32, (DimensionRole)i);
                }*/

            static const Dimension DIMENSIONS[] =
                {
                Dimension::CreateFrom(Dimension::TYPE_UINT32, DR_NB_VAR_DATA),
                Dimension::CreateFrom(Dimension::TYPE_UINT32, DR_ID_FOR_COMPONENTS_LIST)
                };

            static const DataType DATA_TYPE(ARRAY_BEGIN(DIMENSIONS), ARRAY_END(DIMENSIONS));

            return DATA_TYPE;
            }
    };

class MeshComponentsVarDataTypeDef
    {
    enum DimensionRole
        {
        VAR_DATA_PTR,
        DR_QTY
        };

    public:
        static const DataType&      GetType()
            {
            //NEEDS_WORK_SM - Shouldn't the node id be 64 bits?
            static const Dimension DIMENSIONS[] =
                {
                Dimension::CreateFrom(Dimension::TYPE_UINT32, VAR_DATA_PTR),
                };

            static const DataType DATA_TYPE(ARRAY_BEGIN(DIMENSIONS), ARRAY_END(DIMENSIONS));

            return DATA_TYPE;
            }
    };

} // END unnamed namespace 

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*bool MeshComponentsList::operator== (const MeshComponentsList& pi_rRight) const
{
    return equal(&(*this)[0], &(*this)[0] + MAX_QTY, &pi_rRight[0]);
}*/

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*NEEDS_WORK_SM : Needed?
bool NeighborNodesTable::operator== (const NeighborNodesTable& pi_rRight) const
{
    return equal(&(*this)[0], &(*this)[0] + MAX_QTY, &pi_rRight[0]);
}
*/

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
        m_pNodesStatsDir(0),
        m_pNodesMeshStatsDir(0),
        m_pNodeMeshIDsDir(0),
        m_pNeighborNodesDir(0),   
        m_pNeighborNodesVarDataDir(0), 
        m_pParentNodeDir(0),
        m_pMeshComponentsDir(0),
        m_pMeshComponentsVarDataDir(0)
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
const NeighborNodesTable& BTreeIndexHandler::GetNeighborNodes (NodeID pi_ID) const
    {
    HPRECONDITION(pi_ID != GetNullNodeID());
    return m_pNeighborNodesDir->Get(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NeighborNodesTable& BTreeIndexHandler::EditNeighborNodes (NodeID pi_ID)
    {
    HPRECONDITION(pi_ID != GetNullNodeID());
    return m_pNeighborNodesDir->Edit(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const NeighborNodeInfo* BTreeIndexHandler::GetNeighborNodesVarData (const NeighborNodesTable& neighborNodesTable) const
    {   
    HPRECONDITION(neighborNodesTable.GetVarDataPacketID() != GetNullPacketID());            
    return m_pNeighborNodesVarDataDir->Get(neighborNodesTable.GetVarDataPacketID());
    }
       
/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
NeighborNodeInfo* BTreeIndexHandler::EditNeighborNodesVarData (NeighborNodesTable& neighborNodesTable)
    {                   
    PacketID packetId = neighborNodesTable.GetVarDataPacketID();
       
    NeighborNodeInfo* varData = m_pNeighborNodesVarDataDir->Edit(packetId, neighborNodesTable.GetNbVarData());    

    assert(varData != 0 || packetId == GetNullPacketID());
       
    neighborNodesTable.SetVarDataPacketID(packetId);
    
    return varData;     
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const NodeID& BTreeIndexHandler::GetParentNode (NodeID pi_ID) const
    {
    HPRECONDITION(pi_ID != GetNullNodeID());
    return m_pParentNodeDir->Get(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
NodeID& BTreeIndexHandler::EditParentNode (NodeID pi_ID)
    {
    HPRECONDITION(pi_ID != GetNullNodeID());
    return m_pParentNodeDir->Edit(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const MeshComponentsList& BTreeIndexHandler::GetMeshComponents (NodeID pi_ID) const
    {
    HPRECONDITION(pi_ID != GetNullNodeID());
    return m_pMeshComponentsDir->Get(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MeshComponentsList& BTreeIndexHandler::EditMeshComponents(NodeID pi_ID)
    {
    HPRECONDITION(pi_ID != GetNullNodeID());
    return m_pMeshComponentsDir->Edit(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const int* BTreeIndexHandler::GetMeshComponentsVarData(const MeshComponentsList& meshComponents) const
    {   
    HPRECONDITION(meshComponents.GetVarDataPacketID() != GetNullPacketID());
    return m_pMeshComponentsVarDataDir->Get(meshComponents.GetVarDataPacketID());
    }
       
/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int* BTreeIndexHandler::EditMeshComponentsVarData( MeshComponentsList& meshComponents, size_t nOfComponents)
    {                   
    PacketID packetId = meshComponents.GetVarDataPacketID();
       
    int* varData = m_pMeshComponentsVarDataDir->Edit(packetId, nOfComponents);

    assert(varData != NULL || packetId == GetNullPacketID());
       
    meshComponents.SetVarDataPacketID(packetId);
    
    return varData;     
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
    uint32_t SplitTreshold; //BEIJING_WIP_STM: All SplitTreshold ref should be change to UInt32 instead of size_t. Review use of size_t in pointIndex and cie...
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
* @bsimethod                                                  Mathieu.St-Pierre   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::GetArePoints3d   (NodeID  pi_ID) const
    {
    return m_pNodesMeshStatsDir->Get(pi_ID).m_arePoints3d;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::SetArePoints3d  (NodeID  pi_ID,
                                         bool    pi_arePoints3d) const
    {
    m_pNodesMeshStatsDir->Edit(pi_ID).m_arePoints3d = pi_arePoints3d;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned short BTreeIndexHandler::GetNb3dPointsBins   (NodeID  pi_ID) const
    {
  //  return m_pNodesStatsDir->Get(pi_ID).m_nb3dPointsBins;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::SetNb3dPointsBins  (NodeID         pi_ID,
                                            unsigned short pi_nb3dPointsBins) const
    {
    //m_pNodesStatsDir->Edit(pi_ID).m_nb3dPointsBins = pi_nb3dPointsBins;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t BTreeIndexHandler::GetMeshIndexesCount   (NodeID  pi_ID) const
    {
    return m_pNodesMeshStatsDir->Get(pi_ID).m_meshIndexesCount;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::SetMeshIndexesCount  (NodeID  pi_ID,
                                                uint64_t pi_count) const
    {
    m_pNodesMeshStatsDir->Edit(pi_ID).m_meshIndexesCount = pi_count;
    return true;
    }
 

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Elenie.Godzaridis   12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t BTreeIndexHandler::GetMeshComponentsCount   (NodeID  pi_ID) const
    {
    return m_pNodesMeshStatsDir->Get(pi_ID).m_meshComponentsCount;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Elenie.Godzaridis   12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool BTreeIndexHandler::SetMeshComponentsCount(NodeID  pi_ID,
                                                uint64_t pi_count) const
    {
    m_pNodesMeshStatsDir->Edit(pi_ID).m_meshComponentsCount = pi_count;
    return true;
    } 

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Elenie.Godzaridis   03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NodeID BTreeIndexHandler::GetGraphBlockID(NodeID pi_ID) const
    {
    return m_pNodeMeshIDsDir->Get(pi_ID).m_meshGraphID;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Elenie.Godzaridis   03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NodeID& BTreeIndexHandler::EditGraphBlockID(NodeID pi_ID)
    {
    return m_pNodeMeshIDsDir->Edit(pi_ID).m_meshGraphID;
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
    static NodesMeshStatisticsDir::Options NODESMESHSTATS_DIR_OPTIONS(QUAD_TREE_DEFAULT.nodeMeshStats);
    static NodesMeshRelatedBlockIDsDir::Options NODESMESH_DIR_OPTIONS(QUAD_TREE_DEFAULT.nodeMeshIDs);
    static NeighborNodesDir::Options NEIGHBORNODES_DIR_OPTIONS(QUAD_TREE_DEFAULT.neighborNodesTable);
    static NeighborNodesVarDataDir::Options NEIGHBORNODES_VAR_DATA_DIR_OPTIONS(QUAD_TREE_DEFAULT.neighborNodesVarDataTable);
    static ParentNodeDir::Options PARENTNODE_DIR_OPTIONS(QUAD_TREE_DEFAULT.parentNode);    
	static MeshComponentsDir::Options MESHCOMPONENTS_DIR_OPTIONS(QUAD_TREE_DEFAULT.meshComponentsList);
    static MeshComponentsVarDataDir::Options MESHCOMPONENTS_VAR_DATA_DIR_OPTIONS(QUAD_TREE_DEFAULT.meshComponentsVarData);

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

    if (0 == m_pNodeMeshIDsDir)
        m_pNodeMeshIDsDir = SubDirMgr<NodesMeshRelatedBlockIDsDir>(*m_pIndexDir).Get(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESH_IDS_SUBDIR,
                                                                                    &NODESMESH_DIR_OPTIONS);
    Success &= 0 != m_pNodeMeshIDsDir;

    if (0 == m_pNeighborNodesDir)
        m_pNeighborNodesDir = SubDirMgr<NeighborNodesDir>(*m_pIndexDir).Get(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_NEIGHBORNODES_SUBDIR, &NEIGHBORNODES_DIR_OPTIONS);
    Success &= 0 != m_pNeighborNodesDir;

    if (0 == m_pNeighborNodesVarDataDir)
        m_pNeighborNodesVarDataDir = SubDirMgr<NeighborNodesVarDataDir>(*m_pIndexDir).Get(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_NEIGHBORNODES_VARDATA_SUBDIR, &NEIGHBORNODES_VAR_DATA_DIR_OPTIONS);
    Success &= 0 != m_pNeighborNodesVarDataDir;

    if (0 == m_pParentNodeDir)
        m_pParentNodeDir = SubDirMgr<ParentNodeDir>(*m_pIndexDir).Get(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_PARENTNODE_SUBDIR, &PARENTNODE_DIR_OPTIONS);
    Success &= 0 != m_pParentNodeDir;
		
	if (0 == m_pMeshComponentsDir)
        m_pMeshComponentsDir = SubDirMgr<MeshComponentsDir>(*m_pIndexDir).Get(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESHCOMPONENTS_SUBDIR, &MESHCOMPONENTS_DIR_OPTIONS);
    Success &= 0 != m_pMeshComponentsDir; 

    if (0 == m_pMeshComponentsVarDataDir)
        m_pMeshComponentsVarDataDir = SubDirMgr<MeshComponentsVarDataDir>(*m_pIndexDir).Get(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESHCOMPONENTS_VARDATA_SUBDIR, &MESHCOMPONENTS_VAR_DATA_DIR_OPTIONS);
    Success &= 0 != m_pMeshComponentsVarDataDir;

    if (0 == m_pNodesMeshStatsDir)
        m_pNodesMeshStatsDir = SubDirMgr<NodesMeshStatisticsDir>(*m_pIndexDir).Get(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESH_STATISTICS_SUBDIR,
        &NODESMESHSTATS_DIR_OPTIONS);
    Success &= 0 != m_pNodesMeshStatsDir;

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
    static NodesMeshStatisticsDir::Options NODESMESHSTATS_DIR_OPTIONS(QUAD_TREE_DEFAULT.nodeMeshStats);
    static NodesMeshRelatedBlockIDsDir::Options NODESMESH_DIR_OPTIONS(QUAD_TREE_DEFAULT.nodeMeshIDs);
    static NeighborNodesDir::Options NEIGHBORNODES_DIR_OPTIONS(QUAD_TREE_DEFAULT.neighborNodesTable);
    static NeighborNodesVarDataDir::Options NEIGHBORNODES_VAR_DATA_DIR_OPTIONS(QUAD_TREE_DEFAULT.neighborNodesVarDataTable);
    static ParentNodeDir::Options PARENTNODE_DIR_OPTIONS(QUAD_TREE_DEFAULT.parentNode);
	static MeshComponentsDir::Options MESHCOMPONENTS_DIR_OPTIONS(QUAD_TREE_DEFAULT.meshComponentsList);
    static MeshComponentsVarDataDir::Options MESHCOMPONENTS_VAR_DATA_DIR_OPTIONS(QUAD_TREE_DEFAULT.meshComponentsVarData);
    

    const CreateConfig ExtentsDirConfig(ExtentTypeDef::GetType(), PacketMgr(*m_pIndexDir).GetCompression());
    const CreateConfig SubNodesDirConfig(SubNodesTypeDef::GetType(), PacketMgr(*m_pIndexDir).GetCompression());
    const CreateConfig NodesStatsDirConfig(NodeStatisticsTypeDef::GetType(), PacketMgr(*m_pIndexDir).GetCompression());
    const CreateConfig NodesMeshStatsDirConfig(NodeMeshStatisticsTypeDef::GetType(), PacketMgr(*m_pIndexDir).GetCompression());
    const CreateConfig NodesMeshIDsDirConfig(NodesMeshIDsTypeDef::GetType(), PacketMgr(*m_pIndexDir).GetCompression());
    const CreateConfig NeighborNodesDirConfig(NeighborNodesTypeDef::GetType(), PacketMgr(*m_pIndexDir).GetCompression());
    const CreateConfig NeighborNodesVarDataDirConfig(NeighborNodesVarDataTypeDef::GetType(), PacketMgr(*m_pIndexDir).GetCompression());
    const CreateConfig ParentNodeDirConfig(ParentNodeTypeDef::GetType(), PacketMgr(*m_pIndexDir).GetCompression());
	const CreateConfig MeshComponentsDirConfig(MeshComponentsTypeDef::GetType(), PacketMgr(*m_pIndexDir).GetCompression());
    const CreateConfig MeshComponentsVarDataDirConfig(MeshComponentsVarDataTypeDef::GetType(), PacketMgr(*m_pIndexDir).GetCompression());

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

    m_pNodeMeshIDsDir = SubDirMgr<NodesMeshRelatedBlockIDsDir>(*m_pIndexDir).Create(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESH_IDS_SUBDIR,
                                                                          NodesMeshIDsDirConfig,
                                                                          &NODESMESH_DIR_OPTIONS);
    Success &= 0 != m_pNodeMeshIDsDir;


    m_pNeighborNodesDir = SubDirMgr<NeighborNodesDir>(*m_pIndexDir).Create(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_NEIGHBORNODES_SUBDIR, 
                                                                             NeighborNodesDirConfig, 
                                                                             &NEIGHBORNODES_DIR_OPTIONS);
    Success &= 0 != m_pNeighborNodesDir;

    m_pNeighborNodesVarDataDir = SubDirMgr<NeighborNodesVarDataDir>(*m_pIndexDir).Create(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_NEIGHBORNODES_VARDATA_SUBDIR, 
                                                                                         NeighborNodesVarDataDirConfig, 
                                                                                         &NEIGHBORNODES_VAR_DATA_DIR_OPTIONS);
    Success &= 0 != m_pNeighborNodesVarDataDir;


    m_pParentNodeDir = SubDirMgr<ParentNodeDir>(*m_pIndexDir).Create(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_PARENTNODE_SUBDIR, 
                                                                     ParentNodeDirConfig, 
                                                                     &PARENTNODE_DIR_OPTIONS);
    Success &= 0 != m_pParentNodeDir;

    m_pMeshComponentsDir = SubDirMgr<MeshComponentsDir>(*m_pIndexDir).Create(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESHCOMPONENTS_SUBDIR,
                                                                          MeshComponentsDirConfig,
                                                                          &MESHCOMPONENTS_DIR_OPTIONS);
    Success &= 0 != m_pMeshComponentsDir;

    m_pMeshComponentsVarDataDir = SubDirMgr<MeshComponentsVarDataDir>(*m_pIndexDir).Create(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESHCOMPONENTS_VARDATA_SUBDIR,
                                                                                           MeshComponentsVarDataDirConfig,
                                                                                         &MESHCOMPONENTS_VAR_DATA_DIR_OPTIONS);
    Success &= 0 != m_pMeshComponentsVarDataDir;

    m_pNodesMeshStatsDir = SubDirMgr<NodesMeshStatisticsDir>(*m_pIndexDir).Create(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESH_STATISTICS_SUBDIR,
                                                                          NodesMeshStatsDirConfig,
                                                                          &NODESMESHSTATS_DIR_OPTIONS);
    Success &= 0 != m_pNodesMeshStatsDir;

    // Set default top tile
    Success &= SetTopNode(QUAD_TREE_DEFAULT.topNode);
    Success &= SetBalanced(QUAD_TREE_DEFAULT.balanced);
    Success &= SetProgressive(pi_rOptions.IsProgressive());
    Success &= SetSplitTreshold(pi_rOptions.GetSplitTreshold());

    return Success;
    }




} //End namespace IDTMFile