/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BRepCore/SolidKernel.h>
#include <BRepCore/PSolidUtil.h>
#include <list>
#include <set>
#include <map>
#include <unordered_map>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

BEGIN_UNNAMED_NAMESPACE

#ifndef _EPSILON
#define _EPSILON 1.0e-5
#endif

#ifndef _ROUGH_EPSILON
#define _ROUGH_EPSILON 1.0e-3
#endif

struct PK_MEM_Holder
    {
    private:
        int n_elems;
        void* elems;
    public:
        PK_MEM_Holder(int n, void* e)
            {
            n_elems = n;
            elems = e;
            }
        ~PK_MEM_Holder()
            {
            if (n_elems > 0)
                PK_MEMORY_free(elems);
            }
    };

struct ChiseledConeInfo
    {
    private:
        bool m_validated = false;
        bool m_isValid = false;

    public:
        PK_LOOP_t ends[2] {PK_ENTITY_null, PK_ENTITY_null};
        DEllipse3d ellipses[2];

        bool Validate(double circleEpsilon = _EPSILON, double axisEpsilon = 1.0e-3, double planeEpsilon = _EPSILON);
        bool IsValid() { return m_isValid; }
        void Fill(PK_LOOP_t end1, DEllipse3d ellipse1, PK_LOOP_t end2, DEllipse3d ellipse2, bool valid);
        void SortByEllipses(bool asc);
        bool EqualsByLoop(ChiseledConeInfo const& other);
        bool EqualsByEllipses(ChiseledConeInfo& other, double axisEpsilon);
    };

struct SpunEndInfo
    {
    PK_LOOP_t endLoop = PK_ENTITY_null;
    DEllipse3d end;
    bool endIsPoint = false;
    bool isStart = false;
    };

struct SpunInfo
    {
    private:
        bool m_IsValid = false;

    public:
        PK_SPUN_sf_t spun;
        SpunEndInfo spunEnds[2];
        ICurvePrimitivePtr profile = nullptr;
        int pointedEnds = 0;
        bool Validate();
        bool IsValid() { return m_IsValid; }
    };

struct RevolutionPartInfo
    {
        bool isPlane = false;
        bool isValidPlaneCircles[2]{true, true};
        double tolerance = _EPSILON;
        int nLoops = 0;
        PK_LOOP_t endLoops[2]{PK_ENTITY_null, PK_ENTITY_null};
        DEllipse3d ends[2];
        ICurvePrimitivePtr profile = nullptr;
    };

struct PlaneLoopInfo
    {
    PK_FACE_t face;
    int nLoops;
    int nEdges;
    PK_VECTOR1_t vec;
    };

enum ArcType
    {
    Unknown = 0,
    Full = 1,
    Half = 2,
    Quarter = 3
    };

struct ArcRecoDataInfo
    {
    ArcType type;

    PK_VECTOR1_t startPoint;
    PK_VECTOR1_t quarterPoint;
    PK_VECTOR1_t halfPoint;
    PK_VECTOR1_t endPoint;

    PK_VECTOR1_t arcCenter;
    };

//
struct ExBoxInfo
    {
    PK_FACE_t face;  // face == 0 for empty cap
    PK_VECTOR1_t vec;
    DPoint3d point1;
    DPoint3d point2;
    DPoint3d point3;
    DPoint3d point4;
    };

// classes to ensure unique and sorted points and normals
struct VertexElem
    {
    DPoint3d position;

    VertexElem(DPoint3d pos)
        {
        position = pos;
        }

    bool operator<(const VertexElem& other) const
        {
        if (abs(position.z - other.position.z) < _EPSILON) // z==
            {
            if (abs(position.y - other.position.y) < _EPSILON) // y==
                {
                if (abs(position.x - other.position.x) < _EPSILON)
                    return false;
                return position.x < other.position.x;
                }
            return (position.y < other.position.y);
            }
        return position.z < other.position.z;
        }

    bool operator==(const VertexElem& other) const
        {
        return position.Distance(other.position) < _EPSILON;
        }
    };

struct VectorElem
    {
    PK_VECTOR1_t direction;

    VectorElem(PK_VECTOR1_t dir)
        {
        direction = dir;
        }

    bool operator<(const VectorElem& other) const
        {
        //this may be useful to sort out almost identic vectors
        if (*this == other)
            return false;

        auto coord = (double const*) &direction;
        auto othercoord = (double const*) &(other.direction);
        //! assuming 1 based
        return (abs(coord[0]) + abs(coord[1]) * 2 + abs(coord[2]) * 4) < (abs(othercoord[0]) + abs(othercoord[1]) * 2 + abs(othercoord[2]) * 4);
        }

    bool operator==(const VectorElem& other) const
        {
        PK_LOGICAL_t res;
        PK_VECTOR_is_parallel(direction, other.direction, &res);
        return res == PK_LOGICAL_true;
        }
    };

struct BodyHolder
    {
    private:
        PK_BODY_t m_bodyTag = PK_ENTITY_null;
        int m_facesCount = 0;
        PK_FACE_t* m_faces = nullptr;

        int m_planesCount = 0;
        int m_cylindersCount = 0;
        int m_conesCount = 0;
        int m_spheresCount = 0;
        int m_torusesCount = 0;
        int m_parametricSurfacesCount = 0;
        int m_blendingSurfacesCount = 0;
        int m_offsetSurfacesCount = 0;
        int m_sweptSurfacesCount = 0;
        int m_swungSurfacesCount = 0;
        int m_foreignSurfacesCount = 0;

        CurveVectorPtr ToCurveVector();
        ISolidPrimitivePtr ToSolidPrimitive();
    public:
        BodyHolder(PK_BODY_t bodyTag);
        ~BodyHolder();

        bool IsEmpty() { return m_facesCount == 0; }
        PK_BODY_t GetBody() { return m_bodyTag; }
        int GetNumFaces() { return m_facesCount; }
        PK_FACE_t GetFaceAt(int index);

        bool IsPlane()
            {
            if (m_facesCount == 1 && m_planesCount == 1)
                return true;
            if (m_facesCount == 2 && m_planesCount == 2)
                {
                //check if there are 2 planes but they are reverse coincident (degenerated solid body)
                PK_FACE_is_coincident_o_t coiopts;
                PK_FACE_is_coincident_o_m(coiopts);
                PK_FACE_coi_t coit;
                PK_VECTOR_t outv;
                PK_ERROR_code_t err = PK_FACE_is_coincident(m_faces[0], m_faces[1], _EPSILON, &coiopts, &coit, &outv);
                if (PK_ERROR_no_errors == err && PK_FACE_coi_yes_reversed_c == coit)
                    return true;
                }
            return false;
            }
        bool IsCylinder() { return m_facesCount == 1 && m_cylindersCount == 1; }
        bool IsCone() { return m_facesCount == 1 && m_conesCount == 1; }
        bool IsSphere() { return m_facesCount == 1 && m_spheresCount == 1; }
        bool IsTorus() { return m_facesCount == 1 && m_torusesCount == 1; }
        bool IsParametricSurface() { return m_facesCount == 1 && m_parametricSurfacesCount == 1; }
        bool IsBlendingSurface() { return m_facesCount == 1 && m_blendingSurfacesCount == 1; }
        bool IsOffsetSurface() { return m_facesCount == 1 && m_offsetSurfacesCount == 1; }
        bool IsSweptSurface() { return m_facesCount == 1 && m_sweptSurfacesCount == 1; }
        bool IsSwungSurface() { return m_facesCount == 1 && m_swungSurfacesCount == 1; }
        bool IsForeignSurface() { return m_facesCount == 1 && m_foreignSurfacesCount == 1; }

        bool CanBePipe();
        bool CanBeBox();
        bool CanBeQuadPyramid();
        bool CanBeExtrusion();
        bool CanBeRotationalSweep();

        SimplifiedGeometryType DeduceGeometryType()
            {
            return IsPlane() ? SimplifiedGeometryType::CurveVector : SimplifiedGeometryType::SolidPrimitive;
            }

        IGeometryPtr ProduceSimplifiedGeometry(SimplifiedGeometryType requestedType);
    };

struct EdgeData
    {
    private:
        PK_EDGE_t m_edge;
        PK_FACE_t m_face;
        PK_CURVE_t m_edgeCurve;
        PK_CLASS_t m_edgeCurveClass;
        PK_VECTOR_t m_edgeCurveStartEnd[2];
        PK_INTERVAL_t m_edgeCurveInterval;
        PK_LOGICAL_t  m_edgeCurveSense;

        DPoint3d m_startEnd[2];

        bool m_checkedForLine = false;
        bool m_hasLength = false;
        bool m_hasPrecision = false;

        bool m_isLine = false;
        double m_length = 0;
        double m_precision = 0;

    public:
        EdgeData(PK_EDGE_t edge, PK_FACE_t face);

        bool IsLine();
        bool IsEqualLine(DPoint3d& start, DPoint3d& end);
        double GetLength();
        double GetPrecision();
        DPoint3d GetEndAt(int index);
        bvector<DPoint3d> Get3dPoints(int segmentsCount);
    };

struct pair_hash
    {
    inline size_t operator()(const std::pair<PK_EDGE_t, PK_FACE_t> & v) const
        {
        return (size_t)v.first << 32 | v.second;
        }
    };

struct EdgesHolder
    {
    private:
        std::unordered_map<std::pair<PK_EDGE_t, PK_FACE_t>, EdgeData, pair_hash> m_edges;

    public:
        EdgeData& GetEdgeData(PK_EDGE_t edge, PK_FACE_t face);
    };

struct Processor
    {
    friend SpunInfo;
    friend EdgeData;
    friend ChiseledConeInfo;

    private:
        static BentleyStatus TryGetEllipseFromLoop(DEllipse3dP output, PK_LOOP_t loopTag, double circleEpsilon = _EPSILON, double axisEpsilon = 1.0e-3, double planeEpsilon = _EPSILON);
        static BentleyStatus TryGetEllipseFromPoints(DEllipse3dP output, bvector<DPoint3d> curvePoints, double circleEpsilon = _EPSILON, double axisEpsilon = 1.0e-3, double planeEpsilon = _EPSILON);
        static bvector<DPoint3d> Get3dPointsFromCurve(PK_CURVE_t curve, PK_INTERVAL_t interval, int segmentsCount);
        static DPoint3d Get3dPointFromCurve(PK_CURVE_t curve, double t);
        static DPoint3d Get3dPointFromVertex(PK_VERTEX_t vertex);
        static bool IsLoopJustPoint(PK_LOOP_t loopTag, DPoint3dP point);
        static bool TryGetVertexFromLoopByIndex(PK_LOOP_t loopTag, int index, DPoint3dP point);
        static bool IsLoopsSame(PK_LOOP_t loopTag1, PK_LOOP_t loopTag2);
        static bool IsEllipsesSame(DEllipse3dCR ellipse1, DEllipse3dCR ellipse2, double axisEpsilon = _EPSILON);
        static bool IsPointsInSamePlane(bvector<DPoint3d> points, DVec3dP normal, double planeEpsilon = _EPSILON);
        static bool IsEdgesHaveSameForm(PK_EDGE_t edge1, PK_FACE_t face1, PK_EDGE_t edge2, PK_FACE_t face2, DVec3d translation, int segmentsCount);
        static bool IsEdgesHaveSameForm(EdgeData& edge1, EdgeData& edge2, DVec3d translation, int segmentsCount);
        static BentleyStatus GetEdgeCurve(PK_EDGE_t edge, PK_FACE_t face, PK_CURVE_t *edgeCurve, PK_CLASS_t *edgeCurveClass, PK_VECTOR_t edgeCurveEnds[2], PK_INTERVAL_t *edgeCurveInterval, PK_LOGICAL_t  *edgeCurveSense);
        static double GetPrecision(int entity, double defaultPrecision = _ROUGH_EPSILON);
        static int FaceComplexity(PK_FACE_t face);
        static bool IsOrthogonal(PK_VECTOR_t a, PK_VECTOR_t b);
        static bool IsCollinear(PK_VECTOR_t a, PK_VECTOR_t b);
        static bool ProjectFace(PK_FACE_t face1, PK_FACE_t face2, PK_VECTOR1_t vec, PK_FACE_t * outFace);
        static bool TryConvertBSplineToArc(PK_BCURVE_t inputBCurve, PK_CURVE_t * outputCurve, PK_INTERVAL_t * outputInterval);
        static void DetectPossibleArcType(PK_BCURVE_t inputBCurve, ArcRecoDataInfo *data);
        static bool IsCoincident(PK_CURVE_t curve1, PK_INTERVAL_t interval1, PK_CURVE_t curve2, PK_INTERVAL_t interval2);
        static bool CreateArcFromInfo(ArcRecoDataInfo info, PK_CURVE_t * outputCurve, PK_INTERVAL_t * outputInterval);
        static void GetPerpendiculars(PK_VECTOR1_t axis, PK_VECTOR1_t * norm1, PK_VECTOR1_t * norm2);

        static ISolidPrimitivePtr ConstructChiseledConesFromSectionsWithUnrelatedEllipseDefinitions
        (
            bvector<DEllipse3d> profile,    // sequence of ellipses
            bool isCapped                   // Is ruled sweep capped
        );
        static ISolidPrimitivePtr ConstructChiseledPipesFromSectionsWithUnrelatedEllipseDefinitions
        (
            bvector<ChiseledConeInfo> profile,  // sequence of 'rings'
            bool isCapped                       // Is ruled sweep capped
        );
        static DVec3d ConstructCorrespondingEllipseVector
        (
            DPoint3dCR centerA,         // center of ellipse A
            DVec3dCR   vectorA,         // any vector from centerA to a point on ellipseA
            DPoint3dCR centerB,         // center of ellipseB
            DVec3dCR   vectorB0,        // 0 degree vector (e.g. major axis)
            DVec3dCR   vectorB90        // 90 degree vector (e.g. minor axis
        );
        static CurveVectorPtr CreateSplitDisk(DEllipse3dCR arc, CurveVector::BoundaryType boundaryType = CurveVector::BOUNDARY_TYPE_Outer);
        static CurveVectorPtr CreateTriangle(PK_VERTEX_t * vertices);
        static ICurvePrimitivePtr Create3dCurve(PK_CURVE_t curve);
        static ICurvePrimitivePtr Create3dCurveByInterval(PK_CURVE_t curve, PK_INTERVAL_t curveInterval, bool simplifyBsplines = true);
        static ICurvePrimitivePtr Create3dCurveFromSpCurve(PK_CURVE_t curve, PK_INTERVAL_t curveInterval, PK_VECTOR_t curveEnds[2]);
        static ICurvePrimitivePtr Create3dCurveFromEdge(PK_EDGE_t edge, PK_FACE_t face);
        static ICurvePrimitivePtr CreateBSplineFromEllipse(DEllipse3dCR arc);
        static CurveVectorPtr GetCurveVectorFromLoop(PK_LOOP_t loopTag, CurveVector::BoundaryType boundaryType, double epsilon = _EPSILON);
        static BentleyStatus ProcessParasolidPlane(PK_PLANE_t Surface, PK_LOOP_t *loops, PK_BODY_t bodyTag, PlaneLoopInfo& loopInfo, ExBoxInfo& exboxInfo, std::set<VertexElem>& vertexSet, std::set<VectorElem>& vectorSet);

        static ISolidPrimitivePtr CreateBox(std::set<VertexElem> vertexSet);
        static ISolidPrimitivePtr CreateBoxEx(std::set<VectorElem> vectorSet, std::list<ExBoxInfo>& boxInfo, int nFaces, const PK_FACE_t* faces);
        static ISolidPrimitivePtr CreateExtrude(PK_FACE_t face, PK_FACE_t face2);

        static DRange3d LoopExtents(PK_LOOP_t loop);
        static DRange3d GetExtents(PK_TOPOL_t topol);
        static bool LoopIsInLoop(PK_LOOP_t loop, PK_LOOP_t loopCheck);
        static bool FaceIsPlane(PK_FACE_t face);
        static bool FaceGetPlane(PK_FACE_t face, DVec3d& normal, DPlane3d& plane);
        static DPoint3d VertexGetCoordinates(PK_VERTEX_t vertex);
        static bool EdgeIsLine(PK_FACE_t face, PK_EDGE_t edge);
        static bool IsEdgeEqualLine(PK_FACE_t face, PK_EDGE_t edge, DPoint3d& start, DPoint3d& end);
        static bool FacesAreOppositeEx(EdgesHolder& edgesHolder,PK_FACE_t face1, PK_FACE_t face2, PK_FACE_t*  pFaceTagArray, int faceCount, DVec3d& extrusionVector);
        static bool IsSolidBodyExtrusion(PK_BODY_t bodyTag, PK_FACE_t face1, DRange3d *face1Range, PK_FACE_t face2, DRange3d *face2Range, PK_FACE_t*  pFaceTagArray, int faceCount, DVec3d& extrusionVector);
        static bool IsSolidBodySkewedExtrusion(PK_BODY_t bodyTag, PK_FACE_t face1, DRange3d *face1Range, PK_FACE_t face2, DRange3d *face2Range, PK_FACE_t*  pFaceTagArray, int faceCount, PK_FACE_t * projectedFace);
        static bool AreFacesConnectedAlongVector(PK_FACE_t* pFaceTagArray, int faceCount, PK_FACE_t face1, PK_FACE_t face2, PK_VECTOR1_t extVec);
        static bool MakeSectionsCompatible(CurveVectorPtr section1, CurveVectorPtr section2);

        static DPoint3d P(PK_VECTOR_t vector);
        static DPoint3d P(PK_POINT_sf_t point);
        static DVec3d V(PK_VECTOR_t vector);
        static PK_VECTOR_t PK_VEC(DVec3dCR vector);
        static PK_VECTOR_t PK_VEC_Diff(PK_VECTOR_t vector0, PK_VECTOR_t vector1);
        static bool PK_VEC_Zero(PK_VECTOR_t vector);

    public:
        static ISolidPrimitivePtr ParasolidChiseledConesToDgnRuledSweep(PK_BODY_t bodyTag);
        static ISolidPrimitivePtr ParasolidConeToDgnRotationalSweep(PK_BODY_t bodyTag);
        static ISolidPrimitivePtr ParasolidSpunToDgnRotationalSweep(PK_BODY_t bodyTag);
        static ISolidPrimitivePtr ParasolidSewedSpunsToDgnRotationalSweep(PK_BODY_t bodyTag);
        static ISolidPrimitivePtr ParasolidComplexBodyToDgnRotationalSweep(PK_BODY_t bodyTag);
        static ISolidPrimitivePtr ParasolidBodyToDgnBox(PK_BODY_t bodyTag);
        static ISolidPrimitivePtr ParasolidPipeToDgnRuledSweep(PK_BODY_t bodyTag);
        static ISolidPrimitivePtr ParasolidExtrusionToDgnExtrusion(PK_BODY_t bodyTag);
        static ISolidPrimitivePtr ParasolidSkewedExtrusionToDgnRuledSweep(PK_BODY_t bodyTag);
        static ISolidPrimitivePtr ParasolidQuadPyramidToDgnRuledSweep(PK_BODY_t bodyTag);
        static ISolidPrimitivePtr ParasolidTorusSegmentBodyToDgnRotationalSweep(PK_BODY_t bodyTag);
        static ISolidPrimitivePtr ParasolidRevolutionToDgnRotationalSweep(PK_BODY_t bodyTag);

        static CurveVectorPtr ParasolidFaceToCurveVector(PK_FACE_t face);
    };

END_UNNAMED_NAMESPACE

inline bool GE(double a, double b)
    {
    return (fabs(a - b) < _EPSILON) ? true : (a > b);
    }
inline bool GT(double a, double b)
    {
    return (fabs(a - b) < _EPSILON) ? false : (a > b);
    }
inline bool LE(double a, double b)
    {
    return (fabs(a - b) < _EPSILON) ? true : (a < b);
    }
inline bool LT(double a, double b)
    {
    return (fabs(a - b) < _EPSILON) ? false : (a < b);
    }
inline bool EQ(double a, double b)
    {
    return (fabs(a - b) < _EPSILON) ? true : false;
    }

inline bool IS_CONTANED(DRange3d& ri, DRange3d& ro)
    {
    return  (GE(ri.low.x, ro.low.x)
             && GE(ri.low.y, ro.low.y)
             && GE(ri.low.z, ro.low.z)
             && LE(ri.high.x, ro.high.x)
             && LE(ri.high.y, ro.high.y)
             && LE(ri.high.z, ro.high.z));
    }

BodyHolder::~BodyHolder()
    {
    if (m_facesCount > 0)
        PK_MEMORY_free(m_faces);
    }

BodyHolder::BodyHolder(PK_BODY_t bodyTag)
    {
    m_bodyTag = bodyTag;

    // Extract faces from body.
    PK_BODY_ask_faces(m_bodyTag, &m_facesCount, &m_faces);

    // Collect faces statistic.
    for (int i = 0; i < m_facesCount; i++)
        {
        // Extract surface from face.
        PK_SURF_t surf;
        PK_FACE_ask_surf(m_faces[i], &surf);

        // Extract class of surface entity.
        PK_CLASS_t surfaceClass;
        PK_ENTITY_ask_class(surf, &surfaceClass);

        switch (surfaceClass)
            {
                case PK_CLASS_plane: // 4001 plane
                    m_planesCount++;
                    break;
                case PK_CLASS_cyl: // 4002 cylinder
                    m_cylindersCount++;
                    break;
                case PK_CLASS_cone: // 4003 cone
                    m_conesCount++;
                    break;
                case PK_CLASS_sphere: // 4004 sphere
                    m_spheresCount++;
                    break;
                case PK_CLASS_torus: // 4005 torus
                    m_torusesCount++;
                    break;
                case PK_CLASS_bsurf: // 4006 parametric surface
                    m_parametricSurfacesCount++;
                    break;
                case PK_CLASS_blendsf: // 4007 blending surface
                    m_blendingSurfacesCount++;
                    break;
                case PK_CLASS_offset: // 4008 offset surface
                    m_offsetSurfacesCount++;
                    break;
                case PK_CLASS_swept: // 4009 swept surface
                    m_sweptSurfacesCount++;
                    break;
                case PK_CLASS_spun: // 4010 swung surface
                    m_swungSurfacesCount++;
                    break;
                case PK_CLASS_fsurf:// 4011 foreign surface
                    m_foreignSurfacesCount++;
                    break;
            }
        }
    }

PK_FACE_t BodyHolder::GetFaceAt(int index)
    {
    if (index >= 0 && index < m_facesCount)
        return m_faces[index];

    return PK_ENTITY_null;
    }

bool BodyHolder::CanBePipe()
    {
    if (IsEmpty())
        return false;

    // DgnRuledSweep can be constructed if body consists of
    // planes, cylinders and cones.
    // Only 0 or 2 planes (for caps) are allowed.
    if (m_planesCount != 0 && m_planesCount != 2)
        return false;

    return m_planesCount + m_cylindersCount + m_conesCount == m_facesCount;
    }

bool BodyHolder::CanBeQuadPyramid()
    {
    if (IsEmpty())
        return false;

    // QuadPyramid can be constructed if body consists of 5 planes.
    return m_planesCount == 5 && m_planesCount == m_facesCount;
    }

bool BodyHolder::CanBeBox()
    {
    if (IsEmpty())
        return false;

    // DgnBox can be constructed if body consists of planes.
    // Should be at least 4 planes.
    return m_planesCount > 3 && m_planesCount == m_facesCount;
    }

bool BodyHolder::CanBeExtrusion()
    {
    if (IsEmpty())
        return false;

    // DgnExtrusion can be constructed if body consists of
    // planes, cylinders, cones, swung surfaces.
    // Should be at least 4 faces and 2 planes.
    if (m_facesCount < 4 || m_planesCount < 2)
        return false;

    return m_planesCount + m_cylindersCount + m_conesCount + m_swungSurfacesCount == m_facesCount;
    }

bool BodyHolder::CanBeRotationalSweep()
    {
    if (IsEmpty())
        return false;
    // Exclude only planes or mesh-like topologies
    if (m_planesCount == m_facesCount)
        return false;
    // DgnRotationalSweep can be constructed if body consists of
    // planes, cylinders, cones, swung, tori, spheres surfaces.
    return m_planesCount + m_cylindersCount + m_conesCount + m_swungSurfacesCount + m_torusesCount + m_spheresCount == m_facesCount;
    }


EdgeData::EdgeData(PK_EDGE_t edge, PK_FACE_t face)
    {
    m_edge = edge;
    m_face = face;
    Processor::GetEdgeCurve(edge, face, &m_edgeCurve, &m_edgeCurveClass, m_edgeCurveStartEnd, &m_edgeCurveInterval, &m_edgeCurveSense);
    m_startEnd[0] = Processor::P(m_edgeCurveStartEnd[0]);
    m_startEnd[1] = Processor::P(m_edgeCurveStartEnd[1]);
    }

bool EdgeData::IsLine()
    {
    if (!m_checkedForLine)
        {
        // Check edge curve to "line" type
        switch (m_edgeCurveClass)
            {
                case PK_CLASS_line:
                    m_isLine = true;
                case PK_CLASS_spcurve:
                {
                double length = GetLength();
                double distance = m_startEnd[0].Distance(m_startEnd[1]);

                // If curve length and distance between ends are the same, it's a line.
                if (fabs(length - distance) < _EPSILON)
                    m_isLine = true;
                }
                break;
            }

        m_checkedForLine = true;
        }

    return m_isLine;
    }

bool EdgeData::IsEqualLine(DPoint3d& start, DPoint3d& end)
    {
    if (!IsLine())
        return false;

    if ((m_startEnd[0].IsEqual(start, _EPSILON) && m_startEnd[1].IsEqual(end, _EPSILON)) ||
        (m_startEnd[1].IsEqual(start, _EPSILON) && m_startEnd[0].IsEqual(end, _EPSILON)))
        return true;

    return false;
    }

double EdgeData::GetLength()
    {
    if (!m_hasLength)
        {
        PK_INTERVAL_t range = {0, 0};
        PK_CURVE_find_length(m_edgeCurve, m_edgeCurveInterval, &m_length, &range);
        m_hasLength = true;
        }

    return m_length;
    }

double EdgeData::GetPrecision()
    {
    if (!m_hasPrecision)
        {
        m_precision = Processor::GetPrecision(m_edge);
        m_hasPrecision = true;
        }

    return m_precision;
    }

DPoint3d EdgeData::GetEndAt(int index)
    {
    if (index >= 0 && index < 2)
        return m_startEnd[index];

    return DPoint3d::FromZero();
    }

bvector<DPoint3d> EdgeData::Get3dPoints(int segmentsCount)
    {
    return Processor::Get3dPointsFromCurve(m_edgeCurve, m_edgeCurveInterval, segmentsCount);
    }


EdgeData& EdgesHolder::GetEdgeData(PK_EDGE_t edge, PK_FACE_t face)
    {
    std::pair<PK_EDGE_t, PK_FACE_t> key = std::make_pair(edge, face);
    auto iter = m_edges.find(key);
    if (iter != m_edges.end())
        return iter->second;

    return m_edges.emplace(std::make_pair(key, EdgeData(edge, face))).first->second;
    }


DPoint3d Processor::P(PK_VECTOR_t vector)
    {
    return DPoint3d::From(vector.coord[0], vector.coord[1], vector.coord[2]);
    }

DPoint3d Processor::P(PK_POINT_sf_t point)
    {
    return DPoint3d::FromArray(point.position.coord);
    }

DVec3d Processor::V(PK_VECTOR_t vector)
    {
    return DVec3d::From(vector.coord[0], vector.coord[1], vector.coord[2]);
    }

PK_VECTOR_t Processor::PK_VEC(DVec3dCR vector)
    {
    PK_VECTOR_t ret;
    ret.coord[0] = vector.x;
    ret.coord[1] = vector.y;
    ret.coord[2] = vector.z;
    return ret;
    }

PK_VECTOR_t Processor::PK_VEC_Diff(PK_VECTOR_t vector0, PK_VECTOR_t vector1)
    {
    PK_VECTOR_t ret;
    ret.coord[0] = vector0.coord[0] - vector1.coord[0];
    ret.coord[1] = vector0.coord[1] - vector1.coord[1];
    ret.coord[2] = vector0.coord[2] - vector1.coord[2];
    return ret;
    }

bool Processor::PK_VEC_Zero(PK_VECTOR_t vector)
    {
    if (fabs(vector.coord[0]) > _EPSILON ||
        fabs(vector.coord[1]) > _EPSILON ||
        fabs(vector.coord[2]) > _EPSILON)
        return false;
    return true;
    }

