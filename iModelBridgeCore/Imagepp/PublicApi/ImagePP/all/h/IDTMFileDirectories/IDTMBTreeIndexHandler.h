//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/IDTMFileDirectories/IDTMBTreeIndexHandler.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : QuadTreeIndexHandler
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/IDTMFileDirectories/IDTMSpatialIndexDir.h>

#include <Imagepp/all/h/HPUArray.h>
#include <Imagepp/all/h/IDTMTypes.h>
#include <Imagepp/all/h/HTGFFSubDirHelpers.h>

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

    _HDLLg bool                             operator== (const SubNodesTable& pi_rRight) const;

    value_type&                             operator[] (size_t pi_Idx)          { HPRECONDITION(pi_Idx < MAX_QTY); return *(&m_SubTile0 + pi_Idx); }
    const value_type&                       operator[] (size_t pi_Idx) const    { HPRECONDITION(pi_Idx < MAX_QTY); return *(&m_SubTile0 + pi_Idx); }

    // NOTE: Do not use directly. Access this data via [] operator instead. Those are still public so that
    //       this structure can be initialized like a aggregate (via: = {1,2,3,4}). We do not represent
    //       children list with an array in order to provide better debug information in the debugger
    //       when this struct is part of a list.
    value_type m_SubTile0, m_SubTile1, m_SubTile2, m_SubTile3, m_SubTile4, m_SubTile5, m_SubTile6, m_SubTile7;
    };


/*---------------------------------------------------------------------------------**//**
* @description  POD class storing statistics for a node.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct NodeStatistics
    {
    uint64_t                    m_totalPointCount;
    uint64_t                    m_totalLinearFeatureCount;
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
        _HDLLg explicit         Options                            (bool                        pi_isProgressive,
                                                                    size_t                      pi_splitTreshold);

        _HDLLg bool             IsProgressive                      () const;
        _HDLLg size_t           GetSplitTreshold                   () const;

    private:
        virtual bool            _SaveTo                            (SpatialIndexDir&            pi_rIndex) const override;

        bool                    m_isProgressive;
        size_t                  m_splitTreshold;
        };


    _HDLLg virtual              ~BTreeIndexHandler                 ();

    _HDLLg static bool          IsCompatibleWith                   (const SpatialIndexDir&      pi_rpIndexDir); 

    //_HDLLg static CPtr          CreateFrom                         (const SpatialIndexDir::CPtr& pi_rpPointDir);
    _HDLLg static Ptr           CreateFrom                         (SpatialIndexDir*            pi_rpIndexDir);


    /*---------------------------------------------------------------------------------**//**
    * Level of detail pyramid(resolution) accessors
    +---------------+---------------+---------------+---------------+---------------+------*/
    _HDLLg NodeID               GetTopNode                         () const;
    _HDLLg bool                 SetTopNode                         (NodeID                      pi_ID);

    _HDLLg bool                 HasSubNodes                        (NodeID                      pi_ID) const;

    _HDLLg const SubNodesTable& GetSubNodes                        (NodeID                      pi_ID) const;
    _HDLLg SubNodesTable&       EditSubNodes                       (NodeID                      pi_ID);

    _HDLLg bool                 IsBalanced                         () const;
    _HDLLg bool                 SetBalanced                        (bool                        pi_Balanced);

    _HDLLg bool                 IsProgressive                      () const;
    _HDLLg size_t               GetSplitTreshold                   () const;


    _HDLLg const Extent3d64f&   GetContentExtent                   (NodeID                      pi_ID) const;
    _HDLLg Extent3d64f&         EditContentExtent                  (NodeID                      pi_ID);

    /*---------------------------------------------------------------------------------**//**
    * Statistics stored for nodes
    +---------------+---------------+---------------+---------------+---------------+------*/
    _HDLLg uint64_t             GetTotalPointCount                 (NodeID                      pi_ID) const;
    _HDLLg bool                 SetTotalPointCount                 (NodeID                      pi_ID,
                                                                    uint64_t                    pi_count) const;

    _HDLLg uint64_t             GetTotalLinearFeatureCount         (NodeID                      pi_ID) const;
    _HDLLg bool                 SetTotalLinearFeatureCount         (NodeID                      pi_ID,
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
    typedef HTGFF::AttributeSubDir<NodeStatistics>
                                NodesStatisticsDir;

    SpatialIndexDir*            m_pIndexDir;
    ExtentsDir*                 m_pContentExtentsDir;
    SubNodesDir*                m_pSubNodesDir;
    NodesStatisticsDir*         m_pNodesStatsDir;
    };





} //End namespace IDTMFile