declare module BentleyApi.Dgn {

    //! A wrapper for BentleyApi::DPoint3d
    class JsDPoint3d implements IDisposable {
        constructor(x: cxx_double, y: cxx_double, z: cxx_double);
        X: cxx_double;
        Y: cxx_double;
        Z: cxx_double;

        Clone () : JsDPoint3dP;

        Interpolate (fraction : cxx_double, right : JsDPoint3dP) : JsDPoint3dP;
        Plus (vector : JsDVector3dP) : JsDPoint3dP;
        Minus (vector : JsDVector3dP) : JsDPoint3dP;
        PlusScaled (vector : JsDVector3dP, scalar : cxx_double) : JsDPoint3dP;
        Plus2Scaled (vectorA : JsDVector3dP, scalarA : cxx_double, vectorB : JsDVector3dP, scalarB : cxx_double) : JsDPoint3dP;
        Plus3Scaled (vectorA : JsDVector3dP, scalarA : cxx_double, vectorB : JsDVector3dP, scalarB : cxx_double, vectorC : JsDVector3dP, scalarC : cxx_double) : JsDPoint3dP;

        VectorTo(other : JsDPoint3dP): JsDVector3dP;

        Distance(other: JsDPoint3dP): cxx_double;
        DistanceSquared (other : JsDPoint3dP) : cxx_double;        
        MaxAbsDiff(vectorB: JsDPoint3dP): cxx_double;
        MaxAbs (): cxx_double;

        OnDispose(): void;
        Dispose(): void;
    }

    //! A wrapper for BentleyApi::DPoint2d
    class JsDPoint2d implements IDisposable {
        constructor(x: cxx_double, y: cxx_double);
        Clone () : JsDPoint2dP;
        X: cxx_double;
        Y: cxx_double;
        OnDispose(): void;
        Dispose(): void;

        Interpolate(fraction: cxx_double, right: JsDPoint2dP): JsDPoint2dP;
        Plus(vector: JsDVector2dP): JsDPoint2dP;
        Minus(vector: JsDVector2dP): JsDPoint2dP;
        PlusScaled(vector: JsDVector2dP, scalar: cxx_double): JsDPoint2dP;
        Plus2Scaled(vectorA: JsDVector2dP, scalarA: cxx_double, vectorB: JsDVector2dP, scalarB: cxx_double): JsDPoint2dP;
        Plus3Scaled(vectorA: JsDVector2dP, scalarA: cxx_double, vectorB: JsDVector2dP, scalarB: cxx_double, vectorC: JsDVector2dP, scalarC: cxx_double): JsDPoint2dP;

        VectorTo (other: JsDPoint2dP) : JsDVector2dP;

        Distance(other: JsDPoint2dP): cxx_double;
        DistanceSquared(other: JsDPoint2dP): cxx_double;        
        MaxAbsDiff(vectorB: JsDPoint2dP): cxx_double;
        MaxAbs(): cxx_double;

    }


    type JsDPoint3dP = cxx_pointer<JsDPoint3d>;
    type JsDPoint2dP = cxx_pointer<JsDPoint2d>;
    //! A wrapper for BentleyApi::DVector3d
    class JsDVector3d implements IDisposable {
        constructor(x: cxx_double, y: cxx_double, z: cxx_double);
        X: cxx_double;
        Y: cxx_double;
        Z: cxx_double;

        OnDispose(): void;
        Dispose(): void;



    Normalize () : JsDVector3dP;
    
    Clone () : JsDVector3dP;

    //Scaled vector by -1
    Negate () : JsDVector3dP;
        
    UnitVectorTowards (target : JsDPoint3dP) : JsDVector3dP;

    //Returns new vector that begins at start, and ends at end
    static FromStartEnd (start : JsDPoint3dP, end : JsDPoint3dP) : JsDVector3dP;
    //Same as start end, but normalizes result (magnitude : makes 1) : the
    static FromStartEndNormalize (start : JsDPoint3dP, end : JsDPoint3dP) : JsDVector3dP;
    //return a vector same length as source but rotate 90 degrees CCW
    FromCCWPerpendicularXY (source : JsDVector3dP) : JsDVector3dP;  
    FromRotate90Towards (source : JsDVector3dP, target : JsDVector3dP) : JsDVector3dP;
    static FromRotate90Around (source : JsDVector3dP, axis : JsDVector3dP) : JsDVector3dP;


