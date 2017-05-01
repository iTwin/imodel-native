//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMBTreeIndexHandler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : QuadTreeIndexHandler
//-----------------------------------------------------------------------------

#pragma once

#include <STMInternal/Storage/IDTMFileDirectories/IDTMSpatialIndexDir.h>

#include <STMInternal/Storage/HPUArray.h>
#include <STMInternal/Storage/IDTMTypes.h>
#include <STMInternal/Storage/HTGFFSubDirHelpers.h>

namespace IDTMFile {



#ifdef _WIN32
#pragma pack(push, IDTMFileIdent, 4)
#else
#pragma pack(push, 4)
#endif

/*---------------------------------------------------------------------------------**//**
* @description  Aggregate class used to represent tile children. Possible children
*               quantities are to be 0, 1, 4 or 8. Children are read from left to right
*               (index 0 to index 7) until NO_CHILD_ID or max index is reached. Default
*               value is expected to be all indexes == NO_CHILD_ID;
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct SubNodesTable
    {
    typedef NodeID                          value_type;

    static const size_t                     MAX_QTY         = 8;
    static value_type                       GetNoSubNodeID () { return GetNullNodeID(); }

     bool                             operator== (const SubNodesTable& pi_rRight) const;

    value_type&                             operator[] (size_t pi_Idx)          { HPRECONDITION(pi_Idx < MAX_QTY); return *(&m_SubTile0 + pi_Idx); }
    const value_type&                       operator[] (size_t pi_Idx) const    { HPRECONDITION(pi_Idx < MAX_QTY); return *(&m_SubTile0 + pi_Idx); }

    // NOTE: Do not use directly. Access this data via [] operator instead. Those are still public so that
    //       this structure can be initialized like a aggregate (via: = {1,2,3,4}). We do not represent
    //       children list with an array in order to provide better debug information in the debugger
    //       when this struct is part of a list.
    value_type m_SubTile0, m_SubTile1, m_SubTile2, m_SubTile3, m_SubTile4, m_SubTile5, m_SubTile6, m_SubTile7;
    };



template <class T> struct VariableSizeDataBaseTable
    {
    private : 
        
        uint32_t   m_nbVariableData; 
        PacketID m_variableDataID; 
        //T*           m_variableData;

    protected :

        VariableSizeDataBaseTable()
            {
            m_variableDataID = GetNullPacketID();
            m_nbVariableData = 0;
            //m_variableData = 0;
            }
        
    public : 
        
        //size_t GetVarDataSize() {return sizeof(T);}

        int GetNbVarData() const {return m_nbVariableData;}

        void SetNbVarData(int nbVarData) {m_nbVariableData = nbVarData;}

        const PacketID& GetVarDataPacketID() const {return m_variableDataID;}

        void SetVarDataPacketID(const PacketID& variableDataID) {m_variableDataID = variableDataID;}

        /*
        const T* GetVarDataPtr() {return m_variableData;}

        void     SetVarDataPtr(T* variableData) {m_variableData = variableData;}

        T&                             operator[] (size_t pi_Idx)          { HPRECONDITION(pi_Idx < m_nbVariableData); return m_variableData[pi_Idx]; }
        const T&                       operator[] (size_t pi_Idx) const    { HPRECONDITION(pi_Idx < m_nbVariableData); return m_variableData[pi_Idx]; }
        */
    };

/*---------------------------------------------------------------------------------**//**
* @description  Table containing the neighbor nodes
* @bsiclass                                                  Mathieu.St-Pierre   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct NeighborNodeInfo 
    {
    uint8_t   m_nodePos;     
    NodeID m_nodeId; 
    };

struct NeighborNodesTable : public VariableSizeDataBaseTable<NeighborNodeInfo>
    {
    NeighborNodesTable()
        {        
        }
    
    static const size_t                     MAX_QTY         = 26;
    
    typedef NodeID                          value_type;

    /*
    static value_type                       GetNoSubNodeID () { return GetNullNodeID(); }

     bool                             operator== (const NeighborNodesTable& pi_rRight) const;
    */

    /*
    value_type&                             operator[] (size_t pi_Idx)          { HPRECONDITION(pi_Idx < MAX_QTY); return m_NeighborTiles[pi_Idx]; }
    const value_type&                       operator[] (size_t pi_Idx) const    { HPRECONDITION(pi_Idx < MAX_QTY); return m_NeighborTiles[pi_Idx]; }
    */

    // NOTE: Do not use directly. Access this data via [] operator instead. Those are still public so that
    //       this structure can be initialized like a aggregate (via: = {1,2,3,4}). 
    bool m_AreNeighborTilesStitched[MAX_QTY];    
    };

