/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/




// ===================================================================================

struct CGLineSegmentDetail
{
DPoint3d startPoint;
DPoint3d endPoint;
};

struct CGLineSegmentFlags
{
CGLineSegmentFlags ()
    {
    startPoint_defined = false;
    endPoint_defined = false;
    }

bool startPoint_defined;
bool endPoint_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!startPoint_defined)
        numUndefined++;
    if (!endPoint_defined)
        numUndefined++;
    return numUndefined;
    }
};


// ===================================================================================

struct CGCircularArcDetail
{
PlacementOriginZX placement;
double radius;
Angle startAngle;
Angle sweepAngle;
};

struct CGCircularArcFlags
{
CGCircularArcFlags ()
    {
    placement_defined = false;
    radius_defined = false;
    startAngle_defined = false;
    sweepAngle_defined = false;
    }

bool placement_defined;
bool radius_defined;
bool startAngle_defined;
bool sweepAngle_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!placement_defined)
        numUndefined++;
    if (!radius_defined)
        numUndefined++;
    if (!startAngle_defined)
        numUndefined++;
    if (!sweepAngle_defined)
        numUndefined++;
    return numUndefined;
    }
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

struct CGDgnBoxFlags
{
CGDgnBoxFlags ()
    {
    baseOrigin_defined = false;
    topOrigin_defined = false;
    vectorX_defined = false;
    vectorY_defined = false;
    baseX_defined = false;
    baseY_defined = false;
    topX_defined = false;
    topY_defined = false;
    capped_defined = false;
    }

bool baseOrigin_defined;
bool topOrigin_defined;
bool vectorX_defined;
bool vectorY_defined;
bool baseX_defined;
bool baseY_defined;
bool topX_defined;
bool topY_defined;
bool capped_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!baseOrigin_defined)
        numUndefined++;
    if (!topOrigin_defined)
        numUndefined++;
    if (!vectorX_defined)
        numUndefined++;
    if (!vectorY_defined)
        numUndefined++;
    if (!baseX_defined)
        numUndefined++;
    if (!baseY_defined)
        numUndefined++;
    if (!topX_defined)
        numUndefined++;
    if (!topY_defined)
        numUndefined++;
    if (!capped_defined)
        numUndefined++;
    return numUndefined;
    }
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

struct CGDgnSphereFlags
{
CGDgnSphereFlags ()
    {
    center_defined = false;
    vectorX_defined = false;
    vectorZ_defined = false;
    radiusXY_defined = false;
    radiusZ_defined = false;
    startLatitude_defined = false;
    latitudeSweep_defined = false;
    capped_defined = false;
    }

bool center_defined;
bool vectorX_defined;
bool vectorZ_defined;
bool radiusXY_defined;
bool radiusZ_defined;
bool startLatitude_defined;
bool latitudeSweep_defined;
bool capped_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!center_defined)
        numUndefined++;
    if (!vectorX_defined)
        numUndefined++;
    if (!vectorZ_defined)
        numUndefined++;
    if (!radiusXY_defined)
        numUndefined++;
    if (!radiusZ_defined)
        numUndefined++;
    if (!startLatitude_defined)
        numUndefined++;
    if (!latitudeSweep_defined)
        numUndefined++;
    if (!capped_defined)
        numUndefined++;
    return numUndefined;
    }
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

struct CGDgnConeFlags
{
CGDgnConeFlags ()
    {
    centerA_defined = false;
    centerB_defined = false;
    vectorX_defined = false;
    vectorY_defined = false;
    radiusA_defined = false;
    radiusB_defined = false;
    capped_defined = false;
    }

bool centerA_defined;
bool centerB_defined;
bool vectorX_defined;
bool vectorY_defined;
bool radiusA_defined;
bool radiusB_defined;
bool capped_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!centerA_defined)
        numUndefined++;
    if (!centerB_defined)
        numUndefined++;
    if (!vectorX_defined)
        numUndefined++;
    if (!vectorY_defined)
        numUndefined++;
    if (!radiusA_defined)
        numUndefined++;
    if (!radiusB_defined)
        numUndefined++;
    if (!capped_defined)
        numUndefined++;
    return numUndefined;
    }
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

