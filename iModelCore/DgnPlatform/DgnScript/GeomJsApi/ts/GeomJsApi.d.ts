declare module Bentley.Dgn /*** NATIVE_TYPE_NAME = BentleyApi::Dgn ***/ 
{
/* *******************************************************************************
* ISOLATED CLASSES:  DPoint3d, DPoint2d, DVector3d, DVector2d, RotMatrix, Transform
***********************************************************************************/
/**
* @description 3D point with X,Y,Z properties.
*/
    class DPoint3d implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDPoint3d ***/ 
        /** Constructor from x,y,z coordinates */
        constructor(x: cxx_double, y: cxx_double, z: cxx_double);
/** Read/Write access to X component */
        X: cxx_double;
/** Read/Write access to Y component */
        Y: cxx_double;
/** Read/Write access to Z component */
        Z: cxx_double;

        Clone () : DPoint3dP;
/** Return a point interpolated between this and the right param. */
        Interpolate (fraction : cxx_double, right : DPoint3dP) : DPoint3dP;
/** return point plus vector. */
        Plus (vector : DVector3dP) : DPoint3dP;
/** Return point minus vector */
        Minus (vector : DVector3dP) : DPoint3dP;
/** return point + vector * scalar */
        PlusScaled (vector : DVector3dP, scalar : cxx_double) : DPoint3dP;
/** return point + vectorA * scalarA + vectorB * scalarB */
        Plus2Scaled (vectorA : DVector3dP, scalarA : cxx_double, vectorB : DVector3dP, scalarB : cxx_double) : DPoint3dP;
/** return point + vectorA * scalarA + vectorB * scalarB + vectorC * scalarC */
        Plus3Scaled (vectorA : DVector3dP, scalarA : cxx_double, vectorB : DVector3dP, scalarB : cxx_double, vectorC : DVector3dP, scalarC : cxx_double) : DPoint3dP;

/** Return a (full length) vector from this point to other */
        VectorTo(other : DPoint3dP): DVector3dP;
/** Return the distance from this point to other */
        Distance(other: DPoint3dP): cxx_double;
/** Return squared distance from this point to other */
        DistanceSquared (other : DPoint3dP) : cxx_double;        
/** Return the largest absolute distance between corresponding compontents */
        MaxAbsDiff(vectorB: DPoint3dP): cxx_double;
/** Return the largest absolute value of any component */
        MaxAbs (): cxx_double;

        OnDispose(): void;
        Dispose(): void;
    }

/**
* @description 2D point with X,Y properties.
*/
    class DPoint2d implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDPoint2d ***/ 
        /** Constructor from x,y coordinates */
        constructor(x: cxx_double, y: cxx_double);
        Clone () : DPoint2dP;
/** Read/Write access to X component */
        X: cxx_double;
/** Read/Write access to Y component */
        Y: cxx_double;
        OnDispose(): void;
        Dispose(): void;

        Interpolate(fraction: cxx_double, right: DPoint2dP): DPoint2dP;
/** return point plus vector. */
        Plus(vector: DVector2dP): DPoint2dP;
/** Return point minus vector */
        Minus(vector: DVector2dP): DPoint2dP;
/** return point + vector * scalar */
        PlusScaled(vector: DVector2dP, scalar: cxx_double): DPoint2dP;
/** return point + vectorA * scalarA + vectorB * scalarB */
        Plus2Scaled(vectorA: DVector2dP, scalarA: cxx_double, vectorB: DVector2dP, scalarB: cxx_double): DPoint2dP;
/** return point + vectorA * scalarA + vectorB * scalarB + vectorC * scalarC */
        Plus3Scaled(vectorA: DVector2dP, scalarA: cxx_double, vectorB: DVector2dP, scalarB: cxx_double, vectorC: DVector2dP, scalarC: cxx_double): DPoint2dP;

        VectorTo (other: DPoint2dP) : DVector2dP;

/** Return a (full length) vector from this point to other */
        Distance(other: DPoint2dP): cxx_double;
/** Return squared distance from this point to other */
        DistanceSquared(other: DPoint2dP): cxx_double;        
/** Return the largest absolute distance between corresponding compontents */
        MaxAbsDiff(vectorB: DPoint2dP): cxx_double;
/** Return the largest absolute value of any component */
        MaxAbs(): cxx_double;

    }


    type DPoint3dP = cxx_pointer<DPoint3d>;
    type DPoint2dP = cxx_pointer<DPoint2d>;
/**
* @description 3D vector with X,Y,Z properties.
*/
    class DVector3d implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDVector3d ***/ 
        /** Constructor from x,y,z components */
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