struct MeshComponentsList : public VariableSizeDataBaseTable<int>
    {
    MeshComponentsList()
        {
       // for (int ind = 0; ind < MAX_QTY; ind++) m_components[ind] = GetNoPointID();
        }
    typedef int                          value_type;

  /*  static const size_t                     MAX_QTY = 10000;
    static value_type                       GetNoPointID() { return -1; }

     bool                             operator== (const MeshComponentsList& pi_rRight) const;

    value_type&                             operator[] (size_t pi_Idx) { HPRECONDITION(pi_Idx < MAX_QTY); return m_components[pi_Idx]; }
    const value_type&                       operator[] (size_t pi_Idx) const { HPRECONDITION(pi_Idx < MAX_QTY); return m_components[pi_Idx]; }

    // NOTE: Do not use directly. Access this data via [] operator instead. Those are still public so that
    //       this structure can be initialized like a aggregate (via: = {1,2,3,4}). We do not represent
    //       children list with an array in order to provide better debug information in the debugger
    //       when this struct is part of a list.
    value_type m_components[MAX_QTY];*/
    //PacketID m_variableDataID;
    };

/*---------------------------------------------------------------------------------**//**
* @description  POD class storing statistics for a node.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct NodeStatistics
    {
    uint64_t                      m_totalPointCount;    
    uint64_t                       m_totalLinearFeatureCount;
    //unsigned short              m_nb3dPointsBins;         
    };

struct NodeMeshStatistics
    {
    uint64_t                       m_meshIndexesCount;
    uint64_t                       m_meshComponentsCount;
    bool                        m_arePoints3d;
    };

struct NodeMeshIDs
    {
    NodeID                      m_meshGraphID;

    NodeMeshIDs()
        {
        m_meshGraphID = GetNullNodeID();
        }
    };

#ifdef _WIN32
#pragma pack(pop, IDTMFileIdent)
#else
#pragma pack(pop)
#endif

/*---------------------------------------------------------------------------------**//**
* @description  Handler that wrap on a spatial index directory to offer quad tree
*               specific accessors.
*
* @see SpatialIndexDir
* @see SpatialIndexHandler
*
* @bsiclass                                                  Raymond.Gauthier   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class BTreeIndexHandler : public HTGFF::SingleDirHandler<BTreeIndexHandler, SpatialIndexDir>
    {
public:
    struct Options : public SpatialIndexDir::Options
        {
         explicit         Options                            (bool                        pi_isProgressive,
                                                                    size_t                      pi_splitTreshold);

         bool             IsProgressive                      () const;
         size_t           GetSplitTreshold                   () const;

    private:
        virtual bool            _SaveTo                            (SpatialIndexDir&            pi_rIndex) const override;

        bool                    m_isProgressive;
        size_t                  m_splitTreshold;
        };


     virtual              ~BTreeIndexHandler                 ();

     static bool          IsCompatibleWith                   (const SpatialIndexDir&      pi_rpIndexDir); 

    // static CPtr          CreateFrom                         (const SpatialIndexDir::CPtr& pi_rpPointDir);
     static Ptr           CreateFrom                         (SpatialIndexDir*            pi_rpIndexDir);


    /*---------------------------------------------------------------------------------**//**
    * Level of detail pyramid(resolution) accessors
    +---------------+---------------+---------------+---------------+---------------+------*/
     NodeID               GetTopNode                         () const;
     bool                 SetTopNode                         (NodeID                      pi_ID);

     bool                 HasSubNodes                        (NodeID                      pi_ID) const;

     const SubNodesTable& GetSubNodes                        (NodeID                      pi_ID) const;
     SubNodesTable&       EditSubNodes                       (NodeID                      pi_ID);
    
     const NeighborNodesTable& GetNeighborNodes              (NodeID                      pi_ID) const;
     NeighborNodesTable&       EditNeighborNodes             (NodeID                      pi_ID);

     const NeighborNodeInfo* GetNeighborNodesVarData (const NeighborNodesTable& neighborNodesTable) const;
     NeighborNodeInfo*       EditNeighborNodesVarData (NeighborNodesTable& neighborNodesTable);

     const NodeID& GetParentNode  (NodeID                      pi_ID) const;
     NodeID&       EditParentNode (NodeID                      pi_ID);

     const MeshComponentsList& GetMeshComponents             (NodeID                      pi_ID) const;
     MeshComponentsList&       EditMeshComponents            (NodeID                      pi_ID);

     const int* GetMeshComponentsVarData(const MeshComponentsList& meshComponents) const;
     int*       EditMeshComponentsVarData(MeshComponentsList& meshComponents, size_t nOfComponents);

     NodeID               GetGraphBlockID(NodeID pi_ID) const;
     NodeID&              EditGraphBlockID(NodeID pi_ID);

     bool                 IsBalanced                         () const;
     bool                 SetBalanced                        (bool                        pi_Balanced);

     bool                 IsProgressive                      () const;
     size_t               GetSplitTreshold                   () const;


     const Extent3d64f&   GetContentExtent                   (NodeID                      pi_ID) const;
     Extent3d64f&         EditContentExtent                  (NodeID                      pi_ID);

    /*---------------------------------------------------------------------------------**//**
    * Statistics stored for nodes
    +---------------+---------------+---------------+---------------+---------------+------*/
     uint64_t             GetTotalPointCount                 (NodeID                      pi_ID) const;
     bool                 SetTotalPointCount                 (NodeID                      pi_ID,
                                                                           uint64_t                       pi_count) const;

     bool                 GetArePoints3d                     (NodeID                      pi_ID) const;
     bool                 SetArePoints3d                     (NodeID                      pi_ID,
                                                                           bool                        pi_arePoints3d) const;

     unsigned short       GetNb3dPointsBins                  (NodeID                      pi_ID) const;
     bool                 SetNb3dPointsBins                  (NodeID                      pi_ID,
                                                                           unsigned short              pi_nb3dPointsBins) const;

     uint64_t                GetMeshIndexesCount                 (NodeID                      pi_ID) const;
     bool                 SetMeshIndexesCount                 (NodeID                      pi_ID,
                                                                            uint64_t                       pi_count) const;    

     uint64_t               GetMeshComponentsCount               (NodeID                      pi_ID) const;
     bool                 SetMeshComponentsCount               (NodeID                      pi_ID,
                                                                    uint64_t                      pi_count) const;
    
     uint64_t               GetTotalLinearFeatureCount         (NodeID                      pi_ID) const;
     bool                 SetTotalLinearFeatureCount         (NodeID                      pi_ID,
                                                                    uint64_t                    pi_count) const;