    static FromXYAngleAndMagnitude(angle: cxx_double, magnitude: cxx_double): JsDVector3dP;

    Interpolate(fraction: cxx_double, right: JsDVector3dP): JsDVector3dP;
    Plus(vector: JsDVector3dP): JsDVector3dP;
    Minus(vector: JsDVector3dP): JsDVector3dP;
    PlusScaled(vector: JsDVector3dP, scalar: cxx_double): JsDVector3dP;
    Plus2Scaled(vectorA: JsDVector3dP, scalarA: cxx_double, vectorB: JsDVector3dP, scalarB: cxx_double): JsDVector3dP;
    Plus3Scaled(vectorA: JsDVector3dP, scalarA: cxx_double, vectorB: JsDVector3dP, scalarB: cxx_double, vectorC: JsDVector3dP, scalarC: cxx_double): JsDVector3dP;

    VectorTo(other: JsDVector3dP): JsDVector3dP;

    Scale (scale : cxx_double) : JsDVector3dP;
    ScaleToLength (scale : cxx_double) : JsDVector3dP;
    CrossProduct (vectorB : JsDVector3dP) : JsDVector3dP;
    NormalizedCrossProduct (vectorB : JsDVector3dP) : JsDVector3dP;
    SizedCrossProduct (vectorA : JsDVector3dP, vectorB : JsDVector3dP, productLength : cxx_double) : JsDVector3dP;
    RotateXY (angle : cxx_double) : JsDVector3dP;
    UnitPerpendicularXY (vector : JsDVector3dP) : JsDVector3dP;
    CrossProductMagnitude (vectorB : JsDVector3dP) : cxx_double;
    DotProduct (vectorB : JsDVector3dP) : cxx_double;
    DotProductXY (vectorB : JsDVector3dP) : cxx_double;
    CrossProductXY (vectorB : JsDVector3dP) : cxx_double;
    TripleProduct (vectorB : JsDVector3dP, vectorC : JsDVector3dP) : cxx_double;
    Magnitude () : cxx_double;
    MagnitudeSquared () : cxx_double;
    Distance (vectorB : JsDVector3dP) : cxx_double;
    DistanceSquared (vectorB : JsDVector3dP) : cxx_double;
    MaxAbs () : cxx_double;
    MaxAbsDiff (vectorB: JsDVector3dP): cxx_double;
    UnitPerpendicular () : JsDVector3dP;
    AngleTo (vectorB : JsDVector3dP) : JsAngleP;
    AngleToXY (vectorB : JsDVector3dP) : JsAngleP;
    SmallerUnorientedAngleTo (vectorB : JsDVector3dP) : JsAngleP;
    SignedAngleTo (vectorB : JsDVector3dP, upVector : JsDVector3dP) : JsAngleP;
    PlanarAngleTo (vectorB : JsDVector3dP, planeNormal : JsDVector3dP) : JsAngleP;
    IsInSmallerSector (vectorA : JsDVector3dP, vectorB : JsDVector3dP) : cxx_bool;
    IsInCCWSector (vectorA : JsDVector3dP, vectorB : JsDVector3dP, upVector : JsDVector3dP) : cxx_bool;
    IsParallelTo (vectorB : JsDVector3dP) : cxx_bool;
    IsPerpendicularTo (vectorB : JsDVector3dP) : cxx_bool;

    }

    type JsDVector3dP = cxx_pointer<JsDVector3d>;

    //! A wrapper for BentleyApi::DVector2d
    class JsDVector2d implements IDisposable
    {
        constructor(x: cxx_double, y: cxx_double);
        X: cxx_double;
        Y: cxx_double;

        OnDispose(): void;
        Dispose(): void;


        Clone(): JsDVector2dP;

