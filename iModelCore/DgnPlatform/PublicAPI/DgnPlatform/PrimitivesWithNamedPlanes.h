/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/PrimitivesWithNamedPlanes.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_DGN_NAMESPACE

struct DGVariable;

/*=================================================================================**//**
* Arc-like plane-oriented Primitive.

DOF
ZO          3   (2 rotational and 1 translational) 2 rotations to establish the orientation of the wall's base plane, plus translation of the wall along its base plane normal
YO          3   (1 rotational and 2 translational) 1 rotation about the wall's normal, plus translation in the wall's local x-y plane
s           1   Translation of the start point along the arc. Corresponds 1:1 with start angle.
e           1   Translation of the end point along the arc. Corresponds 1:1 with the sweep angle.
r           1   radius
d           1   direction of sweep

Planes
ZO          "z origin" plane is the base of the wall. Its normal points up.
YO          Tangent plane anywhere along the sweep of the arc.
C           A plane with no fixed normal whose origin is the arc center.
S           "start origin" plane is the end cap of the start of the wall. Its normal is tangent to the arc.
E           "end origin" plane is the end cap of the end of the wall. Its normal is tangent to the arc.

Plane ->    DOF
ZO          ZO
YO          YO
C           YO
S           s
E           e

* @bsiclass                                     Sam.Wilson                      07/2010
+===============+===============+===============+===============+===============+======*/
struct ArcPrimitiveWithNamedPlanes
    {
protected:
    DEllipse3d  m_arc;
    double      m_height;

    StatusInt SetXyPlaneNormal (bool& changed, DVec3dCR newVec);
    static void GetVariablesForCenter (bvector<bpair<DGVariable*,bool> >& nodes, DependencyGraph& graph, DgnElementP ref, bool);

public:
    ArcPrimitiveWithNamedPlanes () {}

    ArcPrimitiveWithNamedPlanes (DEllipse3dCR a, double h) : m_arc(a), m_height(h) {}

    //  ------------------------------------------------------------------
    //  Load/Store
    //  ------------------------------------------------------------------
    template<typename XAITERTYPE>
    ArcPrimitiveWithNamedPlanes (XAITERTYPE const& xa)
        {
        DataInternalizer source ((Byte*)xa.PeekData(), xa.GetSize());
        Load (source);
        }

    DGNPLATFORM_EXPORT void Load (DataInternalizer& source);
    DGNPLATFORM_EXPORT void Store (DataExternalizer& sink);

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT DVec3d GetNormal () const;

    //  ------------------------------------------------------------------
    //  Transform
    //  ------------------------------------------------------------------
    DGNPLATFORM_EXPORT void DoTransform (Transform const&);

    //  ------------------------------------------------------------------
    //  Named Planes
    //  ------------------------------------------------------------------
    DGNPLATFORM_EXPORT wchar_t const*  FindNamedPlaneFromPoint (DPoint3dCR ptIn) const;
    DGNPLATFORM_EXPORT DPlane3d         GetNamedPlane (wchar_t const*, DPlane3dCP, DPoint3dCP) const;
    DGNPLATFORM_EXPORT StatusInt        MakeNamedPlaneCoincident (bool& reallyChanged, WCharCP planeName, DPlane3dCR targetPlane,                         DependencyGraph&, DgnElementP);
    DGNPLATFORM_EXPORT StatusInt        MoveNamedPlaneToContact  (bool& reallyChanged, WCharCP planeName, DPlane3dCR targetPlane, ICurvePrimitiveCR tc,   DependencyGraph&, DgnElementP);
    DGNPLATFORM_EXPORT StatusInt        MakeTangentAtNamedPlane  (bool& reallyChanged, WCharCP planeName, DPlane3dCR perpPlane, DPlane3dCR tanPlane,      DependencyGraph&, DgnElementP);
    DGNPLATFORM_EXPORT StatusInt        SetOffsetOfNamedPlane    (bool& reallyChanged, WCharCP planeName, double offset,                                  DependencyGraph& graph, DgnElementP);
    DGNPLATFORM_EXPORT StatusInt        GetOffsetOfNamedPlane    (double& offset, WCharCP planeName) const;
    DGNPLATFORM_EXPORT ICurvePrimitivePtr GetCurveForMoveNamedPlaneToContact () const;
    DGNPLATFORM_EXPORT StatusInt        ComputeOffsetFromNamedPlaneAlongCurve (double& offset, DPoint3dCR pointAtOffset, WCharCP planeName) const;
    DGNPLATFORM_EXPORT StatusInt        ComputePointAtOffsetFromNamedPlaneAlongCurve (DRay3dR pointAtOffset, double offset, WCharCP planeName) const;

    DGNPLATFORM_EXPORT static StatusInt GetVariablesForMakeNamedPlaneCoincident (bvector<bpair<DGVariable*,bool> >& nodes, DependencyGraph& graph, DgnElementP ref, WCharCP planeName);
    DGNPLATFORM_EXPORT static StatusInt GetVariablesForMoveNamedPlaneToContact  (bvector<bpair<DGVariable*,bool> >& nodes, DependencyGraph& graph, DgnElementP ref, WCharCP planeName);
    DGNPLATFORM_EXPORT static StatusInt GetVariablesForMakeTangentAtNamedPlane  (bvector<bpair<DGVariable*,bool> >& nodes, DependencyGraph& graph, DgnElementP ref, WCharCP planeName);
    DGNPLATFORM_EXPORT static StatusInt GetVariablesForComputePointAtOffsetFromNamedPlaneAlongCurve (bvector<bpair<DGVariable*,bool> >& nodes, DependencyGraph& graph, DgnElementP ref, WCharCP planeName);
    DGNPLATFORM_EXPORT static StatusInt GetVariablesForSetOffset                (bvector<bpair<DGVariable*,bool> >& nodes, DependencyGraph& graph, DgnElementP ref, WCharCP planeName);

#if defined (CDOLLAR_EXPERIMENT)
/*=================================================================================**//**
* @bsiclass                                     Sam.Wilson                      12/2010
+===============+===============+===============+===============+===============+======*/
struct          ComputeXYConstraint : DGConstraint
    {
protected:
    DGNPLATFORM_EXPORT virtual WString _GetDescription (DependencyGraph const& g, bool wantDetails) const override;
    DGNPLATFORM_EXPORT virtual void    _WriteConstraintFailure (DependencyGraph& g, DGConstraintFailure const&) override;
    DGNPLATFORM_EXPORT virtual void    _DeleteConstraintFailure (DependencyGraph& g) override;

    virtual void    _ProjectCenterToPlane (DgnElementP, DPlane3dCR) = 0;
    virtual void    _ProjectCenterToLine  (DgnElementP, DRay3dCR) = 0;

    DGNPLATFORM_EXPORT ComputeXYConstraint (DependencyGraph&, DgnElementP arc);
    DGNPLATFORM_EXPORT virtual void     _Satisfy (DependencyGraph& graph) override;

    }; // ComputeXYConstraint
#endif

}; // ArcPrimitiveWithNamedPlanes

