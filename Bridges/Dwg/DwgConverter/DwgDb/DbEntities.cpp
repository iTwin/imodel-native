/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    "DwgDbInternal.h"

USING_NAMESPACE_DWGDB

// a convenient macro to call a method returned void from OpenDWG and Acad::ErrorStatus from RealDWG:
#ifdef DWGTOOLKIT_OpenDwg
#define RETURNVOIDORSTATUS(_method_)    { ##_method_##; return DwgDbStatus::Success; }
#elif DWGTOOLKIT_RealDwg
#define RETURNVOIDORSTATUS(_method_)    { return ToDwgDbStatus(##_method_##); }
#endif


// Add entities that are sub-classed from toolkit's entity classes
DWGDB_ENTITY_DEFINE_BASEMEMBERS(Entity)
DWGDB_ENTITY_DEFINE_MEMBERS(Line)
DWGDB_ENTITY_DEFINE_MEMBERS(Point)
DWGDB_ENTITY_DEFINE_MEMBERS(Arc)
DWGDB_ENTITY_DEFINE_MEMBERS(Circle)
DWGDB_ENTITY_DEFINE_MEMBERS(Ellipse)
DWGDB_ENTITY_DEFINE_MEMBERS(Face)
DWGDB_ENTITY_DEFINE_MEMBERS(FaceRecord)
DWGDB_ENTITY_DEFINE_MEMBERS(Polyline)
DWGDB_ENTITY_DEFINE_MEMBERS(2dPolyline)
DWGDB_ENTITY_DEFINE_MEMBERS(3dPolyline)
DWGDB_ENTITY_DEFINE_MEMBERS(PolyFaceMesh)
DWGDB_ENTITY_DEFINE_MEMBERS(PolyFaceMeshVertex)
DWGDB_ENTITY_DEFINE_MEMBERS(PolygonMesh)
DWGDB_ENTITY_DEFINE_MEMBERS(PolygonMeshVertex)
DWGDB_ENTITY_DEFINE_MEMBERS(Hatch)
DWGDB_ENTITY_DEFINE_MEMBERS(Light)
DWGDB_ENTITY_DEFINE_MEMBERS(Region)
DWGDB_ENTITY_DEFINE_MEMBERS(3dSolid)
DWGDB_ENTITY_DEFINE_MEMBERS(Body)
DWGDB_ENTITY_DEFINE_MEMBERS(Solid)
DWGDB_ENTITY_DEFINE_MEMBERS(Shape)
DWGDB_ENTITY_DEFINE_MEMBERS(Spline)
DWGDB_ENTITY_DEFINE_MEMBERS(Trace)
DWGDB_ENTITY_DEFINE_MEMBERS(Text)
DWGDB_ENTITY_DEFINE_MEMBERS(MText)
DWGDB_ENTITY_DEFINE_MEMBERS(Attribute)
DWGDB_ENTITY_DEFINE_MEMBERS(AttributeDefinition)
DWGDB_ENTITY_DEFINE_MEMBERS(BlockReference)
DWGDB_ENTITY_DEFINE_MEMBERS(ViewRepBlockReference)
DWGDB_ENTITY_DEFINE_MEMBERS(RasterImage)
DWGDB_ENTITY_DEFINE_MEMBERS(PointCloudEx)
DWGDB_ENTITY_DEFINE_MEMBERS(Viewport)
DWGDB_ENTITY_DEFINE_MEMBERS(ViewBorder)
DWGDB_SURFACE_DEFINE_MEMBERS(Surface)
DWGDB_SURFACE_DEFINE_MEMBERS(ExtrudedSurface)
DWGDB_SURFACE_DEFINE_MEMBERS(LoftedSurface)
DWGDB_SURFACE_DEFINE_MEMBERS(NurbSurface)
DWGDB_SURFACE_DEFINE_MEMBERS(PlaneSurface)
DWGDB_SURFACE_DEFINE_MEMBERS(RevolvedSurface)
DWGDB_SURFACE_DEFINE_MEMBERS(SweptSurface)



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/20
+---------------+---------------+---------------+---------------+---------------+------*/
DwgString   DwgDbEntity::GetOriginalClassName () const
    {
    if (T_Super::isAProxy())
        {
        DWGDB_TypeCP(ProxyEntity) proxyEntity = DWGDB_Type(ProxyEntity)::cast (this);
        if (proxyEntity != nullptr)
            return  proxyEntity->originalClassName();
        }
    return T_Super::isA()->name ();
    }
bool    DwgDbEntity::IsDimension () const { return nullptr != DWGDB_Type(Dimension)::cast(this); }

