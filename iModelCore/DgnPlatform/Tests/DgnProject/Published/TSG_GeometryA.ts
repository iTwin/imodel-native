//! Script that is executed by one of the unit tests in DgnScriptContext_Test.cpp
module DgnScriptTests {
    BentleyApi.Dgn.JsUtils.ReportError(':Test1 A');
    
    var checker = new DgnScriptChecker.Checker();
    BentleyApi.Dgn.JsUtils.ReportError(':Test1 B');
    // Test1 - DPoint3d properties
    function test1(): void {
        var point = new BentleyApi.Dgn.JsDPoint3d(2, 1, 3);
        if (point.X != 2)
            BentleyApi.Dgn.JsUtils.ReportError('bad X');
        if (point.Y != 1)
            BentleyApi.Dgn.JsUtils.ReportError('bad Y');
        if (point.Z != 3)
            BentleyApi.Dgn.JsUtils.ReportError('bad Z');
    }

    function test2(): void {
        var pointA = new BentleyApi.Dgn.JsDPoint3d(2, 1, 3);
        var pointB = new BentleyApi.Dgn.JsDPoint3d(4, 0.4, 2);
        var segmentC = new BentleyApi.Dgn.JsDSegment3d(pointA, pointB);
        var curveD = BentleyApi.Dgn.JsCurvePrimitive.CreateLineSegment(segmentC);
        var s = 0.4;
        var pointC1 = segmentC.PointAtFraction(s);
        var pointD1 = curveD.PointAtFraction(s);
        checker.IsNearJsDPoint3d(pointD1, pointD1);
    }

    function test3(): void {
        var arrayA = new BentleyApi.Dgn.JsDPoint3dArray();
        arrayA.Add(new BentleyApi.Dgn.JsDPoint3d(1, 2, 3));
        arrayA.Add(new BentleyApi.Dgn.JsDPoint3d(2, 3, 4));
        arrayA.Add(new BentleyApi.Dgn.JsDPoint3d(4, 3, 4));
        checker.NearDouble(3, arrayA.Size(), true);
        var cpLineStringA = BentleyApi.Dgn.JsCurvePrimitive.CreateLineString(arrayA);
        var pointB = cpLineStringA.PointAtFraction(0.5);


    }
    function Negate(vecA: BentleyApi.Dgn.JsDVector3d): void {
        var vecB = vecA.Clone ();
        //Fix
        var vecC = vecB.Negate();
        vecC = vecC.Negate();
        checker.IsNearJsDVector3d(vecC, vecA);
    }
    function FromStartEnd(pointA: BentleyApi.Dgn.JsDPoint3d, pointB: BentleyApi.Dgn.JsDPoint3d): void {
        var vectorA = new BentleyApi.Dgn.JsDVector3d(pointB.X - pointA.X, pointB.Y - pointA.Y, pointB.Z - pointA.Z);
        var resultA = BentleyApi.Dgn.JsDVector3d.FromStartEnd(pointA, pointB);
        checker.IsNearJsDVector3d(resultA, vectorA);
        var normalizedA = resultA.Normalize();
        var resultB = BentleyApi.Dgn.JsDVector3d.FromStartEndNormalize(pointA, pointB);
        checker.NearDouble(normalizedA.Magnitude(), resultB.Magnitude(), true);

    }
    function Perpendicular(v: BentleyApi.Dgn.JsDVector3d): void {
        v.X = 0;
        var u = v.UnitPerpendicularXY(v);

        checker.NearDouble(0, v.DotProduct(u), true);

        checker.CheckBool(true, v.IsPerpendicularTo(u));
    }
    function Parallel(v: BentleyApi.Dgn.JsDVector3d): void {
        var u = v.Scale(2);
        checker.NearDouble(0, v.CrossProductMagnitude(u), true);
        checker.CheckBool(true, u.IsParallelTo(v));
    }
    function Adding(u: BentleyApi.Dgn.JsDVector3d, v: BentleyApi.Dgn.JsDVector3d, w: BentleyApi.Dgn.JsDVector3d): void {
        var vu = v.Plus(u);
        var uv = u.Plus(v);
        var v2 = uv.Minus (u);
        var uvFromAdd2 = u.PlusScaled (v, 2);
        checker.IsNearJsDVector3d(vu, uv);
        checker.IsNearJsDVector3d(v2, v);
        var uvAddScaled = u.PlusScaled (v, 2);
        checker.NearDouble(uvAddScaled.Magnitude(), uvFromAdd2.Magnitude(), true);
        var vuwFromAdd3 = u.Plus2Scaled (v, 1, w, 1);
        var uvw = uv.Plus (w);
        checker.IsNearJsDVector3d(vuwFromAdd3, uvw);
        vu = vu.ScaleToLength(2);
        uv = uv.Normalize();
        uv = uv.Scale(2);
        checker.NearDouble(uv.Magnitude(), vu.Magnitude(), true);


    }
    function CrossProducts(u: BentleyApi.Dgn.JsDVector3d, v: BentleyApi.Dgn.JsDVector3d): void {
        var VCrossU = v.CrossProduct(u);
        var NormalizedVCrossU = v.NormalizedCrossProduct(u);
        VCrossU = VCrossU.Normalize();
        checker.IsNearJsDVector3d(NormalizedVCrossU, VCrossU);

        VCrossU.SizedCrossProduct(v, u, 4);
        NormalizedVCrossU.Scale(4);
        checker.IsNearJsDVector3d(VCrossU, NormalizedVCrossU);
    }
    function Distances(u: BentleyApi.Dgn.JsDVector3d, v: BentleyApi.Dgn.JsDVector3d): void {
        var Dist = u.Distance(v);
        var uv = u.Minus (v);
        checker.NearDouble(Dist, uv.Magnitude(), true);
        var DistSq = u.DistanceSquared(v);
        var uvMagnitudeSq = uv.MagnitudeSquared();
        checker.NearDouble(DistSq, uvMagnitudeSq, true);
    }
    function XY(u: BentleyApi.Dgn.JsDVector3d, v: BentleyApi.Dgn.JsDVector3d): void {
        var uXY = new BentleyApi.Dgn.JsDVector3d(u.X, u.Y, 0);
        var vXY = new BentleyApi.Dgn.JsDVector3d(v.X, v.Y, 0);
        var uvXYDotProduct = uXY.DotProduct(vXY);
        var uvDotProductXY = u.DotProductXY(v);
        checker.NearDouble(uvXYDotProduct, uvDotProductXY, true);
        var VCrossUXY = v.CrossProductXY(u);
        var VCrossU = vXY.CrossProduct(uXY);
        checker.NearDouble(VCrossU.MagnitudeSquared(), VCrossUXY * VCrossUXY, true);

    }
    function Rotate(u: BentleyApi.Dgn.JsDVector3d): void {
        u.Z = 0;
        var degrees90 = BentleyApi.Dgn.JsAngle.CreateDegrees(90);
        var v = u.RotateXY(degrees90.Radians);
        checker.NearDouble(u.AngleTo(v).Radians, degrees90.Radians, true);
    }

