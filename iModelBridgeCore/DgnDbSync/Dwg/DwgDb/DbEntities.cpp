/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgDb/DbEntities.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgDbInternal.h"

USING_NAMESPACE_DWGDB

// Add entities that are sub-classed from toolkit's entity classes
DWGDB_ENTITY_DEFINE_MEMBERS(Entity)
DWGDB_ENTITY_DEFINE_MEMBERS(Line)
DWGDB_ENTITY_DEFINE_MEMBERS(Point)
DWGDB_ENTITY_DEFINE_MEMBERS(Arc)
DWGDB_ENTITY_DEFINE_MEMBERS(Circle)
DWGDB_ENTITY_DEFINE_MEMBERS(Ellipse)
DWGDB_ENTITY_DEFINE_MEMBERS(Face)
DWGDB_ENTITY_DEFINE_MEMBERS(Polyline)
DWGDB_ENTITY_DEFINE_MEMBERS(2dPolyline)
DWGDB_ENTITY_DEFINE_MEMBERS(3dPolyline)
DWGDB_ENTITY_DEFINE_MEMBERS(Hatch)
DWGDB_ENTITY_DEFINE_MEMBERS(Region)
DWGDB_ENTITY_DEFINE_MEMBERS(Solid)
DWGDB_ENTITY_DEFINE_MEMBERS(Shape)
DWGDB_ENTITY_DEFINE_MEMBERS(Spline)
DWGDB_ENTITY_DEFINE_MEMBERS(Trace)
DWGDB_ENTITY_DEFINE_MEMBERS(Text)
DWGDB_ENTITY_DEFINE_MEMBERS(Attribute)
DWGDB_ENTITY_DEFINE_MEMBERS(AttributeDefinition)
DWGDB_ENTITY_DEFINE_MEMBERS(BlockReference)
DWGDB_ENTITY_DEFINE_MEMBERS(RasterImage)
DWGDB_ENTITY_DEFINE_MEMBERS(PointCloudEx)
DWGDB_ENTITY_DEFINE_MEMBERS(Viewport)