/**
* @description 2D vector with X,Y properties.
*/
    class DVector2d implements IDisposable
    {
        /*** NATIVE_TYPE_NAME = JsDVector2d ***/ 
        /** Constructor from x,y,z parts */
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



/**
* @description Carrier for Yaw, Pitch, and roll angles.
*/
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

    
/**
@description A strongly typed angle, with explicitly named access to degrees and radians
*/
    class Angle implements IDisposable, BeJsProjection_SuppressConstructor {
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

/**
@description An axis-aligned range box with Low and High points.
*/
    class DRange3d implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDRange3d ***/ 
        //! constructor for empty range.
        constructor();
        Clone () : DRange3dP;
        
        Low: DPoint3dP;
        High: DPoint3dP;

/** Test if the box is in a null (just initialized) condition.  Note that a range around a single point is NOT null. */
        IsNull(): cxx_bool;
/** Test of the range contains a single point. */
        IsSinglePoint(): cxx_bool;
/** Length of the box in the x direction */
        XLength(): cxx_double;
/** Length of the box in the y direction */
        YLength(): cxx_double;
/** Length of the box in the z direction */
        ZLength(): cxx_double;
/** Largest absolute value among any coordinates in the box corners. */
        MaxAbs(): cxx_double;
/** Test if the x direction size is nearly zero */
        IsAlmostZeroX(): cxx_bool;
/** Test if the x direction size is nearly zero */
        IsAlmostZeroY(): cxx_bool;
/** Test if the x direction size is nearly zero */
        IsAlmostZeroZ(): cxx_bool;

/** Test if a point given as x,y,z is within the range. */
        ContainsXYZ(x: cxx_double, y: cxx_double, z: cxx_double): cxx_bool;
/** Test if a point is within the range. */
        ContainsPoint(point: DPoint3dP): cxx_bool;
/** Test if the x,y coordinates of a point are within the range. */
        ContainsPointXY(point: DPoint3dP): cxx_bool;
/** Test of other range is within this range */
        ContainsRange(other: DRange3dP): cxx_bool;

/** Test if there is any intersection with other range */        
        IntersectsRange(other: DRange3dP): cxx_bool;

/** Return 0 if the point is within the range, otherwise the distance to the closest face or corner */
        DistanceToPoint(point: DPoint3dP): cxx_double;
/** Return 0 if the ranges overlap, otherwise the smallest distance between. */
        DistanceToRange(other: DRange3dP): cxx_double;

/** Expand this range by distance a (possbily signed) in all directions */
        ExtendAllDirections(a: cxx_double): void;
/** Expand this range by distances a (possbily signed) in all directions */
        ExtendXYZ(x: cxx_double, y: cxx_double, z: cxx_double): void;
/** Expand this range to include a point. */
        ExtendPoint(point: DPoint3dP): void;
/** Expand this range to include a range. */
        ExtendRange(range: DRange3dP): void;

/** Return the intersection of ranges. */
        Intersect(other: DRange3dP): DRange3dP;
/** Return the union of ranges. */
        Union(other: DRange3dP): DRange3dP;
/** Put this range in a null state */
        Init(): void;

        OnDispose(): void;
        Dispose(): void;
    }

    type DRange3dP = cxx_pointer<DRange3d>;


/**
@description A ray defined by origin and vector.
*/
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

/**
@description A plane defined by origin and vector normal
*/
    class DPlane3d implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDPlane3d ***/ 
        Clone(): DPlane3dP;
        constructor (
                Origin : DPoint3dP,
                Normal: DVector3dP
                );
        
        Origin: DPoint3dP;
        Normal: DVector3dP;
        OnDispose(): void;
        Dispose(): void;
        /** Evalate (xyz-origin) DOT normal. */
        Evaluate (xyz : DPoint3dP) : cxx_double;
    }

    type DPlane3dP = cxx_pointer<DPlane3d>;






    type DPoint3dArrayP = cxx_pointer<DPoint3dArray>;

/**
@description An array of DPoint3d.
*/
    class DPoint3dArray implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDPoint3dArray ***/ 
        Clone(): DPoint3dArrayP;
        constructor();

        Add(value: DPoint3dP): void;
        AddXYZ (x: cxx_double, y: cxx_double, z: cxx_double): void;
        AddXY(x: cxx_double, y: cxx_double): void;
        TransformInPlace (transform: TransformP): void;
        Size(): cxx_double;
        Clear (): void;
        Append(other: DPoint3dArrayP): void;

        At(index: cxx_double): DPoint3dP;

        OnDispose(): void;
        Dispose(): void;
    }

    type GeometryNodeP = cxx_pointer<GeometryNode>;

/**
@description A GeoemtryNode is a a node in a tree or DAG structure.
<ul>
<li>Each node has two arrays of "children":
<ul>
<li>Geometry -- an array of geometry
<li>Members -- an array of child GeometryNodes.  Each member has a transform to be applied to the chid node.
</ul>
<li>Recursively following the Member pointers traces the tree or directed acyclic graph (DAG).
<li>The structure must be acyclic. (I.e. the recursive searches do not check for revisit, and will get stuck in endless loops if there are cycles.)
<li>A single node with no child members is essentially an array of Geometry.
</ul>
*/
    class GeometryNode implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsGeometryNode ***/ 
        constructor();

        /** Add a child geometry */
        AddGeometry(value: GeometryP): void;
        /** Add a child node */
        AddMemberWithTransform (value: GeometryNodeP, transform: TransformP): void;
        /** return the numer of child geometry */
        GeometrySize(): cxx_double;
        /** return the number of child nodes */
        MemberSize(): cxx_double;
        /** Clear the geometry array */
        ClearGeometry (): void;
        /** Clear the child node array*/
        ClearMembers (): void;
        /** access geometry by index */
        GeometryAt(index: cxx_double): GeometryP;
        /** access child nodes by index*/
        MemberAt(index: cxx_double): GeometryNodeP;
        /** access child transform by index */
        MemberTransformAt(index: cxx_double): TransformP;

        /** Collect transformed clones all geometry in the tree (or dag).  The geometry is returned in a geometry node
            with identity transform and no node children.
        **/            
        Flatten ():GeometryNodeP;
        /** Test for identical structure and coordinates */
        IsSameStructureAndGeometry (other: GeometryNodeP) : cxx_bool;
        OnDispose(): void;
        Dispose(): void;
    }



    type DoubleArrayP = cxx_pointer<DoubleArray>;

/**
@description An array of doubles.
*/
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

/**
@description A 3x3 matrix, as used for rotations.  (But scaled and skewed vectors are also possible.)
*/
    class RotMatrix implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsRotMatrix ***/ 
        constructor();
        constructor
            (
            axx : cxx_double, axy : cxx_double, axz : cxx_double,
            ayx : cxx_double, ayy : cxx_double, ayz : cxx_double,
            azx : cxx_double, azy : cxx_double, azz : cxx_double
            );
/** Create a rotation of specified angle around an axis */
        static CreateRotationAroundVector(axis: DVector3dP, angle: AngleP): RotMatrixP;
/** Create a 90 degree rotation around an axis */
        static Create90DegreeRotationAroundVector(axis: DVector3dP): RotMatrixP;
/** Create an identity matrix */
        static CreateIdentity () : RotMatrixP;
/** Create a matrix with uniform scale factors */
        static CreateUniformScale (scaleFactor : cxx_double) : RotMatrixP;
/** Create a matrix by direct list of values going across rows. */
        static CreateRowValues(x00: cxx_double, x01: cxx_double, x02: cxx_double,
            x10: cxx_double, x11: cxx_double, x12: cxx_double,
            x20: cxx_double, x21: cxx_double, x22: cxx_double): RotMatrixP;

/** Create a clone */
        Clone () : RotMatrixP;
/** Return the X column */
        ColumnX : DVector3dP;        
/** Return the Y column */
        ColumnY : DVector3dP;        