        Interpolate(fraction: cxx_double, right: JsDVector2dP): JsDVector2dP;
        Plus(vector: JsDVector2dP): JsDVector2dP;
        Minus(vector: JsDVector2dP): JsDVector2dP;
        PlusScaled(vector: JsDVector2dP, scalar: cxx_double): JsDVector2dP;
        Plus2Scaled(vectorA: JsDVector2dP, scalarA: cxx_double, vectorB: JsDVector2dP, scalarB: cxx_double): JsDVector2dP;
        Plus3Scaled(vectorA: JsDVector2dP, scalarA: cxx_double, vectorB: JsDVector2dP, scalarB: cxx_double, vectorC: JsDVector2dP, scalarC: cxx_double): JsDVector2dP;

        VectorTo(other: JsDVector2dP ): JsDVector2dP;

        Magnitude(): cxx_double;
        MagnitudeSquared(): cxx_double;
        Distance(vectorB: JsDVector2dP): cxx_double;
        DistanceSquared(vectorB: JsDVector2dP): cxx_double;
        MaxAbs(): cxx_double;
        MaxAbsDiff(vectorB: JsDVector2dP): cxx_double;

    }
    type JsDVector2dP = cxx_pointer<JsDVector2d>;



    //! A wrapper for BentleyApi::YawPitchRollAngles
    class JsYawPitchRollAngles implements IDisposable {
        constructor(yaw: cxx_double, pitch: cxx_double, roll: cxx_double);
        Clone () : JsYawPitchRollAnglesP;
        YawDegrees: cxx_double;
        PitchDegrees: cxx_double;
        RollDegrees: cxx_double;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsYawPitchRollAnglesP = cxx_pointer<JsYawPitchRollAngles>;

    
    //! A strongly typed angle, with explicitly named access to degrees and radians
    class JsAngle implements IDisposable {

        Clone () : JsAngleP;        
        static CreateDegrees (value : cxx_double) : JsAngleP;
        static CreateRadians (value : cxx_double) : JsAngleP;
        Radians : cxx_double;
        Degrees : cxx_double;
        OnDispose(): void;
        Dispose(): void;
    }
    type JsAngleP = cxx_pointer<JsAngle>;

    //! A wrapper for BentleyApi::DEllipse3d
    class JsDRange3d implements IDisposable {
        //! constructor for empty range.
        constructor();
        Clone () : JsDRange3dP;
        
        Low: JsDPoint3dP;
        High: JsDPoint3dP;

        OnDispose(): void;
        Dispose(): void;
    }

    type JsDRange3dP = cxx_pointer<JsDRange3d>;


    //! A wrapper for BentleyApi::DEllipse3d
    class JsDEllipse3d implements IDisposable {
        constructor (
                center      : JsDPoint3dP,
                vector0     : JsDVector3dP,
                vector90    : JsDVector3dP,
                startAngle  : JsAngleP,
                sweepAngle  : JsAngleP
                );
        Clone () : JsDEllipse3dP;

        PointAtFraction (f : cxx_double) : JsDPoint3dP;                
        OnDispose(): void;
        Dispose(): void;
    }

    type JsDEllipse3dP = cxx_pointer<JsDEllipse3d>;

    //! A wrapper for BentleyApi::DSegment3d
    class JsDSegment3d implements IDisposable {
        Clone () : JsDSegment3dP;
    
        constructor (
                startPoint : JsDPoint3dP,
                endPoint   : JsDPoint3dP
                );
        PointAtFraction (f : cxx_double) : JsDPoint3dP;                
        OnDispose(): void;
        Dispose(): void;
    }

    type JsDSegment3dP = cxx_pointer<JsDSegment3d>;


    //! A wrapper for BentleyApi::DRay3d
    class JsDRay3d implements IDisposable {
        Clone () : JsDRay3dP;
        constructor (
                Origin : JsDPoint3dP,
                Direction   : JsDVector3dP
                );
        PointAtFraction (f : cxx_double) : JsDPoint3dP;                
        Origin: JsDPoint3dP;
        Direction: JsDVector3dP;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsDRay3dP = cxx_pointer<JsDRay3d>;