// Tries to convert Parasolid cone (and cylinder as particular case of cone)
// sequence to DgnRuledSweep.
ISolidPrimitivePtr Processor::ParasolidChiseledConesToDgnRuledSweep(PK_BODY_t bodyTag)
    {
    // Extract faces from body.
    int n_faces;
    PK_FACE_t* faces = nullptr;
    PK_BODY_ask_faces(bodyTag, &n_faces, &faces);
    PK_MEM_Holder fholder(n_faces, faces);

    if (n_faces < 1)
        return nullptr;

    // Trimming ellipses of cones.
    bvector<ChiseledConeInfo> pipes;
    PK_LOOP_t startLoop = PK_ENTITY_null, endLoop = PK_ENTITY_null;

    // Caps (if exists).
    PK_LOOP_t caps[2]{PK_ENTITY_null, PK_ENTITY_null};
    int capsCount = 0;

    for (int iFace = 0; iFace < n_faces; iFace++)
        {
        // Extract surface from face.
        PK_SURF_t surf;
        PK_FACE_ask_surf(faces[iFace], &surf);

        // Extract class of surface entity.
        PK_CLASS_t surfaceClass;
        PK_ENTITY_ask_class(surf, &surfaceClass);

        // Extract trimming loops.
        int n_loops;
        PK_LOOP_t *loops;
        PK_FACE_ask_loops(faces[iFace], &n_loops, &loops);
        PK_MEM_Holder lholder(n_loops, loops);

        switch (surfaceClass)
            {
            case PK_CLASS_plane: // 4001 plane
                if (n_loops == 1)
                    {
                    if (capsCount < 2)
                        {
                        caps[capsCount] = loops[0];
                        capsCount++;
                        }
                    else
                        return nullptr; // 3-rd face is a plane. It's not a cones sequence.
                    }
                else
                    return nullptr; // It's not an elliptical cap.
                break;
            case PK_CLASS_cyl: // 4002 cylinder
            case PK_CLASS_cone: // 4003 cone
                // Process trimming loops.
                if (n_loops == 2)
                    {
                    ChiseledConeInfo pipe;
                    pipe.ends[0] = loops[0];
                    pipe.ends[1] = loops[1];
                    pipes.push_back(pipe);
                    }
                else
                    return nullptr; // It's not a cones sequence.
                break;
            default:
                return nullptr; // It's not a cones sequence.
            }
        }

    // Should be 0 or 2 caps.
    if ((capsCount != 0 && capsCount != 2) ||
        (capsCount == 2 && (caps[0] == PK_ENTITY_null || caps[1] == PK_ENTITY_null)))
        return nullptr;

    // Should be at least 1 cone.
    if (pipes.size() < 1)
        return nullptr;

    // Fill profile sequence by ellipses.
    bvector<DEllipse3d> profile;
    for(int i = 0; i < pipes.size(); i++)
        {
        DEllipse3d end1, end2;
        if (TryGetEllipseFromLoop(&end1, pipes[i].ends[0], _EPSILON, 3.0e-3, _ROUGH_EPSILON) == SUCCESS &&
            TryGetEllipseFromLoop(&end2, pipes[i].ends[1], _EPSILON, 3.0e-3, _ROUGH_EPSILON) == SUCCESS)
            {
            // Most frequent case - just one pipe.
            if(i == 0)
                {
                profile.push_back(end1);
                profile.push_back(end2);
                startLoop = pipes[i].ends[0];
                endLoop = pipes[i].ends[1];
                }
            // Pipe ends are not sorted.
            // So check ends order of first pipe if profile has more than 1 pipe.
            else if(i == 1)
                {
                if(IsEllipsesSame(profile[0], end1, 0.01) || IsEllipsesSame(profile[0], end2, 0.01))
                    {
                    DEllipse3d tempEl = profile[0];
                    profile[0] = profile[1];
                    profile[1] = tempEl;

                    PK_LOOP_t tempLoop = startLoop;
                    startLoop = endLoop;
                    endLoop = tempLoop;
                    }
                }

            // Check that pipe in sequence has common end with previous one
            // and add other end to profile.
            if(i > 0)
                {
                if (IsEllipsesSame(profile[i], end1, 0.01))
                    {
                    profile.push_back(end2);
                    endLoop = pipes[i].ends[1];
                    }
                else if (IsEllipsesSame(profile[i], end2, 0.01))
                    {
                    profile.push_back(end1);
                    endLoop = pipes[i].ends[0];
                    }
                else
                    return nullptr;
                }
            }
        else
            return nullptr;
        }

    // Check that caps are really caps of cones sequence (if exists).
    if(capsCount == 2)
        {
        if (!(IsLoopsSame(startLoop, caps[0]) || IsLoopsSame(startLoop, caps[1])) ||
            !(IsLoopsSame(endLoop, caps[0]) || IsLoopsSame(endLoop, caps[1])))
            {
            // Loops curves of cylinder and caps are not same (by tags).
            // But loops geometry can be the same. So check.
            // For circular caps epsilon 0.01 is enough due to usage rough tolerances
            // on Parasolid bodies creation.
            DEllipse3d cap1, cap2;
            if (TryGetEllipseFromLoop(&cap1, caps[0], 0.01, 3.0e-3, _ROUGH_EPSILON) == SUCCESS &&
                TryGetEllipseFromLoop(&cap2, caps[1], 0.01, 3.0e-3, _ROUGH_EPSILON) == SUCCESS)
                {
                // Compare ellipses of cylinder ends with caps.
                // Epsilon 0.01 seems enough due to usage rough tolerances
                // on Parasolid bodies creation.
                if (!(IsEllipsesSame(profile[0], cap1, 0.01) || IsEllipsesSame(profile[0], cap2, 0.01)) ||
                    !(IsEllipsesSame(profile.back(), cap1, 0.01) || IsEllipsesSame(profile.back(), cap2, 0.01)))
                    return nullptr;
                }
            else
                return nullptr;
            }
        }

    return ConstructChiseledConesFromSectionsWithUnrelatedEllipseDefinitions(profile, capsCount == 2);
    }

// Tries to convert trimmed Parasolid cone (or cylinder as particular case) to DgnRotationalSweep.
ISolidPrimitivePtr Processor::ParasolidConeToDgnRotationalSweep(PK_BODY_t bodyTag)
    {
    // Extract faces from body.
    int n_faces;
    PK_FACE_t* faces = nullptr;
    PK_BODY_ask_faces(bodyTag, &n_faces, &faces);
    PK_MEM_Holder fholder(n_faces, faces);

    // Should be 1 face.
    if (n_faces != 1)
        return nullptr;

    // Extract surface from face.
    PK_SURF_t surf;
    PK_FACE_ask_surf(faces[0], &surf);

    // Extract class of surface entity.
    PK_CLASS_t surfaceClass;
    PK_ENTITY_ask_class(surf, &surfaceClass);

    // Check that surface is a cone or cylinder.
    if (surfaceClass != PK_CLASS_cone && surfaceClass != PK_CLASS_cyl)
        return nullptr;

    // Extract trimming loops.
    int n_loops;
    PK_LOOP_t *loops;
    PK_FACE_ask_loops(faces[0], &n_loops, &loops);
    PK_MEM_Holder lholder(n_loops, loops);

    // Should be 1 loop.
    if (n_loops != 1)
        return nullptr;

    // Extract edges from loop.
    int n_edges;
    PK_EDGE_t *edges;
    PK_LOOP_ask_edges(loops[0], &n_edges, &edges);
    PK_MEM_Holder eholder(n_edges, edges);

    // Should be 4 edges.
    if (n_edges != 4)
        return nullptr;

    // Extract cone info.
    DVec3d coneAxis;
    DPoint3d center;
    double radius = 0.0;
    bool isCone = true;
    if (surfaceClass == PK_CLASS_cone)
        {
        PK_CONE_sf_t cone;
        PK_CONE_ask(surf, &cone);
        coneAxis = V(cone.basis_set.axis);
        center = P(cone.basis_set.location);
        }
    else
        {
        PK_CYL_sf_t cyl;
        PK_CYL_ask(surf, &cyl);
        coneAxis = V(cyl.basis_set.axis);
        center = P(cyl.basis_set.location);
        radius = cyl.radius;
        isCone = false;
        }

    bvector<DSegment3d> lines;
    bvector<DSegment3d> arcs;
    double sweepAngle = 0;
    double sign = 1.0;

    // For rotational sweep creation edges should be following:
    // - 2 lines with same lengths
    // - 2 circular arcs with same sweep angles
    for (int iEdge = 0; iEdge < n_edges; iEdge++)
        {
        // Extract curve from edge.
        PK_CURVE_t edgeCurve;
        PK_CLASS_t edgeCurveClass = PK_CLASS_null;
        PK_INTERVAL_t edgeCurveInterval = {0, 0};
        PK_VECTOR_t edgeCurveEnds[2];
        PK_LOGICAL_t  edgeCurveSense;
        GetEdgeCurve(edges[iEdge], faces[0], &edgeCurve, &edgeCurveClass, edgeCurveEnds, &edgeCurveInterval, &edgeCurveSense);

        // Edge curve was not found.
        // Something went wrong.
        if (edgeCurve == PK_ENTITY_null)
            return nullptr;

        switch (edgeCurveClass)
            {
            case PK_CLASS_line: // 3001 line
                {
                lines.push_back(DSegment3d::From(P(edgeCurveEnds[0]), P(edgeCurveEnds[1])));
                }
                break;
            case PK_CLASS_circle: // 3002 circle
                {
                PK_CIRCLE_sf_t circle;
                PK_CIRCLE_ask(edgeCurve, &circle);
                // Normal of plane should be parallel to cone axis.
                DVec3d elNormal = V(circle.basis_set.axis);
                if (elNormal.IsParallelTo(coneAxis))
                    {
                    // First arc is used as sweep arc.
                    // If arc normal is opposite to cone axis, sweep angle should be negated.
                    if (arcs.size() == 0)
                        {
                        double dot = elNormal.DotProduct(coneAxis);
                        if (dot < 0.0)
                            sign = -1.0;
                        }

                    arcs.push_back(DSegment3d::From(P(edgeCurveEnds[0]), P(edgeCurveEnds[1])));

                    // Sweep angles should be the same.
                    if (sweepAngle > 0 && fabs(sweepAngle - (edgeCurveInterval.value[1] - edgeCurveInterval.value[0]) > _EPSILON))
                        return nullptr;
                    sweepAngle = edgeCurveInterval.value[1] - edgeCurveInterval.value[0];
                    }
                }
                break;
            case PK_CLASS_ellipse: // 3003 ellipse
                {
                PK_ELLIPSE_sf_t ellipse;
                PK_ELLIPSE_ask(edgeCurve, &ellipse);
                // Normal of plane should be parallel to cone axis.
                DVec3d elNormal = V(ellipse.basis_set.axis);
                if (elNormal.IsParallelTo(coneAxis))
                    {
                    // First arc is used as sweep arc.
                    // If arc normal is opposite to cone axis, sweep angle should be negated.
                    if (arcs.size() == 0)
                        {
                        double dot = elNormal.DotProduct(coneAxis);
                        if (dot < 0.0)
                            sign = -1.0;
                        }

                    arcs.push_back(DSegment3d::From(P(edgeCurveEnds[0]), P(edgeCurveEnds[1])));

                    // Sweep angles should be the same.
                    if (sweepAngle > 0 && fabs(sweepAngle - (edgeCurveInterval.value[1] - edgeCurveInterval.value[0]) > _EPSILON))
                        return nullptr;
                    sweepAngle = edgeCurveInterval.value[1] - edgeCurveInterval.value[0];
                    }
                }
                break;
            case PK_CLASS_spcurve: // 3006 2d bspline curve (sp curve)
                {
                double length = 0;
                PK_INTERVAL_t range = {0, 0};
                PK_CURVE_find_length(edgeCurve, edgeCurveInterval, &length, &range);
                DPoint3d end1 = P(edgeCurveEnds[0]);
                DPoint3d end2 = P(edgeCurveEnds[1]);
                double distance = end1.Distance(end2);

                // If curve length and distance between ends are the same, it's a line.
                // BUT: If such line is perpendicular to cone axis (zero dot product), it's a small arc!
                if (fabs(length - distance) < _EPSILON && fabs(coneAxis.DotProduct(end2 - end1)) > _EPSILON)
                    lines.push_back(DSegment3d::From(end1, end2));
                else
                    {
                    // Try to detect circular arc.
                    // Curve points should be in the same plane.
                    // Normal of plane should be parallel to cone axis.
                    bvector<DPoint3d> points = Get3dPointsFromCurve(edgeCurve, edgeCurveInterval, 120);
                    DVec3d normal;
                    if (!IsPointsInSamePlane(points, &normal))
                        return nullptr;

                    if (normal.IsParallelTo(coneAxis))
                        {
                        // First arc is used as sweep arc.
                        // If arc normal is opposite to cone axis, sweep angle should be negated.
                        if (arcs.size() == 0)
                            {
                            double dot = normal.DotProduct(coneAxis);
                            if (dot < 0.0)
                                sign = -1.0;
                            }

                        arcs.push_back(DSegment3d::From(end1, end2));

                        // Radius of cylinder is constant known value.
                        // In case of cone calculate radius for trimming circle.
                        double r = radius;
                        if (isCone)
                            {
                            DVec3d v = end1 - center; // Vector from cone center to start point of trimming curve (on cone surface).
                            // Find angle between cone axis and vector.
                            // Angle should be between 0 and half of PI.
                            double alpha = v.AngleTo(coneAxis);
                            if (alpha > PI / 2.0)
                                alpha = PI - alpha;
                            r = v.Magnitude() * sin(alpha); // Radius of trimming curve (circle).
                            }
                        double sweep = length / r; // Arc sweep angle in radians.

                        // Sweep angles should be the same.
                        if (sweepAngle > 0 && fabs(sweepAngle - sweep) > _EPSILON)
                            return nullptr;
                        sweepAngle = sweep;
                        }
                    else
                        return nullptr;
                    }
                }
                break;
            default:
                return nullptr;
            }
        }

    if (arcs.size() == 2 && lines.size() == 2)
        {
        // Lines should have same length.
        if (fabs(lines[0].Length() - lines[1].Length()) > _EPSILON)
            return nullptr;

        DSegment3d profile = lines[0];
        if (lines[1].point[0].Distance(arcs[0].point[0]) < _EPSILON || lines[1].point[1].Distance(arcs[0].point[0]) < _EPSILON)
            profile = lines[1];

        CurveVectorPtr curveVector(new CurveVector(CurveVector::BoundaryType::BOUNDARY_TYPE_Open));
        curveVector->Add(ICurvePrimitive::CreateLine(profile));
        DgnRotationalSweepDetail rotationDetails(curveVector, center, coneAxis, sweepAngle * sign, false);
        return ISolidPrimitive::CreateDgnRotationalSweep(rotationDetails);
        }

    return nullptr;
    }

// Tries to create DgnRotationalSweep from Parasolid spun surface.
ISolidPrimitivePtr Processor::ParasolidSpunToDgnRotationalSweep(PK_BODY_t bodyTag)
    {
    // Extract faces from body.
    int n_faces;
    PK_FACE_t* faces = nullptr;
    PK_BODY_ask_faces(bodyTag, &n_faces, &faces);
    PK_MEM_Holder fholder(n_faces, faces);

    // Should be 1, 2 or 3 faces.
    if (n_faces < 1 || n_faces > 3)
        return nullptr;

    // Spun info.
    SpunInfo spunInfo;

    // Caps (if exists).
    PK_LOOP_t caps[2]{PK_ENTITY_null, PK_ENTITY_null};
    PK_FACE_t capFace[2]{PK_ENTITY_null, PK_ENTITY_null};
    int capsCount = 0;

    for (int iFace = 0; iFace < n_faces; iFace++)
        {
        // Extract surface from face.
        PK_SURF_t surf;
        PK_FACE_ask_surf(faces[iFace], &surf);

        // Extract class of surface entity.
        PK_CLASS_t surfaceClass;
        PK_ENTITY_ask_class(surf, &surfaceClass);

        // Extract trimming loops.
        int n_loops;
        PK_LOOP_t *loops;
        PK_FACE_ask_loops(faces[iFace], &n_loops, &loops);
        PK_MEM_Holder lholder(n_loops, loops);

        switch (surfaceClass)
            {
            case PK_CLASS_plane: // 4001 plane
                if (n_loops == 1)
                    {
                    if (capsCount < 2)
                        {
                        caps[capsCount] = loops[0];
                        capFace[capsCount] = faces[iFace];
                        capsCount++;
                        }
                    else
                        return nullptr; // 3-rd face is a plane. It's not a capped spun.
                    }
                else
                    return nullptr; // It's not an circular cap.
                break;
            case PK_CLASS_spun: // 4010 swung surface
                if (n_loops == 2 && spunInfo.spunEnds[0].endLoop == PK_ENTITY_null)
                    {
                    spunInfo.spunEnds[0].endLoop = loops[0];
                    spunInfo.spunEnds[1].endLoop = loops[1];
                    PK_SPUN_ask(surf, &spunInfo.spun);
                    }
                else
                    return nullptr; // It's not fit spun surface.
                break;
            default:
                return nullptr; // It's not a spun surface.
            }
        }

    // Check that spun is valid.
    if (!spunInfo.Validate())
        return nullptr;

    // If end is a point, appropriate cap is missing. And vice versa.
    if (spunInfo.pointedEnds + capsCount > 2)
        return nullptr;

    // Check that caps are really caps of spun surface (if exists).
    int capsFound = 0;
    DSegment3d startCap, endCap;
    startCap.InitZero();
    endCap.InitZero();
    DPoint3d ptProfile[2];
    spunInfo.profile->GetStartEnd(ptProfile[0], ptProfile[1]);
    for (int i = 0; i < capsCount; i++)
        for (int j = 0; j < 2; j++)
        {
        DEllipse3d cap;
        // Check that spun loops curves and caps are the same (by tags).
        // If not, loops geometry can be the same. So check.
        // For circular caps epsilon 0.01 is enough due to usage rough tolerances
        // on Parasolid bodies creation.
        if(!spunInfo.spunEnds[j].endIsPoint)
            if (IsLoopsSame(spunInfo.spunEnds[j].endLoop, caps[i]) ||
                (TryGetEllipseFromLoop(&cap, caps[i], 0.01) == SUCCESS && IsEllipsesSame(spunInfo.spunEnds[j].end, cap, 0.01)))
                {
                capsFound++;
                if (spunInfo.spunEnds[j].isStart)
                    startCap = DSegment3d::From(spunInfo.spunEnds[j].end.center, ptProfile[0]); // From axis to start point of profile.
                else
                    endCap = DSegment3d::From(ptProfile[1], spunInfo.spunEnds[j].end.center); // From end point of profile to axis.
                }
        }

    // Some caps are not really caps.
    if (capsCount != capsFound)
        return nullptr;

    CurveVectorPtr curveVector(new CurveVector(CurveVector::BoundaryType::BOUNDARY_TYPE_Open));
    if(startCap.Length() != 0)
        curveVector->Add(ICurvePrimitive::CreateLine(startCap));
    curveVector->Add(spunInfo.profile);
    if (endCap.Length() != 0)
        curveVector->Add(ICurvePrimitive::CreateLine(endCap));
    DgnRotationalSweepDetail rotationDetails(curveVector, P(spunInfo.spun.axis.location), V(spunInfo.spun.axis.axis), PI * 2.0, false);
    return ISolidPrimitive::CreateDgnRotationalSweep(rotationDetails);
    }

// Tries to create DgnRotationalSweep from Parasolid sewed spuns surfaces.
ISolidPrimitivePtr Processor::ParasolidSewedSpunsToDgnRotationalSweep(PK_BODY_t bodyTag)
    {
    // Extract faces from body.
    int n_faces;
    PK_FACE_t* faces = nullptr;
    PK_BODY_ask_faces(bodyTag, &n_faces, &faces);
    PK_MEM_Holder fholder(n_faces, faces);

    // Should be more than 1 face.
    if (n_faces < 2)
        return nullptr;

    // Collection of spuns from body.
    bvector<SpunInfo> spuns;

    for (int iFace = 0; iFace < n_faces; iFace++)
        {
        // Extract surface from face.
        PK_SURF_t surf;
        PK_FACE_ask_surf(faces[iFace], &surf);

        // Extract class of surface entity.
        PK_CLASS_t surfaceClass;
        PK_ENTITY_ask_class(surf, &surfaceClass);

        // Extract trimming loops.
        int n_loops;
        PK_LOOP_t *loops;
        PK_FACE_ask_loops(faces[iFace], &n_loops, &loops);
        PK_MEM_Holder lholder(n_loops, loops);

        if (surfaceClass == PK_CLASS_spun)
            {
            // Should be only 2 loops.
            if (n_loops == 2)
                {
                SpunInfo spunInfo;
                spunInfo.spunEnds[0].endLoop = loops[0];
                spunInfo.spunEnds[1].endLoop = loops[1];
                PK_SPUN_ask(surf, &spunInfo.spun);

                // Check if spun is valid.
                if (spunInfo.Validate())
                    spuns.push_back(spunInfo);
                else
                    return nullptr;
                }
            else
                return nullptr;
            }
        else
            return nullptr; // Should be only spuns.
        }

    // Check that each spun has at least one same boundary with closest neighbour
    // and construct profile for rotational sweep.
    SpunInfo prevSpun = spuns[0];
    CurveVectorPtr curveVector(new CurveVector(CurveVector::BoundaryType::BOUNDARY_TYPE_Open));
    for(int i = 1; i < spuns.size(); i++)
        {
        SpunInfo currSpun = spuns[i];
        bool isOk = false;

        for(int iPrev = 0; iPrev < 2 && !isOk; iPrev++)
            for (int iCurr = 0; iCurr < 2 && !isOk; iCurr++)
                if(IsEllipsesSame(prevSpun.spunEnds[iPrev].end, currSpun.spunEnds[iCurr].end))
                    {
                    // Same boundary found.
                    // So add curve to rotational sweep profile and go to next spun checking.

                    // Reverse first curve if necessary for creation of oriented profile.
                    if(i == 1)
                        {
                        // If common vertex is not an end point of first spun curve,
                        // reverse curve.
                        if (prevSpun.spunEnds[iPrev].isStart)
                            prevSpun.profile->ReverseCurvesInPlace();
                        curveVector->Add(prevSpun.profile);
                        }

                    isOk = true;
                    // If common vertex is not a start point of spun curve,
                    // reverse curve.
                    if (!currSpun.spunEnds[iCurr].isStart)
                        currSpun.profile->ReverseCurvesInPlace();
                    curveVector->Add(currSpun.profile);
                    prevSpun = currSpun;
                    }

        // Neighboring spuns has no same boundary.
        // Unable to create rotational sweep.
        if (!isOk)
            return nullptr;
        }

    DgnRotationalSweepDetail rotationDetails(curveVector, P(spuns[0].spun.axis.location), V(spuns[0].spun.axis.axis), PI * 2.0, false);
    return ISolidPrimitive::CreateDgnRotationalSweep(rotationDetails);
    }

// Tries to create DgnRotationalSweep from Parasolid torus body.
// Body should be constructed from 2 planes and number(at least 1) tori surfaces.
ISolidPrimitivePtr Processor::ParasolidTorusSegmentBodyToDgnRotationalSweep(PK_BODY_t bodyTag)
    {
    PK_ERROR_code_t err = 0;

    // this algorithm only for solids
    PK_BODY_type_t btype;
    if (PK_ERROR_no_errors != PK_BODY_ask_type(bodyTag, &btype) || PK_BODY_type_solid_c != btype)
        return nullptr;

    // Extract faces from body.
    int n_faces = 0;
    PK_FACE_t* faces = NULL;
    PK_BODY_ask_faces(bodyTag, &n_faces, &faces);
    PK_MEM_Holder fholder(n_faces, faces);

    // Should be at least 3 faces.
    if (n_faces <= 2)
        return nullptr;

    PK_FACE_t plane1 = PK_ENTITY_null, plane2 = PK_ENTITY_null;
    std::vector<PK_TORUS_t> tori;

    DVec3d norm1 = DVec3d::FromZero();//normal vector for the first planar face

    for (int i = 0; i < n_faces; i++)
        {
        // Extract surface from face.
        PK_SURF_t surf;
        PK_LOGICAL_t sense;
        PK_FACE_ask_oriented_surf(faces[i], &surf, &sense);

        // Extract class of surface entity.
        PK_CLASS_t surfaceClass;
        PK_ENTITY_ask_class(surf, &surfaceClass);

        switch (surfaceClass)
            {
                case PK_CLASS_plane: // 4001 plane
                {
                if (PK_ENTITY_null == plane1)
                    {
                    plane1 = faces[i];
                    PK_PLANE_sf_t planeData;
                    PK_PLANE_ask(surf, &planeData);
                    norm1 = V(planeData.basis_set.axis);
                    if (PK_LOGICAL_false == sense)
                        norm1.Negate();
                    }
                else if (PK_ENTITY_null == plane2)
                    {
                    plane2 = faces[i];
                    }
                else
                    return nullptr;// should be only 2 planes
                }
                break;
                case PK_CLASS_torus: // 4005 torus
                {
                tori.push_back(surf);
                }
                break;
                default:
                    return nullptr;// other types are not allowed
            }
        }

    if (PK_ENTITY_null == plane2)
        return nullptr;// should be exactly 2 planes

    if (tori.empty())
        return nullptr;// should be at least 1 torus

    PK_TORUS_sf_t torusData;
    DVec3d center = DVec3d::FromZero();
    double radius = 0;

    // check all tori have the same center and major radius
    for (int i = 0; i < tori.size(); i++)
        {
        PK_TORUS_ask(tori[i], &torusData);
        // check for self-intersection
        if (torusData.major_radius < 0 || torusData.minor_radius + _EPSILON > torusData.major_radius)
            return nullptr;

        if (0 == i)
            {
            center = V(torusData.basis_set.location);
            radius = torusData.major_radius;
            }
        else
            {
            // consistency with the first one
            if (center.Distance(V(torusData.basis_set.location)) > _EPSILON || fabs(radius - torusData.major_radius) > _EPSILON)
                return nullptr;
            }
        }

    DRange3d range1 = GetExtents(plane1);
    DRange3d range2 = GetExtents(plane2);
    DVec3d center1 = (DVec3d::From(range1.low) + DVec3d::From(range1.high)) * 0.5;
    DVec3d center2 = (DVec3d::From(range2.low) + DVec3d::From(range2.high)) * 0.5;

    DVec3d v1 = DVec3d::FromStartEnd(center, center1); v1.Normalize();
    DVec3d v2 = DVec3d::FromStartEnd(center, center2); v2.Normalize();
    DVec3d up = DVec3d::FromCrossProduct(norm1, v1); up.Normalize();

    // calculate v2 in local coord system: center + (v1, -norm1)
    double v2x = v2.DotProduct(v1);
    double v2y = v2.DotProduct(-1 * norm1);

    double angle = 0;
    if (v2y >= 0)
        angle = acos(v2x);
    else
        angle = 2 * PI - acos(v2x);

    PK_VECTOR_t pos = PK_VEC(center);
    PK_VECTOR1_t dir = PK_VEC(up);
    PK_TRANSF_t tr;
    err = PK_TRANSF_create_rotation(pos, dir, angle, &tr);

    // check if face "plane1" rotated by "angle" around "center" point is coincident with face "plane2"
    PK_FACE_is_coincident_o_t coiopts;
    PK_FACE_is_coincident_o_m(coiopts);
    coiopts.transf1 = tr;
    coiopts.transf2 = PK_ENTITY_null;
    PK_FACE_coi_t coit;
    PK_VECTOR_t outv;
    err = PK_FACE_is_coincident(plane1, plane2, 1e-4, &coiopts, &coit, &outv);

    if (PK_ERROR_no_errors == err && PK_FACE_coi_yes_reversed_c == coit)
        {
        CurveVectorPtr curveVector = ParasolidFaceToCurveVector(plane1);
        if (!curveVector.IsNull())
            {
            DgnRotationalSweepDetail rotationDetails(curveVector, center, up, angle, true);
            return ISolidPrimitive::CreateDgnRotationalSweep(rotationDetails);
            }
        }

    return nullptr;
    }

// Calculates two perpendiculars for "axis" vector
void Processor::GetPerpendiculars(PK_VECTOR1_t axis, PK_VECTOR1_t * norm1, PK_VECTOR1_t * norm2)
    {
    PK_VECTOR1_t zero = { 0, 0, 0 };
    PK_VECTOR_perpendicular(axis, zero, norm1);
    *norm2 = PK_VEC(DVec3d::FromNormalizedCrossProduct(V(axis), V(*norm1)));
    }