/** Return the Z column */
        ColumnZ : DVector3dP;        

/** Return the X row */
        RowX: DVector3dP;        
/** Return the Y row */
        RowY: DVector3dP;        
/** Return the Z row */
        RowZ: DVector3dP;        

/** Create a matrix with specific scales in each principal direction */
        static CreateScale (scaleX : cxx_double, scaleY : cxx_double, scaleZ : cxx_double) : RotMatrixP;
/** Create a matrix from column vectors. */
        static CreateColumns (vectorU : DVector3dP, vectorV : DVector3dP, vectorZ : DVector3dP) : RotMatrixP;
/** Create a matrix from row vectors. */
        static CreateRows (vectorU : DVector3dP, vectorV : DVector3dP, vectorZ : DVector3dP) : RotMatrixP;

        
/** Create a matrix that scales along a specified direction. The scale factor can be negative; for instance scale of -1.0 (negative one) is a mirror. */
        static CreateDirectionalScale (direction : DVector3dP, scale : cxx_double) : RotMatrixP;
/** Create a matrix with the indicated column in the (normalized) direction, and the other two columns perpendicular. All columns are normalized.
<ul>
<li>The direction vector is normalized and appears in column axisIndex
<li>If the direction vector is not close to Z, the "next" column ((axisIndex + 1) mod 3) will be in the XY plane in the direction of (direction cross Z)
<li>If the dirction vector is close to Z, the "next" column ((axisIndex + 1) mode 3) will be in the direction of (direction cross Y)
</ul>
*/
        static Create1Vector (direction : DVector3dP, axisIndex : cxx_double) : RotMatrixP;
        static CreateFromXYVectors (vectorX : DVector3dP, vectorY : DVector3dP, axisIndex : cxx_double) : RotMatrixP;
/** Multiply the matrix * vector, i.e. the vector is a column vector on the right. 
    @return the vector result
*/
        MultiplyVector (vector : DVector3dP) : DVector3dP;        
/** Multiply the vector * matrix, i.e. the vector is a row vector on the left.
    @return the vector result
*/
        MultiplyTransposeVector (vector : DVector3dP) : DVector3dP;        

/** Multiply the matrix * (x,y,z), i.e. the vector (x,y,z) is a column vector on the right.
    @return the vector result
*/
        MultiplyXYZ (x : cxx_double, y : cxx_double, z : cxx_double) : DVector3dP;        
/** Multiply the (x,y,z) * matrix, i.e. the vector (x,y,z) is a row vector on the left.
    @return the vector result
*/
        MultiplyTransposeXYZ ( x : cxx_double, y : cxx_double, z : cxx_double) : DVector3dP;        

        Solve (rightHandSide : DVector3dP) : DVector3dP;
/** Mulitply two matrices.
    @return the matrix result
*/
         MultiplyMatrix (other : RotMatrixP) : RotMatrixP;

/** return the transposed matrix */
         Transpose () : RotMatrixP;
/** return the inverse matrix.  The return is  null if the matrix is singular (has columns that are coplanar or colinear) */
         Inverse () : RotMatrixP;
/** return the entry at specific row and column */
        At(row : cxx_double, column : cxx_double) : cxx_double;
/** Set the entry at specific row and column */
        SetAt (row : cxx_double, column : cxx_double, value : cxx_double) : void;
/** Scale columns by respective factors */            
        ScaleColumnsInPlace (scaleX : cxx_double, scaleY : cxx_double, scaleZ : cxx_double) : void;
/** Scale rows by respective factors */
        ScaleRowsInPlace (scaleX : cxx_double, scaleY : cxx_double, scaleZ : cxx_double) : void;

/** Return the determinant */
        Determinant () : cxx_double;
/** Return an estimate of how independent the columns are.  Near zero is bad. */
        ConditionNumber () : cxx_double;
/** Return the sum of squares of all entries */
        SumSquares () : cxx_double;
/** Return the sum of squares of diagonal entries */
        SumDiagonalSquares () : cxx_double;
/** Return the Maximum absolute value of any single entry */
        MaxAbs () : cxx_double;
/** Return the maximum absolute difference between corresponding entries */
        MaxDiff (other : RotMatrixP) : cxx_double;
/** Test if the matrix is (very near to) an identity */        
        IsIdentity () : cxx_bool;
/** Test if the off diagonal entries are all nearly zero */
        IsDiagonal () : cxx_bool;
/** Test if the matrix is shuffles and negates columns. */
        IsSignedPermutation () : cxx_bool;
/** Test if the matrix is a pure rotation. */
        IsRigid () : cxx_bool;
/** Test if all rows and columns are length 1 and are perpendicular to each other.  (I.e. the matrix is either a pure rotation with uniform scale factor of 1 or -1) */
        HasUnitLengthMutuallyPerpendicularRowsAndColumns () : cxx_bool;


        OnDispose(): void;
        Dispose(): void;
    }

    type TransformP = cxx_pointer<Transform>;
