/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DwgDbInternal.h"
#include <Bentley/Desktop/FileSystem.h>

BEGIN_DWGDB_NAMESPACE

//#define DIAGNOSTICDEBUG
//#define NOISYDEBUG
//#define DEBUG_DUMP_TOPOLOGY
//#define DEBUG_ADDING_PKGEOMETRY

#ifdef DIAGNOSTICDEBUG
#define DIAGNOSTICPRINTF        ::printf
#else
#define DIAGNOSTICPRINTF
#endif

#ifdef NOISYDEBUG
#define NOISYDEBUG_PRINTF       ::printf
#else
#define NOISYDEBUG_PRINTF
#endif

#define BR_Type(_type_)         DWG_Type(Br##_type_##)
#define GeomExtents             DWGDB_SDKNAME(OdGeExtents3d, AcDbExtents)

#define BrStatus                DWGDB_SDKNAME(OdBrErrorStatus, AcBr::ErrorStatus)
#define BrStatVal(_value_)      DWGDB_SDKNAME(OdBrErrorStatus::odbr##_value_##, AcBr::ErrorStatus::e##_value_##)
#define BrStatusOk              DWGDB_SDKNAME(OdBrErrorStatus::odbrOK, AcBr::ErrorStatus::eOk)
#define TrueOrOk                DWGDB_SDKNAME(true, AcBr::eOk)

#define BrLoopType              DWGDB_SDKNAME(BrLoopType, AcBr::LoopType)
#define BrLoopTypeVal(_value_)  DWGDB_SDKNAME(BrLoopType::odbr##_value_##, AcBr::LoopType::k##_value_##)

#define BrShellType             DWGDB_SDKNAME(BrShellType, AcBr::ShellType)
#define BrShellTypeVal(_value_) DWGDB_SDKNAME(BrShellType::odbr##_value_##, AcBr::ShellType::k##_value_##)

// Parasolid's fixed size box: 1000x1000x1000
static double                   s_max_parasolidSizeBox = 1000.0;


