




// ===================================================================================

struct CGLineSegmentDetail
{
DPoint3d startPoint;
DPoint3d endPoint;
};



// ===================================================================================

struct CGCircularArcDetail
{
PlacementOriginZX placement;
double radius;
Angle startAngle;
Angle sweepAngle;
};



// ===================================================================================

struct CGDgnBoxDetail
{
DPoint3d baseOrigin;
DPoint3d topOrigin;
DVector3d vectorX;
DVector3d vectorY;
double baseX;
double baseY;
double topX;
double topY;
bool capped;
};



// ===================================================================================

struct CGDgnSphereDetail
{
DPoint3d center;
DVector3d vectorX;
DVector3d vectorZ;
double radiusXY;
double radiusZ;
Angle startLatitude;
Angle latitudeSweep;
bool capped;
};



// ===================================================================================

struct CGDgnConeDetail
{
DPoint3d centerA;
DPoint3d centerB;
DVector3d vectorX;
DVector3d vectorY;
double radiusA;
double radiusB;
bool capped;
};



// ===================================================================================

struct CGDgnTorusPipeDetail
{
DPoint3d center;
DVector3d vectorX;
DVector3d vectorY;
double majorRadius;
double minorRadius;
Angle sweepAngle;
bool capped;
};



// ===================================================================================

struct CGBlockDetail
{
PlacementOriginZX placement;
DPoint3d cornerA;
DPoint3d cornerB;
bool bSolidFlag;
};



// ===================================================================================

struct CGCircularConeDetail
{
PlacementOriginZX placement;
double height;
double radiusA;
double radiusB;
bool bSolidFlag;
};



// ===================================================================================

struct CGCircularCylinderDetail
{
PlacementOriginZX placement;
double height;
double radius;
bool bSolidFlag;
};



// ===================================================================================

struct CGCircularDiskDetail
{
PlacementOriginZX placement;
double radius;
};



// ===================================================================================

struct CGCoordinateDetail
{
DPoint3d xyz;
};



// ===================================================================================

struct CGEllipticArcDetail
{
PlacementOriginZX placement;
double radiusA;
double radiusB;
Angle startAngle;
Angle sweepAngle;
};



// ===================================================================================

struct CGEllipticDiskDetail
{
PlacementOriginZX placement;
double radiusA;
double radiusB;
};



// ===================================================================================

struct CGSingleLineTextDetail
{
PlacementOriginZX placement;
String textString;
String fontName;
double characterXSize;
double characterYSize;
int justification;
};



// ===================================================================================

struct CGSkewedConeDetail
{
PlacementOriginZX placement;
DPoint3d centerB;
double radiusA;
double radiusB;
bool bSolidFlag;
};



// ===================================================================================

struct CGSphereDetail
{
PlacementOriginZX placement;
double radius;
};



// ===================================================================================

struct CGTorusPipeDetail
{
PlacementOriginZX placement;
double radiusA;
double radiusB;
Angle startAngle;
Angle sweepAngle;
bool bSolidFlag;
};



// ===================================================================================

struct CGVectorDetail
{
DPoint3d xyz;
DVector3d vector;
};






// ===================================================================================
struct CGIndexedMeshDetail
{
bvector<DPoint3d> xyzArray;
bvector<int> coordIndexArray;
bvector<DPoint2d> uvArray;
bvector<int> paramIndexArray;
bvector<DVector3d> normalArray;
bvector<int> normalIndexArray;
bvector<DVector3d> colorArray;
bvector<int> colorIndexArray;
};



// ===================================================================================
struct CGAdjacentSurfacePatchesDetail
{
bvector<IGeometryPtr> patchArray;
};



// ===================================================================================
struct CGBsplineCurveDetail
{
int order;
bool closed;
bvector<DPoint3d> controlPointArray;
bvector<double> weightArray;
bvector<double> knotArray;
};



// ===================================================================================
struct CGBsplineSurfaceDetail
{
int orderU;
bool closedU;
int numUControlPoint;
int orderV;
bool closedV;
int numVControlPoint;
bvector<DPoint3d> controlPointArray;
bvector<double> weightArray;
bvector<double> knotUArray;
bvector<double> knotVArray;
};



