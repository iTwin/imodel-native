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

    // *** NB: Keep this consistent with {DgnScriptContext_Test.cpp}TEST_F(DgnScriptTest, RunScripts)
    class Params
    {
        modelName: string;
        categoryName: string;
    };

    function TestDgnDbScript(db: be.DgnDb, params: Params): number
    {
        var model = db.Models.GetModel(db.Models.QueryModelId(be.DgnModel.CreateModelCode(params.modelName)));

        var ele = be.PhysicalElement.Create(model, params.categoryName, '');

        var builder = new be.ElementGeometryBuilder(ele, new be.DPoint3d(0, 0, 0), new be.YawPitchRollAngles(0, 0, 0));

        var spdetail: be.DgnSphereDetail = be.DgnSphereDetail.CreateSphere(new be.DPoint3d(0, 0, 0), 1.0);
        var solid: be.SolidPrimitive = be.SolidPrimitive.CreateDgnSphere(spdetail);
        builder.Append(solid);

        if (0 != builder.SetGeomStreamAndPlacement(ele))
            Bentley.Dgn.Logging.Message('TestRunner', Bentley.Dgn.LoggingSeverity.Error, 'SetGeomStreamAndPlacement failed');

        ele.Insert();

        Bentley.Dgn.Logging.Message('TestRunner', Bentley.Dgn.LoggingSeverity.Info, 'Hello from DgnScriptTests.TestDgnDbScript');
        return 0;
    }

    be.RegisterDgnDbScript('DgnScriptTests.TestDgnDbScript', TestDgnDbScript);
}