DPoint3d    DwgDbLine::GetStartPoint () const { return Util::DPoint3dFrom(T_Super::startPoint()); }
DPoint3d    DwgDbLine::GetEndPoint () const { return Util::DPoint3dFrom(T_Super::endPoint()); }
DVec3d      DwgDbLine::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double      DwgDbLine::GetThickness () const { return T_Super::thickness(); }
DwgDbStatus DwgDbLine::SetStartPoint (DPoint3dCR p) { RETURNVOIDORSTATUS(T_Super::setStartPoint(Util::GePoint3dFrom(p))); }
DwgDbStatus DwgDbLine::SetEndPoint (DPoint3dCR p) { RETURNVOIDORSTATUS(T_Super::setEndPoint(Util::GePoint3dFrom(p))); }
DwgDbStatus DwgDbLine::SetNormal (DVec3dCR v) { RETURNVOIDORSTATUS(T_Super::setNormal(Util::GeVector3dFrom(v))); }
DwgDbStatus DwgDbLine::SetThickness (double t) { RETURNVOIDORSTATUS(T_Super::setThickness(t)); }

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
DwgDbStatus DwgDbPolyline::ConvertFrom (DwgDbEntityP& source, bool handIdOver)
    {
#ifdef DWGTOOLKIT_OpenDwg
    return ToDwgDbStatus(T_Super::convertFrom(OdDbEntity::cast(source), handIdOver));
#elif DWGTOOLKIT_RealDwg
    AcDbEntity* acEntity = AcDbEntity::cast (source);
    Acad::ErrorStatus   es = T_Super::convertFrom (acEntity, handIdOver);
    source = DwgDbEntity::Cast (acEntity);
    return  ToDwgDbStatus(es);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbPolyline::ConvertTo (DwgDb2dPolylineP& dest, bool handIdOver)
    {
#ifdef DWGTOOLKIT_OpenDwg
     return ToDwgDbStatus(T_Super::convertTo(OdDb2dPolyline::cast(dest), handIdOver));
#elif DWGTOOLKIT_RealDwg
    AcDb2dPolyline* acPline = AcDb2dPolyline::cast (dest);
    Acad::ErrorStatus   es = T_Super::convertTo (acPline, handIdOver);
    dest = DwgDb2dPolyline::Cast (acPline);
    return  ToDwgDbStatus(es);
#endif
    }

bool       DwgDbPolyline::IsClosed () const { return DWGDB_IsTrue(T_Super::isClosed()); }
size_t     DwgDbPolyline::GetNumPoints () const { return T_Super::numVerts(); }
bool       DwgDbPolyline::HasWidth () const { return DWGDB_IsTrue(T_Super::hasWidth()); }
bool       DwgDbPolyline::HasBulges () const { return DWGDB_IsTrue(T_Super::hasBulges()); }
double     DwgDbPolyline::GetElevation () const { return T_Super::elevation(); }
double     DwgDbPolyline::GetThickness () const { return T_Super::thickness(); }
bool       DwgDbPolyline::HasPlinegen () const { return DWGDB_IsTrue(T_Super::hasPlinegen()); }
DVec3d     DwgDbPolyline::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
DwgDbStatus DwgDbPolyline::SetPointAt(size_t i, DPoint2dCR p) { RETURNVOIDORSTATUS(T_Super::setPointAt((unsigned int)i, Util::GePoint2dFrom(p))); }
DwgDbStatus DwgDbPolyline::AddVertexAt(size_t i, DPoint2dCR p, double b, double w0, double w1, uint32_t v) { RETURNVOIDORSTATUS(T_Super::addVertexAt((unsigned int)i, Util::GePoint2dFrom(p), b, w0, w1, v)); }
DwgDbStatus DwgDbPolyline::SetWidthsAt(size_t i, double s, double e) { RETURNVOIDORSTATUS(T_Super::setWidthsAt((unsigned int)i, s, e)); }
DwgDbStatus DwgDbPolyline::SetWidthsAt(size_t i, DPoint2dCR p) { RETURNVOIDORSTATUS(T_Super::setWidthsAt((unsigned int)i, p.x, p.y)); }
DwgDbStatus DwgDbPolyline::SetConstantWidth(double w) { RETURNVOIDORSTATUS(T_Super::setConstantWidth(w)); }
DwgDbStatus DwgDbPolyline::SetBulgeAt(size_t i, double b) { RETURNVOIDORSTATUS(T_Super::setBulgeAt((unsigned int)i, b)); }
DwgDbStatus DwgDbPolyline::SetThickness(double t) { RETURNVOIDORSTATUS(T_Super::setThickness(t)); }
void DwgDbPolyline::SetClosed (bool b) { T_Super::setClosed(b); }
void DwgDbPolyline::SetElevation (double e) { T_Super::setElevation(e); }
void DwgDbPolyline::SetPlinegen (bool b) { T_Super::setPlinegen(b); }
void DwgDbPolyline::Reset (bool reuse, size_t n) { T_Super::reset(reuse, static_cast<unsigned int>(n)); }

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
* @bsimethod                                                    Don.Fu          10/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDb2dPolyline::GetPoints (DPoint3dArrayR out) const
    {
    out.clear ();
    auto iter = this->GetVertexIterator ();
    if (!iter.IsValid() || !iter->IsValid())
        return  DwgDbStatus::MemoryError;

    for (iter->Start(); !iter->Done(); iter->Next())
        {
#ifdef DWGTOOLKIT_OpenDwg
        OdDb2dVertexPtr vertex = iter->GetObjectId().openObject (OdDb::kForRead);
        if (!vertex.isNull())
#elif DWGTOOLKIT_RealDwg
        AcDbSmartObjectPointer<AcDb2dVertex>    vertex(iter->GetObjectId(), AcDb::kForRead);
        if (vertex.openStatus() == Acad::eOk)
#endif
            {
            // a database resident vertex
            out.push_back (Util::DPoint3dFrom(vertex->position()));
            }
        else
            {
            // a non-db resident vertex
            auto newVertex = DWGDB_Type(2dVertex)::cast (iter->GetEntity());
            if (newVertex != nullptr)
                out.push_back (Util::DPoint3dFrom(newVertex->position()));
            }
        }
    return  out.empty() ? DwgDbStatus::UnknownError : DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDb2dPolyline::SetVertex (DWGDB_TypeP(2dVertex) vertex, DPoint2dCR point, DPoint2dCR widths)
    {
    if (nullptr == vertex)
        return  DwgDbStatus::MemoryError;
    vertex->copyFrom (this);
    DWGGE_Type(Point3d) point3d(point.x, point.y, T_Super::elevation());
    vertex->setPosition (point3d);   
    if (widths.x > 1.e-4)
        vertex->setStartWidth (widths.x);
    if (widths.y > 1.e-4)
        vertex->setStartWidth (widths.y);
    return  DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDb2dPolyline::AppendVertex (DwgDbObjectIdR outId, DPoint2dCR point, DPoint2dCR widths)
    {
    DwgDbStatus status;
#ifdef DWGTOOLKIT_OpenDwg
    OdDb2dVertexPtr newVertex = OdDb2dVertex::createObject ();
    status = this->SetVertex (newVertex, point, widths);
    if (status == DwgDbStatus::Success)
        {
        // Map OdDb::Poly2dType to OdDb::Vertex2dType - NEEDSREVIEW: remove this if Teigha handles it!
        OdDb::Vertex2dType  vertexType;
        switch (T_Super::polyType())
            {
            case OdDb::k2dFitCurvePoly:     
                vertexType = OdDb::k2dSplineFitVertex;
                break;
            case OdDb::k2dQuadSplinePoly:
            case OdDb::k2dCubicSplinePoly:
                vertexType = OdDb::k2dSplineCtlVertex;
                break;
            case OdDb::k2dSimplePoly:
            default:
                vertexType = OdDb::k2dVertex;
                break;
            }
        newVertex->setVertexType (vertexType);
        outId = T_Super::appendVertex (newVertex);
        return  outId.IsValid() ? DwgDbStatus::Success : DwgDbStatus::UnknownError;
        }
#elif DWGTOOLKIT_RealDwg
    AcDb2dVertex*   newVertex = new AcDb2dVertex ();
    status = this->SetVertex (newVertex, point, widths);
    if (status == DwgDbStatus::Success)
        status = ToDwgDbStatus (T_Super::appendVertex(outId, newVertex));
    if (status != DwgDbStatus::Success && nullptr != newVertex && newVertex->isNewObject())
        delete newVertex;
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDb2dPolyline::InsertVertexAt (DwgDbObjectIdR outId, DwgDbObjectIdCR atVertex, DPoint2dCR point, DPoint2dCR widths)
    {
    DwgDbStatus status;
#ifdef DWGTOOLKIT_OpenDwg
    OdDb2dVertexPtr newVertex = OdDb2dVertex::createObject ();
    status = this->SetVertex (newVertex, point, widths);
    if (status == DwgDbStatus::Success)
        {
        outId = T_Super::insertVertexAt (atVertex, newVertex);
        return  outId.IsValid() ? DwgDbStatus::Success : DwgDbStatus::UnknownError;
        }
#elif DWGTOOLKIT_RealDwg
    AcDb2dVertex*   newVertex = new AcDb2dVertex ();
    status = this->SetVertex (newVertex, point, widths);
    if (status == DwgDbStatus::Success)
        status = ToDwgDbStatus (T_Super::insertVertexAt(outId, atVertex, newVertex));
    if (status != DwgDbStatus::Success && nullptr != newVertex && newVertex->isNewObject())
        delete newVertex;
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDb2dPolyline::SetClosed (bool isClosed)
    {
#ifdef DWGTOOLKIT_OpenDwg
    if (isClosed)
        T_Super::makeClosed ();
    else
        T_Super::makeOpen ();
    return  DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
    return  ToDwgDbStatus(T_Super::setClosed(isClosed));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDb2dPolyline::SetConstantWidth (double width)
    {
#ifdef DWGTOOLKIT_OpenDwg
    BeAssert (false && "Teigha does not have OdDb2dPolyline::setConstantWidth!");
    return  DwgDbStatus::NotSupported;
#elif DWGTOOLKIT_RealDwg
    RETURNVOIDORSTATUS(T_Super::setConstantWidth(width));
#endif
    }

bool       DwgDb2dPolyline::IsClosed () const { return DWGDB_IsTrue(T_Super::isClosed()); }
double     DwgDb2dPolyline::GetElevation () const { return T_Super::elevation(); }
double     DwgDb2dPolyline::GetThickness () const { return T_Super::thickness(); }
bool       DwgDb2dPolyline::HasPlinegen () const { return DWGDB_IsTrue(T_Super::isLinetypeGenerationOn()); }
DVec3d     DwgDb2dPolyline::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
DwgDb2dPolyline::Type   DwgDb2dPolyline::GetType () const { return DWGDB_CASTFROMENUM_DB(2dPolyline::Type)(T_Super::polyType()); }
DwgDbObjectIteratorPtr  DwgDb2dPolyline::GetVertexIterator () const { return new DwgDbObjectIterator(T_Super::vertexIterator()); }
DwgDbStatus DwgDb2dPolyline::ConvertToType (Type t) { return ToDwgDbStatus(T_Super::convertToPolyType(DWGDB_CASTTOENUM_DB(Poly2dType)(t))); }
DwgDbStatus DwgDb2dPolyline::MakeClosed () { RETURNVOIDORSTATUS(T_Super::makeClosed()); }
DwgDbStatus DwgDb2dPolyline::MakeOpen () { RETURNVOIDORSTATUS(T_Super::makeOpen()); }
DwgDbStatus DwgDb2dPolyline::SetNormal (DVec3dCR v) { RETURNVOIDORSTATUS(T_Super::setNormal(Util::GeVector3dFrom(v))); }
DwgDbStatus DwgDb2dPolyline::SetElevation (double z) { RETURNVOIDORSTATUS(T_Super::setElevation(z)); }
DwgDbStatus DwgDb2dPolyline::SetThickness (double t) { RETURNVOIDORSTATUS(T_Super::setThickness(t)); }
DwgDbStatus DwgDb2dPolyline::SetLinetypeGeneration (bool onOff) { if (onOff) RETURNVOIDORSTATUS(T_Super::setLinetypeGenerationOn()) else RETURNVOIDORSTATUS(T_Super::setLinetypeGenerationOff()); }
DwgDbStatus DwgDb2dPolyline::SetPolylineType (Type t) { RETURNVOIDORSTATUS(T_Super::setPolyType(DWGDB_CASTTOENUM_DB(Poly2dType)(t))); }
DwgDbStatus DwgDb2dPolyline::SplineFit () { RETURNVOIDORSTATUS(T_Super::splineFit()); }
DwgDbStatus DwgDb2dPolyline::SplineFit (Type t, uint16_t n) {  RETURNVOIDORSTATUS(T_Super::splineFit(DWGDB_CASTTOENUM_DB(Poly2dType)(t), n)); }
DwgDbStatus DwgDb2dPolyline::Straighten () { RETURNVOIDORSTATUS(T_Super::straighten()); }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDb3dPolyline::SetClosed (bool isClosed)
    {
#ifdef DWGTOOLKIT_OpenDwg
    if (isClosed)
        T_Super::makeClosed ();
    else
        T_Super::makeOpen ();
    return  DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
    return  ToDwgDbStatus(T_Super::setClosed(isClosed));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDb3dPolyline::SetVertex (DWGDB_TypeP(3dPolylineVertex) vertex, DPoint3dCR point)
    {
    if (nullptr == vertex)
        return  DwgDbStatus::MemoryError;
    vertex->copyFrom (this);
    vertex->setPosition (Util::GePoint3dFrom(point));
    return  DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDb3dPolyline::AppendVertex (DwgDbObjectIdR outId, DPoint3dCR point)
    {
    DwgDbStatus status;
#ifdef DWGTOOLKIT_OpenDwg
    OdDb3dPolylineVertexPtr newVertex = OdDb3dPolylineVertex::createObject ();
    status = this->SetVertex (newVertex, point);
    if (status == DwgDbStatus::Success)
        {
        // Map OdDb::Poly3dType to OdDb::Vertex3dType - NEEDSREVIEW: remove this if Teigha handles it!
        OdDb::Vertex3dType  vertexType;
        switch (T_Super::polyType())
            {
            case OdDb::k3dQuadSplinePoly:
            case OdDb::k3dCubicSplinePoly:
                vertexType = OdDb::k3dControlVertex;
                break;
            case OdDb::k2dSimplePoly:
            default:
                vertexType = OdDb::k3dSimpleVertex;
                break;
            }
        newVertex->setVertexType (vertexType);
        outId = T_Super::appendVertex (newVertex);
        return  outId.IsValid() ? DwgDbStatus::Success : DwgDbStatus::UnknownError;
        }
#elif DWGTOOLKIT_RealDwg
    AcDb3dPolylineVertex*   newVertex = new AcDb3dPolylineVertex ();
    status = this->SetVertex (newVertex, point);
    if (status == DwgDbStatus::Success)
        status = ToDwgDbStatus (T_Super::appendVertex(outId, newVertex));
    if (status != DwgDbStatus::Success && nullptr != newVertex && newVertex->isNewObject())
        delete newVertex;
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDb3dPolyline::InsertVertexAt (DwgDbObjectIdR outId, DwgDbObjectIdCR atVertex, DPoint3dCR point)
    {
    DwgDbStatus status;
#ifdef DWGTOOLKIT_OpenDwg
    OdDb3dPolylineVertexPtr newVertex = OdDb3dPolylineVertex::createObject ();
    status = this->SetVertex (newVertex, point);
    if (status == DwgDbStatus::Success)
        {
        outId = T_Super::insertVertexAt (atVertex, newVertex);
        return  outId.IsValid() ? DwgDbStatus::Success : DwgDbStatus::UnknownError;
        }
#elif DWGTOOLKIT_RealDwg
    AcDb3dPolylineVertex*   newVertex = new AcDb3dPolylineVertex ();
    status = this->SetVertex (newVertex, point);
    if (status == DwgDbStatus::Success)
        status = ToDwgDbStatus (T_Super::insertVertexAt(outId, atVertex, newVertex));
    if (status != DwgDbStatus::Success && nullptr != newVertex && newVertex->isNewObject())
        delete newVertex;
#endif
    return  status;
    }
DwgDb3dPolyline::Type   DwgDb3dPolyline::GetType () const { return DWGDB_CASTFROMENUM_DB(3dPolyline::Type)(T_Super::polyType()); }
DwgDbObjectIteratorPtr  DwgDb3dPolyline::GetVertexIterator () const { return new DwgDbObjectIterator(T_Super::vertexIterator()); }
DwgDbStatus DwgDb3dPolyline::MakeOpen () { RETURNVOIDORSTATUS(T_Super::makeOpen()); }
DwgDbStatus DwgDb3dPolyline::MakeClosed () { RETURNVOIDORSTATUS(T_Super::makeClosed()); }
DwgDbStatus DwgDb3dPolyline::SplineFit () { RETURNVOIDORSTATUS(T_Super::splineFit()); }
DwgDbStatus DwgDb3dPolyline::SplineFit (Type t, uint16_t n) { RETURNVOIDORSTATUS(T_Super::splineFit(DWGDB_CASTTOENUM_DB(Poly3dType)(t), n)); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbPolyFaceMesh::AppendVertex (DwgDbObjectIdR outId, DwgDbPolyFaceMeshVertexP vertex)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    outId = T_Super::appendVertex (vertex);
    status = outId.isValid() ? DwgDbStatus::Success : DwgDbStatus::UnknownError;
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::appendVertex(outId, vertex));
#endif
    return  status;
    }
int16_t DwgDbPolyFaceMesh::GetNumFaces() const { return T_Super::numFaces(); }
int16_t DwgDbPolyFaceMesh::GetNumVertices() const { return T_Super::numVertices(); }
DwgDbStatus DwgDbPolyFaceMesh::AppendFaceRecord(DwgDbFaceRecordP face) { RETURNVOIDORSTATUS(T_Super::appendFaceRecord(face)); }
DwgDbObjectIteratorPtr DwgDbPolyFaceMesh::GetVertexIterator() const { return new DwgDbObjectIterator(T_Super::vertexIterator()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbPolygonMesh::AppendVertex (DwgDbObjectIdR outId, DwgDbPolygonMeshVertexP vertex)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    outId = T_Super::appendVertex (vertex);
    status = outId.isValid() ? DwgDbStatus::Success : DwgDbStatus::UnknownError;
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::appendVertex(outId, vertex));
#endif
    return  status;
    }
DwgDbPolygonMesh::Type DwgDbPolygonMesh::GetPolyMeshType() const { return static_cast<Type>(T_Super::polyMeshType()); }
DwgDbStatus DwgDbPolygonMesh::SetPolyMeshType(Type t) { RETURNVOIDORSTATUS(T_Super::setPolyMeshType(DWGDB_CASTTOENUM_DB(PolyMeshType)(t))); }
DwgDbStatus DwgDbPolygonMesh::ConvertTo(Type t){ RETURNVOIDORSTATUS(T_Super::convertToPolyMeshType(DWGDB_CASTTOENUM_DB(PolyMeshType)(t))); }
int16_t     DwgDbPolygonMesh::GetMSize() const { return T_Super::mSize(); }
int16_t     DwgDbPolygonMesh::GetNSize() const { return T_Super::nSize(); }
DwgDbStatus DwgDbPolygonMesh::SetMSize(int16_t m) { RETURNVOIDORSTATUS(T_Super::setMSize(m)); }
DwgDbStatus DwgDbPolygonMesh::SetNSize(int16_t n) { RETURNVOIDORSTATUS(T_Super::setNSize(n)); }
bool        DwgDbPolygonMesh::IsMClosed() const { return T_Super::isMClosed(); }
bool        DwgDbPolygonMesh::IsNClosed() const { return T_Super::isNClosed(); }
DwgDbStatus DwgDbPolygonMesh::MakeMClosed() { RETURNVOIDORSTATUS(T_Super::makeMClosed()); }
DwgDbStatus DwgDbPolygonMesh::MakeMOpen() { RETURNVOIDORSTATUS(T_Super::makeMOpen()); }
DwgDbStatus DwgDbPolygonMesh::MakeNClosed() { RETURNVOIDORSTATUS(T_Super::makeNClosed()); }
DwgDbStatus DwgDbPolygonMesh::MakeNOpen() { RETURNVOIDORSTATUS(T_Super::makeNOpen()); }
int16_t     DwgDbPolygonMesh::GetMSurfaceDensity() const { return T_Super::mSurfaceDensity(); }
int16_t     DwgDbPolygonMesh::GetNSurfaceDensity() const { return T_Super::nSurfaceDensity(); }
DwgDbStatus DwgDbPolygonMesh::Straighten() { RETURNVOIDORSTATUS(T_Super::straighten()); }
DwgDbStatus DwgDbPolygonMesh::SurfaceFit() { RETURNVOIDORSTATUS(T_Super::surfaceFit()); }
DwgDbStatus DwgDbPolygonMesh::SurfaceFit(Type t, int16_t u, int16_t v) { RETURNVOIDORSTATUS(T_Super::surfaceFit(DWGDB_CASTTOENUM_DB(PolyMeshType)(t), u, v)); }
DwgDbObjectIteratorPtr DwgDbPolygonMesh::GetVertexIterator() const { return new DwgDbObjectIterator(T_Super::vertexIterator()); }

DPoint3d    DwgDbPolyFaceMeshVertex::GetPosition() const { return Util::DPoint3dFrom(T_Super::position()); }
DwgDbStatus DwgDbPolyFaceMeshVertex::SetPosition(DPoint3dCR p) { RETURNVOIDORSTATUS(T_Super::setPosition(Util::GePoint3dFrom(p))); }

DwgDbPolygonMeshVertex::Type DwgDbPolygonMeshVertex::GetVertexType() const { return static_cast<Type>(T_Super::vertexType()); }
DPoint3d    DwgDbPolygonMeshVertex::GetPosition() const { return Util::DPoint3dFrom(T_Super::position()); }
DwgDbStatus DwgDbPolygonMeshVertex::SetPosition(DPoint3dCR p) { RETURNVOIDORSTATUS(T_Super::setPosition(Util::GePoint3dFrom(p))); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbFaceRecord::GetVertexAt(int16_t faceIndex, int16_t& vertexIndex) const
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    vertexIndex = T_Super::getVertexAt (faceIndex);
    status = vertexIndex < 0 ? DwgDbStatus::UnknownError : DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::getVertexAt(faceIndex, vertexIndex));
#endif
    return  status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbFaceRecord::GetEdgeVisibilityAt (int16_t faceIndex, bool& visible) const
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    visible = T_Super::isEdgeVisibleAt (faceIndex);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::isEdgeVisibleAt(faceIndex, visible));
#endif
    return  status;
    }
DwgDbStatus DwgDbFaceRecord::MakeEdgeVisibleAt(int16_t f) { RETURNVOIDORSTATUS(T_Super::makeEdgeVisibleAt(f)); }
DwgDbStatus DwgDbFaceRecord::MakeEdgeInvisibleAt(int16_t f) { RETURNVOIDORSTATUS(T_Super::makeEdgeInvisibleAt(f)); }
DwgDbStatus DwgDbFaceRecord::SetVertexAt(int16_t f, int16_t v) { RETURNVOIDORSTATUS(T_Super::setVertexAt(f, v)); }

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
DwgDbStatus DwgDbArc::SetCenter (DPoint3dCR center)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setCenter (Util::GePoint3dFrom(center));
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setCenter(Util::GePoint3dFrom(center)));
#endif
    return  status;
    }
DwgDbStatus DwgDbArc::SetRadius (double r)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setRadius (r);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setRadius(r));
#endif
    return  status;
    }
DwgDbStatus DwgDbArc::SetStartAngle (double radians)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setStartAngle (radians);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setStartAngle(radians));
#endif
    return  status;
    }