struct CGDgnTorusPipeFlags
{
CGDgnTorusPipeFlags ()
    {
    center_defined = false;
    vectorX_defined = false;
    vectorY_defined = false;
    majorRadius_defined = false;
    minorRadius_defined = false;
    sweepAngle_defined = false;
    capped_defined = false;
    }

bool center_defined;
bool vectorX_defined;
bool vectorY_defined;
bool majorRadius_defined;
bool minorRadius_defined;
bool sweepAngle_defined;
bool capped_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!center_defined)
        numUndefined++;
    if (!vectorX_defined)
        numUndefined++;
    if (!vectorY_defined)
        numUndefined++;
    if (!majorRadius_defined)
        numUndefined++;
    if (!minorRadius_defined)
        numUndefined++;
    if (!sweepAngle_defined)
        numUndefined++;
    if (!capped_defined)
        numUndefined++;
    return numUndefined;
    }
};


// ===================================================================================

struct CGBlockDetail
{
PlacementOriginZX placement;
DPoint3d cornerA;
DPoint3d cornerB;
bool bSolidFlag;
};

struct CGBlockFlags
{
CGBlockFlags ()
    {
    placement_defined = false;
    cornerA_defined = false;
    cornerB_defined = false;
    bSolidFlag_defined = false;
    }

bool placement_defined;
bool cornerA_defined;
bool cornerB_defined;
bool bSolidFlag_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!placement_defined)
        numUndefined++;
    if (!cornerA_defined)
        numUndefined++;
    if (!cornerB_defined)
        numUndefined++;
    if (!bSolidFlag_defined)
        numUndefined++;
    return numUndefined;
    }
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

struct CGCircularConeFlags
{
CGCircularConeFlags ()
    {
    placement_defined = false;
    height_defined = false;
    radiusA_defined = false;
    radiusB_defined = false;
    bSolidFlag_defined = false;
    }

bool placement_defined;
bool height_defined;
bool radiusA_defined;
bool radiusB_defined;
bool bSolidFlag_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!placement_defined)
        numUndefined++;
    if (!height_defined)
        numUndefined++;
    if (!radiusA_defined)
        numUndefined++;
    if (!radiusB_defined)
        numUndefined++;
    if (!bSolidFlag_defined)
        numUndefined++;
    return numUndefined;
    }
};


// ===================================================================================

struct CGCircularCylinderDetail
{
PlacementOriginZX placement;
double height;
double radius;
bool bSolidFlag;
};

struct CGCircularCylinderFlags
{
CGCircularCylinderFlags ()
    {
    placement_defined = false;
    height_defined = false;
    radius_defined = false;
    bSolidFlag_defined = false;
    }

bool placement_defined;
bool height_defined;
bool radius_defined;
bool bSolidFlag_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!placement_defined)
        numUndefined++;
    if (!height_defined)
        numUndefined++;
    if (!radius_defined)
        numUndefined++;
    if (!bSolidFlag_defined)
        numUndefined++;
    return numUndefined;
    }
};


// ===================================================================================

struct CGCircularDiskDetail
{
PlacementOriginZX placement;
double radius;
};

struct CGCircularDiskFlags
{
CGCircularDiskFlags ()
    {
    placement_defined = false;
    radius_defined = false;
    }

bool placement_defined;
bool radius_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!placement_defined)
        numUndefined++;
    if (!radius_defined)
        numUndefined++;
    return numUndefined;
    }
};


// ===================================================================================

struct CGCoordinateDetail
{
DPoint3d xyz;
};

struct CGCoordinateFlags
{
CGCoordinateFlags ()
    {
    xyz_defined = false;
    }

bool xyz_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!xyz_defined)
        numUndefined++;
    return numUndefined;
    }
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

struct CGEllipticArcFlags
{
CGEllipticArcFlags ()
    {
    placement_defined = false;
    radiusA_defined = false;
    radiusB_defined = false;
    startAngle_defined = false;
    sweepAngle_defined = false;
    }

bool placement_defined;
bool radiusA_defined;
bool radiusB_defined;
bool startAngle_defined;
bool sweepAngle_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!placement_defined)
        numUndefined++;
    if (!radiusA_defined)
        numUndefined++;
    if (!radiusB_defined)
        numUndefined++;
    if (!startAngle_defined)
        numUndefined++;
    if (!sweepAngle_defined)
        numUndefined++;
    return numUndefined;
    }
};


