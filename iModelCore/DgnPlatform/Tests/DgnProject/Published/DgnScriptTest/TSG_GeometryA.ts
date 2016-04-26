//! Script that is executed by one of the unit tests in DgnScriptContext_Test.cpp
module DgnScriptTests {
    function logMessage(msg: string): void {
        Bentley.Dgn.Logging.Message('TestRunner', Bentley.Dgn.LoggingSeverity.Info, msg);
    }

    logMessage('Test1 A');
    
    var checker = new DgnScriptChecker.Checker();
    logMessage('Test1 B');
    // Test1 - DPoint3d properties
    function test1(): void {
        var point = new Bentley.Dgn.DPoint3d(2, 1, 3);
        if (point.X != 2)
            Bentley.Dgn.Script.ReportError('bad X');
        if (point.Y != 1)
            Bentley.Dgn.Script.ReportError('bad Y');
        if (point.Z != 3)
            Bentley.Dgn.Script.ReportError('bad Z');
    }

    function test2(): void
    {
        var pointA = new Bentley.Dgn.DPoint3d(2, 1, 3);
        var pointB = new Bentley.Dgn.DPoint3d(4, 0.4, 2);
        var curveD = new Bentley.Dgn.LineSegment (pointA, pointB);
        var pointA1=curveD.PointAtFraction(0.0);
        var pointB1 = curveD.PointAtFraction(1.0);
        checker.IsNearDPoint3d(pointA, pointA1);
        checker.IsNearDPoint3d(pointB, pointB1);
    }

    function test3(): void {
        var arrayA = new Bentley.Dgn.DPoint3dArray();
        arrayA.AddXYZ (1, 2, 3);
        arrayA.AddXYZ (2, 3, 4);
        arrayA.AddXYZ (4, 3, 4);
        checker.NearDouble(3, arrayA.Size(), true);
        var cpLineStringA = new Bentley.Dgn.LineString (arrayA);
        var pointB = cpLineStringA.PointAtFraction(0.5);


    }
    function Negate(vecA: Bentley.Dgn.DVector3d): void {
        var vecB = vecA.Clone ();
        //Fix
        var vecC = vecB.Negate();
        vecC = vecC.Negate();
        checker.IsNearDVector3d(vecC, vecA);
    }
    function FromStartEnd(pointA: Bentley.Dgn.DPoint3d, pointB: Bentley.Dgn.DPoint3d): void {
        var vectorA = new Bentley.Dgn.DVector3d(pointB.X - pointA.X, pointB.Y - pointA.Y, pointB.Z - pointA.Z);
        var resultA = pointA.VectorTo (pointB);
        checker.IsNearDVector3d(resultA, vectorA);
        var normalizedA = resultA.Normalize();
        var resultB = pointA.VectorTo (pointB);
        checker.NearDouble(normalizedA.Magnitude(), resultB.Magnitude(), true);

    }
    function Perpendicular(v: Bentley.Dgn.DVector3d): void {
        v.X = 0;
        var u = v.UnitPerpendicularXY(v);

        checker.NearDouble(0, v.DotProduct(u), true);

        checker.CheckBool(true, v.IsPerpendicularTo(u));
    }
    function Parallel(v: Bentley.Dgn.DVector3d): void {
        var u = v.Scale(2);
        checker.NearDouble(0, v.CrossProductMagnitude(u), true);
        checker.CheckBool(true, u.IsParallelTo(v));
    }
    function Adding(u: Bentley.Dgn.DVector3d, v: Bentley.Dgn.DVector3d, w: Bentley.Dgn.DVector3d): void {
        var vu = v.Plus(u);
        var uv = u.Plus(v);
        var v2 = uv.Minus (u);
        var uvFromAdd2 = u.PlusScaled (v, 2);
        checker.IsNearDVector3d(vu, uv);
        checker.IsNearDVector3d(v2, v);
        var uvAddScaled = u.PlusScaled (v, 2);
        checker.NearDouble(uvAddScaled.Magnitude(), uvFromAdd2.Magnitude(), true);
        var vuwFromAdd3 = u.Plus2Scaled (v, 1, w, 1);
        var uvw = uv.Plus (w);
        checker.IsNearDVector3d(vuwFromAdd3, uvw);
        vu = vu.ScaleToLength(2);
        uv = uv.Normalize();
        uv = uv.Scale(2);
        checker.NearDouble(uv.Magnitude(), vu.Magnitude(), true);


    }
    function CrossProducts(u: Bentley.Dgn.DVector3d, v: Bentley.Dgn.DVector3d): void {
        var VCrossU = v.CrossProduct(u);
        var NormalizedVCrossU = v.UnitCrossProduct(u);
        VCrossU = VCrossU.Normalize();
        checker.IsNearDVector3d(NormalizedVCrossU, VCrossU);

        VCrossU.SizedCrossProduct(v, u, 4);
        NormalizedVCrossU.Scale(4);
        checker.IsNearDVector3d(VCrossU, NormalizedVCrossU);
    }
    function Distances(u: Bentley.Dgn.DVector3d, v: Bentley.Dgn.DVector3d): void {
        var Dist = u.Distance(v);
        var uv = u.Minus (v);
        checker.NearDouble(Dist, uv.Magnitude(), true);
        var DistSq = u.DistanceSquared(v);
        var uvMagnitudeSq = uv.MagnitudeSquared();
        checker.NearDouble(DistSq, uvMagnitudeSq, true);
    }
    function XY(u: Bentley.Dgn.DVector3d, v: Bentley.Dgn.DVector3d): void {
        var uXY = new Bentley.Dgn.DVector3d(u.X, u.Y, 0);
        var vXY = new Bentley.Dgn.DVector3d(v.X, v.Y, 0);
        var uvXYDotProduct = uXY.DotProduct(vXY);
        var uvDotProductXY = u.DotProductXY(v);
        checker.NearDouble(uvXYDotProduct, uvDotProductXY, true);
        var VCrossUXY = v.CrossProductXY(u);
        var VCrossU = vXY.CrossProduct(uXY);
        checker.NearDouble(VCrossU.MagnitudeSquared(), VCrossUXY * VCrossUXY, true);

    }
    function Rotate(u: Bentley.Dgn.DVector3d): void {
        u.Z = 0;
        var degrees90 = Bentley.Dgn.Angle.CreateDegrees(90);
        var v = u.RotateXY(degrees90.Radians);
        checker.NearDouble(u.AngleTo(v).Radians, degrees90.Radians, true);
    }