/**
@description A 3x4 matrix combining a RotMatrix and point for translation, rotation, scale and skew transformations
*/
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
        /** Create a coordinate frame with x,y,z columns given directly */
        static CreateOriginAndVectors (origin : DPoint3dP, xVector : DVector3dP, yVector : DVector3dP, zVector : DVector3dP) : TransformP;
        /** Create a coordinate frame with x,y,z columns as vectors from origin to target points */
        static CreateOriginAnd3TargetPoints (origin : DPoint3dP, xPoint : DPoint3dP, yPoint : DPoint3dP, zPoint : DPoint3dP) : TransformP;
        /** Create a transform that sweeps points along the sweepDirection to a point on the plane */
        static CreateSweepToPlane (sweepDirection: DVector3dP, plane : DPlane3dP) : TransformP;


        static CreateOriginAnd2TargetPoints (origin : DPoint3dP, xPoint : DPoint3dP, yPoint : DPoint3dP) : TransformP;

        static CreateRotationAroundRay (ray : DRay3dP, angle : AngleP) : TransformP;                
        
        /** Create a normalized frame.
        <ul>
        <li>The given origin is the translation part of the transform.
        <li>The unit vector from origin towards target appears as the axisIndex column.
        <li>the other two columns are perpendicular to that, as described in RotMatix.Create1Vector
        </ul>
        */
        static CreateNormalizedFrameOriginTarget (origin : DPoint3dP, target : DPoint3dP, axisIndex : cxx_double) : TransformP;

        
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

    //! A wrapper for BentleyApi::DPoint3dDVec3dDVec3d
    class DPoint3dDVector3dDVector3d implements IDisposable {
        /*** NATIVE_TYPE_NAME = JsDPoint3dDVector3dDVector3d ***/ 
        constructor(origin: DPoint3dP, vectorU: DVector3dP, vectorV: DVector3dP)
        Evaluate (u: cxx_double, v: cxx_double) : DPoint3dP;
        OnDispose(): void;
        Dispose(): void;

        GetOrigin () : DPoint3dP;
        GetVectorU (): DVector3dP;
        GetVectorV (): DVector3dP;
        /** Return the transform from the skew plane to world.  This X and Y axes of this transform have both the length and diretction of VectorU and VectorV -- i.e. it can be skewed.   The Z axis is a unit normal. */
        GetLocalToWorldTransform (): TransformP;
        /** Return the transform from the world to the (possibly skewed) plane system. */
        GetWorldToLocalTransform (): TransformP;
        /** Return the transform with unit X vector along VectorU, unitY perpendicular and in the plane of VectorU and VectorV, and unitZ perpendicular. */
        GetNormalizedLocalToWorldTransform (): TransformP;
        /** Return the transform from world to the NormalizedLocal frame */
        GetWorldToNormalizedLocalTransform (): TransformP;
    }

    type DPoint3dDVector3dDVector3dP = cxx_pointer<DPoint3dDVector3dDVector3d>;



/**
@description Base class for many geometry types.  The geometry types have this inheritance structure:
<ul>
<li>CurvePrimitive -- a curve with parameterization from fractional coordinates 0 to 1.
    <ul>
    <li>LineSegment
    <li>EllipticArc
    <li>BsplineCurve
    <li>CatenaryCurve
    <li>SpiralCurve
    </ul>
<li>CurveVector
    <ul>
    <li>Path -- curves with head-to-tail sequence.
    <li>Loop -- single loop on a plane
    <li>ParityRegion -- multiple loops on a plane, with the region inside and outside defined by parity rules.
    <li>UnionRegion -- Nested planar regions combined as a union of the individual regions.
    <li>UnstructuredCurves -- curves collected in an array with no expected head-to-tail sequence or loop structure.
    </ul>
<li>SolidPrimitive -- specialized surfaces and solids
    <ul>
    <li>DgnBox -- a box or view frustum
    <li>DgnSphere -- a slice of a sphere
    <li>DgnCone -- a cone or cylinder
    <li>DgnTorusPipe -- a portion of torus
    <li>DgnExtrusion -- a linear sweep of a base curve or region
    <li>DgnRotationalSweep -- a rotational sweep of a base curve or region
    <li>DgnRuledSweep -- a ruled surface between contours.
    </ul>
</ul>
*/
class Geometry implements IDisposable, BeJsProjection_SuppressConstructor
{
    /*** NATIVE_TYPE_NAME = JsGeometry ***/ 
Clone (): GeometryP;
    OnDispose(): void;
    Dispose(): void;
    /** Try to apply a transform to this geometry. */
    TryTransformInPlace (transform: TransformP) : cxx_bool;

    /** Test if the contained element types match (but without comparing any coordinates).*/
    IsSameStructure (other: GeometryP) : cxx_bool;
    /** Test if the contained element types and coordinates match.*/
    IsSameStructureAndGeometry (other: GeometryP) : cxx_bool;
}
type GeometryP = cxx_pointer<Geometry>;
   
/**
@description Data bundle describing fractional and xyz coordinates of a point on a CurvePrimitive
*/
class CurveLocationDetail implements BeJsProjection_SuppressConstructor
{
/*** NATIVE_TYPE_NAME = JsCurveLocationDetail ***/
/** Fractional position along the curve. */
Fraction:cxx_double;
/** point coordinates */
Point: DPoint3dP;
/** the curve primitive */
Curve: CurvePrimitiveP;
/** a numeric field with context-specific meaning.*/
A:cxx_double; 

}

type CurveLocationDetailP = cxx_pointer<CurveLocationDetail>;
/**
@description Intermediate base class for various curve types.
<ul>
<li>LineSegment
<li>LineString
<li>EllipticArc
<li>BsplineCurve
</ul>

All curves have a fractional coordinate.
<ul>
<li>Fraction coordinate 0.0 (zero) is the start of the curve.
<li>Fraction coordinate 1.0 (one) is the end of the curve.
<li>Fractions between 0 and 1 progress monotonically along the curve.  Larger fractions are always at greater distance along the curve.
<li>However, the fractional coordinates may not be strictly proportional to distance.  For instance, they are strictly proportional for LineSegment and for EllipticArcs that are circular.
</ul>
*/
class CurvePrimitive extends Geometry implements BeJsProjection_SuppressConstructor
    {
    /*** NATIVE_TYPE_NAME = JsCurvePrimitive ***/ 
    Clone(): CurvePrimitiveP;
    CurvePrimitiveType(): cxx_double;
/** Return a frenet frame at a fractional position.
<ul>
<li> x axis is forward along the curve (tangent)
<li> y axis is towards the center of curvature.
<li> z axis is x cross y
</ul>
*/
FrenetFrameAtFraction (f : cxx_double) : TransformP;
    /** return the point at fractional position along the curve. */
    PointAtFraction(f: cxx_double): DPoint3dP; 
    /** return the point and unit tangent at fractional position along the curve. */
    PointAndUnitTangentAtFraction(f: cxx_double): DRay3dP; 
    /** return the point and derivative (tangent) at fractional position along the curve. */
    PointAndDerivativeAtFraction(f: cxx_double): DRay3dP; 

    /** return start point of the primitive.  Equivalent to PointAtFraction (0.0); */
    GetStartPoint () : DPoint3dP;
/** return end point of the primitive.  Equivalent to PointAtFraction (1.0); */
    GetEndPoint () : DPoint3dP; 
/** Find the closest point within the curve */
    ClosestPointBounded (spacePoint: DPoint3dP) : CurveLocationDetailP;
    }

    type CurvePrimitiveP = cxx_pointer<CurvePrimitive>;



    /** @description Line segment, defined by start and end points.
    */
    class LineSegment extends CurvePrimitive {
        /*** NATIVE_TYPE_NAME = JsLineSegment ***/
        Clone(): LineSegmentP;
        constructor (pointA : DPoint3dP, pointB : DPoint3dP);
        /** create with direct xyz values for start and end */
        static CreateXYZ (ax: cxx_double, ay: cxx_double, az: cxx_double, bx: cxx_double, by: cxx_double, bz: cxx_double) : LineSegmentP;

    }

    type LineSegmentP = cxx_pointer<LineSegment>;


    /** @description An array of points on a LineString */
    class LineString extends CurvePrimitive {
        /*** NATIVE_TYPE_NAME = JsLineString ***/
        Clone(): LineStringP;
        constructor (points: DPoint3dArrayP);
        /** Return the points as an array */
        GetPoints () : DPoint3dArrayP;
    }

    type LineStringP = cxx_pointer<LineString>;