uint16_t            DwgDbEntity::GetColorIndex () const { return static_cast<uint16_t>(T_Super::colorIndex()); }
DwgCmColor          DwgDbEntity::GetColor () const { return static_cast<DwgCmColor>(T_Super::color()); }
DwgCmEntityColor    DwgDbEntity::GetEntityColor () const { return static_cast<DwgCmEntityColor>(T_Super::entityColor()); }
DwgDbObjectId       DwgDbEntity::GetLayerId () const { return T_Super::layerId(); }
DwgDbObjectId       DwgDbEntity::GetLinetypeId () const { return T_Super::linetypeId(); }
double              DwgDbEntity::GetLinetypeScale () const { return T_Super::linetypeScale(); }
DwgDbLineWeight     DwgDbEntity::GetLineweight () const { return DWGDB_UPWARDCAST(LineWeight)(T_Super::lineWeight()); }
DwgDbObjectId       DwgDbEntity::GetMaterialId () const { return T_Super::materialId(); }
DwgTransparency     DwgDbEntity::GetTransparency () const { return T_Super::transparency(); }
DwgDbVisibility     DwgDbEntity::GetVisibility () const { return static_cast<DwgDbVisibility>(T_Super::visibility()); }
void                DwgDbEntity::List () const { T_Super::list(); }
DwgGiDrawablePtr    DwgDbEntity::GetDrawable () { return new DwgGiDrawable(T_Super::drawable()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgDbEntity::GetEcs (TransformR ecs) const
    {
    DWGGE_Type(Matrix3d)  geMatrix;

#ifdef DWGTOOLKIT_OpenDwg
    geMatrix = T_Super::getEcs ();
#elif DWGTOOLKIT_RealDwg
    T_Super::getEcs (geMatrix);
#endif

    return Util::GetTransform (ecs, geMatrix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbEntity::GetGripPoints (DPoint3dArrayR points, DwgDbIntArrayP snapModes, DwgDbIntArrayP geomIds) const
    {
    DWGGE_Type(Point3dArray)    acPoints;
    DwgDbStatus                 status = DwgDbStatus::FirstDwgDbError;

#ifdef DWGTOOLKIT_OpenDwg

    status = ToDwgDbStatus(T_Super::getGripPoints(acPoints));

#elif DWGTOOLKIT_RealDwg
    AcDbIntArray                acModes;
    AcDbIntArray                acGeomIds;

    status = ToDwgDbStatus(T_Super::getGripPoints(acPoints, acModes, acGeomIds));

    if (DwgDbStatus::Success == status)
        {
        if (nullptr != snapModes)
            {
            for (int i = 0; i < acModes.length(); i++)
                snapModes->push_back (acModes.at(i));
            }
        if (nullptr != geomIds)
            {
            for (int i = 0; i < acGeomIds.length(); i++)
                geomIds->push_back (acGeomIds.at(i));
            }
        }
#endif

    if (DwgDbStatus::Success == status)
        {
        size_t      nPoints = static_cast <size_t> (acPoints.length());
        for (size_t i = 0; i < nPoints; i++)
            points.push_back (Util::DPoint3dFrom(acPoints.at(static_cast<int>(i))));
        }
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbEntity::GetRange (DRange3dR range) const
    {
    DWGDB_SDKNAME(OdGeExtents3d,AcDbExtents)    extents;

    DwgDbStatus status = ToDwgDbStatus (T_Super::getGeomExtents(extents));

    if (DwgDbStatus::Success == status)
        range = Util::DRange3dFrom (extents);
    else
        range.Init ();

    return  status;
    }


DPoint3d    DwgDbLine::GetStartPoint () const { return Util::DPoint3dFrom(T_Super::startPoint()); }
DPoint3d    DwgDbLine::GetEndPoint () const { return Util::DPoint3dFrom(T_Super::endPoint()); }
DVec3d      DwgDbLine::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double      DwgDbLine::GetThickness () const { return T_Super::thickness(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbPolyline::SetFromGiPolyline (DWGGI_TypeCR(Polyline) giPolyline)
    {
    DwgDbStatus status = DwgDbStatus::InvalidData;

#ifdef DWGTOOLKIT_OpenDwg
    unsigned int        numPoints = giPolyline.numVerts ();
    if (0 == numPoints)
        return  status;

    bool        hasWidth = giPolyline.hasWidth ();
    bool        hasConstantWidth = false;
    double      constantWidth = giPolyline.getConstantWidth ();
    if (0.0 != constantWidth)
        {
        T_Super::setConstantWidth (constantWidth);
        hasConstantWidth = true;
        }

    for (unsigned int i = 0; i < numPoints; i++)
        {
        OdGePoint2d     point = OdGePoint2d::kOrigin;
        giPolyline.getPointAt (i, point);
        T_Super::setPointAt (i, point);

        double          bulge = giPolyline.getBulgeAt (i);
        T_Super::setBulgeAt (i, bulge);

        if (hasWidth && !hasConstantWidth)
            {
            double      startWidth = 0.0, endWidth = 0.0;
            giPolyline.getWidthsAt (i, startWidth, endWidth);
            T_Super::setWidthsAt (i, startWidth, endWidth);
            }
        }
    
    T_Super::setNormal (giPolyline.normal());
    T_Super::setPlinegen (giPolyline.hasPlinegen());
    T_Super::setElevation (giPolyline.elevation());
    T_Super::setThickness (giPolyline.thickness());

#elif DWGTOOLKIT_RealDwg
    const AcGePoint3d*  points = giPolyline.vertexList ();
    Adesk::UInt32       numPoints = giPolyline.points ();
    if (nullptr == points || 0 == numPoints)
        return  status;

    for (Adesk::UInt32  i = 0; i < numPoints; i++)
        status = ToDwgDbStatus (T_Super::setPointAt(i, AcGePoint2d(points[i].x, points[i].y)));

    T_Super::setPlinegen (AcGiPolyline::kEndToEnd == giPolyline.linetypeGen());
    T_Super::setElevation (points[0].z);

    const AcGeVector3d*   normal = giPolyline.normal ();
    if (nullptr != normal)
        status = ToDwgDbStatus (T_Super::setNormal(*normal));

    // WIP - convert arcSegmentFlags?
#endif  // DWGTOOLKIT_

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d   DwgDbPolyline::GetPointAt (size_t index) const
    {
    DWGGE_Type(Point3d) point;
    T_Super::getPointAt (static_cast<unsigned int>(index), point);
    return  Util::DPoint3dFrom (point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool       DwgDbPolyline::GetWidthsAt (size_t index, double& start, double& end) const
    {
    start = end = 0.0;

#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getWidthsAt (static_cast<unsigned int>(index), start, end);
    return  true;
#elif DWGTOOLKIT_RealDwg
    return Acad::eOk == T_Super::getWidthsAt(static_cast<unsigned int>(index), start, end);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d   DwgDbPolyline::GetWidthsAt (size_t index) const
    {
    DPoint2d    widths;
    if (!this->GetWidthsAt(index, widths.x, widths.y))
        widths.Zero ();
    return  widths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool       DwgDbPolyline::GetConstantWidth (double& width) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    width = T_Super::getConstantWidth ();
    return  0.0 != width;
#elif DWGTOOLKIT_RealDwg
    return  Acad::eOk == T_Super::getConstantWidth (width);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
double     DwgDbPolyline::GetBulgeAt (size_t index) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return  T_Super::getBulgeAt (static_cast<unsigned int>(index));
#elif DWGTOOLKIT_RealDwg
    double  bulge = 0.0;
    return  Acad::eOk == T_Super::getBulgeAt(static_cast<unsigned int>(index), bulge) ? bulge : 0.0;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void       DwgDbPolyline::GetEcs (TransformR ecs) const
    {
    DWGGE_Type(Matrix3d)    matrix;
#ifdef DWGTOOLKIT_OpenDwg
    matrix =  T_Super::getEcs ();
#elif DWGTOOLKIT_RealDwg
    T_Super::getEcs (matrix);
#endif
    Util::GetTransform (ecs, matrix);
    }

bool       DwgDbPolyline::IsClosed () const { return DWGDB_IsTrue(T_Super::isClosed()); }
size_t     DwgDbPolyline::GetNumPoints () const { return T_Super::numVerts(); }
bool       DwgDbPolyline::HasWidth () const { return DWGDB_IsTrue(T_Super::hasWidth()); }
bool       DwgDbPolyline::HasBulges () const { return DWGDB_IsTrue(T_Super::hasBulges()); }
double     DwgDbPolyline::GetElevation () const { return T_Super::elevation(); }
double     DwgDbPolyline::GetThickness () const { return T_Super::thickness(); }
bool       DwgDbPolyline::HasPlinegen () const { return DWGDB_IsTrue(T_Super::hasPlinegen()); }
DVec3d     DwgDbPolyline::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool       DwgDb2dPolyline::GetConstantWidth (double& width) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    width = T_Super::defaultStartWidth ();
    return  fabs(width - T_Super::defaultEndWidth()) < 1.e-5;
#elif DWGTOOLKIT_RealDwg
    return  Acad::eOk == T_Super::constantWidth (width);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void       DwgDb2dPolyline::GetEcs (TransformR ecs) const
    {
    DWGGE_Type(Matrix3d)    matrix;
#ifdef DWGTOOLKIT_OpenDwg
    matrix =  T_Super::getEcs ();
#elif DWGTOOLKIT_RealDwg
    T_Super::getEcs (matrix);
#endif
    Util::GetTransform (ecs, matrix);
    }
bool       DwgDb2dPolyline::IsClosed () const { return DWGDB_IsTrue(T_Super::isClosed()); }
double     DwgDb2dPolyline::GetElevation () const { return T_Super::elevation(); }
double     DwgDb2dPolyline::GetThickness () const { return T_Super::thickness(); }
bool       DwgDb2dPolyline::HasPlinegen () const { return DWGDB_IsTrue(T_Super::isLinetypeGenerationOn()); }
DVec3d     DwgDb2dPolyline::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
DwgDbObjectIterator     DwgDb2dPolyline::GetVertexIterator () const { return DwgDbObjectIterator(T_Super::vertexIterator()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDb3dPolyline::Straighten ()
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::straighten ();
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::straighten());
#endif
    return  status;
    }
DwgDb3dPolyline::Type   DwgDb3dPolyline::GetType () const { return DWGDB_CASTFROMENUM_DB(3dPolyline::Type)(T_Super::polyType()); }
DwgDbObjectIterator     DwgDb3dPolyline::GetVertexIterator () const { return DwgDbObjectIterator(T_Super::vertexIterator()); }

DPoint3d   DwgDbArc::GetCenter () const { return Util::DPoint3dFrom(T_Super::center()); }
double     DwgDbArc::GetRadius () const { return T_Super::radius(); }
double     DwgDbArc::GetStartAngle () const { return T_Super::startAngle(); }
double     DwgDbArc::GetEndAngle () const { return T_Super::endAngle(); }
DVec3d     DwgDbArc::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double     DwgDbArc::GetThickness () const { return T_Super::thickness(); }
double     DwgDbArc::GetTotalAngle () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    double  swept = T_Super::endAngle() - T_Super::startAngle();
    return Angle::AdjustToSweep (swept,0, msGeomConst_2pi);
#elif DWGTOOLKIT_RealDwg
    return T_Super::totalAngle ();
#endif
    }

DPoint3d   DwgDbCircle::GetCenter () const { return Util::DPoint3dFrom(T_Super::center()); }
double     DwgDbCircle::GetDiameter () const { return DWGDB_CALLSDKMETHOD(T_Super::radius() * 2, T_Super::diameter()); }
DVec3d     DwgDbCircle::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double     DwgDbCircle::GetThickness () const { return T_Super::thickness(); }

DPoint3d   DwgDbEllipse::GetCenter () const { return Util::DPoint3dFrom(T_Super::center()); }
double     DwgDbEllipse::GetMajorRadius () const { return DWGDB_CALLSDKMETHOD(T_Super::majorAxis().length(), T_Super::majorRadius()); }
DVec3d     DwgDbEllipse::GetMajorAxis () const { return Util::DVec3dFrom(T_Super::majorAxis()); }
double     DwgDbEllipse::GetMinorRadius () const { return DWGDB_CALLSDKMETHOD(T_Super::minorAxis().length(), T_Super::minorRadius()); }
DVec3d     DwgDbEllipse::GetMinorAxis () const { return Util::DVec3dFrom(T_Super::minorAxis()); }
DVec3d     DwgDbEllipse::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double     DwgDbEllipse::GetStartAngle () const { return T_Super::startAngle(); }
double     DwgDbEllipse::GetEndAngle () const { return T_Super::endAngle(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbSpline::GetNurbsData (int16_t& degree, bool& rational, bool& closed, bool& periodic, DPoint3dArrayR poles, DwgDbDoubleArrayR knots, DwgDbDoubleArrayR weights, double& poleTol, double& knotTol) const
    {
    DWGGE_Type(Point3dArray)    gePoles;
    DWGGE_Type(DoubleArray)     geKnots;
    DWGGE_Type(DoubleArray)     geWeights;
    int d = 0;

    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getNurbsData (d, rational, closed, periodic, gePoles, geKnots, geWeights, poleTol, knotTol);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::getNurbsData(d, rational, closed, periodic, gePoles, geKnots, geWeights, poleTol, knotTol));
#endif
    if (status == DwgDbStatus::Success)
        {
        degree = static_cast <int16_t> (d);

        Util::GetPointArray (poles, gePoles);

        for (uint16_t i = 0; i < geKnots.length(); i++)
            knots.push_back (geKnots.at(i));

        for (uint16_t i = 0; i < geWeights.length(); i++)
            weights.push_back (geWeights.at(i));
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgDbAttribute::GetValueString (DwgStringR value) const
    {
    value.Empty ();
#ifdef DWGTOOLKIT_OpenDwg
    if (T_Super::isMTextAttribute())
        {
        OdDbMTextPtr    mtext = T_Super::getMTextAttribute ();
        if (!mtext.isNull())
            {
            mtext->convertFieldToText ();
            value = mtext->contents ();
            }
        }
    else
        {
        value = T_Super::textString ();
        }
#elif DWGTOOLKIT_RealDwg
    if (T_Super::isMTextAttribute())
        {
        AcDbMText*    mtext = T_Super::getMTextAttribute ();
        if (nullptr != mtext)
            {
            mtext->convertFieldToText ();

            ACHAR*  str = mtext->text ();
            if (nullptr != str)
                {
                value.assign (str);
                ::acutDelString (str);
                }
            delete mtext;
            }
        }
    else
        {
        value.assign (T_Super::textStringConst());
        }
#endif
    return  !value.IsEmpty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbAttribute::SetFrom (DwgDbAttributeDefinition const* attrdef, TransformCR toBlockRef)
    {
#ifdef DWGTOOLKIT_OpenDwg
    OdGeMatrix3d    matrix;
    Util::GetGeMatrix (matrix, toBlockRef);

    T_Super::setAttributeFromBlock (attrdef, matrix);

    return  DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg

    AcGeMatrix3d    matrix;
    Util::GetGeMatrix (matrix, toBlockRef);

    Acad::ErrorStatus   status = T_Super::setAttributeFromBlock (attrdef, matrix);

    return  ToDwgDbStatus(status);
#endif
    }
DPoint3d   DwgDbAttribute::GetOrigin () const { return Util::DPoint3dFrom(T_Super::position()); }
DVec3d     DwgDbAttribute::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double     DwgDbAttribute::GetThickness () const { return T_Super::thickness(); }
bool       DwgDbAttribute::IsInvisible () const { return T_Super::isInvisible(); }
bool       DwgDbAttribute::IsConstant () const { return T_Super::isConstant(); }
bool       DwgDbAttribute::IsMTextAttribute () const { return T_Super::isMTextAttribute(); }
bool       DwgDbAttribute::IsPreset () const { return T_Super::isPreset(); }
bool       DwgDbAttribute::IsVerifiable () const { return T_Super::isVerifiable(); }
bool       DwgDbAttribute::IsLocked () const { return DWGDB_CALLSDKMETHOD(false,T_Super::isReallyLocked()); }
DwgString  DwgDbAttribute::GetTag () const { return DWGDB_CALLSDKMETHOD(T_Super::tag(),T_Super::tagConst()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgDbAttributeDefinition::GetValueString (DwgStringR value) const
    {
    value.Empty ();
#ifdef DWGTOOLKIT_OpenDwg
    if (T_Super::isMTextAttributeDefinition())
        {
        OdDbMTextPtr    mtext = T_Super::getMTextAttributeDefinition ();
        if (!mtext.isNull())
            {
            mtext->convertFieldToText ();
            value = mtext->contents ();
            }
        }
    else
        {
        value = T_Super::textString ();
        }
#elif DWGTOOLKIT_RealDwg
    if (T_Super::isMTextAttributeDefinition())
        {
        AcDbMText*    mtext = T_Super::getMTextAttributeDefinition ();
        if (nullptr != mtext)
            {
            mtext->convertFieldToText ();

            ACHAR*  rtf = mtext->contentsRTF ();
            if (nullptr != rtf)
                {
                value.assign (rtf);
                ::acutDelString (rtf);
                }
            delete mtext;
            }
        }
    else
        {
        value.assign (T_Super::textStringConst());
        }
#endif
    return  !value.IsEmpty();
    }
DPoint3d   DwgDbAttributeDefinition::GetOrigin () const { return Util::DPoint3dFrom(T_Super::position()); }
DVec3d     DwgDbAttributeDefinition::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double     DwgDbAttributeDefinition::GetThickness () const { return T_Super::thickness(); }
bool       DwgDbAttributeDefinition::IsInvisible () const { return T_Super::isInvisible(); }
bool       DwgDbAttributeDefinition::IsConstant () const { return T_Super::isConstant(); }
bool       DwgDbAttributeDefinition::IsMTextAttributeDefinition () const { return T_Super::isMTextAttributeDefinition(); }
bool       DwgDbAttributeDefinition::IsPreset () const { return T_Super::isPreset(); }
bool       DwgDbAttributeDefinition::IsVerifiable () const { return T_Super::isVerifiable(); }
DwgString  DwgDbAttributeDefinition::GetTag () const { return DWGDB_CALLSDKMETHOD(T_Super::tag(),T_Super::tagConst()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbViewport::GetUcs (DPoint3dR origin, DVec3dR xAxis, DVec3d yAxis) const
    { 
    DWGGE_Type(Point3d)     geOrigin;
    DWGGE_Type(Vector3d)    xDir, yDir;
    DwgDbStatus             status = DwgDbStatus::UnknownError;

#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getUcs (geOrigin, xDir, yDir);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::getUcs(geOrigin, xDir, yDir));
#endif

    if (DwgDbStatus::Success == status)
        {
        origin = Util::DPoint3dFrom (geOrigin);
        xAxis = Util::DVec3dFrom (xDir);
        yAxis = Util::DVec3dFrom (yDir);
        }
    else
        {
        origin.Zero ();
        xAxis.Init (0., 0., 1.0);
        yAxis.Init (0., 0., 1.0);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus    DwgDbViewport::SetCenterPoint (DPoint3dCR center)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setCenterPoint (Util::GePoint3dFrom(center));
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setCenterPoint(Util::GePoint3dFrom(center)));
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus    DwgDbViewport::SetViewCenter (DPoint2dCR center)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setViewCenter (Util::GePoint2dFrom(center));
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setViewCenter(Util::GePoint2dFrom(center)));
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus    DwgDbViewport::SetHeight (double height)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setHeight (height);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setHeight(height));
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus    DwgDbViewport::SetViewHeight (double height)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setViewHeight (height);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setViewHeight(height));
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus    DwgDbViewport::GetFrozenLayers (DwgDbObjectIdArrayR idsOut) const
    {
    DwgDbStatus status = DwgDbStatus::Success;
    DWGDB_Type(ObjectIdArray)   idArray;

#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getFrozenLayerList (idArray);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::getFrozenLayerList(idArray));
#endif

    uint32_t    count = idArray.length ();
    for (uint32_t i = 0; i < count; i++)
        idsOut.push_back (idArray.at(i));

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbViewport::GetAnnotationScale (double& scale) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    OdResult    status = OdResult::eNullObjectPointer;

    OdDbAnnotationScalePtr  annoScale = T_Super::annotationScale ();
    if (!annoScale.isNull())
        status = annoScale->getScale (scale);

#elif DWGTOOLKIT_RealDwg
    Acad::ErrorStatus       status = Acad::eAmbiguousOutput;
    AcDbAnnotationScale*    annoScale = T_Super::annotationScale ();
    if (nullptr != annoScale)
        {
        status = annoScale->getScale (scale);
        delete annoScale;
        }
#endif

    return  ToDwgDbStatus(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus    DwgDbViewport::SetWidth (double width)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setHeight (width);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setWidth(width));
#endif
    return  status;
    }
bool           DwgDbViewport::IsOn () const { return T_Super::isOn(); }
bool           DwgDbViewport::IsGridEnabled () const { return T_Super::isGridOn(); }
bool           DwgDbViewport::IsUcsIconEnabled () const { return T_Super::isUcsIconVisible(); }
bool           DwgDbViewport::IsUcsSavedWithViewport () const { return T_Super::isUcsSavedWithViewport(); }
bool           DwgDbViewport::IsFrontClipEnabled () const { return T_Super::isFrontClipOn(); }
bool           DwgDbViewport::IsFrontClipAtEye () const { return T_Super::isFrontClipAtEyeOn(); }
double         DwgDbViewport::GetFrontClipDistance () const { return T_Super::frontClipDistance(); }
bool           DwgDbViewport::IsBackClipEnabled () const { return T_Super::isBackClipOn(); }
double         DwgDbViewport::GetBackClipDistance () const { return T_Super::backClipDistance(); }
bool           DwgDbViewport::IsPerspectiveEnabled () const { return T_Super::isPerspectiveOn(); }
bool           DwgDbViewport::IsDefaultLightingOn () const { return T_Super::isDefaultLightingOn(); }
bool           DwgDbViewport::IsTransparentOn () const { return T_Super::isTransparent(); }
DwgDbObjectId  DwgDbViewport::GetClipEntity () const { return T_Super::nonRectClipEntityId(); }
DwgDbObjectId  DwgDbViewport::GetBackground () const { return T_Super::background(); }
DwgDbObjectId  DwgDbViewport::GetVisualStyle () const { return T_Super::visualStyle(); }
DVec3d         DwgDbViewport::GetViewDirection () const { return Util::DVec3dFrom(T_Super::viewDirection()); }
double         DwgDbViewport::GetHeight () const { return T_Super::height(); }
double         DwgDbViewport::GetViewHeight () const { return T_Super::viewHeight(); }
double         DwgDbViewport::GetWidth () const { return T_Super::width(); }
double         DwgDbViewport::GetLensLength () const { return T_Super::lensLength(); }
double         DwgDbViewport::GetViewTwist () const { return T_Super::twistAngle(); }
double         DwgDbViewport::GetUcsElevation () const { return T_Super::elevation(); }
DPoint3d       DwgDbViewport::GetCenterPoint () const { return Util::DPoint3dFrom(T_Super::centerPoint()); }
DPoint2d       DwgDbViewport::GetViewCenter () const { return Util::DPoint2dFrom(T_Super::viewCenter()); }
DPoint3d       DwgDbViewport::GetViewTarget () const { return Util::DPoint3dFrom(T_Super::viewTarget()); }
DPoint2d       DwgDbViewport::GetGridIncrements () const { return Util::DPoint2dFrom(T_Super::gridIncrement()); }
DPoint2d       DwgDbViewport::GetSnapIncrements () const { return Util::DPoint2dFrom(T_Super::snapIncrement()); }
DPoint2d       DwgDbViewport::GetSnapBase () const { return Util::DPoint2dFrom(T_Super::snapBasePoint()); }
double         DwgDbViewport::GetSnapAngle () const { return T_Super::snapAngle(); }
SnapIsoPair    DwgDbViewport::GetSnapPair () const { return static_cast<SnapIsoPair>(T_Super::snapIsoPair()); }
bool           DwgDbViewport::IsSnapEnabled () const { return T_Super::isSnapOn(); }
bool           DwgDbViewport::IsIsometricSnapEnabled () const { return T_Super::isSnapIsometric(); }
int16_t        DwgDbViewport::GetGridMajor () const { return gridMajor(); }
bool           DwgDbViewport::IsLayerFrozen (DwgDbObjectIdCR layerId) const { return T_Super::isLayerFrozenInViewport(layerId); }
double         DwgDbViewport::GetCustomScale () const { return T_Super::customScale(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d        DwgDbBlockReference::GetScaleFactors () const
    {
    DWGGE_Type(Scale3d) scale = T_Super::scaleFactors ();
    return DPoint3d::From (scale.sx, scale.sy, scale.sz);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbBlockReference::GetExtentsBestFit (DRange3dR out, TransformCR parentXform) const
    {
    DWGGE_Type(Matrix3d) matrix;
    Util::GetGeMatrix (matrix, parentXform);

    DWGDB_SDKNAME(OdGeExtents3d,AcDbExtents)    extents;

    DwgDbStatus status = ToDwgDbStatus (T_Super::geomExtentsBestFit(extents, matrix));

    if (DwgDbStatus::Success == status)
        out = Util::DRange3dFrom (extents);
    else
        out.Init ();

    return  status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgDbBlockReference::IsXAttachment (WStringP blockName, WStringP path) const
    {
    DwgDbBlockTableRecordPtr    block(T_Super::blockTableRecord(), DwgDbOpenMode::ForRead);
    if (!block.IsNull() && block->isFromExternalReference())
        {
        if (nullptr != blockName)
            blockName->assign (block->GetName ());
        if (nullptr != path)
            path->assign (block->GetPath ());
        return  true;
        }
    return  false;
    }

DwgDbObjectId   DwgDbBlockReference::GetBlockTableRecordId () const { return T_Super::blockTableRecord(); }
DPoint3d        DwgDbBlockReference::GetPosition () const { return Util::DPoint3dFrom(T_Super::position()); }
void            DwgDbBlockReference::GetBlockTransform (TransformR out) const { return Util::GetTransform(out, T_Super::blockTransform()); }
DVec3d          DwgDbBlockReference::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
DwgDbStatus     DwgDbBlockReference::ExplodeToOwnerSpace () const { return ToDwgDbStatus(T_Super::explodeToOwnerSpace()); }
DwgDbObjectIterator DwgDbBlockReference::GetAttributeIterator () const { return DwgDbObjectIterator(T_Super::attributeIterator()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbBlockReference::OpenSpatialFilter (DwgDbSpatialFilterPtr& filterOut, DwgDbOpenMode mode) const
    {
    DwgDbStatus status = DwgDbStatus::ObjectNotOpenYet;

#ifdef DWGTOOLKIT_OpenDwg
    OdDbFilterPtr   odFilter = OdDbIndexFilterManager::getFilter (this, OdDbSpatialFilter::desc(), FromDwgDbOpenMode(mode));
    if (odFilter.isNull() || !(filterOut = DwgDbSpatialFilter::Cast(odFilter.get())).isNull())
        status = DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg
    AcDbFilter*         acFilter = nullptr;
    Acad::ErrorStatus   es = AcDbIndexFilterManager::getFilter (this, AcDbSpatialFilter::desc(), FromDwgDbOpenMode(mode), acFilter);
    if (Acad::eOk == es && nullptr != acFilter)
        {
        DwgDbSpatialFilterP spatial = DwgDbSpatialFilter::Cast (acFilter);
        if (nullptr == spatial || Acad::eOk != (es = filterOut.acquire(spatial)))
            {
            acFilter->close ();
            if (Acad::eOk == es)
                es = Acad::eNullObjectPointer;
            }
        }
    status = ToDwgDbStatus (es);
#endif  // DWGTOOLKIT_

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          DwgDbHatch::GetGradientColors (DwgColorArrayR colorsOut, DwgDbDoubleArrayR valuesOut)
    {
    uint32_t    count = 0;

#ifdef DWGTOOLKIT_OpenDwg
    OdCmColorArray      colors;
    OdGeDoubleArray     values;

    T_Super::getGradientColors (colors, values);
    count = static_cast<uint32_t> (colors.size());

#elif DWGTOOLKIT_RealDwg
    AcCmColor*  colors = nullptr;
    float*      values = nullptr;
    
    if (Acad::eOk != T_Super::getGradientColors(count, colors, values))
        return  0;
#endif  // DWGTOOLKIT_

    for (uint32_t i = 0; i < count; i++)
        {
        colorsOut.push_back (static_cast<DwgCmColorCR>(colors[i]));
        valuesOut.push_back (static_cast<double>(values[i]));
        }

    return  colorsOut.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          DwgDbHatch::GetPatternDefinitions (DwgPatternArrayR patternsOut) const
    {
    for (int i = 0; i < T_Super::numPatternDefinitions(); i++)
        {
        double  angle = 0.0, baseX = 0.0, baseY = 0.0, offsetX = 0.0, offsetY = 0.0;
        DWGGE_Type(DoubleArray) dashes;

        T_Super::getPatternDefinitionAt(i, angle, baseX, baseY, offsetX, offsetY, dashes);

        patternsOut.push_back (DwgGiHatchPatternDefinition(angle, baseX, baseY, offsetX, offsetY, dashes));
        }
    return  patternsOut.size();
    }

DwgDbHatch::PatternType DwgDbHatch::GetPatternType () const { return DWGDB_UPWARDCAST(Hatch::PatternType)(T_Super::patternType()); }
DwgDbHatch::HatchType   DwgDbHatch::GetHatchType () const { return DWGDB_UPWARDCAST(Hatch::HatchType)(T_Super::hatchObjectType()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbHatch::SetPattern (DwgDbHatch::PatternType type, DwgStringCR name)
    {
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setPattern (static_cast<OdDbHatch::HatchPatternType>(type), static_cast<const OdString>(name));
    return  DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg
    Acad::ErrorStatus   es = T_Super::setPattern (static_cast<AcDbHatch::HatchPatternType>(type), static_cast<const ACHAR*>(name.c_str()));
    return ToDwgDbStatus(es);
#endif  // DWGTOOLKIT_
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbHatch::SetHatchType (DwgDbHatch::HatchType type)
    {
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setHatchObjectType (static_cast<OdDbHatch::HatchObjectType>(type));
    return  DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg
    Acad::ErrorStatus   es = T_Super::setHatchObjectType (static_cast<AcDbHatch::HatchObjectType>(type));
    return ToDwgDbStatus(es);
#endif  // DWGTOOLKIT_
    }

bool            DwgDbHatch::IsGradient () const { return T_Super::isGradient(); }
double          DwgDbHatch::GetGradientAngle () const { return T_Super::gradientAngle(); }
double          DwgDbHatch::GetGradientShift () const { return T_Super::gradientShift(); }
double          DwgDbHatch::GetGradientTint () const { return T_Super::getShadeTintValue(); }
DwgString       DwgDbHatch::GetGradientName () const { return T_Super::gradientName(); }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbRasterImage::GetFileName (DwgStringR name, DwgStringP activeFile) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    OdDbRasterImageDefPtr   def = T_Super::imageDefId().safeOpenObject ();
    if (def.isNull())
        return  DwgDbStatus::ObjectNotOpenYet;

#elif DWGTOOLKIT_RealDwg
    AcDbSmartObjectPointer<AcDbRasterImageDef>  def (T_Super::imageDefId(), AcDb::kForRead);
    if (Acad::eOk != def.openStatus())
        return  ToDwgDbStatus(def.openStatus());

#endif  // DWGTOOLKIT_

    // when AcDbRasterImageDef is not initialized, we'd get a proxy:
    if (def->isAProxy())
        {
        BeAssert (false && "AcDbRasterImageDef is a proxy - the class may not have been registered!");
        return  DwgDbStatus::InvalidData;
        }

    // load the raster image file
    if (!def->isLoaded())
        {
        DwgDbStatus status = DwgDbStatus::Success;
        if (!def->isWriteEnabled())
            {
#ifdef DWGTOOLKIT_RealDwg
            status = (DwgDbStatus)
#endif
            def->upgradeOpen ();
            }
        
        if (DwgDbStatus::Success != status || DwgDbStatus::Success != (status = ToDwgDbStatus(def->load())))
            BeDataAssert (false && "Unable to load raster image file!!");
        }

    name = def->sourceFileName ();
    if (nullptr != activeFile)
        *activeFile = def->activeFileName ();

    return  name.IsEmpty() ? DwgDbStatus::UnknownError : DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgDbRasterImage::GetOrientation (DPoint3dR origin, DVec3dR xAxis, DVec3dR yAxis) const
    {
    DWGGE_Type(Point3d) o;
    DWGGE_Type(Vector3d) u, v;
    T_Super::getOrientation (o, u, v);

    origin = Util::DPoint3dFrom (o);
    xAxis = Util::DVec3dFrom (u);
    yAxis = Util::DVec3dFrom (v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgDbRasterImage::GetOrientation (TransformR orientation, bool pixelsToModel) const
    {
    DWGGE_Type(Point3d) o;
    DWGGE_Type(Vector3d) u, v;
    T_Super::getOrientation (o, u, v);

    DWGGE_Type(Vector2d)    imageSize = T_Super::imageSize ();
    if (pixelsToModel)
        {
        if (fabs(imageSize.x) > 1.0e-8)
            u /= imageSize.x;
        if (fabs(imageSize.y) > 1.0e-8)
            v /= imageSize.y;
        }
    DWGGE_Type(Vector3d)    n = u.crossProduct (v);
    n.normalize ();

    orientation = Transform::FromOriginAndVectors (Util::DPoint3dFrom(o), Util::DVec3dFrom(u), Util::DVec3dFrom(v),  Util::DVec3dFrom(n));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbRasterImage::GetModelTransform (TransformR pixelToModel) const
    {
    DWGGE_Type(Matrix3d)    toModel;
    DwgDbStatus             status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    toModel = T_Super::getPixelToModelTransform ();
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus(T_Super::getPixelToModelTransform(toModel));
#endif
    if (DwgDbStatus::Success == status)
        Util::GetTransform (pixelToModel, toModel);
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          DwgDbRasterImage::GetClippingBoundary (DPoint3dArrayR out) const
    {
    if (!T_Super::isClipped())
        return  0;

    // will transform clipping points to model coordinate system
    DWGGE_Type(Matrix3d)    toModel;
#ifdef DWGTOOLKIT_OpenDwg
    toModel = T_Super::getPixelToModelTransform ();
#elif DWGTOOLKIT_RealDwg
    if (DwgDbStatus::Success != ToDwgDbStatus(T_Super::getPixelToModelTransform(toModel)))
        return  0;
#endif

    DWGGE_TypeCR(Point2dArray)  points = T_Super::clipBoundary ();
    for (uint32_t i = 0; i < (uint32_t)points.length(); i++)
        {
        DWGGE_Type(Point3d)     point (points[i].x, points[i].y, 0.0);

        point.transformBy (toModel);
        out.push_back (Util::DPoint3dFrom(point));
        }    

    return  out.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbRasterImage::GetVertices (DPoint3dArrayR verticesOut) const
    {
    DWGGE_Type(Point3dArray)    points;
    DwgDbStatus                 status = DwgDbStatus::Success;

#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getVertices (points);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::getVertices(points));
#endif  // DWGTOOLKIT_

    if (DwgDbStatus::Success == status)
        {
        for (uint32_t i = 0; i < (uint32_t)points.length(); i++)
            verticesOut.push_back (Util::DPoint3dFrom(points[i]));
        }
    return  status;
    }
DVec2d          DwgDbRasterImage::GetImageSize (bool cachedValue) const { return Util::DVec2dFrom(T_Super::imageSize(cachedValue)); }
DVec2d          DwgDbRasterImage::GetScale () const { return Util::DVec2dFrom(T_Super::scale()); }
bool            DwgDbRasterImage::IsShownClipped () const { return DWGDB_CALLSDKMETHOD(true,T_Super::isShownClipped()); }
bool            DwgDbRasterImage::IsClipInverted () const { return T_Super::isClipInverted(); }
bool            DwgDbRasterImage::IsClipped () const { return T_Super::isClipped(); }
bool            DwgDbRasterImage::IsDisplayed () const { return DWGDB_CALLSDKMETHOD(T_Super::isSetDisplayOpt(ImageDisplayOpt::kShow),T_Super::isImageShown()); }
size_t          DwgDbRasterImage::GetClippingBoundary (DPoint2dArrayR out) const { return Util::GetPointArray(out, T_Super::clipBoundary()); }
DwgDbObjectId   DwgDbRasterImage::GetImageDefinitionId () const { return T_Super::imageDefId(); }
DwgDbRasterImage::ClipType  DwgDbRasterImage::GetClipBoundaryType () const { return DWGDB_UPWARDCAST(RasterImage::ClipType)(T_Super::clipBoundaryType()); }

#ifdef DWGTOOLKIT_RealDwg
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
IAcDbPointCloudPointProcessor::ProcessSate  IPointsProcessor::process (const IAcDbPointCloudDataBuffer* buffer)
    {
    PointCloudDataQueryP    dataQuery = new PointCloudDataQuery (buffer);
    if (nullptr == dataQuery)
        return  ProcessSate::Abort;

    IPointsProcessor::Status status = this->_Process (dataQuery);
    delete dataQuery;

    return static_cast<IAcDbPointCloudPointProcessor::ProcessSate>(status);
    }
#endif

size_t          PointCloudDataQuery::GetNumPoints() const { return DWGDB_CALLSDKMETHOD(0,nullptr==m_pointCloudDataBuffer ? 0 : static_cast<size_t>(m_pointCloudDataBuffer->numPoints())); }
double const*   PointCloudDataQuery::GetPointsAsDoubles() const { return DWGDB_CALLSDKMETHOD(nullptr,nullptr==m_pointCloudDataBuffer ? nullptr : reinterpret_cast<double const*>(m_pointCloudDataBuffer->points())); }
double const*   PointCloudDataQuery::GetNormalsAsDoubles() const { return DWGDB_CALLSDKMETHOD(nullptr,nullptr==m_pointCloudDataBuffer ? nullptr : reinterpret_cast<double const*>(m_pointCloudDataBuffer->normals())); }
uint8_t const*  PointCloudDataQuery::GetIntensity() const { return DWGDB_CALLSDKMETHOD(nullptr,nullptr==m_pointCloudDataBuffer ? nullptr : static_cast<uint8_t const*>(m_pointCloudDataBuffer->intensity())); }
uint8_t const*  PointCloudDataQuery::GetClassifications() const { return DWGDB_CALLSDKMETHOD(nullptr,nullptr==m_pointCloudDataBuffer ? nullptr : static_cast<uint8_t const*>(m_pointCloudDataBuffer->classifications())); }
PointCloudDataQuery::Rgba const*   PointCloudDataQuery::GetColors() const { return DWGDB_CALLSDKMETHOD(nullptr,nullptr==m_pointCloudDataBuffer ? nullptr : static_cast<PointCloudDataQuery::Rgba const*>(m_pointCloudDataBuffer->colors())); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            PointCloudDataQuery::GetTransform(TransformR out) const
    {
#ifdef DWGTOOLKIT_RealDwg
    if (nullptr != m_pointCloudDataBuffer)
        Util::GetTransform (out, m_pointCloudDataBuffer->transform());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbPointCloudEx::TraversePointData (IPointsProcessor* processor, IPointsFilter* filter, PointCloudDataQuery::Type type, int lod) const
    {
#ifdef DWGTOOLKIT_RealDwg
    IAcDbPointCloudPointProcessor*      pp = dynamic_cast<IAcDbPointCloudPointProcessor*> (processor);
    IAcDbPointCloudSpatialFilter*       pf = dynamic_cast<IAcDbPointCloudSpatialFilter*> (filter);
    IAcDbPointCloudDataBuffer::DataType dt = static_cast<IAcDbPointCloudDataBuffer::DataType> (type);

    Acad::ErrorStatus   es = T_Super::traverseAllPointData (pp, pf, dt, lod);

    return  ToDwgDbStatus(es);
#else

    return  DwgDbStatus::NotSupported;
#endif  // DWGTOOLKIT_
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgDbPointCloudEx::GetEcs (TransformR out) const
    {
    DWGGE_Type(Matrix3d)    ecs;
#ifdef DWGTOOLKIT_OpenDwg
    ecs = T_Super::getEcs ();
#elif DWGTOOLKIT_RealDwg
    T_Super::getEcs (ecs);
#endif
    Util::GetTransform (out, ecs);
    }
DPoint3d        DwgDbPointCloudEx::GetLocation () const { return Util::DPoint3dFrom(T_Super::location()); }
double          DwgDbPointCloudEx::GetScale () const { return T_Super::scale(); }
double          DwgDbPointCloudEx::GetRotation () const { return T_Super::rotation(); }
DwgDbStatus     DwgDbPointCloudEx::GetMinDistPrecision (double& prec) const { return DWGDB_CALLSDKMETHOD(DwgDbStatus::NotSupported,ToDwgDbStatus(T_Super::getMinDistPrecision(prec))); }
bool            DwgDbPointCloudEx::IsSupported () const { return DWGDB_CALLSDKMETHOD(false,true); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgDbPointCloudEx::GetNativeCloudExtent (DRange3dR extent) const
    {
    DWGDB_SDKNAME(OdGeExtents3d,AcDbExtents) ext;
    T_Super::getNativeCloudExtent (ext);
    extent = Util::DRange3dFrom (ext);
    }

DPoint3d        DwgDbPoint::GetPosition () const { return Util::DPoint3dFrom(T_Super::position()); }
DVec3d          DwgDbPoint::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double          DwgDbPoint::GetThickness () const { return T_Super::thickness(); }

DVec3d          DwgDbSolid::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double          DwgDbSolid::GetThickness () const { return T_Super::thickness(); }

DVec3d          DwgDbShape::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double          DwgDbShape::GetThickness () const { return T_Super::thickness(); }

DVec3d          DwgDbTrace::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double          DwgDbTrace::GetThickness () const { return T_Super::thickness(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbFace::GetTransform (TransformR transform) const
    {
    DWGGE_Type(Plane)   plane;
    DWGDB_SDKENUM_DB(Planarity) planarity;
    
    DwgDbStatus status = ToDwgDbStatus(T_Super::getPlane(plane, planarity));

    if (status == DwgDbStatus::Success)
        {
        DWGGE_Type(Point3d)     origin;
        DWGGE_Type(Vector3d)    xAxis, yAxis, zAxis;

        plane.getCoordSystem (origin, xAxis, yAxis);
        zAxis = plane.normal ();

        transform.InitFromOriginAndVectors (Util::DPoint3dFrom(origin), Util::DVec3dFrom(xAxis), Util::DVec3dFrom(yAxis), Util::DVec3dFrom(zAxis));
        }
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbFace::GetVertexAt (DPoint3dR point, uint16_t index) const
    {
    if (index > 3)
        return  DwgDbStatus::InvalidInput;

    DWGGE_Type(Point3d) gePoint;
    DwgDbStatus status = DwgDbStatus::Success;

#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getVertexAt (index, gePoint);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::getVertexAt(index, gePoint));
#endif

    if (status == DwgDbStatus::Success)
        point = Util::DPoint3dFrom (gePoint);
    return  status;
    }
bool            DwgDbFace::IsPlanar () const { return   T_Super::isPlanar(); }

DPoint3d        DwgDbText::GetPosition () const { return Util::DPoint3dFrom(T_Super::position()); }
DVec3d          DwgDbText::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double          DwgDbText::GetThickness () const { return T_Super::thickness(); }
DwgString       DwgDbText::GetTextString () const { return DWGDB_CALLSDKMETHOD(T_Super::textString(),T_Super::textStringConst()); }
