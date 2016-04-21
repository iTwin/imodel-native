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
    function PopulateProperties(el: be.GeometricElement3d): void {

        el.SetProperty("IntegerProperty1", be.ECValue.FromInteger(1));
        el.SetProperty("IntegerProperty2", be.ECValue.FromInteger(2));
        el.SetProperty("IntegerProperty3", be.ECValue.FromInteger(3));
        el.SetProperty("IntegerProperty4", be.ECValue.FromInteger(4));
        el.SetProperty("DoubleProperty1", be.ECValue.FromDouble(1));
        el.SetProperty("DoubleProperty2", be.ECValue.FromDouble(2));
        el.SetProperty("DoubleProperty3", be.ECValue.FromDouble(3));
        el.SetProperty("DoubleProperty4", be.ECValue.FromDouble(4));
        el.SetProperty("PointProperty1",  be.ECValue.FromPoint3d(new be.DPoint3d(1,2,3))); 
        el.SetProperty("PointProperty2",  be.ECValue.FromPoint3d(new be.DPoint3d(2,2,3))); 
        el.SetProperty("PointProperty3",  be.ECValue.FromPoint3d(new be.DPoint3d(3,2,3))); 
        el.SetProperty("PointProperty4",  be.ECValue.FromPoint3d(new be.DPoint3d(4,2,3))); 
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    function Test(db: be.DgnDb, params: Params): number
    {
        var model = db.Models.GetModel(db.Models.QueryModelId(be.DgnModel.CreateModelCode(params.newModelName)));
        var catid = be.DgnCategory.QueryCategoryId(params.categoryName, db);

        //var starti = Date.now();
        var lowcommit = 9999999;
        var highcommit = 0;
        var ncommits = 0;
        var totalcommit = 0;

        for (var i = 0; i < 10; ++i) {

            //var startj = Date.now();
            var kvalues = "";
            var comma = "";

            for (var j = 0; j < 10; ++j) {

                var startk = Date.now();

                for (var k = 0; k < 10000; ++k) {
                    var ele = be.GeometricElement3d.CreateGeometricElement3d(model, catid, 'DgnPlatformTest.TestElementWithNoHandler');
                    var builder = new be.GeometryBuilder(ele, new be.DPoint3d(i, j, k), new be.YawPitchRollAngles(0, 0, 0));
                    var geom: be.SolidPrimitive;
                    if (k % 3 == 0)
                        geom = be.DgnSphere.CreateSphere(new be.DPoint3d(0, 0, 0), 1.0);
                    else if (k % 3 == 1)
                        geom = be.DgnBox.CreateCenteredBox(new be.DPoint3d(0, 0, 0), new be.DVector3d(1, 1, 1), true);
                    else
                        geom = be.DgnCone.CreateCircularCone(new be.DPoint3d(0, 0, 0), new be.DPoint3d(0, 0, 1), 1, 1, true);
                    builder.AppendGeometry(geom);
                    if (0 != builder.SetGeometryStreamAndPlacement(ele)) {
                        be.Script.ReportError('SetGeometryStreamAndPlacement failed');
                        return -1;
                    }
                    //PopulateProperties(ele);
                    if (0 != ele.Insert()) {
                        be.Script.ReportError('Insert failed');
                        return -1;
                    }
                    // make it easier on GC by releasing the objects that I know that I don't need any more.
                    builder.Dispose(); builder = null;
                    geom.Dispose(); geom = null;
                    ele.Dispose(); ele = null;
                }

                var endk = Date.now();
                var elapsedk = endk - startk;
                kvalues = kvalues + comma + elapsedk;
                comma = ",";

                db.SaveChanges();

                var endcommitk = Date.now();
                var elapsedcommit = endcommitk - endk;
                //be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, "k commit:" + elapsedk);
                if (elapsedcommit < lowcommit)
                    lowcommit = elapsedcommit;
                if (elapsedcommit > highcommit)
                    highcommit = elapsedcommit;
                totalcommit += elapsedcommit;
                ++ncommits;
            
            }

            be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, kvalues);
            //var endj = Date.now();
            //var elapsedj = endj - startj;
            //be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, "0:" + elapsedj);
        }

        //var endi = Date.now();
        //var elapsedi = endi - starti;
        //be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, "i:" + elapsedi);

        be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, "commit time: low=" + lowcommit + " high=" + highcommit + " avg=" + (totalcommit / ncommits));

        return 0;
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    be.RegisterDgnDbScript('DgnScriptTests.TestDgnDbScriptPerformance', Test);
    be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, "registered TestDgnDbScriptPerformance");
}

