/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnScriptTest/DgnScriptTest.ts $
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
        // Try out the new categories iterator
        var catid: be.DgnObjectId = null;
        var categories: be.DgnObjectIdSet = be.DgnCategory.QueryCategories(db);
        for (var catiter = categories.Begin(); categories.IsValid(catiter); categories.ToNext(catiter))
            {
            var cat = be.DgnCategory.QueryCategory(categories.GetId(catiter), db);
            if (cat.CategoryName == params.categoryName)
                catid = cat.CategoryId;
            }
        if (!catid)
            be.Script.ReportError('failed to find category ' + params.categoryName + ' using category iterator');

        //  Verify that we can find it the easy way, too.
        if (!catid.Equals(be.DgnCategory.QueryCategoryId(params.categoryName, db)))
            be.Script.ReportError('QueryCategoryId failed?');

        //  Create element
        var model = db.Models.GetModel(db.Models.QueryModelId(be.DgnModel.CreateModelCode(params.modelName)));
        var ele = be.PhysicalElement.Create(model, catid, '');

        //  Try out SolidPrimitive
        var builder = new be.ElementGeometryBuilder(ele, new be.DPoint3d(0, 0, 0), new be.YawPitchRollAngles(0, 0, 0));

        var sphere = be.DgnSphere.CreateSphere(new be.DPoint3d(0, 0, 0), 1.0);
        builder.Append(sphere);

        if (0 != builder.SetGeomStreamAndPlacement(ele))
            be.Script.ReportError('SetGeomStreamAndPlacement failed');

        ele.Insert();

        return 0;
    }

    be.RegisterDgnDbScript('DgnScriptTests.TestDgnDbScript', TestDgnDbScript);
}

