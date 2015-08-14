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
 
        NearDouble(a: number, b: number, reportError : boolean) : boolean
        {
        var d = this.Abs(b - a);
        if (d < this.AbsTol)
            return true;
        if (reportError)
            BentleyApi.Dgn.JsUtils.ReportError('NearDouble');
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

    // Run the tests
    debugger;

    //BentleyApi.Dgn.JsUtils.ReportError('hello world');
    test1();
    test2();
    test3();
}