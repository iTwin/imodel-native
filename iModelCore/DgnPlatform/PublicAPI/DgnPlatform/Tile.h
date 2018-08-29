/*--------------------------------------------------------------------------------------+ 
|
|     $Source: PublicAPI/DgnPlatform/Tile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include <DgnPlatform/RealityDataCache.h>

#define BEGIN_TILE_NAMESPACE    BEGIN_BENTLEY_DGN_NAMESPACE namespace Tile {
#define END_TILE_NAMESPACE      } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_TILE    using namespace BentleyApi::Dgn::Tile;

BEGIN_TILE_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(NodeId);
DEFINE_POINTER_SUFFIX_TYPEDEFS(ContentId);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
enum class TreeType : uint8_t
{
    Model,
    Classifier,
};

//=======================================================================================
//! Uniquely identifies a volume within the oct-tree by its depth and range.
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct NodeId
{
protected:
    uint64_t    m_i;
    uint64_t    m_j;
    uint64_t    m_k;
    uint8_t     m_depth;

    static int8_t Compare(NodeIdCR lhs, NodeIdCR rhs)
        {
        if (lhs.m_depth != rhs.m_depth) return lhs.m_depth < rhs.m_depth ? -1 : 1;
        else if (lhs.m_i != rhs.m_i) return lhs.m_i < rhs.m_i ? -1 : 1;
        else if (lhs.m_j != rhs.m_j) return lhs.m_j < rhs.m_j ? -1 : 1;
        else if (lhs.m_k != rhs.m_k) return lhs.m_k < rhs.m_k ? -1 : 1;
        else return 0;
        }

    void EnforceDepth()
        {
        if (0 == m_depth)
            {
            BeAssert(0 == m_i && 0 == m_j && 0 == m_k);
            m_i = m_j = m_k = 0;
            }
        }
public:
    NodeId(uint8_t depth, uint64_t i, uint64_t j, uint64_t k) : m_i(i), m_j(j), m_k(k), m_depth(depth) { EnforceDepth(); }
    NodeId() : m_i(0), m_j(0), m_k(0), m_depth(0) { }

    static NodeId RootId() { return NodeId(); }
    bool IsRoot() const { return 0 == GetDepth(); }

    uint8_t GetDepth() const { return m_depth; }

    bool operator==(NodeIdCR other) const { return 0 == Compare(*this, other); }
    bool operator!=(NodeIdCR other) const { return !(*this == other); }
    bool operator<(NodeIdCR rhs) const { return Compare(*this, rhs) < 0; }

    DGNPLATFORM_EXPORT NodeId ComputeParentId() const;
    DGNPLATFORM_EXPORT DRange3d ComputeRange(DRange3dCR rootRange, bool is2d) const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct ContentId : NodeId
{
private:
    DEFINE_T_SUPER(NodeId);

    uint8_t     m_mult;
public:
    ContentId(NodeIdCR nodeId, uint8_t mult) : T_Super(nodeId), m_mult(0 == mult ? 1 : mult) { }
    ContentId(uint8_t depth, uint64_t i, uint64_t j, uint64_t k, uint8_t mult) : ContentId(NodeId(depth, i, j, k), mult) { }
    ContentId() : ContentId(0, 0, 0, 0, 1) { }

    bool operator==(ContentIdCR other) const { return T_Super::operator==(other) && m_mult == other.m_mult; }
    bool operator!=(ContentIdCR other) const { return !(*this == other); }
    bool operator<(ContentIdCR rhs) const
        {
        auto compId = Compare(*this, rhs);
        if (0 != compId) return compId < 0;
        else if (m_mult != rhs.m_mult) return m_mult < rhs.m_mult;
        else return false;
        }

    DGNPLATFORM_EXPORT Utf8String ToString() const;
    DGNPLATFORM_EXPORT bool FromString(Utf8CP str);

    double GetSizeMultiplier() const { BeAssert(0 != m_mult); return static_cast<double>(m_mult); }
};

END_TILE_NAMESPACE