DwgDbStatus DwgDbArc::SetEndAngle (double radians)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setEndAngle (radians);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setEndAngle(radians));
#endif
    return  status;
    }
DwgDbStatus DwgDbArc::SetThickness (double thickness)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setThickness (thickness);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setThickness(thickness));
#endif
    return  status;
    }
DwgDbStatus DwgDbArc::SetNormal (DVec3dCR v) { RETURNVOIDORSTATUS(T_Super::setNormal(Util::GeVector3dFrom(v))); }
DwgDbStatus DwgDbArc::ReverseCurve () { RETURNVOIDORSTATUS(T_Super::reverseCurve()); }

DPoint3d   DwgDbCircle::GetCenter () const { return Util::DPoint3dFrom(T_Super::center()); }
double     DwgDbCircle::GetDiameter () const { return DWGDB_CALLSDKMETHOD(T_Super::radius() * 2, T_Super::diameter()); }
DVec3d     DwgDbCircle::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double     DwgDbCircle::GetThickness () const { return T_Super::thickness(); }
DwgDbStatus DwgDbCircle::SetCenter (DPoint3dCR center)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setCenter (Util::GePoint3dFrom(center));
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setCenter(Util::GePoint3dFrom(center)));
#endif
    return  status;
    }
DwgDbStatus DwgDbCircle::SetRadius (double radius)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setRadius (radius);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setRadius(radius));
#endif
    return  status;
    }
DwgDbStatus DwgDbCircle::SetThickness (double thickness)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setThickness (thickness);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setThickness(thickness));
#endif
    return  status;
    }

