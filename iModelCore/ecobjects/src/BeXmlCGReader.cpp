/*--------------------------------------------------------------------------------------+
|
|  $Source: src/BeXmlCGReader.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
// If primitve has a child curve vector, just extract it.
// Otherwise put the primitive in a new curve vector.
static CurveVectorPtr CurveVectorOf (ICurvePrimitivePtr primitive, CurveVector::BoundaryType btype)
    {
    if (NULL == primitive.get())
        return CurveVectorPtr ();
    CurveVectorCP child = primitive->GetChildCurveVectorCP ();
    if (NULL != child)
        {
        return child->Clone ();
        }
    CurveVectorPtr vector = CurveVector::Create (btype);
    vector->push_back (primitive);
    return vector;
    }

BeXmlNodeP FindChild (BeXmlNodeP parent, CharCP name)
    {
    for (BeXmlNodeP child = parent->GetFirstChild (BEXMLNODE_Any); NULL != child;
                child = child->GetNextSibling (BEXMLNODE_Any))
        {
        if (0 == BeStringUtilities::Stricmp (child->GetName (), name))
            return child;
        }
    return NULL;
    }

bool GetDPoint3d (BeXmlNodeP element, DPoint3dR xyz)
    {
    BeXmlNodeP text;
    bvector<double> doubles;
    if (   NULL != element
        && NULL != (text = element->GetFirstChild (BEXMLNODE_Any))
        && BEXML_Success == text->GetContentDoubleValues (doubles)
        && doubles.size () == 3)
        {
        xyz.x = doubles[0];
        xyz.y = doubles[1];
        xyz.z = doubles[2];
        return true;
        }
    return false;
    }

bool GetDouble (BeXmlNodeP element, double &value)
    {
    BeXmlNodeP text;
    return NULL != element
        && NULL != (text = element->GetFirstChild (BEXMLNODE_Any))
        && BEXML_Success == text->GetContentDoubleValue (value);
    }


bool GetBool (BeXmlNodeP element, bool &value)
    {
    BeXmlNodeP text;
    return NULL != element
        && NULL != (text = element->GetFirstChild (BEXMLNODE_Any))
        && BEXML_Success == text->GetContentBooleanValue (value);
    }



bool FindChildBool (BeXmlNodeP parent, CharCP name, bool &value) {return GetBool (FindChild (parent, name), value);}
bool FindChildDPoint3d (BeXmlNodeP parent, CharCP name, DPoint3dR xyz){return GetDPoint3d (FindChild (parent, name), xyz);}
bool FindChildDouble (BeXmlNodeP parent, CharCP name, double &value) {return GetDouble (FindChild (parent, name), value);}

bool FindChildInt (BeXmlNodeP parent, CharCP name, int &value)
    {
    BeXmlNodeP child = FindChild (parent, name);
    if (NULL != child
        && BEXML_Success == child->GetContentInt32Value (value))
        {
        return true;
        }
    return false;
    }

bool FindChildPlacement (BeXmlNodeP parent, CharCP name, DPoint3dR origin, RotMatrixR axes)
    {
    BeXmlNodeP child = FindChild (parent, name);
    DVec3d vectorX, vectorZ;
    if (NULL != child
        && FindChildDPoint3d (child, "origin", origin)
        && FindChildDPoint3d (child, "vectorZ", vectorZ)    // DVec3d has DPoint3d base class !!!
        && FindChildDPoint3d (child, "vectorX", vectorX)
        )
        {
        DVec3d vectorY;
        vectorY.CrossProduct (vectorZ, vectorX);
        axes = RotMatrix::FromColumnVectors (vectorX, vectorY, vectorZ);
        axes.SquareAndNormalizeColumns (axes, 2, 0);
        return true;
        }
    return false;
    }

bool GetPoints (BeXmlNodeP parent, CharCP listName, CharCP pointName, bvector<DPoint3d> &points)
    {
    BeXmlNodeP listNode = FindChild (parent, listName);
    if (NULL == listNode)
        return false;
    DPoint3d xyz;
    for (BeXmlNodeP child = listNode->GetFirstChild (BEXMLNODE_Element); NULL != child;
                child = child->GetNextSibling (BEXMLNODE_Element))
        {
        if ((NULL == pointName
             || 0 == BeStringUtilities::Stricmp (child->GetName (), pointName))
            && GetDPoint3d (child, xyz))
            points.push_back (xyz);
        }
    return true;
    }

bool GetDoubles (BeXmlNodeP parent, CharCP listName, CharCP pointName, bvector<double> &values)
    {
    BeXmlNodeP listNode = FindChild (parent, listName);
    if (NULL == listName)
        return false;
    double value;
    for (BeXmlNodeP child = listNode->GetFirstChild (BEXMLNODE_Any); NULL != child;
                child = child->GetNextSibling (BEXMLNODE_Any))
        {
        if (0 == BeStringUtilities::Stricmp (child->GetName (), pointName)
            && GetDouble (child, value))
            values.push_back (value);
        }
    return true;
    }



bool BeXmlCGParser::TryParse (BeXmlNodeP node, MSBsplineSurfacePtr &result)
    {
    int orderU = 0;
    int orderV = 0;
    bool closedU = false;
    bool closedV = false;
    int numPolesU = 0;
    int numPolesV = 0;

    bvector<DPoint3d> points;
    bvector<double>   knotsU;
    bvector<double>   knotsV;
    bvector<double>   weights;


    if (0 == BeStringUtilities::Stricmp (node->GetName (), "BsplineSurface")
        && FindChildInt (node, "orderU", orderU)
        && FindChildInt (node, "numUControlPoint", numPolesU)
        && FindChildInt (node, "orderV", orderV)
        && FindChildInt (node, "numVControlPoint", numPolesV)
        && GetPoints (node, "ListOfControlPoint", NULL, points)
        )
        {
        FindChildBool (node, "closedU", closedU);   // optional !!
        FindChildBool (node, "closedV", closedV);   // optional !!
        GetDoubles (node, "ListOfKnotU", "knotU", knotsU);
        GetDoubles (node, "ListOfKnotU", "knotV", knotsV);
        MSBsplineSurfacePtr surface = MSBsplineSurface::CreatePtr ();
        if (SUCCESS == surface->Populate (points,
                weights.size () > 0 ? &weights : NULL,
                knotsU.size () > 0 ? &knotsU : NULL, orderU, numPolesU, closedU,
                knotsV.size () > 0 ? &knotsV : NULL, orderV, numPolesV, closedV,
                true
                ))
            {
            result = surface;
            return true;
            }
        return true;
        }
    return false;
    }

bool BeXmlCGParser::TryParse (BeXmlNodeP node, ICurvePrimitivePtr &result)
    {
    if (BeStringUtilities::Stricmp (node->GetName (), "LineSegment") == 0)
        {
        DSegment3d segment;
        if (   FindChildDPoint3d (node, "StartPoint", segment.point[0])
            && FindChildDPoint3d (node, "EndPoint", segment.point[1]))
            {
            result = ICurvePrimitive::CreateLine (segment);
            return true;
            }
        }
    else if (BeStringUtilities::Stricmp (node->GetName (), "CircularArc") == 0)
        {
        RotMatrix axes;
        DPoint3d origin;
        double radius = 0.0;
        double startDegrees = 0.0;
        double sweepDegrees = 0.0;
        if (   FindChildPlacement (node, "placement", origin, axes)
            && FindChildDouble (node, "radius", radius)
            && FindChildDouble (node, "startAngle", startDegrees)
            && FindChildDouble (node, "sweepAngle", sweepDegrees)
            )
            {
            result = ICurvePrimitive::CreateArc (
                        DEllipse3d::FromScaledRotMatrix (
                                origin, axes,
                                radius, radius,
                                Angle::DegreesToRadians (startDegrees),
                                Angle::DegreesToRadians (sweepDegrees)
                                ));
            return true;
            }
        }
    else if (BeStringUtilities::Stricmp (node->GetName (), "EllipticArc") == 0)
        {
        RotMatrix axes;
        DPoint3d origin;
        double radiusA = 0.0;
        double radiusB = 0.0;
        double startDegrees = 0.0;
        double sweepDegrees = 0.0;
        if (   FindChildPlacement (node, "placement", origin, axes)
            && FindChildDouble (node, "radiusA", radiusA)
            && FindChildDouble (node, "radiusB", radiusB)
            && FindChildDouble (node, "startAngle", startDegrees)
            && FindChildDouble (node, "startAngle", startDegrees)
            && FindChildDouble (node, "sweepAngle", sweepDegrees)
            )
            {
            result = ICurvePrimitive::CreateArc (
                        DEllipse3d::FromScaledRotMatrix (
                                origin, axes,
                                radiusA, radiusB,
                                Angle::DegreesToRadians (startDegrees),
                                Angle::DegreesToRadians (sweepDegrees)
                                ));
            return true;
            }
        }
    else if (BeStringUtilities::Stricmp (node->GetName (), "SurfacePatch") == 0)
        {
        BeXmlNodeP exteriorLoop = FindChild (node, "ExteriorLoop");
        BeXmlNodeP holeLoops     = FindChild (node, "ListOfHoleLoop");
        if (NULL != exteriorLoop)
            {
            ICurvePrimitivePtr exteriorChild;
            CurveVectorPtr loops = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
            if (TryParse (exteriorLoop->GetFirstChild (BEXMLNODE_Element), exteriorChild))
                {
                loops->push_back (exteriorChild);
                if (NULL != holeLoops)
                    {
                    for (BeXmlNodeP child = holeLoops->GetFirstChild (BEXMLNODE_Element); NULL != child;
                                child = child->GetNextSibling (BEXMLNODE_Element))
                        {
                        ICurvePrimitivePtr hole;
                        if (TryParse (child, hole))
                            {
                            loops->push_back (hole);
                            size_t index = loops->size () - 1;
                            CurveVector::BoundaryType boundaryType;
                            if (loops->GetChildBoundaryType (index, boundaryType))
                                loops->SetChildBoundaryType (index, CurveVector::BOUNDARY_TYPE_Inner);
                            }
                        }
                    }
                result = ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*loops);
                // hmm.. how to verify/announce validity (closed curve as child)???
                return true;
                }
            }
        }
    else if (BeStringUtilities::Stricmp (node->GetName (), "CurveChain") == 0)
        {
        BeXmlNodeP curveList = FindChild (node, "ListOfCurve");
        CurveVectorPtr curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
        for (BeXmlNodeP child = curveList->GetFirstChild (BEXMLNODE_Element); NULL != child;
                    child = child->GetNextSibling (BEXMLNODE_Element))
            {
            ICurvePrimitivePtr segment;
            TryParse (child, segment);
            curves->push_back (segment);
            }
        result = ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*curves);
        return true;
        }
    else if (BeStringUtilities::Stricmp (node->GetName (), "Linestring") == 0)
        {
        bvector<DPoint3d> points;
        if (GetPoints (node, "ListOfPoint", NULL, points))    // allow any tag name in the points !!!
            {
            result = ICurvePrimitive::CreateLineString (points);
            return true;
            }
        }
    else if (BeStringUtilities::Stricmp (node->GetName (), "Polygon") == 0)
        {
        bvector<DPoint3d> points;
        if (GetPoints (node, "ListOfPoint", NULL, points))    // allow any tag name in the points !!!
            {
            ICurvePrimitivePtr linestring = ICurvePrimitive::CreateLineString (points);
            CurveVectorPtr area = CurveVector::Create
                        (CurveVector::BOUNDARY_TYPE_Outer);
            area->push_back (linestring);
            result = ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*area);
            return true;
            }
        }
    else if (BeStringUtilities::Stricmp (node->GetName (), "CircularDisk") == 0)
        {
        RotMatrix axes;
        DPoint3d origin;
        double radius;
        if (   FindChildPlacement (node, "placement", origin, axes)
            && FindChildDouble (node, "radius", radius)
            )
            {
            ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (
                        DEllipse3d::FromScaledRotMatrix (
                                origin, axes,
                                radius, radius,
                                0.0, Angle::TwoPi ()));
            CurveVectorPtr area = CurveVector::Create
                        (CurveVector::BOUNDARY_TYPE_Outer);
            area->push_back (arc);
            result = ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*area);
            return true;
            }
        }
    else if (BeStringUtilities::Stricmp (node->GetName (), "EllipticDisk") == 0)
        {
        RotMatrix axes;
        DPoint3d origin;
        double radiusA, radiusB;
        if (   FindChildPlacement (node, "placement", origin, axes)
            && FindChildDouble (node, "radiusA", radiusA)
            && FindChildDouble (node, "radiusB", radiusB)
            )
            {
            ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (
                        DEllipse3d::FromScaledRotMatrix (
                                origin, axes,
                                radiusA, radiusB,
                                0.0, Angle::TwoPi ()));
            CurveVectorPtr area = CurveVector::Create
                        (CurveVector::BOUNDARY_TYPE_Outer);
            area->push_back (arc);
            result = ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*area);
            return true;
            }
        }
    else if (BeStringUtilities::Stricmp (node->GetName (), "BsplineCurve") == 0)
        {
        int order;
        bool closed = false;
        bvector<DPoint3d> points;
        bvector<double>   knots;
        bvector<double>   weights;
        if (FindChildInt (node, "order", order)
            && GetPoints (node, "ListOfControlPoint", NULL, points)) // "ControlPoint" ???
            {
            MSBsplineCurvePtr bcurve = MSBsplineCurve::CreatePtr ();
            GetDoubles (node, "ListOfKnot", "knot", knots);
            GetDoubles (node, "ListOfWeight", "weight", weights);
            FindChildBool (node, "closed", closed);
            bcurve->Populate (points,
                            weights.size () > 0 ? &weights : NULL,
                            knots.size () > 0 ? &knots : NULL,
                            order, closed, true);
            result = ICurvePrimitive::CreateBsplineCurve (*bcurve);
            return true;
            }
        }
    result = ICurvePrimitivePtr ();
    return false;
    }

bool BeXmlCGParser::TryParse (BeXmlNodeP node, ISolidPrimitivePtr &result)
    {
    CharCP name = node->GetName ();
    if (BeStringUtilities::Stricmp (name, "CircularCone") == 0)
        {
        RotMatrix axes;
        DPoint3d centerA, centerB;
        double radiusA, radiusB, height;
        bool capped;
        if (   FindChildPlacement (node, "placement", centerA, axes)
            && FindChildDouble (node, "radiusA", radiusA)
            && FindChildDouble (node, "radiusB", radiusB)
            && FindChildDouble (node, "height", height)
            && FindChildBool (node, "bSolidFlag", capped)
            )
            {
            DVec3d vectorX, vectorY, vectorZ;
            axes.GetColumns (vectorX, vectorY, vectorZ);
            centerB.SumOf (centerA, vectorZ, height);
            DgnConeDetail coneDetail (centerA, centerB,
                        axes, 
                        radiusA, radiusB,
                        capped);
            result = ISolidPrimitive::CreateDgnCone (coneDetail);
            return true;
            }
        }
    else if (BeStringUtilities::Stricmp (name, "SkewedCone") == 0)
        {
        RotMatrix axes;
        DPoint3d centerA, centerB;
        double radiusA, radiusB;
        bool capped;
        if (   FindChildPlacement (node, "placement", centerA, axes)
            && FindChildDouble (node, "radiusA", radiusA)
            && FindChildDouble (node, "radiusB", radiusB)
            && FindChildDPoint3d (node, "centerB", centerB)
            && FindChildBool (node, "bSolidFlag", capped)
            )
            {
            DVec3d vectorX, vectorY, vectorZ;
            axes.GetColumns (vectorX, vectorY, vectorZ);
            DgnConeDetail coneDetail (centerA, centerB,
                        axes, 
                        radiusA, radiusB,
                        capped);
            result = ISolidPrimitive::CreateDgnCone (coneDetail);
            return true;
            }
        }
    else if (BeStringUtilities::Stricmp (name, "Sphere") == 0)
        {
        RotMatrix axes;
        DPoint3d center;
        double radius;
        if (   FindChildPlacement (node, "placement", center, axes)
            && FindChildDouble (node, "radius", radius)
            )
            {
            DgnSphereDetail detail (center, axes, radius);
            result = ISolidPrimitive::CreateDgnSphere (detail);
            return true;
            }
        }
    else if (BeStringUtilities::Stricmp (name, "Box") == 0)
        {
        RotMatrix axes;
        DPoint3d origin;
        DVec3d cornerA, cornerB;
        bool bSolid;
        if (   FindChildPlacement (node, "placement", origin, axes)
            && FindChildDPoint3d (node, "cornerA", cornerA)
            && FindChildDPoint3d (node, "cornerB", cornerB)
            && FindChildBool (node, "bSolidFlag", bSolid)
            )
            {
            DVec3d vectorX, vectorY, vectorZ;
            axes.GetColumns (vectorX, vectorY, vectorZ);
            DPoint3d baseOrigin = DPoint3d::FromProduct (origin, axes, cornerA.x, cornerA.y, cornerA.z);
            DPoint3d topOrigin  = DPoint3d::FromProduct (origin, axes, cornerA.x, cornerA.y, cornerB.z);
            double dx = cornerB.x - cornerA.x;
            double dy = cornerB.y - cornerA.y;
            DgnBoxDetail detail (baseOrigin, topOrigin, vectorX, vectorY, dx, dy, dx, dy, bSolid);
            result = ISolidPrimitive::CreateDgnBox (detail);
            return true;
            }
        }
    else if (BeStringUtilities::Stricmp (name, "CircularCylinder") == 0)
        {
        RotMatrix axes;
        DPoint3d centerA, centerB;
        double radius;
        double height;
        bool capped = false;
        FindChildBool (node, "bSolidFlag", capped);
        if (   FindChildPlacement (node, "placement", centerA, axes)
            && FindChildDouble (node, "radius", radius)
            && FindChildDouble (node, "height", height)
            )
            {
            DVec3d vectorX, vectorY, vectorZ;
            axes.GetColumns (vectorX, vectorY, vectorZ);
            centerB.SumOf (centerA, vectorZ, height);
            DgnConeDetail coneDetail (centerA, centerB,
                        axes, 
                        radius, radius,
                        capped);
            result = ISolidPrimitive::CreateDgnCone (coneDetail);
            return true;
            }
        }
    else if (BeStringUtilities::Stricmp (name, "TorusPipe") == 0)
        {
        RotMatrix axes;
        DPoint3d center;
        double radiusA, radiusB;
        double sweepDegrees = 360.0, startDegrees = 0.0;
        bool capped = false;
        FindChildBool (node, "bSolidFlag", capped);
        FindChildDouble (node, "startAngle", startDegrees);
        FindChildDouble (node, "sweepAngle", sweepDegrees);
        if (   FindChildPlacement (node, "placement", center, axes)
            && FindChildDouble (node, "radiusA", radiusA)
            && FindChildDouble (node, "radiusB", radiusB)
            )
            {
            DVec3d vectorX, vectorY, vectorZ, vector0;
            axes.GetColumns (vectorX, vectorY, vectorZ);
            vector0 = vectorX;
            double startRadians = Angle::DegreesToRadians (startDegrees);
            double sweepRadians = Angle::DegreesToRadians (sweepDegrees);
            if (startDegrees != 0.0)
                vector0.SumOf (vectorX, cos (startRadians), vectorY, sin(startRadians));
            DgnTorusPipeDetail detail (center,
                        vectorX, vectorY, 
                        radiusA, radiusB,
                        sweepRadians,
                        capped);
            result = ISolidPrimitive::CreateDgnTorusPipe (detail);
            return true;
            }
        }
    else if (BeStringUtilities::Stricmp (name, "SurfaceBySweptCurve") == 0)
        {
        BeXmlNodeP baseGeometryNode = FindChild (node, "BaseGeometry");
        BeXmlNodeP railCurveNode    = FindChild (node, "RailCurve");
        ICurvePrimitivePtr baseGeometry;
        ICurvePrimitivePtr railCurve;
        if (   NULL != baseGeometryNode
            && NULL != railCurveNode
            && TryParse (baseGeometryNode->GetFirstChild (), baseGeometry)
            && TryParse (railCurveNode->GetFirstChild (), railCurve)
           )
            {
            DSegment3d segment;
            DEllipse3d arc;
            if (0 != railCurve->TryGetLine (segment))
                {
                DVec3d vector = DVec3d::FromStartEnd (segment.point[0], segment.point[1]);
                result = ISolidPrimitive::CreateDgnExtrusion (DgnExtrusionDetail (
                        CurveVectorOf (baseGeometry, CurveVector::BOUNDARY_TYPE_Open),
                                vector, false));
                return true;
                }
            else if (0 != railCurve->TryGetArc (arc))
                {
                DVec3d normal = DVec3d::FromNormalizedCrossProduct (arc.vector0, arc.vector90);
                result = ISolidPrimitive::CreateDgnRotationalSweep (DgnRotationalSweepDetail (
                        CurveVectorOf (baseGeometry, CurveVector::BOUNDARY_TYPE_Open),
                                arc.center, normal, arc.sweep, false));
                return true;
                }
            }
        }
    return false;
    }

BeXmlCGParser::BeXmlCGParser ()
    {
    }


bool BeXmlCGParser::TryParse (BeXmlNodeP node, bvector<IGeometryPtr> &geometry, size_t maxDepth)
    {
    ICurvePrimitivePtr curvePrimitive;
    ISolidPrimitivePtr solidPrimitive;
    MSBsplineSurfacePtr surface;
    size_t count = 0;
    if (TryParse (node, curvePrimitive))
        {
        geometry.push_back (IGeometryPtr::Create(curvePrimitive));
        count = 1;
        }
    else if (TryParse (node, solidPrimitive))
        {
        geometry.push_back (IGeometryPtr::Create(solidPrimitive));
        count = 1;
        }
    else if (TryParse (node, surface))
        {
        geometry.push_back (IGeometryPtr::Create (surface));
        count = 1;
        }
    else if (maxDepth == 0)
        {
        }
    else
        {
        for (BeXmlNodeP child = node->GetFirstChild (BEXMLNODE_Element); NULL != child;
                    child = child->GetNextSibling (BEXMLNODE_Element))
            {
            count += TryParse (child, geometry, maxDepth - 1);
            }
        }
    return count > 0;
    }

bool BeXmlCGParser::TryParse (Utf8CP beXmlCGString, ICurvePrimitivePtr &result)
    {
    size_t stringByteCount = strlen (beXmlCGString) * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXmlDom(BeXmlDom::CreateAndReadFromString (xmlStatus, beXmlCGString, stringByteCount));

    BeXmlNodeP child(pXmlDom->GetRootElement());
    return TryParse(child, result);
    }

bool BeXmlCGParser::TryParse (Utf8CP beXmlCGString, ISolidPrimitivePtr &result)
    {
    size_t stringByteCount = strlen (beXmlCGString) * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXmlDom(BeXmlDom::CreateAndReadFromString (xmlStatus, beXmlCGString, stringByteCount));

    BeXmlNodeP child(pXmlDom->GetRootElement());
    return TryParse(child, result);
    }

bool BeXmlCGParser::TryParse (Utf8CP beXmlCGString, MSBsplineSurfacePtr &result)
    {
    size_t stringByteCount = strlen (beXmlCGString) * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXmlDom(BeXmlDom::CreateAndReadFromString (xmlStatus, beXmlCGString, stringByteCount));

    BeXmlNodeP child(pXmlDom->GetRootElement());
    return TryParse(child, result);
    }

bool BeXmlCGParser::TryParse (Utf8CP beXmlCGString, bvector<IGeometryPtr> &geometry, size_t maxDepth)
    {
    size_t stringByteCount = strlen (beXmlCGString) * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXmlDom(BeXmlDom::CreateAndReadFromString (xmlStatus, beXmlCGString, stringByteCount));

    BeXmlNodeP child(pXmlDom->GetRootElement());
    return TryParse(child, geometry, maxDepth);
    }

END_BENTLEY_ECOBJECT_NAMESPACE