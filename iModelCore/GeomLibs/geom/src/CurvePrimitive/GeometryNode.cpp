/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

GeometryNode::GeometryNode () {}

//! Return a reference to the array of IGeometryPtr
bvector<IGeometryPtr> &GeometryNode::Geometry () {return m_geometry;}
//! Reutrn a reference to the array of GeometryNodePtr
bvector<GeometryNode::MemberWithTransform> &GeometryNode::Members () {return m_members;}

//! Create with identity transform.
GeometryNodePtr GeometryNode::Create (){return new GeometryNode ();}

void GeometryNode::AddMember (GeometryNodePtr const &geometry, TransformCR transform)
    {
    m_members.push_back (MemberWithTransform (geometry, transform));
    }

void GeometryNode::AddGeometry (IGeometryPtr const &geometry)
    {
    m_geometry.push_back (geometry);
    }

void GeometryNode::ClearGeometry (){ m_geometry.clear ();}
void GeometryNode::ClearMembers () { m_members .clear ();}

//! append (transformed clone of) geometry.  Optionally recurse to chid nodes.
void GeometryNode::AppendTransformedGeometry (bvector<IGeometryPtr> &geometry, bool recurse) const
    {
    for (auto & g : m_geometry)
        {
        geometry.push_back (g->Clone ());
        }
    if (recurse)
        {
        for (auto &member: m_members)
            {
            member.m_member->AppendTransformedGeometry (geometry, member.m_transform, recurse);
            }
        }
    }

//! append (transformed clone of) geometry.  Optionally recurse to chid nodes.
void GeometryNode::AppendTransformedGeometry (bvector<IGeometryPtr> &geometry, TransformCR transformA, bool recurse) const
    {
    for (auto & g : m_geometry)
        {
        auto g1 = g->Clone (transformA);
        if (g1.IsValid ())
            geometry.push_back (g1);
        }
    if (recurse)
        {
        for (auto &member: m_members)
            {
            Transform transformB = transformA * member.m_transform;
            member.m_member->AppendTransformedGeometry (geometry, transformB, recurse);
            }
        }
    }

GeometryNodePtr GeometryNode::Flatten () const
    {
    auto flat = GeometryNode::Create ();
    AppendTransformedGeometry (flat->Geometry (), true);
    return flat;
    }

bool GeometryNode::TryGetRange (DRange3dR range) const
    {
    range.Init ();
    DRange3d range1;
    for (auto &g1 : m_geometry)
        {
        if (g1->TryGetRange (range1))
            range.Extend (range1);
        }

    for (auto &member : m_members)
        {
        if (member.m_member->TryGetRange (range1, member.m_transform))
            range.Extend (range1);
        }
    return !range.IsNull ();
    }

bool GeometryNode::TryGetRange (DRange3dR range, TransformCR transformA) const
    {
    range.Init ();
    DRange3d range1;
    for (auto &g1 : m_geometry)
        {
        if (g1->TryGetRange (range1, transformA))
            range.Extend (range1);
        }

    for (auto &member : m_members)
        {
        Transform transformB = transformA * member.m_transform;
        if (member.m_member->TryGetRange (range1, transformB))
            range.Extend (range1);
        }
    return !range.IsNull ();
    }

bool GeometryNode::TryTransformInPlace (TransformCR transform)
    {
    bool ok = true;
    for (auto &g1 : m_geometry)
        {
        ok = ok & g1->TryTransformInPlace (transform);
        }

    for (auto &member : m_members)
        {
        member.m_transform = transform * member.m_transform;
        }
    return ok;
    }

bool GeometryNode::IsSameStructureAndGeometry (GeometryNodeCR other, double tolerance) const
    {
    if (other.m_geometry.size () != m_geometry.size ())
        return false;
    if (other.m_members.size () != m_members.size ())
        return false;

    for (size_t i = 0; i < m_geometry.size (); i++)
        {
        if (!m_geometry[i]->IsSameStructureAndGeometry (*other.m_geometry[i], tolerance))
            return false;
        }

    for (size_t i = 0; i < m_members.size (); i++)
        {
        if (!m_members[i].m_transform.IsEqual (other.m_members[i].m_transform, Angle::SmallAngle (), tolerance))
            return false;
        if (!m_members[i].m_member->IsSameStructureAndGeometry (*other.m_members[i].m_member, tolerance))
            return false;
        }
    return true;
    }

GeometryNodePtr GeometryNode::Clone () const
    {
    auto clone = Create ();
    for (auto &g1 : m_geometry)
        clone->m_geometry.push_back (g1->Clone ());

    for (auto &member : m_members)
        clone->m_members.push_back (MemberWithTransform (member.m_member->Clone (), member.m_transform));
    return clone;
    }

GeometryNodePtr GeometryNode::Clone (TransformCR transform) const
    {
    auto clone = Clone ();
    clone->TryTransformInPlace (transform);
    return clone;
    }




END_BENTLEY_GEOMETRY_NAMESPACE