DPoint3d   DwgDbEllipse::GetCenter () const { return Util::DPoint3dFrom(T_Super::center()); }
double     DwgDbEllipse::GetMajorRadius () const { return DWGDB_CALLSDKMETHOD(T_Super::majorAxis().length(), T_Super::majorRadius()); }
DVec3d     DwgDbEllipse::GetMajorAxis () const { return Util::DVec3dFrom(T_Super::majorAxis()); }
double     DwgDbEllipse::GetMinorRadius () const { return DWGDB_CALLSDKMETHOD(T_Super::minorAxis().length(), T_Super::minorRadius()); }
DVec3d     DwgDbEllipse::GetMinorAxis () const { return Util::DVec3dFrom(T_Super::minorAxis()); }
DVec3d     DwgDbEllipse::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double     DwgDbEllipse::GetStartAngle () const { return T_Super::startAngle(); }
double     DwgDbEllipse::GetStartParam () const { double p=0.0; T_Super::getStartParam(p); return p; }
double     DwgDbEllipse::GetEndAngle () const { return T_Super::endAngle(); }
double     DwgDbEllipse::GetEndParam () const { double p=0.0; T_Super::getEndParam(p); return p; }
DwgDbStatus DwgDbEllipse::SetCenter (DPoint3dCR p) { RETURNVOIDORSTATUS(T_Super::setCenter(Util::GePoint3dFrom(p))); }
DwgDbStatus DwgDbEllipse::SetStartAngle (double a) { RETURNVOIDORSTATUS(T_Super::setStartAngle(a)); }
DwgDbStatus DwgDbEllipse::SetStartParam (double a) { RETURNVOIDORSTATUS(T_Super::setStartParam(a)); }
DwgDbStatus DwgDbEllipse::SetEndAngle (double a) { RETURNVOIDORSTATUS(T_Super::setEndAngle(a)); }
DwgDbStatus DwgDbEllipse::SetEndParam (double a) { RETURNVOIDORSTATUS(T_Super::setEndParam(a)); }
DwgDbStatus DwgDbEllipse::Set (DPoint3dCR c, DVec3dCR n, DVec3dCR m, double r, double a1, double a2) { RETURNVOIDORSTATUS(T_Super::set(Util::GePoint3dFrom(c), Util::GeVector3dFrom(n), Util::GeVector3dFrom(m), r, a1, a2)); }
DwgDbStatus DwgDbEllipse::ReverseCurve () { RETURNVOIDORSTATUS(T_Super::reverseCurve()); }
DwgDbStatus DwgDbEllipse::SetMajorRadius (double radius)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    BeAssert (false && "Missing method OdDbEllipse::setMajorRadius() in Teigha!");
    status = DwgDbStatus::NotSupported;
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setMajorRadius(radius));
#endif
    return  status;
    }