    //Rotmatrix tests
    function IdentityMatrix(u :Bentley.Dgn.DVector3d, n: number): void {
        var identity = Bentley.Dgn.RotMatrix.CreateIdentity();
        var multiplied = identity.MultiplyVector(u);
        checker.IsNearDVector3d(u,multiplied);
        var identityX =  Bentley.Dgn.RotMatrix.CreateUniformScale(n);
        var identityScaledRows = identity.Clone ();
        var identityScaledColumns = identity.Clone ();
        identityScaledColumns.ScaleColumnsInPlace(n,n,n);
        identityScaledRows.ScaleRowsInPlace(n,n,n);
        checker.IsNearRotmatrix(identityX,identityScaledRows,true)
        checker.IsNearRotmatrix(identityX,identityScaledColumns,true);
        checker.CheckBool(identity.IsIdentity(),true);
    }
    function MultiplyMatrix(a : Bentley.Dgn.RotMatrix, b : Bentley.Dgn.RotMatrix): void {
    var product = a.MultiplyMatrix(b);
    var productTranspose = product.Transpose();
    var aTranspose = a.Transpose();
    var bTranspose = b.Transpose();
    var testProductTranspose = aTranspose.MultiplyMatrix(bTranspose);
    checker.IsNearRotmatrix(testProductTranspose,productTranspose,true);
    }
    function InverseMatrix(b : Bentley.Dgn.RotMatrix): void {
    var inverseB = b.Inverse();
    var testIdentity = b.MultiplyMatrix(inverseB);
    var identity = Bentley.Dgn.RotMatrix.CreateIdentity();
    checker.IsNearRotmatrix(identity,testIdentity,true);
    }
    function NinetyDegreeRotation(u :Bentley.Dgn.DVector3d): void {
    var a = Bentley.Dgn.RotMatrix.Create90DegreeRotationAroundVector(v);
    checker.CheckBool(a.IsRigid(),true);
    var degrees90 = Bentley.Dgn.Angle.CreateDegrees(90);
    var b = Bentley.Dgn.RotMatrix.CreateRotationAroundVector(v,degrees90)
    checker.IsNearRotmatrix(a,b,true);
    var v1 = a.MultiplyVector(v);
    var v2 = b.MultiplyVector(v);
    checker.IsNearDVector3d(v1,v2);

    }
    function Determinants(a : Bentley.Dgn.RotMatrix): void {
    var b = a.Determinant();
    var aTranspose = a.Transpose();
    var c = aTranspose.Determinant();
    checker.NearDouble(b,c,true);
    }