// ===================================================================================

struct CGEllipticDiskDetail
{
PlacementOriginZX placement;
double radiusA;
double radiusB;
};

struct CGEllipticDiskFlags
{
CGEllipticDiskFlags ()
    {
    placement_defined = false;
    radiusA_defined = false;
    radiusB_defined = false;
    }

bool placement_defined;
bool radiusA_defined;
bool radiusB_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!placement_defined)
        numUndefined++;
    if (!radiusA_defined)
        numUndefined++;
    if (!radiusB_defined)
        numUndefined++;
    return numUndefined;
    }
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

struct CGSingleLineTextFlags
{
CGSingleLineTextFlags ()
    {
    placement_defined = false;
    textString_defined = false;
    fontName_defined = false;
    characterXSize_defined = false;
    characterYSize_defined = false;
    justification_defined = false;
    }

bool placement_defined;
bool textString_defined;
bool fontName_defined;
bool characterXSize_defined;
bool characterYSize_defined;
bool justification_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!placement_defined)
        numUndefined++;
    if (!textString_defined)
        numUndefined++;
    if (!fontName_defined)
        numUndefined++;
    if (!characterXSize_defined)
        numUndefined++;
    if (!characterYSize_defined)
        numUndefined++;
    if (!justification_defined)
        numUndefined++;
    return numUndefined;
    }
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

struct CGSkewedConeFlags
{
CGSkewedConeFlags ()
    {
    placement_defined = false;
    centerB_defined = false;
    radiusA_defined = false;
    radiusB_defined = false;
    bSolidFlag_defined = false;
    }

bool placement_defined;
bool centerB_defined;
bool radiusA_defined;
bool radiusB_defined;
bool bSolidFlag_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!placement_defined)
        numUndefined++;
    if (!centerB_defined)
        numUndefined++;
    if (!radiusA_defined)
        numUndefined++;
    if (!radiusB_defined)
        numUndefined++;
    if (!bSolidFlag_defined)
        numUndefined++;
    return numUndefined;
    }
};


// ===================================================================================

struct CGSphereDetail
{
PlacementOriginZX placement;
double radius;
};

struct CGSphereFlags
{
CGSphereFlags ()
    {
    placement_defined = false;
    radius_defined = false;
    }

bool placement_defined;
bool radius_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!placement_defined)
        numUndefined++;
    if (!radius_defined)
        numUndefined++;
    return numUndefined;
    }
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

struct CGTorusPipeFlags
{
CGTorusPipeFlags ()
    {
    placement_defined = false;
    radiusA_defined = false;
    radiusB_defined = false;
    startAngle_defined = false;
    sweepAngle_defined = false;
    bSolidFlag_defined = false;
    }

bool placement_defined;
bool radiusA_defined;
bool radiusB_defined;
bool startAngle_defined;
bool sweepAngle_defined;
bool bSolidFlag_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!placement_defined)
        numUndefined++;
    if (!radiusA_defined)
        numUndefined++;
    if (!radiusB_defined)
        numUndefined++;
    if (!startAngle_defined)
        numUndefined++;
    if (!sweepAngle_defined)
        numUndefined++;
    if (!bSolidFlag_defined)
        numUndefined++;
    return numUndefined;
    }
};


// ===================================================================================

struct CGVectorDetail
{
DPoint3d xyz;
DVector3d vector;
};