    //Rotmatrix tests
    function IdentityMatrix(u :BentleyApi.Dgn.JsDVector3d, n: number): void {
        var identity = BentleyApi.Dgn.JsRotMatrix.CreateIdentity();
        var multiplied = identity.MultiplyVector(u);
        checker.IsNearJsDVector3d(u,multiplied);
        var identityX =  BentleyApi.Dgn.JsRotMatrix.CreateUniformScale(n);
        var identityScaledRows = identity.Clone ();
        var identityScaledColumns = identity.Clone ();
        identityScaledColumns.ScaleColumnsInPlace(n,n,n);
        identityScaledRows.ScaleRowsInPlace(n,n,n);
        checker.IsNearJsRotmatrix(identityX,identityScaledRows,true)
        checker.IsNearJsRotmatrix(identityX,identityScaledColumns,true);
        checker.CheckBool(identity.IsIdentity(),true);
    }
    function MultiplyMatrix(a : BentleyApi.Dgn.JsRotMatrix, b : BentleyApi.Dgn.JsRotMatrix): void {
    var product = a.MultiplyMatrix(b);
    var productTranspose = product.Transpose();
    var aTranspose = a.Transpose();
    var bTranspose = b.Transpose();
    var testProductTranspose = aTranspose.MultiplyMatrix(bTranspose);
    checker.IsNearJsRotmatrix(testProductTranspose,productTranspose,true);
    }
    function InverseMatrix(b : BentleyApi.Dgn.JsRotMatrix): void {
    var inverseB = b.Inverse();
    var testIdentity = b.MultiplyMatrix(inverseB);
    var identity = BentleyApi.Dgn.JsRotMatrix.CreateIdentity();
    checker.IsNearJsRotmatrix(identity,testIdentity,true);
    }
    function NinetyDegreeRotation(u :BentleyApi.Dgn.JsDVector3d): void {
    var a = BentleyApi.Dgn.JsRotMatrix.Create90DegreeRotationAroundVector(v);
    checker.CheckBool(a.IsRigid(),true);
    var degrees90 = BentleyApi.Dgn.JsAngle.CreateDegrees(90);
    var b = BentleyApi.Dgn.JsRotMatrix.CreateRotationAroundVector(v,degrees90)
    checker.IsNearJsRotmatrix(a,b,true);
    var v1 = a.MultiplyVector(v);
    var v2 = b.MultiplyVector(v);
    checker.IsNearJsDVector3d(v1,v2);

    }
    function Determinants(a : BentleyApi.Dgn.JsRotMatrix): void {
    var b = a.Determinant();
    var aTranspose = a.Transpose();
    var c = aTranspose.Determinant();
    checker.NearDouble(b,c,true);
    }