    function SetAt(a : Bentley.Dgn.RotMatrix, n: number): void {
        a.SetAt(1,1,n);
        checker.NearDouble(a.At(1,1),n,true);
    }
    function Diagonal(x: number, y: number, z: number): void {

        var a = Bentley.Dgn.RotMatrix.CreateScale(x,y,z);
        var testDeterminant = x * y * z;
        var detA = a.Determinant();
        checker.NearDouble(detA,testDeterminant,true);
        checker.CheckBool(a.IsDiagonal(),true);
    }
    function Solve(a : Bentley.Dgn.RotMatrix,v :Bentley.Dgn.DVector3d): void {
        var solved = a.Solve(v);
        var testV = a.MultiplyVector(solved)
        checker.IsNearDVector3d(v,testV);
    }
    function SetRowsAndColumns(u :Bentley.Dgn.DVector3d, v :Bentley.Dgn.DVector3d, w: Bentley.Dgn.DVector3d): void {
        var a = Bentley.Dgn.RotMatrix.CreateRows(u,v,w);
        var b = Bentley.Dgn.RotMatrix.CreateColumns(u,v,w);
        var transposeA = a.Transpose();
        checker.IsNearRotmatrix(b,transposeA,true);
    }
    function MultiplyVectors(a : Bentley.Dgn.RotMatrix,u :Bentley.Dgn.DVector3d): void {
        var test = a.MultiplyVector(u);
        var testXYZ = a.MultiplyXYZ(u.X,u.Y,u.Z);
        checker.IsNearDVector3d(test,testXYZ);
        var aTranspose = a.Transpose();
        var test1 = aTranspose.MultiplyVector(u);
        var test2 = a.MultiplyTransposeVector(u)
        checker.IsNearDVector3d(test1,test2);
        var test3 = a.MultiplyTransposeXYZ(u.X,u.Y,u.Z);
        checker.IsNearDVector3d(test1,test3);
    }
    function ConditionNumber(){
        var m1 = Bentley.Dgn.RotMatrix.CreateRowValues(1.0, 2.0, 3.0, 2.0, 3.0, 1.0, 3.0, 1.0, 2.0);
        var scaled = m1.Clone ();
        scaled.ScaleRowsInPlace(6,6,6);
        scaled.ScaleColumnsInPlace(6,6,6);
        var m1ConditionNumber = m1.ConditionNumber()
        var scaledConditionNumber = scaled.ConditionNumber();
        checker.NearDouble(m1ConditionNumber,scaledConditionNumber,true);
    }
    function Orthogonal(){
    var m1 = Bentley.Dgn.RotMatrix.CreateRowValues(1.0, 2.0, 3.0, 2.0, 3.0, 1.0, 3.0, 1.0, 2.0);
    var m1Transpose = m1.Transpose();
    var m1Inverse = m1.Inverse();
    if(m1Transpose == m1Inverse)
        checker.CheckBool(true,m1.HasUnitLengthMutuallyPerpendicularRowsAndColumns())
    
    else
        checker.CheckBool(false,m1.HasUnitLengthMutuallyPerpendicularRowsAndColumns());
    }
    // Run the tests
    //debugger;
    //Vector
    var a = new Bentley.Dgn.DPoint3d(2, 3, 4);
    var b = new Bentley.Dgn.DPoint3d(10,11, 12);
    var u =  new Bentley.Dgn.DVector3d(3,6,8);
    var v =  new Bentley.Dgn.DVector3d(4,9,14);
    var w =  new Bentley.Dgn.DVector3d(1,-6,2);
    

    FromStartEnd(a,b);
    Negate(w);
    Perpendicular(v);
    Parallel(u);
    Adding(u,v,w);
    CrossProducts(u,v);
    Distances(u,v);
    XY(u,v);
    Rotate(u);

    //Matrix
    var m1 = Bentley.Dgn.RotMatrix.CreateRowValues(1.0, 2.0, 3.0, 2.0, 3.0, 1.0, 3.0, 1.0, 2.0);
    var m2 = Bentley.Dgn.RotMatrix.CreateRowValues(4.0, 5.0, 6.0, 5.0, 6.0, 4.0, 6.0, 4.0, 5.0);
    var n = 13;
    var x = 2;
    var y = 4;
    var z = 16;
    IdentityMatrix(u,n);
    MultiplyMatrix(m1,m2);
    InverseMatrix(m2);
    NinetyDegreeRotation(u);
    Determinants(m2);
    SetAt(m2,n);
    Diagonal(x,y,z);
    Solve(m2,w);
    SetRowsAndColumns(u,v,w);
    MultiplyVectors(m1,v);
    ConditionNumber();
    Orthogonal();



    function ExerciseMesh(mesh: Bentley.Dgn.PolyfaceMesh, expectedFacets : number)
        {
        var faceData = mesh.InspectFaces();
        logMessage("Start Mesh Exercise");
        
        var visitor = Bentley.Dgn.PolyfaceVisitor.CreateVisitor(mesh, 1);
        var facetCount = 0;
        for (visitor.Reset(); visitor.AdvanceToNextFacet();)
            {
            facetCount = facetCount + 1;
            var i = 0;
            var n = visitor.GetEdgeCount();
            for (; i < n; i++)
                {
                var xyz = visitor.GetPoint(i);
                var uv = visitor.GetParam(i);
                var normal = visitor.GetNormal (i);
                }
            }
        checker.NearDouble (expectedFacets, facetCount, true);
        logMessage("End Mesh Exercise");
        }
    