// ===================================================================================
struct CGCurveChainDetail
{
bvector<ICurvePrimitivePtr> curveArray;
};



// ===================================================================================
struct CGCurveGroupDetail
{
bvector<IGeometryPtr> curveArray;
};



// ===================================================================================
struct CGCurveReferenceDetail
{
bool reversed;
IGeometryPtr parentCurve;
};



// ===================================================================================
struct CGGroupDetail
{
bvector<IGeometryPtr> memberArray;
};



// ===================================================================================
struct CGInterpolatingCurveDetail
{
int endConditionCode;
int knotCode;
DVector3d startVector;
DVector3d endVector;
bvector<DPoint3d> PointArray;
bvector<double> KnotArray;
};



// ===================================================================================
struct CGLineStringDetail
{
bvector<DPoint3d> PointArray;
};



// ===================================================================================
struct CGOperationDetail
{
String name;
bvector<IGeometryPtr> memberArray;
};



// ===================================================================================
struct CGParametricSurfacePatchDetail
{
LoopType loopType;
IGeometryPtr surface;
bvector<IGeometryPtr> loopArray;
};



// ===================================================================================
struct CGPointChainDetail
{
bvector<IGeometryPtr> PointArray;
};



// ===================================================================================
struct CGPointGroupDetail
{
bvector<IGeometryPtr> memberArray;
};



// ===================================================================================
struct CGPolygonDetail
{
bvector<DPoint3d> pointArray;
};



// ===================================================================================
struct CGPrimitiveCurveReferenceDetail
{
bool reversed;
ICurvePrimitivePtr parentCurve;
};



// ===================================================================================
struct CGSharedGroupDefDetail
{
String name;
IGeometryPtr geometry;
};



// ===================================================================================
struct CGSharedGroupInstanceDetail
{
String sharedGroupName;
Transform transform;
};



// ===================================================================================
struct CGShelledSolidDetail
{
IGeometryPtr BoundingSurface;
};



// ===================================================================================
struct CGSolidBySweptSurfaceDetail
{
IGeometryPtr baseGeometry;
IGeometryPtr railCurve;
};



// ===================================================================================
struct CGSolidByRuledSweepDetail
{
bvector<IGeometryPtr> SectionArray;
};



// ===================================================================================
struct CGSurfaceByRuledSweepDetail
{
bvector<IGeometryPtr> SectionArray;
};



// ===================================================================================
struct CGSolidGroupDetail
{
bvector<IGeometryPtr> solidArray;
};



// ===================================================================================
struct CGSpiralDetail
{
String spiralType;
DPoint3d startPoint;
Angle startBearing;
double startCurvature;
DPoint3d endPoint;
Angle endBearing;
double endCurvature;
IGeometryPtr geometry;
};



// ===================================================================================
struct CGSurfaceBySweptCurveDetail
{
IGeometryPtr baseGeometry;
IGeometryPtr railCurve;
};



// ===================================================================================
struct CGSurfaceGroupDetail
{
bvector<IGeometryPtr> surfaceArray;
};



// ===================================================================================
struct CGSurfacePatchDetail
{
IGeometryPtr exteriorLoop;
bvector<IGeometryPtr> holeLoopArray;
};



// ===================================================================================
struct CGTransformedGeometryDetail
{
Transform transform;
IGeometryPtr geometry;
};



// ===================================================================================
struct CGDgnExtrusionDetail
{
DVector3d extrusionVector;
bool capped;
CurveVectorPtr baseGeometry;
};



// ===================================================================================
struct CGDgnRotationalSweepDetail
{
DPoint3d center;
DVector3d axis;
Angle sweepAngle;
bool capped;
CurveVectorPtr baseGeometry;
};



// ===================================================================================
struct CGDgnRuledSweepDetail
{
bool capped;
bvector<CurveVectorPtr> contourArray;
};



// ===================================================================================
struct CGTransitionSpiralDetail
{
String spiralType;
PlacementOriginZX placement;
Angle startBearing;
double startRadius;
Angle endBearing;
double endRadius;
double activeStartFraction;
double activeEndFraction;
IGeometryPtr geometry;
};


