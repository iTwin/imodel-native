declare module Bentley.Dgn /*** NATIVE_TYPE_NAME = BentleyApi::Dgn ***/ 
{
    //! A wrapper for BentleyApi::DPoint3d
    class DPoint3d implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDPoint3d ***/ 
        constructor(x: cxx_double, y: cxx_double, z: cxx_double);
        X: cxx_double;
        Y: cxx_double;
        Z: cxx_double;

        Clone () : DPoint3dP;

        Interpolate (fraction : cxx_double, right : DPoint3dP) : DPoint3dP;
        Plus (vector : DVector3dP) : DPoint3dP;
        Minus (vector : DVector3dP) : DPoint3dP;
        PlusScaled (vector : DVector3dP, scalar : cxx_double) : DPoint3dP;
        Plus2Scaled (vectorA : DVector3dP, scalarA : cxx_double, vectorB : DVector3dP, scalarB : cxx_double) : DPoint3dP;
        Plus3Scaled (vectorA : DVector3dP, scalarA : cxx_double, vectorB : DVector3dP, scalarB : cxx_double, vectorC : DVector3dP, scalarC : cxx_double) : DPoint3dP;

        VectorTo(other : DPoint3dP): DVector3dP;

        Distance(other: DPoint3dP): cxx_double;
        DistanceSquared (other : DPoint3dP) : cxx_double;        
        MaxAbsDiff(vectorB: DPoint3dP): cxx_double;
        MaxAbs (): cxx_double;

        OnDispose(): void;
        Dispose(): void;
    }

    //! A wrapper for BentleyApi::DPoint2d
    class DPoint2d implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDPoint2d ***/ 
        constructor(x: cxx_double, y: cxx_double);
        Clone () : DPoint2dP;
        X: cxx_double;
        Y: cxx_double;
        OnDispose(): void;
        Dispose(): void;

        Interpolate(fraction: cxx_double, right: DPoint2dP): DPoint2dP;
        Plus(vector: DVector2dP): DPoint2dP;
        Minus(vector: DVector2dP): DPoint2dP;
        PlusScaled(vector: DVector2dP, scalar: cxx_double): DPoint2dP;
        Plus2Scaled(vectorA: DVector2dP, scalarA: cxx_double, vectorB: DVector2dP, scalarB: cxx_double): DPoint2dP;
        Plus3Scaled(vectorA: DVector2dP, scalarA: cxx_double, vectorB: DVector2dP, scalarB: cxx_double, vectorC: DVector2dP, scalarC: cxx_double): DPoint2dP;

        VectorTo (other: DPoint2dP) : DVector2dP;

        Distance(other: DPoint2dP): cxx_double;
        DistanceSquared(other: DPoint2dP): cxx_double;        
        MaxAbsDiff(vectorB: DPoint2dP): cxx_double;
        MaxAbs(): cxx_double;

    }


    type DPoint3dP = cxx_pointer<DPoint3d>;
    type DPoint2dP = cxx_pointer<DPoint2d>;
    //! A wrapper for BentleyApi::DVector3d
    class DVector3d implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDVector3d ***/ 
        constructor(x: cxx_double, y: cxx_double, z: cxx_double);
        X: cxx_double;
        Y: cxx_double;
        Z: cxx_double;

        OnDispose(): void;
        Dispose(): void;



    Normalize () : DVector3dP;
    
    Clone () : DVector3dP;

    //Scaled vector by -1
    Negate () : DVector3dP;
        
    UnitVectorTo(target: DVector3dP) : DVector3dP;

    //return a vector same length as source but rotate 90 degrees CCW
    FromCCWPerpendicularXY (source : DVector3dP) : DVector3dP;  
    Rotate90Towards (target : DVector3dP) : DVector3dP;
    Rotate90Around (axis : DVector3dP) : DVector3dP;


    static CreateXYAngleAndMagnitude(angle: AngleP, magnitude: cxx_double): DVector3dP;
    // Adding vectors
    Interpolate(fraction: cxx_double, right: DVector3dP): DVector3dP;
    Plus(vector: DVector3dP): DVector3dP;
    Minus(vector: DVector3dP): DVector3dP;
    PlusScaled(vector: DVector3dP, scalar: cxx_double): DVector3dP;
    Plus2Scaled(vectorA: DVector3dP, scalarA: cxx_double, vectorB: DVector3dP, scalarB: cxx_double): DVector3dP;
    Plus3Scaled(vectorA: DVector3dP, scalarA: cxx_double, vectorB: DVector3dP, scalarB: cxx_double, vectorC: DVector3dP, scalarC: cxx_double): DVector3dP;

    VectorTo(other: DVector3dP): DVector3dP;

    Scale (scale : cxx_double) : DVector3dP;

    ScaleToLength(scale: cxx_double): DVector3dP;
    UnitCrossProduct(vectorB: DVector3dP): DVector3dP;
    SizedCrossProduct (vectorA : DVector3dP, vectorB : DVector3dP, productLength : cxx_double) : DVector3dP;
    UnitPerpendicularXY(vector: DVector3dP): DVector3dP;
    UnitPerpendicular(): DVector3dP;

    RotateXY(angle: cxx_double): DVector3dP;

    // products
    CrossProductMagnitude (vectorB : DVector3dP) : cxx_double;
    DotProduct (vectorB : DVector3dP) : cxx_double;
    DotProductXY (vectorB : DVector3dP) : cxx_double;
    CrossProductXY (vectorB : DVector3dP) : cxx_double;
    TripleProduct(vectorB: DVector3dP, vectorC: DVector3dP): cxx_double;
    CrossProduct(vectorB: DVector3dP): DVector3dP;


    Magnitude () : cxx_double;
    MagnitudeSquared () : cxx_double;
    Distance (vectorB : DVector3dP) : cxx_double;
    DistanceSquared (vectorB : DVector3dP) : cxx_double;
    MaxAbs () : cxx_double;
    MaxAbsDiff(vectorB: DVector3dP): cxx_double;


    // angles
    AngleTo (vectorB : DVector3dP) : AngleP;
    AngleToXY (vectorB : DVector3dP) : AngleP;
    SmallerUnorientedAngleTo (vectorB : DVector3dP) : AngleP;
    SignedAngleTo (vectorB : DVector3dP, upVector : DVector3dP) : AngleP;
    PlanarAngleTo (vectorB : DVector3dP, planeNormal : DVector3dP) : AngleP;
    // sectors
    IsInSmallerSector(vectorA: DVector3dP, vectorB: DVector3dP): cxx_bool;
    IsInCCWSector (vectorA : DVector3dP, vectorB : DVector3dP, upVector : DVector3dP) : cxx_bool;
    IsParallelTo (vectorB : DVector3dP) : cxx_bool;
    IsPerpendicularTo (vectorB : DVector3dP) : cxx_bool;

    }

    type DVector3dP = cxx_pointer<DVector3d>;

    //! A wrapper for BentleyApi::DVector2d
    class DVector2d implements IDisposable
    {
        /*** NATIVE_TYPE_NAME = JsDVector2d ***/ 
        constructor(x: cxx_double, y: cxx_double);
        X: cxx_double;
        Y: cxx_double;

        OnDispose(): void;
        Dispose(): void;


        Clone(): DVector2dP;

        Interpolate(fraction: cxx_double, right: DVector2dP): DVector2dP;
        Plus(vector: DVector2dP): DVector2dP;
        Minus(vector: DVector2dP): DVector2dP;
        PlusScaled(vector: DVector2dP, scalar: cxx_double): DVector2dP;
        Plus2Scaled(vectorA: DVector2dP, scalarA: cxx_double, vectorB: DVector2dP, scalarB: cxx_double): DVector2dP;
        Plus3Scaled(vectorA: DVector2dP, scalarA: cxx_double, vectorB: DVector2dP, scalarB: cxx_double, vectorC: DVector2dP, scalarC: cxx_double): DVector2dP;

        VectorTo(other: DVector2dP ): DVector2dP;

        Magnitude(): cxx_double;
        MagnitudeSquared(): cxx_double;
        Distance(vectorB: DVector2dP): cxx_double;
        DistanceSquared(vectorB: DVector2dP): cxx_double;
        MaxAbs(): cxx_double;
        MaxAbsDiff(vectorB: DVector2dP): cxx_double;

    }
    type DVector2dP = cxx_pointer<DVector2d>;



    //! A wrapper for BentleyApi::YawPitchRollAngles
    class YawPitchRollAngles implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsYawPitchRollAngles ***/ 
        constructor(yaw: cxx_double, pitch: cxx_double, roll: cxx_double);
        Clone () : YawPitchRollAnglesP;
        YawDegrees: cxx_double;
        PitchDegrees: cxx_double;
        RollDegrees: cxx_double;
        OnDispose(): void;
        Dispose(): void;
    }

    type YawPitchRollAnglesP = cxx_pointer<YawPitchRollAngles>;

    
    //! A strongly typed angle, with explicitly named access to degrees and radians
    class Angle implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsAngle ***/ 

        Clone () : AngleP;        
        static CreateDegrees (value : cxx_double) : AngleP;
        static CreateRadians (value : cxx_double) : AngleP;
        Radians : cxx_double;
        Degrees : cxx_double;
        OnDispose(): void;
        Dispose(): void;
    }
    type AngleP = cxx_pointer<Angle>;

    //! A wrapper for BentleyApi::DEllipse3d
    class DRange3d implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDRange3d ***/ 
        //! constructor for empty range.
        constructor();
        Clone () : DRange3dP;
        
        Low: DPoint3dP;
        High: DPoint3dP;


        IsNull(): cxx_bool;
        IsSinglePoint(): cxx_bool;

        XLength(): cxx_double;
        YLength(): cxx_double;
        ZLength(): cxx_double;

        MaxAbs(): cxx_double;
        IsAlmostZeroX(): cxx_bool;
        IsAlmostZeroY(): cxx_bool;
        IsAlmostZeroZ(): cxx_bool;

        ContainsXYZ(x: cxx_double, y: cxx_double, z: cxx_double): cxx_bool;
        ContainsPoint(point: DPoint3dP): cxx_bool;
        ContainsPointPointXY(point: DPoint3dP): cxx_bool;
        ContainsRange(other: DRange3dP): cxx_bool;

        IntersectsRange(other: DRange3dP): cxx_bool;

        // policy: only return distances (squared : no -- sqrt time will not show up in ts)
        DistanceToPoint(point: DPoint3dP): cxx_double;
        DistanceToRange(other: DRange3dP): cxx_double;


        ExtendAllDirections(a: cxx_double): void;
        ExtendXYZ(x: cxx_double, y: cxx_double, z: cxx_double): void;
        ExtendPoint(point: DPoint3dP): void;
        ExtendRange(range: DRange3dP): void;

        Intersect(other: DRange3dP): DRange3dP;
        Union(other: DRange3dP): DRange3dP;

        Init(): void;

        OnDispose(): void;
        Dispose(): void;
    }

    type DRange3dP = cxx_pointer<DRange3d>;


    //! A wrapper for BentleyApi::DEllipse3d
    class DEllipse3d implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDEllipse3d ***/ 
        constructor(
                center      : DPoint3dP,
                vector0     : DVector3dP,
                vector90    : DVector3dP,
                startAngle  : AngleP,
                sweepAngle  : AngleP
                );
        Clone () : DEllipse3dP;

        PointAtFraction (f : cxx_double) : DPoint3dP;                
        OnDispose(): void;
        Dispose(): void;
    }

    type DEllipse3dP = cxx_pointer<DEllipse3d>;

    //! A wrapper for BentleyApi::DSegment3d
    class DSegment3d implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDSegment3d ***/ 
        Clone(): DSegment3dP;
    
        constructor (
                startPoint : DPoint3dP,
                endPoint   : DPoint3dP
                );
        PointAtFraction (f : cxx_double) : DPoint3dP;                
        OnDispose(): void;
        Dispose(): void;
    }

    type DSegment3dP = cxx_pointer<DSegment3d>;


    //! A wrapper for BentleyApi::DRay3d
    class DRay3d implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDRay3d ***/ 
        Clone(): DRay3dP;
        constructor (
                Origin : DPoint3dP,
                Direction   : DVector3dP
                );
        PointAtFraction (f : cxx_double) : DPoint3dP;                
        Origin: DPoint3dP;
        Direction: DVector3dP;
        OnDispose(): void;
        Dispose(): void;
    }

    type DRay3dP = cxx_pointer<DRay3d>;


    type DPoint3dArrayP = cxx_pointer<DPoint3dArray>;

    //! A wrapper for BentleyApi::DPoint3dArray
    class DPoint3dArray implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDPoint3dArray ***/ 
        Clone(): DPoint3dArrayP;
        constructor();

        Add(value: DPoint3dP): void;

        Size(): cxx_double;
        Clear (): void;
        Append(other: DPoint3dArrayP): void;

        At(index: cxx_double): DPoint3dP;

        OnDispose(): void;
        Dispose(): void;
    }

    type DoubleArrayP = cxx_pointer<DoubleArray>;

    //! A wrapper for BentleyApi::DoubleArray
    class DoubleArray implements IDisposable
    {
        /*** NATIVE_TYPE_NAME = JsDoubleArray ***/ 
        Clone(): DoubleArrayP;
        constructor();

        Add(value: cxx_double): void;

        Size(): cxx_double;
        Clear(): void;
        Append(other: DoubleArrayP): void;

        At(index: cxx_double): cxx_double;

        OnDispose(): void;
        Dispose(): void;
    }


    type RotMatrixP = cxx_pointer<RotMatrix>;

    //! A wrapper for BentleyApi::JsRotMatrix
    class RotMatrix implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsRotMatrix ***/ 
        constructor();
        constructor
            (
            axx : cxx_double, axy : cxx_double, axz : cxx_double,
            ayx : cxx_double, ayy : cxx_double, ayz : cxx_double,
            azx : cxx_double, azy : cxx_double, azz : cxx_double
            );
        static CreateRotationAroundVector(axis: DVector3dP, angle: AngleP): RotMatrixP;
        static Create90DegreeRotationAroundVector(axis: DVector3dP): RotMatrixP;
        static CreateIdentity () : RotMatrixP;
        static CreateUniformScale (scaleFactor : cxx_double) : RotMatrixP;
        static CreateRowValues(x00: cxx_double, x01: cxx_double, x02: cxx_double,
            x10: cxx_double, x11: cxx_double, x12: cxx_double,
            x20: cxx_double, x21: cxx_double, x22: cxx_double): RotMatrixP;

        Clone () : RotMatrixP;
        ColumnX : DVector3dP;        
        ColumnY : DVector3dP;        
        ColumnZ : DVector3dP;        

        RowX: DVector3dP;        
        RowY: DVector3dP;        
        RowZ: DVector3dP;        

        static CreateScale (scaleX : cxx_double, scaleY : cxx_double, scaleZ : cxx_double) : RotMatrixP;
        static CreateColumns (vectorU : DVector3dP, vectorV : DVector3dP, vectorZ : DVector3dP) : RotMatrixP;
        static CreateRows (vectorU : DVector3dP, vectorV : DVector3dP, vectorZ : DVector3dP) : RotMatrixP;

        
        static CreateDirectionalScale (direction : DVector3dP, scale : cxx_double) : RotMatrixP;
        // TODO: square and normalize !!!
        
        static Create1Vector (direction : DVector3dP, axisIndex : cxx_double) : RotMatrixP;
        static CreateFromXYVectors (vectorX : DVector3dP, vectorY : DVector3dP, axisIndex : cxx_double) : RotMatrixP;
        MultiplyVector (vector : DVector3dP) : DVector3dP;        
        MultiplyTransposeVector (vector : DVector3dP) : DVector3dP;        

        MultiplyXYZ (x : cxx_double, y : cxx_double, z : cxx_double) : DVector3dP;        
            
        MultiplyTransposeXYZ ( x : cxx_double, y : cxx_double, z : cxx_double) : DVector3dP;        

        Solve (rightHandSide : DVector3dP) : DVector3dP;

         MultiplyMatrix (other : RotMatrixP) : RotMatrixP;

         Transpose () : RotMatrixP;
         Inverse () : RotMatrixP;
        At(ai : cxx_double, aj : cxx_double) : cxx_double;
        SetAt (ai : cxx_double, aj : cxx_double, value : cxx_double) : void;
            
        ScaleColumnsInPlace (scaleX : cxx_double, scaleY : cxx_double, scaleZ : cxx_double) : void;

        ScaleRowsInPlace (scaleX : cxx_double, scaleY : cxx_double, scaleZ : cxx_double) : void;

        Determinant () : cxx_double;
        ConditionNumber () : cxx_double;
        SumSquares () : cxx_double;
        SumDiagonalSquares () : cxx_double;
        MaxAbs () : cxx_double;
        MaxDiff (other : RotMatrixP) : cxx_double;
        
        IsIdentity () : cxx_bool;
        IsDiagonal () : cxx_bool;
        IsSignedPermutation () : cxx_bool;
        IsRigid () : cxx_bool;
        HasUnitLengthMutuallyPerpendicularRowsAndColumns () : cxx_bool;


        OnDispose(): void;
        Dispose(): void;
    }

    type TransformP = cxx_pointer<Transform>;
    //! A wrapper for BentleyApi::Transform
    class Transform implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsTransform ***/ 
        constructor();
        constructor
            (
            axx : cxx_double, axy : cxx_double, axz : cxx_double, axw : cxx_double,
            ayx : cxx_double, ayy : cxx_double, ayz : cxx_double, ayw : cxx_double,
            azx : cxx_double, azy : cxx_double, azz : cxx_double, azw : cxx_double
            );


        Clone () : TransformP;


        static CreateIdentity () : TransformP;
        static CreateMatrix (matrix : RotMatrixP) : TransformP;
        static CreateMatrixAndTranslation (matrix : RotMatrixP, translation : DPoint3dP) : TransformP;
        static CreateMatrixAndFixedPoint (matrix : RotMatrixP, fixedPoint : DPoint3dP) : TransformP;
        static CreateTranslation (translation : DPoint3dP) : TransformP;
        static CreateTranslationXYZ (x : cxx_double, y : cxx_double, z : cxx_double) : TransformP;
        static CreateScaleAroundPoint (fixedPoint : DPoint3dP, scaleX : cxx_double, scaleY : cxx_double, scaleZ : cxx_double) : TransformP;

        static CreateOriginAndVectors (origin : DPoint3dP, xVector : DVector3dP, yVector : DVector3dP, zVector : DVector3dP) : TransformP;

        static CreateOriginAnd3TargetPoints (origin : DPoint3dP, xPoint : DPoint3dP, yPoint : DPoint3dP, zPoint : DPoint3dP) : TransformP;

        static CreateOriginAnd2TargetPoints (origin : DPoint3dP, xPoint : DPoint3dP, yPoint : DPoint3dP) : TransformP;

        static CreateRotationAroundRay (ray : DRay3dP, angle : AngleP) : TransformP;                
        
        
        MultiplyPoint (Point : DPoint3dP) : DPoint3dP;
            

        MultiplyXYZ (x : cxx_double, y : cxx_double, z : cxx_double) : DPoint3dP;        
            
        MultiplyMatrixOnly (vector : DVector3dP) : DVector3dP;        
        MultiplyTransposeMatrixOnly (vector : DVector3dP) : DVector3dP;        

        MultiplyXYZMatrixOnly (x : cxx_double, y : cxx_double, z : cxx_double) : DVector3dP;        
            
        MultiplyTransposeXYZMatrixOnly (x : cxx_double, y : cxx_double, z : cxx_double) : DVector3dP;        

        Solve (rightHandSide : DPoint3dP) : DPoint3dP;

         MultiplyTransform (other : TransformP) : TransformP;
         Inverse () : TransformP;
        GetColumnX () : DVector3dP;
        GetColumnY () : DVector3dP;
        GetColumnZ () : DVector3dP;
        GetRowX () : DVector3dP;
        GetRowY () : DVector3dP;
        GetRowZ () : DVector3dP;
        
        SetColumnX (value : DVector3dP) : void;
        SetColumnY (value : DVector3dP) : void;
        SetColumnZ (value : DVector3dP) : void;

        SetRowX (value : DVector3dP) : void;
        SetRowY (value : DVector3dP) : void;
        SetRowZ (value : DVector3dP) : void;

        GetMatrix () : RotMatrixP;
        SetMatrix (value : RotMatrixP) : void;

        GetTranslation () : DPoint3dP;
        SetTranslation (value : DPoint3dP) : void;
        ZeroTranslation () : void;
        SetFixedPoint (value : DPoint3dP) : void;
        MatrixEntryAt (ai : cxx_double, aj : cxx_double) : cxx_double;
        SetMatrixAt (ai : cxx_double, aj : cxx_double, value : cxx_double) : void;

        Determinant () : cxx_double;
        MaxDiff (other : TransformP) : cxx_double;
        MatrixColumnMagnitude (axisIndex : cxx_double) : cxx_double;
        IsIdentity () : cxx_bool;
        IsRigid () : cxx_bool;
            
        ScaleMatrixColumnsInPlace (scaleX : cxx_double, scaleY : cxx_double, scaleZ : cxx_double) : void;
        OnDispose(): void;
        Dispose(): void;
    }



    //! A wrapper for a polyface visitor
    class BsplineCurve implements IDisposable, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsBsplineCurve ***/ 

        OnDispose(): void;
        Dispose(): void;
        IsPeriodic () : cxx_bool;
        static CreateFromPoles(xyz: DPoint3dArrayP,
            weights: DoubleArrayP,
            knots : DoubleArrayP,
            order: cxx_double, closed: cxx_bool, preWeighted: cxx_bool): BsplineCurveP;
    }

    type BsplineCurveP = cxx_pointer<BsplineCurve>;



    