/** A circular or elliptic arc.
<ul>
<li>The equation for stroking the arc as a function of angle theta is {xyzOnArc = xyzCenter + Vector0 * cos(theta) + Vector90 * sin (theta)}
<li>Theta 0 and 90 degree correspond to the Vector0 and Vector90 directions from the center.
<li>If Vector0 and Vector90 are perpendicular and have the same length, the ellipse is a circle.
<li>If Vector0 and Vector90 are perpendicular, those directions are the customary major and minor axis points.
<li>Nearly all 'user created' ellipses will have perpendicular axes.   However, ellipses affected by skew transformations can have non-perpendicular vectors.
This property is extremely convenient for passing ellipses through viewing transformations.
</ul>
*/
    class EllipticArc extends CurvePrimitive implements BeJsProjection_SuppressConstructor
        {
        /*** NATIVE_TYPE_NAME = JsEllipticArc ***/
        Clone(): EllipticArcP;
        constructor(center: DPoint3dP, vector0: DVector3dP, vector90: DVector3dP, startAngle: AngleP, sweepAngle : AngleP);
        static CreateCircleStartMidEnd (startPoint: DPoint3dP, interiorPoint: DPoint3dP, endPoint: DPoint3dP) : EllipticArcP;
        static CreateCircleXY (center: DPoint3dP, radius: cxx_double) : EllipticArcP;
        /** Create a circular arc that fillets (replaces pointB) in the path pointA to pointB to pointC. The radius is given,
                so the tangency point can be outside the distance to pointA or pointC */
        static CreateFilletAtMiddlePoint (pointA: DPoint3dP, pointB: DPoint3dP, pointC: DPoint3dP, radius: cxx_double) : EllipticArcP;
        /** Create the largest arc that fillets the corner at pointB and has tangencies no further away than pointA and pointC */
        static CreateLargestFilletAtMiddlePoint (pointA: DPoint3dP, pointB: DPoint3dP, pointC: DPoint3dP) : EllipticArcP;

        /** Create a circular arc from a start point, initial direction, and plane normal vector. */
        static CreateStartTangentNormalRadiusSweep (
                startPoint: DPoint3dP, 
                startTangent: DVector3dP,
                planeNormal: DVector3dP,
                radius: cxx_double,
                sweep: AngleP
                ) : EllipticArcP;
        //! Return the frenet frame of the arc fraction, but with translation to arc center.
        CenterFrameAtFraction (fraction : cxx_double) : TransformP;

        GetCenter (): DPoint3dP;
        GetVector0 (): DVector3dP;
        GetVector90 (): DVector3dP;
        GetStartAngle (): AngleP;
        GetSweepAngle(): AngleP;
        GetEndAngle (): AngleP;
        /** Clone the ellipse, but modify vectors and angles so that the vectors are the customary perpendicular axes */
        CloneWithPerpendicularAxes () :EllipticArcP;
        /** Return center, Vector0 and Vector90 as a basis plane. Note that these are not unit vectors. */
        GetBasisPlane () : DPoint3dDVector3dDVector3dP;
        /** Test if the ellipse is circular.   (If so, the radius is GetVector0 ().Magnitude ()) */
        IsCircular () : cxx_bool
    }

    type EllipticArcP = cxx_pointer<EllipticArc>;


    //! A wrapper for a bspline curve
    //! Because curve contruction is error-prone and proper constructors cannot indicate errors, 
    //!   curve creation is through the static methods.
    //!  , BeJsProjection_SuppressConstructor
    class BsplineCurve extends CurvePrimitive implements BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsBsplineCurve ***/
        constructor ();
        Clone(): BsplineCurveP;

        IsPeriodic(): cxx_bool;
        static Create(xyz: DPoint3dArrayP,
            weights: DoubleArrayP,
            knots: DoubleArrayP,
            order: cxx_double, closed: cxx_bool, preWeighted: cxx_bool): BsplineCurveP;
        static CreateFromPoles(xyz: DPoint3dArrayP, order: cxx_double): BsplineCurveP;

    }

    type BsplineCurveP = cxx_pointer<BsplineCurve>;

    //! A wrapper for a catenary curve
    //!<ul>
    //!<li>In the local coordinate system the curve is y = a * cosh(x/a)
    //!</ul>
    class CatenaryCurve extends CurvePrimitive implements BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsCatenaryCurve ***/
        constructor ();
        Clone(): CatenaryCurveP;
        // Create a catenary curve:   xyz = origin + x * xVector + yVector * a * cosh(x/a)        
        static CreateFromCoefficientAndXLimits (origin: DPoint3dP, xVector: DVector3dP, yVector: DVector3dP, a: cxx_double, xStart: cxx_double, xEnd: cxx_double) : CatenaryCurveP;
    }
    type CatenaryCurveP = cxx_pointer<CatenaryCurve>;


    //! A wrapper for a Spiral curve
    //!<ul>
    //!<li>In the local coordinate system the curve is y = a * cosh(x/a)
    //!<li>Spirals can be one of these transition types
    //!<ul>
    //!<li>10: Clothoid
    //!<li>11: Bloss
    //!<li>12: Biquadratic
    //!<li>13: Cosine
    //!<li>14: Sine
    //!</ul>
    //!</ul>
    class SpiralCurve extends CurvePrimitive implements BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsSpiralCurve ***/
        constructor ();
        Clone(): SpiralCurveP;

    //! Create a spiral with radius and curvature at start, length along the spiral, and radius at end.
    //! @param spiralType [in] transition selector -- see SpiralCurve summary
    //! @param startBearing [in] direction (in xy plane) at the start of the curve.
    //! @param startRadius [in] radius of curvature at start.  (0 radius is straight line)
    //! @param length [in] length of full spiral curve
    //! @param endRadius [in] radius at end of the full spiral
    //! @param frame [in] coordinate frame with origin at start of full spiral, bearing angle 0 along x axis.
    //! @param fractionA [in] fractional position (distance along) at start of active subset of the spiral.  This is nearly always 0.0.
    //! @param fractionB [in] fractional position (distance along) at end of active subset of the spiral.  This is nearly always 1.0.
        static CreateSpiralBearingRadiusLengthRadius (
            spiralType: cxx_double,
            startBearing: AngleP,
            startRadius: cxx_double,
            length:     cxx_double,
            endRadius: cxx_double,
            frame: TransformP,
            fractionA: cxx_double,
            fractionB: cxx_double
            ) : SpiralCurveP;
    }

    type SpiralCurveP = cxx_pointer<SpiralCurve>;