// Tries to create DgnRotationalSweep from Parasolid body consists of sequence of rotational parts.
ISolidPrimitivePtr Processor::ParasolidRevolutionToDgnRotationalSweep(PK_BODY_t bodyTag)
    {
    PK_ERROR_code_t err = PK_ERROR_no_errors;

    // this algorithm only for solids
    PK_BODY_type_t btype;
    if (PK_ERROR_no_errors != PK_BODY_ask_type(bodyTag, &btype) || PK_BODY_type_solid_c != btype)
        return nullptr;

    int n_faces = 0;
    PK_FACE_t* faces = NULL;
    PK_BODY_ask_faces(bodyTag, &n_faces, &faces);
    PK_MEM_Holder fholder(n_faces, faces);

    // find the first rotation axis
    PK_VECTOR1_t axis = { 0, 0, 0 }, location = { 0, 0, 0 };
    bool found = false;
    for (int iFace = 0; iFace < n_faces && !found; iFace++)
        {
        PK_SURF_t surf;
        PK_FACE_ask_surf(faces[iFace], &surf);
        PK_CLASS_t surfaceClass;
        PK_ENTITY_ask_class(surf, &surfaceClass);

        switch (surfaceClass)
            {
                case PK_CLASS_cyl: // 4002 cylinder
                {
                PK_CYL_sf_t cyl;
                PK_CYL_ask(surf, &cyl);
                axis = cyl.basis_set.axis;
                location = cyl.basis_set.location;
                found = true;
                }
                break;
                case PK_CLASS_cone: // 4003 cone
                {
                PK_CONE_sf_t cone;
                PK_CONE_ask(surf, &cone);
                axis = cone.basis_set.axis;
                location = cone.basis_set.location;
                found = true;
                }
                break;
                case PK_CLASS_spun: // 4010 swung surface
                {
                PK_SPUN_sf_t spun;
                PK_SPUN_ask(surf, &spun);
                axis = spun.axis.axis;
                location = spun.axis.location;
                found = true;
                }
                break;
                case PK_CLASS_torus: // 4005 torus
                {
                PK_TORUS_sf_t torus;
                PK_TORUS_ask(surf, &torus);
                axis = torus.basis_set.axis;
                location = torus.basis_set.location;
                found = true;
                }
                break;
            }
        }

    if (!found)
        return nullptr;

    // check all items have the same rotation axis
    for (int iFace = 0; iFace < n_faces; iFace++)
        {
        PK_SURF_t surf;
        PK_FACE_ask_surf(faces[iFace], &surf);
        PK_CLASS_t surfaceClass;
        PK_ENTITY_ask_class(surf, &surfaceClass);

        switch (surfaceClass)
            {
                case PK_CLASS_plane: // 4001 plane
                {
                PK_PLANE_sf_t plane;
                PK_PLANE_ask(surf, &plane);
                if (!IsCollinear(plane.basis_set.axis, axis)) // plane's normal should be collinear with rotation axis
                    return nullptr;
                }
                break;
                case PK_CLASS_cyl: // 4002 cylinder
                {
                PK_CYL_sf_t cyl;
                PK_CYL_ask(surf, &cyl);
                if (!IsCollinear(cyl.basis_set.axis, axis)) // cylinder's axis should be collinear with rotation axis
                    return nullptr;
                PK_VECTOR1_t vec = PK_VEC_Diff(cyl.basis_set.location, location);
                if (!PK_VEC_Zero(vec) && !IsCollinear(vec, axis)) // cylinder's location should lie on rotation axis
                    return nullptr;
                }
                break;
                case PK_CLASS_cone: // 4003 cone
                {
                PK_CONE_sf_t cone;
                PK_CONE_ask(surf, &cone);
                if (!IsCollinear(cone.basis_set.axis, axis)) // cone's axis should be collinear with rotation axis
                    return nullptr;
                PK_VECTOR1_t vec = PK_VEC_Diff(cone.basis_set.location, location);
                if (!PK_VEC_Zero(vec) && !IsCollinear(vec, axis)) // cone's location should lie on rotation axis
                    return nullptr;
                }
                break;
                case PK_CLASS_spun: // 4010 swung surface
                {
                PK_SPUN_sf_t spun;
                PK_SPUN_ask(surf, &spun);
                if (!IsCollinear(spun.axis.axis, axis)) // spun's axis should be collinear with rotation axis
                    return nullptr;
                PK_VECTOR1_t vec = PK_VEC_Diff(spun.axis.location, location);
                if (!PK_VEC_Zero(vec) && !IsCollinear(vec, axis)) // spun's location should lie on rotation axis
                    return nullptr;
                }
                break;
                case PK_CLASS_torus: // 4005 torus
                {
                PK_TORUS_sf_t torus;
                PK_TORUS_ask(surf, &torus);
                if (!IsCollinear(torus.basis_set.axis, axis)) // torus' axis should be collinear with rotation axis
                    return nullptr;
                PK_VECTOR1_t vec = PK_VEC_Diff(torus.basis_set.location, location);
                if (!PK_VEC_Zero(vec) && !IsCollinear(vec, axis)) // torus' location should lie on rotation axis
                    return nullptr;
                }
                break;
                case PK_CLASS_sphere: // 4004 sphere
                {
                PK_SPHERE_sf_t sphere;
                PK_SPHERE_ask(surf, &sphere);
                PK_VECTOR1_t vec = PK_VEC_Diff(sphere.basis_set.location, location);
                if (!PK_VEC_Zero(vec) && !IsCollinear(vec, axis)) // sphere's center should lie on rotation axis
                    return nullptr;
                }
                break;
                default:
                    return nullptr; // other types are not allowed
            }
        }

    // create revolution element:
    // 1. find two perpendiculars for "axis" vector.
    // 2. create two planes with these normals.
    // 3. perform sectioning initial body by these two planes: we will have 1/4 piece of initial body.
    // 4. find face with normal collinear with the first perpendicular
    //    and use it as profile for DgnRotationalSweep.
    PK_VECTOR1_t norm = { 0, 0, 0 }, norm2 = { 0, 0, 0 };
    GetPerpendiculars(axis, &norm, &norm2);
    PK_PLANE_t sectionPlane = PK_ENTITY_null;
    PK_PLANE_sf_t plane;
    plane.basis_set.axis = norm;
    plane.basis_set.ref_direction = axis;
    plane.basis_set.location = location;
    PK_PLANE_create(&plane, &sectionPlane);

    PK_BODY_section_o_t opts;
    PK_BODY_section_o_m(opts);
    PK_section_r_t rt {};
    err = PK_BODY_section_with_surf(bodyTag, sectionPlane, &opts, &rt);
    if (PK_ERROR_no_errors != err)
        {
        PK_section_r_f(&rt);
        return nullptr;
        }

    PK_FACE_t profileFace = PK_ENTITY_null;

    if (1 == rt.back_bodies.length && 1 == rt.front_bodies.length)
        {
        plane.basis_set.axis = norm2;
        plane.basis_set.ref_direction = axis;
        plane.basis_set.location = location;
        PK_PLANE_create(&plane, &sectionPlane);

        PK_section_r_t rt2 {};
        err = PK_BODY_section_with_surf(rt.front_bodies.array[0], sectionPlane, &opts, &rt2);
        if (PK_ERROR_no_errors != err)
            {
            PK_section_r_f(&rt);
            PK_section_r_f(&rt2);
            return nullptr;
            }

        if (1 == rt2.back_bodies.length && 1 == rt2.front_bodies.length)
            {
            PK_BODY_t b = rt2.front_bodies.array[0];

            int nf2 = 0;
            PK_FACE_t* faces2 = NULL;
            PK_BODY_ask_faces(b, &nf2, &faces2);
            PK_MEM_Holder fholder2(nf2, faces2);


            for (int iFace = 0; iFace < nf2; iFace++)
                {
                PK_SURF_t surf;
                PK_FACE_ask_surf(faces2[iFace], &surf);
                PK_CLASS_t surfaceClass;
                PK_ENTITY_ask_class(surf, &surfaceClass);

                switch (surfaceClass)
                    {
                        case PK_CLASS_plane:
                        {
                        PK_PLANE_sf_t surfPlane;
                        PK_PLANE_ask(surf, &surfPlane);
                        if (IsCollinear(surfPlane.basis_set.axis, norm))
                            {
                            if (PK_ENTITY_null == profileFace)
                                profileFace = faces2[iFace];
                            else
                                {
                                PK_section_r_f(&rt);
                                PK_section_r_f(&rt2);
                                return nullptr;// there should be only 1 face for profile
                                }
                            }
                        }
                        break;
                    }
                }
            }

        PK_section_r_f(&rt2);
        }

    PK_section_r_f(&rt);

    if (PK_ENTITY_null != profileFace)
        {
        CurveVectorPtr profile = ParasolidFaceToCurveVector(profileFace);
        if (profile.IsNull())
            return nullptr;
        DVec3d origin = V(location);
        DVec3d zVec = V(axis);
        DgnRotationalSweepDetail rotationDetails(profile, origin, zVec, PI * 2, true);
        return ISolidPrimitive::CreateDgnRotationalSweep(rotationDetails);
        }
    return nullptr;
    }

// Tries to create DgnRotationalSweep from Parasolid complex body.
// Body can contain curcular planes (full circle or ring),
// cylinders, cones, spuns.
ISolidPrimitivePtr Processor::ParasolidComplexBodyToDgnRotationalSweep(PK_BODY_t bodyTag)
    {
    // Extract faces from body.
    int n_faces;
    PK_FACE_t* faces = nullptr;
    PK_BODY_ask_faces(bodyTag, &n_faces, &faces);
    PK_MEM_Holder fholder(n_faces, faces);

    // Should be more than 1 face.
    if (n_faces < 2)
        return nullptr;

    // In much cases sewing  is done with rough tolerance.
    // It's very critical for detection of circular trimming loop on plane.
    // So try to find first non-planar surface for detection of correct origin.
    // Also collect necessary data for further analysis.
    // NOTE: all trimming loops should be circles for creation of DgnRotationalSweep.
    bvector<RevolutionPartInfo> parts;
    int nonPlanarIndex = -1;
    DVec3d zVec = DVec3d::FromZero();
    DVec3d xVec = DVec3d::FromZero();
    bvector<DPoint3d> first, last;
    for (int iFace = 0; iFace < n_faces; iFace++)
        {
        // Extract surface from face.
        PK_SURF_t surf;
        PK_FACE_ask_surf(faces[iFace], &surf);

        // Extract class of surface entity.
        PK_CLASS_t surfaceClass;
        PK_ENTITY_ask_class(surf, &surfaceClass);

        // Extract trimming loops.
        int n_loops;
        PK_LOOP_t *loops;
        PK_FACE_ask_loops(faces[iFace], &n_loops, &loops);
        PK_MEM_Holder lholder(n_loops, loops);

        // Get vertices of first and last faces for normal direction calculation.
        if (iFace == 0 || iFace == n_faces - 1)
            for (int i = 0; i < n_loops; i++)
                {
                DPoint3d p;
                if (TryGetVertexFromLoopByIndex(loops[i], 0, &p))
                    {
                    if (iFace == 0)
                        first.push_back(p);
                    else
                        last.push_back(p);
                    }
                }

        if (n_loops == 0 || loops == nullptr)
            return nullptr;

        DVec3d axis, xdir;
        RevolutionPartInfo info;
        info.nLoops = n_loops;
        info.endLoops[0] = loops[0];
        if(n_loops > 1)
            info.endLoops[1] = loops[1];
        info.isPlane = surfaceClass == PK_CLASS_plane;
        switch (surfaceClass)
            {
            case PK_CLASS_plane: // 4001 plane
                if (n_loops <= 2)
                    {
                    PK_PLANE_sf_t plane;
                    PK_PLANE_ask(surf, &plane);
                    axis = V(plane.basis_set.axis);
                    xdir = V(plane.basis_set.ref_direction);
                    }
                else
                    return nullptr; // Should be 1 or 2 loops.
                break;
            case PK_CLASS_cyl: // 4002 cylinder
                if (n_loops == 2)
                    {
                    PK_CYL_sf_t cyl;
                    PK_CYL_ask(surf, &cyl);
                    axis = V(cyl.basis_set.axis);
                    xdir = V(cyl.basis_set.ref_direction);
                    }
                else
                    return nullptr; // Should be 2 loops.
                break;
            case PK_CLASS_cone: // 4003 cone
                if (n_loops == 2)
                    {
                    PK_CONE_sf_t cone;
                    PK_CONE_ask(surf, &cone);
                    axis = V(cone.basis_set.axis);
                    xdir = V(cone.basis_set.ref_direction);
                    }
                else
                    return nullptr; // Should be 2 loops.
                break;
            case PK_CLASS_spun: // 4010 swung surface
                if (n_loops == 2)
                    {
                    SpunInfo spunInfo;
                    spunInfo.spunEnds[0].endLoop = loops[0];
                    spunInfo.spunEnds[1].endLoop = loops[1];
                    PK_SPUN_ask(surf, &spunInfo.spun);

                    // Check if spun is valid for rotational sweep.
                    if (spunInfo.Validate())
                        {
                        axis = V(spunInfo.spun.axis.axis);
                        xdir = spunInfo.spunEnds[0].end.vector0;
                        info.profile = spunInfo.profile;
                        info.ends[0] = spunInfo.spunEnds[0].end;
                        info.ends[1] = spunInfo.spunEnds[1].end;
                        }
                    else
                        return nullptr; // Not valid spun.
                    }
                else
                    return nullptr; // Should be 2 loops.
                break;
            default:
                return nullptr; // Not fit surface type.
            }

        if (iFace == 0) // Set rotational sweep axes.
            {
            xVec = xdir;
            zVec = axis;
            }
        else if (!zVec.IsParallelTo(axis)) // Check that surface axis is parallel to rotational sweep axis.
            return nullptr;

        // Save index of first non-planar surface.
        if (nonPlanarIndex < 0 && !info.isPlane)
            nonPlanarIndex = iFace;

        // Validate trimming loops.
        // All of them should be circles.
        // For planes will be additional circularity checking due to rough sewing tolerances.
        // For spuns ends are already validated.
        if (info.profile.IsNull())
            for (int i = 0; i < info.nLoops; i++)
                if (SUCCESS != TryGetEllipseFromLoop(&info.ends[i], info.endLoops[i]))
                    {
                    if (info.isPlane)
                        {
                        info.isValidPlaneCircles[i] = false;
                        // Try to get circle with rough epsilon for plane.
                        if (TryGetEllipseFromLoop(&info.ends[i], info.endLoops[i], 0.01) == SUCCESS && info.ends[i].IsCircular() && info.ends[i].IsFullEllipse())
                            {
                            info.isValidPlaneCircles[i] = true;
                            info.tolerance = 0.01;
                            }
                        }
                    else
                        return nullptr;
                    }
                else if (!info.ends[i].IsCircular() && !info.ends[i].IsFullEllipse())
                    {
                    if (info.isPlane)
                        info.isValidPlaneCircles[i] = false;
                    else
                        return nullptr;
                    }

        parts.push_back(info);
        }

    xVec.Normalize();
    zVec.Normalize();

    // Try to detect normal direction by vertices.
    if (nonPlanarIndex >= 0)
        {
        DVec3d dir = DVec3d::FromZero();
        double dot = 0;
        bool found = false;
        for (int i = 0; i < first.size() && !found; i++)
            for (int j = 0; j < last.size() && !found; j++)
                {
                dir = DVec3d::From(last[j] - first[i]);
                dot = dir.DotProduct(zVec);
                if (dir.Magnitude() > 0 && fabs(dot) > _EPSILON)
                    {
                    if (dot < 0)
                        zVec.Negate();
                    found = true;
                    }
                }
        }

    DVec3d yVec;
    yVec.CrossProduct(zVec, xVec);
    // Init rotational sweep origin from center of circular loop of first non-planar surface (if exists).
    // Otherwise try to find center in plane circles.
    DPoint3d origin = DPoint3d::FromZero();
    if (nonPlanarIndex >= 0)
        {
        DVec3d dir = DVec3d::From(parts[nonPlanarIndex].ends[1].center - parts[nonPlanarIndex].ends[0].center);
        origin = parts[nonPlanarIndex].ends[zVec.DotProduct(dir) > 0 ? 0 : 1].center;
        }
    else
        for (int i = 0; i < parts.size(); i++)
            if (parts[i].isValidPlaneCircles[0])
                {
                origin = parts[i].ends[0].center;
                break;
                }
            else if (parts[i].isValidPlaneCircles[1])
                {
                origin = parts[i].ends[1].center;
                break;
                }

    // Calculate local-to-world and world-to-local transformations.
    Transform ltw, wtl;
    ltw.InitFromOriginAndVectors(origin, xVec, yVec, zVec);
    wtl.InverseOf(ltw);

    // Profile for rotational sweep.
    CurveVectorPtr curveVector(new CurveVector(CurveVector::BoundaryType::BOUNDARY_TYPE_Open));

    // Construct profile in X0Z plane and then transform it to world coordinates.
    // Iterate through all parts.
    RevolutionPartInfo prevPart = parts[0];
    DPoint3d prevPt = DPoint3d::FromZero();
    int iPrev = -1, iCurr = -1;
    for(int i = 1; i < parts.size() + 1; i++)
        {
        RevolutionPartInfo currPart;
        bool found = false;

        if (i < parts.size())
            {
            currPart = parts[i];
            iPrev = -1, iCurr = -1;

            // Check that neighboring parts have same boundary.
            // Firstly check loops by tags.
            for (iPrev = 0; iPrev < prevPart.nLoops && !found; iPrev++)
                for (iCurr = 0; iCurr < currPart.nLoops && !found; iCurr++)
                    if (IsLoopsSame(prevPart.endLoops[iPrev], currPart.endLoops[iCurr]))
                        found = true;

            // Loops different by tags. But circles geometry can be the same.
            // So check circles.
            if (!found)
                for (iPrev = 0; iPrev < prevPart.nLoops && !found; iPrev++)
                    for (iCurr = 0; iCurr < currPart.nLoops && !found; iCurr++)
                        {
                        // Check that circle for planar part was calculated.
                        if (prevPart.isPlane && !prevPart.isValidPlaneCircles[iPrev])
                            continue;

                        // Check that circle for planar part was calculated.
                        if (currPart.isPlane && !currPart.isValidPlaneCircles[iCurr])
                            continue;

                        if (IsEllipsesSame(prevPart.ends[iPrev], currPart.ends[iCurr], std::max(prevPart.tolerance, currPart.tolerance)))
                            found = true;
                        }
            }

        // If common boundary was found, construct profile in X0Z plane.
        if (found || i == parts.size())
            {
            if(found)
                {
                // Index of end circle from prevoius part.
                iPrev--;
                // Index of start circle of current part.
                iCurr--;
                }

            // For iPrev and iCurr allowed only 0 or 1 values.
            if (iPrev < 0 || iPrev > 1 || iCurr < 0 || iCurr > 1)
                return nullptr;

            if (i == parts.size())
                currPart.isPlane = true;

            if (prevPart.profile.IsValid()) // Spun profile curve.
                {
                // Transform curve to local coordinates.
                prevPart.profile->TransformInPlace(wtl);
                DPoint3d ptProfile[2];
                prevPart.profile->GetStartEnd(ptProfile[0], ptProfile[1]);
                // Get start curve point (by index of start circle of previous part).
                DVec3d target = DVec3d::From(ptProfile[1 - iPrev]);
                // Calculate rotation angle for placing curve to X0Z plane.
                double angle = target.AngleToXY(DVec3d::From(1.0, 0, target.z));
                // Create additional transform.
                Transform tr = Transform::FromIdentity();
                tr.SetMatrix(RotMatrix::FromAxisAndRotationAngle(2, angle));
                // Place curve to X0Z plane and add to contour.
                prevPart.profile->TransformInPlace(tr);
                prevPart.profile->GetStartEnd(ptProfile[0], ptProfile[1]);
                // If start point of profile section is an end point of spun profile,
                // reverse curve for creation of correct rotational sweep profile.
                if (1 - iPrev != 0)
                    prevPart.profile->ReverseCurvesInPlace();
                curveVector->Add(prevPart.profile);
                // Save end point.
                prevPt = ptProfile[iPrev];
                iPrev = 1 - iCurr;
                }
            else // Construct line for other part types.
                {
                DPoint3d end;

                // Profile start point not defined yet. So calculate.
                if (i == 1)
                    {
                    // Index of start circle from previous part.
                    int startIndex = 1 - iPrev;

                    // If plane has only 1 loop, it's a full filled circle, not ring.
                    // So x-coordinate will be 0.
                    if (prevPart.isPlane && prevPart.nLoops == 1 && startIndex == 1)
                        prevPt = DPoint3d::FromZero();
                    else if (prevPart.isValidPlaneCircles[startIndex]) // Calculate x-coordinate by circle radius and z-coordinate by start circle distance from origin.
                        {
                        if (!prevPart.isPlane)
                            {
                            DVec3d diff = DVec3d::From(prevPart.ends[startIndex].center - origin);
                            prevPt = DPoint3d::FromXYZ(prevPart.ends[startIndex].vector0.Magnitude(), 0, diff.DotProduct(zVec));
                            }
                        else
                            prevPt = DPoint3d::FromXYZ(prevPart.ends[startIndex].vector0.Magnitude(), 0, 0);
                        }
                    else // Start circle of plane was not calculated. Can't calculate profile start point.
                        return nullptr;
                    }

                // Calculate z-size of previous part:
                //    0 for plane
                //    distance between ellipses for other cases.
                double zShift = 0;
                if (!prevPart.isPlane)
                    {
                    DVec3d diff = DVec3d::From(prevPart.ends[iPrev].center - prevPart.ends[1 - iPrev].center);
                    zShift = diff.DotProduct(zVec);
                    }

                // End point of line lies on found common circle.
                // X-coordinate defined by circle radius.
                // If previous part is plane, get circle from current part (if part non-planar),
                // because circles from planar parts quite bad due to rough tolerance on sewing.
                if (prevPart.isPlane && !currPart.isPlane)
                    end = DPoint3d::FromXYZ(currPart.ends[iCurr].vector0.Magnitude(), 0, prevPt.z + zShift);
                else
                    {
                    // If plane has only 1 loop, it's a full filled circle, not ring.
                    // So x-coordinate will be 0.
                    if (prevPart.isPlane && prevPart.nLoops == 1 && iPrev == 1)
                        end = DPoint3d::FromXYZ(0, 0, prevPt.z + zShift);
                    else // Calculate x-coordinate by circle radius.
                        end = DPoint3d::FromXYZ(prevPart.ends[iPrev].vector0.Magnitude(), 0, prevPt.z + zShift);
                    }

                // Add line to contour.
                curveVector->Add(ICurvePrimitive::CreateLine(DSegment3d::From(prevPt, end)));
                prevPt = end;
                iPrev = 1 - iCurr;
                }
            }
        else
            return nullptr; // Common boundary not found. Can't create rotational sweep.

        prevPart = currPart;
        }

    curveVector->TransformInPlace(ltw);
    DgnRotationalSweepDetail rotationDetails(curveVector, origin, zVec, PI * 2.0, false);
    return ISolidPrimitive::CreateDgnRotationalSweep(rotationDetails);
    }

CurveVectorPtr Processor::CreateTriangle(PK_VERTEX_t * vertices)
    {
    CurveVectorPtr result = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);

    bvector<DPoint3d> pnts;
    pnts.push_back(VertexGetCoordinates(vertices[0]));
    pnts.push_back(VertexGetCoordinates(vertices[1]));
    pnts.push_back(VertexGetCoordinates(vertices[2]));
    pnts.push_back(VertexGetCoordinates(vertices[0]));

    result->push_back(ICurvePrimitive::CreateLineString(pnts));

    return result;
    }

// Tries to convert Parasolid quad pyramid body to DgnRuledSweep.
// Parasolid body should be constructed exactly from 5 planes.
// It seems to be possible to support common case of pyramid (not only quad):
// if we will use base face - as a first section and top vertex - as a second,
// but in this case the second section will be degenerated and may cause problems (e.g. with visualizing).
// Can be redesigned in future.
ISolidPrimitivePtr Processor::ParasolidQuadPyramidToDgnRuledSweep(PK_BODY_t bodyTag)
    {
    //this algorithm only for solids
    PK_BODY_type_t btype;
    if (PK_ERROR_no_errors != PK_BODY_ask_type(bodyTag, &btype) || PK_BODY_type_solid_c != btype)
        return nullptr;

    int n_faces;
    PK_FACE_t * faces = NULL;
    PK_BODY_ask_faces(bodyTag, &n_faces, &faces);
    PK_MEM_Holder fholder(n_faces, faces);

    // Should be exactly 5 faces.
    if (5 != n_faces)
        return nullptr;

    //try to find base face (face with 4 edges)
    PK_FACE_t baseFace = PK_ENTITY_null;

    for (int i = 0; i < n_faces; i++)
        {
        int n_edges = 0;
        PK_FACE_ask_edges(faces[i], &n_edges, NULL);

        if (4 == n_edges)
            {
            if (PK_ENTITY_null == baseFace)
                baseFace = faces[i];
            else
                return nullptr;//should be only one base face with 4 edges
            }
        else if (3 != n_edges)
            return nullptr;//the only supported configuration: one base with 4 edges & others with 3 edges
        }

    //try to find top vertex (vertex which belongs to 4 faces)
    int n_verts = 0;
    PK_VERTEX_t * verts = NULL;
    PK_BODY_ask_vertices(bodyTag, &n_verts, &verts);
    PK_MEM_Holder v1holder(n_verts, verts);

    PK_VERTEX_t topVertex = PK_ENTITY_null;

    for (int i = 0; i < n_verts; i++)
        {
        int n_faces_this_vertex = 0;
        PK_VERTEX_ask_faces(verts[i], &n_faces_this_vertex, NULL);
        if (4 == n_faces_this_vertex)
            {
            if (PK_ENTITY_null == topVertex)
                topVertex = verts[i];
            else
                return nullptr;//should be only one top vertex
            }
        }


    if (PK_ENTITY_null != baseFace && PK_ENTITY_null != topVertex)
        {
        int n_v = 0;
        PK_VERTEX_t * vs = NULL;
        PK_FACE_ask_vertices(baseFace, &n_v, &vs);
        PK_MEM_Holder v2holder(n_v, vs);

        PK_VERTEX_t triangle[3];

        triangle[0] = topVertex;
        triangle[1] = vs[0];
        triangle[2] = vs[1];
        CurveVectorPtr section1 = CreateTriangle(triangle);

        triangle[0] = topVertex;
        triangle[1] = vs[3];
        triangle[2] = vs[2];
        CurveVectorPtr section2 = CreateTriangle(triangle);

        DgnRuledSweepDetail ruledDetail = DgnRuledSweepDetail(section1, section2, true);
        return ISolidPrimitive::CreateDgnRuledSweep(ruledDetail);
        }

    return nullptr;
    }

// Tries to convert Parasolid body to DgnBox or DgnExtrusion.
// Parasolid body should be constructed only from planes.
ISolidPrimitivePtr Processor::ParasolidBodyToDgnBox(PK_BODY_t bodyTag)
    {
    // Extract faces from body.
    int n_faces;
    PK_FACE_t* faces = nullptr;
    PK_BODY_ask_faces(bodyTag, &n_faces, &faces);
    PK_MEM_Holder fholder(n_faces, faces);

    // Should be at least 4 faces.
    if (n_faces < 4)
        return nullptr;

    bool boxResult = true;
    bool extrudeResult = true;
    std::list<int> faceCount;

    // check a box candidate
    std::list<ExBoxInfo> exInfo;//4 sides, opposite parallel
    std::list<PlaneLoopInfo> extrudeInfo; //other
    std::set<VertexElem> vertexSet;
    std::set<VectorElem> vectorSet;

    for (int iFace = 0; iFace < n_faces; iFace++)
        {
        // Extract surface from face.
        PK_SURF_t surf;
        PK_FACE_ask_surf(faces[iFace], &surf);

        // Extract trimming loops.
        int n_loops;
        PK_LOOP_t *loops;
        PK_FACE_ask_loops(faces[iFace], &n_loops, &loops);
        PK_MEM_Holder lholder(n_loops, loops);

        PK_CLASS_t surfaceClass;
        PK_ENTITY_ask_class(surf, &surfaceClass);
        switch (surfaceClass)
            {
                case PK_CLASS_plane: // 4001 plane
                {
                PlaneLoopInfo loopInfo {faces[iFace], n_loops,0, {0,0,0}};
                ExBoxInfo boxInfo;
                if (BentleyStatus::SUCCESS != ProcessParasolidPlane(surf, loops, bodyTag, loopInfo, boxInfo, vertexSet, vectorSet))
                    {
                    boxResult = false;
                    extrudeInfo.push_back(loopInfo);
                    }
                else
                    {
                    boxInfo.face = faces[iFace];
                    exInfo.push_back(boxInfo);
                    }
                break;
                }
                default: // Only planes allowed.
                    return nullptr;
            }
        }

    if (vertexSet.size() == 8 && (n_faces == 4 || n_faces == 6))
        {
        if (boxResult && n_faces == 6 && vectorSet.size() == 3)
            return CreateBox(vertexSet);
        else if (vectorSet.size() != 6)
            {
            auto boxEx = CreateBoxEx(vectorSet, exInfo, n_faces, faces);
            if (boxEx != nullptr)
                return boxEx;
            }
        }

    if (extrudeResult)
        {
        extrudeResult = extrudeInfo.size() == 2 && extrudeInfo.front().nLoops == extrudeInfo.back().nLoops &&
            extrudeInfo.front().nEdges == extrudeInfo.back().nEdges && n_faces == extrudeInfo.front().nEdges + 2;
        //TODO - uncapped extrude
        if (extrudeResult)
            {
            PK_LOGICAL_t bresult;
            auto axis = extrudeInfo.front().vec;
            PK_VECTOR_is_parallel(axis, extrudeInfo.back().vec, &bresult);

            extrudeResult = bresult == PK_LOGICAL_true;
            if (extrudeResult)
                return CreateExtrude(extrudeInfo.front().face, extrudeInfo.back().face);
            }
        }

    return nullptr;
    }