    type JsDPoint3dArrayP = cxx_pointer<JsDPoint3dArray>;

    //! A wrapper for BentleyApi::DPoint3dArray
    class JsDPoint3dArray implements IDisposable {
        Clone () : JsDPoint3dArrayP;
        constructor();

        Add(value: JsDPoint3dP): void;

        Size(): cxx_double;
        Clear (): void;
        Append(other: JsDPoint3dArrayP): void;

        At(index: cxx_double): JsDPoint3dP;

        OnDispose(): void;
        Dispose(): void;
    }

    type JsRotMatrixP = cxx_pointer<JsRotMatrix>;

    //! A wrapper for BentleyApi::JsRotMatrix
    class JsRotMatrix implements IDisposable {
        constructor();
        constructor
            (
            axx : cxx_double, axy : cxx_double, axz : cxx_double,
            ayx : cxx_double, ayy : cxx_double, ayz : cxx_double,
            azx : cxx_double, azy : cxx_double, azz : cxx_double
            );
        static CreateRotationAroundVector(axis: JsDVector3dP, angle: JsAngleP): JsRotMatrixP;
        static Create90DegreeRotationAroundVector(axis: JsDVector3dP): JsRotMatrixP;
        static CreateIdentity () : JsRotMatrixP;
        static CreateUniformScale (scaleFactor : cxx_double) : JsRotMatrixP;
        static CreateRowValues(x00: cxx_double, x01: cxx_double, x02: cxx_double,
            x10: cxx_double, x11: cxx_double, x12: cxx_double,
            x20: cxx_double, x21: cxx_double, x22: cxx_double): JsRotMatrixP;

        Clone () : JsRotMatrixP;
        ColumnX : JsDVector3dP;        
        ColumnY : JsDVector3dP;        
        ColumnZ : JsDVector3dP;        

        RowX: JsDVector3dP;        
        RowY: JsDVector3dP;        
        RowZ: JsDVector3dP;        

        static CreateScale (scaleX : cxx_double, scaleY : cxx_double, scaleZ : cxx_double) : JsRotMatrixP;
        static CreateColumns (vectorU : JsDVector3dP, vectorV : JsDVector3dP, vectorZ : JsDVector3dP) : JsRotMatrixP;
        static CreateRows (vectorU : JsDVector3dP, vectorV : JsDVector3dP, vectorZ : JsDVector3dP) : JsRotMatrixP;

        
        static CreateDirectionalScale (direction : JsDVector3dP, scale : cxx_double) : JsRotMatrixP;
        // TODO: square and normalize !!!
        
        static Create1Vector (direction : JsDVector3dP, axisIndex : cxx_double) : JsRotMatrixP;
        static CreateFromXYVectors (vectorX : JsDVector3dP, vectorY : JsDVector3dP, axisIndex : cxx_double) : JsRotMatrixP;
        MultiplyVector (vector : JsDVector3dP) : JsDVector3dP;        
        MultiplyTransposeVector (vector : JsDVector3dP) : JsDVector3dP;        

        MultiplyXYZ (x : cxx_double, y : cxx_double, z : cxx_double) : JsDVector3dP;        
            
        MultiplyTransposeXYZ ( x : cxx_double, y : cxx_double, z : cxx_double) : JsDVector3dP;        

        Solve (rightHandSide : JsDVector3dP) : JsDVector3dP;

         MultiplyMatrix (other : JsRotMatrixP) : JsRotMatrixP;

         Transpose () : JsRotMatrixP;
         Inverse () : JsRotMatrixP;
        At(ai : cxx_double, aj : cxx_double) : cxx_double;
        SetAt (ai : cxx_double, aj : cxx_double, value : cxx_double) : void;
            
        ScaleColumnsInPlace (scaleX : cxx_double, scaleY : cxx_double, scaleZ : cxx_double) : void;

        ScaleRowsInPlace (scaleX : cxx_double, scaleY : cxx_double, scaleZ : cxx_double) : void;