/*=================================================================================**//**
*
* A rigid body with an origin and three orientation vectors.
*
DOF
ZO          3   (2 rotational and 1 translational) 2 rotations to establish the orientation of the wall's base plane, plus translation of the wall along its base plane normal
YO          3   (1 rotational and 2 translational) 1 rotation about the wall's normal, plus translation in the wall's local x-y plane
X           1   (                 1 translational) translation along the body's local x-axis

Planes
ZO          "z origin" plane is the base of the body, with its normal pointing up.
YO          "y origin" plane is right face of the body, with its normal pointing into the body.
XO          "x origin" plane is the front face of the body, with its normal pointing into the body.

Plane ->    DOF
ZO          ZO
YO          YO
XO          X

* @bsiclass                                     sam.wilson                      03/2011
+===============+===============+===============+===============+===============+======*/
struct          RigidBodyPrimitiveWithNamedPlane
{
private:
    DVec3d      m_x, m_y, m_z;
    DPoint3d    m_origin;
public:
    RigidBodyPrimitiveWithNamedPlane (DVec3dCR x, DVec3dCR y, DVec3dCR z, DPoint3dCR o) : m_x(x), m_y(y), m_z(z), m_origin(o) {;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT DVec3d  GetXAxis () const;
    DGNPLATFORM_EXPORT DVec3d  GetYAxis () const;
    DGNPLATFORM_EXPORT DVec3d  GetZAxis () const;
    DGNPLATFORM_EXPORT DPoint3d GetOrigin () const;

    //  ------------------------------------------------------------------
    //  Transform
    //  ------------------------------------------------------------------
    DGNPLATFORM_EXPORT void DoTransform (Transform const&);

    //  ------------------------------------------------------------------
    //  Named Planes
    //  ------------------------------------------------------------------
    DGNPLATFORM_EXPORT DPlane3d         GetNamedPlane (wchar_t const*, DPlane3dCP, DPoint3dCP) const;
    DGNPLATFORM_EXPORT StatusInt        MakeNamedPlaneCoincident (bool& reallyChanged, WCharCP planeName, DPlane3dCR targetPlane, DependencyGraph&, DgnElementP);
    DGNPLATFORM_EXPORT StatusInt        MoveNamedPlaneToContact  (bool& reallyChanged, WCharCP planeName, DPlane3dCR targetPlane, ICurvePrimitiveCR targetCurve, DependencyGraph&, DgnElementP);
    DGNPLATFORM_EXPORT StatusInt        MakeTangentAtNamedPlane  (bool& reallyChanged, WCharCP planeName, DPlane3dCR perpPlane, DPlane3dCR tanPlane, DependencyGraph&, DgnElementP);
    DGNPLATFORM_EXPORT ICurvePrimitivePtr GetSourceCurveForMoveNamedPlaneToContact (WCharCP planeName) const;
    DGNPLATFORM_EXPORT ICurvePrimitivePtr GetTargetCurveForMoveNamedPlaneToContact (WCharCP planeName) const;

    DGNPLATFORM_EXPORT static StatusInt GetVariablesForMakeNamedPlaneCoincident (bvector<bpair<DGVariable*,bool> >& nodes, DependencyGraph& graph, DgnElementP ref, WCharCP planeName);
    DGNPLATFORM_EXPORT static StatusInt GetVariablesForMoveNamedPlaneToContact  (bvector<bpair<DGVariable*,bool> >& nodes, DependencyGraph& graph, DgnElementP ref, WCharCP planeName);
    DGNPLATFORM_EXPORT static StatusInt GetVariablesForMakeTangentAtNamedPlane  (bvector<bpair<DGVariable*,bool> >& nodes, DependencyGraph& graph, DgnElementP ref, WCharCP planeName);

}; // LcsPrimitiveWithNamedPlane

struct PrimitivesWithNamedPlane
    {
    DGNPLATFORM_EXPORT static StatusInt ComputePointOfIntersection (DPoint3dR isectPoint, ICurvePrimitiveCR thisCurve, ICurvePrimitiveCR targetCurve, DPoint3dCR testPointIn);
    };

END_BENTLEY_DGN_NAMESPACE