// Tries to create CurvePrimitive from Parasolid 3d curve.
ICurvePrimitivePtr Processor::Create3dCurve(PK_CURVE_t curve)
    {
    // Extract interval.
    PK_INTERVAL_t interval = {0, 0};
    PK_CURVE_ask_interval(curve, &interval);
    return Create3dCurveByInterval(curve, interval);
    }

// Method detects if curve1 is coincident with curve2.
// The only potential problem: we have no control on tolerances.
// It can be redesigned with checking enough set of curves points, but first we need
// examples where it will be more effective.
// Moreover, since this curves are usually part of complex loops - we will need to take into account
// relationships of neighbors because loop can become self-intersecting if we will use big tolerances.
// So let's use Parasolid's session tolerance until there will be significant reason to do rough comparison.
bool Processor::IsCoincident(PK_CURVE_t curve1, PK_INTERVAL_t interval1, PK_CURVE_t curve2, PK_INTERVAL_t interval2)
    {
    PK_ERROR_code_t err = 0;

    //check for start/end points
    double tolerance = 1e-8;//we use session tolerance to be the same with PK_CURVE_intersect_curve
    PK_SESSION_ask_precision(&tolerance);
    PK_VECTOR_t p1, p2;

    err = PK_CURVE_eval(curve1, interval1.value[0], 0, &p1);
    err = PK_CURVE_eval(curve2, interval2.value[0], 0, &p2);
    double dist = V(p1).Distance(V(p2));
    if (dist > tolerance)
        return false;

    err = PK_CURVE_eval(curve1, interval1.value[1], 0, &p1);
    err = PK_CURVE_eval(curve2, interval2.value[1], 0, &p2);
    dist = V(p1).Distance(V(p2));
    if (dist > tolerance)
        return false;

    PK_CURVE_intersect_curve_o_t iopts;
    PK_CURVE_intersect_curve_o_m(iopts);

    int n_vectors;
    PK_VECTOR_t                 *vectors;
    double                      *ts_1;
    double                      *ts_2;
    PK_intersect_vector_t       *types;

    err = PK_CURVE_intersect_curve(curve1, interval1, curve2, interval2, &iopts, &n_vectors, &vectors, &ts_1, &ts_2, &types);
    PK_MEM_Holder hVecs(n_vectors, vectors);
    PK_MEM_Holder hTs1(n_vectors, ts_1);
    PK_MEM_Holder hTs2(n_vectors, ts_2);
    PK_MEM_Holder hTypes(n_vectors, types);

    // if there are 2 intersection points and one of them is start of coincedence region,
    // and the second is end of coincidence region - it means curves have the same geometry
    if (PK_ERROR_no_errors == err && 2 == n_vectors)
        {
        if ((types[0] == PK_intersect_vector_start_c && types[1] == PK_intersect_vector_end_c) ||
            (types[0] == PK_intersect_vector_end_c && types[1] == PK_intersect_vector_start_c))
            {
            return true;
            }
        }

    return false;
    }

// Analyzes b-spline curve and prepare set of points for possible arc representation.
// It detects just a possible variant which should be compared further with initial b-spline.
void Processor::DetectPossibleArcType(PK_BCURVE_t inputBCurve, ArcRecoDataInfo *data)
    {
    PK_ERROR_code_t err = 0;
    data->type = ArcType::Unknown;

    PK_BCURVE_sf_t bcurve;
    err = PK_BCURVE_ask(inputBCurve, &bcurve);
    if (PK_ERROR_no_errors != err)
        return;

    double startKnot = bcurve.knot[0];
    double endKnot = bcurve.knot[bcurve.n_knots - 1];

    err = PK_CURVE_eval(inputBCurve, startKnot, 0, &data->startPoint);
    err = PK_CURVE_eval(inputBCurve, (startKnot + endKnot) * 0.25, 0, &data->quarterPoint);
    err = PK_CURVE_eval(inputBCurve, (startKnot + endKnot) * 0.5, 0, &data->halfPoint);
    err = PK_CURVE_eval(inputBCurve, endKnot, 0, &data->endPoint);

    if (PK_LOGICAL_true == bcurve.is_closed)
        {
        data->type = ArcType::Full;

        data->arcCenter.coord[0] = (data->startPoint.coord[0] + data->halfPoint.coord[0]) * 0.5;
        data->arcCenter.coord[1] = (data->startPoint.coord[1] + data->halfPoint.coord[1]) * 0.5;
        data->arcCenter.coord[2] = (data->startPoint.coord[2] + data->halfPoint.coord[2]) * 0.5;
        }
    else
        {
        PK_VECTOR1_t tangent;
        PK_VECTOR1_t principal_normal1, principal_normal2;
        PK_VECTOR1_t binormal;
        double curvature;

        err = PK_CURVE_eval_curvature(inputBCurve, startKnot, &tangent, &principal_normal1, &binormal, &curvature);
        err = PK_CURVE_eval_curvature(inputBCurve, endKnot, &tangent, &principal_normal2, &binormal, &curvature);

        PK_LOGICAL_t parallel = PK_LOGICAL_false;
        err = PK_VECTOR_is_parallel(principal_normal1, principal_normal2, &parallel);

        if (PK_LOGICAL_true == parallel)
            {
            // half of ellipse/circle

            data->arcCenter.coord[0] = (data->startPoint.coord[0] + data->endPoint.coord[0]) * 0.5;
            data->arcCenter.coord[1] = (data->startPoint.coord[1] + data->endPoint.coord[1]) * 0.5;
            data->arcCenter.coord[2] = (data->startPoint.coord[2] + data->endPoint.coord[2]) * 0.5;

            data->type = ArcType::Half;
            }
        else
            {
            // 1/4 of ellipse/circle

            PK_LINE_t line1, line2;
            PK_LINE_sf_t line1data;
            line1data.basis_set.location = data->startPoint;
            line1data.basis_set.axis = principal_normal1;
            err = PK_LINE_create(&line1data, &line1);

            PK_LINE_sf_t line2data;
            line2data.basis_set.location = data->endPoint;
            line2data.basis_set.axis = principal_normal2;
            err = PK_LINE_create(&line2data, &line2);

            PK_INTERVAL_t interval = { -10000, 10000 };

            PK_CURVE_intersect_curve_o_t iopts;
            PK_CURVE_intersect_curve_o_m(iopts);

            int n_vectors;
            PK_VECTOR_t                 *vectors;
            double                      *ts_1;
            double                      *ts_2;
            PK_intersect_vector_t       *types;

            err = PK_CURVE_intersect_curve(line1, interval, line2, interval, &iopts, &n_vectors, &vectors, &ts_1, &ts_2, &types);
            PK_MEM_Holder hVecs(n_vectors, vectors);
            PK_MEM_Holder hTs1(n_vectors, ts_1);
            PK_MEM_Holder hTs2(n_vectors, ts_2);
            PK_MEM_Holder hTypes(n_vectors, types);

            if (PK_ERROR_no_errors == err && 1 == n_vectors)
                {
                data->arcCenter = vectors[0];
                data->type = ArcType::Quarter;
                }
            }
        }
    }


// Creates elliptical/circular arc from ArcRecoDataInfo.
bool Processor::CreateArcFromInfo(ArcRecoDataInfo info, PK_CURVE_t * outputCurve, PK_INTERVAL_t * outputInterval)
{
    PK_ERROR_code_t err = 0;

    DPoint3d arcCenterPnt = P(info.arcCenter);
    DPoint3d startPnt = P(info.startPoint);
    DPoint3d quaterPnt = P(info.quarterPoint);
    DPoint3d midPnt = P(info.halfPoint);
    DPoint3d endPnt = P(info.endPoint);

    switch (info.type)
        {
            case ArcType::Full:
            {
            //for now only circle is supported
            double r1 = arcCenterPnt.Distance(startPnt);
            double r2 = arcCenterPnt.Distance(quaterPnt);

            if (fabs(r2 - r1) < _EPSILON)
                {
                DVec3d axis1 = DVec3d::FromStartEndNormalize(arcCenterPnt, startPnt);
                DVec3d axis2 = DVec3d::FromStartEndNormalize(arcCenterPnt, quaterPnt);
                DVec3d norm = DVec3d::FromNormalizedCrossProduct(axis1, axis2);

                PK_CIRCLE_sf_t circleDat;
                circleDat.radius = r1;
                circleDat.basis_set.location = info.arcCenter;

                circleDat.basis_set.axis.coord[0] = norm.x;
                circleDat.basis_set.axis.coord[1] = norm.y;
                circleDat.basis_set.axis.coord[2] = norm.z;

                circleDat.basis_set.ref_direction.coord[0] = axis1.x;
                circleDat.basis_set.ref_direction.coord[1] = axis1.y;
                circleDat.basis_set.ref_direction.coord[2] = axis1.z;

                PK_CIRCLE_t circ;
                err = PK_CIRCLE_create(&circleDat, &circ);

                if (PK_ERROR_no_errors == err)
                    {
                    *outputCurve = circ;
                    *outputInterval = { 0, PI * 2 };
                    return true;
                    }
                }
            break;
            }

            case ArcType::Half:
            case ArcType::Quarter:
            {
            bool bQuarter = info.type == ArcType::Quarter;
            double a = arcCenterPnt.Distance(startPnt);
            double b = arcCenterPnt.Distance(bQuarter ? endPnt : midPnt);

            if (a > b + _EPSILON)
                {
                PK_ELLIPSE_sf_t elData;
                elData.R1 = a;
                elData.R2 = b;
                elData.basis_set.location = info.arcCenter;

                DVec3d x = DVec3d::FromStartEndNormalize(arcCenterPnt, startPnt);
                DVec3d y = DVec3d::FromStartEndNormalize(arcCenterPnt, bQuarter ? endPnt : midPnt);
                DVec3d n = DVec3d::FromNormalizedCrossProduct(x, y);

                elData.basis_set.axis.coord[0] = n.x;
                elData.basis_set.axis.coord[1] = n.y;
                elData.basis_set.axis.coord[2] = n.z;

                elData.basis_set.ref_direction.coord[0] = x.x;
                elData.basis_set.ref_direction.coord[1] = x.y;
                elData.basis_set.ref_direction.coord[2] = x.z;

                PK_ELLIPSE_t el;
                err = PK_ELLIPSE_create(&elData, &el);
                if (PK_ERROR_no_errors == err)
                    {
                    *outputCurve = el;
                    *outputInterval = { 0, bQuarter ? PI * 0.5 : PI };
                    return true;
                    }
                }
            else if (a + _EPSILON < b)
                {
                PK_ELLIPSE_sf_t elData;
                elData.R1 = b;
                elData.R2 = a;
                elData.basis_set.location = info.arcCenter;

                DVec3d x = -1 * DVec3d::FromStartEndNormalize(arcCenterPnt, bQuarter ? endPnt : midPnt);
                DVec3d y = DVec3d::FromStartEndNormalize(arcCenterPnt, startPnt);
                DVec3d n = DVec3d::FromNormalizedCrossProduct(x, y);

                elData.basis_set.axis.coord[0] = n.x;
                elData.basis_set.axis.coord[1] = n.y;
                elData.basis_set.axis.coord[2] = n.z;

                elData.basis_set.ref_direction.coord[0] = x.x;
                elData.basis_set.ref_direction.coord[1] = x.y;
                elData.basis_set.ref_direction.coord[2] = x.z;

                PK_ELLIPSE_t el;
                err = PK_ELLIPSE_create(&elData, &el);
                if (PK_ERROR_no_errors == err)
                    {
                    *outputCurve = el;
                    *outputInterval = { PI * 0.5, bQuarter ? PI : 3.0 * PI / 2.0 };
                    return true;
                    }
                }
            else
                {
                //circle arc
                PK_CIRCLE_sf_t cirData;
                cirData.radius = a;
                cirData.basis_set.location = info.arcCenter;

                DVec3d x = DVec3d::FromStartEndNormalize(arcCenterPnt, startPnt);
                DVec3d y = DVec3d::FromStartEndNormalize(arcCenterPnt, bQuarter ? endPnt : midPnt);
                DVec3d n = DVec3d::FromNormalizedCrossProduct(x, y);

                cirData.basis_set.axis.coord[0] = n.x;
                cirData.basis_set.axis.coord[1] = n.y;
                cirData.basis_set.axis.coord[2] = n.z;

                cirData.basis_set.ref_direction.coord[0] = x.x;
                cirData.basis_set.ref_direction.coord[1] = x.y;
                cirData.basis_set.ref_direction.coord[2] = x.z;

                PK_CIRCLE_t cir;
                err = PK_CIRCLE_create(&cirData, &cir);
                if (PK_ERROR_no_errors == err)
                    {
                    *outputCurve = cir;
                    *outputInterval = { 0, bQuarter ? PI * 0.5 : PI };
                    return true;
                    }
                }
            break;
            }
            default:
                break;
        }

    return false;
    }

// Tries to recognize arc from b-spline. It works in following way:
//   1. analyze b-spline and prepare possible arc type / points
//   2. construct arc from possible arc type / points
//   3. check if constructed arc is concident with initial b-spline
// If "yes" it means we have found arc representation for b-spline.
// Method is not 100%. We just try some fixed configurations.
bool Processor::TryConvertBSplineToArc(PK_BCURVE_t inputBCurve, PK_CURVE_t * outputCurve, PK_INTERVAL_t * outputInterval)
    {
    PK_ERROR_code_t err = 0;

    PK_BCURVE_sf_t bcurve;
    err = PK_BCURVE_ask(inputBCurve, &bcurve);
    if (PK_ERROR_no_errors != err)
        return false;

    ArcRecoDataInfo elData;
    DetectPossibleArcType(inputBCurve, &elData);

    if (CreateArcFromInfo(elData, outputCurve, outputInterval))
        {
        PK_INTERVAL_t bsInt = { bcurve.knot[0], bcurve.knot[bcurve.n_knots - 1] };

        // check if possible variant of arc is coincident with initial b-spline
        if (IsCoincident(inputBCurve, bsInt, *outputCurve, *outputInterval))
            return true;
        }

    return false;
    }

// Tries to create CurvePrimitive from Parasolid 3d curve by given interval.
ICurvePrimitivePtr Processor::Create3dCurveByInterval(PK_CURVE_t curve, PK_INTERVAL_t curveInterval, bool simplifyBsplines)
    {
    // Extract curve class;
    PK_CLASS_t curveClass;
    PK_ENTITY_ask_class(curve, &curveClass);
    switch (curveClass)
        {
            case PK_CLASS_line: // 3001 line
            {
                PK_LINE_sf_t line;
                PK_LINE_ask(curve, &line);
                DPoint3d start = DPoint3d::FromSumOf(P(line.basis_set.location), V(line.basis_set.axis) * curveInterval.value[0]);
                DPoint3d end = DPoint3d::FromSumOf(P(line.basis_set.location), V(line.basis_set.axis) * curveInterval.value[1]);
                return ICurvePrimitive::CreateLine(start, end);
            }
            case PK_CLASS_circle: // 3002 circle
            {
            PK_CIRCLE_sf_t circle;
            PK_CIRCLE_ask(curve, &circle);
            DVec3d majAxis = V(circle.basis_set.ref_direction) * circle.radius;
            DVec3d minAxis;
            minAxis.CrossProduct(V(circle.basis_set.axis), V(circle.basis_set.ref_direction));
            minAxis *= circle.radius;
            return ICurvePrimitive::CreateArc(DEllipse3d::FromVectors(P(circle.basis_set.location), majAxis, minAxis, curveInterval.value[0], curveInterval.value[1] - curveInterval.value[0]));
            }
            case PK_CLASS_ellipse: // 3003 ellipse
            {
            PK_ELLIPSE_sf_t ellipse;
            PK_ELLIPSE_ask(curve, &ellipse);
            DVec3d majAxis = V(ellipse.basis_set.ref_direction) * ellipse.R1;
            DVec3d minAxis;
            minAxis.CrossProduct(V(ellipse.basis_set.axis), V(ellipse.basis_set.ref_direction));
            minAxis *= ellipse.R2;
            return ICurvePrimitive::CreateArc(DEllipse3d::FromVectors(P(ellipse.basis_set.location), majAxis, minAxis, curveInterval.value[0], curveInterval.value[1] - curveInterval.value[0]));
            }
            case PK_CLASS_bcurve: // 3005 bspline curve
            {
            PK_CURVE_t arc;
            PK_INTERVAL_t arcInterval;
            if (simplifyBsplines && TryConvertBSplineToArc(curve, &arc, &arcInterval))
                return Create3dCurveByInterval(arc, arcInterval);

            PK_BCURVE_sf_t bcurve;
            PK_BCURVE_ask(curve, &bcurve);

            if (!(bcurve.is_rational == PK_LOGICAL_false && bcurve.vertex_dim == 3) &&
                !(bcurve.is_rational == PK_LOGICAL_true && bcurve.vertex_dim == 4))
                return nullptr;

            bvector<DPoint3d> poles;
            bvector<double> weights;
            bvector<double> knots;

            for (int i = 0; i < bcurve.n_vertices; i++)
                {
                poles.push_back(DPoint3d::FromXYZ(bcurve.vertex[i*bcurve.vertex_dim],
                                                  bcurve.vertex[i*bcurve.vertex_dim + 1],
                                                  bcurve.vertex[i*bcurve.vertex_dim + 2]));
                if (bcurve.is_rational == PK_LOGICAL_true)
                    weights.push_back(bcurve.vertex[i*bcurve.vertex_dim + 3]);
                }

            for (int i = 0; i < bcurve.n_knots; i++)
                for (int j = 0; j < bcurve.knot_mult[i]; j++)
                    knots.push_back(bcurve.knot[i]);

            MSBsplineCurvePtr msb = MSBsplineCurve::CreateFromPolesAndOrder(poles, &weights, &knots, bcurve.degree + 1, bcurve.is_closed == PK_LOGICAL_true, true);
            if (msb.IsValid())
                return ICurvePrimitive::CreateBsplineCurve(msb);

            return nullptr;
            }
            default:
                return nullptr;
        }
    }

// Tries to create 3d curve from Parasolid 2d bspline curve (sp curve).
ICurvePrimitivePtr Processor::Create3dCurveFromSpCurve(PK_CURVE_t curve, PK_INTERVAL_t curveInterval, PK_VECTOR_t curveEnds[2])
    {
    // Extract spcurve data.
    PK_SPCURVE_sf_t spcurve;
    PK_SPCURVE_ask(curve, &spcurve);

    // Check that curve is just a line.
    // In such case distance between curve start and end is equals to curve length.
    double length = 0;
    PK_INTERVAL_t range = {0, 0};
    PK_CURVE_find_length(spcurve.curve, curveInterval, &length, &range);
    double distance = P(curveEnds[0]).Distance(P(curveEnds[1]));
    if (fabs(length - distance) < _EPSILON)
        return ICurvePrimitive::CreateLine(DSegment3d::From(P(curveEnds[0]), P(curveEnds[1])));

    // Try to extract 3d bspline curve.
    PK_CURVE_make_bcurve_o_t options;
    PK_CURVE_make_bcurve_o_m(options);
    PK_CURVE_make_bcurve_t status = 0;
    PK_BCURVE_t bcurve = PK_ENTITY_null;
    double achievedTol = 0;
    PK_achieved_cont_t achievedCont = 0;
    PK_ERROR_code_t err = PK_CURVE_make_bcurve_2(curve, curveInterval, &options, &status, &bcurve, &achievedTol, &achievedCont);

    if (PK_ERROR_no_errors == err)
        return Create3dCurve(bcurve);

    return nullptr;
    }

// Tries to create 3d curve from edge.
ICurvePrimitivePtr Processor::Create3dCurveFromEdge(PK_EDGE_t edge, PK_FACE_t face)
    {
    // Extract curve from edge.
    PK_CURVE_t edgeCurve;
    PK_CLASS_t edgeCurveClass = PK_CLASS_null;
    PK_INTERVAL_t edgeCurveInterval = {0, 0};
    PK_VECTOR_t edgeCurveEnds[2];
    PK_LOGICAL_t  edgeCurveSense;
    GetEdgeCurve(edge, face, &edgeCurve, &edgeCurveClass, edgeCurveEnds, &edgeCurveInterval, &edgeCurveSense);

    // Edge curve was not found.
    // Something went wrong.
    if (edgeCurve == PK_ENTITY_null)
        return nullptr;

    if (edgeCurveClass == PK_CLASS_spcurve) // 3006 2d bspline curve (sp curve)
        return Create3dCurveFromSpCurve(edgeCurve, edgeCurveInterval, edgeCurveEnds);
    else
        return Create3dCurveByInterval(edgeCurve, edgeCurveInterval);
    }

// Tries to create 3d curve vector from Parasolid trimming loop.
CurveVectorPtr Processor::GetCurveVectorFromLoop(PK_LOOP_t loopTag, CurveVector::BoundaryType boundaryType, double epsilon)
    {
    // Extract face tag.
    PK_FACE_t face;
    PK_LOOP_ask_face(loopTag, &face);

    // Extract edges from loop.
    int n_edges;
    PK_EDGE_t *edges;
    PK_LOOP_ask_edges(loopTag, &n_edges, &edges);
    PK_MEM_Holder eholder(n_edges, edges);

    // Should be at least 1 edge.
    if (n_edges < 1)
        return nullptr;

    // Collect curves from all edges.
    bvector<ICurvePrimitivePtr> curves;
    for (int iEdge = 0; iEdge < n_edges; iEdge++)
        {
        ICurvePrimitivePtr curve = Create3dCurveFromEdge(edges[iEdge], face);
        if (curve.IsValid())
            curves.push_back(curve);
        else
            return nullptr; // Something went wrong.
        }

    // Fill curve vector.
    // Curves in neighboring edges can has "opposite" directions:
    // end point of curve from edge 1 does not match with start point of curve from edge 2.
    // So reverse some curves if necessary for creation of closed contour.
    CurveVectorPtr curveVector(new CurveVector(boundaryType));

    if (n_edges == 1) // Nothing to analyze for 1 edge.
        curveVector->Add(curves[0]);
    else
        {
        DPoint3d end;
        for (int iEdge = 1; iEdge < n_edges; iEdge++)
            {
            // Extract start and end vertices of curve.
            ICurvePrimitivePtr curve = curves[iEdge];
            DPoint3d ptCurve[2];
            curve->GetStartEnd(ptCurve[0], ptCurve[1]);

            // Analyze first curve.
            if (iEdge == 1)
                {
                // Extract start and end vertices of first curve.
                ICurvePrimitivePtr firstCurve = curves[0];
                DPoint3d ptFirst[2];
                firstCurve->GetStartEnd(ptFirst[0], ptFirst[1]);

                // Find common vertex.
                // End vertex of first curve is common. OK.
                if (ptCurve[0].IsEqual(ptFirst[1], epsilon) || ptCurve[1].IsEqual(ptFirst[1], epsilon))
                    {
                    end = ptFirst[1];
                    }
                // Start vertex of first curve is common. Reverse start curve.
                else if (ptCurve[0].IsEqual(ptFirst[0], epsilon) || ptCurve[1].IsEqual(ptFirst[0], epsilon))
                    {
                    firstCurve->ReverseCurvesInPlace();
                    end = ptFirst[0];
                    }
                else // No common vertex.
                    return nullptr;

                curveVector->Add(firstCurve);
                }

            // Analyze curve.
            if (ptCurve[0].IsEqual(end, epsilon)) // Start vertex of curve is the same as end vertex of previous curve. OK.
                {
                end = ptCurve[1];
                }
            else if (ptCurve[1].IsEqual(end, epsilon)) // End vertex of curve is the same as end vertex of previous curve. Reverse curve.
                {
                curve->ReverseCurvesInPlace();
                end = ptCurve[0];
                }
            else // No common vertex.
                return nullptr;

            curveVector->Add(curve);
            }
        }

    return curveVector;
    }

// Checks that trimming loop is just a point.
// If true, returns a point.
bool Processor::IsLoopJustPoint(PK_LOOP_t loopTag, DPoint3dP point)
    {
    point->Zero();

    int n_edges;
    PK_LOOP_ask_edges(loopTag, &n_edges, NULL);

    if(n_edges == 0)
        {
        int n_vertices;
        PK_VERTEX_t *vertices;
        PK_LOOP_ask_vertices(loopTag, &n_vertices, &vertices);
        PK_MEM_Holder vholder(n_vertices, vertices);

        if(n_vertices == 1)
            {
            PK_POINT_t pt;
            PK_VERTEX_ask_point(vertices[0], &pt);
            PK_POINT_sf_t ptData;
            PK_POINT_ask(pt, &ptData);
            *point = P(ptData.position);
            return true;
            }
        }

    return false;
    }

// Tries to get vertex from trimming loop by index.
bool Processor::TryGetVertexFromLoopByIndex(PK_LOOP_t loopTag, int index, DPoint3dP point)
    {
    point->Zero();

    int n_vertices;
    PK_VERTEX_t *vertices;
    PK_LOOP_ask_vertices(loopTag, &n_vertices, &vertices);
    PK_MEM_Holder vholder(n_vertices, vertices);

    if (n_vertices > 0 && index >= 0 && n_vertices > index)
        {
        PK_POINT_t pt;
        PK_VERTEX_ask_point(vertices[index], &pt);
        PK_POINT_sf_t ptData;
        PK_POINT_ask(pt, &ptData);
        *point = P(ptData.position);
        return true;
        }

    return false;
    }

// Tries to approximate list of consecutive points to ellipse or circle.
// circleEpsilon is an allowed MinMajRatio deviation from 1.0 for circle.
// axisEpsilon is an allowed deviation (in radians) from perpendicular ellipse axes.
// planeEpsilon is an allowed deviation for planarity checking.
BentleyStatus Processor::TryGetEllipseFromPoints(DEllipse3dP output, bvector<DPoint3d> curvePoints, double circleEpsilon, double axisEpsilon, double planeEpsilon)
    {
    if (curvePoints.size()<1)
        return ERROR;
    // Not closed curve is not an ellipse.
    if (!curvePoints[0].IsEqual(curvePoints.back(), _EPSILON))
        return ERROR;

    // Check if all points are in the same plane.
    DVec3d normal;
    if (!IsPointsInSamePlane(curvePoints, &normal, planeEpsilon))
        return ERROR;

    DRange3d ellipseRange;
    ellipseRange.Init();
    ellipseRange.Extend(curvePoints);

    // Calculate center point of ellipse.
    DPoint3d center = DPoint3d::From(ellipseRange.low.x + ellipseRange.XLength() / 2.0,
                                     ellipseRange.low.y + ellipseRange.YLength() / 2.0,
                                     ellipseRange.low.z + ellipseRange.ZLength() / 2.0);

    // Min axis has minimum length.
    // Maj axis has maximun length.
    // So try to find fit curve points.
    DPoint3d minAxisPoint, majAxisPoint;
    double min = ellipseRange.DiagonalDistance(), max = 0;
    for (auto& point : curvePoints)
        {
        double len = point.Distance(center);

        // Candidate for point of min axis.
        if (len < min)
            {
            minAxisPoint = point;
            min = len;
            }

        // Candidate for point of maj axis.
        if (len > max)
            {
            majAxisPoint = point;
            max = len;
            }
        }

    // Calculate maj and min axis.
    DVec3d majAxis = DVec3d::FromStartEnd(center, majAxisPoint);
    DVec3d minAxis = DVec3d::FromStartEnd(center, minAxisPoint);
    if (fabs(1.0 - minAxis.Magnitude() / majAxis.Magnitude()) < circleEpsilon) // Circle.
        {
        *output = DEllipse3d::FromCenterNormalRadius(center, normal, majAxis.Magnitude());
        return SUCCESS;
        }

    // If angle between vectors is close to Pi/2, ellipse was found.
    if (fabs(PI / 2.0 - majAxis.AngleTo(minAxis)) < axisEpsilon) // Ellipse.
        {
        *output = DEllipse3d::FromVectors(center, majAxis, minAxis, 0, PI * 2.0);
        return SUCCESS;
        }

    return ERROR;
    }

