/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnScriptTest/DgnScriptTestPerformance.ts $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// Script that is executed by one of the unit tests in DgnScriptContext_Test.cpp
module DgnScriptPerformanceTests {

    import be = Bentley.Dgn;

    //---------------------------------------------------------------------------------------
    // *** NB: Keep this consistent with {DgnScriptContext_Test.cpp}TEST_F(DgnScriptTest, RunScriptPerformanceTests)
    // @bsimethod                                   
    //---------------------------------------------------------------------------------------
    class Params
    {
        modelName: string;
        newModelName: string;
        categoryName: string;
    };

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    function Test(db: be.DgnDb, params: Params): number
    {
        var model = db.Models.GetModel(db.Models.QueryModelId(be.DgnModel.CreateModelCode(params.newModelName)));
        var catid = be.DgnCategory.QueryCategoryId(params.categoryName, db);

        var starti = Date.now();

        for (var i = 0; i < 10; ++i) {

            var startj = Date.now();

            for (var j = 0; j < 10; ++j) {

                var startk = Date.now();

                for (var k = 0; k < 10000; ++k) {
                    var ele = be.GeometricElement3d.CreateGeometricElement3d(model, catid, 'DgnPlatformTest.TestElementWithNoHandler');
                    var builder = new be.GeometryBuilder(ele, new be.DPoint3d(i, j, k), new be.YawPitchRollAngles(0, 0, 0));
                    var sphere = be.DgnSphere.CreateSphere(new be.DPoint3d(0, 0, 0), 1.0);
                    builder.AppendGeometry(sphere);
                    if (0 != builder.SetGeometryStreamAndPlacement(ele)) {
                        be.Script.ReportError('SetGeometryStreamAndPlacement failed');
                        return -1;
                    }
                    if (0 != ele.Insert()) {
                        be.Script.ReportError('Insert failed');
                        return -1;
                    }
                    // make it easier on GC by releasing the objects that I know that I don't need any more.
                    builder.Dispose(); builder = null;
                    sphere.Dispose(); sphere = null;
                    ele.Dispose(); ele = null;
                }

                db.SaveChanges();

                var endk = Date.now();
                var elapsedk = endk - startk;
                be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, "k:" + elapsedk);
            
            }

            var endj = Date.now();
            var elapsedj = endj - startj;
            be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, "j:" + elapsedj);
        }

        var endi = Date.now();
        var elapsedi = endi - starti;
        be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, "i:" + elapsedi);

        return 0;
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    be.RegisterDgnDbScript('DgnScriptTests.TestDgnDbScriptPerformance', Test);
    be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, "registered TestDgnDbScriptPerformance");
}