private:
    friend                      super_class;

    explicit                    BTreeIndexHandler                  (SpatialIndexDir*            pi_pIndexDir);
    
    bool                        SetProgressive                     (bool                        pi_Progressive);
    bool                        SetSplitTreshold                   (size_t                      pi_PointCount);

    virtual bool                _Save                              () override;
    virtual bool                _Load                              () override;
    bool                        Create                             (const Options&              pi_rOptions);

    typedef HTGFF::AttributeSubDir<Extent3d64f> 
                                ExtentsDir;
    typedef HTGFF::AttributeSubDir<SubNodesTable>           
                                SubNodesDir;
    typedef HTGFF::AttributeSubDir<NeighborNodesTable>           
                                NeighborNodesDir;
    typedef HTGFF::VarSizeAttributeSubDir<NeighborNodeInfo>           
                                NeighborNodesVarDataDir;
    typedef HTGFF::AttributeSubDir<NodeStatistics>
                                NodesStatisticsDir;
    typedef HTGFF::AttributeSubDir<NodeMeshStatistics>
        NodesMeshStatisticsDir;
    typedef HTGFF::AttributeSubDir<NodeMeshIDs>
        NodesMeshRelatedBlockIDsDir;
    typedef HTGFF::AttributeSubDir<IDTMFile::NodeID>           
                                ParentNodeDir;
	typedef HTGFF::AttributeSubDir<MeshComponentsList>
                               MeshComponentsDir;
    typedef HTGFF::VarSizeAttributeSubDir<int>
                               MeshComponentsVarDataDir;

    SpatialIndexDir*            m_pIndexDir;
    ExtentsDir*                 m_pContentExtentsDir;
    SubNodesDir*                m_pSubNodesDir;
    NeighborNodesDir*           m_pNeighborNodesDir;
    NeighborNodesVarDataDir*    m_pNeighborNodesVarDataDir;
    NodesStatisticsDir*         m_pNodesStatsDir;   
    NodesMeshStatisticsDir*         m_pNodesMeshStatsDir;
    NodesMeshRelatedBlockIDsDir*         m_pNodeMeshIDsDir;
    ParentNodeDir*              m_pParentNodeDir;
	MeshComponentsDir*          m_pMeshComponentsDir;
    MeshComponentsVarDataDir*   m_pMeshComponentsVarDataDir;


    };





} //End namespace IDTMFile