// Tries to approximate trimming loop to ellipse or circle.
// circleEpsilon is an allowed MinMajRatio deviation from 1.0 for circle.
// axisEpsilon is an allowed deviation (in radians) from perpendicular ellipse axes.
// planeEpsilon is an allowed deviation for planarity checking.
BentleyStatus Processor::TryGetEllipseFromLoop(DEllipse3dP output, PK_LOOP_t loopTag, double circleEpsilon, double axisEpsilon, double planeEpsilon)
    {
    // Extract face tag.
    PK_FACE_t face;
    PK_LOOP_ask_face(loopTag, &face);

    // Extract edges from loop.
    int n_edges;
    PK_EDGE_t *edges;
    PK_LOOP_ask_edges(loopTag, &n_edges, &edges);
    PK_MEM_Holder eholder(n_edges, edges);

    bvector<DPoint3d> curvePoints;
    for (int iEdge = 0; iEdge < n_edges; iEdge++)
        {
        // Extract curve from edge.
        PK_CURVE_t edgeCurve;
        PK_CLASS_t edgeCurveClass = PK_CLASS_null; // TODO Initialize with proper defaults
        PK_INTERVAL_t edgeCurveInterval = {0, 0}; // TODO Initialize with proper defaults
        PK_VECTOR_t edgeCurveEnds[2];
        PK_LOGICAL_t  edgeCurveSense;
        GetEdgeCurve(edges[iEdge], face, &edgeCurve, &edgeCurveClass, edgeCurveEnds, &edgeCurveInterval, &edgeCurveSense);

        // Edge curve was not found.
        // Something went wrong.
        if (edgeCurve == PK_ENTITY_null)
            return ERROR;

        switch (edgeCurveClass)
            {
            case PK_CLASS_circle: // 3002 circle
                {
                // TODO: check possibility of few elliptical or circular arcs.
                if (n_edges > 1)
                    return ERROR;

                // Full circle has interval from 0 to 2*PI.
                if (fabs(edgeCurveInterval.value[0]) < _EPSILON && fabs(edgeCurveInterval.value[1] - 2.0 * PI) < _EPSILON)
                    {
                    PK_CIRCLE_sf_t circle;
                    PK_CIRCLE_ask(edgeCurve, &circle);
                    *output = DEllipse3d::FromCenterNormalRadius(P(circle.basis_set.location), V(circle.basis_set.axis), circle.radius);
                    return SUCCESS;
                    }
                else
                    return ERROR;
                }
                break;
            case PK_CLASS_ellipse: // 3003 ellipse
                {
                // TODO: check possibility of few elliptical or circular arcs.
                if (n_edges > 1)
                    return ERROR;

                // Full ellipse has interval from 0 to 2*PI.
                if (fabs(edgeCurveInterval.value[0]) < _EPSILON && fabs(edgeCurveInterval.value[1] - 2.0 * PI) < _EPSILON)
                    {
                    PK_ELLIPSE_sf_t ellipse;
                    PK_ELLIPSE_ask(edgeCurve, &ellipse);
                    DVec3d majAxis = V(ellipse.basis_set.ref_direction) * ellipse.R1;
                    DVec3d minAxis;
                    minAxis.CrossProduct(V(ellipse.basis_set.axis), V(ellipse.basis_set.ref_direction));
                    minAxis *= ellipse.R2;
                    *output = DEllipse3d::FromVectors(P(ellipse.basis_set.location), majAxis, minAxis, 0, PI * 2.0);
                    return SUCCESS;
                    }
                else
                    return ERROR;
                }
                break;
            case PK_CLASS_spcurve: // 3006 2d bspline curve (sp curve)
                {
                // Collect trimming curve points to full trimming loop.
                bvector<DPoint3d> points = Get3dPointsFromCurve(edgeCurve, edgeCurveInterval, 1200);
                curvePoints.insert(curvePoints.end(), points.begin(), points.end());
                }
                break;
            default:
                return ERROR;
            }
        }

    return TryGetEllipseFromPoints(output, curvePoints, circleEpsilon, axisEpsilon, planeEpsilon);
    }

BentleyStatus Processor::GetEdgeCurve(PK_EDGE_t edge, PK_FACE_t face, PK_CURVE_t *edgeCurve, PK_CLASS_t *edgeCurveClass, PK_VECTOR_t edgeCurveEnds[2], PK_INTERVAL_t *edgeCurveInterval, PK_LOGICAL_t  *edgeCurveSense)
    {
    // Extract curve from edge.
    PK_EDGE_ask_curve(edge, edgeCurve);
    // If edge curve is null, curve should be extracted from fin.
    if (*edgeCurve == PK_ENTITY_null)
        {
        int n_fins;
        PK_EDGE_t *fins;
        PK_EDGE_ask_fins(edge, &n_fins, &fins);
        PK_MEM_Holder fholder(n_fins, fins);

        if (n_fins > 0)
            {
            for (int iFin = 0; iFin < n_fins; iFin++)
                {
                PK_FACE_t finFace;
                PK_FIN_ask_face(fins[iFin], &finFace);
                // Very important to find fin of appropriate face,
                // because parametrization of different faces is different.
                if (face == finFace)
                    {
                    // Fin is found.
                    // Extract curve geometry.
                    PK_FIN_ask_geometry(fins[iFin], PK_LOGICAL_true, edgeCurve, edgeCurveClass, edgeCurveEnds, edgeCurveInterval, edgeCurveSense);
                    return SUCCESS;
                    break;
                    }
                }
            }
        else
            return ERROR;
        }
    else
        {
        // Extract curve geometry.
        PK_EDGE_ask_geometry(edge, PK_LOGICAL_true, edgeCurve, edgeCurveClass, edgeCurveEnds, edgeCurveInterval, edgeCurveSense);
        return SUCCESS;
        }

    return ERROR;
    }

// Checks that all points from list belongs to same plane.
// If points count is from 0 to 3, result will be true.
// If points count 3 or more and all points belongs to same plane,
// output normal vector to the plane will be defined (if possible).
//
// NOTE: this function does not find 3 points from list that are not on a single line for normal calculation.
// Plane normal is calculated by 3 points with indices  0, pointsCount / 3, pointsCount * 2 / 3.
bool Processor::IsPointsInSamePlane(bvector<DPoint3d> points, DVec3dP normal, double planeEpsilon)
    {
    // Reset normal.
    normal->Zero();

    // 3 or less points are definitely in the same plane.
    if (points.size() <= 3)
        {
        // Calculate normal.
        if(points.size() == 3)
            normal->NormalizedCrossProduct(points[1] - points[0], points[2] - points[0]);
        return true;
        }

    // Calculate plane normal by 3 points (2 vectors).
    normal->NormalizedCrossProduct(points[points.size() / 3] - points[0], points[points.size() / 3 * 2] - points[0]);

    // Zero normal. 3 points on a single line.
    if (normal->IsEqual(DVec3d::FromZero(), _EPSILON))
        return false;

    // Calculate D.
    double d = normal->DotProduct(points[0]);

    // Check if all points are in the same plane.
    for (auto& point : points)
        if (fabs(normal->DotProduct(point) - d) > planeEpsilon)
            {
            normal->Zero();
            return false;
            }

    return true;
    }

bool Processor::IsLoopsSame(PK_LOOP_t loopTag1, PK_LOOP_t loopTag2)
    {
    // Extract edges of first trimming loop.
    int n_edges1;
    PK_EDGE_t *edges1;
    PK_LOOP_ask_edges(loopTag1, &n_edges1, &edges1);
    PK_MEM_Holder eholder1(n_edges1, edges1);

    // Extract edges of second trimming loop.
    int n_edges2;
    PK_EDGE_t *edges2;
    PK_LOOP_ask_edges(loopTag2, &n_edges2, &edges2);
    PK_MEM_Holder eholder2(n_edges2, edges2);

    // Trimming loops are the same if they contain same edges.
    // Edges order in loops may be different.
    if(n_edges1 == n_edges2)
        {
        for(int iEdge1 = 0; iEdge1 < n_edges1; iEdge1++)
            {
            bool exist = false;
            for (int iEdge2 = 0; iEdge2 < n_edges2; iEdge2++)
                if(edges1[iEdge1] == edges2[iEdge2])
                    {
                    exist = true;
                    break;
                    }

            if (!exist)
                return false;
            }

        return true;
        }

    return false;
    }

// Checks if ellipses are the same by following conditions:
// - Both ellipses are full (sweep angle from 0 to 2PI).
// - Origins of ellipses are the same.
// - Ratio of appropriate axes lengths less than axisEpsilon value.
bool Processor::IsEllipsesSame(DEllipse3dCR ellipse1, DEllipse3dCR ellipse2, double axisEpsilon)
    {
    if (!ellipse1.IsFullEllipse() || !ellipse2.IsFullEllipse())
        return false;

    if (ellipse1.center.Distance(ellipse2.center) > _EPSILON)
        return false;

    if (fabs(1.0 - ellipse1.vector0.Magnitude() / ellipse2.vector0.Magnitude()) > axisEpsilon ||
        fabs(1.0 - ellipse1.vector90.Magnitude() / ellipse2.vector90.Magnitude()) > axisEpsilon)
        return false;

    return true;
    }

// Extracts 3d point from Parasolid vertex.
DPoint3d Processor::Get3dPointFromVertex(PK_VERTEX_t vertex)
    {
    PK_POINT_t point;
    PK_POINT_sf_t pointSf;
    PK_VERTEX_ask_point(vertex, &point);
    PK_POINT_ask(point, &pointSf);
    return P(pointSf);
    }

// Extracts 3d point from Parasolid curve at a given parameter value.
DPoint3d Processor::Get3dPointFromCurve(PK_CURVE_t curve, double t)
    {
    // Extract class of curve.
    PK_CLASS_t curveClass;
    PK_ENTITY_ask_class(curve, &curveClass);

    if(curveClass == PK_CLASS_spcurve) // 3006 2d bspline curve (sp curve)
        {
        // Extract spcurve.
        PK_SPCURVE_sf_t spcurve;
        PK_SPCURVE_ask(curve, &spcurve);

        // Extract parametric point.
        PK_VECTOR_t paramPoint;
        PK_CURVE_eval(spcurve.curve, t, 0, &paramPoint);

        // Get 3d point of surface by uv parameters.
        PK_UV_t uvPoint;
        uvPoint.param[0] = paramPoint.coord[0];
        uvPoint.param[1] = paramPoint.coord[1];
        PK_VECTOR_t surfPoint;
        PK_SURF_eval(spcurve.surf, uvPoint, 0, 0, PK_LOGICAL_false, &surfPoint);
        return P(surfPoint);
        }

    // Extract 3d point.
    PK_VECTOR_t point;
    PK_CURVE_eval(curve, t, 0, &point);
    return P(point);
    }

// Extracts necessary count of 3d points from Parasolid curve at a given interval.
bvector<DPoint3d> Processor::Get3dPointsFromCurve(PK_CURVE_t curve, PK_INTERVAL_t interval, int segmentsCount)
    {
    bvector<DPoint3d> result;

    if (segmentsCount > 0)
        {
        PK_CURVE_t inputCurve = curve;
        PK_SURF_t surf = PK_ENTITY_null;
        result.reserve(segmentsCount + 1);

        // Get necessary curve range from interval in parameter space.
        double range = interval.value[1] - interval.value[0];

        // Extract class of curve.
        PK_CLASS_t curveClass;
        PK_ENTITY_ask_class(inputCurve, &curveClass);
        if (curveClass == PK_CLASS_spcurve) // 3006 2d bspline curve (sp curve)
            {
            PK_SPCURVE_sf_t spcurve;
            PK_SPCURVE_ask(inputCurve, &spcurve);
            inputCurve = spcurve.curve;
            surf = spcurve.surf;
            }

        // Get 3d points of curve.
        PK_VECTOR_t point;
        for (int i = 0; i <= segmentsCount; i++)
            {
            double t = interval.value[0] + range * ((double) i / segmentsCount);

            if (curveClass == PK_CLASS_spcurve) // 3006 2d bspline curve (sp curve)
                {
                // Extract parametric point.
                PK_CURVE_eval(inputCurve, t, 0, &point);

                // Get 3d point of surface by uv parameters.
                PK_UV_t uvPoint;
                uvPoint.param[0] = point.coord[0];
                uvPoint.param[1] = point.coord[1];
                PK_SURF_eval(surf, uvPoint, 0, 0, PK_LOGICAL_false, &point);
                result.push_back(P(point));
                }
            else
                {
                // Extract 3d point.
                PK_CURVE_eval(inputCurve, t, 0, &point);
                result.push_back(P(point));
                }
            }
        }

    return result;
    }

// When a cone (or cylinder) is cut but unrelated planes at two ends, the "Major and minor axis" points commonly
// used to define the ellipse are determined inedpendently on each plane, and are not on the same rule lines of the cone.
//
// This logic takes ellipseA and ellipseB from any two sections and constructs replacement for ellipseB, such that
// the parameterizations agree and thus preserve rule lines of the (possibly skewed) cone.
//
// Typically both inputs will be "major minor" ellipse form.
// But the "top ellipse" of the ruled surface will have non-perpendicular defining vectors.
ISolidPrimitivePtr Processor::ConstructChiseledConesFromSectionsWithUnrelatedEllipseDefinitions
(
    bvector<DEllipse3d> profile,    // sequence of ellipses
    bool isCapped                   // Is ruled sweep capped
)
    {
    if (profile.size() < 2)
        return nullptr;

    DEllipse3d ellipseA = profile[0];
    auto cvA = CreateSplitDisk(ellipseA);
    auto sections = bvector<CurveVectorPtr>{cvA};

    for(int i = 1; i < profile.size(); i++)
        {
        // vectorC0 and vectorC90 are defining vectors for the same ellipse points as ellipseB.
        // But they are shifted so that they are in the same rule lines as vectorA0 and vectorA90.
        DEllipse3d ellipseB = profile[i];
        DVec3d vectorC0 = ConstructCorrespondingEllipseVector(ellipseA.center, ellipseA.vector0, ellipseB.center, ellipseB.vector0, ellipseB.vector90);
        DVec3d vectorC90 = ConstructCorrespondingEllipseVector(ellipseA.center, ellipseA.vector90, ellipseB.center, ellipseB.vector0, ellipseB.vector90);
        DEllipse3d ellipseC = DEllipse3d::FromVectors(ellipseB.center, vectorC0, vectorC90, 0.0, Angle::TwoPi());
        auto cvC = CreateSplitDisk(ellipseC);
        sections.push_back(cvC);
        ellipseA = ellipseC;
        }

    return ISolidPrimitive::CreateDgnRuledSweep(DgnRuledSweepDetail(sections, isCapped));
    }

ISolidPrimitivePtr Processor::ConstructChiseledPipesFromSectionsWithUnrelatedEllipseDefinitions
(
    bvector<ChiseledConeInfo> profile,  // sequence of 'rings'
    bool isCapped                       // Is ruled sweep capped
)
    {
    if (profile.size() < 2)
        return nullptr;

    DEllipse3d ellipseA = profile[0].ellipses[0];
    ellipseA.center.Subtract(profile[1].ellipses[0].center - profile[0].ellipses[0].center);
    bvector<CurveVectorPtr> sections;
    for (int i = 0; i < profile.size(); i++)
        {
        ChiseledConeInfo info = profile[i];
        if (!info.IsValid())
            return nullptr;

        CurveVectorPtr profileCurve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
        for (int j = 0; j < 2; j++)
            {
            DEllipse3d ellipseB = info.ellipses[j];
            DVec3d vectorC0 = ConstructCorrespondingEllipseVector(ellipseA.center, ellipseA.vector0, ellipseB.center, ellipseB.vector0, ellipseB.vector90);
            DVec3d vectorC90 = ConstructCorrespondingEllipseVector(ellipseA.center, ellipseA.vector90, ellipseB.center, ellipseB.vector0, ellipseB.vector90);
            DEllipse3d ellipseC = DEllipse3d::FromVectors(ellipseB.center, vectorC0, vectorC90, 0.0, Angle::TwoPi());
            profileCurve->Add(CreateSplitDisk(ellipseC, j == 0 ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Inner));
            if (j == 1)
                ellipseA = ellipseC;
            }

        sections.push_back(profileCurve);
        }

    return ISolidPrimitive::CreateDgnRuledSweep(DgnRuledSweepDetail(sections, isCapped));
    }

// Create a curve vector for an elliptic disk.
// This should be an single arc primitive in a loop.
// But PP is aggressive about converting that to an ellipse element whose start point
//   might be shifted to a major axis point.
// To prevent that, split the arc in two pieces.
CurveVectorPtr Processor::CreateSplitDisk(DEllipse3dCR arc, CurveVector::BoundaryType boundaryType)
    {
    auto cv = CurveVector::Create(boundaryType);
    auto arc0 = arc;
    auto arc1 = arc;
    arc0.sweep *= 0.5;
    arc1.sweep *= 0.5;
    arc1.start = arc0.start + arc0.sweep;
    cv->push_back(ICurvePrimitive::CreateArc(arc0));
    cv->push_back(ICurvePrimitive::CreateArc(arc1));
    return cv;
    }

// GIVEN ... vectorA from centerA to any point on one section ellipse for a cone.
//           (Note that pointA = centerA + vectorA is on both (a) the ellipse at section A and (b) a particular
//                rule line of the cone.)
// AND   ... the defining vectors (usually major/minor vectors) of the other ellipse . . .
// COMPUTE A vector from centerB to that is (a) on ellipseB and (b) on the same cone rule line as vectorA
//
DVec3d Processor::ConstructCorrespondingEllipseVector
(
    DPoint3dCR centerA,         // center of ellipse A
    DVec3dCR   vectorA,         // any vector from centerA to a point on ellipseA
    DPoint3dCR centerB,         // center of ellipseB
    DVec3dCR   vectorB0,        // 0 degree vector (e.g. major axis)
    DVec3dCR   vectorB90        // 90 degree vector (e.g. minor axis)
)
    {
    DVec3d vectorAB = centerB - centerA;
    DVec3d planeNormal = DVec3d::FromCrossProduct(vectorA, vectorAB);
    // The centerA, centerB, and vectorA are in a plane, with planeNormal perpendicular to all vectors in that plane.
    // Find the intersections of ellipseB with that plane.
    // There are two intersections, 180 degrees apart in the parameter space of ellipseB.
    // A vector from centerB to a point "at parameter space angle theta" on ellipseB
    //     vectorB0 * cos(theta) + vectorB90 * sin(theta)
    // To make that vector "in plane", it must be perpendicular to the planeNormal:
    //     planeNormal DOT (vectorB0 * cos(theta) + vectorB90 * sin(theta)) = 0
    double dot0 = planeNormal.DotProduct(vectorB0);
    double dot90 = planeNormal.DotProduct(vectorB90);
    // we need         dot0 * cos(theta) + dot90 * sin(theta) = 0
    //      i.e.    sin(theta)/cos(theta) = tan (theta) = - dot0 / dot90
    // There are two such angles.  The angles are 180 degrees apart and define vectors that are opposite of each other.
    double theta = atan2(-dot0, dot90);
    DVec3d candidateVector = vectorB0 * cos(theta) + vectorB90 * sin(theta);
    double dot = candidateVector.DotProduct(vectorA);
    // Special case when vectors are perpendicular and checking can't be based on dot product sign.
    if (fabs(dot) < 1e-4)
        {
        // Line segments [centerA; centerB] and [centerA+vectorA; centerB+candidateVector] are in the same plane.
        // If candidateVector has proper direction, segments have no intersection.
        // If segments have intersection, candidateVector should be inversed.

        // Move segments to local coordinates.
        // Make vectorAB a X axis, planeNormal normal a Z axis and centerA an origin.
        // If ends of segment [centerA+vectorA; centerB+candidateVector] have Y-coord with different signs,
        // inverse candidateVector.
        DVec3d xAxis = vectorAB.ValidatedNormalize();
        DVec3d zAxis = planeNormal.ValidatedNormalize();
        DVec3d yAxis = DVec3d::FromCrossProduct(zAxis, xAxis);
        RotMatrix rm = RotMatrix::FromColumnVectors(xAxis, yAxis, zAxis); // Local to world matrix.
        rm.Invert(); // World to local matrix.
        DPoint3d origin = DPoint3d::From(-centerA.x, -centerA.y, -centerA.z);
        rm.Multiply(origin); // Calculate origin.
        Transform toLocal = Transform::From(rm, origin);
        DPoint3d pt1 = centerA + vectorA;
        toLocal.Multiply(pt1);
        DPoint3d pt2 = centerB + candidateVector;
        toLocal.Multiply(pt2);

        if (pt1.y*pt2.y < 0)
            candidateVector = candidateVector * -1.0;
        }
    else if (dot < 0.0)
        candidateVector = candidateVector * -1.0;

    return candidateVector;
    }

BentleyStatus Processor::ProcessParasolidPlane(PK_PLANE_t Surface, PK_LOOP_t *loops, PK_BODY_t bodyTag, PlaneLoopInfo& loopInfo, ExBoxInfo& exboxInfo, std::set<VertexElem>& vertexSet, std::set<VectorElem>& vectorSet)
    {
    PK_PLANE_sf_t Data;
    PK_ERROR_code_t err = PK_PLANE_ask(Surface, &Data);

    loopInfo.vec = Data.basis_set.axis;
    vectorSet.insert(loopInfo.vec);

    if (PK_ERROR_no_errors != err)
        return ERROR;

    // Process trimming loops.
    for (int i = 0; i < loopInfo.nLoops; i++)
        {
        int nVert;
        PK_VERTEX_t *Verts;
        bvector<DPoint3d> loopPoints;
        if (PK_ERROR_ok == PK_LOOP_ask_vertices(loops[i], &nVert, &Verts))
            {
            PK_MEM_Holder vholder(nVert, Verts);

            DPoint3d prevPoint, currPoint, nextPoint;
            for (int j = 0; j < nVert; j++)
                {
                currPoint = Get3dPointFromVertex(Verts[j]);

                // Some sides of plane can be broken to several edges.
                // So try to join such edges to one.
                prevPoint = Get3dPointFromVertex(Verts[j > 0 ? j - 1 : nVert - 1]);
                nextPoint = Get3dPointFromVertex(Verts[j < nVert - 1 ? j + 1 : 0]);
                if ((currPoint - prevPoint).IsParallelTo(nextPoint - currPoint, _EPSILON))
                    continue;

                loopPoints.push_back(currPoint);
                }

            // Fill vertex set.
            for (auto& point : loopPoints)
                vertexSet.insert(point);

            loopInfo.nEdges += (int) (loopPoints.size());
            if (loopPoints.size() != 4)
                loopPoints.clear();
            }

        //1 loop, 4 edges candidate for a box side
        if (loopInfo.nLoops == 1)
            {
            if (loopInfo.nEdges == 4)
                {
                auto vec1 = loopPoints[1] - loopPoints[0];
                if (vec1.IsParallelTo(loopPoints[3] - loopPoints[2], _EPSILON))
                    {
                    auto vec2 = loopPoints[2] - loopPoints[1];
                    if (vec2.IsParallelTo(loopPoints[3] - loopPoints[0], _EPSILON))
                        {
                        exboxInfo.vec = loopInfo.vec;
                        std::set<VertexElem> sortPoints;
                        sortPoints.insert(DPoint3d(loopPoints[0]));
                        sortPoints.insert(DPoint3d(loopPoints[1]));
                        sortPoints.insert(DPoint3d(loopPoints[2]));
                        sortPoints.insert(DPoint3d(loopPoints[3]));
                        auto it = sortPoints.begin();
                        exboxInfo.point1 = it++->position;
                        exboxInfo.point2 = it++->position;
                        exboxInfo.point3 = it->position;
                        return BentleyStatus::SUCCESS;
                        }
                    }
                }
            }
        }
    return BentleyStatus::ERROR;
    }

// the complete all parallel boxes only
ISolidPrimitivePtr Processor::CreateBox(std::set<VertexElem> vertexSet)
    {
    auto pit = vertexSet.begin();
    auto point1 = pit++->position;
    auto point2 = pit++->position;
    auto point3 = pit++->position;
    auto point4 = pit++->position;
    UNUSED_VARIABLE(point4); // ###TODO: Waiting to hear from authors if this indicates a bug.
    auto point5 = pit->position;

    auto vectorX_ = point2 - point1;
    auto vectorY_ = point3 - point1;
    vectorX_.Normalize();
    vectorY_.Normalize();

    double baseX = point1.Distance(point2);
    double baseY = point1.Distance(point3);

    DgnBoxDetail boxDetail = DgnBoxDetail(point1, point5, vectorX_, vectorY_, baseX, baseY, baseX, baseY, true);
    return ISolidPrimitive::CreateDgnBox(boxDetail);
    }

bool  isFaceAdjucent(const PK_FACE_t& face1, const PK_FACE_t& face2, std::vector<DPoint3d>& loop1, std::vector<DPoint3d>& loop2 )
    {
    std::set<PK_LOOP_t> loopResult;
    std::vector<DPoint3d> commonPoints;
    std::set<VertexElem> vertexSet;
    int nVerts;
    PK_VERTEX_t *verts;
    loop1.clear(); loop2.clear();
    PK_FACE_ask_vertices(face1, &nVerts, &verts);
    PK_MEM_Holder vholder(nVerts, verts);

    for (int i = 0; i < nVerts; i++)
        {
        PK_POINT_t pt;
        PK_POINT_sf_t point1;
        PK_VERTEX_ask_point(verts[i], &pt);
        PK_POINT_ask(pt, &point1);
        vertexSet.insert(DPoint3d::FromArray(point1.position.coord));
        }

    PK_VERTEX_t *verts2;
    PK_FACE_ask_vertices(face2, &nVerts, &verts2);
    PK_MEM_Holder vholder2(nVerts, verts2);

    for (int i = 0; i < nVerts; i++)
        {
        PK_POINT_t pt;
        PK_POINT_sf_t point1;
        PK_VERTEX_ask_point(verts2[i], &pt);
        PK_POINT_ask(pt, &point1);

        auto insresult = vertexSet.insert(DPoint3d::FromArray(point1.position.coord));
        if (!insresult.second)//common pt
            commonPoints.push_back(insresult.first->position);
        }

    if (commonPoints.size() == 2)
        {
        int nEdges;
        PK_EDGE_t *edges;
        PK_FACE_ask_edges(face1, &nEdges, &edges);
        PK_MEM_Holder eholder(nEdges, edges);

        for (int iEdge = 0; iEdge < nEdges; iEdge++)
            {
            PK_VERTEX_t vertices[2] {PK_ENTITY_null, PK_ENTITY_null};
            PK_EDGE_ask_vertices(edges[iEdge], vertices);
            if (vertices[0] != PK_ENTITY_null && vertices[1] != PK_ENTITY_null)
                {
                PK_POINT_t p1, p2;
                PK_POINT_sf_t point1, point2;
                PK_VERTEX_ask_point(vertices[0], &p1);
                PK_POINT_ask(p1, &point1);
                PK_VERTEX_ask_point(vertices[1], &p2);
                PK_POINT_ask(p2, &point2);
                auto cmpPoint1 = DPoint3d::FromArray(point1.position.coord);
                auto cmpPoint2 = DPoint3d::FromArray(point2.position.coord);

                if (commonPoints.front().IsEqual(cmpPoint1) && commonPoints.back().IsEqual(cmpPoint2))
                    continue;
                if (commonPoints.front().IsEqual(cmpPoint2) && commonPoints.back().IsEqual(cmpPoint1))
                    continue;

                if (commonPoints.front().IsEqual(cmpPoint1))
                    {
                    loop1.push_back(cmpPoint2);
                    }
                if (commonPoints.back().IsEqual(cmpPoint1))
                    {
                    loop2.push_back(cmpPoint2);
                    }
                if (commonPoints.front().IsEqual(cmpPoint2))
                    {
                    loop1.push_back(cmpPoint1);
                    }
                if (commonPoints.back().IsEqual(cmpPoint2))
                    {
                    loop2.push_back(cmpPoint1);
                    }
                }
            }

            loop1.push_back(commonPoints.front());
            loop2.push_back(commonPoints.back());

            PK_FACE_ask_edges(face2, &nEdges, &edges);
            PK_MEM_Holder eholder2(nEdges, edges);

            for (int iEdge = 0; iEdge < nEdges; iEdge++)
                {
                PK_VERTEX_t vertices[2] {PK_ENTITY_null, PK_ENTITY_null};
                PK_EDGE_ask_vertices(edges[iEdge], vertices);
                if (vertices[0] != PK_ENTITY_null && vertices[1] != PK_ENTITY_null)
                    {
                    PK_POINT_t p1, p2;
                    PK_POINT_sf_t point1, point2;
                    PK_VERTEX_ask_point(vertices[0], &p1);
                    PK_POINT_ask(p1, &point1);
                    PK_VERTEX_ask_point(vertices[1], &p2);
                    PK_POINT_ask(p2, &point2);
                    auto cmpPoint1 = DPoint3d::FromArray(point1.position.coord);
                    auto cmpPoint2 = DPoint3d::FromArray(point2.position.coord);
                    if (commonPoints.front().IsEqual(cmpPoint1) && commonPoints.back().IsEqual(cmpPoint2))
                        continue;
                    if (commonPoints.front().IsEqual(cmpPoint2) && commonPoints.back().IsEqual(cmpPoint1))
                        continue;

                    if (commonPoints.front().IsEqual(cmpPoint1))
                        {
                        loop1.push_back(cmpPoint2);
                        }
                    if (commonPoints.back().IsEqual(cmpPoint1))
                        {
                        loop2.push_back(cmpPoint2);
                        }
                    if (commonPoints.front().IsEqual(cmpPoint2))
                        {
                        loop1.push_back(cmpPoint1);
                        }
                    if (commonPoints.back().IsEqual(cmpPoint2))
                        {
                        loop2.push_back(cmpPoint1);
                        }
                    }
                }

            }
    return loop1.size()==3 && loop2.size() == 3;
    }