/**
@description A CurveVector is a collection of curves.
<ul>
<li>An object the base type CurveVector is never instantated -- only derived types will be instantiated.
<li>There derived classes for various particular structures built of multiple curves:
    <ul>
    <li>UnstructuredCurves -- curves collected in an array with no expected head-to-tail sequence or loop structure.
    <li>Path -- curves with head-to-tail sequence.
    <li>PlanarRegion -- curves that are coplanar and appear in loops that bound regions.
    </ul>
</ul>
//!
*/
    class CurveVector extends Geometry implements BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsCurveVector ***/
        Clone(): CurveVectorP;
        BoundaryType(): cxx_double;
        /** Access an [index] member of the curve vector.
        <ul>
        <li>If the member at this index is a child curve vector, the return is null
        <li>If the member at this index is a CurvePrimitive, return the primitive.
        </ul>
        */
        MemberAsCurvePrimitive (index: cxx_double) :CurvePrimitiveP;
        /** Access an [index] member of the curve vector.
        <ul>
        <li>If the member at this index is a (child) CurveVector, return the curve vector
        <li>If the member at this index is a CurvePrimitive, return null.
        </ul>
        */
        MemberAsCurveVector (index: cxx_double) :CurveVectorP;
    }

    type CurveVectorP = cxx_pointer<CurveVector>;
/** Subclass for CurveVectors that are planar regions.  These come as
<ul>
<li>Loop -- Coplanar curves that form a single loop.
        <ul><li>A Loop may contain only CurvePrimitive objects as childeren.</ul>
<li>ParityRegion -- one or more coplanar curves that bound area by parity rules.
        <ul><li>A ParityRegion may contain only Loop objects as childeren.</ul>
<li>UnionRegion -- one or more loops and ParityRegions
        <ul><li>A UnionRegion may contain an PlanarRegion object (Loop, ParityRegion, UnionRegion) as childeren.</ul>
</ul>
*/
class PlanarRegion extends CurveVector implements BeJsProjection_SuppressConstructor
    {
    /*** NATIVE_TYPE_NAME = JsPlanarRegion ***/
    Area ():cxx_double;
    }

    type PlanarRegionP = cxx_pointer<PlanarRegion>;

/** A collection of CurvePrimitive's that join head to tail.
    <ul><li>A Path may contain only CurvePrimitive objects as childeren.</ul>
*/
    class Path extends CurveVector {
        /*** NATIVE_TYPE_NAME = JsPath ***/
        constructor();
        Clone(): PathP;
        /** Add a curve primtive to this curve vector.
        */
        Add(primitive: CurvePrimitiveP): void;
        /** Create a Path with a single initial curve.  Additional curves can be added later. */
        static Create1 (curve: CurvePrimitiveP) : PathP;
        /** Create a Path with a two initial curves.  Additional curves can be added later. */
        static Create2 (curve1: CurvePrimitiveP, curve2: CurvePrimitiveP) : PathP;
    }

    type PathP = cxx_pointer<Path>;

    /** A collection of CurvePrimitive's that join head to tail and form a complete closed loop.
    <ul><li>A Loop may contain only CurvePrimitive objects as childeren.</ul>
    */
    class Loop extends PlanarRegion {
        /*** NATIVE_TYPE_NAME = JsLoop ***/
        Clone(): LoopP;
        /** Create an empty loop.  Use "Add" to insert curves.*/
        constructor();
        /** Add a curve primtive to this curve vector. */
        Add(primitive: CurvePrimitiveP): void;
        /** Create a loop with a single initial curve.  Additional curves can be added later. */
        static Create1 (curve: CurvePrimitiveP) : LoopP;
        /** Create a loop with a two initial curves.  Additional curves can be added later. */
        static Create2 (curve1: CurvePrimitiveP, curve2: CurvePrimitiveP) : LoopP;
        /** Create a regular polygon in a plane parallel to the xy plane
        @param center [in] center of polygon
        @param xDistance [in] x-direction distance from center to a polygon point.
        @param numEdges [in] number of edges (and vertices)
        @param isOuterRadius [in] if true, the xDistance is the outer radius, i.e. there is vertex on the x axis point.
               If false, xDistance is the inner radius, i.e. there is Y-direction edge crossing the x axis point.
        */
        static CreateRegularPolygonXY (center: DPoint3dP, xDistance: cxx_double, numEdges: cxx_double, isOuterRadius: cxx_bool) : LoopP;
    }

    type LoopP = cxx_pointer<Loop>;
    /**  A collection of CurvePrimitive's and CurveVector's.
    */
    class UnstructuredCurves extends CurveVector {
        /*** NATIVE_TYPE_NAME = JsUnstructuredCurveVector ***/
        Clone(): UnstructuredCurvesP;
        constructor();
        /** Add a curve primtive to this CurveVector
        */
        Add(primitive: CurvePrimitiveP): void;
        /** Add a (child) CurveVector to this CurveVector.  */
        Add(curveVector: CurveVectorP): void;
    }

    type UnstructuredCurvesP = cxx_pointer<UnstructuredCurves>;

    /** A collection of coplanar Loop objects.
    <ul>
    <li>The contained loopsare expected to be coplanar.
    <li>For planar area analysis, the contained objects are interpretted by parity rules.
    </ul>
    */
    class ParityRegion extends PlanarRegion{
        /*** NATIVE_TYPE_NAME = JsParityRegion ***/
        Clone(): ParityRegionP;
        /** Create an empty parity region.  Use "Add" to insert loops.*/
        constructor();
        Add(loop: LoopP): void;
        /** Create a region with a single initial loop.  Additional loops can be added later. */
        static Create1 (loop: LoopP) : ParityRegionP;
        /** Create a region with a two initial loops.  Additional loops can be added later. */
        static Create2 (loop1: LoopP, loop2: LoopP) : ParityRegionP;
    }

    type ParityRegionP = cxx_pointer<ParityRegion>;

    /** A collection of Loop and ParityRegion objects.
    <ul>
    <li>The contained objects are expected to be coplanar.
    <li>For planar area analysis, the contained objects are interpretted as a union of the individual regions.
    </ul>
    */
    class UnionRegion extends PlanarRegion {
        /*** NATIVE_TYPE_NAME = JsUnionRegion ***/
        Clone(): UnionRegionP;
        constructor();
        Add(child: CurveVectorP): void;
    }

    type UnionRegionP = cxx_pointer<UnionRegion>;




