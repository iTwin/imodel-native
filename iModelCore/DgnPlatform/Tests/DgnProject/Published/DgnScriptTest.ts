/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnScriptTest.ts $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// Script that is executed by one of the unit tests in DgnScriptContext_Test.cpp
module DgnScriptTests {

    import be = Bentley.Dgn;

    function TestDgnDbScript(db : be.DgnDb, params: any): number
    {
        var spdetail: be.DgnSphereDetail = be.DgnSphereDetail.CreateSphere(new be.DPoint3d(0, 0, 0), 1.0);
        var solid: be.SolidPrimitive = be.SolidPrimitive.CreateDgnSphere(spdetail);

        Bentley.Dgn.Logging.Message('TestRunner', Bentley.Dgn.LoggingSeverity.Info, 'Hello from DgnScriptTests.TestDgnDbScript');
        return 0;
    }

    be.RegisterDgnDbScript('DgnScriptTests.TestDgnDbScript', TestDgnDbScript);
}