//asymmetric and uncapped "boxes"
ISolidPrimitivePtr Processor::CreateBoxEx(std::set<VectorElem> vectorSet, std::list<ExBoxInfo>& boxInfo, int nFaces, const PK_FACE_t* faces)
    {
    if (nFaces == 4)    //  find missing sides first
        {
        std::vector<DPoint3d> loop1;
        std::vector<DPoint3d> loop2;
        std::vector<DPoint3d> loop3;
        std::vector<DPoint3d> loop4;
        //TODO - implement with a face loop sum
        if (isFaceAdjucent(faces[0], faces[1], loop1, loop2) && isFaceAdjucent(faces[2], faces[3], loop3, loop4) ||
            isFaceAdjucent(faces[0], faces[3], loop1, loop2) && isFaceAdjucent(faces[1], faces[2], loop3, loop4))
            {
                if (loop1.front() == loop3.front() || loop1.front() == loop3.back())
                    {
                    loop1.push_back(loop3[1]);
                    loop2.push_back(loop4[1]);
                    }
                if (loop1.front() == loop4.front() || loop1.front() == loop4.back())
                    {
                    loop1.push_back(loop4[1]);
                    loop2.push_back(loop3[1]);
                    }
            }

        if (loop1.size() == 4 && loop2.size() == 4)
            {
            //Note - only parallelogramm allowed at the the base and top, and the holes at the base and top only
            if (loop1[0].Distance(loop1[1]) - loop1[2].Distance(loop1[3]) > _EPSILON)
                return nullptr;

            if (loop1[0].Distance(loop1[3]) - loop1[1].Distance(loop1[2]) > _EPSILON)
                return nullptr;

            DVec3d vec;
            vec.CrossProduct(DVec3d::From(loop1[1]-loop1[0]), DVec3d::From(loop1[2] - loop1[0]));
            vec.Normalize();
            PK_VECTOR1_t vecp;
            vecp.coord[0] = vec.x;
            vecp.coord[1] = vec.y;
            vecp.coord[2] = vec.z;
            vectorSet.insert(VectorElem(vecp));
            ExBoxInfo adFace1 = {0,vecp,loop1[0],loop1[1],loop1[3], loop1[2]};

            vec.CrossProduct(DVec3d::From(loop2[1] - loop2[0]), DVec3d::From(loop2[2] - loop2[0]));
            vec.Normalize();
            vecp.coord[0] = vec.x;
            vecp.coord[1] = vec.y;
            vecp.coord[2] = vec.z;
            vectorSet.insert(VectorElem(vecp));
            ExBoxInfo adFace2 = {0,vecp,loop2[0],loop2[1],loop2[3], loop2[2]};

            boxInfo.push_back(adFace1);
            boxInfo.push_back(adFace2);
            }
        }
    //
    if (vectorSet.size() == 6 || boxInfo.size() < 2) //no parallel faces
        return nullptr;

    //searching for appropriate face pair
    for (auto it = boxInfo.begin(); it != boxInfo.end(); ++it)
        {
        auto it1 = it; ++it1;
        for (; it1 != boxInfo.end(); ++it1)
            {
            PK_LOGICAL_t bresult;
            // faces parallel?
            PK_VECTOR_is_parallel(it1->vec, it->vec, &bresult);
            if (bresult == PK_LOGICAL_true)
                {
                //edges parallel?
                if ((it1->point3 - it1->point1).IsParallelTo(it->point3 - it->point1) && (it1->point2 - it1->point1).IsParallelTo(it->point2 - it->point1))
                    {
                    if (nFaces == 4)
                        {
                        //there may be two missing faces at the base and other side , both or none and no other
                        bool missingFace = (it->face != 0) ^ (it1->face != 0);

                        for (auto it2 = boxInfo.begin(); it2 != boxInfo.end() && !missingFace; ++it2)
                            {
                            if (it2 != it && it2 != it1 && it2->face == 0)
                                missingFace = true;
                            }
                        if (missingFace)
                            continue;
                        //Note - only parallelogramm allowed and it was not checked for missing faces
                        if (it->face == 0)// Note it1->face == 0 as well
                            {
                            if (fabs(it1->point3.Distance(it1->point1) - it1->point4.Distance(it1->point2)) > _EPSILON)
                                continue;
                            if (fabs(it1->point2.Distance(it1->point1) - it1->point4.Distance(it1->point3)) > _EPSILON)
                                continue;
                            if (fabs(it->point3.Distance(it->point1) - it->point4.Distance(it->point2)) > _EPSILON)
                                continue;
                            if (fabs(it->point2.Distance(it->point1) - it->point4.Distance(it->point3)) > _EPSILON)
                                continue;
                            }
                        }
                    //local coord X and Y may be not perpendicular
                    auto vectorX = it->point2 - it->point1;
                    auto vectorY = it->point3 - it->point1;
                    vectorX.Normalize();
                    vectorY.Normalize();

                    double baseX = it->point1.Distance(it->point2);
                    double baseY = it->point1.Distance(it->point3);

                    double topX = it1->point1.Distance(it1->point2);
                    double topY = it1->point1.Distance(it1->point3);
                    bool bCapped = it->face != 0;

                    DgnBoxDetail boxDetail = DgnBoxDetail(it->point1, it1->point1, vectorX, vectorY, baseX, baseY, topX, topY, bCapped);
                    return ISolidPrimitive::CreateDgnBox(boxDetail);
                    }
                }
            }
        }
    return nullptr;

    }

ISolidPrimitivePtr Processor::CreateExtrude(PK_FACE_t face, PK_FACE_t face2 )
    {
    CurveVectorPtr curveVector;

    std::set<VertexElem> vertexSet1;
    std::set<VertexElem> vertexSet2;
    // Extract trimming loops.
    int n_loops;
    PK_LOOP_t *loops;
    PK_FACE_ask_loops(face, &n_loops, &loops);
    PK_MEM_Holder lholder(n_loops, loops);

    curveVector = CurveVector::Create((n_loops==1) ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_ParityRegion);

    // Process trimming loops(second time).
    for (int i = 0; i < n_loops; i++)
        {
        int nVert;
        PK_VERTEX_t *Verts;
        if (PK_ERROR_ok == PK_LOOP_ask_vertices(loops[i], &nVert, &Verts))
            {
            PK_MEM_Holder vholder(nVert, Verts);

            bvector<DPoint3d> points;
            DPoint3d prevPoint, currPoint, nextPoint;
            for (int j = 0; j < nVert; j++)
                {
                currPoint = Get3dPointFromVertex(Verts[j]);

                // Some sides of plane can be broken to several edges.
                // So try to join such edges to one.
                prevPoint = Get3dPointFromVertex(Verts[j > 0 ? j - 1 : nVert - 1]);
                nextPoint = Get3dPointFromVertex(Verts[j < nVert - 1 ? j + 1 : 0]);
                if ((currPoint - prevPoint).IsParallelTo(nextPoint - currPoint, _EPSILON))
                    continue;

                points.push_back(currPoint);
                }

            // Something went wrong.
            if (points.empty())
                return nullptr;

            // Fill vertex set.
            for (auto& point : points)
                vertexSet1.insert(point);

            // Make contour physically closed.
            if (!points[0].AlmostEqual(points[points.size() - 1], _EPSILON))
                points.push_back(points[0]);

            // TODO actually there may be bspline and other types
            auto line = ICurvePrimitive::CreateLineString(points);
            if (n_loops > 1)
                {
                PK_LOOP_type_t loopType = PK_LOOP_type_error_c;
                PK_LOOP_ask_type(loops[i], &loopType);
                CurveVector::BoundaryType boundaryType;
                if (loopType == PK_LOOP_type_outer_c)
                    boundaryType = CurveVector::BOUNDARY_TYPE_Outer;
                else if (loopType == PK_LOOP_type_inner_c)
                    boundaryType = CurveVector::BOUNDARY_TYPE_Inner;
                else
                    return nullptr;

                CurveVectorPtr loop = CurveVector::Create(boundaryType, line);
                if (loop.IsNull())
                    return nullptr;

                curveVector->push_back(ICurvePrimitive::CreateChildCurveVector_SwapFromSource(*loop));
                }
            else
                curveVector->Add(line);
            }
        }

    // Asking points of the second face to detect and check the shift.
    PK_FACE_ask_loops(face2, &n_loops, &loops);
    PK_MEM_Holder lholder2(n_loops, loops);

    for (int i = 0; i < n_loops; i++)
        {
        int nVert;
        PK_VERTEX_t *Verts;
        if (PK_ERROR_ok == PK_LOOP_ask_vertices(loops[i], &nVert, &Verts))
            {
            PK_MEM_Holder vholder(nVert, Verts);

            bvector<DPoint3d> points;
            DPoint3d prevPoint, currPoint, nextPoint;
            for (int j = 0; j < nVert; j++)
                {
                currPoint = Get3dPointFromVertex(Verts[j]);

                // Some sides of plane can be broken to several edges.
                // So try to join such edges to one.
                prevPoint = Get3dPointFromVertex(Verts[j > 0 ? j - 1 : nVert - 1]);
                nextPoint = Get3dPointFromVertex(Verts[j < nVert - 1 ? j + 1 : 0]);
                if ((currPoint - prevPoint).IsParallelTo(nextPoint - currPoint, _EPSILON))
                    continue;

                points.push_back(currPoint);
                }

            // Something went wrong.
            if (points.empty())
                return nullptr;

            // Fill vertex set.
            for (auto& point : points)
                vertexSet2.insert(point);
            }
        }

    std::set<VertexElem> vectorSet; //Intentionally using the vertex sorting algorithm, because the shift vectors are not 1-based.

    auto pit1 = vertexSet1.begin();
    auto pit2 = vertexSet2.begin();
    while (pit1 != vertexSet1.end())
        {
        vectorSet.insert(pit2->position - pit1->position);
        pit1++; pit2++;
        }

    // All the points must be moved the same way.
    if (vectorSet.size() != 1)
        {
        return nullptr;
        }

    DgnExtrusionDetail exDetail = DgnExtrusionDetail(curveVector, DVec3d::From(vectorSet.begin()->position),true);
    return ISolidPrimitive::CreateDgnExtrusion(exDetail);
    }

ISolidPrimitivePtr Processor::ParasolidPipeToDgnRuledSweep(PK_BODY_t bodyTag)
    {
    int n_faces;
    int capsCount = 0;
    PK_FACE_t* faces = nullptr;
    PK_BODY_ask_faces(bodyTag, &n_faces, &faces);
    PK_MEM_Holder fholder(n_faces, faces);

    if (n_faces != 4)
        return nullptr;

    PK_CLASS_t bodyClass;
    PK_ENTITY_ask_class(bodyTag, &bodyClass);
    bvector<ChiseledConeInfo> pipes;
    ChiseledConeInfo caps[2];

    for (int iFace = 0; iFace < n_faces; iFace++)
        {
        // Extract surface from face.
        PK_SURF_t surf;
        PK_FACE_ask_surf(faces[iFace], &surf);

        // Extract class of surface entity.
        PK_CLASS_t surfaceClass;
        PK_ENTITY_ask_class(surf, &surfaceClass);

        // Extract trimming loops.
        int n_loops;
        PK_LOOP_t *loops;
        PK_FACE_ask_loops(faces[iFace], &n_loops, &loops);
        PK_MEM_Holder lholder(n_loops, loops);

        switch (surfaceClass)
            {
                case PK_CLASS_plane: // 4001 plane
                    if (n_loops == 2)
                        {
                        if (capsCount < 2)
                            {
                            caps[capsCount].ends[0] = loops[0];
                            caps[capsCount].ends[1] = loops[1];
                            capsCount++;
                            }
                        else
                            return nullptr; // 3-rd face is a plane. It's not a cones sequence.
                        }
                    else
                        return nullptr; // It's not an elliptical cap.
                    break;
                case PK_CLASS_cyl: // 4002 cylinder
                case PK_CLASS_cone: // 4003 cone
                    // Process trimming loops.
                    if (n_loops == 2)
                        {
                        ChiseledConeInfo pipe;
                        pipe.ends[0] = loops[0];
                        pipe.ends[1] = loops[1];
                        pipes.push_back(pipe);
                        }
                    else
                        return nullptr; // It's not a cones sequence.
                    break;
                default:
                    return nullptr; // It's not a cones sequence.
            }
        }

    // Should be 2 caps.
    if (capsCount != 2)
        return nullptr;

    // Should be 2 cones.
    if (pipes.size() != 2)
        return nullptr;

    // Check that chiseled cones are valid (both ends are ellipses).
    for (int i = 0; i < pipes.size(); i++)
        if (!pipes[i].Validate(_EPSILON, 3.0e-3, _ROUGH_EPSILON))
            return nullptr;

    // Select pair of ellipses (with same centers) for each pipe end.
    // Bigger (outer) ellipse in pair for end should be placed first.
    ChiseledConeInfo pipeEnd1;
    ChiseledConeInfo pipeEnd2;
    bool found = false;
    for (int i = 0; i < 2 && !found; i++)
        for (int j = 0; j < 2 && !found; j++)
            {
            if (pipes[0].ellipses[i].center.AlmostEqual(pipes[1].ellipses[j].center, _EPSILON) &&
                pipes[0].ellipses[1 - i].center.AlmostEqual(pipes[1].ellipses[1 - j].center, _EPSILON))
                {
                pipeEnd1.Fill(pipes[0].ends[i], pipes[0].ellipses[i], pipes[1].ends[j], pipes[1].ellipses[j], true);
                pipeEnd1.SortByEllipses(false);
                pipeEnd2.Fill(pipes[0].ends[1 - i], pipes[0].ellipses[1 - i], pipes[1].ends[1 - j], pipes[1].ellipses[1 - j], true);
                pipeEnd2.SortByEllipses(false);
                found = true;
                }
            }

    // If pipe ends not found, return.
    if (!found)
        return nullptr;

    // Check that caps are really caps.
    // Firstly check by loops to avoid annecessary calculations.
    bool checked = false;
    if ((pipeEnd1.EqualsByLoop(caps[0]) && pipeEnd2.EqualsByLoop(caps[1])) ||
        (pipeEnd1.EqualsByLoop(caps[1]) && pipeEnd2.EqualsByLoop(caps[0])))
        checked = true;

    // If loops are different, try to check by ellipses.
    if (!checked)
        {
        if (!caps[0].Validate(0.01, 3.0e-3, _ROUGH_EPSILON) || !caps[1].Validate(0.01, 3.0e-3, _ROUGH_EPSILON))
            return nullptr;

        if ((pipeEnd1.EqualsByEllipses(caps[0], 0.01) && pipeEnd2.EqualsByEllipses(caps[1], 0.01)) ||
            (pipeEnd1.EqualsByEllipses(caps[1], 0.01) && pipeEnd2.EqualsByEllipses(caps[0], 0.01)))
            checked = true;
        }

    // Planes are not a caps, so return.
    if (!checked)
        return nullptr;

    auto profile = bvector<ChiseledConeInfo> {pipeEnd1,pipeEnd2};
    return ConstructChiseledPipesFromSectionsWithUnrelatedEllipseDefinitions(profile, true);
    }

// Checks that Parasolid face is a plane.
bool Processor::FaceIsPlane(PK_FACE_t face)
    {
    bool br = false;
    PK_SURF_t  surface = PK_ENTITY_null;
    PK_FACE_ask_surf(face, &surface);

    if (PK_ENTITY_null != surface)
        {
        PK_CLASS_t surfaceClass = PK_ENTITY_null;
        PK_ENTITY_ask_class(surface, &surfaceClass);
        br = PK_CLASS_plane == surfaceClass;
        }
    return br;
    }

// Extracts 3d plane from Parasolid face.
bool Processor::FaceGetPlane(PK_FACE_t face, DVec3d& normal, DPlane3d& plane)
    {
    bool br = false;

    PK_SURF_t       surfaceTag = 0;
    PK_LOGICAL_t    orientation;

    PK_FACE_ask_oriented_surf(face, &surfaceTag, &orientation);

    PK_UVBOX_t      uvBox;

    if (!surfaceTag || SUCCESS != PK_FACE_find_uvbox(face, &uvBox))
        return br;

    PK_UV_t         uv;

    uv.param[0] = (uvBox.param[2] + uvBox.param[0]) / 2.0;
    uv.param[1] = (uvBox.param[3] + uvBox.param[1]) / 2.0;

    PK_LOGICAL_t    triangular = PK_LOGICAL_false;
    PK_VECTOR_t     psPoint, psNormal;

    if (SUCCESS != PK_SURF_eval_with_normal(surfaceTag, uv, 0, 0, triangular, &psPoint, &psNormal))
        return br;

    normal = DVec3d::From(psNormal.coord[0], psNormal.coord[1], psNormal.coord[2]);

    if (orientation == PK_LOGICAL_false)
        normal.Negate();

    DPoint3d origin = DPoint3d::From(psPoint.coord[0], psPoint.coord[1], psPoint.coord[2]);
    plane = DPlane3d::FromOriginAndNormal(origin, normal);
    br = true;

    return br;
    }

// Extracts 3d point from Parasolid vertex.
DPoint3d Processor::VertexGetCoordinates(PK_VERTEX_t vertex)
    {
    PK_POINT_t point;
    PK_VERTEX_ask_point(vertex, &point);
    PK_POINT_sf_t spoint;
    PK_POINT_ask(point, &spoint);
    return P(spoint.position);
    }

// Checks that edge is just a line.
bool Processor::EdgeIsLine(PK_FACE_t face, PK_EDGE_t edge)
    {
    PK_CURVE_t edgeCurve;
    PK_CLASS_t edgeCurveClass;
    PK_VECTOR_t edgeCurveStartEnd[2];
    PK_INTERVAL_t edgeCurveInterval;
    PK_LOGICAL_t  edgeCurveSense;

    if (SUCCESS != GetEdgeCurve(edge, face, &edgeCurve, &edgeCurveClass, edgeCurveStartEnd, &edgeCurveInterval, &edgeCurveSense))
        return false;

    // Check edge curve to "line" type
    switch (edgeCurveClass)
        {
            case PK_CLASS_line:
                return true;
            case PK_CLASS_spcurve:
                {
                double length = 0;
                PK_INTERVAL_t range = {0, 0};
                PK_CURVE_find_length(edgeCurve, edgeCurveInterval, &length, &range);
                double distance = P(edgeCurveStartEnd[0]).Distance(P(edgeCurveStartEnd[1]));

                // If curve length and distance between ends are the same, it's a line.
                if (fabs(length - distance) < _EPSILON)
                    return true;
                }
                break;
        }

    return false;
    }

// Checks that edge is just a line with desired start and end points.
bool Processor::IsEdgeEqualLine(PK_FACE_t face, PK_EDGE_t edge, DPoint3d& start, DPoint3d& end)
    {
    bool br = false;
    PK_CURVE_t edgeCurve;
    PK_CLASS_t edgeCurveClass;
    PK_VECTOR_t edgeCurveStartEnd[2];
    PK_INTERVAL_t edgeCurveInterval;
    PK_LOGICAL_t  edgeCurveSense;

    if (SUCCESS != GetEdgeCurve(edge, face, &edgeCurve, &edgeCurveClass, edgeCurveStartEnd, &edgeCurveInterval, &edgeCurveSense))
        return false;

    // Check edge curve to "line" type
    switch (edgeCurveClass)
        {
        case PK_CLASS_line:
            break;
        case PK_CLASS_spcurve:
            {
            double length = 0;
            PK_INTERVAL_t range = {0, 0};
            PK_CURVE_find_length(edgeCurve, edgeCurveInterval, &length, &range);
            double distance = P(edgeCurveStartEnd[0]).Distance(P(edgeCurveStartEnd[1]));

            // If curve length and distance between ends aren't the same, it's not a line.
            if (fabs(length - distance) >= _EPSILON)
                return br;
            }
            break;
        }

    if (P(edgeCurveStartEnd[0]).IsEqual(start, _EPSILON) && P(edgeCurveStartEnd[1]).IsEqual(end, _EPSILON)
        || P(edgeCurveStartEnd[1]).IsEqual(start, _EPSILON) && P(edgeCurveStartEnd[0]).IsEqual(end, _EPSILON)
        )
        br = true;

    return br;
    }

// It's need to check this algorithms
// Calculates 3d range of Parasolid timming loop.
DRange3d Processor::LoopExtents(PK_LOOP_t loop)
    {
    //todo:
    int nVertices = 0;
    PK_VERTEX_t* vertices;
    PK_LOOP_ask_vertices(loop, &nVertices, &vertices);
    PK_MEM_Holder vholder(nVertices, vertices);

    DRange3d extents;
    extents.InitFrom(VertexGetCoordinates(vertices[0]));
    for (int i = 1; i < nVertices; i++)
        extents.Extend(VertexGetCoordinates(vertices[i]));
    return extents;
    }

// Checks that one Parasolid trimming loop contains another loop inside (by extents).
bool Processor::LoopIsInLoop(PK_LOOP_t loopCheck, PK_LOOP_t loop)
    {
    //todo:
    bool br = false;
    DRange3d loopExtents = LoopExtents(loop);
    DRange3d loopCheckExtents = LoopExtents(loopCheck);
    br = IS_CONTANED(loopCheckExtents, loopExtents);
    return br;
    }

// Extracts 3d range from component.
DRange3d Processor::GetExtents(PK_TOPOL_t topol)
    {
    DRange3d range = DRange3d::NullRange();

    PK_BOX_t box;
    if (PK_ERROR_no_errors == PK_TOPOL_find_box(topol, &box))
        {
        range.InitFrom(box.coord[0], box.coord[1], box.coord[2],
                       box.coord[3], box.coord[4], box.coord[5]);
        }

    return range;
    }

// Returns precision of the given entity (edge or vertex) or default precision if entity precision less.
double Processor::GetPrecision(int entity, double defaultPrecision)
    {
    double precision = defaultPrecision;

    // Extract entity class.
    PK_CLASS_t entityClass;
    PK_ENTITY_ask_class(entity, &entityClass);

    // Extract precision for edge or vertex.
    switch (entityClass)
        {
            case PK_CLASS_edge:
                PK_EDGE_ask_precision(entity, &precision);
                break;
            case PK_CLASS_vertex:
                PK_VERTEX_ask_precision(entity, &precision);
                break;
        }

    if (precision > 1.0 || precision < defaultPrecision)
        precision = defaultPrecision;

    return precision;
    }

// Checks that edges have the same geometry form (edge2 is a translation of edge1).
// segmentsCount is an enough control points (on curve interval) for comparison.
bool Processor::IsEdgesHaveSameForm(PK_EDGE_t edge1, PK_FACE_t face1, PK_EDGE_t edge2, PK_FACE_t face2, DVec3d translation, int segmentsCount)
    {
    // Extract curves from edges.
    PK_CURVE_t edgeCurve1, edgeCurve2;
    PK_CLASS_t edgeCurveClass1 = PK_CLASS_null, edgeCurveClass2 = PK_CLASS_null;
    PK_INTERVAL_t edgeCurveInterval1 = {0, 0}, edgeCurveInterval2 = {0, 0};
    PK_VECTOR_t edgeCurveEnds1[2], edgeCurveEnds2[2];
    PK_LOGICAL_t  edgeCurveSense1, edgeCurveSense2;
    GetEdgeCurve(edge1, face1, &edgeCurve1, &edgeCurveClass1, edgeCurveEnds1, &edgeCurveInterval1, &edgeCurveSense1);
    GetEdgeCurve(edge2, face2, &edgeCurve2, &edgeCurveClass2, edgeCurveEnds2, &edgeCurveInterval2, &edgeCurveSense2);

    // Extract edges precision and get max of them.
    double precision = std::max(GetPrecision(edge1), GetPrecision(edge2));

    // Translate ends of first curve.
    DPoint3d ends1[2], ends2[2];
    ends1[0] = P(edgeCurveEnds1[0]) + translation;
    ends1[1] = P(edgeCurveEnds1[1]) + translation;
    ends2[0] = P(edgeCurveEnds2[0]);
    ends2[1] = P(edgeCurveEnds2[1]);

    // Check that first curve ends match with second curve ends.
    bool ok = false;
    bool reverse = false;
    for (int i = 0; i < 2 && !ok; i++)
        for (int j = 0; j < 2 && !ok; j++)
            if (ends2[j].IsEqual(ends1[i], precision))
                if (ends2[1 - j].IsEqual(ends1[1 - i], precision))
                    {
                    ok = true;
                    reverse = i != j;
                    }

    if (!ok)
        return false;

    // Find curves lengths.
    double length1 = 0, length2 = 0;
    PK_INTERVAL_t range1 = {0, 0}, range2 = {0, 0};
    PK_CURVE_find_length(edgeCurve1, edgeCurveInterval1, &length1, &range1);
    PK_CURVE_find_length(edgeCurve2, edgeCurveInterval2, &length2, &range2);

    // Lengths of curves should be the same.
    // Vertex precision can't be less than edge precision,
    // but edge precision will be enough for comparison.
    // Multiplier 2 because edge has 2 vertices (1 at each end).
    if (fabs(length1 - length2) > precision * 2.0)
        return false;

    // If both curves are lines, no additional checkings necessary.
    double distance1 = ends1[0].Distance(ends1[1]);
    double distance2 = ends2[0].Distance(ends2[1]);
    if (fabs(length1 - distance1) < _ROUGH_EPSILON && fabs(length2 - distance2) < _ROUGH_EPSILON)
        return true;

    // If curves are not lines, approximately compare their form by points matching.
    bvector<DPoint3d> points1 = Get3dPointsFromCurve(edgeCurve1, edgeCurveInterval1, segmentsCount);
    bvector<DPoint3d> points2 = Get3dPointsFromCurve(edgeCurve2, edgeCurveInterval2, segmentsCount);
    if (reverse)
        std::reverse(points2.begin(), points2.end());

    for (int i = 0; i < segmentsCount; i++)
        {
        DPoint3d p = points1[i] + translation;
        if (!p.IsEqual(points2[i], precision))
            return false;
        }

    return true;
    }

// Checks that edges have the same geometry form (edge2 is a translation of edge1).
// segmentsCount is an enough control points (on curve interval) for comparison.
bool Processor::IsEdgesHaveSameForm(EdgeData& edge1, EdgeData& edge2, DVec3d translation, int segmentsCount)
    {
    // Extract edges precision and get max of them.
    double precision = std::max(edge1.GetPrecision(), edge2.GetPrecision());

    // Translate ends of first curve.
    DPoint3d ends1[2], ends2[2];
    ends1[0] = edge1.GetEndAt(0) + translation;
    ends1[1] = edge1.GetEndAt(1) + translation;
    ends2[0] = edge2.GetEndAt(0);
    ends2[1] = edge2.GetEndAt(1);

    // Check that first curve ends match with second curve ends.
    bool ok = false;
    bool reverse = false;
    for (int i = 0; i < 2 && !ok; i++)
        for (int j = 0; j < 2 && !ok; j++)
            if (ends2[j].IsEqual(ends1[i], precision))
                if (ends2[1 - j].IsEqual(ends1[1 - i], precision))
                    {
                    ok = true;
                    reverse = i != j;
                    }

    if (!ok)
        return false;

    // Find curves lengths.
    double length1 = edge1.GetLength();
    double length2 = edge2.GetLength();

    // Lengths of curves should be the same.
    // Vertex precision can't be less than edge precision,
    // but edge precision will be enough for comparison.
    // Multiplier 2 because edge has 2 vertices (1 at each end).
    if (fabs(length1 - length2) > precision * 2.0)
        return false;

    // If both curves are lines, no additional checkings necessary.
    double distance1 = ends1[0].Distance(ends1[1]);
    double distance2 = ends2[0].Distance(ends2[1]);
    if (fabs(length1 - distance1) < _ROUGH_EPSILON && fabs(length2 - distance2) < _ROUGH_EPSILON)
        return true;

    // If curves are not lines, approximately compare their form by points matching.
    bvector<DPoint3d> points1 = edge1.Get3dPoints(segmentsCount);
    bvector<DPoint3d> points2 = edge2.Get3dPoints(segmentsCount);
    if (reverse)
        std::reverse(points2.begin(), points2.end());

    for (int i = 0; i < segmentsCount; i++)
        {
        DPoint3d p = points1[i] + translation;
        if (!p.IsEqual(points2[i], precision))
            return false;
        }

    return true;
    }