/**
A PolyfaceMesh is a collection of coordinates and indices defining a mesh structure.

<ul>
<li>Index data
    <ul>
    <li>PointIndices
        <ul>
        <li>A zero index in a VariableSizeIndexed mesh indicates that this is the end of the index block for this facet.
        <li>A zero index in a FixedIndex mesh indicates that this and all further indices within this facet's index block
                are unused.
        <li>All non-zero indices are
            <ul>
            <li>"Signed"
                <ul>
                <li>a positive index indicates that the following edge is to be displayed in wireframe mode.
                <li>a negative index indicates the the succeeeding edge is not to be displayed in wireframe mode.
                </ul>
            <li>"One Based" -- the point with programmatic index [0] in the Point array is stored as index value 1, and
                the point with programmatic zero based index [i] is stored as (i+1) in the index array.
            </ul>
        </ul>
    <li>Normal, Param, and Color Indices
        <ul>
        <li>For both fixed and variable blocking, the number of indices in the Normal, Param, and Color index arrays
            <ul>
            <li>must have exactly the same number of entries
            <li>must have zeros in exactly the same places.
            <li>are "one" based.
            </ul>
        <li>Normal, Param, and Color indices must all be zero or positive.  (i.e. there is no special meaning for negative indices.)
        </ul>
    </ul>
</ul>
*/
class PolyfaceMesh extends Geometry implements BeJsProjection_SuppressConstructor 
    {
    /*** NATIVE_TYPE_NAME = JsPolyfaceMesh ***/ 
    Clone(): PolyfaceMeshP;

    GetTwoSided () : cxx_bool;
    /** Create a mesh in which
    <ul>
     <li>data order around facets is controlled by integer index data that is separate from the arrays of
        coordinate, normal, texture parameters, and color.
    <li>Each individual facet may have any number of vertices.
    <li>Vertex counts are indicated by zeros in the index array.
    </ul>
    */
   static CreateVariableSizeIndexed() : PolyfaceMeshP;
    
    /** Create a mesh in which
    <ul>
    <li>data order around facets is controlled by integer index data that is separate from the arrays of
        coordinate, normal, texture parameters, and color.
    <li>There is a mesh-wide maxiumum number of vertices per face.
    <li>Individual facets may have fewer than the maximum.
    <li>The index data is organized as fixed size blocks of indices.
    </ul>
    */
    static CreateFixedIndexed(aNumPerFace : cxx_double) : PolyfaceMeshP;
    /** Create a mesh in which
    <ul>
    <li>coordinate data arrays (Point, Normal, Param, Color) are all organized as rows of a grid
    <li>The number of Points (etc) in a row is specified to the contstructor.
    <li>A single four-sided facet (possibly nonplanar) is defined in each grid quad.
    <li>There are no index arrays
    </ul>
    */
    static CreateQuadGrid(numPerRow : cxx_double) : PolyfaceMeshP;

    /** Create a mesh in which
    <ul>
    <li>coordinate data arrays (Point, Normal, Param, Color) are all organized as rows of a grid
    <li>The number of Points (etc) in a row is specified to the contstructor.
    <li>Two triangles are defined in each quad of the grid.
    <li>There are no index arrays
    </ul>
    */
    static CreateTriangleGrid(numPerRow : cxx_double) : PolyfaceMeshP;


    /** Create a mesh in which
    <ul>
    <li>coordinate data arrays (Point, Normal, Param, Color) are all organized in blocks of 3
    <li>Each block of 3 defines a triangle.
    <li>There are no index arrays
    <li>Note that this is an extremely heavy memory user because shared vertex coordinates are replicated
            in the block for each the facets that use the vertex.
    </ul>
    */
    static CreateCoordinateTriangleMesh() : PolyfaceMeshP;

    /** Create a mesh in which
    <ul>
    <li>coordinate data arrays (Point, Normal, Param, Color) are all organized in blocks of 4
    <li>Each block of 3 defines a 4-sided facet.
    <li>There are no index arrays
    <li>Note that this is an extremely heavy memory user because shared vertex coordinates are replicated
            in the block for each the facets that use the vertex.
    </ul>
    */
    static CreateCoordinateQuadMesh() : PolyfaceMeshP;

    PolyfaceMeshStyle() : cxx_double;

    // Single-value AddXxx methods -- note that Append method on blocked arrays SetActive () : does !!!
    AddPoint(data : DPoint3dP) : void;

    AddNormal(data : DVector3dP) : void;
    AddParam(data : DPoint2dP) : void;
    AddIntColor(aColor : cxx_double) : void;
    /** Add an index to the PointIndex array.
    */
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


//! A non-instantiable wrapper for BentleyApi::JsSolidPrimitive
class SolidPrimitive extends Geometry implements BeJsProjection_SuppressConstructor {
    /*** NATIVE_TYPE_NAME = JsSolidPrimitive ***/
    Clone(): SolidPrimitiveP;
}
type SolidPrimitiveP = cxx_pointer<SolidPrimitive>;

    //! A wrapper for BentleyApi::JsDgnCone
    class DgnCone extends SolidPrimitive implements BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsDgnCone ***/
    
        static CreateCircularCone(
            centerA: DPoint3dP,
            centerB: DPoint3dP,
            radiusA: cxx_double,
            radiusB: cxx_double,
            capped: cxx_bool
            ): DgnConeP;

        static CreateCircularConeXYZ(
            ax : cxx_double,
            ay : cxx_double,
            az : cxx_double,
            bx : cxx_double,
            by : cxx_double,
            bz : cxx_double,
            radiusA: cxx_double,
            radiusB: cxx_double,
            capped: cxx_bool
            ): DgnConeP;
    }