    function SetAt(a : BentleyApi.Dgn.JsRotMatrix, n: number): void {
        a.SetAt(1,1,n);
        checker.NearDouble(a.At(1,1),n,true);
    }
    function Diagonal(x: number, y: number, z: number): void {

        var a = BentleyApi.Dgn.JsRotMatrix.CreateScale(x,y,z);
        var testDeterminant = x * y * z;
        var detA = a.Determinant();
        checker.NearDouble(detA,testDeterminant,true);
        checker.CheckBool(a.IsDiagonal(),true);
    }
    function Solve(a : BentleyApi.Dgn.JsRotMatrix,v :BentleyApi.Dgn.JsDVector3d): void {
        var solved = a.Solve(v);
        var testV = a.MultiplyVector(solved)
        checker.IsNearJsDVector3d(v,testV);
    }
    function SetRowsAndColumns(u :BentleyApi.Dgn.JsDVector3d, v :BentleyApi.Dgn.JsDVector3d, w: BentleyApi.Dgn.JsDVector3d): void {
        var a = BentleyApi.Dgn.JsRotMatrix.CreateRows(u,v,w);
        var b = BentleyApi.Dgn.JsRotMatrix.CreateColumns(u,v,w);
        var transposeA = a.Transpose();
        checker.IsNearJsRotmatrix(b,transposeA,true);
    }
    function MultiplyVectors(a : BentleyApi.Dgn.JsRotMatrix,u :BentleyApi.Dgn.JsDVector3d): void {
        var test = a.MultiplyVector(u);
        var testXYZ = a.MultiplyXYZ(u.X,u.Y,u.Z);
        checker.IsNearJsDVector3d(test,testXYZ);
        var aTranspose = a.Transpose();
        var test1 = aTranspose.MultiplyVector(u);
        var test2 = a.MultiplyTransposeVector(u)
        checker.IsNearJsDVector3d(test1,test2);
        var test3 = a.MultiplyTransposeXYZ(u.X,u.Y,u.Z);
        checker.IsNearJsDVector3d(test1,test3);
    }
    function ConditionNumber(){
        var m1 = BentleyApi.Dgn.JsRotMatrix.CreateRowValues(1.0, 2.0, 3.0, 2.0, 3.0, 1.0, 3.0, 1.0, 2.0);
        var scaled = m1.Clone ();
        scaled.ScaleRowsInPlace(6,6,6);
        scaled.ScaleColumnsInPlace(6,6,6);
        var m1ConditionNumber = m1.ConditionNumber()
        var scaledConditionNumber = scaled.ConditionNumber();
        checker.NearDouble(m1ConditionNumber,scaledConditionNumber,true);
    }
    function Orthogonal(){
    var m1 = BentleyApi.Dgn.JsRotMatrix.CreateRowValues(1.0, 2.0, 3.0, 2.0, 3.0, 1.0, 3.0, 1.0, 2.0);
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
    var a = new BentleyApi.Dgn.JsDPoint3d(2, 3, 4);
    var b = new BentleyApi.Dgn.JsDPoint3d(10,11, 12);
    var u =  new BentleyApi.Dgn.JsDVector3d(3,6,8);
    var v =  new BentleyApi.Dgn.JsDVector3d(4,9,14);
    var w =  new BentleyApi.Dgn.JsDVector3d(1,-6,2);
    

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
    var m1 = BentleyApi.Dgn.JsRotMatrix.CreateRowValues(1.0, 2.0, 3.0, 2.0, 3.0, 1.0, 3.0, 1.0, 2.0);
    var m2 = BentleyApi.Dgn.JsRotMatrix.CreateRowValues(4.0, 5.0, 6.0, 5.0, 6.0, 4.0, 6.0, 4.0, 5.0);
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



    function ExerciseMesh(mesh: BentleyApi.Dgn.JsPolyfaceMesh, expectedFacets : number)
        {
        var faceData = mesh.InspectFaces();
        BentleyApi.Dgn.JsUtils.ReportError(":Start Mesh Exercise");
        
        var visitor = BentleyApi.Dgn.JsPolyfaceVisitor.CreateVisitor(mesh, 1);
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
        BentleyApi.Dgn.JsUtils.ReportError(":End Mesh Exercise");
        }
    
    function t_polyfaceMeshA ()
        {
        var mesh = BentleyApi.Dgn.JsPolyfaceMesh.CreateVariableSizeIndexed ();
        mesh.AddPoint (new BentleyApi.Dgn.JsDPoint3d(0,0,0));
        mesh.AddPoint (new BentleyApi.Dgn.JsDPoint3d(1,0,0));
        mesh.AddPoint (new BentleyApi.Dgn.JsDPoint3d(1,1,0));
        mesh.AddPoint (new BentleyApi.Dgn.JsDPoint3d(0,1,0));
        mesh.AddPointIndex (1);
        mesh.AddPointIndex (2);
        mesh.AddPointIndex (3);
        mesh.AddPointIndex (4);
        mesh.AddPointIndex (0);
        checker.NearDouble (1, mesh.GetFacetCount (), true);
        ExerciseMesh(mesh, 1);        
        mesh.AddPoint(new BentleyApi.Dgn.JsDPoint3d(2, 0, 0));
        mesh.AddPointIndex(2);
        mesh.AddPointIndex(5);
        mesh.AddPointIndex(4);
        mesh.AddPointIndex(0);
        ExerciseMesh(mesh, 2);
        }

    t_polyfaceMeshA ();  

    BentleyApi.Dgn.JsUtils.ReportError(':Test1 Z');
}

