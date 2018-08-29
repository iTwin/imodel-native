/*--------------------------------------------------------------------------------------+ 
|
|     $Source: PublicAPI/DgnPlatform/NewTileTree.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include <DgnPlatform/RealityDataCache.h>

#define BEGIN_NEWTILETREE_NAMESPACE    BEGIN_BENTLEY_DGN_NAMESPACE namespace NewTileTree {
#define END_NEWTILETREE_NAMESPACE      } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_NEWTILETREE    using namespace BentleyApi::Dgn::NewTileTree;

BEGIN_NEWTILETREE_NAMESPACE

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
enum class TreeType : uint8_t
{
    Model,
    Classifier,
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct ContentId
{
private:
    uint64_t    m_i;
    uint64_t    m_j;
    uint64_t    m_k;
    uint8_t     m_depth;
    uint8_t     m_mult;
public:
    ContentId(uint8_t depth, uint64_t i, uint64_t j, uint64_t k, uint8_t mult) : m_i(i), m_j(j), m_k(k), m_depth(depth), m_mult(0 == mult ? 1 : mult) { }
    ContentId() : ContentId(0, 0, 0, 0, 1) { }

    static ContentId RootId() { return ContentId(); }

    bool operator==(ContentId const& other) const { return m_depth == other.m_depth && m_i == other.m_i && m_j == other.m_j && m_k == other.m_k && m_mult == other.m_mult; }
    bool operator!=(ContentId const& other) const { return !(*this == other); }
    bool operator<(ContentId const& rhs) const
        {
        // memcmp could be faster, or not - let optimizer figure it out. We don't actually care about the ordering, only that it's strict-weak.
        if (m_depth != rhs.m_depth) return m_depth < rhs.m_depth;
        else if (m_i != rhs.m_i) return m_i < rhs.m_i;
        else if (m_j != rhs.m_j) return m_j < rhs.m_j;
        else if (m_k != rhs.m_k) return m_k < rhs.m_k;
        else if (m_mult != rhs.m_mult) return m_mult != rhs.m_mult;
        else return false;
        }

    DGNPLATFORM_EXPORT Utf8String ToString() const;
    DGNPLATFORM_EXPORT bool FromString(Utf8CP str);
};

END_NEWTILETREE_NAMESPACE

