declare module BentleyApi.Dgn {

    //! A wrapper for BentleyApi::DPoint3d
    class JsDPoint3d implements IDisposable {
        constructor(x: cxx_double, y: cxx_double, z: cxx_double);
        X: cxx_double;
        Y: cxx_double;
        Z: cxx_double;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsDPoint3dP = cxx_pointer<JsDPoint3d>;

    //! A wrapper for BentleyApi::DVector3d
    class JsDVector3d implements IDisposable {
        constructor(x: cxx_double, y: cxx_double, z: cxx_double);
        X: cxx_double;
        Y: cxx_double;
        Z: cxx_double;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsDVector3dP = cxx_pointer<JsDVector3d>;

    //! A wrapper for BentleyApi::YawPitchRollAngles
    class JsYawPitchRollAngles implements IDisposable {
        constructor(yaw: cxx_double, pitch: cxx_double, roll: cxx_double);
        YawDegrees: cxx_double;
        PitchDegrees: cxx_double;
        RollDegrees: cxx_double;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsYawPitchRollAnglesP = cxx_pointer<JsYawPitchRollAngles>;

    
    //! A strongly typed angle, with explicitly named access to degrees and radians
    class JsAngle implements IDisposable {
        
        static CreateDegrees (value : cxx_double) : JsAngleP;
        static CreateRadians (value : cxx_double) : JsAngleP;
        Radians : cxx_double;
        Degrees : cxx_double;
        OnDispose(): void;
        Dispose(): void;
    }
    type JsAngleP = cxx_pointer<JsAngle>;



 


    //! A wrapper for BentleyApi::DEllipse3d
    class JsDEllipse3d implements IDisposable {
        constructor (
                center      : JsDPoint3dP,
                vector0     : JsDVector3dP,
                vector90    : JsDVector3dP,
                startAngle  : JsAngleP,
                sweepAngle  : JsAngleP
                );
        PointAtFraction (f : cxx_double) : JsDPoint3dP;                
        OnDispose(): void;
        Dispose(): void;
    }

    type JsDEllipse3dP = cxx_pointer<JsDEllipse3d>;

    //! A wrapper for BentleyApi::DSegment3d
    class JsDSegment3d implements IDisposable {
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
        static CreateIdentity () : JsRotMatrixP;
        static CreateUniformScale (scaleFactor : cxx_double) : JsRotMatrixP;
        
        ColumnX : JsDVector3dP;        
        ColumnY : JsDVector3dP;        
        ColumnZ : JsDVector3dP;        

        RowX: JsDVector3dP;        
        RowY: JsDVector3dP;        
        RowZ: JsDVector3dP;        

        CreateScale (scaleX : cxx_double, scaleY : cxx_double, scaleZ : cxx_double) : JsRotMatrixP;
        CreateColumns (vectorU : JsDVector3dP, vectorV : JsDVector3dP, vectorZ : JsDVector3dP) : JsRotMatrixP;
        CreateRows (vectorU : JsDVector3dP, vectorV : JsDVector3dP, vectorZ : JsDVector3dP) : JsRotMatrixP;

        CreateRotationAroundVector (axis : JsDVector3dP, angle : JsAngleP) : JsRotMatrixP;
        Create90DegreeRotationAroundVector (axis : JsDVector3dP) : JsRotMatrixP;
        CreateDirectionalScale (direction : JsDVector3dP, scale : cxx_double) : JsRotMatrixP;
        // TODO: square and normalize !!!
        
        Create1Vector (direction : JsDVector3dP, axisIndex : cxx_double) : JsRotMatrixP;
        CreateFromXYVectors (vectorX : JsDVector3dP, vectorY : JsDVector3dP, axisIndex : cxx_double) : JsRotMatrixP;
        MultiplyVector (vector : JsDVector3dP) : JsDVector3dP;        
        MultiplyTransposeVector (vector : JsDVector3dP) : JsDVector3dP;        

        MultiplyXYZ (vector : JsDVector3dP, x : cxx_double, y : cxx_double, z : cxx_double) : JsDVector3dP;        
            
        MultiplyTransposeXYZ (vector : JsDVector3dP, x : cxx_double, y : cxx_double, z : cxx_double) : JsDVector3dP;        

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
        IsDigaonal () : cxx_bool;
        IsSignedPermutation () : cxx_bool;
        IsRigid () : cxx_bool;
        HasUnitLengthMutuallyPerpendicularRowsAndColumns () : cxx_bool;


        OnDispose(): void;
        Dispose(): void;
    }

    type JsTransformP = cxx_pointer<JsTransform>;
    //! A wrapper for BentleyApi::JsRotMatrix
    class JsTransform implements IDisposable {
        constructor();
        constructor
            (
            axx : cxx_double, axy : cxx_double, axz : cxx_double, axw : cxx_double,
            ayx : cxx_double, ayy : cxx_double, ayz : cxx_double, ayw : cxx_double,
            azx : cxx_double, azy : cxx_double, azz : cxx_double, azw : cxx_double
            );

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
            

        MultiplyXYZ (Point : JsDPoint3dP, x : cxx_double, y : cxx_double, z : cxx_double) : JsDPoint3dP;        
            
        MultiplyMatrixOnly (vector : JsDVector3dP) : JsDVector3dP;        
        MultiplyTransposeMatrixOnly (vector : JsDVector3dP) : JsDVector3dP;        

        MultiplyXYZMatrixOnly (vector : JsDVector3dP, x : cxx_double, y : cxx_double, z : cxx_double) : JsDVector3dP;        
            
        MultiplyTransposeXYZMatrixOnly (vector : JsDVector3dP, x : cxx_double, y : cxx_double, z : cxx_double) : JsDVector3dP;        

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
    class JsCurvePrimitive implements IDisposable {
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



}