struct CGVectorFlags
{
CGVectorFlags ()
    {
    xyz_defined = false;
    vector_defined = false;
    }

bool xyz_defined;
bool vector_defined;

int NumUndefined ()
    {
    int numUndefined = 0;
    
    if (!xyz_defined)
        numUndefined++;
    if (!vector_defined)
        numUndefined++;
    return numUndefined;
    }
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

struct CGIndexedMeshFlags
{
CGIndexedMeshFlags ()
    {
    xyzArray_defined = false;
    coordIndexArray_defined = false;
    uvArray_defined = false;
    paramIndexArray_defined = false;
    normalArray_defined = false;
    normalIndexArray_defined = false;
    colorArray_defined = false;
    colorIndexArray_defined = false;    
    
    }

bool xyzArray_defined;
bool coordIndexArray_defined;
bool uvArray_defined;
bool paramIndexArray_defined;
bool normalArray_defined;
bool normalIndexArray_defined;
bool colorArray_defined;
bool colorIndexArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!xyzArray_defined)
        numUndefined++;
    if (!coordIndexArray_defined)
        numUndefined++;
    if (!uvArray_defined)
        numUndefined++;
    if (!paramIndexArray_defined)
        numUndefined++;
    if (!normalArray_defined)
        numUndefined++;
    if (!normalIndexArray_defined)
        numUndefined++;
    if (!colorArray_defined)
        numUndefined++;
    if (!colorIndexArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGAdjacentSurfacePatchesDetail
{
bvector<IGeometryPtr> patchArray;
};

struct CGAdjacentSurfacePatchesFlags
{
CGAdjacentSurfacePatchesFlags ()
    {
    patchArray_defined = false;    
    
    }

bool patchArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!patchArray_defined)
        numUndefined++;  
    return numUndefined;
    }
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

struct CGBsplineCurveFlags
{
CGBsplineCurveFlags ()
    {
    order_defined = false;
    closed_defined = false;
    controlPointArray_defined = false;
    weightArray_defined = false;
    knotArray_defined = false;    
    
    }

bool order_defined;
bool closed_defined;
bool controlPointArray_defined;
bool weightArray_defined;
bool knotArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!order_defined)
        numUndefined++;
    if (!closed_defined)
        numUndefined++;
    if (!controlPointArray_defined)
        numUndefined++;
    if (!weightArray_defined)
        numUndefined++;
    if (!knotArray_defined)
        numUndefined++;  
    return numUndefined;
    }
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