type DgnConeP = cxx_pointer<DgnCone>;

    //! A wrapper for BentleyApi::JsDgnSphere
    class DgnSphere extends SolidPrimitive implements BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsDgnSphere ***/

        static CreateSphere(
            center: DPoint3dP,
            radius: cxx_double
            ): DgnSphereP;

    }

type DgnSphereP = cxx_pointer<DgnSphere>;

    //! A wrapper for BentleyApi::JsDgnBox
    class DgnBox extends SolidPrimitive implements BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsDgnBox ***/

        /**
         * Initialize box detail fields from center and size.
         * @param [in] center center of box in XYZ
         * @param [in] diagonalSize total diagonal lengths (i.e. x,y,z side lengths)
         * @param [in] capped true if closed top and bottom.
         */
        static CreateCenteredBox(center: DPoint3dP, diagonalSize: DVector3dP, capped: cxx_bool): DgnBoxP;

        static CreateBox(
            baseOrigin: DPoint3dP,
            topOrigin: DPoint3dP,
            unitX: DVector3dP,
            unitY: DVector3dP,
            baseX: cxx_double,
            baseY: cxx_double,
            topX: cxx_double,
            topY: cxx_double,
            capped: cxx_bool
            ): DgnBoxP;

    }

type DgnBoxP = cxx_pointer<DgnBox>;

    //! A wrapper for BentleyApi::JsDgnTorusPipe
    class DgnTorusPipe extends SolidPrimitive implements BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsDgnTorusPipe ***/

        static CreateTorusPipe(
            baseOrigin: DPoint3dP,
            unitX: DVector3dP,
            unitY: DVector3dP,
            majorRadius: cxx_double,
            minorRadius: cxx_double,
            sweep: AngleP,
            capped: cxx_bool
            ): DgnTorusPipeP;

        static CreateFromArc(
            arc: EllipticArcP,
            minorRadius: cxx_double,
            capped: cxx_bool
            ): DgnTorusPipeP;

    }

type DgnTorusPipeP=cxx_pointer<DgnTorusPipe>;

    //! A wrapper for BentleyApi::JsDgnExtrusion
    class DgnExtrusion extends SolidPrimitive implements BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnExtrusion ***/

        static Create (
            profile: CurveVectorP,
            vector: DVector3dP,
            capped: cxx_bool
            ): DgnExtrusionP;

    }

type DgnExtrusionP=cxx_pointer<DgnExtrusion>;


    //! A wrapper for BentleyApi::JsDgnRotationalSweep
    class DgnRotationalSweep extends SolidPrimitive implements BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnRotationalSweep ***/

        static Create(
            profile: CurveVectorP,
            center: DPoint3dP,
            axis: DVector3dP,
            sweep: AngleP,
            capped: cxx_bool
            ): DgnRotationalSweepP;

    }

type DgnRotationalSweepP=cxx_pointer<DgnRotationalSweep>;

    //! A wrapper for BentleyApi::DgnRuledSweep
    class DgnRuledSweep extends SolidPrimitive implements BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnRuledSweep ***/

        static Create(
            profiles: UnstructuredCurvesP,
            capped: cxx_bool
            ): DgnRuledSweepP;

    }

type DgnRuledSweepP=cxx_pointer<DgnRuledSweep>;



//! A wrapper for an array of partial curve detail pairs such as returned from curve-curve intersection methods
//!<ul>
//! <li>Each detail pair has an "A" curve and "B" curve.
//! <li>curve, start fraction, and end fraction can be indpendently accessed for each pair.
//! <li>each pair can be tested to see if it is a single point (e.g. simple intersection) or not (e.g. colinear sections of lines)
//!</ul>
class PartialCurveDetailPairArray implements IDisposable, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsPartialCurveDetailPairArray ***/

    //! @return the curve object for the "A" curve
    GetCurveA (index: cxx_double) : CurvePrimitiveP;
    //! @return the "start fraction" for the "A" curve of indexed detail.
    GetFractionA0 (index: cxx_double) : cxx_double;
    //! @return the "end fraction" for the "A" curve of indexed detail.
    GetFractionA1 (index: cxx_double) : cxx_double;

    //! @return the curve object for the "B" curve
    GetCurveB (index: cxx_double) : CurvePrimitiveP;
    //! @return the "start fraction" for the "B" curve of indexed detail.
    GetFractionB0 (index: cxx_double) : cxx_double;
    //! @return the "end fraction" for the "B" curve of indexed detail.
    GetFractionB1 (index: cxx_double) : cxx_double;
    //! test if the indexed detail is a single point (simple curve intersection).
    IsSinglePointPair (index: cxx_double) : cxx_bool;


        OnDispose(): void;
        Dispose(): void;
    }

type PartialCurveDetailPairArrayP=cxx_pointer<PartialCurveDetailPairArray>;


//! A static class for operations involving two curves.
class CurveCurve implements IDisposable, BeJsProjection_SuppressConstructor
    {
    /*** NATIVE_TYPE_NAME = JsCurveCurve ***/
    //! Compute all intersections among curves, as viewed in XY.  (Intersctions may be single point or overlap of curve sections)
    //! @return array of descriptions of intersections.
    IntersectPrimitivesXY (
                curveA: CurvePrimitiveP,
                curveB: CurvePrimitiveP,
                extend: cxx_bool
                ): PartialCurveDetailPairArrayP;

        OnDispose(): void;
        Dispose(): void;
    }

type CurveCurveP=cxx_pointer<CurveCurve>;




}
