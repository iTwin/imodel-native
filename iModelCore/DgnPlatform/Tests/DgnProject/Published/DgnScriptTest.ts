//! Script that is executed by one of the unit tests in DgnScriptContext_Test.cpp
module DgnScriptTests {
    
    class Checker {
        AbsTol: number;
        RelTol: number;

        constructor () {
            this.AbsTol = 1.0e-12;
            this.RelTol = 1.0e-12;
        }

        Abs(a: number) : number { return a >= 0 ? a : -a; }
        ConstructErrorString(a: number, b: number) : string 
            {
            var labelA = "Value of A was: "
            var labelB = "Value of B was: "
            var fullA = labelA.concat(a.toString());
            var fullB = labelB.concat(b.toString());
            var fullString = fullA.concat(fullB);
            return fullString;
            }
        NearDouble(a: number, b: number, reportError : boolean) : boolean
        {
        var d = this.Abs(b - a);
        if (d < this.AbsTol)
            return true;
        if (reportError)
            var message = this.ConstructErrorString(a,b);
            BentleyApi.Dgn.JsUtils.ReportError(message);
        return false;
        }

        IsNearJsDPoint3d(a: BentleyApi.Dgn.JsDPoint3d, b: BentleyApi.Dgn.JsDPoint3d) {
            if (    this.NearDouble(this.Abs(a.X - b.X), 0.0, false)
                && this.NearDouble(this.Abs(a.Y - b.Y), 0.0, false)
                && this.NearDouble(this.Abs(a.Z - b.Z), 0.0, false)
                )
                return true;
            BentleyApi.Dgn.JsUtils.ReportError('NearPoint');
            return false;
        }
        IsNearJsDVector3d(a: BentleyApi.Dgn.JsDVector3d, b: BentleyApi.Dgn.JsDVector3d) {
            if (    this.NearDouble(this.Abs(a.X - b.X), 0.0, false)
                && this.NearDouble(this.Abs(a.Y - b.Y), 0.0, false)
                && this.NearDouble(this.Abs(a.Z - b.Z), 0.0, false)
                )
                return true;
            BentleyApi.Dgn.JsUtils.ReportError('NearVector');
            return false;
        }
        CheckBool(a: boolean, b: boolean) : boolean
            {
            if(a==b)
                return true;
            BentleyApi.Dgn.JsUtils.ReportError("Not Equal");
            return false;
            }
    }
    var checker = new Checker();
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
    function Negate(vecA :BentleyApi.Dgn.JsDVector3d): void{
        var vecB = BentleyApi.Dgn.JsDVector3d.CreateClone(vecA);
        //Fix
        var vecC = vecB.Negate();
        vecC = vecC.Negate();
        checker.IsNearJsDVector3d(vecC,vecA);            
    }
    function FromStartEnd(pointA : BentleyApi.Dgn.JsDPoint3d, pointB : BentleyApi.Dgn.JsDPoint3d): void{
        var vectorA = new BentleyApi.Dgn.JsDVector3d(pointB.X - pointA.X,pointB.Y - pointA.Y,pointB.Z - pointA.Z);
        var resultA = BentleyApi.Dgn.JsDVector3d.FromStartEnd(pointA,pointB);
        checker.IsNearJsDVector3d(resultA,vectorA);
        var normalizedA = resultA.Normalize();
        var resultB = BentleyApi.Dgn.JsDVector3d.FromStartEndNormalize(pointA,pointB);
        checker.NearDouble(normalizedA.Magnitude(),resultB.Magnitude(),true);

    }
    function Perpendicular(v :BentleyApi.Dgn.JsDVector3d): void{
        v.X = 0;
        var u = v.UnitPerpendicularXY(v);

        checker.NearDouble(0,v.DotProduct(u),true);

        checker.CheckBool(true,v.IsPerpendicularTo(u));
    }
    function Parallel(v :BentleyApi.Dgn.JsDVector3d): void{
        var u = v.Scale(2);
        checker.NearDouble(0,v.CrossProductMagnitude(u),true);
        checker.CheckBool(true,u.IsParallelTo(v));
    }
    function Adding(u :BentleyApi.Dgn.JsDVector3d, v :BentleyApi.Dgn.JsDVector3d, w :BentleyApi.Dgn.JsDVector3d): void{
        var vu = v.Add(u);
        var uv = u.Add(v);
        var v2 = uv.Subtract(u);
        var uvFromAdd2 = BentleyApi.Dgn.JsDVector3d.FromAdd2Scaled(u,1,v,2);
        checker.IsNearJsDVector3d(vu,uv);
        checker.IsNearJsDVector3d(v2,v);
        var uvAddScaled = u.AddScaled(v,2);
        checker.NearDouble(uvAddScaled.Magnitude(),uvFromAdd2.Magnitude(),true);
        var vuwFromAdd3 = BentleyApi.Dgn.JsDVector3d.FromAdd3Scaled(u,1,v,1,w,1);
        var uvw = uv.Add(w);
        checker.IsNearJsDVector3d(vuwFromAdd3,uvw);
        vu = vu.ScaleToLength(2);
        uv = uv.Normalize();
        uv = uv.Scale(2);
        checker.NearDouble(uv.Magnitude(),vu.Magnitude(),true);


    }
    function CrossProducts(u :BentleyApi.Dgn.JsDVector3d, v :BentleyApi.Dgn.JsDVector3d): void {
        var VCrossU = v.CrossProduct(u);
        var NormalizedVCrossU = v.NormalizedCrossProduct(u);
        VCrossU = VCrossU.Normalize();
        checker.IsNearJsDVector3d(NormalizedVCrossU,VCrossU);
        
        VCrossU.SizedCrossProduct(v,u,4);
        NormalizedVCrossU.Scale(4);
        checker.IsNearJsDVector3d(VCrossU, NormalizedVCrossU);
    }
    function Distances(u :BentleyApi.Dgn.JsDVector3d, v :BentleyApi.Dgn.JsDVector3d): void {
        var Dist = u.Distance(v);
        var uv = u.Subtract(v);
        checker.NearDouble(Dist, uv.Magnitude(),true);
        var DistSq = u.DistanceSquared(v);
        var uvMagnitudeSq = uv.MagnitudeSquared();
        checker.NearDouble(DistSq, uvMagnitudeSq,true);
    }
    function XY(u :BentleyApi.Dgn.JsDVector3d, v :BentleyApi.Dgn.JsDVector3d): void{
        var uXY = new BentleyApi.Dgn.JsDVector3d(u.X,u.Y,0);
        var vXY = new BentleyApi.Dgn.JsDVector3d(v.X,v.Y,0);
        var uvXYDotProduct = uXY.DotProduct(vXY);
        var uvDotProductXY = u.DotProductXY(v);
        checker.NearDouble(uvXYDotProduct,uvDotProductXY,true);
        var VCrossUXY = v.CrossProductXY(u);
        var VCrossU = vXY.CrossProduct(uXY);
        checker.NearDouble(VCrossU.MagnitudeSquared(),VCrossUXY * VCrossUXY,true);

    }
    function Rotate(u :BentleyApi.Dgn.JsDVector3d): void {
        u.Z = 0;
        var degrees90 = BentleyApi.Dgn.JsAngle.CreateDegrees(90);
        var v = u.RotateXY(degrees90.Radians);
        checker.NearDouble(u.AngleTo(v).Radians , degrees90.Radians, true);
    }


    // Run the tests
    //debugger;
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
}