bool Processor::IsOrthogonal(PK_VECTOR_t a, PK_VECTOR_t b)
    {
    double c = a.coord[0] * b.coord[0] +
        a.coord[1] * b.coord[1] +
        a.coord[2] * b.coord[2];

    if (fabs(c) < 1e-4)
        return true;
    else
        return false;
    }

bool Processor::IsCollinear(PK_VECTOR_t a, PK_VECTOR_t b)
    {
    double k = 0;
    if (fabs(a.coord[0]) > 1e-8 && fabs(b.coord[0]) > 1e-8)
        k = a.coord[0] / b.coord[0];
    else if (fabs(a.coord[1]) > 1e-8 && fabs(b.coord[1]) > 1e-8)
        k = a.coord[1] / b.coord[1];
    else if (fabs(a.coord[2]) > 1e-8 && fabs(b.coord[2]) > 1e-8)
        k = a.coord[2] / b.coord[2];
    else
        return false;

    if (fabs(a.coord[0] - k * b.coord[0]) < 1e-4 &&
        fabs(a.coord[1] - k * b.coord[1]) < 1e-4 &&
        fabs(a.coord[2] - k * b.coord[2]) < 1e-4)
        return true;
    else
        return false;
    }

// Calculate face complexity. Bigger value means more complex face.
// No need in very precise calculation - this is some sort of heuristics based mostly on vertices count.
int Processor::FaceComplexity(PK_FACE_t face)
    {
    int complexity = 0;

    int ne = 0;
    PK_EDGE_t * edges;
    PK_FACE_ask_edges(face, &ne, &edges);
    PK_MEM_Holder eholder(ne, edges);

    for (int i = 0; i < ne; i++)
        {
        // Extract curve from edge.
        PK_CURVE_t edgeCurve;
        PK_CLASS_t edgeCurveClass = PK_CLASS_null;
        PK_INTERVAL_t edgeCurveInterval = { 0, 0 };
        PK_VECTOR_t edgeCurveEnds[2];
        PK_LOGICAL_t  edgeCurveSense;
        GetEdgeCurve(edges[i], face, &edgeCurve, &edgeCurveClass, edgeCurveEnds, &edgeCurveInterval, &edgeCurveSense);

        switch (edgeCurveClass)
            {
            case PK_CLASS_line: // 3001 line
                {
                complexity += 2;
                }
                break;
            case PK_CLASS_circle: // 3002 circle
                {
                complexity += 3;
                }
                break;
            case PK_CLASS_ellipse: // 3003 ellipse
                {
                complexity += 4;
                }
                break;
            case PK_CLASS_bcurve: // 3005 bspline curve
                {
                PK_BCURVE_sf_t curveData;
                PK_BCURVE_ask(edgeCurve, &curveData);
                complexity = 5 + curveData.n_vertices + (curveData.is_rational == PK_LOGICAL_true ? 1 : 0);
                }
                break;
            case PK_CLASS_spcurve: // 3006 sp-curve
                {
                PK_SPCURVE_sf_t spcurveData;
                PK_SPCURVE_ask(edgeCurve, &spcurveData);
                PK_BCURVE_sf_t curveData;
                PK_BCURVE_ask(spcurveData.curve, &curveData);
                complexity = 10 + curveData.n_vertices + (curveData.is_rational == PK_LOGICAL_true ? 1 : 0);
                }
                break;
            }
        }

    return complexity;
    }

// Projects 'face1' along 'vec' to surface of 'face2'.
// Result returns in 'outFace'.
bool Processor::ProjectFace(PK_FACE_t face1, PK_FACE_t face2, PK_VECTOR1_t vec, PK_FACE_t * outFace)
    {
    PK_ERROR_code_t err = 0;
    PK_BODY_t masterFaceBody = PK_ENTITY_null;
    err = PK_FACE_make_sheet_body(1, &face1, &masterFaceBody);
    if (PK_ERROR_no_errors != err)
        return false;

    PK_BODY_ask_topology_o_t topts;
    PK_BODY_ask_topology_o_m(topts);
    int n_topols = 0;
    PK_TOPOL_t * topols = NULL;
    PK_CLASS_t * classes = NULL;
    int n_relations = 0;
    int * parents = NULL;
    int * children = NULL;
    PK_TOPOL_sense_t * senses = NULL;
    err = PK_BODY_ask_topology(masterFaceBody, &topts, &n_topols, &topols, &classes, &n_relations, &parents, &children, &senses);
    PK_MEM_Holder topolsh(n_topols, topols);
    PK_MEM_Holder classesh(n_topols, classes);
    PK_MEM_Holder parentsh(n_relations, parents);
    PK_MEM_Holder childrenh(n_relations, children);
    PK_MEM_Holder sensesh(n_relations, senses);

    std::vector<PK_CURVE_t> curves;
    std::vector<PK_LOGICAL_t> curvesenses;
    std::vector<int> curvemap;// stores corresponding edge topology index
    std::vector<PK_POINT_t> points;
    std::vector<int> pointmap;// stores corresponding vertex topology index

    // As a target surface for projection we will use surface of 'face2', but
    // oriented similar 'face1' relatevely projection vector 'vec' (in the same direction as 'face1').
    // It is needed bacause we are going to use this projection as the second section for ruled sweep
    PK_SURF_t surf2;
    PK_LOGICAL_t surf2sense;
    PK_FACE_ask_oriented_surf(face2, &surf2, &surf2sense);
    PK_PLANE_sf_t planeData2;
    PK_PLANE_ask(surf2, &planeData2);

    PK_SURF_t surf1;
    PK_LOGICAL_t surf1sense;
    PK_FACE_ask_oriented_surf(face1, &surf1, &surf1sense);
    PK_PLANE_sf_t planeData1;
    PK_PLANE_ask(surf1, &planeData1);

    double scalar1 = planeData1.basis_set.axis.coord[0] * vec.coord[0] +
        planeData1.basis_set.axis.coord[1] * vec.coord[1] +
        planeData1.basis_set.axis.coord[2] * vec.coord[2];
    double scalar2 = planeData2.basis_set.axis.coord[0] * vec.coord[0] +
        planeData2.basis_set.axis.coord[1] * vec.coord[1] +
        planeData2.basis_set.axis.coord[2] * vec.coord[2];

    if (scalar1 * scalar2 < 0)
        {
        planeData2.basis_set.axis.coord[0] = -planeData2.basis_set.axis.coord[0];
        planeData2.basis_set.axis.coord[1] = -planeData2.basis_set.axis.coord[1];
        planeData2.basis_set.axis.coord[2] = -planeData2.basis_set.axis.coord[2];
        }

    PK_SURF_t targetSurf;
    PK_LOGICAL_t targetSurfSense = surf1sense;// since we will use 'face1' topology, we should use the same sense
    int masterFaceTopol = 0;
    err = PK_PLANE_create(&planeData2, &targetSurf);


    for (int i = 0; i < n_topols; i++)
        {
        PK_CLASS_t cl = classes[i];
        if (PK_CLASS_face == cl)
            {
            // there are should be only one face in 'masterFaceBody'
            masterFaceTopol = i;
            }
        else if (PK_CLASS_edge == cl)
            {
            // get curve and find its projection to 'surf'
            PK_EDGE_t ed = topols[i];

            PK_CURVE_t masterCurve = PK_ENTITY_null;
            PK_INTERVAL_t masterInterval = { 0, 1 };
            PK_CLASS_t masterCurveClass;
            PK_VECTOR_t ends[2];
            PK_LOGICAL_t sense;
            err = PK_EDGE_ask_geometry(ed, PK_LOGICAL_true, &masterCurve, &masterCurveClass, ends, &masterInterval, &sense);

            PK_CURVE_project_r_t prt;
            PK_CURVE_project_o_t popts;
            PK_CURVE_project_o_m(popts);

            popts.have_direction = PK_LOGICAL_true;
            popts.direction = vec;

            PK_ENTITY_track_r_t track;
            err = PK_CURVE_project(1, &masterCurve, &masterInterval, 1, &targetSurf, &popts, &prt, &track);
            if (PK_ERROR_no_errors == err && 1 == prt.n_geoms)
                {
                curves.push_back(prt.geoms[0].geom);
                curvesenses.push_back(sense);
                curvemap.push_back(i);

                PK_CURVE_project_r_f(&prt);
                PK_ENTITY_track_r_f(&track);
                }
            else
                return false;
            }
        else if (PK_CLASS_vertex == cl)
            {
            // get point and find its projection
            PK_POINT_sf_t pnt_data;
            PK_POINT_t pnt;

            err = PK_VERTEX_ask_point(topols[i], &pnt);
            err = PK_POINT_ask(pnt, &pnt_data);
            PK_LINE_t line;
            PK_LINE_sf_t line_data;
            line_data.basis_set.location = pnt_data.position;
            line_data.basis_set.axis = vec;
            err = PK_LINE_create(&line_data, &line);

            PK_SURF_intersect_curve_o_t iopts;
            PK_SURF_intersect_curve_o_m(iopts);
            PK_INTERVAL_t interval = { 0, 10000.0 };
            int ni = 0;
            PK_VECTOR_t * intersections = NULL;
            PK_UV_t * uvs = NULL;
            double * ts = NULL;
            PK_intersect_vector_t * vecs = NULL;
            err = PK_SURF_intersect_curve(targetSurf, line, interval, &iopts, &ni, &intersections, &uvs, &ts, &vecs);
            PK_MEM_Holder ih(ni, intersections);
            PK_MEM_Holder uvh(ni, uvs);
            PK_MEM_Holder th(ni, ts);
            PK_MEM_Holder vh(ni, vecs);

            if (PK_ERROR_no_errors == err && 1 == ni)
                {
                pnt_data.position = intersections[0];
                PK_POINT_t p = PK_ENTITY_null;
                err = PK_POINT_create(&pnt_data, &p);
                points.push_back(p);
                pointmap.push_back(i);
                }
            else
                return false;
            }
        }

    //create new body
    PK_BODY_create_topology_2_o_t options;
    PK_BODY_create_topology_2_o_m(options);

    PK_BODY_create_topology_2_r_t const_creation;

    // 1. create topology
    err = PK_BODY_create_topology_2(n_topols, classes, n_relations, parents, children, senses, &options, &const_creation);

    if (PK_ERROR_no_errors == err &&
        1 == const_creation.n_create_faults &&
        PK_BODY_state_ok_c == const_creation.create_faults[0].state)
        {
        // 2. attach projected geometry

        for (int i = 0; i < points.size(); i++)
            {
            int topol_ind = pointmap[i];

            // we may have deviations between projected vertices and projected curve's start/end points
            // so let's set precision to avoid PK_ERROR_bad_vertex on further attaching curve
            err = PK_VERTEX_set_precision(const_creation.topols[topol_ind], 1e-4);
            if (PK_ERROR_no_errors != err)
                return false;

            err = PK_VERTEX_attach_points(1, &const_creation.topols[topol_ind], &points[i]);
            if (PK_ERROR_no_errors != err)
                return false;
            }

        PK_EDGE_attach_curves_o_t options_attach;
        PK_EDGE_attach_curves_o_m(options_attach);
        options_attach.have_senses = PK_LOGICAL_true;

        PK_ENTITY_track_r_t tracking;

        for (int i = 0; i < curves.size(); i++)
            {
            options_attach.senses = &curvesenses[i];
            int topol_ind = curvemap[i];
            err = PK_EDGE_attach_curves_2(1, &const_creation.topols[topol_ind], &curves[i], &options_attach, &tracking);
            if (PK_ERROR_no_errors != err)
                return false;

            PK_ENTITY_track_r_f(&tracking);
            }

        err = PK_FACE_attach_surfs(1, &const_creation.topols[masterFaceTopol], &targetSurf, &targetSurfSense);
        if (PK_ERROR_no_errors != err)
            return false;
        }


    PK_BODY_t resb = const_creation.topols[0];

    // Free the memory associated with the return structure
    PK_BODY_create_topology_2_r_f(&const_creation);

    int n_geoms = 0;
    PK_GEOM_t * geoms = NULL;
    // we call simplification here to transform intersection curves to b-curves
    err = PK_BODY_simplify_geom(resb, PK_LOGICAL_true, &n_geoms, &geoms);
    PK_MEM_Holder gh(n_geoms, geoms);


    int n_f = 0;
    PK_FACE_t * ff = NULL;
    err = PK_BODY_ask_faces(resb, &n_f, &ff);
    PK_MEM_Holder fh(n_f, ff);

    if (PK_ERROR_no_errors == err && 1 == n_f)
        {
        *outFace = *ff;
        return true;
        }
    else
        return false;
    }

// Checks if all faces except face1 / face2 are "vertical" (go along vector 'vec' from face1 to face2)
bool Processor::AreFacesConnectedAlongVector(PK_FACE_t* pFaceTagArray, int faceCount, PK_FACE_t face1, PK_FACE_t face2, PK_VECTOR1_t vec)
    {
    PK_ERROR_t err = 0;

    for (int i = 0; i < faceCount; i++)
        {
        PK_FACE_t f = pFaceTagArray[i];
        if (f == face1 || f == face2)
            continue;

        PK_SURF_t s;
        PK_FACE_ask_surf(f, &s);

        PK_CLASS_t cl;
        PK_ENTITY_ask_class(s, &cl);

        if (PK_CLASS_plane == cl)
            {
            PK_PLANE_sf_t planeData;
            err = PK_PLANE_ask(s, &planeData);

            if (PK_ERROR_no_errors != err || !IsOrthogonal(planeData.basis_set.axis, vec))
                return false;
            }
        else if (PK_CLASS_cyl == cl)
            {
            PK_CYL_sf_t cylData;
            err = PK_CYL_ask(s, &cylData);

            if (PK_ERROR_no_errors != err || !IsCollinear(cylData.basis_set.axis, vec))
                return false;
            }
        else if (PK_CLASS_cone == cl)
            {
            PK_CONE_sf_t coneData;
            err = PK_CONE_ask(s, &coneData);

            if (PK_ERROR_no_errors != err || !IsCollinear(coneData.basis_set.axis, vec) || fabs(coneData.semi_angle) > _EPSILON)
                return false;
            }
        else if (PK_CLASS_spun == cl)
            {
            PK_SPUN_sf_t spunData;
            err = PK_SPUN_ask(s, &spunData);

            if (PK_ERROR_no_errors != err)
                return false;

            //check if curve represents line
            PK_INTERVAL_t interval, int2;
            PK_CURVE_ask_interval(spunData.curve, &interval);

            PK_VECTOR_t pnts[2];
            double len;
            PK_CURVE_eval(spunData.curve, interval.value[0], 0, &pnts[0]);
            PK_CURVE_eval(spunData.curve, interval.value[1], 0, &pnts[1]);
            PK_CURVE_find_length(spunData.curve, interval, &len, &int2);

            PK_VECTOR_t v;
            v.coord[0] = pnts[1].coord[0] - pnts[0].coord[0];
            v.coord[1] = pnts[1].coord[1] - pnts[0].coord[1];
            v.coord[2] = pnts[1].coord[2] - pnts[0].coord[2];

            if (fabs(len - V(v).Magnitude()) > 1e-4)
                return false;

            if (!IsCollinear(spunData.axis.axis, vec) || !IsCollinear(v, vec))
                return false;
            }
        else
            return false;
        }

    return true;
    }

// Checks if face1/face2 are opposite sides of skewed extrusion (ruled sweep).
// Returns projection of face1 (projectedFace) to be used as the second section for DgnRuledSweep
bool Processor::IsSolidBodySkewedExtrusion(PK_BODY_t bodyTag, PK_FACE_t face1, DRange3d *face1Range, PK_FACE_t face2, DRange3d *face2Range, PK_FACE_t* pFaceTagArray, int faceCount, PK_FACE_t * projectedFace)
    {
    PK_ERROR_code_t err = 0;

    DVec3d center1 = (DVec3d::From(face1Range->low) + DVec3d::From(face1Range->high)) * 0.5;
    DVec3d center2 = (DVec3d::From(face2Range->low) + DVec3d::From(face2Range->high)) * 0.5;

    // 1. find "extrusion" vector
    DVec3d extrusionVector = DVec3d::FromStartEndNormalize(center1, center2);
    PK_VECTOR1_t extVec;
    extVec.coord[0] = extrusionVector.x;
    extVec.coord[1] = extrusionVector.y;
    extVec.coord[2] = extrusionVector.z;

    // 2. check if all faces except face1/face2 are "vertical"
    if (!AreFacesConnectedAlongVector(pFaceTagArray, faceCount, face1, face2, extVec))
        return false;

    // Here is explanation why we project 'face1' instead of working with the opposite face.
    // We want to construct DgnRuledSweep and want to found the second face (for 'face1') to use it as the second section.
    // Sections of DgnRuledSweep should be fully identical in terms of topology because these sections
    // will be connected correspondingly : first-with-first, second-with-second etc.
    // If we will use 'face2' as a second section for 'face1' there will be some problems:
    //     a. face2 is reverse for face1
    //     b. there can be different loops order
    //     c. there can be different first edges for opposite loops
    // That is why if we will want to use 'face2' geometry we will need to find corresponding parts and re-construct topology.
    // So it seems to be easier to use copy of 'face1' with projected geometry.
    // The only known "side-effect" for this: circular/elliptical arcs will be projected as b-spline curves.
    // Usually they will be recognized back to arcs in TryConvertBSplineToArc method, but not all of them for current moment.

    PK_FACE_t outFace = PK_ENTITY_null;
    if (ProjectFace(face1, face2, extVec, &outFace))
        {
        PK_FACE_is_coincident_o_t coincopts;
        PK_FACE_is_coincident_o_m(coincopts);
        PK_FACE_coi_t coit;
        PK_VECTOR_t vec;
        // Check for coincident is redundant because if solid body has all other faces are "vertical" (go along extrusion vector)
        // it is seems to be enough for these two faces are reverse coincident (otherwise not all faces will be "vertical").
        // But we use this to be sure we've constructed proper face.
        err = PK_FACE_is_coincident(face2, outFace, 1e-4, &coincopts, &coit, &vec);
        if (PK_ERROR_no_errors == err && PK_FACE_coi_yes_reversed_c == coit)
            {
            *projectedFace = outFace;
            return true;
            }
        }

    return false;
    }

// Checks if face1/face2 are opposite sides of extrusion.
// This method is only for solid bodies. Because for sheet body it is possible it will have same holes from opposite sides,
// but no "vertical" face between them - this case will be wrongly detected
bool Processor::IsSolidBodyExtrusion(PK_BODY_t bodyTag, PK_FACE_t face1, DRange3d *face1Range, PK_FACE_t face2, DRange3d *face2Range, PK_FACE_t*  pFaceTagArray, int faceCount, DVec3d& extrusionVector)
    {
    DVec3d center1 = (DVec3d::From(face1Range->low) + DVec3d::From(face1Range->high)) * 0.5;
    DVec3d center2 = (DVec3d::From(face2Range->low) + DVec3d::From(face2Range->high)) * 0.5;

    extrusionVector = DVec3d::FromStartEnd(center1, center2);

    //1. check if faces are coincident
    PK_VECTOR1_t extVec;
    extVec.coord[0] = extrusionVector.x;
    extVec.coord[1] = extrusionVector.y;
    extVec.coord[2] = extrusionVector.z;
    PK_TRANSF_t tr;
    PK_ERROR_code_t err = PK_TRANSF_create_translation(extVec, &tr);
    if (PK_ERROR_no_errors != err)
        return false;

    PK_FACE_is_coincident_o_t copts;
    PK_FACE_is_coincident_o_m(copts);
    copts.transf1 = tr;
    copts.transf2 = PK_ENTITY_null;

    PK_FACE_coi_t coit;
    PK_VECTOR_t v;
    err = PK_FACE_is_coincident(face1, face2, 1e-4, &copts, &coit, &v);
    if (PK_ERROR_no_errors != err || PK_FACE_coi_yes_reversed_c != coit)
        return false;

    //2. check if all other faces (except face1/face2) are "vertical"
    if (!AreFacesConnectedAlongVector(pFaceTagArray, faceCount, face1, face2, extVec))
        return false;

    return true;
    }

// Checks that 2 faces are opposite sides of extrusion.
bool Processor::FacesAreOppositeEx(EdgesHolder& edgesHolder, PK_FACE_t face1, PK_FACE_t face2, PK_FACE_t*  pFaceTagArray, int faceCount, DVec3d& extrusionVector)
    {
    bool br = false;
    extrusionVector = DVec3d::FromZero();

    int nVertex1 = 0;
    PK_VERTEX_t* pVertices1 = NULL;
    if (SUCCESS != PK_FACE_ask_vertices(face1, &nVertex1, nullptr) || 0 == nVertex1)
        return br;

    int nVertex2 = 0;
    PK_VERTEX_t* pVertices2 = NULL;
    if (SUCCESS != PK_FACE_ask_vertices(face2, &nVertex2, nullptr) || 0 == nVertex2)
        return br;

    if (nVertex1 != nVertex2)
        return br;

    DPlane3d plane1;
    DVec3d normal1;
    if (!FaceGetPlane(face1, normal1, plane1))
        return br;

    DPlane3d plane2;
    DVec3d normal2;
    if (!FaceGetPlane(face2, normal2, plane2))
        return br;

    if (!normal1.IsParallelTo(normal2))
        return br;

    int nLoops1 = 0;
    PK_LOOP_t *loops1;
    if (SUCCESS != PK_FACE_ask_loops(face1, &nLoops1, &loops1) || 0 == nLoops1)
        return br;
    PK_MEM_Holder lholder1(nLoops1, loops1);

    int nLoops2 = 0;
    PK_LOOP_t *loops2;
    if (SUCCESS != PK_FACE_ask_loops(face2, &nLoops2, &loops2) || 0 == nLoops2 || nLoops1 != nLoops2)
        return br;
    PK_MEM_Holder lholder2(nLoops2, loops2);

    // Find extrusion vector by faces planes.
    DPoint3d projPoint;
    plane1.ProjectPoint(projPoint, plane2.origin);
    extrusionVector = DVec3d::From(plane2.origin - projPoint);

    // Collect number of edges for each face (except extrude candidates).
    // All edges for all faces should be processed.
    std::map<PK_FACE_t, int> mapFacesEdges;
    for (int i = 0; i < faceCount; i++)
        {
        if (face1 == pFaceTagArray[i] || face2 == pFaceTagArray[i])
            continue;

        int nEdges;
        PK_FACE_ask_edges(pFaceTagArray[i], &nEdges, NULL);
        mapFacesEdges[pFaceTagArray[i]] = nEdges;
        }

    // Iterate through all face2 loops and try to project them to face1.
    // All 'vertical' faces (between face1 and face2) should be presented.
    // Find loops, that can be projected to each other (by one of vertices).
    // For planes all 2D loops are either PK_LOOP_type_outer_c or PK_LOOP_type_inner_c.
    // Each face has one outer peripheral loop(PK_LOOP_type_outer_c) and any number of holes (PK_LOOP_type_inner_c).
    PK_LOOP_t loop1 = PK_ENTITY_null;
    PK_LOOP_t loop2 = PK_ENTITY_null;
    for (int il2 = 0; il2 < nLoops2; il2++)
        {
        bool loopFound = false;
        loop2 = loops2[il2];
        PK_LOOP_type_t type2;
        PK_LOOP_ask_type(loop2, &type2);
        for (int il1 = 0; il1 < nLoops1 && !loopFound; il1++)
            {
            loop1 = loops1[il1];
            PK_LOOP_type_t type1;
            PK_LOOP_ask_type(loop1, &type1);

            // Loop types should be the same.
            if (type2 != type1)
                continue;

            PK_LOOP_ask_vertices(loop1, &nVertex1, &pVertices1); PK_MEM_Holder vholder3(nVertex1, pVertices1);
            PK_LOOP_ask_vertices(loop2, &nVertex2, &pVertices2); PK_MEM_Holder vholder4(nVertex2, pVertices2);

            // Vertices count should be the same.
            if (nVertex1 != nVertex2)
                continue;

            // Try to find vertex projection.
            for (int i = 0; i < nVertex2 && !loopFound; i++)
                {
                DPoint3d v2;
                v2 = VertexGetCoordinates(pVertices2[i]);
                for (int j = 0; j < nVertex1 && !loopFound; j++)
                    {
                    DPoint3d v1;
                    v1 = VertexGetCoordinates(pVertices1[j]) + extrusionVector;
                    if (v1.IsEqual(v2, _EPSILON))
                        loopFound = true;
                    }
                }
            }

        // Potential projection not found.
        if (!loopFound)
            return false;

        // Check that edges of face2 are projections of edges of face1.
        int nEdges1, nEdges2;
        PK_EDGE_t *edges1, *edges2;
        PK_LOOP_ask_edges(loop1, &nEdges1, &edges1); PK_MEM_Holder eholder3(nEdges1, edges1);
        PK_LOOP_ask_edges(loop2, &nEdges2, &edges2); PK_MEM_Holder eholder4(nEdges2, edges2);

        // Edges count should be the same.
        if (nEdges1 != nEdges2)
            return false;

        for (int ie2 = 0; ie2 < nEdges2; ie2++)
            {
            // The first entry in the array 'vertices' will be the start vertex,
            // and the second, the end vertex.If the edge is a ring edge, both entries
            // in the return array will take the value PK_ENTITY_null.
            PK_VERTEX_t vertices2[2];
            PK_EDGE_ask_vertices(edges2[ie2], vertices2);

            // TODO: process such case?
            if (vertices2[0] == PK_ENTITY_null || vertices2[1] == PK_ENTITY_null)
                return false;

            DPoint3d v2[2];
            v2[0] = VertexGetCoordinates(vertices2[0]);
            v2[1] = VertexGetCoordinates(vertices2[1]);

            // Project vertices to plane1.
            DPoint3d projPoints[2];
            plane1.ProjectPoint(projPoints[0], v2[0]);
            plane1.ProjectPoint(projPoints[1], v2[1]);

            for (int ie1 = 0; ie1 < nEdges1; ie1++)
                {
                PK_VERTEX_t vertices1[2];
                PK_EDGE_ask_vertices(edges1[ie1], vertices1);

                // TODO: process such case?
                if (vertices1[0] == PK_ENTITY_null || vertices1[1] == PK_ENTITY_null)
                    return false;

                DPoint3d v1[2];
                v1[0] = VertexGetCoordinates(vertices1[0]);
                v1[1] = VertexGetCoordinates(vertices1[1]);

                bool found = false;
                // Try to find edge projection.
                for(int i = 0; i < 2 && !found; i++) // plane1 edge
                    for (int j = 0; j < 2 && !found; j++) // projection from plane2 edge
                        {
                        if (v1[i].IsEqual(projPoints[j], _EPSILON)) // One of vertices is projected successfully.
                            {
                            // Check second vertex.
                            if(v1[1 - i].IsEqual(projPoints[1 - j], _EPSILON))
                                {
                                // Edge ends successfully projected.
                                // Check that edges have the same form.
                                if (!IsEdgesHaveSameForm(edgesHolder.GetEdgeData(edges1[ie1], face1), edgesHolder.GetEdgeData(edges2[ie2], face2), extrusionVector, 10))
                                    {
                                    // Outer contour:
                                    // If faceCount == 4, it can be a half of cylinder and 3 planes (| - schematic top view.
                                    // For such case arc and line vertices are the same. So continue searching.
                                    // For other cases it means that extrusion can't be created.
                                    if (type2 == PK_LOOP_type_outer_c && faceCount == 4)
                                        continue;
                                    else if (type2 == PK_LOOP_type_outer_c)
                                        return false;

                                    // continue search for holes.
                                    found = false;
                                    break;
                                    }

                                // Edge successfully projected.
                                // Try to find face between 'vertical' edges.
                                // If no such face, extrusion can't be created.
                                for (int iF = 0; iF < faceCount; iF++)
                                    {
                                    if (face1 == pFaceTagArray[iF] || face2 == pFaceTagArray[iF])
                                        continue;

                                    // Planar face should have only 1 loop in each case.
                                    // Also face should have only 1 loop for outer contour.
                                    // Otherwise face can have 1 or 2 loops.
                                    int nLoops = 0;
                                    PK_FACE_ask_loops(pFaceTagArray[iF], &nLoops, NULL);
                                    if ((type2 == PK_LOOP_type_outer_c && nLoops != 1) ||
                                        (FaceIsPlane(pFaceTagArray[iF]) && nLoops != 1) ||
                                        (nLoops > 2 || nLoops < 1))
                                        continue;

                                    int edgesCount = 0, iFound = 0;
                                    PK_EDGE_t* pEdges;
                                    PK_FACE_ask_edges(pFaceTagArray[iF], &edgesCount, &pEdges);
                                    PK_MEM_Holder eholder5(edgesCount, pEdges);

                                    // Additional checking for half of cylinder and 3 planes (| - schematic top view.
                                    // For non planar face and linear contour edge (and vice versa) continue searching.
                                    // It's due to the same vertices for cylinder and planar "vertical" faces.
                                    if (faceCount == 4 && (FaceIsPlane(pFaceTagArray[iF]) != edgesHolder.GetEdgeData(edges1[ie1], face1).IsLine()))
                                        continue;

                                    for (int ife = 0; ife < edgesCount; ife++)
                                        {
                                        EdgeData& edgeIfe = edgesHolder.GetEdgeData(pEdges[ife], pFaceTagArray[iF]);

                                        if (edgeIfe.IsEqualLine(v1[i], v2[j]) ||
                                            edgeIfe.IsEqualLine(v1[1 - i], v2[1 - j]) ||
                                            IsEdgesHaveSameForm(edgesHolder.GetEdgeData(edges1[ie1], face1), edgeIfe, DVec3d::FromZero(), 10) ||
                                            IsEdgesHaveSameForm(edgesHolder.GetEdgeData(edges2[ie2], face2), edgeIfe, DVec3d::FromZero(), 10))
                                            iFound++;
                                        }

                                    // Face found. All is OK.
                                    if (iFound > 1)
                                        {
                                        found = true;
                                        mapFacesEdges[pFaceTagArray[iF]] -= iFound;
                                        break;
                                        }
                                    }

                                // No 'vertical' face, extrusion can't be created.
                                if (!found)
                                    return false;
                                }
                            }
                        }
                }
            }
        }

    // Check that all 'vertical' faces are fully processed.
    for (auto const& iter : mapFacesEdges)
        if (iter.second != 0)
            return false;

    return true;
    }