struct CGBsplineSurfaceFlags
{
CGBsplineSurfaceFlags ()
    {
    orderU_defined = false;
    closedU_defined = false;
    numUControlPoint_defined = false;
    orderV_defined = false;
    closedV_defined = false;
    numVControlPoint_defined = false;
    controlPointArray_defined = false;
    weightArray_defined = false;
    knotUArray_defined = false;
    knotVArray_defined = false;    
    
    }

bool orderU_defined;
bool closedU_defined;
bool numUControlPoint_defined;
bool orderV_defined;
bool closedV_defined;
bool numVControlPoint_defined;
bool controlPointArray_defined;
bool weightArray_defined;
bool knotUArray_defined;
bool knotVArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!orderU_defined)
        numUndefined++;
    if (!closedU_defined)
        numUndefined++;
    if (!numUControlPoint_defined)
        numUndefined++;
    if (!orderV_defined)
        numUndefined++;
    if (!closedV_defined)
        numUndefined++;
    if (!numVControlPoint_defined)
        numUndefined++;
    if (!controlPointArray_defined)
        numUndefined++;
    if (!weightArray_defined)
        numUndefined++;
    if (!knotUArray_defined)
        numUndefined++;
    if (!knotVArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGCurveChainDetail
{
bvector<ICurvePrimitivePtr> curveArray;
};

struct CGCurveChainFlags
{
CGCurveChainFlags ()
    {
    curveArray_defined = false;    
    
    }

bool curveArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!curveArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGCurveGroupDetail
{
bvector<IGeometryPtr> curveArray;
};

struct CGCurveGroupFlags
{
CGCurveGroupFlags ()
    {
    curveArray_defined = false;    
    
    }

bool curveArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!curveArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGCurveReferenceDetail
{
bool reversed;
IGeometryPtr parentCurve;
};

struct CGCurveReferenceFlags
{
CGCurveReferenceFlags ()
    {
    reversed_defined = false;
    parentCurve_defined = false;    
    
    }

bool reversed_defined;
bool parentCurve_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!reversed_defined)
        numUndefined++;
    if (!parentCurve_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGGroupDetail
{
bvector<IGeometryPtr> memberArray;
};

struct CGGroupFlags
{
CGGroupFlags ()
    {
    memberArray_defined = false;    
    
    }

bool memberArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!memberArray_defined)
        numUndefined++;  
    return numUndefined;
    }
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

struct CGInterpolatingCurveFlags
{
CGInterpolatingCurveFlags ()
    {
    endConditionCode_defined = false;
    knotCode_defined = false;
    startVector_defined = false;
    endVector_defined = false;
    PointArray_defined = false;
    KnotArray_defined = false;    
    
    }

bool endConditionCode_defined;
bool knotCode_defined;
bool startVector_defined;
bool endVector_defined;
bool PointArray_defined;
bool KnotArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!endConditionCode_defined)
        numUndefined++;
    if (!knotCode_defined)
        numUndefined++;
    if (!startVector_defined)
        numUndefined++;
    if (!endVector_defined)
        numUndefined++;
    if (!PointArray_defined)
        numUndefined++;
    if (!KnotArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGLineStringDetail
{
bvector<DPoint3d> PointArray;
};

struct CGLineStringFlags
{
CGLineStringFlags ()
    {
    PointArray_defined = false;    
    
    }

bool PointArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!PointArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGOperationDetail
{
String name;
bvector<IGeometryPtr> memberArray;
};

struct CGOperationFlags
{
CGOperationFlags ()
    {
    name_defined = false;
    memberArray_defined = false;    
    
    }

bool name_defined;
bool memberArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!name_defined)
        numUndefined++;
    if (!memberArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGParametricSurfacePatchDetail
{
LoopType loopType;
IGeometryPtr surface;
bvector<IGeometryPtr> loopArray;
};

struct CGParametricSurfacePatchFlags
{
CGParametricSurfacePatchFlags ()
    {
    loopType_defined = false;
    surface_defined = false;
    loopArray_defined = false;    
    
    }

bool loopType_defined;
bool surface_defined;
bool loopArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!loopType_defined)
        numUndefined++;
    if (!surface_defined)
        numUndefined++;
    if (!loopArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGPointChainDetail
{
bvector<IGeometryPtr> PointArray;
};

struct CGPointChainFlags
{
CGPointChainFlags ()
    {
    PointArray_defined = false;    
    
    }

bool PointArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!PointArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGPointGroupDetail
{
bvector<IGeometryPtr> memberArray;
};

struct CGPointGroupFlags
{
CGPointGroupFlags ()
    {
    memberArray_defined = false;    
    
    }

bool memberArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!memberArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGPolygonDetail
{
bvector<DPoint3d> pointArray;
};

struct CGPolygonFlags
{
CGPolygonFlags ()
    {
    pointArray_defined = false;    
    
    }

bool pointArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!pointArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGPrimitiveCurveReferenceDetail
{
bool reversed;
ICurvePrimitivePtr parentCurve;
};

struct CGPrimitiveCurveReferenceFlags
{
CGPrimitiveCurveReferenceFlags ()
    {
    reversed_defined = false;
    parentCurve_defined = false;    
    
    }

bool reversed_defined;
bool parentCurve_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!reversed_defined)
        numUndefined++;
    if (!parentCurve_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGPartialCurveDetail
{
double fraction0;
double fraction1;
ICurvePrimitivePtr parentCurve;
};

struct CGPartialCurveFlags
{
CGPartialCurveFlags ()
    {
    fraction0_defined = false;
    fraction1_defined = false;
    parentCurve_defined = false;    
    
    }

bool fraction0_defined;
bool fraction1_defined;
bool parentCurve_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!fraction0_defined)
        numUndefined++;
    if (!fraction1_defined)
        numUndefined++;
    if (!parentCurve_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGSharedGroupDefDetail
{
String name;
IGeometryPtr geometry;
};

struct CGSharedGroupDefFlags
{
CGSharedGroupDefFlags ()
    {
    name_defined = false;
    geometry_defined = false;    
    
    }

bool name_defined;
bool geometry_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!name_defined)
        numUndefined++;
    if (!geometry_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGSharedGroupInstanceDetail
{
String sharedGroupName;
Transform transform;
};

struct CGSharedGroupInstanceFlags
{
CGSharedGroupInstanceFlags ()
    {
    sharedGroupName_defined = false;
    transform_defined = false;    
    
    }

bool sharedGroupName_defined;
bool transform_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!sharedGroupName_defined)
        numUndefined++;
    if (!transform_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGShelledSolidDetail
{
IGeometryPtr BoundingSurface;
};

struct CGShelledSolidFlags
{
CGShelledSolidFlags ()
    {
    BoundingSurface_defined = false;    
    
    }

bool BoundingSurface_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!BoundingSurface_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGSolidBySweptSurfaceDetail
{
IGeometryPtr baseGeometry;
IGeometryPtr railCurve;
};

struct CGSolidBySweptSurfaceFlags
{
CGSolidBySweptSurfaceFlags ()
    {
    baseGeometry_defined = false;
    railCurve_defined = false;    
    
    }

bool baseGeometry_defined;
bool railCurve_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!baseGeometry_defined)
        numUndefined++;
    if (!railCurve_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGSolidByRuledSweepDetail
{
bvector<CurveVectorPtr> SectionArray;
};

struct CGSolidByRuledSweepFlags
{
CGSolidByRuledSweepFlags ()
    {
    SectionArray_defined = false;    
    
    }

bool SectionArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!SectionArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGSurfaceByRuledSweepDetail
{
bvector<CurveVectorPtr> SectionArray;
};

struct CGSurfaceByRuledSweepFlags
{
CGSurfaceByRuledSweepFlags ()
    {
    SectionArray_defined = false;    
    
    }

bool SectionArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!SectionArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGSolidGroupDetail
{
bvector<IGeometryPtr> solidArray;
};

struct CGSolidGroupFlags
{
CGSolidGroupFlags ()
    {
    solidArray_defined = false;    
    
    }

bool solidArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!solidArray_defined)
        numUndefined++;  
    return numUndefined;
    }
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

struct CGSpiralFlags
{
CGSpiralFlags ()
    {
    spiralType_defined = false;
    startPoint_defined = false;
    startBearing_defined = false;
    startCurvature_defined = false;
    endPoint_defined = false;
    endBearing_defined = false;
    endCurvature_defined = false;
    geometry_defined = false;    
    
    }

bool spiralType_defined;
bool startPoint_defined;
bool startBearing_defined;
bool startCurvature_defined;
bool endPoint_defined;
bool endBearing_defined;
bool endCurvature_defined;
bool geometry_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!spiralType_defined)
        numUndefined++;
    if (!startPoint_defined)
        numUndefined++;
    if (!startBearing_defined)
        numUndefined++;
    if (!startCurvature_defined)
        numUndefined++;
    if (!endPoint_defined)
        numUndefined++;
    if (!endBearing_defined)
        numUndefined++;
    if (!endCurvature_defined)
        numUndefined++;
    if (!geometry_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGSurfaceBySweptCurveDetail
{
IGeometryPtr baseGeometry;
IGeometryPtr railCurve;
};

struct CGSurfaceBySweptCurveFlags
{
CGSurfaceBySweptCurveFlags ()
    {
    baseGeometry_defined = false;
    railCurve_defined = false;    
    
    }

bool baseGeometry_defined;
bool railCurve_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!baseGeometry_defined)
        numUndefined++;
    if (!railCurve_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGSurfaceGroupDetail
{
bvector<IGeometryPtr> surfaceArray;
};

struct CGSurfaceGroupFlags
{
CGSurfaceGroupFlags ()
    {
    surfaceArray_defined = false;    
    
    }

bool surfaceArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!surfaceArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGSurfacePatchDetail
{
IGeometryPtr exteriorLoop;
bvector<IGeometryPtr> holeLoopArray;
};

struct CGSurfacePatchFlags
{
CGSurfacePatchFlags ()
    {
    exteriorLoop_defined = false;
    holeLoopArray_defined = false;    
    
    }

bool exteriorLoop_defined;
bool holeLoopArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!exteriorLoop_defined)
        numUndefined++;
    if (!holeLoopArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGTransformedGeometryDetail
{
Transform transform;
IGeometryPtr geometry;
};

struct CGTransformedGeometryFlags
{
CGTransformedGeometryFlags ()
    {
    transform_defined = false;
    geometry_defined = false;    
    
    }

bool transform_defined;
bool geometry_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!transform_defined)
        numUndefined++;
    if (!geometry_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGDgnExtrusionDetail
{
DVector3d extrusionVector;
bool capped;
CurveVectorPtr baseGeometry;
};

struct CGDgnExtrusionFlags
{
CGDgnExtrusionFlags ()
    {
    extrusionVector_defined = false;
    capped_defined = false;
    baseGeometry_defined = false;    
    
    }

bool extrusionVector_defined;
bool capped_defined;
bool baseGeometry_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!extrusionVector_defined)
        numUndefined++;
    if (!capped_defined)
        numUndefined++;
    if (!baseGeometry_defined)
        numUndefined++;  
    return numUndefined;
    }
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

struct CGDgnRotationalSweepFlags
{
CGDgnRotationalSweepFlags ()
    {
    center_defined = false;
    axis_defined = false;
    sweepAngle_defined = false;
    capped_defined = false;
    baseGeometry_defined = false;    
    
    }

bool center_defined;
bool axis_defined;
bool sweepAngle_defined;
bool capped_defined;
bool baseGeometry_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!center_defined)
        numUndefined++;
    if (!axis_defined)
        numUndefined++;
    if (!sweepAngle_defined)
        numUndefined++;
    if (!capped_defined)
        numUndefined++;
    if (!baseGeometry_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGDgnRuledSweepDetail
{
bool capped;
bvector<CurveVectorPtr> contourArray;
};

struct CGDgnRuledSweepFlags
{
CGDgnRuledSweepFlags ()
    {
    capped_defined = false;
    contourArray_defined = false;    
    
    }

bool capped_defined;
bool contourArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!capped_defined)
        numUndefined++;
    if (!contourArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGTransitionSpiralDetail
{
String spiralType;
PlacementOriginZX placement;
Angle startBearing;
double startRadius;
Angle endBearing;
double length;
double endRadius;
double activeStartFraction;
double activeEndFraction;
IGeometryPtr geometry;
};

struct CGTransitionSpiralFlags
{
CGTransitionSpiralFlags ()
    {
    spiralType_defined = false;
    placement_defined = false;
    startBearing_defined = false;
    startRadius_defined = false;
    endBearing_defined = false;
    endRadius_defined = false;
    length_defined = false;
    activeStartFraction_defined = false;
    activeEndFraction_defined = false;
    geometry_defined = false;    
    
    }

bool spiralType_defined;
bool placement_defined;
bool startBearing_defined;
bool startRadius_defined;
bool endBearing_defined;
bool endRadius_defined;
bool length_defined;
bool activeStartFraction_defined;
bool activeEndFraction_defined;
bool geometry_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!spiralType_defined)
        numUndefined++;
    if (!placement_defined)
        numUndefined++;
    if (!startBearing_defined)
        numUndefined++;
    if (!startRadius_defined)
        numUndefined++;
    if (!endBearing_defined)
        numUndefined++;
    if (!endRadius_defined)
        numUndefined++;
    if (!activeStartFraction_defined)
        numUndefined++;
    if (!activeEndFraction_defined)
        numUndefined++;
    if (!geometry_defined)
        numUndefined++;  
    if (!length_defined)
        numUndefined++;  
    return numUndefined;
    }
};


// ===================================================================================
struct CGDgnCurveVectorDetail
{
int boundaryType;
bvector<ICurvePrimitivePtr> memberArray;
};

struct CGDgnCurveVectorFlags
{
CGDgnCurveVectorFlags ()
    {
    boundaryType_defined = false;
    memberArray_defined = false;    
    
    }

bool boundaryType_defined;
bool memberArray_defined;    

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!boundaryType_defined)
        numUndefined++;
    if (!memberArray_defined)
        numUndefined++;  
    return numUndefined;
    }
};