//! A wrapper for BentleyApi::JsCurvePrimitive
class CurvePrimitive implements IDisposable
    {
    /*** NATIVE_TYPE_NAME = JsCurvePrimitive ***/ 
    Clone(): CurvePrimitiveP;
    constructor ();
    static CreateLineSegment(segment: DSegment3dP): CurvePrimitiveP;
    static CreateEllipticArc(arc: DEllipse3dP): CurvePrimitiveP;
    static CreateLineString(points: DPoint3dArrayP): CurvePrimitiveP;
    static CreateBsplineCurve (curve: BsplineCurveP) : CurvePrimitiveP;
    CurvePrimitiveType(): cxx_double;
    PointAtFraction(f: cxx_double): DPoint3dP; 

    OnDispose(): void;
    Dispose(): void;
    }

    type CurvePrimitiveP = cxx_pointer<CurvePrimitive>;


//! A wrapper for a polyface mesh !!!
class PolyfaceMesh implements IDisposable
    {
    /*** NATIVE_TYPE_NAME = JsPolyfaceMesh ***/ 
    Clone(): PolyfaceMeshP;

    GetTwoSided () : cxx_bool;

    static CreateVariableSizeIndexed() : PolyfaceMeshP;
    
    static CreateFixedIndexed(aNumPerFace : cxx_double) : PolyfaceMeshP;

    static CreateQuadGrid(numPerRow : cxx_double) : PolyfaceMeshP;

    static CreateTriangleGrid(numPerRow : cxx_double) : PolyfaceMeshP;

    static CreateCoordinateTriangleMesh() : PolyfaceMeshP;

    static CreateCoordinateQuadMesh() : PolyfaceMeshP;

    PolyfaceMeshStyle() : cxx_double;

    // Single-value AddXxx methods -- note that Append method on blocked arrays SetActive () : does !!!
    AddPoint(data : DPoint3dP) : void;

    AddNormal(data : DVector3dP) : void;
    AddParam(data : DPoint2dP) : void;
    AddIntColor(aColor : cxx_double) : void;

    AddPointIndex(aData : cxx_double) : void;
    AddNormalIndex(aData : cxx_double) : void;
    AddParamndex(aData : cxx_double) : void;
    AddColorIndex(aData : cxx_double) : void;
 
    InspectFaces() : Bentley_BeJsObject<Object>;

    GetRange() : DRange3dP;

    GetTightTolerance() : cxx_double;
    GetMediumTolerance() : cxx_double;
    IsClosedByEdgePairing() : cxx_bool;
    HasFacets() : cxx_bool;
    HasConvexFacets() : cxx_bool;
    GetFacetCount () : cxx_double;
    GetLargestCoordinate() : cxx_double;

    OnDispose(): void;
    Dispose(): void;
    }
    type PolyfaceMeshP = cxx_pointer<PolyfaceMesh>;


    //! A wrapper for a polyface visitor
class PolyfaceVisitor implements IDisposable
    {
    /*** NATIVE_TYPE_NAME = JsPolyfaceVisitor ***/ 
    static CreateVisitor(mesh: PolyfaceMeshP, aNumWrap : cxx_double) : PolyfaceVisitorP;
    Reset() : void;
    AdvanceToNextFacet() : cxx_bool;
    
    GetPoint(aIndex : cxx_double) : DPoint3dP;
    GetParam(aIndex : cxx_double) : DPoint2dP;
    GetNormal(aIndex : cxx_double) : DVector3dP;
    
    GetEdgeCount () : cxx_double;
    OnDispose(): void;
    Dispose(): void;
}

type PolyfaceVisitorP = cxx_pointer<PolyfaceVisitor>;





}