DwgDbStatus DwgDbEllipse::GetMinorRadius (double radius)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    BeAssert (false && "Missing method OdDbEllipse::setMinorRadius() in Teigha!");
    status = DwgDbStatus::NotSupported;
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setMinorRadius(radius));
#endif
    return  status;
    }

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
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbSpline::SetNurbsData (int16_t degree, bool rational, bool closed, bool periodic, DPoint3dArrayCR poles, DwgDbDoubleArrayCR knots, DwgDbDoubleArrayCR weights, double poleTol, double knotTol)
    {
    DWGGE_Type(Point3dArray)    gePoles;
    Util::GetGePointArray (gePoles, poles);

    DWGGE_Type(DoubleArray) geKnots;
    for (auto knot : knots)
        geKnots.append (knot);
    
    DWGGE_Type(DoubleArray) geWeights;
    for (auto weight : weights)
        geWeights.append (weight);

    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setNurbsData (degree, rational, closed, periodic, gePoles, geKnots, geWeights, poleTol, knotTol);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (
    T_Super::setNurbsData (degree, rational, closed, periodic, gePoles, geKnots, geWeights, poleTol, knotTol));
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbSpline::GetFitData (DPoint3dArrayR points, int16_t& degree, double& tol, bool& hasTangents, DVec3dR startTangent, DVec3dR endTangent) const
    {
    DWGGE_Type(Point3dArray)    gePoints;
    DWGGE_Type(Vector3d)    geTangent0, geTangent1;
    int d = 0;

    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getFitData (gePoints, d, tol, hasTangents, geTangent0, geTangent1);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::getFitData(gePoints, d, tol, hasTangents, geTangent0, geTangent1));
#endif
    if (status == DwgDbStatus::Success)
        {
        degree = static_cast <int16_t> (d);
        Util::GetPointArray (points, gePoints);
        startTangent = Util::DVec3dFrom (geTangent0);
        endTangent = Util::DVec3dFrom (geTangent1);
        }

    return  status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbSpline::SetFitData (DPoint3dArrayCR points, int16_t degree, double fitTol, DVec3dCR startTangent, DVec3dCR endTangent)
    {
    DWGGE_Type(Point3dArray)    gePoints;
    Util::GetGePointArray (gePoints, points);

    DWGGE_Type(Vector3d)    geTangent0 = Util::GeVector3dFrom (startTangent);
    DWGGE_Type(Vector3d)    geTangent1 = Util::GeVector3dFrom (endTangent);
    
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setFitData (gePoints, degree, fitTol, geTangent0, geTangent1);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (
    T_Super::setFitData (gePoints, degree, fitTol, geTangent0, geTangent1));
#endif
    return  status;
    }
DwgDbStatus DwgDbSpline::SetControlPointAt(int i, DPoint3dCR p) { RETURNVOIDORSTATUS(T_Super::setControlPointAt(i,Util::GePoint3dFrom(p))); }
DwgDbStatus DwgDbSpline::SetWeightAt (int i, double w) { RETURNVOIDORSTATUS(T_Super::setWeightAt(i,w)); }
bool        DwgDbSpline::HasFitData () const { return T_Super::hasFitData(); }
DwgDbStatus DwgDbSpline::PurgeFitData () { RETURNVOIDORSTATUS(T_Super::purgeFitData()); }
DwgDbStatus DwgDbSpline::SetFitPointAt (int i, DPoint3dCR p) { RETURNVOIDORSTATUS(T_Super::setFitPointAt(i, Util::GePoint3dFrom(p))); }
DwgDbStatus DwgDbSpline::SetFitTangents (DVec3dCR v1, DVec3dCR v2) { RETURNVOIDORSTATUS(T_Super::setFitTangents(Util::GeVector3dFrom(v1), Util::GeVector3dFrom(v2))); }
DwgDbStatus DwgDbSpline::SetFitTolerance (double t) { RETURNVOIDORSTATUS(T_Super::setFitTol(t)); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbSpline::GetSamplePoints (double fromParam, double toParam, double chordTol, DPoint3dArrayR points, DwgDbDoubleArrayR params, bool resample) const
    {
    DWGGE_TypeP(Curve3d) curve = nullptr;
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    status = ToDwgDbStatus (T_Super::getOdGeCurve(curve, OdGeContext::gTol));
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::getAcGeCurve(curve, AcGeContext::gTol));
#endif

    if (status == DwgDbStatus::Success && curve != nullptr)
        {
        DWGGE_Type(Point3dArray)    gePoints;
        DWGGE_Type(DoubleArray)     geParams;

        curve->getSamplePoints (fromParam, toParam, chordTol, gePoints, geParams, resample);

        uint32_t    nPoints = static_cast<uint32_t> (gePoints.length());
        if (nPoints == 0)
            return  DwgDbStatus::InvalidData;

        for (uint32_t i = 0; i < nPoints; i++)
            {
            points.push_back (Util::DPoint3dFrom(gePoints[i]));
            params.push_back (geParams[i]);
            }

        delete curve;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbSpline::GetSamplePoints (int32_t nSample, DPoint3dArrayR points) const
    {
    DWGGE_TypeP(Curve3d) curve = nullptr;
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    status = ToDwgDbStatus (T_Super::getOdGeCurve(curve, OdGeContext::gTol));
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::getAcGeCurve(curve, AcGeContext::gTol));
#endif

    if (status == DwgDbStatus::Success && curve != nullptr)
        {
        DWGGE_Type(Point3dArray)    gePoints;
        curve->getSamplePoints (nSample, gePoints);
        Util::GetPointArray (points, gePoints);

        delete curve;
        }
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbSpline::GetSamplePoints (int32_t nSample, DPoint3dArrayR points, DwgDbDoubleArrayR params) const
    {
    DWGGE_TypeP(Curve3d) curve = nullptr;
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    status = ToDwgDbStatus (T_Super::getOdGeCurve(curve, OdGeContext::gTol));
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::getAcGeCurve(curve, AcGeContext::gTol));
#endif

    if (status == DwgDbStatus::Success && curve != nullptr)
        {
        DWGGE_Type(Point3dArray)    gePoints;
        DWGGE_Type(DoubleArray)     geParams;

        curve->getSamplePoints (nSample, gePoints, geParams);

        uint32_t    nPoints = static_cast<uint32_t> (gePoints.length());
        if (nPoints == 0)
            return  DwgDbStatus::InvalidData;

        for (uint32_t i = 0; i < nPoints; i++)
            {
            points.push_back (Util::DPoint3dFrom(gePoints[i]));
            params.push_back (geParams[i]);
            }

        delete curve;
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
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbAttribute::SetMTextAttributeConst (DwgDbMTextCP mtext)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    // Teigha does not have OdDbAttribute::setMTextAttributeConst!!
    T_Super::setMTextAttribute (dynamic_cast<OdDbMText*>(const_cast<DwgDbMTextP>(mtext)));
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setMTextAttributeConst(dynamic_cast<AcDbMText const*>(mtext)));
#endif
    return  status;
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
DwgDbStatus DwgDbAttribute::SetOrigin (DPoint3dCR p) { RETURNVOIDORSTATUS(T_Super::setPosition(Util::GePoint3dFrom(p))); }
DwgDbStatus DwgDbAttribute::SetNormal (DVec3dCR n) { RETURNVOIDORSTATUS(T_Super::setNormal(Util::GeVector3dFrom(n))); }
DwgDbStatus DwgDbAttribute::SetThickness (double t) { RETURNVOIDORSTATUS(T_Super::setThickness(t)); }
DwgDbStatus DwgDbAttribute::SetInvisible (bool on) { RETURNVOIDORSTATUS(T_Super::setInvisible(on)); }
DwgDbStatus DwgDbAttribute::SetLockPositionInBlock (bool on) { RETURNVOIDORSTATUS(T_Super::setLockPositionInBlock(on)); }
DwgDbStatus DwgDbAttribute::SetMTextAttribute (DwgDbMTextP mt) { RETURNVOIDORSTATUS(T_Super::setMTextAttribute(DWGDB_DOWNWARDCAST(MText*)(mt))); }
DwgDbStatus DwgDbAttribute::SetTag (WCharCP t) { RETURNVOIDORSTATUS(T_Super::setTag(t)); }
DwgDbStatus DwgDbAttribute::SetAttributeFromBlock (TransformCR t) { DWGGE_Type(Matrix3d) m; Util::GetGeMatrix(m,t); RETURNVOIDORSTATUS(T_Super::setAttributeFromBlock(m)); }
DwgDbStatus DwgDbAttribute::SetAttributeFromBlock (DwgDbAttributeDefinitionCP at, TransformCR t) { DWGGE_Type(Matrix3d) m; Util::GetGeMatrix(m,t); RETURNVOIDORSTATUS(T_Super::setAttributeFromBlock(dynamic_cast<DWGDB_TypeCP(AttributeDefinition)>(at),m)); }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbAttributeDefinition::SetMTextAttributeConst (DwgDbMTextCP mtext)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    // Teigha does not have OdDbAttributeDefinition::setMTextAttributeConst!!
    T_Super::setMTextAttributeDefinition (dynamic_cast<OdDbMText*>(const_cast<DwgDbMTextP>(mtext)));
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setMTextAttributeDefinitionConst(dynamic_cast<AcDbMText const*>(mtext)));
#endif
    return  status;
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
DwgDbStatus DwgDbAttributeDefinition::SetOrigin (DPoint3dCR p) { RETURNVOIDORSTATUS(T_Super::setPosition(Util::GePoint3dFrom(p))); }
DwgDbStatus DwgDbAttributeDefinition::SetNormal (DVec3dCR n) { RETURNVOIDORSTATUS(T_Super::setNormal(Util::GeVector3dFrom(n))); }
DwgDbStatus DwgDbAttributeDefinition::SetThickness (double t) { RETURNVOIDORSTATUS(T_Super::setThickness(t)); }
DwgDbStatus DwgDbAttributeDefinition::SetInvisible (bool on) { RETURNVOIDORSTATUS(T_Super::setInvisible(on)); }
DwgDbStatus DwgDbAttributeDefinition::SetLockPositionInBlock (bool on) { RETURNVOIDORSTATUS(T_Super::setLockPositionInBlock(on)); }
DwgDbStatus DwgDbAttributeDefinition::SetMTextAttribute (DwgDbMTextP mt) { RETURNVOIDORSTATUS(T_Super::setMTextAttributeDefinition(DWGDB_DOWNWARDCAST(MText*)(mt))); }
DwgDbStatus DwgDbAttributeDefinition::SetTag (WCharCP t) { RETURNVOIDORSTATUS(T_Super::setTag(t)); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbViewport::GetUcs (DPoint3dR origin, DVec3dR xAxis, DVec3dR yAxis) const
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
DwgDbStatus DwgDbViewport::SetFrozenLayers (DwgDbObjectIdArrayCR idsIn)
    {
    DwgDbStatus status = DwgDbStatus::Success;
    DWGDB_Type(ObjectIdArray)   idArray;
    Util::GetObjectIdArray (idArray, idsIn);

#ifdef DWGTOOLKIT_OpenDwg
    T_Super::freezeLayersInViewport (idArray);
    status = idArray.length() > 0 ? DwgDbStatus::Success : DwgDbStatus::UnknownError;
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::freezeLayersInViewport(idArray));
#endif
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
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbViewport::SetAnnotationScale (double scale)
    {
    // database residency a prerequisite
    DWGDB_TypeP(Database) dwg = T_Super::database ();
    if (nullptr == dwg)
        return DwgDbStatus::NotPersistentObject;

    DwgDbStatus status = DwgDbStatus::UnknownError;
    DWGDB_TypeP(ObjectContextManager) contextManager = dwg->objectContextManager ();
    if (nullptr == contextManager)
        return  status;

    DWGDB_TypeCP(ObjectContextCollection) contextCollection = contextManager->contextCollection(DWGSTR_NAME(ANNOTATIONSCALES_COLLECTION));
    if (nullptr == contextCollection)
        return  status;

#ifdef DWGTOOLKIT_OpenDwg
    OdDbObjectContextCollectionIteratorPtr iter = contextCollection->newIterator ();
    if (iter.isNull())
        return  status;
#elif DWGTOOLKIT_RealDwg
    AcDbObjectContextCollectionIterator* iter = contextCollection->newIterator ();
    if (nullptr == iter)
        return  status;
#endif

    for (iter->start(); !iter->done(); iter->next())
        {
#ifdef DWGTOOLKIT_OpenDwg
        OdDbObjectContextPtr objectContext = iter->getContext ();
        if (!objectContext.isNull())
#elif DWGTOOLKIT_RealDwg
        AcDbObjectContext*  objectContext = nullptr;
        if (Acad::eOk == iter->getContext(objectContext))
#endif
            {
            bool    matchFound = false;
            double  checkScale = 1.0;

            DWGDB_TypeP(AnnotationScale) annoScale = DWGDB_Type(AnnotationScale)::cast (objectContext);
            if (nullptr != annoScale && ToDwgDbStatus(annoScale->getScale(checkScale)) == DwgDbStatus::Success)
                matchFound = fabs(checkScale - scale) < 1.0e-5;

            if (matchFound)
                status = ToDwgDbStatus (T_Super::setAnnotationScale(annoScale));

#ifdef DWGTOOLKIT_RealDwg
            delete objectContext;
#endif
            if (DwgDbStatus::Success == status)
                break;
            }
        }

#ifdef DWGTOOLKIT_RealDwg
    delete iter;
#endif

    return  status;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbViewport::SetTransparent (bool onOff)
    {
    DwgDbStatus status = DwgDbStatus::NotSupported;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setTransparent ();
    BeAssert (false && "OdDbViewport::setTransparent() not implemented!");
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setTransparent(onOff));
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbViewport::SetSun (DwgDbObjectIdR id, DwgDbSunP sun, bool b) 
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    id = T_Super::setSun (sun);
    if (!id.IsValid())
        status = DwgDbStatus::UnknownError;
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setSun(id, sun, b));
#endif
    return status;
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
DwgDbObjectId  DwgDbViewport::GetSunId () const { return T_Super::sunId(); }
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
DVec2d         DwgDbViewport::GetGridIncrements () const { return Util::DVec2dFrom(T_Super::gridIncrement()); }
DVec2d         DwgDbViewport::GetSnapIncrements () const { return Util::DVec2dFrom(T_Super::snapIncrement()); }
DPoint2d       DwgDbViewport::GetSnapBase () const { return Util::DPoint2dFrom(T_Super::snapBasePoint()); }
double         DwgDbViewport::GetSnapAngle () const { return T_Super::snapAngle(); }
SnapIsoPair    DwgDbViewport::GetSnapIsoPair () const { return static_cast<SnapIsoPair>(T_Super::snapIsoPair()); }
bool           DwgDbViewport::IsSnapEnabled () const { return T_Super::isSnapOn(); }
bool           DwgDbViewport::IsIsometricSnapEnabled () const { return T_Super::isSnapIsometric(); }
int16_t        DwgDbViewport::GetGridMajor () const { return gridMajor(); }
bool           DwgDbViewport::IsLayerFrozen (DwgDbObjectIdCR layerId) const { return T_Super::isLayerFrozenInViewport(layerId); }
double         DwgDbViewport::GetCustomScale () const { return T_Super::customScale(); }
double         DwgDbViewport::GetBrightness () const { return T_Super::brightness(); }
DwgCmColor     DwgDbViewport::GetAmbientLightColor () const { return static_cast<DwgCmColor>(T_Super::ambientLightColor()); }
DwgDbStatus    DwgDbViewport::SetVisualStyle (DwgDbObjectId id) { DwgDbStatus s=DwgDbStatus::Success; DWGDB_CALLSDKMETHOD(T_Super::setVisualStyle(id), s=ToDwgDbStatus(T_Super::setVisualStyle(id))); return s; }
DwgDbStatus DwgDbViewport::SetIsOn (bool b) { if(b) RETURNVOIDORSTATUS(T_Super::setOn()) else RETURNVOIDORSTATUS(T_Super::setOff()); }
DwgDbStatus DwgDbViewport::EnableGrid (bool b) { if(b) RETURNVOIDORSTATUS(T_Super::setGridOn()) else RETURNVOIDORSTATUS(T_Super::setGridOff()); }
DwgDbStatus DwgDbViewport::EnableSnap (bool b) { if(b) RETURNVOIDORSTATUS(T_Super::setSnapOn()) else RETURNVOIDORSTATUS(T_Super::setSnapOff()); }
DwgDbStatus DwgDbViewport::EnableIsometricSnap (bool b) { if(b) RETURNVOIDORSTATUS(T_Super::setSnapIsometric()) else RETURNVOIDORSTATUS(T_Super::setSnapStandard()); }
DwgDbStatus DwgDbViewport::EnableUcsIcon (bool b) { if(b) RETURNVOIDORSTATUS(T_Super::setUcsIconVisible()) else RETURNVOIDORSTATUS(T_Super::setUcsIconInvisible()); }
DwgDbStatus DwgDbViewport::EnableFrontClip (bool b) { if(b) RETURNVOIDORSTATUS(T_Super::setFrontClipOn()) else RETURNVOIDORSTATUS(T_Super::setFrontClipOff()); }
DwgDbStatus DwgDbViewport::EnableBackClip (bool b) { if(b) RETURNVOIDORSTATUS(T_Super::setBackClipOn()) else RETURNVOIDORSTATUS(T_Super::setBackClipOff()); }
DwgDbStatus DwgDbViewport::EnablePerspective (bool b) { if(b) RETURNVOIDORSTATUS(T_Super::setPerspectiveOn()) else RETURNVOIDORSTATUS(T_Super::setPerspectiveOff()); }
DwgDbStatus DwgDbViewport::SetFrontClipAtEye (bool b) { if(b) RETURNVOIDORSTATUS(T_Super::setFrontClipAtEyeOn()) else RETURNVOIDORSTATUS(T_Super::setFrontClipAtEyeOff()); }
DwgDbStatus DwgDbViewport::SetFrontClipDistance (double d) { RETURNVOIDORSTATUS(T_Super::setFrontClipDistance(d)); }
DwgDbStatus DwgDbViewport::SetBackClipDistance (double d) { RETURNVOIDORSTATUS(T_Super::setBackClipDistance(d)); }
DwgDbStatus DwgDbViewport::SetBackground (DwgDbObjectId id) { RETURNVOIDORSTATUS(T_Super::setBackground(id)); }
DwgDbStatus DwgDbViewport::SetClipEntity (DwgDbObjectId id) { RETURNVOIDORSTATUS(T_Super::setNonRectClipEntityId(id)); }
DwgDbStatus DwgDbViewport::SetDefaultLightingOn (bool b) { RETURNVOIDORSTATUS(T_Super::setDefaultLightingOn(b)); }
DwgDbStatus DwgDbViewport::SetViewDirection (DVec3dCR v) { RETURNVOIDORSTATUS(T_Super::setViewDirection(Util::GeVector3dFrom(v))); }
void        DwgDbViewport::SetUcsPerViewport (bool b) { T_Super::setUcsPerViewport(b); }
DwgDbStatus DwgDbViewport::SetUcs (DPoint3dCR o, DVec3dCR x, DVec3dCR y) { RETURNVOIDORSTATUS(T_Super::setUcs(Util::GePoint3dFrom(o),Util::GeVector3dFrom(x),Util::GeVector3dFrom(y))); }
DwgDbStatus DwgDbViewport::SetUcsElevation (double e) { RETURNVOIDORSTATUS(T_Super::setElevation(e)); }
DwgDbStatus DwgDbViewport::SetLensLength (double l) { RETURNVOIDORSTATUS(T_Super::setLensLength(l)); }
DwgDbStatus DwgDbViewport::SetViewTwist (double a) { RETURNVOIDORSTATUS(T_Super::setTwistAngle(a)); }
DwgDbStatus DwgDbViewport::SetViewTarget (DPoint3dCR t) { RETURNVOIDORSTATUS(T_Super::setViewTarget(Util::GePoint3dFrom(t))); }
DwgDbStatus DwgDbViewport::SetGridIncrements (DVec2dCR v) { RETURNVOIDORSTATUS(T_Super::setGridIncrement(Util::GeVector2dFrom(v))); }
DwgDbStatus DwgDbViewport::SetSnapIncrements (DVec2dCR v) { RETURNVOIDORSTATUS(T_Super::setSnapIncrement(Util::GeVector2dFrom(v))); }
DwgDbStatus DwgDbViewport::SetSnapBase (DPoint2dCR p) { RETURNVOIDORSTATUS(T_Super::setSnapBasePoint(Util::GePoint2dFrom(p))); }
DwgDbStatus DwgDbViewport::SetSnapAngle (double a) { RETURNVOIDORSTATUS(T_Super::setSnapAngle(a)); }
DwgDbStatus DwgDbViewport::SetSnapIsoPair (SnapIsoPair s) { RETURNVOIDORSTATUS(T_Super::setSnapIsoPair(static_cast<DwgDbUInt16>(s))); }
DwgDbStatus DwgDbViewport::SetGridMajor (uint16_t i) { RETURNVOIDORSTATUS(T_Super::setGridMajor(i)); }
DwgDbStatus DwgDbViewport::SetCustomScale (double s) { RETURNVOIDORSTATUS(T_Super::setCustomScale(s)); }
DwgDbStatus DwgDbViewport::SetBrightness (double b) { RETURNVOIDORSTATUS(T_Super::setBrightness(b)); }
DwgDbStatus DwgDbViewport::SetAmbientLightColor (DwgCmColorCR c) { RETURNVOIDORSTATUS(T_Super::setAmbientLightColor(c)); }

DwgDbViewBorder::Source DwgDbViewBorder::GetSourceType () const { DWGDB_CALLSDKMETHOD({BeAssert(false && "Unsupported in Teigha!"); return Source::NotDefined;}, {return static_cast<Source>(T_Super::sourceType());}) }
DwgDbViewBorder::ViewStyle DwgDbViewBorder::GetViewStyle () const { DWGDB_CALLSDKMETHOD({BeAssert(false && "Unsupported in Teigha!"); return ViewStyle::FromBase;}, {return static_cast<ViewStyle>(T_Super::viewStyleType());}) }
DwgDbObjectId DwgDbViewBorder::GetViewportId () const { return T_Super::viewportId(); }
double      DwgDbViewBorder::GetHeight () const { DWGDB_CALLSDKMETHOD({BeAssert(false && "Unsupported in Teigha!"); return 0.0;}, {return T_Super::height();}) }
double      DwgDbViewBorder::GetWidth () const { DWGDB_CALLSDKMETHOD({BeAssert(false && "Unsupported in Teigha!"); return 0.0;}, {return T_Super::width();}) }
DPoint3d    DwgDbViewBorder::GetInsertionPoint () const { DWGDB_CALLSDKMETHOD({BeAssert(false && "Unsupported in Teigha!"); return DPoint3d::FromZero();}, {return Util::DPoint3dFrom(T_Super::insertionPoint());}) }
DwgString   DwgDbViewBorder::GetInventorFileReference () const { DWGDB_CALLSDKMETHOD({BeAssert(false && "Unsupported in Teigha!"); return DwgString();}, {return T_Super::inventorFileReference();}) }
bool        DwgDbViewBorder::IsFirstAngleProjection () const { DWGDB_CALLSDKMETHOD({BeAssert(false && "Unsupported in Teigha!"); return false;}, {return T_Super::isFirstAngleProjection();}) }
double      DwgDbViewBorder::GetRotationAngle () const { return T_Super::rotationAngle(); }
double      DwgDbViewBorder::GetScale () const { return T_Super::scale(); }
uint32_t    DwgDbViewBorder::GetShadedDPI () const { DWGDB_CALLSDKMETHOD({BeAssert(false && "Unsupported in Teigha!"); return 0;}, {return T_Super::shadedDPI();}) }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d      DwgDbBlockReference::GetScaleFactors () const
    {
    DWGGE_Type(Scale3d) scale = T_Super::scaleFactors ();
    return DVec3d::From (scale.sx, scale.sy, scale.sz);
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
DwgDbObjectIteratorPtr DwgDbBlockReference::GetAttributeIterator () const { return new DwgDbObjectIterator(T_Super::attributeIterator()); }
DwgDbStatus    DwgDbBlockReference::SetPosition (DPoint3dCR o) { RETURNVOIDORSTATUS(T_Super::setPosition(Util::GePoint3dFrom(o))); }
DwgDbStatus    DwgDbBlockReference::SetRotation (double a) { RETURNVOIDORSTATUS(T_Super::setRotation(a)); }
DwgDbStatus    DwgDbBlockReference::SetScales (DVec3dCR s) { RETURNVOIDORSTATUS(T_Super::setScaleFactors(Util::GeScale3dFrom(s))); }
DwgDbStatus    DwgDbBlockReference::SetNormal (DVec3dCR v) { RETURNVOIDORSTATUS(T_Super::setNormal(Util::GeVector3dFrom(v))); }
DwgDbStatus    DwgDbBlockReference::SetBlockTableRecord (DwgDbObjectIdCR id) { RETURNVOIDORSTATUS(T_Super::setBlockTableRecord(id)); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbBlockReference::OpenSpatialFilter (DwgDbSpatialFilterPtr& filterOut, DwgDbOpenMode mode) const
    {
    DwgDbStatus status = DwgDbStatus::ObjectNotOpenYet;

#ifdef DWGTOOLKIT_OpenDwg
    OdDbFilterPtr   odFilter = OdDbIndexFilterManager::getFilter (this, OdDbSpatialFilter::desc(), FromDwgDbOpenMode(mode));
    if (!odFilter.isNull() && !(filterOut = DwgDbSpatialFilter::Cast(odFilter.detach())).isNull() && filterOut->isReadEnabled())
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
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d          DwgDbViewRepBlockReference::GetScaleFactors () const
    {
    DWGGE_Type(Scale3d) scale = T_Super::scaleFactors ();
    return DVec3d::From (scale.sx, scale.sy, scale.sz);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbViewRepBlockReference::GetExtentsBestFit (DRange3dR out, TransformCR parentXform) const
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
bool            DwgDbViewRepBlockReference::IsXAttachment (WStringP blockName, WStringP path) const
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
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbViewRepBlockReference::OpenSpatialFilter (DwgDbSpatialFilterPtr& filterOut, DwgDbOpenMode mode) const
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
DwgDbObjectId   DwgDbViewRepBlockReference::GetBlockTableRecordId () const { return T_Super::blockTableRecord(); }
DPoint3d        DwgDbViewRepBlockReference::GetPosition () const { return Util::DPoint3dFrom(T_Super::position()); }
void            DwgDbViewRepBlockReference::GetBlockTransform (TransformR out) const { return Util::GetTransform(out, T_Super::blockTransform()); }
DVec3d          DwgDbViewRepBlockReference::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
DwgDbStatus     DwgDbViewRepBlockReference::ExplodeToOwnerSpace () const { return ToDwgDbStatus(T_Super::explodeToOwnerSpace()); }
DwgDbObjectIteratorPtr DwgDbViewRepBlockReference::GetAttributeIterator () const { return new DwgDbObjectIterator(T_Super::attributeIterator()); }
DwgDbObjectId   DwgDbViewRepBlockReference::GetOwnerViewportId () const { return T_Super::ownerViewportId(); }
void            DwgDbViewRepBlockReference::SetOwnerViewportId (DwgDbObjectId id) { T_Super::setOwnerViewportId(id); }

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
DwgDbHatch::Style       DwgDbHatch::GetHatchStyle () const { return DWGDB_UPWARDCAST(Hatch::Style)(T_Super::hatchStyle()); }
DwgDbHatch::LoopType    DwgDbHatch::GetLoopType (size_t i) const { return DWGDB_UPWARDCAST(Hatch::LoopType)(T_Super::loopTypeAt((int)i)); }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbHatch::SetHatchStyle (DwgDbHatch::Style style)
    {
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setHatchStyle (static_cast<OdDbHatch::HatchStyle>(style));
    return  DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg
    Acad::ErrorStatus   es = T_Super::setHatchStyle (static_cast<AcDbHatch::HatchStyle>(style));
    return ToDwgDbStatus(es);
#endif  // DWGTOOLKIT_
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbHatch::GetLoop (size_t loopIndex, CurveVectorPtr& edges) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    EdgeArray   geCurves;
    T_Super::getLoopAt ((int)loopIndex, geCurves);

#elif DWGTOOLKIT_RealDwg
    AcGeVoidPointerArray    geCurves;
    AcGeIntArray    geEdgeTypes;
    Adesk::Int32    looptype;

    auto es = T_Super::getLoopAt ((int)loopIndex, looptype, geCurves, geEdgeTypes);
    if (Acad::eOk != es)
        return ToDwgDbStatus(es);
#endif

    if (geCurves.isEmpty())
        return DwgDbStatus::InvalidData;

    edges = CurveVector::Create (CurveVector::BoundaryType::BOUNDARY_TYPE_None);
    if (!edges.IsValid())
        return  DwgDbStatus::MemoryError;

    int nCurves = (int)geCurves.length ();

#ifdef DWGTOOLKIT_OpenDwg
    for (int i = 0; i < nCurves; i++)
        {
        auto geCurve = static_cast<OdGeCurve2d*>(geCurves[i]);
        if (geCurve != nullptr)
            {
            ICurvePrimitivePtr  curve;
            if (Util::GetCurvePrimitive(curve, geCurve) == DwgDbStatus::Success)
                edges->Add (curve);
            }
        }

#elif DWGTOOLKIT_RealDwg

    for (int i = 0; i < nCurves; i++)
        {
        ICurvePrimitivePtr  primitive;
        switch (geEdgeTypes[i])
            {
            case AcDbHatch::kLine:
                {
                auto line = static_cast<AcGeLineSeg2d*>(geCurves[i]);
                if (line != nullptr)
                    {
                    auto start = line->startPoint ();
                    auto end = line->endPoint ();
                    primitive = ICurvePrimitive::CreateLine (DSegment3d::From(start.x, start.y, 0.0, end.x, end.y, 0.0));
                    }
                break;
                }
            case AcDbHatch::kCirArc:
                {
                auto arc = static_cast<AcGeCircArc2d*>(geCurves[i]);
                if (arc != nullptr)
                    primitive = ICurvePrimitive::CreateArc (Util::DEllipse3dFrom(*arc));
                break;
                }
            case AcDbHatch::kEllArc:
                {
                auto ellipArc = static_cast<AcGeEllipArc2d*>(geCurves[i]);
                if (ellipArc != nullptr)
                    primitive = ICurvePrimitive::CreateArc (Util::DEllipse3dFrom(*ellipArc));
                break;
                }
            case AcDbHatch::kSpline:
                {
                auto spline = static_cast<AcGeNurbCurve2d*>(geCurves[i]);
                if (spline != nullptr)
                    {
                    MSBsplineCurve  curve;
                    if (DwgDbStatus::Success == Util::GetMSBsplineCurve(curve, *spline))
                        primitive = ICurvePrimitive::CreateBsplineCurve (curve);
                    }
                break;
                }
            default:
                BeAssert (false && "Unknown hatch edge type!");
            }
        if (primitive.IsValid())
            edges->Add (primitive);
        }
#endif
    return  edges->empty() ? DwgDbStatus::UnknownError : DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbHatch::GetLoop (size_t loopIndex, DPoint2dArrayR vertices, DwgDbDoubleArrayR bulges) const
    {
    DWGGE_Type(Point2dArray) gePoints;
    DWGGE_Type(DoubleArray) geBulges;

#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getLoopAt ((int)loopIndex, gePoints, geBulges);

#elif DWGTOOLKIT_RealDwg
    Adesk::Int32 looptype;

    auto es = T_Super::getLoopAt ((int)loopIndex, looptype, gePoints, geBulges);
    if (Acad::eOk != es)
        return ToDwgDbStatus(es);
#endif

    if (Util::GetPointArray(vertices, gePoints) < 2)
        return DwgDbStatus::InvalidData;

    int nBulges = (int)geBulges.length ();
    for (int i = 0; i < nBulges; i++)
        bulges.push_back (geBulges[i]);

    return  DwgDbStatus::Success;
    }

bool            DwgDbHatch::IsHatch () const { return T_Super::isHatch(); }
bool            DwgDbHatch::IsSolidFill () const { return T_Super::isSolidFill(); }
bool            DwgDbHatch::IsGradient () const { return T_Super::isGradient(); }
bool            DwgDbHatch::IsAssociative () const { return T_Super::associative(); }
DwgDbStatus     DwgDbHatch::SetAssociative (bool on) { RETURNVOIDORSTATUS(T_Super::setAssociative(on)); }
double          DwgDbHatch::GetGradientAngle () const { return T_Super::gradientAngle(); }
double          DwgDbHatch::GetGradientShift () const { return T_Super::gradientShift(); }
double          DwgDbHatch::GetGradientTint () const { return T_Super::getShadeTintValue(); }
DwgString       DwgDbHatch::GetGradientName () const { return T_Super::gradientName(); }
DVec3d          DwgDbHatch::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
size_t          DwgDbHatch::GetLoopCount () const { return T_Super::numLoops(); }
DwgCmColor      DwgDbHatch::GetBackgroundColor () const { return T_Super::backgroundColor(); }
DwgDbStatus     DwgDbHatch::SetBackgroundColor (DwgCmColorCR c) { RETURNVOIDORSTATUS(T_Super::setBackgroundColor(c)); }
double          DwgDbHatch::GetElevation () const { return T_Super::elevation(); }
DwgDbStatus     DwgDbHatch::SetElevation (double z) { RETURNVOIDORSTATUS(T_Super::setElevation(z)); }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbRasterImage::GetFileName (DwgStringR name, DwgStringP activeFile) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    OdDbRasterImageDefPtr   def = T_Super::imageDefId().openObject ();
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
            DwgToolkitHost::GetHost().Warn (L"Unable to load raster image file!");
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

DPoint3d        DwgDbText::GetPosition () const { return Util::DPoint3dFrom(T_Super::position()); }
DVec3d          DwgDbText::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
double          DwgDbText::GetThickness () const { return T_Super::thickness(); }
DwgString       DwgDbText::GetTextString () const { return DWGDB_CALLSDKMETHOD(T_Super::textString(),T_Super::textStringConst()); }

DPoint3d DwgDbMText::GetPosition () const { return Util::DPoint3dFrom(T_Super::location()); }
DVec3d   DwgDbMText::GetNormal () const { return Util::DVec3dFrom(T_Super::normal()); }
DwgString DwgDbMText::GetTextString () const { return T_Super::contents(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbLight::GetLampColorRGB (RgbFactor& rgb) const
    {
    DWGGI_Type(ColorRGB) giRgb;
    DwgDbStatus status = DwgDbStatus::Success;

#ifdef DWGTOOLKIT_OpenDwg
    giRgb = T_Super::lampColorRGB ();
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::lampColorRGB(giRgb));
#endif
    rgb = RgbFactor::From (giRgb.red, giRgb.green, giRgb.blue);
    return  status;
    }

DwgDbStatus DwgDbLight::GetResultingColor (DwgCmColorR color) const
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    color = T_Super::resultingColor ();
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::resultingColor(color));
#endif
    return  status;
    }

DwgDbStatus DwgDbLight::GetWebFile (DwgStringR webFile) const
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    webFile = T_Super::webFile ();
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::webFile(webFile));
#endif
    return  status;
    }

DwgDbStatus DwgDbLight::GetWebRotation (DVec3dR rotation) const
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    rotation = Util::DVec3dFrom (T_Super::webRotation());
#elif DWGTOOLKIT_RealDwg
    AcGeVector3d rot;
    status = ToDwgDbStatus (T_Super::webRotation(rot));
    if (status == DwgDbStatus::Success)
        rotation = Util::DVec3dFrom (rot);
#endif
    return  status;
    }

DwgDbStatus DwgDbLight::GetShadowParameters (DwgGiShadowParametersR shadow) const
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    shadow = T_Super::shadowParameters ();
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::shadowParameters(shadow));
#endif
    return  status;
    }
bool    DwgDbLight::IsOn () const { return T_Super::isOn(); }
DwgDbStatus DwgDbLight::SetOn(bool on) { DwgDbStatus st=DwgDbStatus::Success; DWGDB_CALLSDKMETHOD(T_Super::setOn(on), st=ToDwgDbStatus(T_Super::setOn(on))); return st; }
DwgString   DwgDbLight::GetName () const { return T_Super::name(); }
double      DwgDbLight::GetFalloffAngle () const { return T_Super::falloffAngle(); }
double      DwgDbLight::GetHotspotAngle () const { return T_Super::hotspotAngle(); }
DwgGiDrawable::DrawableType DwgDbLight::GetLightType () const { return static_cast<DwgGiDrawable::DrawableType>(T_Super::lightType()); }
DwgGiLightAttenuationCR DwgDbLight::GetLightAttenuation () const { return static_cast<DwgGiLightAttenuationCR>(T_Super::lightAttenuation()); }
DwgCmColor  DwgDbLight::GetLightColor () const { return static_cast<DwgCmColor>(T_Super::lightColor()); }
double      DwgDbLight::GetIntensity () const { return T_Super::intensity(); }
double      DwgDbLight::GetPhysicalIntensity() const { return T_Super::physicalIntensity(); }
DwgDbLight::IntensityBy DwgDbLight::GetPhysicalIntensityMethod () const { return static_cast<IntensityBy>(T_Super::physicalIntensityMethod()); }
DPoint3d    DwgDbLight::GetPosition () const { return Util::DPoint3dFrom(T_Super::position()); }
DPoint3d    DwgDbLight::GetTargetLocation () const { return Util::DPoint3dFrom(T_Super::targetLocation()); }
bool        DwgDbLight::HasTarget () const { return T_Super::hasTarget(); }
DVec3d      DwgDbLight::GetLightDirection () const { return Util::DVec3dFrom(T_Super::lightDirection()); }
double      DwgDbLight::GetIlluminanceDistance() const { return T_Super::illuminanceDistance(); }
DwgDbLight::LampColorBy DwgDbLight::GetLampColorType () const { return static_cast<LampColorBy>(T_Super::lampColorType()); }
double DwgDbLight::GetLampColorTemperature () const { return T_Super::lampColorTemp(); }
DwgDbLight::PresetColor DwgDbLight::GetPresetLampColor () const { return static_cast<PresetColor>(T_Super::lampColorPreset()); }
DwgDbLight::GlyphDisplay DwgDbLight::GetGlyphDisplay () const { return static_cast<GlyphDisplay>(T_Super::glyphDisplay()); }
bool   DwgDbLight::IsPlottable () const { return T_Super::isPlottable(); }

uint32_t    DwgDbRegion::GetNumChanges () const { return T_Super::numChanges(); }
DwgDbStatus DwgDbRegion::GetArea (double& area) const { return ToDwgDbStatus(T_Super::getArea(area)); }

uint32_t    DwgDbBody::GetNumChanges () const { return T_Super::numChanges(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDb3dSolid::MassProperties::MassProperties ()
    {
    m_volume = 0.0;
    m_centroid.Zero ();
    ::memset (m_inertiaMoments, 0, sizeof(m_inertiaMoments));
    ::memset (m_inertiaProducts, 0, sizeof(m_inertiaProducts));
    ::memset (m_principleMoments, 0, sizeof(m_principleMoments));
    ::memset (m_principleAxes, 0, sizeof(m_principleAxes));
    ::memset (m_gyrationRadii, 0, sizeof(m_gyrationRadii));
    m_extents.Init ();
    }
DwgDbStatus DwgDb3dSolid::GetMassProperties (MassProperties& out) const
    {
    double  volume, momInertia[3], prodInertia[3], prinMoments[3], radiiGyration[3];
    DWGGE_Type(Point3d) centroid;
    DWGGE_Type(Vector3d) prinAxes[3];
    DWGDB_SDKNAME(OdGeExtents3d,AcDbExtents) extents;

    DwgDbStatus status = ToDwgDbStatus (T_Super::getMassProp(volume, centroid, momInertia, prodInertia, prinMoments, prinAxes, radiiGyration, extents));

    if (status == DwgDbStatus::Success)
        {
        DRange3d    range = Util::DRange3dFrom (extents);
        out.SetVolume (volume);
        out.SetCentroid (Util::DPoint3dFrom(centroid));
        out.SetInertiaMoments (momInertia);
        out.SetInertiaProducts (prodInertia);
        out.SetPrincipleMoments (prinMoments);
        out.SetPrincipleAxes (Util::DVec3dCPFrom(prinAxes));
        out.SetGyrationRadii (radiiGyration);
        out.SetExtents (range);
        }
    return  status;
    }
uint32_t    DwgDb3dSolid::GetNumChanges () const { return T_Super::numChanges(); }
DwgDbStatus DwgDb3dSolid::GetArea (double& area) const { return ToDwgDbStatus(T_Super::getArea(area)); }

/*---------------------------------------------------------------------------------------
Methods of AcDbSurface not exported from RealDWG cause unresolved symbol link errors.
---------------------------------------------------------------------------------------*/
#ifdef DWGTOOLKIT_RealDwg
Acad::ErrorStatus AcDbSurface::createInterferenceObjects(AcArray<AcDbEntity*>& o, AcDbEntity* e, unsigned int flags ) const { return createInterferenceObjects(o, e, flags); }
Acad::ErrorStatus AcDbSurface::booleanUnion(const AcDbSurface* pSurface2, AcDbSurface*& pNewSurface) { return booleanUnion(pSurface2, pNewSurface); }
Acad::ErrorStatus AcDbSurface::booleanSubtract(const AcDbSurface* pSurface2, AcDbSurface*& pNewSurface) { return booleanSubtract(pSurface2, pNewSurface); }
Acad::ErrorStatus AcDbSurface::booleanSubtract(const AcDb3dSolid* pSolid, AcDbSurface*& pNewSurface) { return booleanSubtract(pSolid, pNewSurface); }
Acad::ErrorStatus AcDbSurface::booleanIntersect(const AcDbSurface* pSurface2, AcArray<AcDbEntity*>& out) { return booleanIntersect(pSurface2, out); }
Acad::ErrorStatus AcDbSurface::booleanIntersect(const AcDb3dSolid* pSolid, AcArray<AcDbEntity*>& out) { return booleanIntersect(pSolid, out); }
Acad::ErrorStatus AcDbSurface::imprintEntity(const AcDbEntity* pEntity) { return imprintEntity(pEntity); }
Acad::ErrorStatus AcDbSurface::createSectionObjects(const AcGePlane& plane, AcArray<AcDbEntity*>& out) const { return createSectionObjects(plane, out); }
Acad::ErrorStatus AcDbSurface::sliceByPlane(const AcGePlane& plane, AcDbSurface*& s1, AcDbSurface*& s2) { return sliceByPlane(plane, s1, s2); }
Acad::ErrorStatus AcDbSurface::sliceBySurface(const AcDbSurface* s1, AcDbSurface*& s2, AcDbSurface*& s3) { return sliceBySurface(s1, s2, s3); }
Acad::ErrorStatus AcDbSurface::chamferEdges(const AcArray<AcDbSubentId *>& edges, const AcDbSubentId& face, double d1, double d2) { return chamferEdges(edges, face, d1, d2); }
Acad::ErrorStatus AcDbSurface::filletEdges(const AcArray<AcDbSubentId *>& edges, const AcGeDoubleArray& r, const AcGeDoubleArray& s, const AcGeDoubleArray& e) { return filletEdges(edges, r, s, e); }
Acad::ErrorStatus AcDbSurface::getPlane(AcGePlane& plane, AcDb::Planarity& planarity) const { return getPlane(plane, planarity); }
Acad::ErrorStatus AcDbSurface::setSubentColor(const AcDbSubentId& subentId, const AcCmColor& color) { return setSubentColor(subentId, color); }
Acad::ErrorStatus AcDbSurface::getSubentColor(const AcDbSubentId& subentId, AcCmColor& color) const { return getSubentColor(subentId, color); }
Acad::ErrorStatus AcDbSurface::setSubentMaterial(const AcDbSubentId& subentId, const AcDbObjectId& matId) { return setSubentMaterial(subentId, matId); }
Acad::ErrorStatus AcDbSurface::getSubentMaterial(const AcDbSubentId& subentId, AcDbObjectId& matId) const { return getSubentMaterial(subentId, matId); }
Acad::ErrorStatus AcDbSurface::setSubentMaterialMapper(const AcDbSubentId& subentId, const AcGiMapper& mapper) { return setSubentMaterialMapper(subentId, mapper); }
Acad::ErrorStatus AcDbSurface::getSubentMaterialMapper(const AcDbSubentId& subentId, AcGiMapper& mapper) const { return getSubentMaterialMapper(subentId, mapper); }
Acad::ErrorStatus AcDbSurface::getArea(double& area) const { return getArea(area); }
Acad::ErrorStatus AcDbSurface::dwgInFields(AcDbDwgFiler* filer) { return dwgInFields(filer); }
Acad::ErrorStatus AcDbSurface::dwgOutFields(AcDbDwgFiler* filer) const { return dwgOutFields(filer); }
Acad::ErrorStatus AcDbSurface::dxfInFields(AcDbDxfFiler* filer) { return dxfInFields(filer); }
Acad::ErrorStatus AcDbSurface::dxfOutFields(AcDbDxfFiler* filer) const { return dxfOutFields(filer); }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbNurbSurface::IsPlanar(bool& planar, DPoint3dR pointOut, DVec3dR normalOut) const 
    {
    DWGGE_Type(Point3d) gePoint;
    DWGGE_Type(Vector3d) geVector;
    auto status = T_Super::isPlanar (planar, gePoint, geVector);
    if (ToDwgDbStatus(status) == DwgDbStatus::Success)
        {
        pointOut = Util::DPoint3dFrom (gePoint);
        normalOut = Util::DVec3dFrom (geVector);
        }
    return  ToDwgDbStatus(status);
    }
DwgDbStatus DwgDbNurbSurface::GetNormal(double u, double v, DVec3dR normalOut) const
    {
    DWGGE_Type(Vector3d) geVector;
    auto status = T_Super::getNormal (u, v, geVector);
    if (ToDwgDbStatus(status) == DwgDbStatus::Success)
        normalOut = Util::DVec3dFrom (geVector);
    return ToDwgDbStatus(status);
    }
DwgDbStatus DwgDbNurbSurface::Evaluate(double u, double v, int derivDegree, DPoint3dR point, DVector3dArrayR derivatives) const 
    {
    DWGGE_Type(Vector3dArray) geVectors;
    DWGGE_Type(Point3d) gePoint;
    auto status = T_Super::evaluate (u, v, derivDegree, gePoint, geVectors);
    if (ToDwgDbStatus(status) == DwgDbStatus::Success)
        {
        point = Util::DPoint3dFrom (gePoint);
        Util::GetVectorArray (derivatives, geVectors);
        }
    return ToDwgDbStatus(status);
    }
DwgDbStatus DwgDbNurbSurface::IsPointOnSurface(DPoint3dCR test, bool& answer) const { return ToDwgDbStatus(T_Super::isPointOnSurface(Util::GePoint3dFrom(test), answer)); }
