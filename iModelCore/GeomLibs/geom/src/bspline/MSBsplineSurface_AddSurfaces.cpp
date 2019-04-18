/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>

#include "msbsplinemaster.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static bool AddToVectorIfNonTrivial (bvector<MSBsplineSurfacePtr> &surfaces, MSBsplineSurfacePtr candidate)
    {
    if (candidate.IsValid ()
        && candidate->uParams.numPoles > 1
        && candidate->vParams.numPoles > 1
        )
        {
        surfaces.push_back (candidate);
        return true;
        }
    return false;
    }


struct TrimLoopBuilder
{
private:
bvector <DPoint3d> m_points;
bvector <MSBsplineSurfacePtr> &m_surfaces;
IFacetOptionsR m_facetOptions;
size_t m_numErrors;




public:
TrimLoopBuilder (bvector<MSBsplineSurfacePtr> &surfaces, IFacetOptionsR options)
    : m_facetOptions (options),
      m_surfaces (surfaces),
      m_numErrors(0)
    {
    }

bool AddIfNonTrivial (MSBsplineSurfacePtr surface)
    {
    return AddToVectorIfNonTrivial (m_surfaces, surface);
    }

size_t NumErrors (){ return m_numErrors;}

void RecordError (char const*s)
    {
    m_numErrors++;
    }
    
    
void RecurseToLoops (MSBsplineSurfaceR surface, CurveVectorCR source, TransformCR sourceToParameter)
    {
    if (source.IsClosedPath ())
        {
        m_points.clear ();
        source.AddStrokePoints (m_points, m_facetOptions);
        sourceToParameter.Multiply (m_points, m_points);
        surface.AddTrimBoundary (m_points);
        }
    else if (source.IsParityRegion () || source.IsUnionRegion ())
        {
        for (size_t i = 0; i < source.size(); i++)
            {
            CurveVectorCP child = source.at(i)->GetChildCurveVectorCP ();
            if (NULL != child)
                RecurseToLoops (surface, *child, sourceToParameter);
            }
        }
    else
        RecordError ("CurveVector to surface -- bad CV contents");
    }

void ConvertSingleRegionToSurface (CurveVectorCR source)
    {
    Transform localToWorld, worldToLocal;
    DRange3d localRange;
    CurveVectorPtr localCurves = source.CloneInLocalCoordinates (LOCAL_COORDINATE_SCALE_UnitAxesAtLowerLeft,
            localToWorld, worldToLocal, localRange);
    double dx = localRange.XLength ();
    double dy = localRange.YLength ();
    double sx, sy; 
    if (!DoubleOps::SafeDivideParameter (sx, 1.0, dx, 1.0)
        || !DoubleOps::SafeDivideParameter (sy, 1.0, dy, 1.0))
        {
        RecordError ("CurveVector to trimmed surface -- zero area trim region");
        return;
        }
        
    Transform localToParameter = Transform::FromRowValues (
        sx, 0, 0, 0,
        0, sy, 0, 0,
        0,  0, 1, 0
        );

    if (localCurves.IsValid ())
        {
        // stroke from original to get tolerance in original sizes.
        DPoint3d poles[4];
        localToWorld.Multiply (poles[0], 0, 0, 0);
        localToWorld.Multiply (poles[1], dx, 0, 0);
        localToWorld.Multiply (poles[2], 0, dy, 0);
        localToWorld.Multiply (poles[3], dx, dy, 0);
        MSBsplineSurfacePtr surface = MSBsplineSurface::CreatePtr ();
        surface->InitFromPointsAndOrder (2, 2, 2, 2, poles);
        surface->SetNumRules (2,2);
        surface->SetOuterBoundaryActive (false);
        RecurseToLoops (*surface, *localCurves, localToParameter);
        AddIfNonTrivial (surface);
        }
    }    
    
void AddDisk (DEllipse3dCR disk)
    {
    AddIfNonTrivial (MSBsplineSurface::CreateTrimmedDisk (disk));
    }
    
void AddBilinear (DPoint3dCP points, int i00, int i10, int i01, int i11)
    {
    DPoint3d poles[4];
    poles[0] = points[i00];
    poles[1] = points[i10];
    poles[2] = points[i11];
    poles[3] = points[i01];
    MSBsplineSurfacePtr surface = MSBsplineSurface::CreatePtr ();
    surface->InitFromPointsAndOrder (2, 2, 2, 2, poles);
    m_surfaces.push_back (surface);
    }
    
void Add (DgnConeDetailCR detail)
    {
    ICurvePrimitivePtr baseCurve = detail.GetConstantVSection (SolidLocationDetail::FaceIndices (0,0, 0), 0.0);
    ICurvePrimitivePtr topCurve = detail.GetConstantVSection (SolidLocationDetail::FaceIndices (0,0, 0), 1.0);
    DEllipse3d bottom, top;
    detail.FractionToSection (0.0, bottom);
    detail.FractionToSection (1.0, top);
    bottom.vector0.Negate ();   // forced negated surface normal ...
    //DVec3d axis = DVec3d::FromStartEnd (bottom.center, top.center);
    DSegment3d ruleLine;
    detail.FractionToRule (0.0, ruleLine);
    if (detail.m_capped)
        {
        if (detail.IsRealCap (0))
            AddDisk (bottom);
        if (detail.IsRealCap (1))
            AddDisk (top);
        }

#ifdef MakeRuleCurve
    MSBsplineCurve ruleCurve;
    ruleCurve.InitFromPoints (ruleLine.point, 2);
#endif
    AddIfNonTrivial (MSBsplineSurface::CreateRuled (*baseCurve, *topCurve));
    }

void Add (DgnSphereDetailCR detail)
    {
    DEllipse3d bottom = detail.VFractionToUSectionDEllipse3d (0.0);
    DEllipse3d top = detail.VFractionToUSectionDEllipse3d (1.0);
    bottom.vector0.Negate ();   // forced negated surface normal ...
    DVec3d axis = DVec3d::FromStartEnd (bottom.center, top.center);
    DEllipse3d meridianEllipse = detail.UFractionToVSectionDEllipse3d (0.0);
    if (detail.m_capped)
        {
        if (detail.IsRealCap (0))
            AddDisk (bottom);
        if (detail.IsRealCap (1))
            AddDisk (top);
        }
    // ummm... Should this be built as a unit sphere and transformed?
    MSBsplineCurve meridianCurve;
    meridianCurve.InitFromDEllipse3d (meridianEllipse);
    AddIfNonTrivial (MSBsplineSurface::CreateRotationalSweep (
            meridianCurve, bottom.center, axis, Angle::TwoPi ()));
    meridianCurve.ReleaseMem ();
    }

void Add (DgnBoxDetailCR detail)
    {
    DPoint3d boxCorners[8];
    detail.GetCorners (boxCorners);
    AddBilinear (boxCorners, 0,1,5,4);
    AddBilinear (boxCorners, 1,3,7,5);
    AddBilinear (boxCorners, 3,2,6,7);
    AddBilinear (boxCorners, 2,0,4,6);
    if (detail.m_capped)
        {
        AddBilinear (boxCorners, 4,5,7,6);
        AddBilinear (boxCorners, 0,2,3,1);
        }
    }

void Add (DgnTorusPipeDetailCR detail)
    {
    DEllipse3d cap0 = detail.VFractionToUSectionDEllipse3d (0.0);
    DEllipse3d cap1 = detail.VFractionToUSectionDEllipse3d (1.0);
    cap1.vector0.Negate ();   // forced negated surface normal ...
    DPoint3d center;
    DVec3d axis;
    double sweepRadians;
    if (detail.TryGetRotationAxis (center, axis, sweepRadians))
        {
        if (detail.m_capped && detail.HasRealCaps ())
            {
            AddDisk (cap0);
            AddDisk (cap1);
            }        
        }
    MSBsplineCurve sweepCurve;
    sweepCurve.InitFromDEllipse3d (cap0);
    AddIfNonTrivial (
            MSBsplineSurface::CreateRotationalSweep (
                sweepCurve, center, axis, sweepRadians)
                );
    sweepCurve.ReleaseMem ();
    }

void Add (DgnExtrusionDetailCR detail)
    {
    if (detail.m_capped)
        {
        ConvertSingleRegionToSurface (*detail.m_baseCurve);
        ConvertSingleRegionToSurface (*detail.FractionToProfile (1.0));
        }
    MSBsplineSurface::CreateLinearSweep (m_surfaces, *detail.m_baseCurve, detail.m_extrusionVector);
    }

void AddRuled (bvector<ICurvePrimitivePtr> const &primiitves);
void AddRuled (bvector<CurveVectorPtr> const &profiles)
    {
    bvector<ICurvePrimitivePtr> children;
    
    size_t numProfile = profiles.size ();
    if (numProfile < 2)
        {
        RecordError ("need 2 or more profiles for ruled surface");
        return;
        }
        
    CurveVector::BoundaryType btype = profiles[0]->GetBoundaryType ();
    size_t numPrimitive = profiles[0]->size ();
    for (size_t i = 1; i < numProfile; i++)
        {
        if (profiles[i]->GetBoundaryType () != btype)
            {
            RecordError ("profiles have mismatched boundary types");
            return;
            }
        if (profiles[i]->size () != numPrimitive)
            {
            RecordError ("profiles have mismatched primitive counts");
            return;
            }
        }
    
    bvector<ICurvePrimitivePtr> primitives;
    for (size_t primitiveIndex = 0; primitiveIndex < numPrimitive; primitiveIndex++)
        {
        primitives.clear ();
        for (size_t i = 0; i < numProfile; i++)
            primitives.push_back (profiles[i]->at(primitiveIndex));
        AddRuled (primitives);
        }
    }

void Add (DgnRuledSweepDetailCR detail)
    {
    AddRuled (detail.m_sectionCurves);
    if (detail.m_capped)
        {
        // NEEDS WORK
        }
    }

void Add (DgnRotationalSweepDetailCR detail)
    {
    // NEEDS WORK -- cap surfaces??  on-axis edges???
    DPoint3d center;
    DVec3d axis;
    double sweepRadians;

    if (detail.HasRealCaps ())
        {
        ConvertSingleRegionToSurface (*detail.VFractionToProfile (0.0));
        ConvertSingleRegionToSurface (*detail.VFractionToProfile (1.0));
        }
    if (!detail.TryGetRotationAxis (center, axis, sweepRadians))
        {
        RecordError ("No axis of rotation");
        return;
        }
    MSBsplineSurface::CreateRotationalSweep (m_surfaces, *detail.m_baseCurve, center, axis, sweepRadians);
    }

};