#if defined (BENTLEYCONFIG_PARASOLID)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          05/14
+===============+===============+===============+===============+===============+======*/
class BrepConverter
{
typedef bvector <int>                       ParasolidTagList;
typedef ParasolidTagList*                   ParasolidTagListP;
typedef ParasolidTagList&                   ParasolidTagListR;
typedef bvector <ParasolidTagList>          ParasolidRegionList;
typedef bvector <PK_CLASS_t>                ParasolidClassList;
typedef bvector <PK_TOPOL_sense_t>          ParasolidSenseList;
typedef bpair <void*, int>                  AcBrepToPsClassEntry;
typedef bmap <void*, int>                   AcBrepToPsClassMap;
typedef bpair <PK_ENTITY_t, PK_LOGICAL_t>   IsPositiveOrientationEntry;
typedef bmap <PK_ENTITY_t, PK_LOGICAL_t>    IsPositiveOrientationMap;

private:
    bool                        m_needTransformation;
    DWGGE_Type(Matrix3d)        m_toParasolidMatrix;
    double                      m_toParasolidScale;
    double                      m_vertexTolerance;
    bool                        m_debugReportStarted;
    bool                        m_isSheetBody;
    bool                        m_hasAnalyticSurface;
    // the Brep extracted and cached here from an input DWG entity
    BR_Type(Brep)*              m_acBrepEntity;
    // output Parasolid topology data
    ParasolidClassList          m_pkClasses;
    ParasolidTagList            m_pkParents;
    ParasolidTagList            m_pkChildren;
    ParasolidSenseList          m_pkSenses;
    ParasolidTagList            m_pkGeometryTags;
    IsPositiveOrientationMap    m_isFaceOriented2Surface;
    IsPositiveOrientationMap    m_isEdgeOriented2Curve;
    // intermittent lookup maps to search for a Parasolid class from an AcBr entity
    AcBrepToPsClassMap          m_acBodyToClassIndex;
    AcBrepToPsClassMap          m_acComplexToClassIndex;
    AcBrepToPsClassMap          m_acShellToClassIndex;
    AcBrepToPsClassMap          m_acFaceToClassIndex;
    AcBrepToPsClassMap          m_acLoopToClassIndex;
    AcBrepToPsClassMap          m_acEdgeToClassIndex;
    AcBrepToPsClassMap          m_acVertexToClassIndex;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    InitParasolidData ()
    {
    m_pkClasses.clear ();
    m_pkParents.clear ();
    m_pkChildren.clear ();
    m_pkSenses.clear ();
    m_pkGeometryTags.clear ();
    m_isFaceOriented2Surface.clear ();
    m_isEdgeOriented2Curve.clear ();
    m_acBrepEntity = nullptr;
    m_acBodyToClassIndex.clear ();
    m_acComplexToClassIndex.clear ();
    m_acShellToClassIndex.clear ();
    m_acFaceToClassIndex.clear ();
    m_acLoopToClassIndex.clear ();
    m_acEdgeToClassIndex.clear ();
    m_acVertexToClassIndex.clear ();
    m_needTransformation = false;
    m_toParasolidMatrix.setToIdentity ();
    m_toParasolidScale = 1.0;
    m_vertexTolerance = -1.0;
    m_debugReportStarted = false;
    m_isSheetBody = false;
    m_hasAnalyticSurface = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
static char     SenseToChar (PK_TOPOL_sense_t pkSense)
    {
    // debug topological sense
    char    sign = ' ';
    if (pkSense == PK_TOPOL_sense_positive_c)
        sign = '+';
    else if (pkSense == PK_TOPOL_sense_negative_c)
        sign = '-';
    return  sign;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
static char     OrientationToChar (PK_LOGICAL_t oriented2Geometry)
    {
    // debug topological entity orientation with respect to its geometry
    return  oriented2Geometry ? '+' : '-';
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
static const char*  LooptypeToString (BrLoopType loopType)
    {
    switch (loopType)
        {
#ifdef DWGTOOLKIT_OpenDwg
        case BrLoopType::odbrLoopUnclassified:  return  "unclassified";
        case BrLoopType::odbrLoopExterior:      return  "exterior";
        case BrLoopType::odbrLoopInterior:      return  "interior";
        case BrLoopType::odbrLoopWinding:       return  "winding";
#elif DWGTOOLKIT_RealDwg
        case BrLoopType::kLoopUnclassified:     return  "unclassified";
        case BrLoopType::kLoopExterior:         return  "exterior";
        case BrLoopType::kLoopInterior:         return  "interior";
        case BrLoopType::kLoopWinding:          return  "winding";
#endif
        default:    return  "unknown";
        }
    }

#ifdef DIAGNOSTICDEBUG
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void     DiagnosticPrintAxis1 (PK_AXIS1_sf_s const& pkAxis1, TransformCP transform = nullptr, bool endLine = false)
    {
    // debugging PK_AXIS1_sf_s
    DPoint3d    point = DPoint3d::From (pkAxis1.location.coord[0], pkAxis1.location.coord[1], pkAxis1.location.coord[2]);

    if (nullptr != transform)
        transform->Multiply (point);

    ::printf ("at {%g,%g,%g}, x{%g,%g,%g}", point.x, point.y, point.z, pkAxis1.axis.coord[0], pkAxis1.axis.coord[1], pkAxis1.axis.coord[2]);

    if (endLine)
        ::printf ("\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void     DiagnosticPrintAxis2 (PK_AXIS2_sf_s const& pkAxis2, TransformCP transform = nullptr, bool endLine = false)
    {
    // debugging PK_AXIS2_sf_s
    DPoint3d    point = DPoint3d::From (pkAxis2.location.coord[0], pkAxis2.location.coord[1], pkAxis2.location.coord[2]);

    if (nullptr != transform)
        transform->Multiply (point);

    ::printf ("at {%g,%g,%g}, z{%g,%g,%g}, x{%g,%g,%g}", point.x, point.y, point.z,
            pkAxis2.axis.coord[0], pkAxis2.axis.coord[1], pkAxis2.axis.coord[2],
            pkAxis2.ref_direction.coord[0], pkAxis2.ref_direction.coord[1], pkAxis2.ref_direction.coord[2]);

    if (endLine)
        ::printf ("\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/14
+---------------+---------------+---------------+---------------+---------------+------*/
static const char*  ClassToString (PK_CLASS_t topologyClass)
    {
    static char     classNumber[50] = { 0 };

    switch (topologyClass)
        {
        case PK_CLASS_body:     return  "Body";
        case PK_CLASS_region:   return  "Region";
        case PK_CLASS_shell:    return  "Shell";
        case PK_CLASS_face:     return  "Face";
        case PK_CLASS_loop:     return  "Loop";
        case PK_CLASS_edge:     return  "Edge";
        case PK_CLASS_vertex:   return  "Vertex";
        // topological class types we do not use to create body from ASM
        case PK_CLASS_fin:      return  "Fin";
        case PK_CLASS_part:     return  "Part";
        case PK_CLASS_assembly: return  "Assembly";
        case PK_CLASS_instance: return  "Instance";
        // geometrical classes
        case PK_CLASS_curve:    return  "Curve";
        case PK_CLASS_line:     return  "Line";
        case PK_CLASS_circle:   return  "Circle";
        case PK_CLASS_ellipse:  return  "Ellipse";
        case PK_CLASS_bcurve:   return  "BSplineCurve";
        case PK_CLASS_icurve:   return  "IntersectCurve";
        case PK_CLASS_fcurve:   return  "ForeignCurve";
        case PK_CLASS_spcurve:  return  "SP-Curve";
        case PK_CLASS_trcurve:  return  "TrimmedCurve";
        case PK_CLASS_cpcurve:  return  "CP-Curve";
        default:
            ::sprintf (classNumber, "class=%d", (int)topologyClass);
            return  classNumber;
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IsOrientToCurve (const BR_Type(Edge)& acEdge) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return acEdge.getOrientToCurve ();
#elif DWGTOOLKIT_RealDwg
    Adesk::Boolean  orientationPerASM = false;
    if (BrStatus::eOk == acEdge.getOrientToCurve(orientationPerASM))
        return orientationPerASM;
    else
        return  false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    GetPoint (DWGGE_TypeR(Point3d) pointOut, BR_Type(Vertex) const& vertex) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    pointOut = vertex.getPoint ();
    return  true;
#elif DWGTOOLKIT_RealDwg
    return Acad::eOk == vertex.getPoint (pointOut);
#endif    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    FixCurveOrientation (DWGGE_TypeP(Curve3d) acCurve, BR_Type(Edge)* acEdge)
    {
    /*--------------------------------------------------------------------------------------------------------------
    This is work around a likely bug in RealDWG R2015 in which AcBrEdge::getOrientToCurve may return us an incorrect 
    flag.  The ARX doc says that the flag should be true if the curve parameterization runs in the same direction as 
    the edge direction, i.e. from vertex1 to vertex2.  But that is not always true, as a case found in TFS# 126992.
    Since we cannot reset the flag for the edge, we check the actual orientation of the curve against what is returned
    from getOrientToCurve.  Then we will revert curve parameterization if the two don't match.  We resort to invert
    the curve due to lack of a set method in AcBrEdge to reset the flag.
    --------------------------------------------------------------------------------------------------------------*/
    BR_Type(Vertex)  vertex1, vertex2;
    DWGGE_Type(Point3d) edgePoint1, edgePoint2, curvePoint1, curvePoint2;
    bool orientationPerASM = this->IsOrientToCurve (*acEdge);

    if (acCurve->isClosed())
        {
        // ASM makes a closed curve to always oriented to the edge? TFS# 131913.
        if (!orientationPerASM)
            acCurve->reverseParam ();
        }
    else if (acCurve->hasStartPoint(curvePoint1) && acCurve->hasEndPoint(curvePoint2) &&
        TrueOrOk == acEdge->getVertex1(vertex1) && TrueOrOk == acEdge->getVertex2(vertex2) &&
        this->GetPoint(edgePoint1, vertex1) && this->GetPoint(edgePoint2, vertex2))
        {
        DWGGE_Type(Vector3d)    edgeVector = edgePoint2 - edgePoint1;
        DWGGE_Type(Vector3d)    curveVector = curvePoint2 - curvePoint1;

        edgeVector.normalize ();
        curveVector.normalize ();

        auto actualOrientation = edgeVector.dotProduct(curveVector) > 0.0;

        if (orientationPerASM != actualOrientation)
            acCurve->reverseParam ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    SetBasisAxes (PK_AXIS2_sf_t& outAxes, DWGGE_TypeCR(Point3d) inOrigin, DWGGE_TypeCR(Vector3d) inAxis1, DWGGE_TypeCR(Vector3d) inAxis2)
    {
    // set two mutually perpenticular axes at a point for Parasolid
    if (m_needTransformation)
        {
        DWGGE_Type(Point3d) transformedOrigin = inOrigin;
        transformedOrigin.transformBy (m_toParasolidMatrix);

        memcpy (&outAxes.location.coord[0], &transformedOrigin, sizeof outAxes.location.coord);
        }
    else
        {
        memcpy (&outAxes.location.coord[0], &inOrigin, sizeof outAxes.location.coord);
        }

    memcpy (&outAxes.axis.coord[0], &inAxis1, sizeof outAxes.axis.coord);
    memcpy (&outAxes.ref_direction.coord[0], &inAxis2, sizeof outAxes.ref_direction.coord);

    PK_VECTOR_normalise (outAxes.axis, &outAxes.axis);
    PK_VECTOR_normalise (outAxes.ref_direction, &outAxes.ref_direction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
BrStatus    GetNativeCurve (DWGGE_TypeP(Curve3d)& nativeCurve, DWGGE_TypeCP(Curve3d) acCurve)
    {
    BrStatus    status = BrStatVal(MissingGeometry);

    if (acCurve->type() == DWGGE_Type(::kExternalCurve3d))
        {
        auto externalCurve = static_cast<DWGGE_TypeCP(ExternalCurve3d)> (acCurve);

        if (nullptr != externalCurve && externalCurve->isDefined())
            {
            /*---------------------------------------------------------------------------------------------
            A temporary workaround to prevent AcGeExternalCurve3d::isNativeCurve from crashing on degenerated
            curves as reproduced in TFS# 15528.  We will verify Adesk's fix upon availability of R2017 and 
            remove this code.
            ---------------------------------------------------------------------------------------------*/
            DWGGE_Type(Interval)    interval;
            externalCurve->getInterval (interval);

            double  length = interval.length();
            DWGGE_Type(::EntityId)  degenerateType;
            if (externalCurve->isDegenerate(degenerateType))
                return  BrStatVal(MissingGeometry);

            if (externalCurve->isNativeCurve(nativeCurve))
                status = BrStatusOk;
            }
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
BrStatus    GetNativeCurve (DWGGE_TypeP(Curve3d)& acCurve, DWGGE_TypeP(Curve3d)& nativeCurve, const BR_Type(Edge)* acEdge)
    {
    BrStatus   status = acEdge->getCurve (acCurve);
    if (BrStatusOk != status)
        return  status;

    status = this->GetNativeCurve (nativeCurve, acCurve);

    if (BrStatusOk != status)
        {
        if (nullptr != acCurve)
            delete acCurve;
        if (nullptr != nativeCurve)
            delete nativeCurve;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
BrStatus   GetNativeSurface (DWGGE_TypeP(Surface)& acSurface, DWGGE_TypeP(Surface)& nativeSurface, const BR_Type(Face)* acFace)
    {
#ifdef DWGTOOLKIT_OpenDwg
    acSurface = acFace->getSurface ();
    if (nullptr == acSurface)
        return OdBrErrorStatus::odbrNullObjectPointer;

    OdBrErrorStatus status = OdBrErrorStatus::odbrMissingGeometry;

#elif DWGTOOLKIT_RealDwg

    AcBr::ErrorStatus   status = acFace->getSurface (acSurface);
    if (AcBr::eOk != status)
        return  status;
    status = AcBr::eMissingGeometry;
#endif
    
    if (acSurface->type() == DWGGE_Type(::kExternalBoundedSurface))
        {
        DWGGE_Type(ExternalSurface) externalSurface;
        auto boundedSurface = static_cast <DWGGE_TypeP(ExternalBoundedSurface)> (acSurface);
        if (nullptr != boundedSurface)
            boundedSurface->getBaseSurface (externalSurface);

        if (externalSurface.isDefined() && externalSurface.isNativeSurface(nativeSurface) && nullptr != nativeSurface)
            status = BrStatusOk;
        }

    if (BrStatusOk != status)
        {
        if (nullptr != acSurface)
            delete acSurface;
        if (nullptr != nativeSurface)
            delete nativeSurface;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/14
+---------------+---------------+---------------+---------------+---------------+------*/
int AddSingularVertex (const BR_Type(Face)& acFace)
    {
    /*-------------------------------------------------------------------------------------------------------
    For a singular loop-vertex case, Parasolid expects a singular vertex.  This method finds the surface from
    the input face, get its apex, create a Parasolid point and a vertex, add them into our list.  For a cone
    surface, its apex is obviously the needed singular point.  For other types of surfaces, since there is no
    apex point like a cone has, just get the parametric start point on the surface.
    --------------------------------------------------------------------------------------------------------*/
    DWGGE_Type(Point3d) singularPoint;
    int                 vertexIndex = -1;
    DWGGE_Type(::EntityId)  surfaceType = DWGGE_Type(::kObject);

    BrStatus   es = acFace.getSurfaceType (surfaceType);
    if (BrStatusOk != es)
        {
        DIAGNOSTICPRINTF ("Error getting surface type from which to create a singular vertex!\n");
        return  vertexIndex;
        }

    DWGGE_TypeP(Surface)    acSurface = nullptr, *nativeSurface = nullptr;
    if (BrStatusOk != (es = this->GetNativeSurface(acSurface, nativeSurface, &acFace)))
        {
        DIAGNOSTICPRINTF ("Error getting surface type from which to create a singular vertex!\n");
        return  vertexIndex;
        }

    switch (surfaceType)
        {
        case DWGGE_Type(::kCone):
            {
            // extract the apex point of the cone
            auto acCone = static_cast <DWGGE_TypeP(Cone)> (nativeSurface);
            if (nullptr == acCone)
                es = BrStatVal(InvalidObject);
            else
                singularPoint = acCone->apex ();
            break;
            }
        case DWGGE_Type(::kTorus):
            {
            auto acTorus = static_cast <DWGGE_TypeP(Torus)> (nativeSurface);
            if (nullptr == acTorus)
                {
                es = BrStatVal(InvalidObject);
                break;
                }

            // extract the surace point of {u0, v0}:
            DWGGE_Type(Point2d) params;
            double  angles[2];
            acTorus->getAnglesInU (angles[0], angles[1]);
            params.x = angles[0];
            acTorus->getAnglesInV (angles[0], angles[1]);
            params.y = angles[0];

            singularPoint = acTorus->evalPoint (params);
            break;
            }
        case DWGGE_Type(::kCylinder):
            {
            auto acCylinder = static_cast <DWGGE_TypeP(Cylinder)> (nativeSurface);
            if (nullptr == acCylinder)
                {
                es = BrStatVal(InvalidObject);
                break;
                }

            // extract the surace point of {u0, 0.0}
            DWGGE_Type(Point2d) params;
            acCylinder->getAngles (params.x, params.y);
            params.y = 0.0;

            singularPoint = acCylinder->evalPoint (params);
            break;
            }
        case DWGGE_Type(::kSphere):
            {
            auto acSphere = static_cast <DWGGE_TypeP(Sphere)> (nativeSurface);
            if (nullptr == acSphere)
                {
                es = BrStatVal(InvalidObject);
                break;
                }

            // extract the surace point of {u0, v0}:
            DWGGE_Type(Point2d) params;
            double  angles[2];
            acSphere->getAnglesInU (angles[0], angles[1]);
            params.x = angles[0];
            acSphere->getAnglesInV (angles[0], angles[1]);
            params.y = angles[0];

            singularPoint = acSphere->evalPoint (params);
            break;
            }
        case DWGGE_Type(::kPlane):
            {
            // extract the origin of the plane
            auto plane = static_cast <DWGGE_TypeP(PlanarEnt)> (nativeSurface);
            if (nullptr == plane)
                es = BrStatVal(InvalidObject);
            else
                singularPoint = plane->pointOnPlane ();
            break;
            }
        default:
            // try extracting the point of {0,0} on any other type of surface:
            singularPoint = nativeSurface->evalPoint (DWGGE_Type(Point2d)(0.0, 0.0));
            break;
        }

    if (BrStatusOk == es)
        {
        if (m_needTransformation)
            singularPoint.transformBy (m_toParasolidMatrix);

        // create a Parasolid point
        PK_POINT_sf_t   pointParams;
        memcpy (&pointParams.position.coord[0], &singularPoint, sizeof pointParams.position.coord);

        PK_POINT_t      pointTag = -1;
        PK_ERROR_code_t pkError = PK_POINT_create (&pointParams, &pointTag);

#ifdef DIAGNOSTICDEBUG
        if (m_needTransformation)
            singularPoint.transformBy (m_toParasolidMatrix.inverse());
#endif

        if (PK_ERROR_no_errors == pkError)
            {
            // add the point
            m_pkGeometryTags.push_back ((int)pointTag);

            vertexIndex = (int)(m_pkClasses.size());

            // add a vertex class
            m_pkClasses.push_back (PK_CLASS_vertex);

            NOISYDEBUG_PRINTF ("\t\t\t\t\tSingular Point[%d@%d] at {%f,%f,%f}\n", (int)pointTag, vertexIndex, singularPoint.x, singularPoint.y, singularPoint.z);
            }
        else
            {
            DIAGNOSTICPRINTF ("Error creating a singular PK_POINT at {%g,%g,%g}: %d!\n", singularPoint.x, singularPoint.y, singularPoint.z, pkError);
            vertexIndex = -2;
            }
        }

    if (nullptr != acSurface)
        delete acSurface;
    if (nullptr != nativeSurface)
        delete nativeSurface;

    return  vertexIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CreateVertexGeometry (BR_Type(Entity)* brEntity)
    {
    auto acVertex = dynamic_cast <BR_Type(Vertex)*> (brEntity);
    DWGGE_Type(Point3d) acPoint;
    if (nullptr == acVertex || !this->GetPoint(acPoint, *acVertex))
        return  BSIERROR;

    if (m_needTransformation)
        acPoint.transformBy (m_toParasolidMatrix);

    PK_POINT_sf_t   pointParams;
    memcpy (&pointParams.position.coord[0], &acPoint, sizeof pointParams.position.coord);
    
    PK_POINT_t      pointTag = -1;
    PK_ERROR_code_t pkError = PK_POINT_create (&pointParams, &pointTag);
    
    if (PK_ERROR_no_errors == pkError)
        m_pkGeometryTags.push_back ((int)pointTag);
    else
        DIAGNOSTICPRINTF ("Error creating a PK_POINT at {%g,%g,%g}: %d!\n", acPoint.x, acPoint.y, acPoint.z, pkError);

#ifdef DEBUG_ADDING_PKGEOMETRY
    DWGGE_Type(Point3d) checkPoint;
    acVertex->getPoint (checkPoint);
    ::printf ("\t\t\t\t\tPoint[%d@%lld] at {%.10f,%.10f,%.10f}\n", (int)pointTag, m_pkClasses.size(), checkPoint.x, checkPoint.y, checkPoint.z);
#endif

    return  PK_ERROR_no_errors == pkError ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CreateEdgeLine (DWGGE_Type(LinearEnt3d)* acLinear)
    {
    PK_ERROR_code_t pkError = PK_ERROR_cant_get_curve;
    if (nullptr == acLinear)
        return  pkError;

    auto point = acLinear->pointOnLine ();
    auto direction = acLinear->direction ();
    if (m_needTransformation)
        point.transformBy (m_toParasolidMatrix);

    PK_LINE_sf_t    lineParams;
    memcpy (&lineParams.basis_set.location.coord[0], &point, sizeof(lineParams.basis_set.location));
    memcpy (&lineParams.basis_set.axis.coord[0], &direction, sizeof(lineParams.basis_set.axis));

    PK_LINE_t   lineTag = -1;
    if (PK_ERROR_no_errors == (pkError = PK_LINE_create(&lineParams, &lineTag)))
        m_pkGeometryTags.push_back ((int)lineTag);
    else
        DIAGNOSTICPRINTF ("Parasolid error creating a PK_LINE at {%g,%g,%g}: %d!\n", acLinear->pointOnLine().x, acLinear->pointOnLine().y, acLinear->pointOnLine().z, pkError);

#ifdef DEBUG_ADDING_PKGEOMETRY
    ::printf ("\t\t\t\tLine[%d@%lld] at {%f,%f,%f}, x{%g,%g,%g}\n", (int)lineTag, m_pkClasses.size(), 
        acLinear->pointOnLine().x, acLinear->pointOnLine().y, acLinear->pointOnLine().z, direction.x, direction.y, direction.z);
#endif

    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CheckBSplineCurve (PK_BCURVE_sf_t const& bcurveParams, PK_BCURVE_t& bcurveTag)
    {
    // fix Parasolid's G1 discontinuity potentially resulted from bad ASM data.
    int                 numDiscs = 0;
    double*             discsAt = nullptr;
    PK_INTERVAL_t       range;

    auto pkError = PK_BCURVE_find_g1_discontinuity (bcurveTag, &numDiscs, &discsAt);

    if (PK_ERROR_no_errors == pkError && numDiscs > 0 && PK_ERROR_no_errors == (pkError = PK_CURVE_ask_interval(bcurveTag, &range)))
        {
        PK_BCURVE_t                     newCurveTag = -1;
        PK_BCURVE_fitted_fault_t        fitResult;
        PK_BCURVE_create_fitted_o_t     fitOptions;
        PK_BCURVE_create_fitted_o_m (fitOptions);

        fitOptions.curve.type = PK_CURVE_general_curve_c;
        fitOptions.curve.curve.parasolid_curve = bcurveTag;
        fitOptions.range = range;
        
        pkError = PK_BCURVE_create_fitted (&fitOptions, &newCurveTag, &fitResult);

        if (PK_ERROR_no_errors == pkError && PK_BCURVE_fitted_success_c == fitResult.status && PK_ENTITY_null != newCurveTag)
            {
            // fitting succeeded - replace the old curve
            PK_ENTITY_delete (1, &bcurveTag);
            bcurveTag = newCurveTag;
            }
        else if (newCurveTag != PK_ENTITY_null)
            {
            // clean up on fitting failure
            PK_ENTITY_delete (1, &newCurveTag);
            pkError = PK_ERROR_cant_make_bspline;
            }
        }

    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CreateEdgeBCurve (DWGGE_TypeP(SplineEnt3d) acSpline)
    {
    PK_ERROR_code_t pkError = PK_ERROR_cant_get_curve;
    if (nullptr == acSpline)
        return  pkError;

    PK_BCURVE_sf_t  bcurveParams;

    bcurveParams.degree = acSpline->degree ();
    bcurveParams.n_vertices = acSpline->numControlPoints ();
    bcurveParams.is_rational = acSpline->isRational ();
    bcurveParams.is_closed = acSpline->isClosed ();

    // validate rational spline curve
    DWGGE_TypeP(NurbCurve3d) acNurbCurve = nullptr;
    if (bcurveParams.is_rational)
        acNurbCurve = static_cast <DWGGE_TypeP(NurbCurve3d)> (acSpline);
    if (nullptr == acNurbCurve || acNurbCurve->numWeights() < bcurveParams.n_vertices)
        bcurveParams.is_rational = PK_LOGICAL_false;

    if (nullptr != acNurbCurve)
        {
        // bail out on an ASM, as opposed to Parasolid, spline discountinuity - TFS 153151:
        DWGGE_Type(DoubleArray) discs;
        if (acNurbCurve->getParamsOfG1Discontinuity(discs))
            {
            DIAGNOSTICPRINTF ("NURB curve error: a G1 discontinuity found at %g!\n", discs.at(0));
            return  PK_ERROR_discontinuous_curve;
            }
        if (acNurbCurve->getParamsOfC1Discontinuity(discs))
            {
            DIAGNOSTICPRINTF ("NURB curve error: a C1 discontinuity found at %g!\n", discs.at(0));
            return  PK_ERROR_discontinuous_curve;
            }
        }

    bcurveParams.vertex_dim = bcurveParams.is_rational ? 4 : 3;
    bcurveParams.knot_type = PK_knot_unset_c;
    bcurveParams.form = PK_BCURVE_form_unset_c;
    bcurveParams.self_intersecting = PK_self_intersect_unset_c;

    /*--------------------------------------------------------------------------------------------
    One of Parasolids restrictions on a BCURVE is that, if a B-curve is to be attached to an edge:
        -It must, at the minimum, have G1 continuity.
        -If it is closed, it must have periodic parameterization.

    So we force a closed bcurve to be periodic.  This may push the body to fail but at least won't
    create bad body with horrible rendering like an example of a surface from a cylinder shell cut
    by another perpendicular cylinder.
    --------------------------------------------------------------------------------------------*/
    bcurveParams.is_periodic = bcurveParams.is_closed;

    // Parasolid expects distinct knots
    DWGGE_Type(DoubleArray) distinctKnots;
    DWGGE_Type(KnotVector)  knots = acSpline->knots ();

    knots.getDistinctKnots (distinctKnots);

    bcurveParams.n_knots = distinctKnots.length ();

    bcurveParams.vertex = new double[bcurveParams.n_vertices * bcurveParams.vertex_dim * sizeof(double)];
    if (nullptr == bcurveParams.vertex)
        return  PK_ERROR_memory_full;

    bcurveParams.knot = new double[bcurveParams.n_knots * sizeof(double)];
    if (nullptr == bcurveParams.knot)
        {
        delete [] bcurveParams.vertex;
        return  PK_ERROR_memory_full;
        }
    
    bcurveParams.knot_mult = new int[bcurveParams.n_knots * sizeof(int)];
    if (nullptr == bcurveParams.knot_mult)
        {
        delete [] bcurveParams.vertex;
        delete [] bcurveParams.knot;
        return  PK_ERROR_memory_full;
        }

    // copy control points
    for (int i = 0; i < bcurveParams.n_vertices; i++)
        {
        uint32_t  pkIndex = i * bcurveParams.vertex_dim;
        auto    pole = acSpline->controlPointAt (i);

        if (m_needTransformation)
            pole.transformBy (m_toParasolidMatrix);

        // copy control point at the index
        memcpy (&bcurveParams.vertex[pkIndex], &pole, 3 * sizeof(double));
        // copy weight if rational
        if (4 == bcurveParams.vertex_dim)
            {
            double  weight = bcurveParams.vertex[pkIndex + 3] = acNurbCurve->weightAt (i);

            // Parasolid expects homogenous/weighted poles
            bcurveParams.vertex[pkIndex + 0] *= weight;
            bcurveParams.vertex[pkIndex + 1] *= weight;
            bcurveParams.vertex[pkIndex + 2] *= weight;
            }
        }

    // copy distinct knots and knot multiplicity
    for (int i = 0; i < bcurveParams.n_knots; i++)
        {
        bcurveParams.knot[i] = distinctKnots[i];
        bcurveParams.knot_mult[i] = knots.multiplicityAt (distinctKnots[i]);
        }

    PK_BCURVE_t bcurveTag = -1, tagSaved = -1;
    if (PK_ERROR_no_errors == (pkError = PK_BCURVE_create(&bcurveParams, &bcurveTag)) && PK_ENTITY_null != (tagSaved = bcurveTag) &&
        PK_ERROR_no_errors == (pkError = this->CheckBSplineCurve(bcurveParams, bcurveTag)))
        m_pkGeometryTags.push_back ((int)bcurveTag);
    else
        DIAGNOSTICPRINTF ("Parasolid error creating a PK_BCURVE(%d %d): %d!\n", bcurveParams.degree, bcurveParams.n_vertices, pkError);

#ifdef DEBUG_ADDING_PKGEOMETRY
    if (PK_ERROR_no_errors == pkError)
        {
        int bcurveIndex = (int)m_pkGeometryTags.size() - 1;
        ::printf ("\t\t\t\tBCurve[%d@%d]: degree=%d, nPoles=%d, nKnots=%d, %s rational\n", (int)bcurveTag, bcurveIndex,
                bcurveParams.degree, bcurveParams.n_vertices, bcurveParams.n_knots, bcurveParams.is_rational ? "is" : "not");

        if (tagSaved != bcurveTag)
            {
            PK_BCURVE_sf_t  newParams;
            ::printf ("\t\t\t\t=>Replaced by[%d]", bcurveTag);
            if (PK_ERROR_no_errors == PK_BCURVE_ask(bcurveTag, &newParams))
                ::printf (": degree=%d, nPoles=%d, nKnots=%d, %s rational", newParams.degree, newParams.n_vertices, newParams.n_knots, newParams.is_rational ? "is" : "not");
            ::printf ("\n");
            }
        }
#endif

    delete [] bcurveParams.vertex;
    delete [] bcurveParams.knot;
    delete [] bcurveParams.knot_mult;
    
    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CreateEdgeCircle (DWGGE_TypeP(CircArc3d) acCircArc)
    {
    PK_ERROR_code_t pkError = PK_ERROR_cant_get_curve;
    if (nullptr == acCircArc)
        return  pkError;

    PK_CIRCLE_sf_t  circleParams;
    this->SetBasisAxes (circleParams.basis_set, acCircArc->center(), acCircArc->normal(), acCircArc->refVec());

    circleParams.radius = acCircArc->radius ();
    if (m_needTransformation)
        circleParams.radius *= m_toParasolidScale;

    PK_CIRCLE_t     circleTag = -1;
    if (PK_ERROR_no_errors == (pkError = PK_CIRCLE_create(&circleParams, &circleTag)))
        m_pkGeometryTags.push_back ((int)circleTag);
    else
        DIAGNOSTICPRINTF ("Parasolid error creating a PK_CIRCLE at (%g, %g, %g): %d!\n", acCircArc->center().x, acCircArc->center().y, acCircArc->center().z, pkError);

#ifdef DEBUG_ADDING_PKGEOMETRY
    if (PK_ERROR_no_errors == pkError)
        {
        int         circleIndex = (int)m_pkGeometryTags.size() - 1;
        ::printf ("\t\t\t\tCircle[%d@%d]: R=%g, ", (int)circleTag, circleIndex, acCircArc->radius());

        Transform   untransform;
        Util::GetTransform (untransform, m_toParasolidMatrix.inverse());
        DiagnosticPrintAxis2 (circleParams.basis_set, &untransform, true);
        }
#endif

    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    GetPiecewiseInfoFromCurves (PK_BCURVE_piecewise_sf_t& piecewise,
#ifdef DWGTOOLKIT_OpenDwg
    OdGeCurve3dPtrArray& acCurves
#elif DWGTOOLKIT_RealDwg
    AcGeVoidPointerArray& acCurves
#endif
)
    {
    piecewise.n_segments = acCurves.length ();
    if (piecewise.n_segments < 1)
        return  false;

    piecewise.degree = -1;
    piecewise.is_rational = false;
    piecewise.rep = PK_piecewise_rep_polynomial_c;

    for (int i = 0; i < piecewise.n_segments; i++)
        {
        auto acCurve = static_cast <DWGGE_TypeP(Curve3d)> (acCurves[i]);
        if (nullptr == acCurve)
            return  false;

        DWGGE_TypeP(Curve3d)    nativeCurve = nullptr;
        if (BrStatusOk != this->GetNativeCurve(nativeCurve, acCurve))
            {
            piecewise.degree = -1;
            break;
            }

        auto extCurve = static_cast <DWGGE_TypeP(ExternalCurve3d)> (nativeCurve);
        if (extCurve->isLine() || extCurve->isLineSeg())
            {
            if (piecewise.degree < 1)
                piecewise.degree = 1;
            }
        else if (extCurve->isCircArc() || extCurve->isEllipArc())
            {
            if (piecewise.degree < 2)
                piecewise.degree = 2;
            }
        else if (extCurve->isNurbCurve())
            {
            auto acSpline = (DWGGE_TypeP(SplineEnt3d)) extCurve;
            if (nullptr != acSpline)
                {
                if (acSpline->degree() > piecewise.degree)
                    piecewise.degree = acSpline->degree ();
                piecewise.is_rational = acSpline->isRational ();
                }
            }

        delete nativeCurve;

        if (piecewise.degree < 1)
            return  false;
        }

    if (piecewise.degree < 1)
        return  false;

    piecewise.dim = piecewise.is_rational ? 4 : 3;

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    FreeCurveList (DWGGE_TypeR(VoidPointerArray) acCurves)
    {
#ifdef DWGTOOLKIT_RealDwg
    for (int i = 0; i < (int)acCurves.length(); i++)
        {
        if (nullptr != acCurves[i])
            delete acCurves[i];
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CreateEdgePWCurve (DWGGE_TypeP(CompositeCurve3d) acCompositeCurve)
    {
    PK_ERROR_code_t pkError = PK_ERROR_cant_get_curve;
    if (nullptr == acCompositeCurve)
        return  pkError;

#ifdef DWGTOOLKIT_OpenDwg
    OdGeCurve3dPtrArray     acCurves;
    acCompositeCurve->getCurveList (acCurves);
#elif DWGTOOLKIT_RealDwg
    AcGeVoidPointerArray    acCurves;
    acCompositeCurve->getCurveList (acCurves);
#endif

    // scan curve list for basic piecewise info - support lineaer piecewise for now
    PK_BCURVE_piecewise_sf_t    piecewise;
    if (!this->GetPiecewiseInfoFromCurves(piecewise, acCurves) || piecewise.degree > 1)
        {
#ifdef DWGTOOLKIT_RealDwg
        this->FreeCurveList (acCurves);
#endif
        return  pkError;
        }

    // allocate data array for piecewise bcurve
    piecewise.coeffs = new double[piecewise.n_segments * (piecewise.degree + 1) * piecewise.dim * sizeof(double)];
    if (nullptr != piecewise.coeffs)
        {
#ifdef DWGTOOLKIT_RealDwg
        this->FreeCurveList (acCurves);
#endif
        return  PK_ERROR_memory_full;
        }

    pkError = PK_ERROR_no_errors;

    int     nSegments = acCurves.length ();
    for (int i = 0; i < nSegments; i++)
        {
        DWGGE_TypeP(Curve3d)    nativeCurve = nullptr;
        if (BrStatusOk != this->GetNativeCurve(nativeCurve, static_cast<DWGGE_TypeP(Curve3d)>(acCurves[i])))
            break;  // should not happen
        
        auto    extCurve = static_cast<DWGGE_TypeP(ExternalCurve3d)> (nativeCurve);
        if (extCurve->isLine() || extCurve->isLineSeg())
            {
            DWGGE_Type(Point3d) point;
            if (extCurve->hasStartPoint(point))
                {
                if (m_needTransformation)
                    point.transformBy (m_toParasolidMatrix);

                // copy the point at current index
                memcpy (&piecewise.coeffs[i], &point, 3 * sizeof(double));
                if (piecewise.is_rational)
                    piecewise.coeffs[i + 3] = 1.0;
                }
            else if (extCurve->hasEndPoint(point) && i < (nSegments - 1))
                {
                int         j = piecewise.is_rational ? i + 4 : i + 3;

                if (m_needTransformation)
                    point.transformBy (m_toParasolidMatrix);

                // copy the point at current index
                memcpy (&piecewise.coeffs[i], &point, 3 * sizeof(double));
                if (piecewise.is_rational)
                    piecewise.coeffs[i + 4] = 1.0;
                }
            }
        else
            {
            BeAssert (false && L"Nonlineaer piecewise curve not yet supported!!");
            pkError = PK_ERROR_unsupported_operation;
            break;
            }
        }

     PK_BCURVE_t    bcurveTag = -1;
    if (PK_ERROR_no_errors == pkError)
        {
        if (PK_ERROR_no_errors == (pkError = PK_BCURVE_create_piecewise(&piecewise, &bcurveTag)))
            m_pkGeometryTags.push_back ((int)bcurveTag);
        }
    
#ifdef DWGTOOLKIT_RealDwg
    this->FreeCurveList (acCurves);
#endif

    if (PK_ERROR_no_errors != pkError)
        DIAGNOSTICPRINTF ("Parasolid error creating a piecewise bcurve: %d segments, %d degrees, rep= %d!\n", piecewise.n_segments, piecewise.degree, piecewise.rep);

#ifdef DEBUG_ADDING_PKGEOMETRY
    if (PK_ERROR_no_errors == pkError)
        {
        int bcurveIndex = (int)m_pkGeometryTags.size() - 1;
        ::printf ("\t\t\t\tPiecewiseCurve[%d@%d]: %d segments, %d degrees, rep= %d\n", (int)bcurveTag, bcurveIndex,
                    piecewise.n_segments, piecewise.degree, piecewise.rep);
        }
#endif

    delete[] piecewise.coeffs;

    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CreateEdgeEllipse (DWGGE_TypeP(EllipArc3d) acEllipArc)
    {
    PK_ERROR_code_t pkError = PK_ERROR_cant_get_curve;
    if (nullptr == acEllipArc)
        return  pkError;

    PK_ELLIPSE_sf_t ellipseParams;
    this->SetBasisAxes (ellipseParams.basis_set, acEllipArc->center(), acEllipArc->normal(), acEllipArc->majorAxis());

    ellipseParams.R1 = acEllipArc->majorRadius ();
    ellipseParams.R2 = acEllipArc->minorRadius ();
    if (m_needTransformation)
        {
        ellipseParams.R1 *= m_toParasolidScale;
        ellipseParams.R2 *= m_toParasolidScale;
        }

    PK_ELLIPSE_t    ellipseTag = -1;
    if (PK_ERROR_no_errors == (pkError = PK_ELLIPSE_create(&ellipseParams, &ellipseTag)))
        m_pkGeometryTags.push_back ((int)ellipseTag);
    else
        DIAGNOSTICPRINTF ("Parasolid error creating a PK_ELLIPSE at (%g,%g,%g): %d!\n", acEllipArc->center().x, acEllipArc->center().y, acEllipArc->center().z, pkError);

#ifdef DEBUG_ADDING_PKGEOMETRY
    if (PK_ERROR_no_errors == pkError)
        {
        int ellipseIndex = (int)m_pkGeometryTags.size() - 1;
        ::printf ("\t\t\t\tEllipse[%d@%d]: R=%g, r=%g, ", (int)ellipseTag, ellipseIndex, acEllipArc->majorRadius(), acEllipArc->minorRadius());

        Transform   untransform;
        Util::GetTransform (untransform, m_toParasolidMatrix.inverse());
        DiagnosticPrintAxis2 (ellipseParams.basis_set, &untransform, true);
        }
#endif

    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CreateEdgeGeometry (BR_Type(Entity)* brEntity)
    {
#ifdef DWGTOOLKIT_OpenDwg
    auto acEdge = dynamic_cast <OdBrEdge*> (brEntity);
    OdGe::EntityId  curveType = acEdge->getCurveType ();
#elif DWGTOOLKIT_RealDwg
    AcGe::EntityId  curveType;
    auto acEdge = dynamic_cast <AcBrEdge*> (brEntity);
    if (nullptr == acEdge || AcBr::eOk != acEdge->getCurveType(curveType))
        return  BSIERROR;
#endif

    // extract native curve
    DWGGE_Type(Curve3d) *acCurve = nullptr, *nativeCurve = nullptr;
    if (BrStatusOk != this->GetNativeCurve(acCurve, nativeCurve, acEdge))
        return  BSIERROR;

    // check & invert the curve if its orientation does not match what is returned from AcBrEdge::getOrientToCurve:
    this->FixCurveOrientation (nativeCurve, acEdge);

    PK_ERROR_code_t pkError = PK_ERROR_missing_geom;

    switch (curveType)
        {
        case DWGGE_Type(::kLinearEnt3d):
        case DWGGE_Type(::kLine3d):
        case DWGGE_Type(::kLineSeg3d):
            pkError = this->CreateEdgeLine (static_cast<DWGGE_TypeP(LinearEnt3d)>(nativeCurve));
            break;
        case DWGGE_Type(::kCircArc3d):
            pkError = this->CreateEdgeCircle (static_cast<DWGGE_TypeP(CircArc3d)>(nativeCurve));
            break;
        case DWGGE_Type(::kEllipArc3d):
            pkError = this->CreateEdgeEllipse (static_cast<DWGGE_TypeP(EllipArc3d)>(nativeCurve));
            break;
        case DWGGE_Type(::kSplineEnt3d):
        case DWGGE_Type(::kCubicSplineCurve3d):
        case DWGGE_Type(::kDSpline3d):
        case DWGGE_Type(::kNurbCurve3d):
        case DWGGE_Type(::kAugPolyline3d):
        case DWGGE_Type(::kPolyline3d):
            pkError = this->CreateEdgeBCurve (static_cast<DWGGE_TypeP(SplineEnt3d)>(nativeCurve));
            break;
        case DWGGE_Type(::kCompositeCrv3d):
            pkError = this->CreateEdgePWCurve (static_cast<DWGGE_TypeP(CompositeCurve3d)>(nativeCurve));
            break;
        default:
            BeAssert (false && L"Unsupported curve type for a Brep edge!!");
        }

    delete acCurve;
    delete nativeCurve;

    return  PK_ERROR_no_errors == pkError ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CreateFaceCone (DWGGE_TypeP(Cone) acCone, bool* isOriented = nullptr)
    {
    PK_ERROR_code_t pkError = PK_ERROR_wrong_surface;
    if (nullptr == acCone)
        return  pkError;

    PK_CONE_sf_t    coneParams;
    coneParams.semi_angle = acCone->halfAngle ();
    coneParams.radius = acCone->baseRadius();

    // AcGeCone::axisOfSymmetry does not appear to have a consistent direction - find it by shooting a vector from the apex to the base center:
    auto    axis = acCone->baseCenter().asVector() - acCone->apex().asVector();
    axis.normalize ();

    // handle error
    if (axis.isZeroLength())
        axis = acCone->axisOfSymmetry ();

    if (m_needTransformation)
        coneParams.radius *= m_toParasolidScale;

    this->SetBasisAxes (coneParams.basis_set, acCone->baseCenter(), axis, acCone->refAxis());

    // fix face-surface orientation flag here as AcDbFace has no set method for that.
    if (nullptr != isOriented && !acCone->isOuterNormal())
        *isOriented = !*isOriented;
        
    PK_CONE_t   coneTag = -1;
    if (PK_ERROR_no_errors == (pkError = PK_CONE_create(&coneParams, &coneTag)))
        m_pkGeometryTags.push_back ((int)coneTag);

    m_hasAnalyticSurface = true;

    if (PK_ERROR_no_errors == pkError)
        {
#ifdef NOISYDEBUG
        ::printf ("\t\t\tCone[%d@%lld] at {%f,%f,%f}, R= %g, %cz{%g,%g,%g}\n", (int)coneTag, m_pkGeometryTags.size()-1,
                acCone->baseCenter().x, acCone->baseCenter().y, acCone->baseCenter().z, acCone->baseRadius(),
                acCone->isNormalReversed() ? '-' : '+',
                coneParams.basis_set.axis.coord[0], coneParams.basis_set.axis.coord[1], coneParams.basis_set.axis.coord[2]);
#endif
        }
    else
        {
#ifdef DIAGNOSTICDEBUG
        ::printf ("Parasolid error creating a PK_CONE for Brep face: %d!\n", pkError);
#endif
        }

    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CreateFaceEllipticCone (DWGGE_TypeP(Surface) acSurface)
    {
    PK_ERROR_code_t pkError = PK_ERROR_wrong_surface;
    if (nullptr == acSurface)
        return  pkError;

    // evaluate 3 points on the elliptic cone base
    auto majorPoint0 = acSurface->evalPoint (DWGGE_Type(Point2d)(0.0, 0.0));
    auto majorPoint1 = acSurface->evalPoint (DWGGE_Type(Point2d)(0.0, Angle::Pi()));
    auto minorPoint = acSurface->evalPoint (DWGGE_Type(Point2d)(0.0, Angle::PiOver2()));
    // evaluate the apex point of the cone
    auto apexPoint = acSurface->evalPoint (DWGGE_Type(Point2d)(1.0, 0.0));

    // the center point of the base is the mid-point of the two major points:
    auto center = 0.5 * (majorPoint0 + majorPoint1.asVector());

    // major and minor axes about the center point:
    auto majorAxis = majorPoint0 - center;
    auto minorAxis = minorPoint - center;

    // get the normal vector from major minor axes
    auto zAxis = majorAxis.crossProduct(minorAxis).normalize ();

    PK_CONE_sf_t    coneParams;
    this->SetBasisAxes (coneParams.basis_set, center, zAxis, majorAxis);

    // Parasolid does not have an elliptical cone, so let's hope it is a circular cone:
    coneParams.radius = majorAxis.length ();
    if (m_needTransformation)
        coneParams.radius *= m_toParasolidScale;

    if (fabs(coneParams.radius - minorAxis.length()) > 1.0e-5)
        DIAGNOSTICPRINTF ("ASM's elliptical cone(R= %g, r= %g) has been converted to a circular Parasolid cone!\n", majorAxis.length(), minorAxis.length());

    coneParams.semi_angle = Angle::Pi() - majorAxis.angleTo (apexPoint - majorPoint0);

    PK_CONE_t   coneTag = -1;
    if (PK_ERROR_no_errors == (pkError = PK_CONE_create(&coneParams, &coneTag)))
        m_pkGeometryTags.push_back ((int)coneTag);

    m_hasAnalyticSurface = true;

    if (PK_ERROR_no_errors == pkError)
        {
#ifdef NOISYDEBUG
        ::printf ("\t\t\tEllipCone[%d@%lld] at {%f,%f,%f}, R= %g, r= %g\n", (int)coneTag, m_pkGeometryTags.size()-1, center.x, center.y, center.z, coneParams.radius, minorAxis.length());
#endif
        }
    else
        {
#ifdef DIAGNOSTICDEBUG
        ::printf ("Parasolid error creating a PK_CONE(elliptic cone) for Brep face: %d!\n", pkError);
#endif
        }

    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CreateFacePlane (DWGGE_TypeP(Plane) acPlane)
    {
    PK_ERROR_code_t pkError = PK_ERROR_wrong_surface;
    if (nullptr == acPlane)
        return  pkError;

    DWGGE_Type(Point3d) origin;
    DWGGE_Type(Vector3d) xAxis, yAxis;
    acPlane->getCoordSystem (origin, xAxis, yAxis);

    // AcGePlaneEnt3d::normal is supposed to have taken account of AcGeSurface::isNormalReversed:
    auto zAxis = acPlane->normal ();
    zAxis.normalize ();

    PK_PLANE_sf_t   planeParams;
    this->SetBasisAxes (planeParams.basis_set, origin, zAxis, xAxis);

    PK_PLANE_t  planeTag = -1;
    if (PK_ERROR_no_errors == (pkError = PK_PLANE_create(&planeParams, &planeTag)))
        m_pkGeometryTags.push_back ((int)planeTag);

    if (PK_ERROR_no_errors == pkError)
        {
#ifdef NOISYDEBUG
        ::printf ("\t\t\tPlane[%d@%lld] at {%f,%f,%f}, x{%g,%g,%g}, y{%g,%g,%g}, %cz{%g,%g,%g}\n", (int)planeTag, m_pkGeometryTags.size()-1, origin.x, origin.y, origin.z, xAxis.x, xAxis.y, xAxis.z, yAxis.x, yAxis.y, yAxis.z, acPlane->isNormalReversed() ? '-':'+', zAxis.x, zAxis.y, zAxis.z);
#endif
        }
    else
        {
#ifdef DIAGNOSTICDEBUG
        ::printf ("Parasolid error creating a PK_PLANE for Brep face: %d!\n", pkError);
#endif
        }

    return  pkError;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t FixOrthogonalAxes (PK_AXIS2_sf_t& axes)
    {
    PK_VECTOR1_t    norm;
    PK_ERROR_code_t pkError = PK_VECTOR_perpendicular (axes.axis, axes.ref_direction, &norm);
    if (PK_ERROR_no_errors != pkError)
        return  pkError;

    pkError = PK_VECTOR_perpendicular (axes.axis, norm, &axes.ref_direction);

    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CreateFaceTorus (DWGGE_TypeP(Torus) acTorus, bool* isOriented = nullptr)
    {
    PK_ERROR_code_t pkError = PK_ERROR_wrong_surface;
    if (nullptr == acTorus)
        return  pkError;

    PK_TORUS_sf_t   torusParams;
    this->SetBasisAxes (torusParams.basis_set, acTorus->center(), acTorus->axisOfSymmetry(), acTorus->refAxis());

    torusParams.major_radius = acTorus->majorRadius ();
    torusParams.minor_radius = acTorus->minorRadius ();
    if (m_needTransformation && fabs(m_toParasolidScale - 1.0) > 1.0e-3)
        {
        torusParams.major_radius *= m_toParasolidScale;
        torusParams.minor_radius *= m_toParasolidScale;
        }
    
    // fix face-surface orientation flag here as AcDbFace has no set method for that.
    if (nullptr != isOriented && !acTorus->isOuterNormal())
        *isOriented = !*isOriented;

    PK_TORUS_t  torusTag = -1;
    if (PK_ERROR_no_errors == (pkError = PK_TORUS_create(&torusParams, &torusTag)))
        m_pkGeometryTags.push_back ((int)torusTag);

    if (PK_ERROR_vectors_not_orthogonal == pkError)
        {
        // the axes may have fallen out of Parasold's angular precision - fix the axes and try it again:
        if (PK_ERROR_no_errors == (pkError = this->FixOrthogonalAxes(torusParams.basis_set)) &&
            PK_ERROR_no_errors == (pkError = PK_TORUS_create(&torusParams, &torusTag)))
            m_pkGeometryTags.push_back ((int)torusTag);
        }

    m_hasAnalyticSurface = true;

    if (PK_ERROR_no_errors != pkError)
        DIAGNOSTICPRINTF ("Parasolid error creating a PK_TORUS for Brep face: %d!\n", pkError);

#ifdef NOISYDEBUG
    if (PK_ERROR_no_errors == pkError)
        {
        int torusIndex = (int)m_pkGeometryTags.size() - 1;
        ::printf ("\t\t\tTorus[%d@%d]: R=%g, r=%g, ", (int)torusTag, torusIndex, acTorus->majorRadius(), acTorus->minorRadius());

        Transform   untransform;
        Util::GetTransform (untransform, m_toParasolidMatrix.inverse());
        DiagnosticPrintAxis2 (torusParams.basis_set, &untransform, false);
        ::printf ("%cnormal\n", acTorus->isNormalReversed() ? '-' : '+');
        }
#endif

    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CreateFaceCylinder (DWGGE_TypeP(Cylinder) acCylinder, bool* isOriented = nullptr)
    {
    PK_ERROR_code_t pkError = PK_ERROR_wrong_surface;
    if (nullptr == acCylinder)
        return  pkError;

    PK_CYL_sf_t cylinderParams;
    this->SetBasisAxes (cylinderParams.basis_set, acCylinder->origin(), acCylinder->axisOfSymmetry(), acCylinder->refAxis());

    cylinderParams.radius = acCylinder->radius ();
    if (m_needTransformation)
        cylinderParams.radius *= m_toParasolidScale;

    // fix face-surface orientation flag here as AcDbFace has no set method for that.
    if (nullptr != isOriented && !acCylinder->isOuterNormal())
        *isOriented = !*isOriented;

    PK_CYL_t    cylinderTag = -1;
    if (PK_ERROR_no_errors == (pkError = PK_CYL_create(&cylinderParams, &cylinderTag)))
        m_pkGeometryTags.push_back ((int)cylinderTag);
    else
        DIAGNOSTICPRINTF ("Parasolid error creating a PK_CYL for Brep face: %d!\n", pkError);

    m_hasAnalyticSurface = true;

#ifdef NOISYDEBUG
    if (PK_ERROR_no_errors == pkError)
        {
        int         cylinderIndex = (int)m_pkGeometryTags.size() - 1;
        ::printf ("\t\t\tCylinder[%d@%d] at {%f,%f,%f}, R= %g, %cz{%g,%g,%g}\n", (int)cylinderTag, cylinderIndex,
                    acCylinder->origin().x, acCylinder->origin().y, acCylinder->origin().z, acCylinder->radius(),
                    acCylinder->isNormalReversed() ? '-' : '+',
                    cylinderParams.basis_set.axis.coord[0], cylinderParams.basis_set.axis.coord[1], cylinderParams.basis_set.axis.coord[2]);
        }
#endif
    
    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CreateFaceEllipticCylinder (DWGGE_TypeP(Surface) acSurface)
    {
    PK_ERROR_code_t pkError = PK_ERROR_wrong_surface;
    if (nullptr == acSurface)
        return  pkError;

    // evaluate 3 points on the cylinder base
    auto majorPoint0 = acSurface->evalPoint (DWGGE_Type(Point2d)(0.0, 0.0));
    auto majorPoint1 = acSurface->evalPoint (DWGGE_Type(Point2d)(0.0, Angle::Pi()));
    auto minorPoint = acSurface->evalPoint (DWGGE_Type(Point2d)(0.0, Angle::PiOver2()));

    // the center point of the bsae is the mid-point of the two major points:
    auto center = 0.5 * (majorPoint0 + majorPoint1.asVector());

    // major and minor axes about the center point:
    auto majorAxis = majorPoint0 - center;
    auto minorAxis = minorPoint - center;

    // get the normal vector from major minor axes
    auto zAxis = majorAxis.crossProduct(minorAxis).normalize ();

    PK_CYL_sf_t cylinderParams;
    this->SetBasisAxes (cylinderParams.basis_set, center, zAxis, majorAxis);

    // Parasolid does not have an elliptical cylinder, so let's hope it is a circular cylinder:
    cylinderParams.radius = majorAxis.length ();
    if (m_needTransformation)
        cylinderParams.radius *= m_toParasolidScale;

    if (fabs(cylinderParams.radius - minorAxis.length()) > 1.0e-5)
        DIAGNOSTICPRINTF ("ASM's elliptical cylinder(R= %g, r= %g) has been converted to a circular Parasolid cylinder!\n", majorAxis.length(), minorAxis.length());

    PK_CYL_t    cylinderTag = -1;
    if (PK_ERROR_no_errors == (pkError = PK_CYL_create(&cylinderParams, &cylinderTag)))
        m_pkGeometryTags.push_back ((int)cylinderTag);
    else
        DIAGNOSTICPRINTF ("Parasolid error creating a PK_CYL(elliptic cylinder) for Brep face: %d!\n", pkError);

    m_hasAnalyticSurface = true;

#ifdef NOISYDEBUG
    if (PK_ERROR_no_errors == pkError)
        {
        int cylinderIndex = (int)m_pkGeometryTags.size() - 1;
        ::printf ("\t\t\tEllipCylinder[%d@%d] at {%f,%f,%f}, R= %g, r= %g, %cz{%g,%g,%g}\n", (int)cylinderTag, cylinderIndex,
                    center.x, center.y, center.z, majorAxis.length(), minorAxis.length(), acSurface->isNormalReversed() ? '-' : '+',
                    cylinderParams.basis_set.axis.coord[0], cylinderParams.basis_set.axis.coord[1], cylinderParams.basis_set.axis.coord[2]);
        }
#endif
    
    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CreateFaceSphere (DWGGE_TypeP(Sphere) acSphere, bool* isOriented = nullptr)
    {
    PK_ERROR_code_t pkError = PK_ERROR_wrong_surface;
    if (nullptr == acSphere)
        return  pkError;

    PK_SPHERE_sf_t  sphereParams;
    this->SetBasisAxes (sphereParams.basis_set, acSphere->center(), acSphere->northAxis(), acSphere->refAxis());

    sphereParams.radius = acSphere->radius ();
    if (m_needTransformation)
        sphereParams.radius *= m_toParasolidScale;

    // fix face-surface orientation flag here as AcDbFace has no set method for that.
    if (nullptr != isOriented && !acSphere->isOuterNormal())
        *isOriented = !*isOriented;

    PK_SPHERE_t sphereTag = -1;
    if (PK_ERROR_no_errors == (pkError = PK_SPHERE_create(&sphereParams, &sphereTag)))
        m_pkGeometryTags.push_back ((int)sphereTag);
    else
        DIAGNOSTICPRINTF ("Parasolid error creating a PK_SPHERE for Brep face: %d!\n", pkError);

    m_hasAnalyticSurface = true;

#ifdef NOISYDEBUG
    if (PK_ERROR_no_errors == pkError)
        {
        int sphereIndex = (int)m_pkGeometryTags.size() - 1;
        ::printf ("\t\t\tSphere[%d@%d] at {%f,%f,%f}, R= %g, %cz{%g,%g,%g}\n", (int)sphereTag, sphereIndex,
                    acSphere->center().x, acSphere->center().y, acSphere->center().z, acSphere->radius(),
                    acSphere->isNormalReversed() ? '-' : '+',
                    sphereParams.basis_set.axis.coord[0], sphereParams.basis_set.axis.coord[1], sphereParams.basis_set.axis.coord[2]);
        }
#endif

    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t AllocateBSurfaceMemory (PK_BSURF_sf_t& bsurfaceParams, size_t numPoints)
    {
    bsurfaceParams.vertex = new double[numPoints * bsurfaceParams.vertex_dim * sizeof(double)];

    bsurfaceParams.u_knot = nullptr;
    bsurfaceParams.v_knot = nullptr;
    bsurfaceParams.u_knot_mult = nullptr;
    bsurfaceParams.v_knot_mult = nullptr;
    
    if (nullptr != bsurfaceParams.vertex)
        {
        bsurfaceParams.u_knot = new double[bsurfaceParams.n_u_knots * sizeof(double)];
        bsurfaceParams.u_knot_mult = new int[bsurfaceParams.n_u_knots * sizeof(int)];

        if (nullptr != bsurfaceParams.u_knot && nullptr != bsurfaceParams.u_knot_mult)
            {
            bsurfaceParams.v_knot = new double[bsurfaceParams.n_v_knots * sizeof(double)];
            bsurfaceParams.v_knot_mult = new int[bsurfaceParams.n_v_knots * sizeof(int)];

            if (nullptr != bsurfaceParams.v_knot && nullptr != bsurfaceParams.v_knot_mult)
                return  PK_ERROR_no_errors;
            }
        }

    return this->FreeBSurfaceMemory (bsurfaceParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t FreeBSurfaceMemory (PK_BSURF_sf_t& bsurfaceParams)
    {
    if (nullptr != bsurfaceParams.vertex)
        delete[] bsurfaceParams.vertex;
    if (nullptr != bsurfaceParams.u_knot)
        delete[] bsurfaceParams.u_knot;
    if (nullptr != bsurfaceParams.u_knot_mult)
        delete[] bsurfaceParams.u_knot_mult;
    if (nullptr != bsurfaceParams.v_knot)
        delete[] bsurfaceParams.v_knot;
    if (nullptr != bsurfaceParams.v_knot_mult)
        delete[] bsurfaceParams.v_knot_mult;
    
    return  PK_ERROR_memory_full;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CheckBSplineSurface (PK_BSURF_sf_t const& bsurfaceParams, PK_BSURF_t& bsurfaceTag)
    {
    /*---------------------------------------------------------------------------------------------------------------
    A geometric discontinuity on a Bspline surface can cause a failure later when attaching it to a face. Turning off 
    session's check for continuity seems to get by this specific failure, but may cause other issues elsewhere.  Both 
    PK_SURF_make_bsurf_2 and making a sheet body may return us multiple surfaces which would require us to re-build 
    topology which is not desirable for an import job.  The best solution we have found so far is to create a new BSpline 
    surface by fitting the existing points of an existing surface.  It may change the parameters and perhaps even the 
    shape of the surface, but that is less of a concern we'd have compared to a complete failure of importing the body.
    ----------------------------------------------------------------------------------------------------------------*/
    int nDisc = 0;
    double* disconts = nullptr;
    PK_PARAM_direction_t*   dir = nullptr;
    PK_continuity_t*    orders = nullptr;
    PK_SURF_find_discontinuity_o_t  opts;
    PK_SURF_find_discontinuity_o_m (opts);

    // only want to fix G dicontinuity
    opts.level = PK_continuity_g1_c;

    auto pkError = PK_SURF_find_discontinuity (bsurfaceTag, &opts, &nDisc, &disconts, &dir, &orders);

    if (PK_ERROR_no_errors == pkError && nDisc > 0)
        {
        PK_UVBOX_t                  uvBox;
        pkError = PK_SURF_ask_uvbox (bsurfaceTag, &uvBox);

        if (PK_ERROR_no_errors == pkError)
            {
            PK_BSURF_fitted_fault_t faults;
            PK_BSURF_t              newSurfaceTag = PK_ENTITY_null;

            PK_BSURF_create_fitted_o_t  options;
            PK_BSURF_create_fitted_o_m (options);

            options.surf.type = PK_SURF_general_surf_c;
            options.surf.surf.parasolid_surf = bsurfaceTag;
            options.u_range.value[0] = uvBox.param[0];
            options.u_range.value[1] = uvBox.param[2];
            options.v_range.value[0] = uvBox.param[1];
            options.v_range.value[1] = uvBox.param[3];

            pkError = PK_BSURF_create_fitted (&options, &newSurfaceTag, &faults);

            if (PK_ERROR_no_errors == pkError && PK_BSURF_fitted_success_c == faults.status && newSurfaceTag != PK_ENTITY_null)
                {
                PK_ENTITY_delete (1, &bsurfaceTag);
                bsurfaceTag = newSurfaceTag;
                }
            else if (newSurfaceTag != PK_ENTITY_null)
                {
                PK_ENTITY_delete (1, &newSurfaceTag);
                pkError = PK_ERROR_cant_make_bspline;
                }
            }
        }

    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IsSurfaceNormalInverted (PK_BSURF_t bsurfaceTag, DWGGE_TypeP(NurbSurface) acNurbs)
    {
    // get the UV bounds from ASM:
    DWGGE_Type(Interval)    intervalU, intervalV;
    acNurbs->getEnvelope (intervalU, intervalV);

    PK_UV_t lowerUV, upperUV;
    if (intervalU.isBounded())
        intervalU.getBounds (lowerUV.param[0], upperUV.param[0]);
    else
        lowerUV.param[0] = upperUV.param[0] = 0.0;
    if (intervalV.isBounded())
        intervalV.getBounds (lowerUV.param[1], upperUV.param[1]);
    else
        lowerUV.param[1] = upperUV.param[1] = 0.0;

    // get the surface normal at the lower bounds per ASM calculation:
    DWGGE_Type(Vector3dArray)  derivatives;
    DWGGE_Type(Vector3d)       acNormal;
    DWGGE_Type(Point3d)        acPoint0 = acNurbs->evalPoint (DWGGE_Type(Point2d)(lowerUV.param[0], lowerUV.param[1]), 1, derivatives, acNormal);

    // get the normal at the same UV params per Parasolid calculation:
    PK_VECTOR_t pointsNderivs[50], pkNormal;
    if (PK_ERROR_no_errors == PK_SURF_eval_with_normal(bsurfaceTag, lowerUV, 1, 1, PK_LOGICAL_false, pointsNderivs, &pkNormal))
        {
        acNormal.normalize (DWGGE_Type(Context::gTol));

        DWGGE_Type(Vector3d)    checkNormal(pkNormal.coord[0], pkNormal.coord[1], pkNormal.coord[2]);
        checkNormal.normalize (DWGGE_Type(Context::gTol));

        double  dotProduct = checkNormal.dotProduct (acNormal);
        if (dotProduct < -0.9)
            return  true;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CreateFaceBSurface (DWGGE_TypeP(NurbSurface) acNurbs, bool* isOriented = nullptr)
    {
    PK_ERROR_code_t pkError = PK_ERROR_wrong_surface;
    if (nullptr == acNurbs)
        return  pkError;

    PK_BSURF_sf_t   bsurfaceParams;
    double          periodic = 0;

    bsurfaceParams.u_degree = acNurbs->degreeInU ();
    bsurfaceParams.v_degree = acNurbs->degreeInV ();
    bsurfaceParams.n_u_vertices = acNurbs->numControlPointsInU ();
    bsurfaceParams.n_v_vertices = acNurbs->numControlPointsInV ();
    bsurfaceParams.n_u_knots = acNurbs->numKnotsInU ();     // multiplicit knots for now
    bsurfaceParams.n_v_knots = acNurbs->numKnotsInV ();     // multiplicit knots for now
    bsurfaceParams.is_u_closed = acNurbs->isClosedInU ();
    bsurfaceParams.is_v_closed = acNurbs->isClosedInV ();
    bsurfaceParams.is_u_periodic = acNurbs->isPeriodicInU (periodic);
    bsurfaceParams.is_v_periodic = acNurbs->isPeriodicInV (periodic);

    bsurfaceParams.is_rational = acNurbs->isRationalInU() && acNurbs->isRationalInV();
    bsurfaceParams.vertex_dim = bsurfaceParams.is_rational ? 4 : 3;

    bsurfaceParams.u_knot_type = PK_knot_unset_c;
    bsurfaceParams.v_knot_type = PK_knot_unset_c;
    bsurfaceParams.form = PK_BSURF_form_unset_c;
    bsurfaceParams.self_intersecting = PK_self_intersect_unset_c;
    bsurfaceParams.convexity = PK_convexity_unset_c;

    // get control points, weights, and knots
    size_t  numPoints = bsurfaceParams.n_u_vertices * bsurfaceParams.n_v_vertices;
    if (numPoints < 2)
        return  pkError;

    DWGGE_Type(Point3dArray)    poles;
    DWGGE_Type(DoubleArray)     weights;
    DWGGE_Type(KnotVector)      uKnots, vKnots;

    acNurbs->getControlPoints (poles);
    acNurbs->getWeights (weights);
    acNurbs->getUKnots (uKnots);
    acNurbs->getVKnots (vKnots);

    if (poles.length() < numPoints || (bsurfaceParams.is_rational && weights.length() < numPoints) ||
        uKnots.length() < bsurfaceParams.n_u_knots || vKnots.length() < bsurfaceParams.n_v_knots)
        return  pkError;

    if (PK_ERROR_no_errors != this->AllocateBSurfaceMemory(bsurfaceParams, numPoints))
        return  PK_ERROR_memory_full;

    // copy control points (V-major in Parasolid, same as in ASM)
    int index;  // use int so we don't have to type cast it every where
    for (index = 0; index < numPoints; index++)
        {
        uint32_t  pkIndex = index * bsurfaceParams.vertex_dim;

        if (m_needTransformation)
            poles[index].transformBy (m_toParasolidMatrix);

        // copy control point at index
        memcpy (&bsurfaceParams.vertex[pkIndex], &poles[index], 3 * sizeof(double));

        // handle weights for a rational B-Spline
        if (4 == bsurfaceParams.vertex_dim)
            {
            // Parasolid expects homogenous/weighted poles
            bsurfaceParams.vertex[pkIndex + 0] *= weights[index];
            bsurfaceParams.vertex[pkIndex + 1] *= weights[index];
            bsurfaceParams.vertex[pkIndex + 2] *= weights[index];
            // copy weight
            bsurfaceParams.vertex[pkIndex + 3] = weights[index];
            }
        }

    // Parasolid expects distinct knots
    DWGGE_Type(DoubleArray) uDistinctKnots, vDistinctKnots;
    uKnots.getDistinctKnots (uDistinctKnots);
    vKnots.getDistinctKnots (vDistinctKnots);

    bsurfaceParams.n_u_knots = uDistinctKnots.length ();
    bsurfaceParams.n_v_knots = vDistinctKnots.length ();

    // copy distinct U-knots
    for (index = 0; index < bsurfaceParams.n_u_knots; index++)
        {
        bsurfaceParams.u_knot[index] = uDistinctKnots[index];
        bsurfaceParams.u_knot_mult[index] = uKnots.multiplicityAt (uDistinctKnots[index]);
        }
    
    // copy distinct V-knots
    for (index = 0; index < bsurfaceParams.n_v_knots; index++)
        {
        bsurfaceParams.v_knot[index] = vDistinctKnots[index];
        bsurfaceParams.v_knot_mult[index] = vKnots.multiplicityAt (vDistinctKnots[index]);
        }
    
    PK_BSURF_t  bsurfaceTag = PK_ENTITY_null, tagSaved = PK_ENTITY_null;
    if (PK_ERROR_no_errors == (pkError = PK_BSURF_create(&bsurfaceParams, &bsurfaceTag)) && PK_ENTITY_null != (tagSaved = bsurfaceTag) &&
        PK_ERROR_no_errors == (pkError = this->CheckBSplineSurface(bsurfaceParams, bsurfaceTag)))
        m_pkGeometryTags.push_back ((int)bsurfaceTag);
    else
        DIAGNOSTICPRINTF ("Parasolid error creating a BSpline surface for Brep face: %d!\n", pkError);

    // check surface normal
    if (PK_ERROR_no_errors == pkError && tagSaved == bsurfaceTag && nullptr != isOriented && this->IsSurfaceNormalInverted(bsurfaceTag, acNurbs))
        *isOriented = !(*isOriented);

#ifdef NOISYDEBUG
    if (PK_ERROR_no_errors == pkError)
        {
        int bsurfaceIndex = (int)m_pkGeometryTags.size() - 1;
        DWGGE_Type(Point2d)         param0(0, 0);
        DWGGE_Type(Vector3dArray)   derivatives;
        DWGGE_Type(Vector3d)        normal;

        auto point0 = acNurbs->evalPoint (param0, 1, derivatives, normal);

        ::printf ("\t\t\tNurbSurface[%d@%d]: degree{%d, %d}, nPoles[%d, %d], nKnots[%d, %d], rational[%d, %d], %cnormal0{%g,%g,%g}@{%g,%g,%g}\n", (int)tagSaved, bsurfaceIndex,
                bsurfaceParams.u_degree, bsurfaceParams.v_degree, bsurfaceParams.n_u_vertices, bsurfaceParams.n_v_vertices,
                bsurfaceParams.n_u_knots, bsurfaceParams.n_v_knots, acNurbs->isRationalInU(), acNurbs->isRationalInV(), acNurbs->isNormalReversed() ? '-':'+',
                normal.x, normal.y, normal.z, point0.x, point0.y, point0.z);

        if (tagSaved != bsurfaceTag)
            {
            ::printf ("\t\t\t=>Replaced by[%d]", bsurfaceTag);
            if (PK_ERROR_no_errors == PK_BSURF_ask(bsurfaceTag, &bsurfaceParams))
                ::printf (": degree{%d, %d}, nPoles[%d, %d], nKnots[%d, %d], %srational",
                        bsurfaceParams.u_degree, bsurfaceParams.v_degree, bsurfaceParams.n_u_vertices, bsurfaceParams.n_v_vertices,
                        bsurfaceParams.n_u_knots, bsurfaceParams.n_v_knots, bsurfaceParams.is_rational ? "" : "non-");
            ::printf ("\n");
            }
        }
#endif

    this->FreeBSurfaceMemory (bsurfaceParams);

    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CreateFaceGeometry (BR_Type(Entity)* brEntity, bool* isOriented = nullptr)
    {
    // create geometry for the face and add it to m_pkGeometryTags which will be attached to the face later on
    auto acFace = dynamic_cast <BR_Type(Face)*> (brEntity);
    auto surfaceType = DWGGE_Type(::kObject);
    if (nullptr == acFace || BrStatusOk != acFace->getSurfaceType(surfaceType))
        return  BSIERROR;

    // extract native surface
    DWGGE_Type(Surface) *acSurface = nullptr, *nativeSurface = nullptr;
    if (BrStatusOk != this->GetNativeSurface(acSurface, nativeSurface, acFace))
        return  BSIERROR;

    PK_ERROR_code_t pkError = PK_ERROR_missing_geom;
    switch (surfaceType)
        {
        case DWGGE_Type(::kPlane):
            pkError = this->CreateFacePlane (static_cast<DWGGE_TypeP(Plane)>(nativeSurface));
            break;
        case DWGGE_Type(::kCone):
            pkError = this->CreateFaceCone (static_cast<DWGGE_TypeP(Cone)>(nativeSurface), isOriented);
            break;
        case DWGGE_Type(::kTorus):
            pkError = this->CreateFaceTorus (static_cast<DWGGE_TypeP(Torus)>(nativeSurface), isOriented);
            break;
        case DWGGE_Type(::kCylinder):
            pkError = this->CreateFaceCylinder (static_cast<DWGGE_TypeP(Cylinder)>(nativeSurface), isOriented);
            break;
        case DWGGE_Type(::kSphere):
            pkError = this->CreateFaceSphere (static_cast<DWGGE_TypeP(Sphere)>(nativeSurface), isOriented);
            break;
        case DWGGE_Type(::kNurbSurface):
            pkError = this->CreateFaceBSurface (static_cast<DWGGE_TypeP(NurbSurface)>(nativeSurface), isOriented);
            break;
        // FUTUREWORK: below two types are not supported in DWGGE_Type( according to ARX doc in its sample code - will extrat them from external surface:
	case DWGGE_Type(::kEllipCylinder):
            pkError = this->CreateFaceEllipticCylinder (acSurface);
            break;
	case DWGGE_Type(::kEllipCone):
            pkError = this->CreateFaceEllipticCone (acSurface);
            break;
        default:
            DIAGNOSTICPRINTF ("Missing surface type %d for a Brep face[%lld]\n", surfaceType, (m_pkClasses.size() - 1));
            BeAssert (false && L"Unsupported surface for a Brep face!!");
        }

    delete acSurface;
    delete nativeSurface;

    return  PK_ERROR_no_errors == pkError ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void* GetBrEntityId (BR_Type(Entity)* brEntity) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return  brEntity;
#elif DWGTOOLKIT_RealDwg
    // ASM shared Brep pointer
    return  brEntity->getEntity ();
#endif    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
int GetOrAddTopologicalClass (PK_CLASS_t inClass, BR_Type(Entity)* brEntity, bool* isNewEntry = nullptr, bool* isOriented = nullptr)
    {
    if (nullptr != isNewEntry)
        *isNewEntry = false;

    if (nullptr == brEntity)
        return  -1;

    AcBrepToPsClassMap* brEntity2pkClassMap = nullptr;
    AcBrepToPsClassMap::const_iterator iter;

    switch (inClass)
        {
        case PK_CLASS_body:
            iter = m_acBodyToClassIndex.find (this->GetBrEntityId(brEntity));
            if (iter != m_acBodyToClassIndex.end())
                return  iter->second;
            // will add a new body
            brEntity2pkClassMap = &m_acBodyToClassIndex;
            // no geoemtry for a body
            m_pkGeometryTags.push_back (-1);
            break;
        case PK_CLASS_region:
            iter = m_acComplexToClassIndex.find (this->GetBrEntityId(brEntity));
            if (iter != m_acComplexToClassIndex.end())
                return  iter->second;
            // will add a new body
            brEntity2pkClassMap = &m_acComplexToClassIndex;
            // no geoemtry for a complex/region
            m_pkGeometryTags.push_back (-1);
            break;
        case PK_CLASS_shell:
            iter = m_acShellToClassIndex.find (this->GetBrEntityId(brEntity));
            if (iter != m_acShellToClassIndex.end())
                return  iter->second;
            // will add a new shell
            brEntity2pkClassMap = &m_acShellToClassIndex;
            // no geoemtry for a shell
            m_pkGeometryTags.push_back (-1);
            break;
        case PK_CLASS_face:
            iter = m_acFaceToClassIndex.find (this->GetBrEntityId(brEntity));
            if (iter != m_acFaceToClassIndex.end())
                return  iter->second;
            // will add a new face
            brEntity2pkClassMap = &m_acFaceToClassIndex;
            // add a surface
            if (BSISUCCESS != this->CreateFaceGeometry(brEntity, isOriented))
                return  -1;
            break;
        case PK_CLASS_loop:
            iter = m_acLoopToClassIndex.find (this->GetBrEntityId(brEntity));
            if (iter != m_acLoopToClassIndex.end())
                return  iter->second;
            // will add a new loop
            brEntity2pkClassMap = &m_acLoopToClassIndex;
            // no geoemtry for a loop
            m_pkGeometryTags.push_back (-1);
            break;
        case PK_CLASS_edge:
            iter = m_acEdgeToClassIndex.find (this->GetBrEntityId(brEntity));
            if (iter != m_acEdgeToClassIndex.end())
                return  iter->second;
            // will add a new edge
            brEntity2pkClassMap = &m_acEdgeToClassIndex;
            // add a curve
            if (BSISUCCESS != this->CreateEdgeGeometry(brEntity))
                return  -1;
            break;
        case PK_CLASS_vertex:
            iter = m_acVertexToClassIndex.find (this->GetBrEntityId(brEntity));
            if (iter != m_acVertexToClassIndex.end())
                return  iter->second;
            // will add a new vertex
            brEntity2pkClassMap = &m_acVertexToClassIndex;
            // add a point
            if (BSISUCCESS != this->CreateVertexGeometry(brEntity))
                return  -1;
            break;
        default:
            return  -1;
        }

    if (nullptr == brEntity2pkClassMap)
        return  -1;

    // the index at which the input class will be added
    int newIndex = (int)(m_pkClasses.size());

    // now add the new class
    m_pkClasses.push_back (inClass);

    // add a new map
    brEntity2pkClassMap->insert (AcBrepToPsClassEntry(this->GetBrEntityId(brEntity), newIndex));

    if (nullptr != isNewEntry)
        *isNewEntry = true;

    return  newIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    AddTopologicalRelation (int parent, int child, PK_TOPOL_sense_t sense = PK_TOPOL_sense_none_c)
    {
    /*--------------------------------------------------------------------
    Parasolid topology:
    Entity    Children      No. of children   Parents     No. of parents
    --------------------------------------------------------------------
    Body      Regions             >=1           -             -
    Region    Shells              >=1         Body            1
    Shell     Faces               >=1         Region          1
              Edges               >=1
              Vertex              1
    Face      Loops               >=1         Shell           2
    Loop      Edges               >=1         Face,Edge       >=1
              Fins                >=1
              Vertex              1
    Fin       Edge                1           Loop,Edge       <=2
    Edge      Vertices            <=2         Loop,Fin,Shell  1
              Loop                >=1
              Fin                 >=1
    Vertex    -                   0           Edge,Loop,Shell >=1
    --------------------------------------------------------------------*/
    m_pkParents.push_back (parent);
    m_pkChildren.push_back (child);
    m_pkSenses.push_back (sense);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    CheckParasolidCreateFaults (PK_BODY_create_topology_2_r_t& results, PK_ERROR_t pkError)
    {
    if (PK_ERROR_no_errors != pkError)
        {
        DIAGNOSTICPRINTF ("Failed building Parasolid topology with error code= %d!\n", pkError);
        return  false;
        }

#ifdef DEBUG_DUMP_TOPOLOGY
    ::printf ("Dumping topology created by Parasolid: number of entities created = %d\n", results.n_topols);
    for (int i = 0; i < results.n_topols; i++)
        ::printf ("entity[%d]= %d %s\n", i, results.topols[i], ClassToString(m_pkClasses[i]));
#endif

    if (results.n_create_faults <= 0)
        return  true;

    if (nullptr == results.create_faults)
        {
        DIAGNOSTICPRINTF ("Unexpected result from building Parasolid topology - null result!\n");
        return  false;
        }

    if (PK_BODY_state_ok_c == results.create_faults[0].state)
        return  true;

#ifdef DIAGNOSTICDEBUG
    ::printf ("Failed building Parasolid topology: state[0]= %d, entities= ", results.create_faults[0].state);
    for (int i = 0; i < results.create_faults[0].n_indices; i++)
        ::printf ("%d ", results.create_faults[0].indices[0]);
    ::printf ("\n");
#endif
    
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    AddExclusiveVoidRegions (int bodyIndex, ParasolidRegionList& exteriorRegions)
    {
    for (auto exteriorFaces : exteriorRegions)
        {
        // add a positive void region
        m_pkClasses.push_back (PK_CLASS_region);
        m_pkGeometryTags.push_back (-1);

        int     voidRegionIndex = (int)m_pkClasses.size() - 1;
        this->AddTopologicalRelation (bodyIndex, voidRegionIndex, PK_TOPOL_sense_positive_c);
        NOISYDEBUG_PRINTF ("Body= %d, Region= %d (void)\n", bodyIndex, voidRegionIndex);

        // add a shell to the void region
        m_pkClasses.push_back (PK_CLASS_shell);
        m_pkGeometryTags.push_back (-1);

        int     shellIndex = (int)m_pkClasses.size() - 1;
        this->AddTopologicalRelation (voidRegionIndex, shellIndex, PK_TOPOL_sense_none_c);
        NOISYDEBUG_PRINTF ("\tRegion= %d, Shell= %d\n", voidRegionIndex, shellIndex);

        // add negative faces to the shell
        for (auto faceIndex : exteriorFaces)
            {
            this->AddTopologicalRelation (shellIndex, faceIndex, PK_TOPOL_sense_negative_c);
            NOISYDEBUG_PRINTF ("\t\tShell= %d, Face= -%d\n", shellIndex, faceIndex);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/14
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t CreateSheetBodyFromSurface (PK_BODY_create_topology_2_r_t& results, ParasolidRegionList& exteriorRegions, bool hasHole)
    {
    // This method takes one single face body of a surface and attempts to create a body via PK_SURF_make_sheet_body
    PK_ERROR_code_t pkError = PK_ERROR_unsupported_operation;
    if (!m_isSheetBody || hasHole || 1 != exteriorRegions.size())
        return  pkError;

    auto faces = exteriorRegions.front ();
    if (1 != faces.size())
        return  pkError;

    // make sure we really have got a surface
    PK_SURF_t   surfaceTag = m_pkGeometryTags[faces.front()];
    PK_CLASS_t  entClass = PK_CLASS_null;
    if (PK_ERROR_no_errors != PK_ENTITY_ask_class(surfaceTag, &entClass) || (PK_CLASS_bsurf != entClass && PK_CLASS_surf != entClass))
        return  pkError;

    PK_UVBOX_t  uvBox;
    if (PK_ERROR_no_errors == (pkError = PK_SURF_ask_uvbox(surfaceTag, &uvBox)))
        pkError = PK_SURF_make_sheet_body (surfaceTag, uvBox, &results.body);

    if (PK_ERROR_no_errors == pkError)
        {
#ifdef NOISYDEBUF
        ::printf ("A sheet body[%d] created from a surface[%d], skipping from building the full topology!\n", results.body, surfaceTag);
#endif
        }
    else
        {
#ifdef DIAGNOSTICDEBUG
        ::printf ("Failed creating a sheet body from a surface[%d] - building the full topology...\n", surfaceTag);
#endif
        }

    return  pkError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CreateTopologyAndGeometry (PK_BODY_create_topology_2_r_t& results, bool& createdSheet, PK_BODY_create_topology_2_o_t pkOptions)
    {
    /*--------------------------------------------------------------------------------------------------------------
    This method traverses DWG Brep tree and creates coorespondent Parasolid entities as it moves from body downward
    to vertex topology.  It also creates geometries and save them in m_pkGeometryTags, which will be attached to the 
    topology upon a successful completion of building the topology.
    ---------------------------------------------------------------------------------------------------------------*/
    createdSheet = false;

    BR_Type(BrepComplexTraverser)   complexTraverser;
    BrStatus   es = complexTraverser.setBrep (*m_acBrepEntity);
    if (BrStatusOk != es)
        return  BSIERROR;

    // add the top level body and region:
    int bodyIndex = this->GetOrAddTopologicalClass (PK_CLASS_body, m_acBrepEntity);
    int regionIndex = this->GetOrAddTopologicalClass (PK_CLASS_region, m_acBrepEntity);
    if (bodyIndex < 0 || regionIndex < 0)
        return  BSIERROR;

    /*-------------------------------------------------------------------------------------------------------
    Make the region a child of the solid body
        1) Parasolid seems to use negative sense to denote a solid region and positive for the exclusive void
        2) Parasolid region does not appear to parallel to ASM complex so we merge complexes into shells
    -------------------------------------------------------------------------------------------------------*/
    this->AddTopologicalRelation (bodyIndex, regionIndex, PK_TOPOL_sense_negative_c);
#ifdef NOISYDEBUG
    ::printf ("Body= %d, Region= -%d (solid)\n", bodyIndex, regionIndex);
#endif

    ParasolidRegionList exteriorRegions;
    bool    hasHoleLoop = false;

    // traverse complexes/regions
    while (!complexTraverser.done())
        {
        BR_Type(Complex)    acComplex;
        BR_Type(ComplexShellTraverser)  shellTraverser;
#ifdef DWGTOOLKIT_OpenDwg
        acComplex = complexTraverser.getComplex ();
        if (BrStatusOk != shellTraverser.setComplex(acComplex))
            return  BSIERROR;
#elif DWGTOOLKIT_RealDwg
        if (AcBr::eOk != shellTraverser.setComplex(complexTraverser) || AcBr::eOk != complexTraverser.getComplex(acComplex))
            return  BSIERROR;
#endif

        ParasolidTagList  exteriorFaces;

        // traverse shells
        while (!shellTraverser.done())
            {
            BR_Type(Shell)  shell;
            BrShellType shellType = BrShellTypeVal(ShellUnclassified);
#ifdef DWGTOOLKIT_OpenDwg
            shell = shellTraverser.getShell ();
            shellType = BrShellType::odbrShellExterior;
#elif DWGTOOLKIT_RealDwg
            if (BrStatusOk != shellTraverser.getShell(shell) || BrStatusOk != shell.getType(shellType))
                return  BSIERROR;
#endif

            BR_Type(ShellFaceTraverser) faceTraverser;
            if (BrStatusOk != (es = faceTraverser.setShell(shell)))
                return  BSIERROR;

            int shellIndex = this->GetOrAddTopologicalClass (PK_CLASS_shell, &shell);
            if (shellIndex < 0)
                return  BSIERROR;

            this->AddTopologicalRelation (regionIndex, shellIndex);
            NOISYDEBUG_PRINTF ("\tRegion= %d, shell= %d\n", regionIndex, shellIndex);

            // check for sense of the shell - exterial=positive, interior=negative
            bool isExteriorShell = shellType == BrShellTypeVal(ShellExterior) && exteriorFaces.empty();
            PK_TOPOL_sense_t shellSense = shellType == BrShellTypeVal(ShellExterior) ? PK_TOPOL_sense_positive_c : PK_TOPOL_sense_negative_c;

            // exterior faces for current shell starts at this index
            size_t  shellFacesStartAt = exteriorFaces.size();

            // traverse faces
            while (!faceTraverser.done())
                {
                BR_Type(Face)   face;
                bool face2Surface = false;
#ifdef DWGTOOLKIT_OpenDwg
                face = faceTraverser.getFace ();
                if (face.isEqualTo(m_acBrepEntity))
                    return  BSIERROR;
                face2Surface = face.getOrientToSurface ();
#elif DWGTOOLKIT_RealDwg
                if (BrStatusOk != faceTraverser.getFace(face) || face.isEqualTo(m_acBrepEntity) || BrStatusOk != face.getOrientToSurface(face2Surface))
                    return  BSIERROR;
#endif

                BR_Type(FaceLoopTraverser) loopTraverser;
                bool    isClosedSurface = false;
#ifdef DWGTOOLKIT_OpenDwg
                if (OdBrErrorStatus::odbrOK != (es = loopTraverser.setFace(face)))
#elif DWGTOOLKIT_RealDwg
                if (BrStatusOk != (es = loopTraverser.setFace(faceTraverser)))
#endif
                    {
                    /*-----------------------------------------------------------------------------------------------
                    AcBr::eDegenerateTopology indicates a closed surface, which is intrinsically bounded in both the 
                    u and v directions, such as a full torus or sphere.  Such a face has no loop boundaries.
                    -----------------------------------------------------------------------------------------------*/
                    if (BrStatVal(DegenerateTopology) == es)
                        isClosedSurface = true;
                    else
                        return  BSIERROR;
                    }

                bool    isNewFace = false;
                int     faceIndex = this->GetOrAddTopologicalClass (PK_CLASS_face, &face, &isNewFace, (bool*)&face2Surface);
                if (faceIndex < 0)
                    return  BSIERROR;

                // add a shell->face relation
                this->AddTopologicalRelation (shellIndex, faceIndex, shellSense);
                NOISYDEBUG_PRINTF ("\t\tShell= %d, face= %c%d, oriented= %c\n", shellIndex, SenseToChar(shellSense), faceIndex, OrientationToChar(face2Surface));

                // move on to next face if this face has already been processed
                if (!isNewFace)
                    {
                    if (BrStatusOk != faceTraverser.next())
                        break;      // error
                    else
                        continue;
                    }

                // save face-surface orientation flag in the map
                m_isFaceOriented2Surface.insert (IsPositiveOrientationEntry(faceIndex, (PK_LOGICAL_t)face2Surface));
                // save off the exterior shell faces
                if (isExteriorShell)
                    exteriorFaces.push_back (faceIndex);

                // move on to next face if this face has an intrinsically bounded geometry
                if (isClosedSurface)
                    {
                    if (BrStatusOk != faceTraverser.next())
                        break;  // error
                    else
                        continue;
                    }

                // traverse face loops
                while (!loopTraverser.done())
                    {
                    BR_Type(Loop)   loop;
                    BrLoopType loopType = BrLoopTypeVal(LoopUnclassified);
#ifdef DWGTOOLKIT_OpenDwg
                    loop = loopTraverser.getLoop ();
#elif DWGTOOLKIT_RealDwg
                    if (BrStatusOk != loopTraverser.getLoop(loop) || BrStatusOk != loop.getType(loopType))
                        return  BSIERROR;
#endif

                    BR_Type(LoopEdgeTraverser)  edgeTraverser;
                    bool    isSingular = false;
#ifdef DWGTOOLKIT_OpenDwg
                    if (BrStatusOk != (es = edgeTraverser.setLoop(loop)))
#elif DWGTOOLKIT_RealDwg
                    if (BrStatusOk != (es = edgeTraverser.setLoop(loopTraverser)))
#endif
                        {
                        // AcBr::eDegenerateTopology indicates a singularity, i.e. loop-vertex:
                        if (BrStatVal(DegenerateTopology) == es)
                            isSingular = true;
                        else
                            return  BSIERROR;
                        }

                    int loopIndex = this->GetOrAddTopologicalClass (PK_CLASS_loop, &loop);
                    if (loopIndex < 0)
                        return  BSIERROR;
                        
                    // set positive sense for exterior and negative for interior loop - not used by Parasolid, but helps debugging
                    PK_TOPOL_sense_t    loopSense = PK_TOPOL_sense_none_c;
                    if (BrLoopTypeVal(LoopExterior) == loopType)
                        loopSense = PK_TOPOL_sense_positive_c;
                    else if (BrLoopTypeVal(LoopInterior) == loopType)
                        loopSense = PK_TOPOL_sense_negative_c;

                    // add a face with a loop child
                    this->AddTopologicalRelation (faceIndex, loopIndex, loopSense);
#ifdef NOISYDEBUG
                    ::printf ("\t\t\tFace= %d, loop= %c%d (%s)\n", faceIndex, SenseToChar(loopSense), loopIndex, LooptypeToString(loopType));
#endif

                    // move on to next loop on singularity
                    if (isSingular)
                        {
                        if (BrStatusOk != loopTraverser.next())
                            break;

                        int vertexIndex = this->AddSingularVertex (face);
                        if (vertexIndex > 0)
                            {
                            this->AddTopologicalRelation (loopIndex, vertexIndex);
#ifdef NOISYDEBUG
                            ::printf ("\t\t\t\tSingular Loop-Vertex= %d - %d\n", loopIndex, vertexIndex);
#endif
                            }

                        if (BrStatusOk != loopTraverser.next())
                            break;      // error
                        else
                            continue;
                        }

                    // PK_SURF_make_sheet_body cannot have holes, so we need to know about it prior to creating a sheet body:
                    if (!hasHoleLoop && BrLoopTypeVal(LoopInterior) == loopType)
                        hasHoleLoop = true;

                    // traverse loop edges
                    while (!edgeTraverser.done())
                        {
                        // get both the edge and its two vertices
                        BR_Type(Edge)   edge;
#ifdef DWGTOOLKIT_OpenDwg
                        if ((edge = edgeTraverser.getEdge()).isNull())
                            return  BSIERROR;
#elif DWGTOOLKIT_RealDwg
                        if (AcBr::eOk != edgeTraverser.getEdge(edge) || edge.isNull())
                            return  BSIERROR;
#endif
                        BR_Type(Vertex) vertex1, vertex2;
                        if (TrueOrOk != edge.getVertex1(vertex1) || vertex1.isNull() ||
                            TrueOrOk != edge.getVertex2(vertex2) || vertex2.isNull())
                            return  BSIERROR;
                        // get edge orientation
                        bool    edge2Curve = true, edge2Loop = true;
#ifdef DWGTOOLKIT_OpenDwg
                        edge2Curve = edge.getOrientToCurve ();
                        edge2Loop = edgeTraverser.getEdgeOrientToLoop ();
#elif DWGTOOLKIT_RealDwg
                        if (AcBr::eOk != edge.getOrientToCurve(edge2Curve) || AcBr::eOk != edgeTraverser.getEdgeOrientToLoop(edge2Loop))
                            return  BSIERROR;
#endif

                        PK_TOPOL_sense_t    edgeSense = edge2Loop ? PK_TOPOL_sense_positive_c : PK_TOPOL_sense_negative_c;
                        PK_LOGICAL_t        curveSense = (PK_LOGICAL_t) edge2Curve;

                        // add a loop with a child edge
                        bool    isNewEdge = false;
                        int     edgeIndex = this->GetOrAddTopologicalClass (PK_CLASS_edge, &edge, &isNewEdge);
                        if (edgeIndex < 0)
                            return  BSIERROR;
                        this->AddTopologicalRelation (loopIndex, edgeIndex, edgeSense);
#ifdef NOISYDEBUG
                        ::printf ("\t\t\t\tLoop= %d, edge= %c%d, %coriented\n", loopIndex, SenseToChar(edgeSense), edgeIndex, OrientationToChar(edge2Curve));
#endif

                        // save edge->curve orientation flag in the map
                        m_isEdgeOriented2Curve.insert (IsPositiveOrientationEntry(edgeIndex, curveSense));

                        if (isNewEdge)
                            {
                            // add the new edge with one or two vertex children
                            int vertex1Index = this->GetOrAddTopologicalClass (PK_CLASS_vertex, &vertex1);
                            int vertex2Index = this->GetOrAddTopologicalClass (PK_CLASS_vertex, &vertex2);
                            if (vertex1Index < 0 || vertex2Index < 0)
                                return  BSIERROR;
                            this->AddTopologicalRelation (edgeIndex, vertex1Index);
                            // allow an intrinsically bounded curve such as circle or elllipse to have a single vertex.
                            if (vertex1Index != vertex2Index)
                                this->AddTopologicalRelation (edgeIndex, vertex2Index);

                            // print edge-vertex infor for debugging purpose only
                            this->PrintEdgeVertex (edgeIndex, vertex1Index, vertex2Index);

                            // this is it - a vertex does not have children!
                            }

                        if (BrStatusOk != edgeTraverser.next())
                            break;
                        }   // end edge traverser while

                    if (BrStatusOk != loopTraverser.next())
                        break;
                    }   // end loopTraverser while

                if (BrStatusOk != faceTraverser.next())
                    break;
                }   // end faceTraverser while

            // for a sheet body, add "interior" faces to close a sheet shell
            if (m_isSheetBody)
                {
                size_t  totalFaces = exteriorFaces.size ();
                for (size_t i = shellFacesStartAt; i < totalFaces; i++)
                    {
                    this->AddTopologicalRelation (shellIndex, exteriorFaces[i], PK_TOPOL_sense_negative_c);
#ifdef NOISYDEBUG
                    ::printf ("\t\tShell= %d, face= -%d, (to complete sheet shell)\n", shellIndex, exteriorFaces[i]);
#endif
                    }
                }

            if (BrStatusOk != shellTraverser.next())
                break;
            }   // end shellTraverser while

        if (!exteriorFaces.empty())
            exteriorRegions.push_back (exteriorFaces);

        if (BrStatusOk != complexTraverser.next())
            break;
        }   // end complexTraverser while

    /*--------------------------------------------------------------------------------------------------------------
    Surface discontinuity can cause geometry attachment to fail.  For a sheet body with a single surface, we may be 
    able to workaround that with PK_SURF_make_sheet_body, which can split a surface for multiple faces.  So if the 
    input ASM entity is a sheet body which has a single surface, don't bother to build the full topology then attach 
    the geometry - directly create the body using PK_SURF_make_sheet_body instead. This should also help performance.
    --------------------------------------------------------------------------------------------------------------*/
    auto pkError = this->CreateSheetBodyFromSurface (results, exteriorRegions, hasHoleLoop);
    if (PK_ERROR_no_errors == pkError)
        {
        createdSheet = true;
        return  BSISUCCESS;
        }
    
    // complete the universe by adding exlusive void regions to the solid regions
    if (!m_isSheetBody && !exteriorRegions.empty())
        this->AddExclusiveVoidRegions (bodyIndex, exteriorRegions);

    int nTopols = (int)m_pkClasses.size ();
    int nParents = (int)m_pkParents.size ();
    int nChildren = (int)m_pkChildren.size ();
    if (nTopols < 1 || nParents < 1 || nParents != nChildren)
        return  BSIERROR;

    // debug the data we have built to create the topology
    this->DumpTopology ();

    pkError = PK_BODY_create_topology_2 (nTopols, &m_pkClasses.front(), nParents, &m_pkParents.front(), &m_pkChildren.front(), &m_pkSenses.front(), &pkOptions, &results);

    if (!this->CheckParasolidCreateFaults(results, pkError))
        return  BSIERROR;

    // ISolidKernelEntity does not support general body
    PK_BODY_type_t      bodyType = PK_BODY_type_unspecified_c;
    PK_BODY_ask_type (results.body, &bodyType);
    if (bodyType != PK_BODY_type_solid_c && bodyType != PK_BODY_type_sheet_c)
        return  BSIERROR;

    // we have got a Parasolid topology ready to be attached with geometry.
    return  BSISUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AttachGeometryToTopology (PK_TOPOL_t* pkTopols, int nTopols)
    {
    if (nullptr == pkTopols || nTopols < 1 || nTopols != (int)m_pkGeometryTags.size() || nTopols != (int)m_pkClasses.size())
        return  BSIERROR;

    ParasolidTagList    faceTags, edgeTags, vertexTags;
    ParasolidTagList    surfaceTags, curveTags, pointTags;
    bvector<PK_LOGICAL_t>   face2Surface, edge2Curve;

    for (int i = 0; i < nTopols; i++)
        {
        switch (m_pkClasses[i])
            {
            case PK_CLASS_face:
                {
                faceTags.push_back (pkTopols[i]);
                surfaceTags.push_back (m_pkGeometryTags[i]);

                IsPositiveOrientationMap::const_iterator    iter = m_isFaceOriented2Surface.find (i);
                if (iter == m_isFaceOriented2Surface.end())
                    return  BSIERROR;
                face2Surface.push_back (iter->second);
                break;
                }

            case PK_CLASS_edge:
                {
                edgeTags.push_back (pkTopols[i]);
                curveTags.push_back (m_pkGeometryTags[i]);

                IsPositiveOrientationMap::const_iterator    iter = m_isEdgeOriented2Curve.find (i);
                if (iter == m_isEdgeOriented2Curve.end())
                    return  BSIERROR;
                edge2Curve.push_back (iter->second);
                break;
                }

            case PK_CLASS_vertex:
                {
                vertexTags.push_back (pkTopols[i]);
                pointTags.push_back (m_pkGeometryTags[i]);

                if (m_vertexTolerance > 0.0)
                    PK_VERTEX_set_precision (pkTopols[i], m_vertexTolerance);
                break;
                }
            }
        }

    // first attach points to vertices for a not intrinsically bounded body
    PK_ERROR_code_t     pkError = PK_ERROR_no_errors;
    int                 arraySize = (int) vertexTags.size ();
    if (arraySize > 0)
        pkError = PK_VERTEX_attach_points (arraySize, &vertexTags.front(), &pointTags.front());

    if (PK_ERROR_no_errors != pkError)
        {
        this->DumpGeometry (faceTags, surfaceTags, face2Surface, edgeTags, curveTags, edge2Curve, vertexTags, pointTags);
        DIAGNOSTICPRINTF ("Error attaching %d points to vertices: %d\n", arraySize, pkError);
        return  BSIERROR;
        }

    if ((arraySize = (int)edgeTags.size()) > 0)
        {
        PK_EDGE_attach_curves_o_s   options;
        PK_ENTITY_track_r_s         tracking;

        // set default options before copying edge orientation flags
        PK_EDGE_attach_curves_o_m (options);
        options.have_senses = PK_LOGICAL_true;
        options.senses = &edge2Curve.front ();

        // then attach curves to edges
        pkError = PK_EDGE_attach_curves_2 (arraySize, &edgeTags.front(), &curveTags.front(), &options, &tracking);
        }

    if (PK_ERROR_no_errors != pkError)
        {
        this->DumpGeometry (faceTags, surfaceTags, face2Surface, edgeTags, curveTags, edge2Curve, vertexTags, pointTags);
        DIAGNOSTICPRINTF ("Error attaching %d curves to edges: %d\n", arraySize, pkError);
        return  BSIERROR;
        }

#ifdef DUMP_FACE_ATTACHMENT
    this->StartDebugReport ();
#endif

    // attach surfaces to faces last
    if ((arraySize = (int)faceTags.size()) > 0)
        pkError = PK_FACE_attach_surfs (arraySize, &faceTags.front(), &surfaceTags.front(), &face2Surface.front());

#ifdef DUMP_FACE_ATTACHMENT
    this->StopDebugReport ();
#endif

    if (PK_ERROR_no_errors != pkError)
        {
        this->DumpGeometry (faceTags, surfaceTags, face2Surface, edgeTags, curveTags, edge2Curve, vertexTags, pointTags);
        DIAGNOSTICPRINTF ("Error attaching %d surfaces to faces: %d\n", arraySize, pkError);
        }

    return  PK_ERROR_no_errors == pkError ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IsOutsideParasolidSizeBox (DWGGE_TypeCR(Point3d) minPoint, DWGGE_TypeCR(Point3d) maxPoint)
    {
    // The Parasolid body size is defined as 1000x1000x1000 and centered at {0, 0, 0}.
    double  sizeboxHigh = 0.5 * s_max_parasolidSizeBox;
    double  sizeboxLow  = -sizeboxHigh;

    return  minPoint.x < sizeboxLow  || minPoint.y < sizeboxLow  || minPoint.z < sizeboxLow ||
            maxPoint.x > sizeboxHigh || maxPoint.y > sizeboxHigh || maxPoint.z > sizeboxHigh;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   TransformToParasolidSpace (DWGDB_TypeCP(Entity) acEntity)
    {
    /*---------------------------------------------------------------------------------------------------------------
    This method finds the ASM body size and geometric center location, builds a transformation matrix that transforms
    the body into the Parasolid size box.  It also sets Parasolid's vertex tolerance based on ASM body size.
    ---------------------------------------------------------------------------------------------------------------*/
    m_needTransformation = false;

    GeomExtents extents;
    if (ToDwgDbStatus(acEntity->getGeomExtents(extents)) != DwgDbStatus::Success)
        return  BSIERROR;

    DWGGE_Type(Point3d) minPoint = extents.minPoint ();
    DWGGE_Type(Point3d) maxPoint = extents.maxPoint ();

    double  bodySize = minPoint.distanceTo (maxPoint);
    if (bodySize <= 1.e-10)
        return  BSIERROR;

    // scale the body if the size is either too big or too small
    double  scale = 1.0;
    if (bodySize > s_max_parasolidSizeBox || bodySize < 10.0)
        {
        scale = s_max_parasolidSizeBox / bodySize;

        /*-----------------------------------------------------------------------------------------------
        The scale we get above tightly fits the body to the size box, but we'd like to make some room for
        the sake of Parasolid.  When we shrink a large body into the size box, we scale the body to about
        80% the size box. which seems to work for most cases we have seen so far.  When we enlarge a body, 
        however, we choose to only scale it up to 50% in an attempt to reduce potential geometrical
        distortion by a large scale, which can result in PK_ERROR_bad_tolerance.
        -----------------------------------------------------------------------------------------------*/
        double      largeScale = 0.8 * scale;
        double      smallScale = 0.5 * scale;
        scale = (bodySize > s_max_parasolidSizeBox) ? largeScale : smallScale;

        // reset body size for vertex tolernace - we still want to use the large room as the tolerance base
        bodySize *= largeScale;

        m_toParasolidMatrix.setToScaling (scale);
        m_needTransformation = true;
        }

    // translate the body to align its center at the world origin as needed
    if (m_needTransformation || this->IsOutsideParasolidSizeBox(minPoint, maxPoint))
        {
        DWGGE_Type(Point3d)     center = 0.5 * (minPoint + maxPoint.asVector());
        DWGGE_Type(Vector3d)    translation = center.asVector ();

        translation.negate ();

        if (m_needTransformation)
            translation.setToProduct (translation, scale);

        m_toParasolidMatrix.setTranslation (translation);

        m_needTransformation = true;
        }

    if (m_needTransformation)
        m_toParasolidScale = scale;

    // loosen up vertex tolerance based on body size
    m_vertexTolerance = 1.0e-5 * bodySize;

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/15
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ERROR_code_t RepairParasolidBody (PK_BODY_t bodyTag)
    {
    PK_TOPOL_track_r_t  tracking;
    int                 numEdges = 0;
    PK_EDGE_t*          edges = nullptr;
    PK_ERROR_code_t     error = PK_BODY_ask_edges (bodyTag, &numEdges, &edges);

    // repair edges
    if (PK_ERROR_no_errors == error && numEdges > 0)
        {
        PK_EDGE_repair_o_t  edgeOptions;
        PK_EDGE_repair_o_m (edgeOptions);

        edgeOptions.max_tolerance = 10.0 * m_vertexTolerance;

        error = PK_EDGE_repair (numEdges, edges, &edgeOptions, &tracking);

        if (PK_ERROR_no_errors != error)
            DIAGNOSTICPRINTF ("\tFailed repairing %d edges! PKError=%d\n", numEdges, error);
        }
    else
        {
        numEdges = 0;
        }

    int         numFaces = 0;
    PK_FACE_t*  faces = nullptr;
    error = PK_BODY_ask_faces (bodyTag, &numFaces, &faces);

    // repair faces
    if (PK_ERROR_no_errors == error && numFaces > 0)
        {
        PK_FACE_repair_o_t  faceOptions;
        PK_FACE_repair_o_m (faceOptions);

        for (int i = 0; i < numFaces; i++)
            {
            error = PK_FACE_repair (faces[i], &faceOptions, &tracking);

            if (PK_ERROR_no_errors != error)
                DIAGNOSTICPRINTF ("\tFailed repairing face %d! PKError=%d\n", faces[i], error);
            }
        }

    PK_EDGE_reset_precision_o_t precisionOptions;
    PK_EDGE_reset_precision_o_m (precisionOptions);

    // reset edge curve precision
    for (int i = 0; i < numEdges; i++)
        {
        PK_reset_prec_t result;

        error = PK_EDGE_reset_precision_2 (edges[i], &precisionOptions, &result);

        if (PK_ERROR_no_errors != error)
            DIAGNOSTICPRINTF ("\tFailed resetting precision for edge %d! PKError=%d\n", faces[i], error);
        }

    return  PK_ERROR_no_errors == error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    CheckParasolidBody (PK_BODY_t bodyTag)
    {
    PK_ERROR_code_t     error;
#ifdef DEBUG_DUMP_EDGES
    int                 nEdges = 0;
    PK_EDGE_t*          edges = nullptr;

    PK_BODY_ask_topology_o_t        options;
    PK_BODY_ask_topology_o_m (options);

    int                 nTopols = 0, nRelations = 0;
    PK_TOPOL_t*         topols = nullptr;
    PK_CLASS_t*         classes;
    int*                parents = nullptr;
    int*                children = nullptr;
    PK_TOPOL_sense_t*   senses = nullptr;

    this->StartDebugReport ();
    error = PK_BODY_ask_topology (bodyTag, &options, &nTopols, &topols, &classes, &nRelations, &parents, &children, &senses);
    error = PK_BODY_ask_edges (bodyTag, &nEdges, &edges);

    if (PK_ERROR_no_errors == error && nEdges > 1)
        {
        ::printf ("Dumping %d edges via PK_BODY_ask_edges:\n");
        for (int i = 0; i < nEdges; i++)
            ::printf ("%d ", edges[i]);
        ::printf ("\n");
        }
    else
        {
        ::printf ("Error code returned from PK_BODY_ask_edges = %d, or 0 number of edges returned!\n", error);
        }
    if (nullptr != topols)
        delete topols;
    if (nullptr != classes)
        delete classes;
    if (nullptr != parents)
        delete parents;
    if (nullptr != children)
        delete children;
    if (nullptr != senses)
        delete senses;
#endif

    int                 nFaults = 0;
    PK_check_fault_t*   faults;

    error = PK_BODY_check (bodyTag, nullptr, &nFaults, &faults);

#ifdef DEBUG_DUMP_EDGES
    this->StopDebugReport ();
#endif

    if (PK_ERROR_no_errors == error && nFaults < 1)
        {
        PK_BODY_type_t  bodyType = PK_BODY_type_unspecified_c;
        PK_BODY_ask_type (bodyTag, &bodyType);

        // we don't support general body
        return PK_BODY_type_general_c == bodyType ? false : true;
        }

    bool    allowFaults = true;

    DIAGNOSTICPRINTF ("Parsolid body check failed: error= %d", error);

    for (int i = 0; i < nFaults; i++)
        {
        PK_CLASS_t      ent1Class = 0, ent2Class = 0;
        error = PK_ENTITY_ask_class (faults[i].entity_1, &ent1Class);
        error = PK_ENTITY_ask_class (faults[i].entity_2, &ent2Class);

#ifdef DIAGNOSTICDEBUG
        ::printf ("\n\t%d)state= %d, entity1[%s]= %d, entity2[%s]= %d", i, (int)faults[i].state, ClassToString(ent1Class), faults[i].entity_1, ClassToString(ent2Class), faults[i].entity_2);
#endif

        // treat below faults as error: 
        switch (faults[i].state)
            {
            case PK_FACE_state_bad_vertex_c:        // TFS 168352 - torus surface failed rendering
            case PK_FACE_state_bad_edge_c:          // TFS 204884 - view update hung in a Parasolid faceting call
            case PK_FACE_state_bad_loops_c:         // loops/faces have wrong normals!
                allowFaults = false;
                break;
            default:
                allowFaults = true;
            }

        if (!allowFaults)
            break;
        }
    DIAGNOSTICPRINTF ("\n");

    return  allowFaults;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    CheckParasolidGeometry (PK_GEOM_t geometryTag)
    {
    PK_GEOM_check_o_t   options;
    PK_GEOM_check_o_m (options);

    PK_check_fault_t*   faults = nullptr;
    int                 nFaults = 0;

    PK_ERROR_code_t     error = PK_GEOM_check (geometryTag, &options, &nFaults, &faults);

    if (PK_ERROR_no_errors == error && nFaults < 1)
        {
        // Parasolid can fail on attaching a surface with C1 discontinuity to a face
        PK_CLASS_t      entClass = PK_CLASS_null;
        if (PK_ERROR_no_errors == PK_ENTITY_ask_class(geometryTag, &entClass) && (PK_CLASS_bsurf == entClass || PK_CLASS_surf == entClass))
            {
            PK_SURF_find_discontinuity_o_t  opts;
            PK_SURF_find_discontinuity_o_m (opts);

            int                     nDisc = 0;
            double*                 disconts = nullptr;
            PK_PARAM_direction_t*   dir = nullptr;
            PK_continuity_t*        orders = nullptr;

            if (PK_ERROR_no_errors != PK_SURF_find_discontinuity(geometryTag, &opts, &nDisc, &disconts, &dir, &orders) || nDisc > 0)
                {
#ifdef DIAGNOSTICDEBUG
                ::printf ("\t-BSurface[%d] has %d discontinuity(ies):", geometryTag, nDisc);
                for (int i = 0; i < nDisc; i++)
                    ::printf (" %g(%c c%d)", disconts[i], PK_PARAM_direction_u_c == dir[i] ? 'U' : 'V', orders[i] - PK_continuity_c1_c + 1);
                ::printf ("\n");
#endif
                return  false;
                }
            }
        return  true;
        }

#ifdef DIAGNOSTICDEBUG
    ::printf ("Parsolid geometry check failed: error= %d", error);
    for (int i = 0; i < nFaults; i++)
        {
        PK_CLASS_t      ent1Class = 0, ent2Class = 0;
        error = PK_ENTITY_ask_class (faults[i].entity_1, &ent1Class);
        error = PK_ENTITY_ask_class (faults[i].entity_2, &ent2Class);
        ::printf ("\n\t%d)state= %d, entity1[%s]= %d, entity2[%s]= %d", i, (int)faults[i].state, ClassToString(ent1Class), faults[i].entity_1, ClassToString(ent2Class), faults[i].entity_2);
        }
    ::printf ("\n");
#endif

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    PrintEdgeVertex (int edgeIndex, int vertex1Index, int vertex2Index)
    {
#ifdef NOISYDEBUG
    PK_POINT_sf_s   p1, p2;
    if (PK_ERROR_no_errors == PK_POINT_ask(m_pkGeometryTags[vertex1Index], &p1) && 
        PK_ERROR_no_errors == PK_POINT_ask(m_pkGeometryTags[vertex2Index], &p2))
        {
        if (m_needTransformation)
            {
            DWGGE_Type(Matrix3d)    fromParasolidMatrix = m_toParasolidMatrix.inverse ();
            DWGGE_Type(Point3d)     acPoint (p1.position.coord[0], p1.position.coord[1], p1.position.coord[2]);

            acPoint.transformBy (fromParasolidMatrix);
            memcpy (&p1.position.coord[0], &acPoint, sizeof p1.position.coord);

            acPoint.set (p2.position.coord[0], p2.position.coord[1], p2.position.coord[2]);

            acPoint.transformBy (fromParasolidMatrix);
            memcpy (&p2.position.coord[0], &acPoint, sizeof p2.position.coord);
            }

        ::printf ("\t\t\t\t\tEdge= %d, vertex1= %d{%g,%g,%g}, vertex2= %d{%g,%g,%g}\n", edgeIndex,
                vertex1Index, p1.position.coord[0], p1.position.coord[1], p1.position.coord[2],
                vertex2Index, p2.position.coord[0], p2.position.coord[1], p2.position.coord[2]);
        }
    else
        {
        ::printf ("\t\t\t\t\tEdge= %d, vertex1= %d, vertex2= %d [error extracting points!]\n", edgeIndex, vertex1Index, vertex2Index);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    DumpTopology ()
    {
#ifdef DEBUG_DUMP_TOPOLOGY
    size_t      count = 0;
    ::printf ("Dumping topology passed to Parasolid:\n");
    for (auto topolClass : m_pkClasses)
        ::printf ("Class[%lld]= %d %s\n", count++, topolClass, ClassToString(topolClass));

    count = m_pkParents.size ();
    for (int i = 0; i < count; i++)
        ::printf ("Relation[%d]= parent= %d, child= %c%d\n", i, m_pkParents[i], SenseToChar(m_pkSenses[i]), m_pkChildren[i]);

    count = 0;
    for (auto geometryTag : m_pkGeometryTags)
        {
        if (geometryTag < 0)
            continue;

        ::printf ("Geometry[%lld]= %d  (", count++, geometryTag);

        PK_POINT_sf_t       point;
        PK_LINE_sf_t        line;
        PK_CIRCLE_sf_t      circle;
        PK_ELLIPSE_sf_t     ellipse;
        PK_BCURVE_sf_t      bcurve;
        PK_BSURF_sf_t       bsurf;
        PK_PLANE_sf_t       plane;
        PK_CLASS_t          entClass = PK_CLASS_null;

        PK_ENTITY_ask_class (geometryTag, &entClass);

        if (PK_CLASS_point == entClass && PK_ERROR_no_errors == PK_POINT_ask(geometryTag, &point))
            ::printf ("Point: o{%g, %g, %g}", point.position.coord[0], point.position.coord[1], point.position.coord[2]);
        else if (PK_CLASS_line == entClass && PK_ERROR_no_errors == PK_LINE_ask(geometryTag, &line))
            ::printf ("Line: o{%g, %g, %g}, x{%g, %g, %g}", line.basis_set.location.coord[0], line.basis_set.location.coord[1], line.basis_set.location.coord[2], line.basis_set.axis.coord[0], line.basis_set.axis.coord[1], line.basis_set.axis.coord[2]);
        else if (PK_CLASS_circle == entClass && PK_ERROR_no_errors == PK_CIRCLE_ask(geometryTag, &circle))
            ::printf ("Circle: o{%g, %g, %g}, R=%g", circle.basis_set.location.coord[0], circle.basis_set.location.coord[1], circle.basis_set.location.coord[2], circle.radius);
        else if (PK_CLASS_ellipse == entClass && PK_ERROR_no_errors == PK_ELLIPSE_ask(geometryTag, &ellipse))
            ::printf ("Ellipse: o{%g, %g, %g}, R1=%g R2=%g", ellipse.basis_set.location.coord[0], ellipse.basis_set.location.coord[1], ellipse.basis_set.location.coord[2], ellipse.R1, ellipse.R2);
        else if (PK_CLASS_bcurve == entClass && PK_ERROR_no_errors == PK_BCURVE_ask(geometryTag, &bcurve))
            ::printf ("BCurve: %d degrees, %d poles", bcurve.degree, bcurve.n_vertices);
        else if (PK_CLASS_plane == entClass && PK_ERROR_no_errors == PK_PLANE_ask(geometryTag, &plane))
            ::printf ("Plane: o{%g, %g, %g}, x{%g, %g, %g}, z{%g, %g, %g}", plane.basis_set.location.coord[0], plane.basis_set.location.coord[2], plane.basis_set.location.coord[2],
                        plane.basis_set.ref_direction.coord[0], plane.basis_set.ref_direction.coord[1], plane.basis_set.ref_direction.coord[2], 
                        plane.basis_set.axis.coord[0], plane.basis_set.axis.coord[1], plane.basis_set.axis.coord[2]);
        else if (PK_CLASS_bsurf == entClass && PK_ERROR_no_errors == PK_BSURF_ask(geometryTag, &bsurf))
            ::printf ("BSurface= {%d, %d} degrees", bsurf.u_degree, bsurf.v_degree);
        else
            ::printf ("??");
        ::printf (")\n");
        }
#endif    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    DumpGeometry (ParasolidTagListR faceTags, ParasolidTagListR surfaceTags, bvector<PK_LOGICAL_t>& face2Surface, ParasolidTagListR edgeTags, ParasolidTagListR lineTags, bvector<PK_LOGICAL_t>& edge2Line, ParasolidTagListR vertexTags, ParasolidTagListR pointTags)
    {
#ifdef NOISYDEBUG
    PK_ERROR_code_t     error;
    int                 i = 0;
    ::printf ("Dumping geometry:");
    for (; i < faceTags.size(); i++)
        {
        PK_PLANE_sf_t   plane;
        PK_BSURF_sf_t   bsurface;
        PK_CYL_sf_t     cylinder;
        PK_TORUS_sf_t   torus;
        PK_CLASS_t      entClass = PK_CLASS_null;

        PK_ENTITY_ask_class (surfaceTags[i], &entClass);

        if (PK_CLASS_plane == entClass && PK_ERROR_no_errors == (error = PK_PLANE_ask(surfaceTags[i], &plane)))
            {
            ::printf ("Plane[%d]/Face[%d] ", surfaceTags[i], faceTags[i]);
            DiagnosticPrintAxis2 (plane.basis_set);
            ::printf (" %c\n", OrientationToChar(face2Surface[i]));
            }
        else if (PK_CLASS_bsurf == entClass && PK_ERROR_no_errors == (error = PK_BSURF_ask(surfaceTags[i], &bsurface)))
            {
            ::printf ("BSurface[%d]/Face[%d]: degrees{%d, %d}, nPoles[%d, %d], nKnots[%d, %d], %s rational.\n", surfaceTags[i], faceTags[i],
                        bsurface.u_degree, bsurface.v_degree, bsurface.n_u_vertices, bsurface.n_v_vertices,
                        bsurface.n_u_knots, bsurface.n_v_knots, bsurface.is_rational ? "is" : "not");
            }
        else if (PK_CLASS_cyl == entClass && PK_ERROR_no_errors == (error = PK_CYL_ask(surfaceTags[i], &cylinder)))
            {
            ::printf ("Cylinder[%d]/Face[%d]: ", surfaceTags[i], faceTags[i]);
            DiagnosticPrintAxis2 (cylinder.basis_set);
            ::printf (" %c\n", OrientationToChar(face2Surface[i]));
            }
        else if (PK_CLASS_torus == entClass && PK_ERROR_no_errors == (error = PK_TORUS_ask(surfaceTags[i], &torus)))
            {
            ::printf ("Torus, R= %g, r= %g, ", torus.minor_radius, torus.minor_radius);
            DiagnosticPrintAxis2 (torus.basis_set);
            ::printf (" %c\n", OrientationToChar(face2Surface[i]));
            }
        else
            {
            ::printf ("Surface(class=%d)[%d]/Face[%d]...\n", entClass, surfaceTags[i], faceTags[i]);
            }
        this->CheckParasolidGeometry (surfaceTags[i]);
        }

    for (i = 0; i < edgeTags.size(); i++)
        {
        PK_CURVE_t      curveTag = -1;
        PK_CLASS_t      geomClass = -1;
        PK_VECTOR_t     ends[2];
        PK_LOGICAL_t    sense = PK_LOGICAL_false;
        PK_INTERVAL_t   interval;
        PK_LINE_sf_t    line;
        PK_ELLIPSE_sf_s ellipse;
        PK_CLASS_t      entClass = PK_CLASS_null;

        PK_ENTITY_ask_class (edgeTags[i], &entClass);

        if (PK_CLASS_line == entClass && PK_ERROR_no_errors == (error = PK_LINE_ask(lineTags[i], &line)))
            {
            ::printf ("Line[%d]/Edge[%d] at {%.10f, %.10f, %.10f}, %c\n", lineTags[i], edgeTags[i],
                        line.basis_set.location.coord[0], line.basis_set.location.coord[1], line.basis_set.location.coord[2], 
                        OrientationToChar(edge2Line[i]));
            }
        else if (PK_CLASS_ellipse == entClass && PK_ERROR_no_errors == (error = PK_ELLIPSE_ask(lineTags[i], &ellipse)))
            {
            ::printf ("Ellipse[%d]/Edge[%d] at {%.10f, %.10f, %.10f}, R= %.10f, r=%.10f, %c\n", lineTags[i], edgeTags[i],
                        ellipse.basis_set.location.coord[0], ellipse.basis_set.location.coord[1], ellipse.basis_set.location.coord[2], 
                        ellipse.R1, ellipse.R2, OrientationToChar(edge2Line[i]));
            }
        else if (PK_ERROR_no_errors == (error = PK_EDGE_ask_geometry(edgeTags[i], false, &curveTag, &geomClass, ends, &interval, &sense)))
            {
            ::printf ("Curve[%d]/Edge[%d]: p1{%.10f, %.10f, %.10f}, p2{%.10f, %.10f, %.10f}, %c\n", lineTags[i], edgeTags[i],
                        ends[0].coord[0], ends[0].coord[1], ends[0].coord[2], ends[1].coord[0], ends[1].coord[1], ends[1].coord[2], 
                        OrientationToChar(edge2Line[i]));
            }
        else
            {
            ::printf ("Curve[%d]/Edge[%d]: failed data extraction!!\n", lineTags[i], edgeTags[i]);
            }
        this->CheckParasolidGeometry (lineTags[i]);
        }

    for (i = 0; i < vertexTags.size(); i++)
        {
        PK_POINT_sf_t   point;
        error = PK_POINT_ask (pointTags[i], &point);

        if (PK_ERROR_no_errors == error)
            ::printf ("Point[%d]/Vertex[%d]= %.10f, %.10f, %.10f\n", pointTags[i], vertexTags[i], 
                        point.position.coord[0], point.position.coord[1], point.position.coord[2]);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    StartDebugReport ()
    {
#ifdef DIAGNOSTICDEBUG
    this->StopDebugReport ();

    m_debugReportStarted = false;

    BeFileName  reportFileName;
    if (BeFileNameStatus::Success == Desktop::FileSystem::BeGetTempPath(reportFileName))
        {
        reportFileName.AppendToPath (L"\\Asm2ParasolidReport.xml");

        AString     localeName;
        if (BSISUCCESS == BeStringUtilities::WCharToCurrentLocaleChar(localeName, reportFileName.GetWCharCP()))
            {
            PK_DEBUG_report_start_o_t   reportOptions;
            PK_DEBUG_report_start_o_m (reportOptions);

            if (PK_ERROR_no_errors == PK_DEBUG_report_start(localeName.c_str(), nullptr))
                m_debugReportStarted = true;
            }
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    StopDebugReport ()
    {
    if (m_debugReportStarted)
        {
        PK_DEBUG_report_stop ();
        m_debugReportStarted = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    CleanUpParasolid (PK_BODY_create_topology_2_r_t& results, bool deleteGeometry)
    {
    bvector<PK_ENTITY_t>    entities;
    if (results.body > 0)
        entities.push_back (results.body);

    if (deleteGeometry)
        {
        // delete unattached geometry
        for (auto geometryTag : m_pkGeometryTags)
            {
            if (geometryTag > 0)
                entities.push_back (geometryTag);
            }
        }

    if (entities.size() > 0)
        {
        auto    pkError = PK_ENTITY_delete ((int)entities.size(), &entities.front());

        if (PK_ERROR_no_errors != pkError)
            DIAGNOSTICPRINTF ("Failled deleting %lld Parasolid entities. [Error=%d]!\n", entities.size(), pkError);

        // set the body tag to null so the caller will not try to delete it again:
        results.body = PK_ENTITY_null;
        }

    this->StopDebugReport ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IsSheetBody (DwgDbEntityCP acEntity)
    {
    // AcBrEntity::getVolume is an expensive operation so try AcDbEntity types as our first choice:
    if (acEntity->isKindOf(DWGDB_Type(3dSolid)::desc()))
        return  false;

    // a surface or a region is always a sheet body
    if (acEntity->isKindOf(DWGDB_Type(Surface)::desc()) || acEntity->isKindOf(DWGDB_Type(Region)::desc()))
        return  true;

    // a body type may be sheet or solid - check volume:
    auto    body = DWGDB_Type(Body)::cast (acEntity);
    if (nullptr != body)
        {
        double  volume = 0.0;
#ifdef DWGTOOLKIT_OpenDwg
        OdGeExtents3d   extents;
        if (OdResult::eOk == body->subGetGeomExtents(extents))
            volume = Util::DRange3dFrom(extents).Volume ();
#elif DWGTOOLKIT_RealDwg
        if (AcBr::eOk != m_acBrepEntity->getVolume(volume))
            return  true;
#endif
        return  volume < 1.0e-10;
        }

    return  false;
    }


public:
    BrepConverter ()
        {
        this->InitParasolidData ();
        }

    ~BrepConverter ()
        {
        if (nullptr != m_acBrepEntity)
            delete m_acBrepEntity;
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   InitBrepFrom (DwgDbEntityCP acEntity)
    {
    m_acBrepEntity = new BR_Type(Brep) ();
    if (nullptr == m_acBrepEntity)
        return  BSIERROR;

#ifdef DWGTOOLKIT_OpenDwg
    OdDb3dSolid* solid3d;
    OdDbRegion* region;
    OdDbBody*   body;
    OdDbSurface* surface;

    if ((solid3d = OdDb3dSolid::cast(acEntity)) != nullptr)
        solid3d->brep (*m_acBrepEntity);
    else if ((region = OdDbRegion::cast(acEntity)) != nullptr)
        region->brep (*m_acBrepEntity);
    else if ((body = OdDbBody::cast(acEntity)) != nullptr)
        body->brep (*m_acBrepEntity);
    else if ((surface = OdDbSurface::cast(acEntity)) != nullptr)
        surface->brep (*m_acBrepEntity);
    else
        return  BSIERROR;
        
    if (m_acBrepEntity->isValid())
        return  BSISUCCESS;

#elif DWGTOOLKIT_RealDwg

    if (AcBr::eOk == m_acBrepEntity->set(*acEntity))
        return  BSISUCCESS;
#endif

    return  BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConvertToParasolid (PK_BODY_create_topology_2_r_t& results, TransformR trans, DwgDbEntityCP acEntity)
    {
    if (nullptr == acEntity)
        return  BSIERROR;

    // transform solid entity to fit in Parasolid model cube:
    auto status = this->TransformToParasolidSpace (acEntity);
    if (BSISUCCESS != status)
        return  status;

    Util::GetTransform (trans, m_toParasolidMatrix);

    // extract Brep from input entity and cache it for the converter:
    if (this->InitBrepFrom(acEntity) != BSISUCCESS)
        return  BSIERROR;

    // check the input body for sheet/surface type
    m_isSheetBody = this->IsSheetBody (acEntity);

    PK_BODY_create_topology_2_o_t   options;
    PK_BODY_create_topology_2_o_m (options);

    // create either a solid or a sheet body
    options.body_type = m_isSheetBody ? PK_BODY_type_sheet_c : PK_BODY_type_solid_c;

    this->StartDebugReport ();

    bool    createdSheet = false;

    // step 1: create Parasolid BRep topology from DWG Brep entity
    status = this->CreateTopologyAndGeometry (results, createdSheet, options);
    if (status == BSISUCCESS)
        {
        // if we have created a sheet body, we are done!
        if (createdSheet)
            {
            CleanUpParasolid (results, true);
            return  status;
            }

        // step 2: convert and attach geometry to the Parasolid topology
        status = this->AttachGeometryToTopology (results.topols, results.n_topols);
        }

    if (status != BSISUCCESS)
        {
        CleanUpParasolid (results, true);
        return  status;
        }

    /*------------------------------------------------------------------------------------------------
    If we have reached here we have successfully created a Parasolid body either from building a full
    topology and attaching geometry, or creating a sheet body directly from a single surface.
    ------------------------------------------------------------------------------------------------*/

    // step 3: share Parasolid geometry
    int nRemoved = 0;
    auto pkError = PK_BODY_share_geom (results.body, true, &nRemoved);

    if (pkError != PK_ERROR_no_errors)
        {
        CleanUpParasolid (results, false);
        status = static_cast<BentleyStatus> (pkError);
        return  status;
        }

    // step 4: attempt repairing Parasolid body
    this->RepairParasolidBody (results.body);

    // step 5: final check
    if (!this->CheckParasolidBody(results.body))
        {
        CleanUpParasolid (results, false);
        return  BSIERROR;
        }

    this->StopDebugReport ();

    return  BSISUCCESS;
    }

};  // BrepConverter


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   UtilsLib::ConvertAsmToParasolid (PK_BODY_create_topology_2_r_t& results, TransformR trans, DwgDbEntityCP acEntity)
    {
    BrepConverter converter;
    return converter.ConvertToParasolid (results, trans, acEntity);
    }

#endif // BENTLEYCONFIG_PARASOLID

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool    UtilsLib::IsAsmEntity (DwgDbEntityCP acEntity)
    {
    return  nullptr != DWGDB_Type(3dSolid)::cast(acEntity) ||
            nullptr != DWGDB_Type(Region)::cast(acEntity) ||
            nullptr != DWGDB_Type(Body)::cast(acEntity) ||
            nullptr != DWGDB_Type(Surface)::cast(acEntity);
    }

END_DWGDB_NAMESPACE