        Determinant () : cxx_double;
        ConditionNumber () : cxx_double;
        SumSquares () : cxx_double;
        SumDiagonalSquares () : cxx_double;
        MaxAbs () : cxx_double;
        MaxDiff (other : JsRotMatrixP) : cxx_double;
        
        IsIdentity () : cxx_bool;
        IsDiagonal () : cxx_bool;
        IsSignedPermutation () : cxx_bool;
        IsRigid () : cxx_bool;
        HasUnitLengthMutuallyPerpendicularRowsAndColumns () : cxx_bool;


        OnDispose(): void;
        Dispose(): void;
    }

    type JsTransformP = cxx_pointer<JsTransform>;
    //! A wrapper for BentleyApi::Transform
    class JsTransform implements IDisposable {
        constructor();
        constructor
            (
            axx : cxx_double, axy : cxx_double, axz : cxx_double, axw : cxx_double,
            ayx : cxx_double, ayy : cxx_double, ayz : cxx_double, ayw : cxx_double,
            azx : cxx_double, azy : cxx_double, azz : cxx_double, azw : cxx_double
            );


        Clone () : JsTransformP;


        static CreateIdentity () : JsTransformP;
        static CreateMatrix (matrix : JsRotMatrixP) : JsTransformP;
        static CreateMatrixAndTranslation (matrix : JsRotMatrixP, translation : JsDPoint3dP) : JsTransformP;
        static CreateMatrixAndFixedPoint (matrix : JsRotMatrixP, fixedPoint : JsDPoint3dP) : JsTransformP;
        static CreateTranslation (translation : JsDPoint3dP) : JsTransformP;
        static CreateTranslationXYZ (x : cxx_double, y : cxx_double, z : cxx_double) : JsTransformP;
        static CreateScaleAroundPoint (fixedPoint : JsDPoint3dP, scaleX : cxx_double, scaleY : cxx_double, scaleZ : cxx_double) : JsTransformP;

        static CreateOriginAndVectors (origin : JsDPoint3dP, xVector : JsDVector3dP, yVector : JsDVector3dP, zVector : JsDVector3dP) : JsTransformP;

        static CreateOriginAnd3TargetPoints (origin : JsDPoint3dP, xPoint : JsDPoint3dP, yPoint : JsDPoint3dP, zPoint : JsDPoint3dP) : JsTransformP;

        static CreateOriginAnd2TargetPoints (origin : JsDPoint3dP, xPoint : JsDPoint3dP, yPoint : JsDPoint3dP) : JsTransformP;

        static CreateRotationAroundRay (ray : JsDRay3dP, angle : JsAngleP) : JsTransformP;                
        
        
        MultiplyPoint (Point : JsDPoint3dP) : JsDPoint3dP;
            

        MultiplyXYZ (x : cxx_double, y : cxx_double, z : cxx_double) : JsDPoint3dP;        
            
        MultiplyMatrixOnly (vector : JsDVector3dP) : JsDVector3dP;        
        MultiplyTransposeMatrixOnly (vector : JsDVector3dP) : JsDVector3dP;        

        MultiplyXYZMatrixOnly (x : cxx_double, y : cxx_double, z : cxx_double) : JsDVector3dP;        
            
        MultiplyTransposeXYZMatrixOnly (x : cxx_double, y : cxx_double, z : cxx_double) : JsDVector3dP;        

        Solve (rightHandSide : JsDPoint3dP) : JsDPoint3dP;

         MultiplyTransform (other : JsTransformP) : JsTransformP;
         Inverse () : JsTransformP;
        GetColumnX () : JsDVector3dP;
        GetColumnY () : JsDVector3dP;
        GetColumnZ () : JsDVector3dP;
        GetRowX () : JsDVector3dP;
        GetRowY () : JsDVector3dP;
        GetRowZ () : JsDVector3dP;
        
        SetColumnX (value : JsDVector3dP) : void;
        SetColumnY (value : JsDVector3dP) : void;
        SetColumnZ (value : JsDVector3dP) : void;

        SetRowX (value : JsDVector3dP) : void;
        SetRowY (value : JsDVector3dP) : void;
        SetRowZ (value : JsDVector3dP) : void;

