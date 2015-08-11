//! Script that is executed by one of the unit tests in DgnScriptContext_Test.cpp
module DgnScriptTests {
    
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

    // Run the tests
    test1();
}