void TrimLoopBuilder::AddRuled (bvector<ICurvePrimitivePtr> const &primitives)
    {
    size_t numPrimitive = primitives.size ();
    if (numPrimitive < 2)
        return;
        
    
    bvector<MSBsplineCurve> curves;
    bvector<CurveVectorPtr> children;
    curves.reserve (numPrimitive);
    for (size_t i = 0; i < numPrimitive; i++)
        {
        MSBsplineCurve curve;
        CurveVectorPtr child = primitives[i]->GetChildCurveVectorP ();
        if (child.IsValid ())
            {
            children.push_back (child);
            }
        else if (primitives[i]->GetMSBsplineCurve (curve))
            {
            curves.push_back (curve);   // copies pointer bits !!
            curve.Zero ();
            }
        else
            RecordError ("Child must be vector or curve-convertible");
        }
        
    if (curves.size () == numPrimitive)
        {
        MSBsplineCurveP curveP = &curves[0];
        for (size_t i1 = 1; i1 < numPrimitive; i1++)
            {
            MSBsplineSurface bsurf;
            //if (SUCCESS != bspsurf_ruledSurfaceFromCompatibleCopiesOfCurves (&bsurf, &curveP[i1-1], &curveP[i1]))
            if (SUCCESS != bspsurf_ruledSurface (&bsurf, &curveP[i1-1], &curveP[i1]))
                {
                RecordError ("Incompatible curves for ruled surface");
                }
            else
                {
                MSBsplineSurfacePtr surfptr = bsurf.CreateCapture ();
                AddIfNonTrivial (surfptr);
                }
            }
        }
    else if (children.size () == numPrimitive)
        {
        AddRuled (children);
        }
    else
        RecordError ("Ruled surface children must be all curve vectors or all convertible to bcurve");

    for (size_t i = 0, n = curves.size (); i < n; i++)
        curves[i].ReleaseMem ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EDL             11/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::CreateTrimmedSurfaces
(
bvector <MSBsplineSurfacePtr> &surfaces,
CurveVectorCR source,
IFacetOptionsP options
)
    {
    IFacetOptionsPtr myOptions;
    if (NULL == options)
        {
        myOptions = IFacetOptions::CreateForCurves ();
        options = myOptions.get ();
        }
    TrimLoopBuilder builder (surfaces, *options);
    builder.ConvertSingleRegionToSurface(source);
    return builder.NumErrors () == 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EDL             11/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::CreateTrimmedSurfaces
(
bvector <MSBsplineSurfacePtr> &surfaces,
ISolidPrimitiveCR primitive,
IFacetOptionsP options
)
    {
    size_t numErrors = 0;
    DgnConeDetail             cone;
    DgnSphereDetail           sphere;
    DgnBoxDetail              box;
    DgnTorusPipeDetail        torus;
    DgnExtrusionDetail        extrusion;
    DgnRuledSweepDetail       ruledSweep;
    DgnRotationalSweepDetail  rotationalSweep;

    IFacetOptionsPtr myOptions = NULL;
    if (NULL == options)
        {
        myOptions = IFacetOptions::CreateForCurves ();
        options = myOptions.get ();
        }
    TrimLoopBuilder builder (surfaces, *options);
    
    if (primitive.TryGetDgnConeDetail (cone))
        builder.Add (cone);
    else if (primitive.TryGetDgnSphereDetail (sphere))
        builder.Add (sphere);
    else if (primitive.TryGetDgnBoxDetail (box))
        builder.Add (box);
    else if (primitive.TryGetDgnTorusPipeDetail (torus))
        builder.Add (torus);
    else if (primitive.TryGetDgnExtrusionDetail (extrusion))
        builder.Add (extrusion);
    else if (primitive.TryGetDgnRuledSweepDetail (ruledSweep))
        builder.Add (ruledSweep);
    else if (primitive.TryGetDgnRotationalSweepDetail (rotationalSweep))
        builder.Add (rotationalSweep);

    return numErrors == 0;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EDL             11/12
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineSurfacePtr MSBsplineSurface::CreateLinearSweep (ICurvePrimitiveCR primitive, DVec3dCR delta)
    {
    MSBsplineCurve bcurve;
    if (primitive.GetMSBsplineCurve (bcurve))
        {
        MSBsplineSurfacePtr surface = CreateLinearSweep (bcurve, delta);
        bcurve.ReleaseMem ();
        return surface;
        }
    return NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EDL             11/12
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineSurfacePtr MSBsplineSurface::CreateLinearSweep (MSBsplineCurveCR baseCurve, DVec3dCR delta)
    {
    MSBsplineSurface bsurf;
    if (SUCCESS == bspsurf_surfaceOfProjection (&bsurf, &baseCurve, &delta))
        return bsurf.CreateCapture ();
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EDL             11/12
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineSurfacePtr MSBsplineSurface::CreateRuled (ICurvePrimitiveCR curveA, ICurvePrimitiveCR curveB)
    {
    MSBsplineSurface bsurf;
    MSBsplineCurvePtr bcurveA = curveA.GetMSBsplineCurvePtr ();
    MSBsplineCurvePtr bcurveB = curveB.GetMSBsplineCurvePtr ();
    if (    bcurveA.IsValid ()
        &&  bcurveB.IsValid ()
        &&  SUCCESS == bspsurf_ruledSurface (&bsurf, bcurveA.get (), bcurveB.get ())
        )
        return bsurf.CreateCapture ();
    return NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EDL             11/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::CreateLinearSweep (bvector<MSBsplineSurfacePtr> &surfaces, CurveVectorCR baseCurves, DVec3dCR delta)
    {
    bool ok = true;
    for (size_t i = 0, n = baseCurves.size (); i < n; i++)
        {
        CurveVectorCP child = baseCurves.at(i)->GetChildCurveVectorCP ();
        if (NULL != child)
            ok |= CreateLinearSweep (surfaces, *child, delta);
        else
            {
            ok |= AddToVectorIfNonTrivial (surfaces, CreateLinearSweep (*baseCurves.at(i), delta));
            }
        }
    return ok;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EDL             11/12
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineSurfacePtr MSBsplineSurface::CreateRotationalSweep (MSBsplineCurveCR baseCurve,
DPoint3dCR center, 
DVec3dCR axis,
double sweepRadians)
    {
    MSBsplineSurface bsurf;
    if (SUCCESS == bspsurf_surfaceOfRevolution (&bsurf, &baseCurve, &center, &axis, 0.0, sweepRadians))
        return bsurf.CreateCapture();
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EDL             11/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::CreateRotationalSweep (bvector<MSBsplineSurfacePtr> &surfaces, CurveVectorCR baseCurves,
DPoint3dCR center, 
DVec3dCR axis,
double sweepRadians
)
    {
    bool ok = true;
    for (size_t i = 0, n = baseCurves.size (); i < n; i++)
        {
        CurveVectorCP child = baseCurves.at(i)->GetChildCurveVectorCP ();
        if (NULL != child)
            ok |= CreateRotationalSweep (surfaces, *child, center, axis, sweepRadians);
        else
            {
            ok |= AddToVectorIfNonTrivial (surfaces, CreateRotationalSweep (*baseCurves.at(i), center, axis, sweepRadians));
            }
        }
    return ok;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EDL             11/12
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineSurfacePtr MSBsplineSurface::CreateRotationalSweep (ICurvePrimitiveCR primitive,
DPoint3dCR center, 
DVec3dCR axis,
double sweepRadians
)
    {
    MSBsplineCurve bcurve;
    if (primitive.GetMSBsplineCurve (bcurve))
        {
        MSBsplineSurfacePtr surface = CreateRotationalSweep (bcurve, center, axis, sweepRadians);
        bcurve.ReleaseMem ();
        return surface;
        }
    return NULL;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
