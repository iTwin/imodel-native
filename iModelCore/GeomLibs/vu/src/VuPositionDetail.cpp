/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE






VuPositionDetail::VuPositionDetail (VuP node, DPoint3dCR xyz, TopologyScope topo)
        {
        m_node = node;
        m_xyz    = xyz;
        m_edgeFraction = DBL_MAX;
        m_topology = topo;
        m_iTag = 0;
        m_dTag = 0;
        }

    //! Construct with edge-specific data.
VuPositionDetail::VuPositionDetail (VuP node, DPoint3dCR xyz, double edgeFraction)
        {
        m_node = node;
        m_xyz    = xyz;
        m_edgeFraction = edgeFraction;
        m_topology = Topo_Edge;
        m_iTag = 0;
        m_dTag = 0;
        }


VuPositionDetail::VuPositionDetail ()
        {
        m_node = NULL;
        m_xyz.Zero ();
        m_edgeFraction = DBL_MAX;
        m_topology = Topo_None;
        m_iTag = 0;
        m_dTag = 0;
        }

    // Construct with given iTag but otherwise defaults
VuPositionDetail::VuPositionDetail (ptrdiff_t iTag)
        {
        m_node = NULL;
        m_xyz.Zero ();
        m_edgeFraction = DBL_MAX;
        m_topology = Topo_None;
        m_iTag = iTag;
        m_dTag = 0.0;
        }


ptrdiff_t VuPositionDetail::GetITag () const        {return m_iTag;}
void VuPositionDetail::SetITag (ptrdiff_t value)  {m_iTag = value;}
void VuPositionDetail::IncrementITag (ptrdiff_t step){m_iTag += step;}


double VuPositionDetail::GetDTag () const        {return m_dTag;}
void VuPositionDetail::SetDTag (double value)  {m_dTag = value;}

//! Construct and return as representative of face.
VuPositionDetail VuPositionDetail::FromFace (VuP node, DPoint3dCR xyz)
    {return VuPositionDetail (node, xyz, Topo_Face);}

//! Construct and return as representative of edge.
VuPositionDetail VuPositionDetail::FromEdge (VuP node, DPoint3dCR xyz, double edgeFraction)
    {return VuPositionDetail (node, xyz, edgeFraction);}

//! Construct and return as representative of vertex.
VuPositionDetail VuPositionDetail::FromVertex (VuP node, DPoint3dCR xyz)
    {return VuPositionDetail (node, xyz, Topo_Vertex);}
//! Construct and return as representative of vertex.
VuPositionDetail VuPositionDetail::FromVertex (VuP node)
    {
    DPoint3d xyz;
    vu_getDPoint3d (&xyz, node);
    return VuPositionDetail (node, xyz, Topo_Vertex);
    }


//! @return true if there is a non-null nodeid.
bool VuPositionDetail::HasNodeId () const { return m_node != NULL;}
//! @return edge fraction.
double VuPositionDetail::GetEdgeFraction () const { return m_edgeFraction;}
//! @return topology type
VuPositionDetail::TopologyScope VuPositionDetail::GetTopologyScope () const {return m_topology;}

//! @return true if topology type is face
bool VuPositionDetail::IsFace () const {return m_topology == Topo_Face;}
//! @return true if topology type is edge
bool VuPositionDetail::IsEdge () const {return m_topology == Topo_Edge;}
//! @return true if topology type is vertex
bool VuPositionDetail::IsVertex () const {return m_topology == Topo_Vertex;}
//! @return true if typology type is null
bool VuPositionDetail::IsUnclassified () const {return m_topology == Topo_None;}
    

//! @return node id
VuP VuPositionDetail::GetNode () const { return m_node;}
//! @return coordinates
DPoint3d VuPositionDetail::GetXYZ () const { return m_xyz;}
//! @return vector to coordinates in the other VuPositionDetail
DVec3d VuPositionDetail::VectorTo (VuPositionDetail const &other) const
    {
    DVec3d vector;
    vector.DifferenceOf (other.m_xyz, m_xyz);
    return vector;
    }

//! @return VuPositionDetail for this instance's edge mate;
//!    The returned VuPositionDetail's edgeFraction is {1 - this->EdgeFraction ())
//!    to properly identify the "same" position relative to the other side.
VuPositionDetail VuPositionDetail::EdgeMate () const
    {
    VuPositionDetail result = *this;
    if (m_node == NULL)
        return result;
    result.m_node = vu_edgeMate (m_node);
    if (m_edgeFraction != DBL_MAX)
        result.m_edgeFraction = 1.0 - m_edgeFraction;
    return result;
    }

//! @return x coordinate
double VuPositionDetail::GetX () const { return m_xyz.x;}
//! @return y coordinate
double VuPositionDetail::GetY () const { return m_xyz.y;}
//! @return z coordinate
double VuPositionDetail::GetZ () const { return m_xyz.z;}

// If candidateKey is less than resultKey, replace resultPos and resultKey
// by the candidate data.
bool VuPositionDetail::UpdateMinimizer
(
VuPositionDetail &resultPos, double &resultKey,
VuPositionDetail const &candidatePos, double candidateKey
)
    {
    if (candidateKey < resultKey)
        {
        resultKey = candidateKey;
        resultPos = candidatePos;
        return true;
        }
    return false;
    }


bool VuPositionDetail::UpdateMin
(
VuPositionDetailR result,
VuPositionDetailCR candidate
)
    {
    if (candidate.m_dTag < result.m_dTag)
        {
        result = candidate;
        return true;
        }
    return false;
    }

bool VuPositionDetail::UpdateMax
(
VuPositionDetailR result,
VuPositionDetailCR candidate
)
    {
    if (candidate.m_dTag > result.m_dTag)
        {
        result = candidate;
        return true;
        }
    return false;
    }
    
//! Move pointer to mate on other side of edge.
//! All other member data unchanged !!
void VuPositionDetail::MoveToEdgeMate () {m_node = vu_edgeMate (m_node);}
//! Move pointer to successor around face.
//! All other member data unchanged !!
void VuPositionDetail::MoveToFSucc (){m_node = vu_fsucc (m_node);}
//! Move pointer to successor around vertex.
//! All other member data unchanged !!
void VuPositionDetail::MoveToVSucc (){m_node = vu_vsucc (m_node);}
//! Move pointer to predecessor around face.
//! All other member data unchanged !!
void VuPositionDetail::MoveToFPred (){m_node = vu_fpred (m_node);}
//! Move pointer to predecessor around vertex.
//! All other member data unchanged !!
void VuPositionDetail::MoveToVPred (){m_node = vu_vpred (m_node);}

END_BENTLEY_GEOMETRY_NAMESPACE