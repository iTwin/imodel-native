/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

typedef RefCountedPtr<struct GeometryNode> GeometryNodePtr;

//! A geometry node is a node in a tree or dag.
//! The node contents are
//!<ul>
//!<li>A vector of IGeometryPtr
//!<li>A vector of GeometryNodePtr
//!<li>A transform
//!</ul>
//! GeometryNode's are expected to be assembled in a tree or DAG structure.
//! With the transforms composed during a traversal, this acts as a "scene graph" or "articulated mechanism".
struct GeometryNode : RefCountedBase
{
struct MemberWithTransform
{
private:
friend GeometryNode;
GeometryNodePtr m_member;
Transform       m_transform;
public:
MemberWithTransform (GeometryNodePtr node, TransformCR transform) : m_member (node), m_transform (transform){}
GeometryNodePtr GetMember (){return  m_member;}
Transform GetTransform (){return m_transform;}
void SetMember (GeometryNodePtr const &value){m_member = value;}
void SetTransform (TransformCR value){m_transform = value;}
};


private:
GeometryNode ();
bvector<IGeometryPtr> m_geometry;
bvector<GeometryNode::MemberWithTransform> m_members;

public:

//! Create with identity transform.
static GEOMDLLIMPEXP GeometryNodePtr Create ();

//! Add a child member with placement transform.
GEOMDLLIMPEXP void AddMember (GeometryNodePtr const &child, TransformCR transform);
//! Add geometry to this node.
GEOMDLLIMPEXP void AddGeometry (IGeometryPtr const &geometry);

//! clear the geometry array
GEOMDLLIMPEXP void ClearGeometry ();
//! clear all child nodes.
GEOMDLLIMPEXP void ClearMembers ();
//! Return a reference to the array of IGeometryPtr
GEOMDLLIMPEXP bvector<IGeometryPtr> &Geometry ();
//! Reutrn a reference to the array of GeometryNodePtr
GEOMDLLIMPEXP bvector<MemberWithTransform> &Members ();


//! Clone all geoemtry (recursively) with transforms applied.
GEOMDLLIMPEXP GeometryNodePtr Flatten () const;
//! Recursively get range with transforms applied.
GEOMDLLIMPEXP bool TryGetRange (DRange3dR range) const;
//! Recursively get range with transforms applied, with given transform to start.
GEOMDLLIMPEXP bool TryGetRange (DRange3dR range, TransformCR transform) const;
//! Transform geometry members directly.  Compose transform with each Member.
GEOMDLLIMPEXP bool TryTransformInPlace (TransformCR transform);
//! Recursively apply IsSameStructureAndGeometry test.
GEOMDLLIMPEXP bool IsSameStructureAndGeometry (GeometryNodeCR other, double tolerance = 0.0) const;
//! Recursively clone.
GEOMDLLIMPEXP GeometryNodePtr Clone () const;
//! Recursively clone.  Apply transform (a) to geometry directly contained in the root and (b) The placement transform
//!   for children.
GEOMDLLIMPEXP GeometryNodePtr Clone (TransformCR transform) const;

//! collect (transformed clone of) geometry.  Optionally recurse to chid nodes.
GEOMDLLIMPEXP void AppendTransformedGeometry (bvector<IGeometryPtr> &geometry, bool recurse = true) const;

//! collect (clones) of geometry, transformed by the parentTransform*nodeTransform.  Optionally recurse to chid nodes.
GEOMDLLIMPEXP void AppendTransformedGeometry (bvector<IGeometryPtr> &geometry, TransformCR parentTransform, bool recurse = true) const;

};
END_BENTLEY_GEOMETRY_NAMESPACE