    function t_polyfaceMeshA ()
        {
        var mesh = Bentley.Dgn.PolyfaceMesh.CreateVariableSizeIndexed ();
        mesh.AddPoint (new Bentley.Dgn.DPoint3d(0,0,0));
        mesh.AddPoint (new Bentley.Dgn.DPoint3d(1,0,0));
        mesh.AddPoint (new Bentley.Dgn.DPoint3d(1,1,0));
        mesh.AddPoint (new Bentley.Dgn.DPoint3d(0,1,0));
        mesh.AddPointIndex (1);
        mesh.AddPointIndex (2);
        mesh.AddPointIndex (3);
        mesh.AddPointIndex (4);
        mesh.AddPointIndex (0);
        checker.NearDouble (1, mesh.GetFacetCount (), true);
        ExerciseMesh(mesh, 1);        
        mesh.AddPoint(new Bentley.Dgn.DPoint3d(2, 0, 0));
        mesh.AddPointIndex(2);
        mesh.AddPointIndex(5);
        mesh.AddPointIndex(4);
        mesh.AddPointIndex(0);
        ExerciseMesh(mesh, 2);
        }

    // create and populate regular polygon in a plane parallel to xy.
    // @remarks when isRadiusOuter if TRUE, the first point is placed on the x axis at x=radius.
    // @remarks when isRadiusOuter is FALSE, the first edge is perpendicular to the x axis at x=radius
    // @param [in] origin center of the polygon.
    // @param [in] numSide number of polygon sides.
    // @param [in] radius radius of computed points (see isRadiusOuter parameter)
    // @param [in] isRadiusOuter selects measurement to points (true) or edges (false)
    function RegularXYPolygonPoints (origin : Bentley.Dgn.DPoint3d, numSide:number, radius : number, isRadiusOuter : boolean) : Bentley.Dgn.DPoint3dArray
        {
        var stepDegrees = 360. / numSide;
        var points = new Bentley.Dgn.DPoint3dArray ();
        var startDegrees = isRadiusOuter ? 0.0 : 0.5 * stepDegrees;
        var i = 0;
        for (i = 0; i < numSide; i++)
            {
            var angle = Bentley.Dgn.Angle.CreateDegrees (startDegrees + i * stepDegrees);
            points.Add (origin.Plus (Bentley.Dgn.DVector3d.CreateXYAngleAndMagnitude (angle, radius)));
            }
        // repeat first point
        points.AddXYZ (radius, 0, 0);
        return points;
        }
        var spiral = Bentley.Dgn.SpiralCurve.CreateSpiralBearingRadiusLengthRadius
    function LoopFromPoints (points: Bentley.Dgn.DPoint3dArray) : Bentley.Dgn.Geometry
        {
        var loop = new Bentley.Dgn.Loop ()
        loop.Add (new Bentley.Dgn.LineString (points));
        return loop;
        }

    function TryTransformGeometryArrayInPlace (geometry: Array<Bentley.Dgn.Geometry>, transform: Bentley.Dgn.Transform)
        {
        var i = 0;
       for (;i < geometry.length;i++)
            geometry[i].TryTransformInPlace (transform);
        }
    function CloneAndTransformGeometryArray (geometry :Array<Bentley.Dgn.Geometry>, transform: Bentley.Dgn.Transform):Array<Bentley.Dgn.Geometry>
        {
        var i = 0;
        var result = new Array<Bentley.Dgn.Geometry> ();
        for (;i < geometry.length;i++)
            result.push (geometry[i].Clone ());
        if (transform != null)
            TryTransformGeometryArrayInPlace (geometry, transform);
        return result;
        }

    


    function t_arrayOfGeometry ()
        {        
        var geometry1: Bentley.Dgn.Geometry[];
        geometry1.push (LoopFromPoints (RegularXYPolygonPoints (new Bentley.Dgn.DPoint3d (0,0,0), 4, 1.0, false)));
        var geometry2 = CloneAndTransformGeometryArray (geometry1, Bentley.Dgn.Transform.CreateTranslationXYZ (1,0,3));
        }

    function t_regularPolygon ()
        {
        var nut =
            Bentley.Dgn.DgnExtrusion.Create
                    (
                    Bentley.Dgn.Loop.CreateRegularPolygonXY (
                        new Bentley.Dgn.DPoint3d (1,1,3),
                        2.0, 6, true),
                    new Bentley.Dgn.DVector3d (0,0,1),
                    true
                    );
        }
    t_polyfaceMeshA ();  
    t_regularPolygon ();
    logMessage('Test1 Z');
}