        GetMatrix () : JsRotMatrixP;
        SetMatrix (value : JsRotMatrixP) : void;

        GetTranslation () : JsDPoint3dP;
        SetTranslation (value : JsDPoint3dP) : void;
        ZeroTranslation () : void;
        SetFixedPoint (value : JsDPoint3dP) : void;
        MatrixEntryAt (ai : cxx_double, aj : cxx_double) : cxx_double;
        SetMatrixAt (ai : cxx_double, aj : cxx_double, value : cxx_double) : void;

        Determinant () : cxx_double;
        MaxDiff (other : JsTransformP) : cxx_double;
        MatrixColumnMagnitude (axisIndex : cxx_double) : cxx_double;
        IsIdentity () : cxx_bool;
        IsRigid () : cxx_bool;
            
        ScaleMatrixColumnsInPlace (scaleX : cxx_double, scaleY : cxx_double, scaleZ : cxx_double) : void;
        OnDispose(): void;
        Dispose(): void;
    }





//! A wrapper for BentleyApi::JsCurvePrimitive
class JsCurvePrimitive implements IDisposable
    {
    Clone () : JsCurvePrimitiveP;
    constructor ();
    static CreateLineSegment(segment: JsDSegment3dP): JsCurvePrimitiveP;
    static CreateEllipticArc(arc: JsDEllipse3dP): JsCurvePrimitiveP;
    static CreateLineString(points: JsDPoint3dArrayP): JsCurvePrimitiveP;
    CurvePrimitiveType(): cxx_double;
    PointAtFraction(f: cxx_double): JsDPoint3dP; 

    OnDispose(): void;
    Dispose(): void;
    }

    type JsCurvePrimitiveP = cxx_pointer<JsCurvePrimitive>;


//! A wrapper for a polyface mesh !!!
class JsPolyfaceMesh implements IDisposable
    {
    Clone () : JsPolyfaceMeshP;

    GetTwoSided () : cxx_bool;

    static CreateVariableSizeIndexed() : JsPolyfaceMeshP;
    
    static CreateFixedIndexed(aNumPerFace : cxx_double) : JsPolyfaceMeshP;

    static CreateQuadGrid(numPerRow : cxx_double) : JsPolyfaceMeshP;

    static CreateTriangleGrid(numPerRow : cxx_double) : JsPolyfaceMeshP;

    static CreateCoordinateTriangleMesh() : JsPolyfaceMeshP;

    static CreateCoordinateQuadMesh() : JsPolyfaceMeshP;

    PolyfaceMeshStyle() : cxx_double;

    // Single-value AddXxx methods -- note that Append method on blocked arrays SetActive () : does !!!
    AddPoint(data : JsDPoint3dP) : void;

    AddNormal(data : JsDVector3dP) : void;
    AddParam(data : JsDPoint2dP) : void;
    AddIntColor(aColor : cxx_double) : void;

    AddPointIndex(aData : cxx_double) : void;
    AddNormalIndex(aData : cxx_double) : void;
    AddParamndex(aData : cxx_double) : void;
    AddColorIndex(aData : cxx_double) : void;
 
    InspectFaces() : Bentley_BeJsObject<Object>;

    GetRange() : JsDRange3dP;

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
    type JsPolyfaceMeshP = cxx_pointer<JsPolyfaceMesh>;


    //! A wrapper for a polyface visitor
class JsPolyfaceVisitor implements IDisposable
    {
    static CreateVisitor(mesh : JsPolyfaceMeshP, aNumWrap : cxx_double) : JsPolyfaceVisitorP;
    Reset() : void;
    AdvanceToNextFacet() : cxx_bool;
    
    GetPoint(aIndex : cxx_double) : JsDPoint3dP;
    GetParam(aIndex : cxx_double) : JsDPoint2dP;
    GetNormal(aIndex : cxx_double) : JsDVector3dP;
    
    GetEdgeCount () : cxx_double;
    OnDispose(): void;
    Dispose(): void;
}

type JsPolyfaceVisitorP = cxx_pointer<JsPolyfaceVisitor>;

}