// Constructs curve vector from Parasolid planar face.
CurveVectorPtr Processor::ParasolidFaceToCurveVector(PK_FACE_t face)
    {
    // Extract surface from face.
    PK_SURF_t surf;
    PK_FACE_ask_surf(face, &surf);

    // Extract class of surface entity.
    PK_CLASS_t surfaceClass;
    PK_ENTITY_ask_class(surf, &surfaceClass);

    // Face should be a plane.
    if (surfaceClass != PK_CLASS_plane) // 4001 plane
        return nullptr;

    CurveVectorPtr curveVector = nullptr;

    int nLoops = 0;
    PK_LOOP_t *loops;
    if (SUCCESS == PK_FACE_ask_loops(face, &nLoops, &loops) && nLoops > 0)
        {
        PK_MEM_Holder lholder(nLoops, loops);

        if (nLoops == 1)
            // Face has no holes.
            curveVector = GetCurveVectorFromLoop(loops[0], CurveVector::BOUNDARY_TYPE_Outer, _ROUGH_EPSILON);
        else
            {
            // If the face has several loops then it will be region with hole(s).
            int iOuterLoop = -1;
            for (int i = 0; i < nLoops; i++)
                {
                PK_LOOP_type_t loopType = PK_LOOP_type_error_c;
                PK_LOOP_ask_type(loops[i], &loopType);
                if (loopType == PK_LOOP_type_outer_c)
                    {
                    if (iOuterLoop == -1)
                        iOuterLoop = i;
                    else
                        {
                        iOuterLoop = -1;
                        break;
                        }
                    }
                else if (loopType != PK_LOOP_type_inner_c)
                    {
                    iOuterLoop = -1;
                    break;
                    }
                }
            if (iOuterLoop != -1)
                {
                CurveVectorPtr curveVectorChild = GetCurveVectorFromLoop(loops[iOuterLoop], CurveVector::BOUNDARY_TYPE_Outer, _ROUGH_EPSILON);
                if (!curveVectorChild.IsNull())
                    {
                    curveVector = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
                    curveVector->Add(ICurvePrimitive::CreateChildCurveVector_SwapFromSource(*curveVectorChild));

                    for (int i = 0; i < nLoops; i++)
                        {
                        if (i == iOuterLoop)
                            continue;

                        curveVectorChild = GetCurveVectorFromLoop(loops[i], CurveVector::BOUNDARY_TYPE_Inner, _ROUGH_EPSILON);
                        if (curveVectorChild.IsNull())
                            return nullptr;
                        curveVector->Add(ICurvePrimitive::CreateChildCurveVector_SwapFromSource(*curveVectorChild));

                        }
                    }
                }
            }
        }
    return curveVector;
    }

// Converts ellipse to b-spline.
// We use Parasolid for conversion to have compatible b-splines (because we use this
// for converting arcs for opposite b-splines which were also created by Parasolid).
// If we use platform's MSBSpline::InitFromEllipse -> it will create non-compatible b-spline with dirrefent order/poles/knots.
ICurvePrimitivePtr Processor::CreateBSplineFromEllipse(DEllipse3dCR inputArc)
    {
    DEllipse3d arc = DEllipse3d::FromMajorMinor(inputArc);//make sure vector0 is major axis

    PK_ERROR_code_t err = 0;
    PK_CURVE_t pkCurve = PK_ENTITY_null;
    PK_INTERVAL_t interval = { 0, 1 };

    bool reversed = arc.sweep < 0;
    DVec3d majAxis = arc.vector0;
    double R1 = majAxis.Normalize();
    DVec3d minAxis = arc.vector90;
    double R2 = minAxis.Normalize();
    DVec3d axis = DVec3d::FromCrossProduct(majAxis, minAxis);

    if (R1 > R2 + 1e-7)//R1 MUST be larger than R2 for ellipse
        {
        PK_ELLIPSE_sf_t ellipse;

        ellipse.basis_set.location.coord[0] = arc.center.x;
        ellipse.basis_set.location.coord[1] = arc.center.y;
        ellipse.basis_set.location.coord[2] = arc.center.z;

        ellipse.basis_set.axis.coord[0] = axis.x;
        ellipse.basis_set.axis.coord[1] = axis.y;
        ellipse.basis_set.axis.coord[2] = axis.z;

        ellipse.basis_set.ref_direction.coord[0] = majAxis.x;
        ellipse.basis_set.ref_direction.coord[1] = majAxis.y;
        ellipse.basis_set.ref_direction.coord[2] = majAxis.z;

        ellipse.R1 = R1;
        ellipse.R2 = R2;

        err = PK_ELLIPSE_create(&ellipse, &pkCurve);
        if (PK_ERROR_no_errors != err)
            return nullptr;
        }
    else if (fabs(R1 - R2) < 1e-6)
        {
        //circle
        PK_CIRCLE_sf_t circle;

        circle.basis_set.location.coord[0] = arc.center.x;
        circle.basis_set.location.coord[1] = arc.center.y;
        circle.basis_set.location.coord[2] = arc.center.z;

        circle.basis_set.axis.coord[0] = axis.x;
        circle.basis_set.axis.coord[1] = axis.y;
        circle.basis_set.axis.coord[2] = axis.z;

        circle.basis_set.ref_direction.coord[0] = majAxis.x;
        circle.basis_set.ref_direction.coord[1] = majAxis.y;
        circle.basis_set.ref_direction.coord[2] = majAxis.z;

        circle.radius = R1;

        err = PK_CIRCLE_create(&circle, &pkCurve);
        if (PK_ERROR_no_errors != err)
            return nullptr;
        }
    else
        return nullptr;

    interval = { arc.start, arc.start + arc.sweep };
    if (reversed)
        {
        interval.value[0] = arc.start + arc.sweep;
        interval.value[1] = arc.start;
        }


    PK_CURVE_make_bcurve_o_t bopts;
    PK_CURVE_make_bcurve_o_m(bopts);
    PK_CURVE_make_bcurve_t status;
    PK_BCURVE_t bcurve;
    double achieved_tol;
    PK_achieved_cont_t achieved_cont;
    err = PK_CURVE_make_bcurve_2(pkCurve, interval, &bopts, &status, &bcurve, &achieved_tol, &achieved_cont);
    if (PK_ERROR_no_errors != err)
        return nullptr;

    PK_BCURVE_ask_knots_o_t kopts;
    PK_BCURVE_ask_knots_o_m(kopts);
    int n_knots = 0;
    double * knots = NULL;
    int * multiplicities = NULL;
    err = PK_BCURVE_ask_knots(bcurve, &kopts, &n_knots, &knots, &multiplicities);
    PK_MEM_Holder kh(n_knots, knots);
    PK_MEM_Holder mh(n_knots, multiplicities);

    if (PK_ERROR_no_errors != err || 0 == n_knots)
        return nullptr;

    PK_INTERVAL_t bint = { knots[0], knots[n_knots - 1] };

    ICurvePrimitivePtr curprim = Create3dCurveByInterval(bcurve, bint, false);//"false" to avoid recognition back to ellipse
    if (curprim.IsValid() && reversed)
        curprim->ReverseCurvesInPlace();

    return curprim;
    }

// Visualizer is confusing about arc/bspline [and sometimes bspline/bspline] transition in ruled sweep:
// it shows hollow object with straight profile's lines (instead of arcs/bsplines segments).
// Workaround for this: convert arcs to bsplines.
bool Processor::MakeSectionsCompatible(CurveVectorPtr section1, CurveVectorPtr section2)
    {
    if (section1.IsNull() || section2.IsNull() || section1->size() != section2->size())
        return false;

    for (int i = 0; i < section1->size(); i++)
        {
        ICurvePrimitivePtr p1 = section1->at(i);
        ICurvePrimitivePtr p2 = section2->at(i);

        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == p1->GetCurvePrimitiveType() &&
            ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve == p2->GetCurvePrimitiveType())
            {
            DEllipse3dCP el1 = p1->GetArcCP();
            ICurvePrimitivePtr newCurve1 = CreateBSplineFromEllipse(*el1);

            if (newCurve1.IsValid())
                section1->at(i) = newCurve1;
            else
                return false;
            }
        else if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve == p1->GetCurvePrimitiveType() &&
            ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == p2->GetCurvePrimitiveType())
            {
            DEllipse3dCP el2 = p2->GetArcCP();
            ICurvePrimitivePtr newCurve2 = CreateBSplineFromEllipse(*el2);

            if (newCurve2.IsValid())
                section2->at(i) = newCurve2;
            else
                return false;
            }
        else if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == p1->GetCurvePrimitiveType() &&
            ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == p2->GetCurvePrimitiveType())
            {
            CurveVectorPtr cv1 = p1->GetChildCurveVectorP();
            CurveVectorPtr cv2 = p2->GetChildCurveVectorP();

            if (!MakeSectionsCompatible(cv1, cv2))
                return false;
            }
        else if ((ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve != p1->GetCurvePrimitiveType() && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve == p2->GetCurvePrimitiveType()) ||
                (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve == p1->GetCurvePrimitiveType() && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve != p2->GetCurvePrimitiveType()))
            {
            //other cases not supported yet.
            return false;
            }
        }


    //check if b-splines are compatible
    for (int i = 0; i < section1->size(); i++)
        {
        ICurvePrimitivePtr p1 = section1->at(i);
        ICurvePrimitivePtr p2 = section2->at(i);

        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve == p1->GetCurvePrimitiveType() &&
            ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve == p2->GetCurvePrimitiveType())
            {
            MSBsplineCurve mb1;
            p1->GetMSBsplineCurve(mb1);

            MSBsplineCurve mb2;
            p2->GetMSBsplineCurve(mb2);

            if (!MSBsplineCurve::AreCompatible(mb1, mb2))
                return false;
            }
        }

    return true;
    }

// Tries to convert Parasolid skewed extrusion to DgnRuledSweep.
ISolidPrimitivePtr Processor::ParasolidSkewedExtrusionToDgnRuledSweep(PK_BODY_t bodyTag)
    {
    ISolidPrimitivePtr skewedExtrusion = nullptr;

    //this algorithm only for solids
    PK_BODY_type_t btype;
    if (PK_ERROR_no_errors != PK_BODY_ask_type(bodyTag, &btype) || PK_BODY_type_solid_c != btype)
        return nullptr;

    // TODO: may be this check is common for whole Processor ?
    // need to deeply test...
    // solid body should be constructed only 2 regions: void + solid (so it can not be disjointed or have internal "bubbles")
    int n_regions = 0;
    PK_REGION_t * regions = NULL;
    PK_ERROR_code_t err = PK_BODY_ask_regions(bodyTag, &n_regions, &regions);
    PK_MEM_Holder rh(n_regions, regions);

    if (PK_ERROR_no_errors != err || n_regions != 2)
        return nullptr;

    // Find two opposite planar faces : the second is projection of the first one
    int faceCount = 0;
    PK_FACE_t* pFaceTagArray = NULL;

    if (SUCCESS == PK_BODY_ask_faces(bodyTag, &faceCount, &pFaceTagArray))
        {
        PK_MEM_Holder fholder(faceCount, pFaceTagArray);

        // Should be at least 4 faces.
        // 4 faces is a half of cylinder and 3 planes (| - schematic top view of such case.
        if (faceCount < 4)
            return nullptr;

        DRange3d bodyRange = GetExtents(bodyTag);
        if (bodyRange.IsNull())
            return nullptr;

        PK_FACE_t face1 = PK_ENTITY_null, face2 = PK_ENTITY_null, projectedFace = PK_ENTITY_null;
        bool bFound = false;

        for (int i = 0; i < faceCount && !bFound; i++)
            {
            face1 = pFaceTagArray[i];
            if (!FaceIsPlane(face1))
                continue;

            DRange3d face1Range = GetExtents(face1);

            for (int j = i + 1; j < faceCount && !bFound; j++)
                {
                face2 = pFaceTagArray[j];
                if (!FaceIsPlane(face2))
                    continue;

                DRange3d face2Range = GetExtents(face2);

                // Check that union of faces ranges is the body range.
                DRange3d unionRange;
                unionRange.UnionOf(face1Range, face2Range);
                if (!unionRange.IsEqual(bodyRange, 0.01))
                    continue;

                bFound = IsSolidBodySkewedExtrusion(bodyTag, face1, &face1Range, face2, &face2Range, pFaceTagArray, faceCount, &projectedFace);
                }
            }

        if (bFound)
            {
            CurveVectorPtr section1 = ParasolidFaceToCurveVector(face1);
            CurveVectorPtr section2 = ParasolidFaceToCurveVector(projectedFace);

            if (MakeSectionsCompatible(section1, section2))
                {
                DgnRuledSweepDetail ruledDetail = DgnRuledSweepDetail(section1, section2, true);
                skewedExtrusion = ISolidPrimitive::CreateDgnRuledSweep(ruledDetail);
                }
            }
        }

    return skewedExtrusion;
    }

// Tries to convert Parasolid body to DgnExtrusion.
ISolidPrimitivePtr Processor::ParasolidExtrusionToDgnExtrusion(PK_BODY_t bodyTag)
    {
    ISolidPrimitivePtr extrusion = nullptr;

    PK_BODY_type_t btype;
    if (PK_ERROR_no_errors != PK_BODY_ask_type(bodyTag, &btype))
        return nullptr;

    // TODO: may be this check is common for whole Processor ?
    // need to deeply test...
    if (PK_BODY_type_solid_c == btype)
    {
        // solid body should be constructed only 2 regions: void + solid (so it can not be disjointed or have internal "bubbles")
        int n_regions = 0;
        PK_REGION_t * regions = NULL;
        PK_ERROR_code_t err = PK_BODY_ask_regions(bodyTag, &n_regions, &regions);
        PK_MEM_Holder rh(n_regions, regions);

        if (PK_ERROR_no_errors != err || n_regions != 2)
            return nullptr;
    }

    // Find two oposite planar faces with the same geometry.
    int faceCount = 0;
    PK_FACE_t* pFaceTagArray = NULL;

    if (SUCCESS == PK_BODY_ask_faces(bodyTag, &faceCount, &pFaceTagArray))
        {
        PK_MEM_Holder fholder(faceCount, pFaceTagArray);

        // Should be at least 4 faces.
        // 4 faces is a half of cylinder and 3 planes (| - schematic top view of such case.
        if (faceCount < 4)
            return nullptr;

        DRange3d bodyRange = GetExtents(bodyTag);
        if (bodyRange.IsNull())
            return nullptr;

        PK_FACE_t face1 = PK_ENTITY_null, face2 = PK_ENTITY_null;
        DVec3d extrusionVector;
        bool bFound = false;
        EdgesHolder edgesHolder;
        for (int i = 0; i < faceCount && !bFound; i++)
            {
            face1 = pFaceTagArray[i];
            if (!FaceIsPlane(face1))
                continue;

            DRange3d face1Range = GetExtents(face1);

            for (int j = i + 1; j < faceCount && !bFound; j++)
                {
                face2 = pFaceTagArray[j];
                if (!FaceIsPlane(face2))
                    continue;

                DRange3d face2Range = GetExtents(face2);

                // Check that union of faces ranges is the body range.
                DRange3d unionRange;
                unionRange.UnionOf(face1Range, face2Range);
                if (!unionRange.IsEqual(bodyRange, 0.01))
                    continue;

                switch (btype)
                    {
                    case PK_BODY_type_solid_c:
                        {
                        bFound = IsSolidBodyExtrusion(bodyTag, face1, &face1Range, face2, &face2Range, pFaceTagArray, faceCount, extrusionVector);
                        }
                        break;
                    case PK_BODY_type_sheet_c:
                        {
                        // ideally this should be optional, because in common case it is
                        // not correct to transform sheet body => solid extrusion.
                        bFound = FacesAreOppositeEx(edgesHolder, face1, face2, pFaceTagArray, faceCount, extrusionVector);
                        }
                        break;
                    default:
                        {
                        return nullptr;// other types are not allowed for extrusion
                        }
                    }

                if (bFound)
                    {
                    // Since PK_FACE_is_coincident (see IsSolidBodyExtrusion method) can detect equivalence
                    // of faces with different set of geometry (e.g. there can be circle on 'face1'
                    // with corresponding spcurve on 'face2') - it is reasonable to choose
                    // the simpliest face for the base of extrusion
                    if (FaceComplexity(face2) < FaceComplexity(face1))
                        {
                        PK_FACE_t tmp = face1;
                        face1 = face2;
                        face2 = tmp;
                        extrusionVector.Negate();
                        }
                    }

                }
            }
        if (bFound)
            {
            // Faces i and j (face1, face2)
            // face1 will be extrusion profile - make Dgn curve vector from the face loops
            CurveVectorPtr extrusionProfile = ParasolidFaceToCurveVector(face1);
            if (!extrusionProfile.IsNull())
                {
                DgnExtrusionDetail extrusionDetail = DgnExtrusionDetail(extrusionProfile, extrusionVector, true);
                extrusion = ISolidPrimitive::CreateDgnExtrusion(extrusionDetail);
                }
            }
        }

    return extrusion;
    }

// Validates that Parasolid spun surface is DgnRotationalSweep-ready.
bool SpunInfo::Validate()
    {
    m_IsValid = false;

    if (spunEnds[0].endLoop == PK_ENTITY_null || spunEnds[1].endLoop == PK_ENTITY_null)
        return false;

    // Try to create spun profile.
    profile = Processor::Create3dCurve(spun.curve);
    if (profile.IsNull())
        return false;

    // Check that spun trimming loops are points or circles.
    DVec3d spunAxis = Processor::V(spun.axis.axis);
    for (int i = 0; i < 2; i++)
        {
        DPoint3d pt;
        spunEnds[i].endIsPoint = Processor::IsLoopJustPoint(spunEnds[i].endLoop, &pt);
        if (spunEnds[i].endIsPoint)
            {
            spunEnds[i].end = DEllipse3d::FromCenterNormalRadius(pt, spunAxis, 1e-7);
            pointedEnds++;
            }

        // If ends neither points nor circles, can't create DgnRotationalSweep.
        if (!spunEnds[i].endIsPoint && Processor::TryGetEllipseFromLoop(&spunEnds[i].end, spunEnds[i].endLoop) != SUCCESS && !spunEnds[i].end.IsCircular() && !spunEnds[i].end.IsFullEllipse())
            return false;

        // Spun end ellipses should be perpendicular to spun axis.
        if (!spunAxis.IsParallelTo(spunEnds[i].end.CrossProductOfBasisVectors()))
            return false;
        }

    // Start and end of spun profile should be on ellipses.
    DPoint3d ptProfile[2];
    profile->GetStartEnd(ptProfile[0], ptProfile[1]);
    for (int i = 0; i < 2; i++)
        {
        double minAngle, minDistanceSquared;
        DPoint3d closestPt;
        spunEnds[0].end.ClosestPointBounded(minAngle, minDistanceSquared, closestPt, ptProfile[i]);
        if (minDistanceSquared > 1e-8)
            spunEnds[1].end.ClosestPointBounded(minAngle, minDistanceSquared, closestPt, ptProfile[i]);
        else
            {
            spunEnds[0].isStart = i == 0;
            continue;
            }

        if (minDistanceSquared > 1e-8)
            return false; // Profile point not on ellipse.
        else
            spunEnds[1].isStart = i == 0;
        }

    m_IsValid = true;
    return true;
    }

// Tries to approximate endsing trimming loops to ellipses or circles.
// circleEpsilon is an allowed MinMajRatio deviation from 1.0 for circle.
// axisEpsilon is an allowed deviation (in radians) from perpendicular ellipse axes.
// planeEpsilon is an allowed deviation for planarity checking.
bool ChiseledConeInfo::Validate(double circleEpsilon, double axisEpsilon, double planeEpsilon)
    {
    m_validated = true;
    m_isValid = true;

    if (Processor::TryGetEllipseFromLoop(&ellipses[0], ends[0], circleEpsilon, axisEpsilon, planeEpsilon) != SUCCESS ||
        Processor::TryGetEllipseFromLoop(&ellipses[1], ends[1], circleEpsilon, axisEpsilon, planeEpsilon) != SUCCESS)
        m_isValid = false;

    return m_isValid;
    }

// Fills chiseled cone info by desired data and set desired valid flag.
void ChiseledConeInfo::Fill(PK_LOOP_t end1, DEllipse3d ellipse1, PK_LOOP_t end2, DEllipse3d ellipse2, bool valid)
    {
    m_validated = m_isValid = valid;
    ends[0] = end1;
    ends[1] = end2;
    ellipses[0] = ellipse1;
    ellipses[1] = ellipse2;
    }

// Sorts chiseled cone ends by ellipses length (size).
// asc - ascender (true) or descender (false) sorting.
void ChiseledConeInfo::SortByEllipses(bool asc)
    {
    if (!m_isValid)
        return;

    if ((asc && ellipses[0].ArcLength() > ellipses[1].ArcLength()) ||
        (!asc && ellipses[0].ArcLength() < ellipses[1].ArcLength()))
        {
        PK_LOOP_t loop = ends[0];
        ends[0] = ends[1];
        ends[1] = loop;

        DEllipse3d el = ellipses[0];
        ellipses[0] = ellipses[1];
        ellipses[0] = el;
        }
    }

// Checks that chiseled cones equals by their loops.
bool ChiseledConeInfo::EqualsByLoop(ChiseledConeInfo const& other)
    {
    if (!(Processor::IsLoopsSame(ends[0], other.ends[0]) || Processor::IsLoopsSame(ends[0], other.ends[1])) ||
        !(Processor::IsLoopsSame(ends[1], other.ends[0]) || Processor::IsLoopsSame(ends[1], other.ends[1])))
        return false;

    return true;
    }

// Checks that chiseled cones equals by their ellipses with desired epsilon.
bool ChiseledConeInfo::EqualsByEllipses(ChiseledConeInfo& other, double axisEpsilon)
    {
    if (!m_isValid || !other.IsValid())
        return false;

    if (!(Processor::IsEllipsesSame(ellipses[0], other.ellipses[0], axisEpsilon) || Processor::IsEllipsesSame(ellipses[0], other.ellipses[1], axisEpsilon)) ||
        !(Processor::IsEllipsesSame(ellipses[1], other.ellipses[0], axisEpsilon) || Processor::IsEllipsesSame(ellipses[1], other.ellipses[1], axisEpsilon)))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr BodyHolder::ProduceSimplifiedGeometry(SimplifiedGeometryType requestedType)
    {
    auto geometryType = DeduceGeometryType();
    if (SimplifiedGeometryType::Unknown != requestedType && requestedType != geometryType)
        return nullptr;

    if (SimplifiedGeometryType::CurveVector == geometryType)
        {
        auto curve = ToCurveVector();
        if (curve.IsValid())
            return IGeometry::Create(curve);
        }
    else
        {
        auto solid = ToSolidPrimitive();
        if (solid.IsValid())
            return IGeometry::Create(solid);
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr BodyHolder::ToCurveVector()
    {
    BeAssert(IsPlane());
    return Processor::ParasolidFaceToCurveVector(GetFaceAt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ISolidPrimitivePtr BodyHolder::ToSolidPrimitive()
    {
    ISolidPrimitivePtr result;
    if (GetNumFaces() == 1)
        {
        if (IsCylinder())
            {
            result = Processor::ParasolidChiseledConesToDgnRuledSweep(GetBody());
            if (result.IsNull())
                result = Processor::ParasolidConeToDgnRotationalSweep(GetBody());
            }
        else if (IsCone())
            result = Processor::ParasolidConeToDgnRotationalSweep(GetBody());
        else if (IsSwungSurface())
            result = Processor::ParasolidSpunToDgnRotationalSweep(GetBody());
        }
    else
        {
        if (CanBePipe())
            {
            result = Processor::ParasolidChiseledConesToDgnRuledSweep(GetBody());
            if (result.IsNull())
                result = Processor::ParasolidPipeToDgnRuledSweep(GetBody());
            }

        if (result.IsNull() && CanBeBox())
            result = Processor::ParasolidBodyToDgnBox(GetBody());

        if (result.IsNull() && CanBeQuadPyramid())
            result = Processor::ParasolidQuadPyramidToDgnRuledSweep(GetBody());

        if (result.IsNull() && CanBeExtrusion())
            {
            result = Processor::ParasolidExtrusionToDgnExtrusion(GetBody());
            if (result.IsNull())
                result = Processor::ParasolidSkewedExtrusionToDgnRuledSweep(GetBody());
            }

        if (result.IsNull() && CanBeRotationalSweep())
            {
            result = Processor::ParasolidSpunToDgnRotationalSweep(GetBody());

            if (result.IsNull())
                result = Processor::ParasolidSewedSpunsToDgnRotationalSweep(GetBody());

            if (result.IsNull())
                result = Processor::ParasolidComplexBodyToDgnRotationalSweep(GetBody());

            // This method can work instead of 3 above (ParasolidSpunToDgnRotationalSweep/ParasolidSewedSpunsToDgnRotationalSweep/ParasolidComplexBodyToDgnRotationalSweep)
            // because it contains common algorithm for detection revolution elements, but since it works only for solids (and not for hollow sheet objects)
            // we still keeping them for now.
            if (result.IsNull())
                result = Processor::ParasolidRevolutionToDgnRotationalSweep(GetBody());

            if (result.IsNull())
                result = Processor::ParasolidTorusSegmentBodyToDgnRotationalSweep(GetBody());
            }
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr PSolidSimplify::Simplify(PK_BODY_t bodyTag, bool removeRedundantFromInput, SimplifiedGeometryType requestedType)
    {
    if (removeRedundantFromInput)
        RemoveRedundantTopology(bodyTag);

    auto mark = BRepUtil::Modify::CreateRollbackMark();
    if (!removeRedundantFromInput)
        RemoveRedundantTopology(bodyTag);

    BodyHolder bodyHolder(bodyTag);
    return bodyHolder.ProduceSimplifiedGeometry(requestedType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SimplifiedGeometryType PSolidSimplify::DeduceGeometryType(PK_BODY_t bodyTag)
    {
    auto mark = BRepUtil::Modify::CreateRollbackMark();
    BodyHolder holder(bodyTag);
    return holder.DeduceGeometryType();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidSimplify::RemoveRedundantTopology(PK_BODY_t bodyTag)
    {
    PK_MARK_t markTag = PK_ENTITY_null;
    PK_MARK_create(&markTag);

    PK_TOPOL_track_r_t tracking {};
    PK_TOPOL_delete_redundant_2_o_s options;
    PK_TOPOL_delete_redundant_2_o_m(options);

    auto succeeded = (SUCCESS == PK_TOPOL_delete_redundant_2(1, &bodyTag, &options, &tracking));

    PK_TOPOL_track_r_f(&tracking);
    if (!succeeded)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);
    return succeeded ? SUCCESS : ERROR;
    }
